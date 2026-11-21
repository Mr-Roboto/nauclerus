/* SPDX-License-Identifier: GPL-3.0-only
 * Nauclerus 1.1 SOLO: black-key MUTE-screen input and momentary releases. */
#include "ui_host.h"
static unsigned fail;
static void chk(const char *s, int ok) {
    printf("%s  %s\n",ok?"ok  ":"FAIL",s);fail+=!ok;
}
static void frame2(void){ui_frame();ui_frame();}
int main(void)
{
    unsigned k;
    ui_host_init();
    solo_mom=solo_down=0;
    chk("black SOLO keys are the eight F#3..A#4 keys",
        SOLO_KEY[0]==1 && SOLO_KEY[7]==17 && KEY_SOLO_MODE==25);
    for(k=0;k<NTRK;k++){
        if (k) {keys(0);frame2();}
        press(B_GLO);
        keys(1u<<SOLO_KEY[k]);
        frame2();
        chk("GLO black key toggles solo without muting the track",
            ui.mute && ((song.solo>>k)&1u) && !trk[k].p[P_MUTE]);
        keys(0);
        frame2();
        keys(1u<<SOLO_KEY[k]);
        frame2();
        chk("second black-key tap unsolos",!((song.solo>>k)&1u));
        keys(0);
        frame2();
        release_all();
        frame2();
    }
    press(B_GLO);
    keys(1u<<KEY_SOLO_MODE);
    frame2();
    chk("F#5 switches to momentary SOLO",solo_mom==1);
    keys(0);
    frame2();
    press(B_GLO);
    keys(1u<<SOLO_KEY[2]);
    frame2();
    chk("momentary black key solos its track",((song.solo>>2)&1u)&&((solo_down>>2)&1u));
    keys(0);
    frame2();
    frame2();
    chk("releasing a momentary black key unsolos it",!((song.solo>>2)&1u) && !(solo_down&(1u<<2)));
    release_all();
    frame2();
    chk("no mute flag changed by SOLO",!trk[2].p[P_MUTE]);
    printf(fail?"solo_test: %u FAILED\n":"solo_test: all checks passed\n",fail);
    return fail?1:0;
}
