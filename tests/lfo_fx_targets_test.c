/* SPDX-License-Identifier: GPL-3.0-only
 * Nauclerus 1.1: append-only LFO FX destinations; production code, host test. */
#include "drum_host.h"
static unsigned fails;
static void ck(const char *name, int yes) { printf("%s  %s\n",yes?"ok  ":"FAIL",name); fails+=!yes; }

int main(void)
{
    const uint32_t ids[4]={P_DIST,P_CHOR,P_DLY,P_REV};
    const char *const names[4]={"DST","CHO","DLY","REV"};
    unsigned j,k;
    host_init_live();
    ck("Nauclerus ACID MORPH retains stored DEST 19",lfo_dest_param(19)==P_AMORPH);
    ck("DEST IDs 20..23 add exactly DIST CHO DLY REV",
       lfo_dest_param(20)==P_DIST && lfo_dest_param(21)==P_CHOR &&
       lfo_dest_param(22)==P_DLY && lfo_dest_param(23)==P_REV &&
       lfo_dest_param(24)==0xFFu);
    for (j=0;j<4;j++) {
        track_t *t=&trk[0];
        host_init_live();
        t->p[P_LFO1+LF_WAVE]=LW_SINE;
        t->p[P_LFO1+LF_MODE]=LM_HZ;
        t->p[P_LFO1+LF_RATE]=64;
        t->p[P_LFO1+LF_PHASE]=32; /* quarter cycle: immediate nonzero */
        t->p[P_LFO1+LF_TRIG]=LT_FREE;
        t->p[P_LSRC1]=0;          /* LFO1 bipolar */
        t->p[P_LFO1+LF_DEPTH]=64;
        t->p[P_LFO1+LF_DEST]=(int16_t)(20+j);
        t->p[ids[j]]=50;
        ck("new DEST descriptor accepts ID",track_desc(t,P_LFO1+LF_DEST)->max==23);
        ck("new LFO destination label follows track FX knob",
           strcmp(lfo_dest_label(t,(int32_t)(20+j)),names[j])==0);
        lfo_track(t,CTL);
        ck("LFO modulates correct FX parameter",
           t->lnum==1u && t->lpid[0]==ids[j] && t->p[ids[j]]!=50);
        for (k=0;k<4;k++) if(k!=j) ck("other FX sends unchanged",t->p[ids[k]]==TP[ids[k]].def);
        {
            int32_t live=0;
            ck("live meter matches active value",lfo_live(t,ids[j],&live)&&live==t->p[ids[j]]);
        }
        lfo_restore_track(t);
        ck("modulation restores saved knob value",t->p[ids[j]]==50);
    }
    printf(fails?"lfo_fx_targets_test: %u FAILED\n":"lfo_fx_targets_test: all checks passed\n",fails);
    return fails?1:0;
}
