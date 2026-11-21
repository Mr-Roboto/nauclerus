/* SPDX-License-Identifier: GPL-3.0-only
 * Nauclerus 1.1 MIDI: routing, TRS->USB THRU and non-echo clock checks.
 * Production firmware included; host only, no hardware or flash. */
#include "ui_host.h"

static unsigned failures;
static void verify(const char *name, int ok) {
    printf("%s  %s\n", ok?"ok  ":"FAIL", name);
    failures += !ok;
}
static uint32_t note(uint32_t ch, uint32_t n, uint32_t vel) {
    return 0x09u | (0x90u | ch) << 8 | n << 16 | vel << 24;
}
static uint32_t realtime(uint32_t st) { return 0x0Fu | st << 8; }
static void midi_input(uint32_t packet, uint32_t source) {
    (void)midi_enqueue(packet, source);
    midi_block();
}

int main(void)
{
    uint32_t w,age[8],i;
    ui_host_init();
    verify("MIDI sync OFF/USB descriptor uses previously reserved G_SYNC ID",
           G_SYNC==G_MIDI+1 && GP[G_SYNC].max==1 &&
           !strcmp(N_SYNC[0],"OFF") && !strcmp(N_SYNC[1],"USB"));
    verify("MIDI ROUT NOTE/SEL/TRACK uses previously reserved G_ROUTE ID",
           G_ROUTE==G_SYNC+1 && GP[G_ROUTE].max==2 &&
           !strcmp(N_ROUTE[0],"NOTE") && !strcmp(N_ROUTE[1],"SEL") &&
           !strcmp(N_ROUTE[2],"TRACK"));
    verify("MIDI THRU action follows former actions without shifting old controls",
           G_THRU==G_METRO+1 && GP_ACT[4].max==1 &&
           !strcmp(GP_ACT[4].label,"THRU"));

    host_init_live();
    usb.config=1;
    for(i=0;i<NTRK;i++) trk[i].p[P_NOTE]=(int16_t)(60+i);
    for(i=0;i<NTRK;i++) age[i]=hit_age(&trk[i]);
    song.g[G_ROUTE]=RT_NOTE;
    midi_input(note(drum_ch(),61,112),1);
    verify("ROUT NOTE triggers track assigned matching NOTE only",
           hit_age(&trk[1])!=age[1] && hit_age(&trk[0])==age[0] &&
           hit_age(&trk[2])==age[2]);

    host_init_live();
    usb.config=1;
    for(i=0;i<NTRK;i++) age[i]=hit_age(&trk[i]);
    song.sel=2;
    song.g[G_ROUTE]=RT_SEL;
    midi_input(note(drum_ch(),67,100),1);
    verify("ROUT SEL plays selected track with pitched MIDI C4 reference",
           hit_age(&trk[2])!=age[2] && hit_age(&trk[0])==age[0]);

    host_init_live();
    usb.config=1;
    for(i=0;i<NTRK;i++) age[i]=hit_age(&trk[i]);
    song.g[G_ROUTE]=RT_TRACK;
    midi_input(note(3,72,100),1);
    verify("ROUT TRACK uses incoming MIDI channels 1..8 for tracks 1..8",
           hit_age(&trk[3])!=age[3] && hit_age(&trk[0])==age[0]);

    host_init_live();
    usb.config=1;
    midi_thru=1;
    mout.fw=mi_w;
    w=mo_w;
    (void)midi_enqueue(note(3,64,100),2u);        /* TRS input, third-party controller */
    mout_scan();
    verify("THRU forwards TRS notes to USB, preserving the original packet",
           mo_w==w+1 && midi_out_q[w%MQ]==note(3,64,100));
    w=mo_w;
    (void)midi_enqueue(note(3,65,100),1u);        /* USB input */
    mout_scan();
    verify("THRU never echoes USB notes back to USB",mo_w==w);
    midi_thru=0;
    (void)midi_enqueue(note(3,66,100),2u);
    mout_scan();
    verify("THRU OFF forwards nothing",mo_w==w);

    song.g[G_SYNC]=SY_USB;
    song.g[G_CLOCK]=2;                              /* TRS external clock */
    w=mo_w;
    (void)midi_enqueue(realtime(0xF8u),2u);
    mout_scan();
    verify("TRS external clock forwarded to USB",mo_w==w+1 &&
           midi_out_q[w%MQ]==realtime(0xF8u));
    song.g[G_CLOCK]=1;                              /* USB external clock */
    w=mo_w;
    (void)midi_enqueue(realtime(0xF8u),1u);
    mout_scan();
    verify("USB clock input never echoes to same port",mo_w==w);
    song.g[G_CLOCK]=0;
    mout_half(10000u);
    w=mout.w;
    mout_start();
    verify("own-clock transport Start is scheduled from the audio timebase",
           mout.w==w+1 && mout.st[w%MOUT_Q]==0xFAu);
    w=mout.w;
    mout_stop();
    verify("own-clock transport Stop is scheduled",
           mout.w==w+1 && mout.st[w%MOUT_Q]==0xFCu);
    printf(failures?"midi_features_test: %u FAILED\n":"midi_features_test: all checks passed\n",failures);
    return failures?1:0;
}
