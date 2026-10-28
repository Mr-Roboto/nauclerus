/* SPDX-License-Identifier: GPL-3.0-only */
/* Actual production sequencer, with the sole activation interface compiled in. */
#define DRUM_REMIX_TEST 1
#include "drum_host.h"
#include <assert.h>

static const unsigned LENGTHS[] = {1, 8, 16, 24, 40, 64};
static uint32_t hash_word(uint32_t h, uint32_t v) { return (h ^ v) * 16777619u; }
static uint32_t trace(uint32_t h, const track_t *t)
{
    const uint32_t values[] = {t->seq_idx, t->seq_cnt, t->seq_pos, t->seq_rem, t->seq_par,
        t->rng, t->rat_n, t->rat_k, t->rat_len, t->rat_vel, (uint8_t)t->rat_nt,
        t->gate_n, t->slide_in, (uint8_t)t->nt, dvage, t->v[0].age, t->v[0].vel,
        (uint32_t)t->v[0].x[1], (uint32_t)t->v[0].x[2], (uint32_t)t->v[0].env[1]};
    for (unsigned i = 0; i < sizeof values / sizeof values[0]; i++) h = hash_word(h, values[i]);
    return h;
}
static uint32_t duration(const track_t *t)
{
    return step_samples(t, div_period((uint32_t)t->p[P_SDIV], t->seq_rem), t->seq_cnt);
}
static void next(track_t *t) { seq_tick(t, duration(t) - t->seq_pos); }
static track_t *fresh(unsigned len, int acid)
{
    host_init();
    track_t *t = &trk[0];
    old_set_model(t, acid ? DM_ACID : DM_KTRX);
    memset(t->step, 0, sizeof t->step);
    t->p[P_SLEN] = (int16_t)len;
    t->p[P_SDIV] = 2;
    t->p[P_SRC] = 0;
    seq_start();
    return t;
}

/* Compare active mapping with a physically rearranged stored pattern played OFF.
 * Both use the production sequencer; no alternate gate/timing/probability engine. */
static uint32_t fixture(unsigned len, int acid, unsigned bank, unsigned pat, int on, int rearrange, unsigned div)
{
    step_t stored[NSTEP], before[NSTEP];
    track_t *t = fresh(len, acid);
    uint32_t h = 2166136261u;
    for (unsigned s = 0; s < NSTEP; s++) {
        memset(&stored[s], 0, sizeof stored[s]);
        stored[s].on = s % 5 != 2;
        stored[s].nt = (int8_t)((int)(s % 24) - 12);
        stored[s].acc = s % 2;
        stored[s].rat = s / 3 % 4;
        stored[s].slide = acid && s % 3 == 0;
        stored[s].tie = acid && s % 7 == 3;
        stored[s].cond = s % 4 == 0 ? 22 : s % 4 == 1 ? COND_1SHOT : s % 4 == 2 ? 11 : 0;
    }
    for (unsigned s = 0; s < NSTEP; s++)
        t->step[s] = stored[rearrange ? remix_source_step(bank, pat, s) : (int)s];
    memcpy(before, t->step, sizeof before);
    t->p[P_SDIV] = (int16_t)div;
    t->p[P_SSWING] = 37;
    song.g[G_SWING] = 23;
    assert(seq_remix_test_set(0, bank, pat, (unsigned)on));
    seq_tick(t, 0);
    for (unsigned c = 0; c < len * 3; c++) {
        assert(t->seq_idx == c % len && t->seq_cnt == c);
        assert(seq_next(t) == &t->step[on ? remix_source_step(bank, pat, (c + 1) % len) : (int)((c + 1) % len)]);
        h = trace(h, t);
        uint32_t cur = duration(t), at = 0;
        for (unsigned j = 1; j <= 8; j++) {
            uint32_t to = cur * j / 8;
            seq_tick(t, to - at);
            h = trace(h, t);
            at = to;
        }
    }
    assert(!memcmp(before, t->step, sizeof before));
    return h;
}
static void test_materialized(void)
{
    static const unsigned selections[][2] = {{0, 1}, {0, 2}, {1, 0}, {8, 9}, {15, 15}};
    for (unsigned l = 0; l < sizeof LENGTHS / sizeof LENGTHS[0]; l++)
        for (int acid = 0; acid <= 1; acid++) {
            uint32_t off = fixture(LENGTHS[l], acid, 0, 0, 0, 0, l);
            assert(off == fixture(LENGTHS[l], acid, 0, 0, 1, 0, l));
            assert(off == fixture(LENGTHS[l], acid, 15, 15, 0, 0, l));
            for (unsigned s = 0; s < sizeof selections / sizeof selections[0]; s++) {
                unsigned bank = selections[s][0], pat = selections[s][1], div = (l + s) % 10;
                assert(fixture(LENGTHS[l], acid, bank, pat, 1, 0, div) ==
                       fixture(LENGTHS[l], acid, bank, pat, 0, 1, div));
            }
        }
    puts("remix seq: OFF/A1 and mapped drums/ACID equal materialized events; six lengths, partial blocks, swing, probability and ratchets passed");
}
static void test_independence(void)
{
    host_init();
    for (unsigned k = 0; k < NTRK; k++) {
        old_set_model(&trk[k], k==2 || k==6 ? DM_ACID : DM_KTRX);
        trk[k].p[P_SLEN] = 64;
        trk[k].p[P_SRC] = 0;
        for (unsigned s = 0; s < NSTEP; s++) {
            memset(&trk[k].step[s], 0, sizeof(step_t));
            trk[k].step[s].on = 1;
            trk[k].step[s].nt = (int8_t)s;
            trk[k].step[s].acc = s & 1;
        }
    }
    seq_start();
    for (unsigned k = 0; k < NTRK; k++) assert(seq_remix_test_set(k, k, 15 - k, 1));
    for (unsigned c = 0; c < 128; c++)
        for (unsigned k = 0; k < NTRK; k++) {
            track_t *t = &trk[k];
            if (!c) seq_tick(t, 0); else next(t);
            unsigned source = (unsigned)remix_source_step(k==2 || k==6 ? k : 7, k==2 || k==6 ? 15-k : 8, c % 64);
            assert(t->seq_idx == c % 64 && t->seq_cnt == c);
            assert(t->nt == (int)source && t->rat_vel == (source & 1 ? 127 : 96));
        }
    uint16_t keep[NTRK];
    for (unsigned k = 0; k < NTRK; k++) keep[k] = seq_remix[k];
    assert(!seq_remix_test_set(8, 0, 0, 1));
    assert(!seq_remix_test_set(0, 16, 0, 1));
    assert(!seq_remix_test_set(0, 0, 16, 1));
    assert(!seq_remix_test_set(0, 0, 0, 2));
    assert(seq_remix_test_set(3, 0, 2, 1));
    for (unsigned k = 0; k < NTRK; k++) if (k==2 || k==6) assert(seq_remix[k] == keep[k]); else assert(seq_remix[k]==258u);
    for (unsigned k = 0; k < NTRK; k++) {
        host_init();
        track_t *t = &trk[k];
        old_set_model(t, DM_ACID);
        t->p[P_SLEN] = 8; t->p[P_SRC] = 0;
        memset(t->step, 0, sizeof t->step);
        t->step[0].on = 1; t->step[0].nt = (int8_t)(-12 + (int)k);
        t->step[5].tie = 1; t->step[5].slide = 1;
        t->step[2].on = 1; t->step[2].nt = 7;
        seq_start();
        assert(seq_remix_test_set(k, 0, 2, 1));
        seq_tick(t, 0);
        uint32_t age = t->v[0].age;
        assert(t->nt == -12 + (int)k && t->gate_n == GATE_HELD);
        next(t); next(t);
        assert(t->nt == 7 && t->v[0].age == age);
    }
    puts("remix seq: shared Drum and independent Bass selections on eight tracks, complete drum events, ACID on every track, invalid activation rejected");
}
static void test_acid(void)
{
    track_t *t = fresh(8, 1);                 /* A3: 0,5,2,7,0,5,2,7 */
    t->step[0].on = 1; t->step[0].nt = -12;
    t->step[1].on = 1; t->step[1].nt = 20;    /* deliberately NOT the next audible event */
    t->step[5].tie = 1; t->step[5].slide = 1; t->step[5].nt = 12;
    t->step[2].on = 1; t->step[2].nt = 7; t->step[2].acc = 1;
    assert(seq_remix_test_set(0, 0, 2, 1));
    seq_tick(t, 0);
    uint32_t age = t->v[0].age;
    assert(t->nt == -12 && t->gate_n == GATE_HELD && seq_next(t) == &t->step[5]);
    next(t);
    assert(t->v[0].age == age && t->nt == -12 && t->slide_in && t->gate_n == GATE_HELD);
    next(t);
    assert(t->v[0].age == age && t->nt == 7 && t->v[0].vel == 127 && !t->slide_in);
    next(t);
    assert(!t->gate_n && !(t->v[0].x[2] & 1));
    next(t);
    assert(t->nt == -12 && t->v[0].age > age); /* repeated source retriggers after rest */
    t = fresh(8, 1);
    t->step[0].on = 1; t->step[0].rat = 3; t->step[5].tie = 1;
    assert(seq_remix_test_set(0, 0, 2, 1));
    seq_tick(t, 0);
    assert(t->rat_n == 4 && t->gate_n != GATE_HELD);
    seq_tick(t, duration(t) * 3 / 4);
    assert(t->rat_k == 4 && t->gate_n == GATE_HELD);
    puts("remix seq: ACID mapped lookahead, tie pitch, previous-slide glide, accent, rest, repeat and final-ratchet gate passed");
}
static void test_ratchets_probability(void)
{
    track_t *t = fresh(8, 0);
    t->p[P_SSWING] = 60;
    t->step[0].on = 1; t->step[0].acc = 1; t->step[0].rat = 3; t->step[0].nt = 12;
    t->step[1].on = 1; t->step[1].nt = -5;
    assert(seq_remix_test_set(0, 1, 0, 1));    /* B1 repeats source 0 on chronological odd slot 1 */
    seq_tick(t, 0);
    uint32_t first = duration(t);
    next(t);
    uint32_t cur = duration(t), age = dvage;
    assert(t->seq_idx == 1 && cur < first && seq_source_step(t, 1) == 0);
    assert(t->rat_n == 4 && t->rat_len == cur && t->rat_vel == 127 && t->rat_nt == 12);
    seq_tick(t, cur / 4 - 1); assert(dvage == age);
    seq_tick(t, 1); assert(dvage == age + 1);
    seq_tick(t, cur / 2 - cur / 4); assert(dvage == age + 2);
    seq_tick(t, cur * 3 / 4 - cur / 2); assert(dvage == age + 3 && t->seq_idx == 1);
    t = fresh(8, 0);
    t->step[0].on = 1; t->step[0].cond = 22; /* 1/2: source 0 is visited twice per loop */
    assert(seq_remix_test_set(0, 0, 1, 1));
    for (unsigned c = 0; c < 32; c++) {
        age = dvage;
        if (!c) seq_tick(t, 0); else next(t);
        assert(dvage - age == ((c % 8 == 0 || c % 8 == 4) && (c / 8) % 2 == 0 ? 1u : 0u));
    }
    puts("remix seq: mapped ratchet count/accent/note at chronological quarter-step times; 1/2 conditions use chronological loops");
}
static void test_drum_tie_rest(void)
{
    track_t *t = fresh(8, 0);
    t->step[0].on = 1;
    t->step[1].on = 1; t->step[1].nt = -5;
    t->step[5].on = 1; t->step[5].tie = 1;
    t->step[7].on = 1; t->step[7].nt = 10;
    assert(seq_remix_test_set(0, 0, 2, 1));
    seq_tick(t, 0);
    uint32_t age = dvage;
    next(t); assert(dvage == age && !t->rat_n); /* mapped drum TIE is silent */
    next(t); assert(dvage == age && !t->rat_n); /* mapped rest is silent */
    next(t); assert(dvage == age + 1 && t->nt == 10);
    next(t); assert(dvage == age + 2 && t->nt == 0);
    puts("remix seq: complete drum flags include silent TIE/rest and repeated hits");
}
static uint32_t grids_trace(int on, int acid)
{
    track_t *t = fresh(16, acid);
    t->p[P_SRC] = 1; song.g[G_GFILL1] = 127;
    assert(seq_remix_test_set(0, 0, 2, (unsigned)on));
    uint32_t h = 2166136261u;
    for (unsigned c = 0; c < 64; c++) {
        assert(seq_source_step(t, c) == c);
        grids_tick(c ? div_samples(3) : 0);
        seq_tick(t, c ? div_samples(3) : 0);
        h = trace(h, t);
        assert(!t->rat_n && t->nt == 0);
    }
    assert(dvage > 0);
    return h;
}
static void test_bypass_and_reset(void)
{
    for (int acid = 0; acid <= 1; acid++) assert(grids_trace(0, acid) == grids_trace(1, acid));
    static step_t slots[2][NTRK][NSTEP];
    track_t *t = fresh(16, 0);
    memset(slots, 0, sizeof slots);
    for (unsigned s = 0; s < 2; s++)
        for (unsigned k = 0; k < NTRK; k++) {
            chain.src[s].step[k] = slots[s][k];
            chain.src[s].timing[k][0] = 16; chain.src[s].timing[k][1] = 2;
            for (unsigned c = 0; c < NSTEP; c++) {
                slots[s][k][c].on = 1; slots[s][k][c].nt = (int8_t)(c % 16 + s * 16);
            }
        }
    chain.cfg.count = 2;
    chain.cfg.row[0] = (chain_row_t){0, 1}; chain.cfg.row[1] = (chain_row_t){1, 1};
    chain.armed = 1;
    assert(seq_remix_test_set(0, 0, 2, 1));
    assert(seq_source_step(t, 1) == 1); /* armed as well as running bypasses */
    seq_start();
    assert(chain.running);
    assert(seq_remix_test_set(0, 0, 2, 1));
    for (unsigned row = 0; row < 2; row++) {
        if (row) { chain.row = 1; chain_apply(0); }
        for (unsigned c = 0; c < 16; c++) {
            if (!c) seq_tick(t, 0); else next(t);
            assert(seq_source_step(t, c) == c && t->nt == (int)(c + row * 16));
        }
    }
    seq_stop();
    for (unsigned k = 0; k < NTRK; k++) assert(!seq_remix[k]);
    assert(seq_remix_config[REMIX_DRUM] == 258);               /* runtime clears; project configuration survives */
    assert(seq_remix_test_set(0, 15, 15, 1));
    seq_start();
    for (unsigned k = 0; k < NTRK; k++) assert(seq_remix[k] == 511u);
    seq_tick(t, 0);
    assert(t->seq_idx == 0 && t->seq_cnt == 0);
    assert(seq_remix_test_set(7, 15, 15, 1));
    seq_start();
    for (unsigned k = 0; k < NTRK; k++) assert(seq_remix[k] == 511u);
    host_init();
    for (unsigned k = 0; k < NTRK; k++) assert(!seq_remix[k] && !seq_remix_config[k]);
    puts("remix seq: Grids bypass/song OFF slots; defaults OFF; STOP clears runtime; PLAY/restart retain project configuration");
}
static void test_chronological_writes(void)
{
    track_t *t = fresh(8, 0);
    t->step[0].on = t->step[4].on = 1;
    assert(seq_remix_test_set(0, 0, 1, 1));
    motion_add(0, 0, P_LEVEL, 33);
    lock_set(0, 0, P_PAN, 12);
    seq_tick(t, 0);
    for (unsigned i = 0; i < 4; i++) next(t);
    assert(t->seq_idx == 4 && seq_source_step(t, 4) == 0);
    assert(t->p[P_LEVEL] == 33 && t->p[P_PAN] == 12);
    t->step[4].on = 0;
    assert(rec_hit(t, 127) == 4 && t->step[4].on); /* recording writes chronological grid */
    puts("remix seq: Motion/lock playback follows source zero; live recording still writes chronological step four");
}
int main(void)
{
    test_materialized();
    test_independence();
    test_acid();
    test_ratchets_probability();
    test_drum_tie_rest();
    test_bypass_and_reset();
    test_chronological_writes();
    puts("remix sequencer: all focused checks passed");
    return 0;
}
