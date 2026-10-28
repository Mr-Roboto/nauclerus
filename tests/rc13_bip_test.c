/* SPDX-License-Identifier: GPL-3.0-only */
/* Run this identical fixture against the pre-RC13 snapshot and current sources.
 * Audio/state fingerprints exercise only unchanged BIP, both signs and all slots. */
#define main rc13_unused_boot_main
#include "boot_test.c"
#undef main
#include <assert.h>
#include <inttypes.h>
static uint64_t fold13(uint64_t h,uint32_t x){return (h^x)*UINT64_C(1099511628211);}
int main(void)
{
    static const int depths[]={-64,-33,-1,0,1,33,64};
    for(unsigned di=0;di<sizeof depths/sizeof *depths;di++) {
        fresh_flash();wipe_ram();ui_host_init();drum_tracks_init();
        drum_set_model(&trk[2],DM_ACID);drum_set_model(&trk[6],DM_ACID);
        for(unsigned k=0;k<NTRK;k++) {
            track_t *t=&trk[k];t->p[P_SLEN]=16;t->p[P_SRC]=0;t->p[P_LEVEL]=88;
            for(unsigned n=0;n<16;n++){t->step[n].on=(k==2||k==6)||n%4==0;t->step[n].acc=n%4==0;t->step[n].nt=(int8_t)((k==2||k==6)?-12+(n%4)*2:0);}
            for(unsigned l=0;l<2;l++){unsigned q=l?P_LFO2:P_LFO1;t->p[q+LF_WAVE]=LW_SINE;t->p[q+LF_MODE]=LM_HZ;t->p[q+LF_RATE]=50;t->p[q+LF_PHASE]=(int16_t)(l?84:19);}
            for(unsigned s=0;s<4;s++){
                t->p[LS_ID[s][LS_SRC]]=(int16_t)(s&1);t->p[LS_ID[s][LS_DEPTH]]=(int16_t)(s&1?-depths[di]:depths[di]);
                t->p[LS_ID[s][LS_DEST]]=(int16_t)(s==0?9:s==1?17:s==2?18:(k==2||k==6)?19:2);
            }
        }
        seq_start();uint64_t audio=UINT64_C(14695981039346656037),state=audio;int32_t out[2*CTL];unsigned nonzero=0;
        for(unsigned b=0;b<4096;b++){
            mix_block(out,CTL);
            for(unsigned i=0;i<2*CTL;i++){audio=fold13(audio,(uint32_t)out[i]);nonzero|=out[i]!=0;}
            for(unsigned k=0;k<NTRK;k++){
                state=fold13(state,trk[k].seq_idx);state=fold13(state,trk[k].lfo[0].out);state=fold13(state,trk[k].lfo[1].out);
                for(unsigned id=0;id<P_COUNT;id++)state=fold13(state,(uint32_t)trk[k].p[id]);
            }
        }
        assert(nonzero);seq_stop();transport_req=0;
        printf("BIP depth=%d audio=%016" PRIx64 " state=%016" PRIx64 "\n",depths[di],audio,state);
    }
    return 0;
}
