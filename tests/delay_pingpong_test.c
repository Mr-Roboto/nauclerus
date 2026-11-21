/* SPDX-License-Identifier: GPL-3.0-only
 * Nauclerus 1.1.0: test the production ping-pong bus and FDRF persistence.
 * Host only, no flashing or real device. */
#include "ui_host.h"

static unsigned failures;
static void verify(const char *label, int ok)
{
    printf("%s  %s\n", ok ? "ok  " : "FAIL", label);
    failures += !ok;
}
static int64_t mag(int32_t n) { return n < 0 ? -(int64_t)n : (int64_t)n; }

int main(void)
{
    static int32_t cho[CTL], din[CTL], rev[CTL], wetout[CTL];
    uint32_t n, i, d, clear_blocks;
    int64_t l_first=0, r_first=0, l_second=0, r_second=0;
    int side_ok=1;

    ui_host_init();
    verify("MODE appended after existing on-flash globals",
           G_DMODE == G_NSTORED && G_DMODE == G_RTYPE + 1 &&
           GP[G_DMODE].def == 0 && GP[G_DMODE].max == 1);
    verify("NORMAL/PING names keep the four-character LCD value field",
           strcmp(N_DMODE[0], "NORM") == 0 && strcmp(N_DMODE[1], "PING") == 0);
    verify("the project record layout is byte-for-byte the original FDRF size",
           sizeof(project_t) == 3840u && __builtin_offsetof(project_t, rsv_mo) == 3252u &&
           __builtin_offsetof(project_t, sum) == 3836u);
    song.g[G_DMODE] = 1;
    project_save(0);
    verify("PING saved into reserved global byte 0", proj_ok(&proj_slot[0]) &&
           proj_slot[0].rsv[0] == 1u);
    song.g[G_DMODE] = 0;
    project_load(0);
    verify("PING restored from saved record", song.g[G_DMODE] == 1);
    proj_slot[0].rsv[0] = 0;
    proj_slot[0].sum = proj_sum(&proj_slot[0]);
    song.g[G_DMODE] = 1;
    project_load(0);
    verify("old FDRF reserved byte 0 loads as NORMAL", song.g[G_DMODE] == 0);

    host_init_live();
    song.g[G_CLOCK] = 0;
    song.g[G_BPM] = 120;
    song.g[G_DTIME] = 3; /* 1/32 */
    song.g[G_DFDBK] = 100;
    song.g[G_DCOLOR] = 95;
    song.g[G_DMIX] = 127;
    song.g[G_DMODE] = 1;
    d = delay_samples();
    verify("echo interval fits old 65,536-sample ring", d > CTL && d < DLY_LEN);
    clear_blocks = DLY_LEN / DLY_CLR;
    for (n=0;n<clear_blocks;n++)
        fx_buses(cho,din,rev,wetout,CTL);
    verify("mode entry clears old delay safely in blocks", dpp.mode == 1 &&
           dpp.clr == 0 && dpp.side == 0);
    din[0]=din[1]=24000;
    for (n=0;n<(3u*d+CTL)/CTL+1u;n++) {
        uint32_t pos=n*CTL;
        fx_buses(cho,din,rev,wetout,CTL);
        side_ok &= dpp.side == 1;
        for (i=0;i<CTL;i++) {
            uint32_t t=pos+i;
            int64_t left=mag(wetout[i]+dly_side[i]),right=mag(wetout[i]);
            if(t>=d-12u&&t<=d+12u){l_first+=left;r_first+=right;}
            if(t>=2u*d-12u&&t<=2u*d+12u){l_second+=left;r_second+=right;}
        }
        din[0]=din[1]=0;
    }
    verify("PING renders a left-right difference while active", side_ok);
    verify("first echo appears mainly LEFT", l_first > 200 && l_first > 4*r_first);
    verify("second echo appears mainly RIGHT", r_second > 100 && r_second > 4*l_second);
    song.g[G_DMODE]=0;
    fx_buses(cho,din,rev,wetout,CTL);
    verify("return to NORMAL immediately disables stereo side", dpp.mode == 0 &&
           dpp.side == 0 && dpp.clr == DLY_LEN-DLY_CLR);
    for(n=1;n<clear_blocks;n++) fx_buses(cho,din,rev,wetout,CTL);
    verify("return to NORMAL flushes old delay echoes", dpp.clr == 0 &&
           dpp.mode == 0 && dpp.side == 0);
    fx_buses(cho,din,rev,wetout,CTL);
    verify("original NORMAL path has no stereo side", dpp.side == 0);

    printf(failures ? "delay_pingpong_test: %u FAILED\n" :
                      "delay_pingpong_test: all checks passed\n", failures);
    return failures ? 1 : 0;
}
