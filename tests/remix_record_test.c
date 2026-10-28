/* SPDX-License-Identifier: GPL-3.0-only */
/* Recording/editing coordinates remain original; only subsequent playback maps. */
#define DRUM_REMIX_TEST 1
#include "ui_host.h"
#include <assert.h>

static const unsigned LENGTHS[] = {1, 8, 16, 24, 40, 64};
typedef struct { step_t step[NTRK][NSTEP]; lk_bank_t bank; } recorded_t;
static uint32_t duration(track_t *t)
{
    return step_samples(t, div_period((uint32_t)t->p[P_SDIV], t->seq_rem), t->seq_cnt);
}
static void next(track_t *t) { seq_tick(t, duration(t) - t->seq_pos); }
static void cursor(track_t *t, unsigned c, int late)
{
    t->seq_idx = (uint16_t)c; t->seq_cnt = c; t->seq_rem = 0;
    t->seq_pos = late < 0 ? 0 : duration(t) / 2 + (unsigned)late;
}
static track_t *fresh(unsigned len, unsigned k, int acid)
{
    ui_host_init(); settings.accel = 0;
    for (unsigned j = 0; j < NTRK; j++) memset(trk[j].step, 0, sizeof trk[j].step);
    track_t *t = &trk[k]; old_set_model(t, acid ? DM_ACID : DM_K909);
    t->p[P_SLEN] = (int16_t)len; t->p[P_SDIV] = 2; t->p[P_SRC] = 0;
    t->p[P_SSWING] = 37; song.g[G_SWING] = 23;
    t->p[P_E1] = 64; t->p[P_PAN] = -7; t->p[P_LEVEL] = 90;
    song.sel = (uint8_t)k; song.rec = (uint8_t)(1u << k);
    seq_start();
    return t;
}
static void save(recorded_t *out)
{
    for (unsigned k = 0; k < NTRK; k++) memcpy(out->step[k], trk[k].step, sizeof out->step[k]);
    out->bank = mo.b;
}
static void hold(unsigned s)
{
    ui.bank = (uint8_t)(s / 16); ui.held = (uint8_t)(s % 16);
    memset(ui.step_t0, 0, sizeof ui.step_t0);
    ui.step_t0[ui.held] = 1; fm1_in.notes = 1u << STEP_KEY[ui.held];
    step_grab(ui.held);
    assert(ui.step_si[ui.held] == s);
}

/* Same original-data session under OFF, A1, repeats and non-identity maps. */
static void record_session(recorded_t *out, unsigned len, unsigned k, int acid,
                           unsigned bank, unsigned pat, unsigned on)
{
    track_t *t = fresh(len, k, acid);
    assert(seq_remix_test_set(k, bank, pat, on));
    unsigned c = len > 1 ? len - 2 : 0;
    cursor(t, c, 0); input_hit(t, 127);
    assert(t->step[c].on && t->step[c].acc && !t->step[c].cond && !t->step[c].rat);
    cursor(t, len - 1, 1); input_hit(t, 96);
    assert(t->step[0].on && t->rskip && t->rskip_idx == 0);

    ui.keys = 1; assert(keys_mode()); song.octave = 1; c = len / 2;
    cursor(t, c, -1); keys_on(7);  /* NOTE +12 at the chronological cursor */
    assert(t->step[c].on && t->step[c].nt == 12 && !t->step[c].tie);
    if (acid) {
        unsigned nx = (c + 1) % len;
        cursor(t, nx, 1); keys_tie();
        if (len > 1) assert(t->step[nx].tie && !t->step[nx].on && t->step[nx].nt == 12);
    }
    unsigned dest = (c + 2) % len;
    unsigned previous = kz.last; int writing = kz.rec;
    cursor(t, dest, -1); keys_on(11);
    assert(t->step[dest].on && t->step[dest].nt == 16 && !t->step[dest].tie);
    if (acid && writing && previous != dest) assert(t->step[previous].slide);
    keys_off();
    ui.keys = 0;
    if (acid) assert(!t->gate_n && !(t->v[0].x[2] & 1));

    c = len > 2 ? 2 : 0;
    cursor(t, c, 0); t->p[P_E1] = 37; motion_capture(t, P_E1);
    int at = lk_find(&mo.b, lk_key(k, c, P_E1), 0);
    assert(at >= 0 && mo.b.val[at] == 37);
    cursor(t, len - 1, 1); t->p[P_E1] = 44; motion_capture(t, P_E1);
    at = lk_find(&mo.b, lk_key(k, 0, P_E1), 0);
    assert(at >= 0 && mo.b.val[at] == 44);

    ui.home = 1; ui.env = 1; hold(len > 4 ? 4 : len - 1);
    unsigned st = ui.step_si[ui.held];
    assert(lock_step() == st && lock_id(1) == P_E1);
    unsigned count = mo.b.count;
    lock_knob(1, 1);
    at = lk_find(&mo.b, lk_key(k, st, P_E1), 1);
    assert(at >= 0 && mo.b.val[at] == 65 && mo.b.count == count + 1);
    lock_knob(1, 1);
    assert(mo.b.val[at] == 66 && mo.b.count == count + 1); /* edit replaces, never duplicates */

    ui.env = 0; ui.home = 0; ui.page = (uint8_t)page_first(FAM_SEQ);
    assert(grid_mode()); hold((len - 1) / 2);
    unsigned edited = ui.step_si[ui.held];
    assert(step_edit(1, 1) && t->step[edited].rat == 1);
    step_accent(ui.held);
    seq_stop();
    assert(t->p[P_E1] == 64 && !lock_held(k, P_E1));

    /* Step Record is stopped-only; a test selection must not affect its cursor. */
    assert(seq_remix_test_set(k, bank, pat, on));
    ui.keys = 1; ui.home = 0; ui.page = (uint8_t)page_first(FAM_SEQ); ui.bank = 0;
    fm1_in.notes = 0;
    assert(srec_ok()); srec_start();
    ui.sr_cur = (uint8_t)(len - 1); srec_note(7); srec_note(11);
    assert(t->step[len - 1].slide && t->step[0].on && t->step[0].nt == 16);
    srec_next(); unsigned edit_cursor = ui.sr_cur;
    int prior_note = t->step[edit_cursor].nt;
    srec_knob(1, 1); assert(t->step[edit_cursor].nt == prior_note + 1);
    srec_rest(); assert(!t->step[edit_cursor].on && !t->step[edit_cursor].tie && !t->step[edit_cursor].rat);
    ui.srec = 0;
    save(out);
    for (unsigned j = 0; j < NTRK; j++) if (j != k)
        for (unsigned s = 0; s < NSTEP; s++) {
            step_t zero; memset(&zero, 0, sizeof zero); assert(!memcmp(&out->step[j][s], &zero, sizeof zero));
        }
    for (unsigned i = 0; i < out->bank.count; i++) assert((out->bank.key[i] >> 13) == k);
    lk_bank_t sealed = out->bank; lk_seal(&sealed, 0); assert(lk_valid(&sealed, 0));
}
static uint32_t fold(uint32_t h, uint32_t v) { return (h ^ v) * 16777619u; }
/* Replay recorded data through production playback, or its materialized source view. */
static uint32_t replay(const recorded_t *data, unsigned len, unsigned k, int acid,
                       unsigned bank, unsigned pat, unsigned on, int materialize)
{
    track_t *t = fresh(len, k, acid); song.rec = 0;
    memset(&mo.b, 0, sizeof mo.b); mo.b.on = data->bank.on;
    for (unsigned c = 0; c < NSTEP; c++) {
        unsigned s = materialize ? (unsigned)remix_source_step(bank, pat, c) : c;
        t->step[c] = data->step[k][s];
        unsigned at = k << 6 | s;
        for (unsigned i = lk_seek(&data->bank, k, s);
             i < data->bank.count && (data->bank.key[i] >> 7) == at; i++)
            assert(lk_put(&mo.b, lk_key(k, c, data->bank.key[i] & 127), data->bank.val[i], lk_is_lock(&data->bank, i)) >= 0);
    }
    recorded_t before; save(&before);
    assert(seq_remix_test_set(k, bank, pat, on)); seq_tick(t, 0);
    uint32_t h = 2166136261u;
    for (unsigned c = 0; c < len * 2; c++) {
        assert(t->seq_idx == c % len);
        h = fold(h, t->nt); h = fold(h, t->v[0].age); h = fold(h, t->rat_n);
        h = fold(h, t->rat_vel); h = fold(h, t->gate_n); h = fold(h, t->slide_in);
        h = fold(h, t->p[P_E1]); h = fold(h, mo.under[k][P_E1]); h = fold(h, t->rng);
        next(t);
    }
    recorded_t after; save(&after); assert(!memcmp(&before, &after, sizeof before));
    seq_stop(); assert(t->p[P_E1] == 64 && !lock_held(k, P_E1));
    return h;
}
static void test_matrix(void)
{
    static const unsigned maps[][2] = {{0, 0}, {0, 1}, {0, 2}, {0, 4}, {1, 0}, {15, 15}};
    recorded_t off, mapped;
    for (unsigned l = 0; l < 6; l++) for (unsigned k = 0; k < NTRK; k++) for (int acid = 0; acid <= 1; acid++) {
        unsigned len = LENGTHS[l];
        record_session(&off, len, k, acid, 0, 0, 0);
        for (unsigned m = 0; m < sizeof maps / sizeof maps[0]; m++) {
            unsigned bank = maps[m][0], pat = maps[m][1];
            record_session(&mapped, len, k, acid, bank, pat, 1);
            assert(!memcmp(&off, &mapped, sizeof off));
            assert(replay(&mapped, len, k, acid, bank, pat, 1, 0) == replay(&mapped, len, k, acid, bank, pat, 0, 1));
        }
        assert(replay(&off, len, k, acid, 0, 0, 0, 0) == replay(&off, len, k, acid, 0, 0, 1, 0));
    }
    puts("remix recording: 672 OFF/A1/active sessions, six lengths, eight tracks, drums/KEYS/ACID/Motion/lock edits/Step Record write identical originals");
    puts("remix recording: 576 recorded-pattern mapped/materialized playback pairs pass; playback never mutates recorded data or bank");
}
static void test_nearest_and_skip(void)
{
    for (unsigned k = 0; k < NTRK; k++) {
        track_t *t = fresh(8, k, 0);
        assert(seq_remix_test_set(k, 0, 4, 1)); /* A5 slot 1 -> source 9, beyond LEN */
        step_t source = t->step[9];
        for (unsigned parity = 0; parity < 2; parity++) {
            memset(t->step, 0, sizeof t->step);
            cursor(t, parity, 0);
            unsigned half = t->seq_pos;
            assert(rec_hit(t, 96) == parity && t->step[parity].on);
            t->step[parity].on = 0; t->seq_pos = half + 1;
            assert(rec_hit(t, 127) == parity + 1 && !t->step[parity].on);
            assert(t->rskip_idx == parity + 1);
        }
        memset(t->step, 0, sizeof t->step);
        t->step[9].on = 1; t->step[9].nt = 9;
        t->step[7].on = 1; t->step[7].nt = 7;
        source = t->step[9];
        seq_start(); assert(seq_remix_test_set(k, 0, 4, 1)); seq_tick(t, 0);
        t->seq_pos = duration(t) / 2 + 1;
        input_hit(t, 127); unsigned age = dvage;
        assert(t->step[1].on && t->rskip_idx == 1 && !memcmp(&source, &t->step[9], sizeof source));
        recorded_t written; save(&written);
        next(t); assert(t->seq_idx == 1 && !t->rskip && dvage == age); /* one chronological suppression */
        for (unsigned c = 0; c < 16; c++) next(t);
        recorded_t after; save(&after); assert(!memcmp(&written, &after, sizeof written));
        seq_stop(); seq_start(); assert(!t->rskip && seq_remix_config[seq_remix_scope(k)] == 260u && seq_remix[k] == 260u);
    }
    puts("remix recording: swung midpoint boundaries, chronological one-shot suppression, beyond-LEN audible sources, repeated playback and restart passed");
}
static void test_acid_phrase_guards(void)
{
    for (unsigned k = 0; k < NTRK; k++) {
        track_t *t = fresh(8, k, 1); assert(seq_remix_test_set(k, 0, 4, 1));
        cursor(t, 7, -1); keys_on(7);
        assert(t->step[7].on && t->step[7].nt == 0 && !t->gate_n);
        cursor(t, 0, -1); unsigned age = t->v[0].age; keys_on(11);
        assert(t->step[7].slide && t->step[0].on && t->step[0].nt == 4 && t->v[0].age == age);
        keys_off(); assert(!(t->v[0].x[2] & 1));
        t = fresh(8, k, 1); assert(seq_remix_test_set(k, 1, 0, 1));
        cursor(t, 6, -1); keys_on(7); step_t start = t->step[6];
        for (unsigned n = 1; n < 8; n++) {
            unsigned c = (6 + n) % 8; cursor(t, c, 1); keys_tie();
            assert(t->step[c].tie && !t->step[c].on);
        }
        cursor(t, 6, 1); keys_tie();
        assert(!kz.rec && !memcmp(&start, &t->step[6], sizeof start));
        t = fresh(8, k, 1); assert(seq_remix_test_set(k, 0, 4, 1));
        cursor(t, 6, -1); keys_on(7);
        for (unsigned c = 0; c < 4; c++) { t->step[c].on = 1; t->step[c].nt = (int8_t)c; }
        step_t keep[4]; memcpy(keep, t->step, sizeof keep);
        t->p[P_SLEN] = 4; cursor(t, 0, 1); keys_tie();
        assert(!kz.rec && !memcmp(keep, t->step, sizeof keep));
        t->p[P_SLEN] = 16; keys_off(); seq_stop();
        t = fresh(4, k, 1); assert(seq_remix_test_set(k, 0, 1, 1));
        cursor(t, 3, -1); keys_on(7); start = t->step[3];
        t->p[P_SLEN] = 8; cursor(t, 4, 1); keys_tie();
        assert(kz.rec && t->step[4].tie && !t->step[4].on && !t->step[0].on && !t->step[0].tie);
        assert(!memcmp(&start, &t->step[3], sizeof start));
        keys_off(); seq_stop();
        t = fresh(1, k, 1); assert(seq_remix_test_set(k, 15, 15, 1));
        cursor(t, 0, -1); keys_on(7); start = t->step[0];
        cursor(t, 0, 1); keys_tie();
        assert(!kz.rec && !memcmp(&start, &t->step[0], sizeof start));
        keys_off(); seq_stop();
    }
    puts("remix ACID recording: chronological NOTE/TIE/SLIDE, legato across wrap, held phrase-start protection and safe LEN cut/growth passed");
}
static void test_all_tracks_armed(void)
{
    fresh(8, 0, 0);
    for (unsigned k = 0; k < NTRK; k++) {
        trk[k].p[P_SLEN] = (int16_t)LENGTHS[k % 6]; trk[k].p[P_SRC] = 0; trk[k].p[P_E1] = 64;
    }
    seq_start(); song.rec = 0xFFu;
    for (unsigned k = 0; k < NTRK; k++) assert(seq_remix_test_set(k, k, 15 - k, 1));
    for (unsigned k = 0; k < NTRK; k++) {
        recorded_t before; save(&before);
        unsigned c = k % LENGTHS[k % 6]; cursor(&trk[k], c, -1); input_hit(&trk[k], 127);
        assert(trk[k].step[c].on && trk[k].step[c].acc);
        for (unsigned j = 0; j < NTRK; j++) if (j != k)
            assert(!memcmp(before.step[j], trk[j].step, sizeof before.step[j]));
        song.sel = (uint8_t)k; trk[k].p[P_E1] = (int16_t)(40 + k); motion_capture(&trk[k], P_E1);
        int at = lk_find(&mo.b, lk_key(k, c, P_E1), 0);
        assert(at >= 0 && mo.b.val[at] == (int)(40 + k));
    }
    assert(mo.b.count == NTRK);
    seq_stop();
    for (unsigned k = 0; k < NTRK; k++) assert(trk[k].p[P_E1] == 64);
    puts("remix recording: all eight tracks armed with independent selections; drum and Motion writes remain isolated");
}
static void test_bypasses(void)
{
    track_t *t = fresh(8, 0, 0); t->p[P_SRC] = 1;
    assert(seq_remix_test_set(0, 0, 2, 1)); cursor(t, 1, -1);
    step_t before[NSTEP]; memcpy(before, t->step, sizeof before);
    input_hit(t, 127); keys_on(7); keys_off();
    assert(!memcmp(before, t->step, sizeof before));
    t->p[P_E1] = 40; motion_capture(t, P_E1);
    assert(lk_find(&mo.b, lk_key(0, 1, P_E1), 0) >= 0 && lk_find(&mo.b, lk_key(0, 5, P_E1), 0) < 0);
    assert(seq_source_step(t, 1) == 1);
    ui.env = 1; ui.held = 1; ui.step_t0[1] = 1; fm1_in.notes = 1u << STEP_KEY[1];
    step_grab(1); assert(ui.step_si[1] >= NSTEP); /* Grids rejects a held editing destination */
    lock_knob(1, 1); assert(mo.b.count == 1);
    ui.keys = 1; ui.home = 0; ui.page = (uint8_t)page_first(FAM_SEQ);
    assert(!srec_ok());
    seq_start(); assert(seq_remix_test_set(0, 0, 2, 1)); seq_tick(t, 0); next(t);
    assert(t->p[P_E1] == 40 && seq_source_step(t, 1) == 1);

    static step_t row[NTRK][NSTEP]; static lk_bank_t row_bank;
    t = fresh(8, 0, 0); memset(row, 0, sizeof row); memset(&row_bank, 0, sizeof row_bank);
    motion_add(0, 5, P_E1, 99);
    chain.src[0].m = &row_bank;
    for (unsigned k = 0; k < NTRK; k++) {
        chain.src[0].step[k] = row[k]; chain.src[0].model[k] = (uint8_t)trk[k].p[P_MODEL];
        chain.src[0].timing[k][0] = 8; chain.src[0].timing[k][1] = 2;
        row[k][1].on = row[k][5].on = 1; row[k][1].nt = 7; row[k][5].nt = 20;
    }
    chain.cfg.count = 1; chain.cfg.row[0] = (chain_row_t){0, 1}; chain.armed = 1;
    seq_start(); assert(chain.running && !song.rec);
    assert(seq_remix_test_set(0, 0, 2, 1)); cursor(t, 1, -1);
    recorded_t saved; save(&saved);
    input_hit(t, 127); keys_on(7); keys_off(); t->p[P_E1] = 30; motion_capture(t, P_E1);
    recorded_t after; save(&after); assert(!memcmp(&saved, &after, sizeof saved));
    ui.env = 1; ui.held = 1; ui.step_t0[1] = 1; fm1_in.notes = 1u << STEP_KEY[1];
    step_grab(1); assert(ui.step_si[1] >= NSTEP); lock_knob(1, 1);
    ui.keys = 1; ui.home = 0; ui.page = (uint8_t)page_first(FAM_SEQ); assert(!srec_ok());
    save(&after); assert(!memcmp(&saved, &after, sizeof saved));
    seq_start(); assert(seq_remix_test_set(0, 0, 2, 1)); seq_tick(t, 0); next(t);
    assert(t->nt == 7 && seq_source_step(t, 1) == 1);
    seq_stop();
    puts("remix recording: Grids recording/editing gates and chronological Motion, song REC disarm/edit gates and row-owned OFF playback passed");
}
int main(void)
{
    test_matrix(); test_nearest_and_skip(); test_acid_phrase_guards(); test_all_tracks_armed(); test_bypasses();
    puts("remix recording/editing: all focused production-code checks passed");
    return 0;
}
