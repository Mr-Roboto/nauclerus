/* SPDX-License-Identifier: GPL-3.0-only */
#define DRUM_REMIX_TEST 1
#include "drum_host.h"
#include <assert.h>

static const unsigned LENGTHS[] = {1, 8, 16, 24, 40, 64};
static uint32_t duration(track_t *t)
{
    return step_samples(t, div_period((uint32_t)t->p[P_SDIV], t->seq_rem), t->seq_cnt);
}
static void next(track_t *t) { seq_tick(t, duration(t) - t->seq_pos); }
static track_t *fresh(unsigned len, int acid)
{
    host_init();
    track_t *t = &trk[0];
    old_set_model(t, acid ? DM_ACID : DM_KTRX);
    t->p[P_SLEN] = (int16_t)len; t->p[P_SRC] = 0; t->p[P_SDIV] = 2;
    t->p[P_LEVEL] = 90; t->p[P_PAN] = -7; t->p[P_E1] = 64;
    memset(t->step, 0, sizeof t->step);
    for (unsigned s = 0; s < NSTEP; s++) t->step[s].on = 1;
    seq_start();
    return t;
}
static void test_chronological_reset(void)
{
    track_t *t = fresh(8, 0);              /* A2: 0,1,2,3,0,1,2,3 */
    motion_add(0, 3, P_LEVEL, 51);
    assert(seq_remix_test_set(0, 0, 1, 1));
    seq_tick(t, 0);
    assert(t->p[P_LEVEL] == 90);
    for (unsigned c = 1; c <= 8; c++) {
        next(t);
        assert(t->seq_idx == c % 8);
        assert(t->p[P_LEVEL] == (c < 3 || c == 8 ? 90 : 51));
        if (c == 4) assert(seq_source_step(t, c) == 0 && motion_moved(0, P_LEVEL));
    }
    next(t); next(t); next(t);
    assert(t->p[P_LEVEL] == 51);
    seq_tick(t, duration(t) / 3);
    assert(t->p[P_LEVEL] == 51);           /* held Motion, no interpolation */
    seq_stop();
    assert(t->p[P_LEVEL] == 90 && !mo.base_ok);
    puts("remix motion: source zero at nonzero chronology holds Motion; chronological wrap/STOP restore base; no interpolation");
}
static void test_eligibility_and_precedence(void)
{
    for (unsigned on = 0; on <= 1; on++) {
        track_t *t = fresh(8, 0);          /* A5: chronological 1 selects source 9, outside LEN */
        t->step[1].on = !on; t->step[9].on = on;
        t->step[9].cond = 1;              /* probability 0: lock eligibility remains ON, not fired */
        motion_add(0, 9, P_LEVEL, 44);
        lock_set(0, 9, P_LEVEL, 100); lock_set(0, 9, P_PAN, 12);
        assert(seq_remix_test_set(0, 0, 4, 1));
        seq_tick(t, 0);
        uint32_t age = dvage;
        next(t);
        assert(seq_source_step(t, 1) == 9 && dvage == age);
        assert(t->p[P_LEVEL] == (on ? 100 : 44) && t->p[P_PAN] == (on ? 12 : -7));
        assert(lock_held(0, P_LEVEL) == (int)on);
        if (on) assert(mo.under[0][P_LEVEL] == 44); /* Motion precedes lock on the same knob */
        next(t);
        assert(t->p[P_LEVEL] == 44 && t->p[P_PAN] == -7 && !lock_held(0, P_LEVEL));
        seq_stop(); assert(t->p[P_LEVEL] == 90);
    }
    track_t *t = fresh(8, 0);
    motion_add(0, 9, P_LEVEL, 44); lock_set(0, 9, P_LEVEL, 100);
    motion_set_play(0, 0);
    assert(seq_remix_test_set(0, 0, 4, 1));
    seq_tick(t, 0); next(t);
    assert(t->p[P_LEVEL] == 100 && mo.under[0][P_LEVEL] == 90 && !motion_on(0));
    next(t); assert(t->p[P_LEVEL] == 90);
    puts("remix locks: beyond-LEN source ON eligibility, dormant OFF locks, probability-independent eligibility, Motion-before-Lock, PLAY OFF passed");
}
static void test_repeats_and_transport(void)
{
    track_t *t = fresh(8, 0);
    motion_add(0, 0, P_LEVEL, 40); lock_set(0, 0, P_LEVEL, 110);
    assert(seq_remix_test_set(0, 1, 0, 1)); /* B1: 0,0,3,3,... */
    seq_tick(t, 0);
    assert(t->p[P_LEVEL] == 110 && mo.under[0][P_LEVEL] == 40);
    next(t);
    assert(t->p[P_LEVEL] == 110 && mo.under[0][P_LEVEL] == 40); /* released before repeated lock */
    next(t); assert(t->p[P_LEVEL] == 40 && !lock_held(0, P_LEVEL));
    seq_start();                          /* restart while a Motion value is held */
    assert(t->p[P_LEVEL] == 90 && mo.base[0][P_LEVEL] == 90);
    assert(seq_remix_test_set(0, 1, 0, 1)); seq_tick(t, 0);
    seq_start();                          /* restart while a lock is held */
    assert(t->p[P_LEVEL] == 90 && !lock_held(0, P_LEVEL));
    assert(seq_remix_test_set(0, 1, 0, 1)); seq_tick(t, 0);
    seq_stop(); assert(t->p[P_LEVEL] == 90 && !lock_held(0, P_LEVEL));
    t = fresh(4, 0);
    motion_add(0, 5, P_LEVEL, 43); lock_set(0, 7, P_LEVEL, 101);
    assert(seq_remix_test_set(0, 0, 2, 1)); /* A3: 0,5,2,7 */
    seq_tick(t, 0);
    for (unsigned loop = 0; loop < 3; loop++) {
        next(t); assert(t->p[P_LEVEL] == 43);
        next(t); assert(t->p[P_LEVEL] == 43);
        next(t); assert(t->p[P_LEVEL] == 101 && mo.under[0][P_LEVEL] == 43);
        next(t); assert(t->p[P_LEVEL] == 90 && !lock_held(0, P_LEVEL));
    }
    puts("remix locks: repeats release/reapply without stacking, partial loop wrap, STOP and both restart restorations passed");
}
static void test_eight_tracks(void)
{
    host_init();
    for (unsigned k = 0; k < NTRK; k++) {
        track_t *t = &trk[k]; old_set_model(t, k==2 || k==6 ? DM_ACID : DM_KTRX);
        t->p[P_SLEN] = (int16_t)LENGTHS[k % 6]; t->p[P_SRC] = 0;
        t->p[P_LEVEL] = 90; t->p[P_PAN] = -7;
        for (unsigned s = 0; s < NSTEP; s++) {
            memset(&t->step[s], 0, sizeof(step_t)); t->step[s].on = s % 5 != 1;
            motion_add(k, s, P_LEVEL, (int32_t)s + 20);
            lock_set(k, s, P_PAN, (int32_t)(s % 31) - 15);
        }
    }
    assert(mo.b.count == LK_MAX && !mo.full && !mo.lfull);
    seq_start();
    for (unsigned k = 0; k < NTRK; k++) assert(seq_remix_test_set(k, k, 15 - k, 1));
    for (unsigned c = 0; c < 128; c++)
        for (unsigned k = 0; k < NTRK; k++) {
            track_t *t = &trk[k];
            if (!c) seq_tick(t, 0); else next(t);
            unsigned s = (unsigned)remix_source_step(k==2 || k==6 ? k : 7, k==2 || k==6 ? 15-k : 8, c % LENGTHS[k % 6]);
            assert(t->seq_idx == c % LENGTHS[k % 6] && t->p[P_LEVEL] == (int)s + 20);
            assert(t->p[P_PAN] == (t->step[s].on ? (int)(s % 31) - 15 : -7));
        }
    motion_set_play(3, 0);
    assert(trk[3].p[P_LEVEL] == 90);
    for (unsigned k = 0; k < NTRK; k++) {
        next(&trk[k]);
        if (k == 3) assert(trk[k].p[P_LEVEL] == 90);
        else assert(motion_on(k));
    }
    seq_stop();
    for (unsigned k = 0; k < NTRK; k++) assert(trk[k].p[P_LEVEL] == 90 && trk[k].p[P_PAN] == -7);
    puts("remix motion/locks: global Drum/two independent Bass selections, all six lengths/four blocks, full existing 1024-event bank and independent PLAY passed");
}
static void test_lfo(void)
{
    track_t *t = fresh(8, 0);
    old_set_model(t, DM_K808);             /* E1 range 0..127, as the existing lock/LFO regression */
    t->p[P_E1] = 64;
    seq_start();
    t->p[P_LFO1 + LF_DEST] = 2; t->p[P_LFO1 + LF_DEPTH] = 10;
    t->p[P_LFO1 + LF_TRIG] = LT_FREE; t->p[P_LFO1 + LF_MODE] = LM_HZ;
    t->p[P_LFO1 + LF_RATE] = 127;
    motion_add(0, 5, P_E1, 30); lock_set(0, 5, P_E1, 100);
    assert(seq_remix_test_set(0, 0, 2, 1)); seq_tick(t, 0);
    lfo_apply(duration(t)); next(t);
    assert(*motion_slot(t, P_E1) == 100 && mo.under[0][P_E1] == 30);
    lfo_restore(); assert(t->p[P_E1] == 100);
    int lo = 999, hi = -999; int32_t v;
    for (unsigned b = 0; b < 40; b++) {
        lfo_apply(CTL); seq_tick(t, CTL);
        assert(t->seq_idx == 1 && *motion_slot(t, P_E1) == 100);
        int32_t audible = t->p[P_E1];
        lfo_restore(); assert(t->p[P_E1] == 100);
        assert(lfo_live(t, P_E1, &v) && v == audible); /* UI accessor expects restored knobs */
        if (audible < lo) lo = audible;
        if (audible > hi) hi = audible;
    }
    if (!(lo < 100 && hi > 100 && lo >= 78 && hi <= 122))
        fprintf(stderr, "mapped LFO observed %d..%d, rate %d, depth %d, waveform %d\n", lo, hi,
                t->p[P_LFO1 + LF_RATE], t->p[P_LFO1 + LF_DEPTH], t->p[P_LFO1 + LF_WAVE]);
    assert(lo < 100 && hi > 100 && lo >= 78 && hi <= 122);
    lfo_apply(duration(t) - t->seq_pos); next(t);
    assert(*motion_slot(t, P_E1) == 30 && !lock_held(0, P_E1));
    lfo_restore(); assert(t->p[P_E1] == 30);
    seq_stop(); assert(t->p[P_E1] == 64);
    puts("remix motion/lock/LFO: incoming-step reapplication, modulation around lock, held Motion after release, STOP base passed");
}
static void test_acid(void)
{
    track_t *t = fresh(8, 1);
    memset(t->step, 0, sizeof t->step);
    t->step[0].on = 1; t->step[0].nt = -12;
    t->step[5].tie = 1; t->step[5].slide = 1;
    t->step[2].on = 1; t->step[2].nt = 7;
    motion_add(0, 5, P_E1, 30); lock_set(0, 5, P_E1, 110); lock_set(0, 2, P_E1, 100);
    assert(seq_remix_test_set(0, 0, 2, 1)); seq_tick(t, 0);
    uint32_t age = t->v[0].age;
    assert(t->gate_n == GATE_HELD);
    next(t); assert(t->p[P_E1] == 30 && !lock_held(0, P_E1) && t->v[0].age == age);
    next(t); assert(t->p[P_E1] == 100 && mo.under[0][P_E1] == 30 && t->nt == 7 && t->v[0].age == age);
    next(t); assert(t->p[P_E1] == 30 && !t->gate_n && !lock_held(0, P_E1));
    next(t); assert(t->p[P_E1] == 30); /* nonzero chronological source zero does not reset */
    seq_stop(); assert(t->p[P_E1] == 64);
    puts("remix ACID: mapped Motion on TIE, ON-only lock eligibility, previous-slide glide and rest release passed");
}
static void test_bypasses(void)
{
    track_t *t = fresh(8, 0); t->p[P_SRC] = 1;
    motion_add(0, 1, P_LEVEL, 44); lock_set(0, 1, P_PAN, 12);
    motion_add(0, 5, P_LEVEL, 88); lock_set(0, 5, P_PAN, 24);
    assert(seq_remix_test_set(0, 0, 2, 1)); seq_tick(t, 0); next(t);
    assert(t->p[P_LEVEL] == 44 && t->p[P_PAN] == 12 && !t->rat_n);
    static step_t steps[2][NTRK][NSTEP]; static lk_bank_t banks[2];
    t = fresh(8, 0); memset(steps, 0, sizeof steps); memset(banks, 0, sizeof banks);
    motion_add(0, 1, P_LEVEL, 127); /* working bank must not replace a row's bank */
    for (unsigned s = 0; s < 2; s++) {
        assert(lk_put(&banks[s], lk_key(0, 1, P_E1), 30 + (int)s, 0) >= 0);
        assert(lk_put(&banks[s], lk_key(0, 1, P_E1), 100 + (int)s, 1) >= 0);
        assert(lk_put(&banks[s], lk_key(0, 1, P_LEVEL), 55, 0) >= 0);
        assert(lk_put(&banks[s], lk_key(0, 1, P_PAN), -12, 1) >= 0);
        banks[s].on = 1; chain.src[s].m = &banks[s];
        for (unsigned k = 0; k < NTRK; k++) {
            chain.src[s].step[k] = steps[s][k]; chain.src[s].timing[k][0] = 8;
            chain.src[s].timing[k][1] = 2; chain.src[s].model[k] = (uint8_t)trk[k].p[P_MODEL];
            for (unsigned c = 0; c < NSTEP; c++) steps[s][k][c].on = 1;
        }
    }
    chain.src[1].model[0] = DM_ACID; /* preserve existing model mismatch filtering of P_E events */
    chain.cfg.count = 2; chain.cfg.row[0] = (chain_row_t){0, 1}; chain.cfg.row[1] = (chain_row_t){1, 1};
    chain.armed = 1; seq_start(); assert(seq_remix_test_set(0, 0, 2, 1));
    seq_tick(t, 0); next(t);
    assert(t->p[P_E1] == 100 && mo.under[0][P_E1] == 30 && t->p[P_LEVEL] == 55 && t->p[P_PAN] == -12);
    chain.row = 1; chain_apply(0);
    assert(t->p[P_E1] == 64 && t->p[P_LEVEL] == 90 && t->p[P_PAN] == -7);
    seq_tick(t, 0); next(t);
    assert(t->p[P_E1] == 64 && !lock_held(0, P_E1) && t->p[P_LEVEL] == 55 && t->p[P_PAN] == -12);
    seq_stop(); assert(t->p[P_E1] == 64 && t->p[P_LEVEL] == 90 && t->p[P_PAN] == -7);
    puts("remix motion/locks: Grids bypass/song OFF slots, row-owned banks, row transitions, model filtering and STOP restoration passed");
}
static uint32_t fold(uint32_t h, uint32_t v) { return (h ^ v) * 16777619u; }
/* Production-engine oracle: rearrange both stored steps and bank keys for OFF. */
static uint32_t fixture(unsigned len, int acid, unsigned bank, unsigned pat, int on, int materialize)
{
    track_t *t = fresh(len, acid);
    step_t original[NSTEP]; lk_bank_t original_bank;
    memset(&original_bank, 0, sizeof original_bank); original_bank.on = 1;
    for (unsigned s = 0; s < NSTEP; s++) {
        memset(&original[s], 0, sizeof original[s]); original[s].on = s % 5 != 2;
        original[s].nt = (int8_t)((int)(s % 24) - 12); original[s].acc = s & 1;
        original[s].tie = acid && s % 7 == 3; original[s].slide = acid && s % 3 == 0;
        original[s].rat = s / 3 % 4; original[s].cond = s % 4 == 0 ? 22 : 0;
        if (s % 3 == 1) lk_put(&original_bank, lk_key(0, s, P_E1), 25 + (int)s, 0);
        if (s % 4 == 1) lk_put(&original_bank, lk_key(0, s, P_E1), 100, 1);
        if (s % 5 == 1) lk_put(&original_bank, lk_key(0, s, P_PAN), (int)(s % 31) - 15, 1);
    }
    for (unsigned c = 0; c < NSTEP; c++) {
        unsigned s = materialize ? (unsigned)remix_source_step(bank, pat, c) : c;
        t->step[c] = original[s];
        unsigned i = lk_seek(&original_bank, 0, s);
        for (; i < original_bank.count && (original_bank.key[i] >> 7) == s; i++)
            assert(lk_put(&mo.b, lk_key(0, c, original_bank.key[i] & 127), original_bank.val[i], lk_is_lock(&original_bank, i)) >= 0);
    }
    mo.b.on = 1;
    lk_bank_t before = mo.b;
    assert(seq_remix_test_set(0, bank, pat, (unsigned)on)); seq_tick(t, 0);
    uint32_t h = 2166136261u;
    for (unsigned c = 0; c < len * 3; c++) {
        assert(t->seq_idx == c % len && t->seq_cnt == c);
        h = fold(h, t->p[P_E1]); h = fold(h, t->p[P_PAN]);
        h = fold(h, mo.under[0][P_E1]); h = fold(h, mo.under[0][P_PAN]);
        h = fold(h, mo.act[0][0]); h = fold(h, mo.lact[0][0]);
        h = fold(h, t->gate_n); h = fold(h, t->v[0].age); h = fold(h, t->nt); h = fold(h, t->rng);
        h = fold(h, t->v[0].k[0]); h = fold(h, duration(t));
        next(t);
    }
    assert(!memcmp(&before, &mo.b, sizeof before));
    seq_stop(); assert(t->p[P_E1] == 64 && t->p[P_PAN] == -7);
    return h;
}
static void test_equivalence(void)
{
    static const unsigned selections[][2] = {{0, 1}, {0, 2}, {1, 0}, {15, 15}};
    for (unsigned l = 0; l < 6; l++) for (int acid = 0; acid <= 1; acid++) {
        uint32_t off = fixture(LENGTHS[l], acid, 0, 0, 0, 0);
        assert(off == fixture(LENGTHS[l], acid, 0, 0, 1, 0));
        assert(off == fixture(LENGTHS[l], acid, 15, 15, 0, 0));
        for (unsigned s = 0; s < 4; s++) {
            unsigned bank = selections[s][0], pat = selections[s][1];
            assert(fixture(LENGTHS[l], acid, bank, pat, 1, 0) == fixture(LENGTHS[l], acid, bank, pat, 0, 1));
        }
    }
    puts("remix motion/locks: OFF/A1 and active/materialized production playback match for drums/ACID, six lengths, repeated sources and all blocks");
}
int main(void)
{
    test_chronological_reset(); test_eligibility_and_precedence(); test_repeats_and_transport();
    test_eight_tracks(); test_lfo(); test_acid(); test_bypasses(); test_equivalence();
    puts("remix motion/locks: all focused checks passed");
    return 0;
}
