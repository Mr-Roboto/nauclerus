/* SPDX-License-Identifier: GPL-3.0-only */
/* Compile unchanged against RC11/RC12: real dense-mix PCM fingerprint and paired host timing. */
#ifndef PROBE_HOST
#define PROBE_HOST "drum_host.h"
#endif
#include PROBE_HOST
#include <time.h>
static double cpu12(void){struct timespec t;clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&t);return t.tv_sec+t.tv_nsec*1e-9;}
static double now12(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec*1e-9;}
int main(int argc,char **argv)
{
    if(argc!=2)return 2;int mode=atoi(argv[1]);host_init_live();
    drum_set_model(&trk[2],DM_ACID);drum_set_model(&trk[6],DM_ACID);
    for(unsigned k=0;k<NTRK;k++){
        track_t *t=&trk[k];t->p[P_SRC]=0;t->p[P_SLEN]=16;t->p[P_SDIV]=2;t->p[P_LEVEL]=(int16_t)((k==2||k==6)?104:88);
        t->p[P_DLY]=20;t->p[P_REV]=15;
        for(unsigned j=0;j<16;j++){t->step[j].on=(k==2||k==6)||j%(k%3+2)==0;t->step[j].nt=(int8_t)((k==2||k==6)?-27+(j%3)*3:0);t->step[j].acc=j%4==0;t->step[j].slide=(k==2||k==6)&&j%8==2;}
        for(unsigned l=0;l<2;l++){unsigned q=l?P_LFO2:P_LFO1;t->p[q+LF_WAVE]=LW_SINE;t->p[q+LF_MODE]=LM_HZ;t->p[q+LF_RATE]=90;t->p[q+LF_PHASE]=(int16_t)(l?64:0);t->p[q+LF_MORPH]=0;t->p[q+LF_TRIG]=LT_PLAY;}
        for(unsigned s=0;s<4;s++){
            t->p[LS_ID[s][LS_SRC]]=(int16_t)((s&1)+(mode?2*((s%2)+1):0));
            t->p[LS_ID[s][LS_DEST]]=(int16_t)((k==2||k==6)?19:s==0?9:s==1?17:s==2?18:3);
            t->p[LS_ID[s][LS_DEPTH]]=(int16_t)(s%2?-16:12);
        }
    }
    trk[2].p[P_E6]=3;trk[6].p[P_E6]=1;acid_wave_resolve();
    for(unsigned k=2;k<=6;k+=4){trk[k].p[P_E7]=127;trk[k].p[P_E2]=120;trk[k].p[P_E3]=120;trk[k].p[P_E4]=127;trk[k].p[P_E5]=127;}
    song.g[G_BPM]=150;seq_start();int32_t out[2*CTL];uint64_t hash=1469598103934665603ull;unsigned peak=0,active=0;
    double begin=now12(),cpu_begin=cpu12();
    for(unsigned n=0;n<32768;n++){
        mix_block(out,CTL);for(unsigned i=0;i<2*CTL;i++){uint32_t v=(uint32_t)out[i];for(unsigned j=0;j<4;j++){hash^=(v>>(8*j))&255u;hash*=1099511628211ull;}unsigned p=(unsigned)abs(out[i]);if(p>peak)peak=p;}
        active+=trk[2].v[0].active && trk[6].v[0].active;
    }
    double elapsed=now12()-begin;
    printf("mode=%d PCM=%016llx mean_us_per_256=%.3f peak=%u both_acid=%u/32768 cpu_us_per_256=%.3f\n",mode,(unsigned long long)hash,elapsed*1e6/4096,peak,active,(cpu12()-cpu_begin)*1e6/4096);
    return peak>32767 || active<32000;
}
