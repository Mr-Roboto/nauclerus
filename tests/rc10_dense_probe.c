/* SPDX-License-Identifier: GPL-3.0-only */
/* Same controlled dense-mix probe compiled against RC9 and current sources. */
#ifndef PROBE_HOST
#define PROBE_HOST "drum_host.h"
#endif
#include PROBE_HOST
#include <math.h>
#include <stdio.h>
static void setup(unsigned note,unsigned cut,unsigned res,unsigned morph)
{
    host_init_live();drum_set_model(&trk[2],DM_ACID);drum_set_model(&trk[6],DM_ACID);
    for(unsigned k=0;k<NTRK;k++){
        trk[k].p[P_SRC]=0;trk[k].p[P_SLEN]=16;trk[k].p[P_SDIV]=2;trk[k].p[P_LEVEL]=(int16_t)((k==2||k==6)?104:88);trk[k].p[P_DLY]=20;trk[k].p[P_REV]=15;
        for(unsigned j=0;j<16;j++){
            trk[k].step[j].on=(k==2||k==6)?1:j%(k%3+2)==0;
            trk[k].step[j].nt=(int8_t)((k==2||k==6)?(int)note-60+(k==6?7:0):0);
            trk[k].step[j].acc=j%4==0;trk[k].step[j].slide=(k==2||k==6)&&j%8==2;
        }
    }
    for(unsigned k=2;k<=6;k+=4){int16_t p[8]={0,50,(int16_t)cut,(int16_t)res,60,64,(int16_t)(k==2?3:1),127};memcpy(trk[k].p+P_E0,p,sizeof p);trk[k].p[P_AMORPH]=(int16_t)(k==2?morph:64);}
    acid_wave_resolve();song.g[G_BPM]=150;seq_start();
}
int main(int argc,char **argv)
{
    if(argc!=2)return 2;unsigned ns[]={24,33,45},ms[]={0,32,64,96,112,127};
    puts("engine,note,cut,res,morph,mix_rms,mix_peak,mix_mean,acid_sparse_rms,acid_sparse_peak,both_acid_active,blocks");
    for(unsigned n=0;n<3;n++)for(unsigned stress=0;stress<2;stress++)for(unsigned m=0;m<6;m++){
        unsigned cut=stress?120:80,res=stress?120:30;setup(ns[n],cut,res,ms[m]);
        double sq=0,dc=0,pk=0,vsq=0,vpk=0;unsigned count=0,active=0;
        for(unsigned block=0;block<4096;block++){
            int32_t pcm[2*CTL];mix_block(pcm,CTL);
            if(block<128)continue;
            for(unsigned i=0;i<2*CTL;i++){double x=pcm[i];sq+=x*x;dc+=x;if(fabs(x)>pk)pk=fabs(x);count++;}
            double v=trk[2].v[0].last;vsq+=v*v;if(fabs(v)>vpk)vpk=fabs(v);
            active+=trk[2].v[0].active && trk[6].v[0].active;
        }
        printf("%s,%u,%u,%u,%u,%.6f,%.0f,%.6f,%.6f,%.0f,%u,%u\n",argv[1],ns[n],cut,res,ms[m],sqrt(sq/count),pk,dc/count,sqrt(vsq/(4096-128)),vpk,active,4096-128);
        seq_stop();transport_req=0;
    }
    return 0;
}
