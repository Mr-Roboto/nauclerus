/* SPDX-License-Identifier: GPL-3.0-only */
/* Reuse the existing production flash, boot and power-failure harness. */
#define DRUM_REMIX_TEST 1
#define main remix_existing_boot_main
#include "boot_test.c"
#undef main
#include <assert.h>
#include <stddef.h>

static void codes(unsigned seed, uint16_t out[REMIX_SCOPES])
{
    /* Serializer fixtures include every retained Bass scope and global Drum.
     * RC2 deliberately replaced the former per-track Drum ownership. */
    for (unsigned k=0;k<REMIX_SCOPES;k++) {
        unsigned code=(seed+k*37)%257;
        seq_remix_config[k]=seq_remix_state(code);
        if(out)out[k]=seq_remix_config[k];
    }
}
static void retained(const uint16_t expected[REMIX_SCOPES])
{
    for(unsigned k=0;k<REMIX_SCOPES;k++)assert(seq_remix_config[k]==expected[k]);
}
static void off(void)
{
    for(unsigned k=0;k<REMIX_SCOPES;k++)assert(!seq_remix_config[k]);
}
static void rehash_extension(project_t *p)
{
    uint32_t h = proj_hash(p->rsv_mo, p->rsv_mo[8]==1 ? 28 : 30);
    for (unsigned i = 0; i < 4; i++) p->rsv_mo[(p->rsv_mo[8]==1 ? 28 : 30) + i] = (uint8_t)(h >> (8 * i));
    p->sum = proj_sum(p);
}
static void test_roundtrip(void)
{
    fresh_flash(); wipe_ram();
    old_set_model(&trk[2],DM_ACID);old_set_model(&trk[6],DM_ACID);
    trk[0].step[7].on = 1; trk[0].step[7].nt = 9; trk[0].p[P_RKEY] = 73;
    motion_add(0, 4, P_E1, 44); lock_set(3, 63, P_LEVEL, 100);
    name_set(chain.name, "REMIX TEST");
    chain.cfg.count = 1; chain.cfg.row[0] = (chain_row_t){2, 3};
    static uint16_t saved[4][REMIX_SCOPES];
    for (unsigned seed = 0; seed <= 256; seed++) {
        unsigned slot = seed % 4; codes(seed, saved[slot]);
        step_t original[NTRK][NSTEP];
        for (unsigned k = 0; k < NTRK; k++) memcpy(original[k], trk[k].step, sizeof original[k]);
        lk_bank_t bank = mo.b;
        project_save(slot); assert(proj_ok(&proj_slot[slot]) && str_eq(ui.msg, "SAVED"));
        assert(proj_remix_valid(&proj_slot[slot]));
        assert(proj_slot[slot].rsv_mo[12] == (saved[slot][0] & 255) && proj_slot[slot].rsv_mo[13] == (saved[slot][0] >> 8));
        for (unsigned k = 0; k < REMIX_SCOPES; k++) assert(proj_remix_get(&proj_slot[slot], k) == saved[slot][k]);
        seq_remix_defaults();
        assert(proj_load(slot, 1)); retained(saved[slot]);
        for (unsigned k = 0; k < NTRK; k++) assert(!memcmp(original[k], trk[k].step, sizeof original[k]));
        assert(bank_events_eq(&bank, &mo.b) && trk[0].p[P_RKEY] == 73 && str_eq(chain.name, "REMIX TEST"));
        assert(chain.cfg.count == 1 && chain.cfg.row[0].slot == 2 && chain.cfg.row[0].repeat == 3);
        seq_start(); retained(saved[slot]);
        for (unsigned k = 0; k < NTRK; k++)
            assert(seq_remix[k] == saved[slot][k==2 || k==6 ? k : REMIX_DRUM]);
        seq_stop(); retained(saved[slot]);
        for (unsigned k = 0; k < NTRK; k++) assert(!seq_remix[k]);
    }
    uint16_t live[REMIX_SCOPES]; for (unsigned k = 0; k < REMIX_SCOPES; k++) live[k] = seq_remix_config[k];
    for (unsigned slot = 0; slot < 4; slot++)
        for (unsigned k = 0; k < REMIX_SCOPES; k++) assert(proj_remix_get(&proj_slot[slot], k) == saved[slot][k]);
    retained(live); assert(!proj_remix_get(&proj_slot[0], REMIX_SCOPES));
    assert(sizeof(project_t) == 3840 && sizeof proj_slot == 15360 && offsetof(project_t, rsv_mo) == 3252);
    assert(offsetof(project_t, song) == 3762 && offsetof(project_t, sum) == 3836 && sizeof(lk_bank_t) == 3208);
    puts("remix projects: all 256 selections plus OFF, eight tracks, LE encoding, A-D independence, steps/names/presets/song/Motion/locks and layout passed");
}
static void test_invalid_and_off(void)
{
    fresh_flash(); wipe_ram(); project_save(0);
    for (unsigned i = 0; i < PROJ_REMIX_BYTES; i++) assert(!proj_slot[0].rsv_mo[i]);
    uint16_t expected[REMIX_SCOPES]; codes(256, expected); project_save(0);
    project_t valid = proj_slot[0]; flash_ok = 0;
    for (unsigned kind = 0; kind < 8; kind++) {
        proj_slot[0] = valid;
        uint8_t *d = proj_slot[0].rsv_mo;
        if (kind == 0) memset(d, 0, sizeof proj_slot[0].rsv_mo);
        if (kind == 1) d[8] = 3;
        if (kind == 2) d[0] ^= 1;
        if (kind == 3) d[9] = 15;
        if (kind == 4) d[10] = 7;
        if (kind == 5) { d[12] = 0; d[13] = 2; } /* reserved enable bit: 512 */
        if (kind == 6) { d[14] = 255; d[15] = 255; }
        rehash_extension(&proj_slot[0]);
        if (kind == 7) { d[30] ^= 1; proj_slot[0].sum = proj_sum(&proj_slot[0]); }
        assert(proj_ok(&proj_slot[0]) && !proj_remix_valid(&proj_slot[0]));
        codes(17, 0); assert(proj_load(0, 1)); off();
    }
    for (unsigned seed = 1; seed <= 512; seed++) {
        proj_slot[0] = valid; rs = seed; rfill(proj_slot[0].rsv_mo, sizeof proj_slot[0].rsv_mo);
        proj_slot[0].sum = proj_sum(&proj_slot[0]);
        assert(proj_load(0, 1)); off();
    }
    proj_slot[0] = valid; proj_slot[0].rsv_mo[12] ^= 1;
    codes(33, expected); assert(!proj_ok(&proj_slot[0]) && !proj_load(0, 1)); retained(expected);
    assert(!proj_remix_get(&proj_slot[0], 0));
    seq_remix_defaults(); project_save(1);
    project_t unchanged = proj_slot[1];
    assert(!proj_remix_valid(&unchanged));
    assert(proj_load(1, 1)); project_save(1); assert(!memcmp(&unchanged, &proj_slot[1], sizeof unchanged));
    puts("remix projects: absent/unsupported/malformed/random extension defaults OFF without rejecting FDRF; corrupt record rejected; all-OFF save byte-exact");
}
static void test_legacy(void)
{
    static const uint32_t magic[15] = {0, PROJ_MAGIC_V1, PROJ_MAGIC_V2, PROJ_MAGIC_V3, PROJ_MAGIC_V4,
        PROJ_MAGIC_V5, PROJ_MAGIC_V6, PROJ_MAGIC_V7, PROJ_MAGIC_V8, PROJ_MAGIC_V9, PROJ_MAGIC_VA,
        PROJ_MAGIC_VB, PROJ_MAGIC_VC, PROJ_MAGIC_VD, PROJ_MAGIC_VE};
    static const uint32_t size[15] = {0, sizeof proj_old.v1, sizeof proj_old.v2, sizeof proj_old.v3,
        sizeof proj_old.v4, sizeof proj_old.v5, sizeof proj_old.v6, sizeof proj_old.v7, sizeof proj_old.v8,
        sizeof proj_old.v9, sizeof proj_old.va, sizeof proj_old.vb, sizeof proj_old.vc, sizeof proj_old.vd, sizeof proj_old.ve};
    for (unsigned ver = 1; ver <= 14; ver++) {
        fresh_flash(); wipe_ram();
        uint8_t raw[sizeof proj_old]; memset(raw, 0, sizeof raw);
        memcpy(raw, &magic[ver], 4); memcpy(raw + 4, &size[ver], 4);
        if (ver == 14) memcpy(raw + offsetof(project_t, rsv_mo), PROJ_REMIX_HEAD, sizeof PROJ_REMIX_HEAD);
        uint32_t sum = proj_hash(raw, size[ver] - 4); memcpy(raw + size[ver] - 4, &sum, 4);
        assert(!st_save(OBJ_PROJECT0, raw, size[ver]));
        codes(256, 0);
        assert(proj_fetch(0) >= 0 && proj_ok(&proj_slot[0]) && proj_load(0, 1)); off();
        for (unsigned i = 0; i < PROJ_REMIX_BYTES; i++) assert(!proj_slot[0].rsv_mo[i]);
    }
    assert(fdre_migrates()); off();
    puts("remix projects: real flash conversion of FDR1 through FDRE loads OFF; FDRE Motion/name/preset/step preservation passed");
}
static void test_boot_transport_and_gates(void)
{
    fresh_flash(); wipe_ram();
    uint16_t expected[REMIX_SCOPES]; codes(1, expected);
    trk[2].step[9].on = 1; trk[2].step[9].nt = 7;
    motion_add(2, 9, P_E1, 33); lock_set(2, 9, P_E2, 99);
    project_save(3); assert(lastp.slot == 3);
    assert(power_on(0) == 0); retained(expected); assert(!song.playing && trk[2].step[9].nt == 7);
    assert(mo.b.count == 2 && proj_remix_get(&proj_slot[3], REMIX_DRUM)==expected[REMIX_DRUM]);
    seq_start(); seq_stop(); seq_start(); retained(expected);
    project_t keep = proj_slot[2]; project_save(2); assert(!memcmp(&keep, &proj_slot[2], sizeof keep));
    assert(str_eq(ui.msg, "STOP TO SAVE")); seq_stop();
    chain.armed = 1; assert(!proj_load(3, 1)); retained(expected); chain.armed = 0;
    codes(20, 0); seq_start(); assert(proj_load(3, 1)); seq_stop(); retained(expected);
    seq_start(); retained(expected); seq_stop();
    init_all(); off();
    for (unsigned k = 0; k < REMIX_SCOPES; k++) assert(proj_remix_get(&proj_slot[3], k) == expected[k]);
    assert(power_on(1) == 0); off(); /* safe boot does not load a last project */
    puts("remix projects: power-on restore, safe/default boot, transport retention, playing save/load and song load gates passed");
}
static void test_torn_pair(void)
{
    static const int at[] = {6, 14, 16};
    for (unsigned i = 0; i < 3; i++) {
        fresh_flash(); wipe_ram();
        uint16_t expected[REMIX_SCOPES]; codes(256, expected);
        trk[0].p[P_E1] = 11; lock_set(0, 4, P_E1, 100); project_save(1);
        codes(1, 0); trk[0].p[P_E1] = 22; lock_set(0, 4, P_E1, 50);
        host_nflog = 0; host_fail_at = at[i]; project_save(1); host_fail_at = -1;
        assert(str_eq(ui.msg, "SAVE ERROR"));
        wipe_ram(); host_init(); persist_boot(); assert(proj_load(1, 1));
        retained(expected);
        int n = lk_find(&mo.b, lk_key(0, 4, P_E1), 1);
        assert(n >= 0 && mo.b.val[n] == 100 && trk[0].p[P_E1] == 11);
    }
    puts("remix projects: torn bank/header/project saves retain the prior configuration + project + lock-bank transaction");
}
static void test_song_ownership(void)
{
    fresh_flash(); wipe_ram(); codes(3, 0);
    for (unsigned k = 0; k < NTRK; k++) {
        trk[k].p[P_SLEN] = 8; trk[k].p[P_SRC] = 0;
        memset(trk[k].step, 0, sizeof trk[k].step);
        trk[k].step[1].on = trk[k].step[5].on = 1;
        trk[k].step[1].nt = 7; trk[k].step[5].nt = 20;
    }
    seq_remix_config[REMIX_DRUM]=seq_remix_state(3);
    project_save(0); assert(seq_remix_test_set(0, 1, 0, 1));
    chain.cfg.count = 1; chain.cfg.row[0] = (chain_row_t){0, 1};
    assert(chain_prepare() == 0); seq_start();
    assert(chain.running && seq_remix_config[REMIX_DRUM] == 272 && seq_source_step(&trk[0], 1) == 5);
    seq_tick(&trk[0], 0);
    seq_tick(&trk[0], step_samples(&trk[0], div_period((uint32_t)trk[0].p[P_SDIV], trk[0].seq_rem), trk[0].seq_cnt));
    assert(trk[0].nt == 20); seq_stop(); assert(seq_remix_config[REMIX_DRUM] == 272);
    puts("remix projects: persisted metadata is readable per slot without loading; song rows use slot-owned Remix");
}
int main(void)
{
    test_roundtrip(); test_invalid_and_off(); test_legacy(); test_boot_transport_and_gates();
    test_torn_pair(); test_song_ownership();
    printf("remix project layout: size %zu, extension %zu, song %zu, sum %zu, slots %zu\n",
           sizeof(project_t), offsetof(project_t, rsv_mo), offsetof(project_t, song), offsetof(project_t, sum), sizeof proj_slot);
    puts("remix project persistence: all focused checks passed");
    return 0;
}
