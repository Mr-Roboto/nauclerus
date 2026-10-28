/* SPDX-License-Identifier: GPL-3.0-only */
/* Accepted-source comparison fixture; see docs/ACID_GOLDEN_0.25.0.md. */
#define main original_drum_suite_main
#include "drum_test.c"
#undef main
int main(void) {
    for(unsigned m=0;m<NMODELS;m++) {
        host_init();drum_set_model(&trk[0],m);drum_hit(&trk[0],127);
        render_track(&trk[0],wl,SECS(1));
        printf("%s %08x",N_MODEL[m],fnv(wl,SECS(1)));
        if(m==DM_ACID){printf(" knobs");for(unsigned i=0;i<8;i++)printf(" %d",trk[0].p[P_E0+i]);printf(" level %d morph %d",trk[0].p[P_LEVEL],trk[0].p[P_AMORPH]);}
        puts("");
        if(m==DM_ACID){FILE*f=fopen("acid.pcm","wb");if(!f)return 2;fwrite(wl,sizeof *wl,SECS(1),f);fclose(f);}
    }
    return 0;
}
