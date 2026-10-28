/* SPDX-License-Identifier: GPL-3.0-only */
/* Nauclerus adapter for the unmodified X0X-RMX v1.1.0 musical core.
 * Configurations belong to physical tracks; transport counters never do.
 * Manual edits are prepared outside the ISR, then copied at a step boundary.
 * Automatic mutation is the source's bounded, once-per-pattern-wrap operation. */
#include "tb3po.c"
typedef struct {
    tb3po_cfg_t cfg;
    uint32_t rng, bars;
    uint8_t stale;
} tb_state_t;
static tb_state_t tb_live[NTRK], tb_song[4][NTRK];
static step_t tb_song_steps[4][NTRK][NSTEP] __attribute__((section(".pool")));
static step_t tb_pending[NTRK][NSTEP] __attribute__((section(".pool")));
static volatile uint8_t tb_pending_valid[NTRK];
static void tb_reset_runtime(tb_state_t *s) { s->rng = s->bars = 0; }
static void tb_defaults_all(void)
{
    uint32_t k;
    for (k = 0; k < NTRK; k++) {
        memset(&tb_live[k], 0, sizeof tb_live[k]);
        tb3po_defaults(&tb_live[k].cfg, 0xBEEFu + k);
        tb_pending_valid[k] = 0;
    }
    memset(tb_song, 0, sizeof tb_song);
}
static void tb_read_part(bpart_t *b, const step_t *s, const track_t *t, const tb_state_t *g)
{
    uint32_t i;
    memset(b, 0, sizeof *b);
    b->len = (uint8_t)clamp(t->p[P_SLEN], 1, NSTEP);
    b->gen = g->cfg;
    for (i = 0; i < NSTEP; i++) {
        b->step[i].note = (uint8_t)clamp((int32_t)s[i].nt + 60, 0, 127);
        b->step[i].flags = (uint8_t)((s[i].tie ? G_TIE : s[i].on ? G_NOTE : G_REST) |
                                    (s[i].acc ? BS_ACCENT : 0) | (s[i].slide ? BS_SLIDE : 0));
    }
}
static int tb_write_part(step_t *dst, const bpart_t *b, const bstep_t *before)
{
    uint32_t i, n = before ? b->len : NSTEP;
    int changed = 0;
    for (i = 0; i < n; i++) {
        step_t s;
        const bstep_t *v = &b->step[i];
        if (before && !memcmp(v, &before[i], sizeof *v)) continue;
        memset(&s, 0, sizeof s);
        s.nt = (int8_t)((int32_t)v->note - 60);
        s.on = bstep_gate(v) == G_NOTE;
        s.tie = bstep_gate(v) == G_TIE;
        s.acc = !!(v->flags & BS_ACCENT);
        s.slide = !!(v->flags & BS_SLIDE);
        changed |= memcmp(&dst[i], &s, sizeof s) != 0;
        dst[i] = s;
    }
    return changed;
}
static int tb_mutate_steps(step_t *dst, const track_t *t, tb_state_t *g, uint32_t *rng)
{
    bpart_t b;
    bstep_t before[NSTEP];
    tb_read_part(&b, dst, t, g);
    memcpy(before, b.step, sizeof before);
    tb3po_mutate(&b, rng);
    g->stale = 0;
    return tb_write_part(dst, &b, before);
}

static uint32_t tb_cfg_value(const tb3po_cfg_t *g, uint32_t id)
{
    switch (id) {
    case 0:return g->density;case 1:return g->accent;case 2:return g->slide;case 3:return g->oct_range;
    case 4:return g->root;case 5:return g->scale;case 6:return g->base_oct;default:return g->mutate_bars;
    }
}
