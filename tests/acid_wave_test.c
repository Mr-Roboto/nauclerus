/* SPDX-License-Identifier: GPL-3.0-only */
/* Focused RC8 checks, using the existing real UI/boot/DSP host harness. */
#define main acid_wave_boot_main
#include "boot_test.c"
#undef main
#include <assert.h>
#include <time.h>
#ifndef RC8_BASELINE
#define RC8_BASELINE "fixtures/acid_rc7_baseline.h"
#endif
#ifndef RC8_REFERENCE
#define RC8_REFERENCE "fixtures/x0x_morph_v110.h"
#endif
#include RC8_BASELINE
#include RC8_REFERENCE
static void input(void){ui_input();ui_leds();}
static void tap(unsigned b){press(b);input();release_all();input();}
static void knob(unsigned k,int n){if(n){turn(EN_K1+k,n);input();}}
static void fresh(void){fresh_flash();wipe_ram();ui_host_init();}
static void sound_page(unsigned n){ui.home=0;ui.page=(uint8_t)(page_first(FAM_SND)+n);page_entered();}
static uint32_t duration(track_t *t){return step_samples(t,div_period((uint32_t)t->p[P_SDIV],t->seq_rem),t->seq_cnt);}
static void next(track_t *t){seq_tick(t,duration(t)-t->seq_pos);}
static void fastslow(void)
{
    static const int16_t want[8]={0,50,80,30,60,64,1,40};
    fresh();drum_tracks_init();assert(TP[P_LEVEL].def==104);
    for(unsigned k=0;k<NTRK;k++)assert(trk[k].p[P_MODEL]==KIT_DEF[k][0] && trk[k].p[P_LEVEL]==104);
    unsigned id=preset_first_of(DM_ACID);
    assert(str_eq(PRE_FACTORY[id].name,"FASTSLOW") && PRE_UID[id]==152 && preset_bank_of(id)==PB_MODERN);
    assert(str_eq(PRE_FACTORY[id+1].name,"SQUELCH"));
    for(unsigned j=0;j<8;j++)assert(PRE_FACTORY[id].k[j]==want[j] && DMODELS[DM_ACID].edit[j].def==want[j]);
    drum_set_model(&trk[2],DM_ACID);assert(trk[2].p[P_LEVEL]==106 && trk[2].pre==id);
    for(unsigned j=0;j<8;j++)assert(trk[2].p[P_E0+j]==want[j]);
    trk[6].p[P_LEVEL]=87;drum_set_model(&trk[6],DM_ACID);assert(trk[6].p[P_LEVEL]==87);
    assert(preset_load(&trk[0],id)==-1); /* two ACID voices still */
    trk[2].p[P_LEVEL]=104;assert(!preset_load(&trk[2],id+1) && trk[2].p[P_LEVEL]==104);
    assert(!preset_load(&trk[2],id) && trk[2].p[P_LEVEL]==104);
    drum_set_model(&trk[2],DM_ACID);assert(trk[2].p[P_LEVEL]==104);
    for(unsigned k=0;k<NTRK;k++)if(k!=2 && k!=6)assert(trk[k].p[P_MODEL]==KIT_DEF[k][0] && trk[k].p[P_LEVEL]==104);
    puts("A PASS: FASTSLOW first MODERN ACID, UID152/exact knobs/fallbacks, model-init level104->106 only, adjusted/preset levels and drum kit unchanged, two-ACID cap retained");
}
static void legacy_audio(void)
{
    for(unsigned wave=0;wave<2;wave++) {
        track_t a=trk[0],b=a;dvoice_t av={0},bv={0};
        int32_t x[CTL],y[CTL];
        static const int16_t p[8]={0,50,40,64,64,64,0,56};
        memcpy(a.p+P_E0,p,sizeof p);a.p[P_E6]=(int16_t)wave;a.p[P_AMORPH]=64;a.p[P_MODEL]=DM_ACID;a.model=DM_ACID;a.nt=-27;b=a;
        av.vel=bv.vel=96;av.active=bv.active=1;
        acid_trigger(&a,&av);baseline_acid_trigger(&b,&bv);
        for(unsigned block=0;block<800;block++) {
            if(block==70 || block==130) {
                a.nt=b.nt=block==70?-20:-32;av.vel=bv.vel=127;
                acid_glide(&a,&av);baseline_acid_glide(&b,&bv);
            }
            if(block==300){acid_gate(&a,&av,0);baseline_acid_gate(&b,&bv,0);}
            if(block==600){av.vel=bv.vel=96;acid_trigger(&a,&av);baseline_acid_trigger(&b,&bv);}
            memset(x,0,sizeof x);memset(y,0,sizeof y);
            acid_render(&a,&av,x,CTL);baseline_acid_render(&b,&bv,y,CTL);
            assert(!memcmp(x,y,sizeof x));
        }
    }
}
static void waves(void)
{
    fresh();legacy_audio();
    unsigned positive=0;double sum=0,power=0;
    for(unsigned i=0;i<65536;i++) {
        int32_t v=acid_plain_wave(i<<16,0,2);positive+=v>0;sum+=(double)v/QONE;power+=(double)v/QONE*v/QONE;
    }
    assert(positive==16384 && fabs(sum/65536)<1e-6 && fabs(power/65536-1.0/3.0)<1e-6);
    static const unsigned ms[]={0,32,63,64,65,96,127};
    static const double ds[]={0,0.00002,0.001,0.01,0.15,0.25,0.2844};
    double maxerr=0,maxdc=0,peak=0;
    for(unsigned m=0;m<128;m++) {
        acid_morph_mix_t q;reference_bass303_morph_mix_t r;
        acid_morph_prepare(&q,(int32_t)m*65536);reference_morph_prepare(&r,(float)m);
        for(unsigned d=0;d<sizeof ds/sizeof ds[0];d++) {
            uint32_t inc=(uint32_t)(ds[d]*4294967296.0),recip=inc?(uint32_t)((1ull<<48)/inc):0;
            double dc=0,energy=0;
            for(unsigned i=0;i<4096;i++) {
                uint32_t ph=i<<20;
                double got=(double)acid_morph_eval(&q,ph,inc,recip)/QONE;
                double want=reference_morph_eval(&r,(float)i/4096,(float)inc/4294967296.0f);
                double err=fabs(got-want);if(err>maxerr)maxerr=err;
                if(fabs(got)>peak)peak=fabs(got);
                dc+=got;energy+=got*got;
                assert(isfinite(got) && fabs(got)<2.0);
            }
            dc=fabs(dc/4096);if(dc>maxdc)maxdc=dc;
            assert(dc<0.0006);
            if(!inc)assert(fabs(energy/4096-1.0/3.0)<0.001);
        }
    }
    assert(maxerr<0.00008);
    for(unsigned j=0;j<sizeof ms/sizeof ms[0];j++) {
        track_t t=trk[0];dvoice_t v={0};int32_t out[CTL];
        t.p[P_MODEL]=DM_ACID;t.model=DM_ACID;t.p[P_E6]=3;t.p[P_AMORPH]=(int16_t)ms[j];t.nt=-27;
        t.p[P_E2]=127;t.p[P_E3]=127;v.vel=127;v.active=1;acid_trigger(&t,&v);
        for(unsigned n=0;n<1000;n++) {
            if(n==100)t.p[P_AMORPH]=127;
            if(n==300)t.p[P_AMORPH]=0;
            memset(out,0,sizeof out);acid_render(&t,&v,out,CTL);
            for(unsigned i=0;i<CTL;i++)assert(abs(out[i])<262144);
            if(n==102)assert(v.k[0]<=127u*65536u);
        }
        assert(v.k[0]==0);
    }
    printf("B PASS: SAW/SQUARE bit-identical vs pre-RC8 (slide/accent/gate/retrigger); PULSE25%%/DC/RMS; all128 Morph values vs pinned cubic oracle, max error %.8f, max phase-grid DC %.8f, peak %.5f; finite high-resonance audio/slew\n",maxerr,maxdc,peak);
}
static void ui_storage(void)
{
    fresh();drum_tracks_init();drum_set_model(&trk[2],DM_ACID);drum_set_model(&trk[6],DM_ACID);
    track_select(2);uint8_t ids[16];assert(snd_list(&trk[2],ids)==15 && ids[9]==P_AMORPH);
    sound_page(1);assert(page_id(cur_page(),3)==P_E6 && DMODELS[DM_ACID].edit[6].max==3);
    knob(3,2);assert(trk[2].p[P_E6]==3);
    track_select(6);sound_page(1);knob(3,1);assert(trk[6].p[P_E6]==2);knob(3,1);
    assert(trk[6].p[P_E6]==2 && trk[2].p[P_E6]==3 && str_eq(ui.msg,"MORPH IN USE"));
    trk[6].p[P_E6]=3; /* a pending Motion-style request must not displace the accepted owner */
    assert(acid_wave_guard(&trk[2])==3 && trk[2].p[P_E6]==3);
    assert(acid_wave_guard(&trk[6])==2 && trk[6].p[P_E6]==2);
    sound_page(2);assert(page_id(cur_page(),1)==P_AMORPH);
    knob(1,36);assert(trk[6].p[P_AMORPH]==100 && trk[2].p[P_AMORPH]==64);
    track_select(2);sound_page(2);knob(1,-53);assert(trk[2].p[P_AMORPH]==11 && trk[6].p[P_AMORPH]==100);
    /* Releasing an owner never applies a rejected request. */
    sound_page(1);knob(3,-3);assert(trk[2].p[P_E6]==0 && trk[6].p[P_E6]==2);
    track_select(6);sound_page(1);knob(3,1);assert(trk[6].p[P_E6]==3);
    /* Preset and runtime/Motion-style requests cannot acquire a second owner. */
    preset_t p=PRE_FACTORY[preset_first_of(DM_ACID)];p.k[6]=3;
    preset_apply(&trk[2],&p,PRE_NONE);assert(trk[2].p[P_E6]!=3 && trk[6].p[P_E6]==3);
    trk[2].p[P_E6]=3;assert(acid_wave_guard(&trk[2])!=3 && trk[2].p[P_E6]!=3);
    /* Ownership also covers the brief outgoing Morph crossfade. */
    drum_hit(&trk[6],96);trk[6].p[P_E6]=2;
    int32_t pcm[CTL]={0};acid_render(&trk[6],&trk[6].v[0],pcm,CTL);
    assert(trk[6].v[0].k[2]>>8);
    unsigned prior=(unsigned)trk[2].p[P_E6];trk[2].p[P_E6]=3;
    assert(acid_wave_guard(&trk[2])==prior && trk[2].p[P_E6]==prior);
    for(unsigned i=0;i<3;i++){memset(pcm,0,sizeof pcm);acid_render(&trk[6],&trk[6].v[0],pcm,CTL);}
    assert(!(trk[6].v[0].k[2]>>8));trk[2].p[P_E6]=3;assert(acid_wave_guard(&trk[2])==3);
    track_select(2);tap(B_ARP);assert(cur_fam()==FAM_TB3PO);
    trk[2].p[P_SLEN]=18;fm1_ms=1234;tap(B_OCTUP);assert(str_eq(ui.msg,"NEW LINE"));
    bpart_t generated={0};generated.len=18;generated.gen=tb_live[2].cfg;tb3po_generate(&generated);
    for(unsigned j=0;j<NSTEP;j++)assert(trk[2].step[j].nt==(int)generated.step[j].note-60 &&
        trk[2].step[j].on==(bstep_gate(&generated.step[j])==G_NOTE));
    assert(trk[2].p[P_AMORPH]==11 && trk[6].p[P_AMORPH]==100);
    unsigned base=tb_live[2].cfg.seed;assert(seq_remix_apply(2,274) && seq_remix_apply(6,291));
    tb_live[2].cfg.scale=6;tb_live[6].cfg.scale=18;
    motion_add(2,0,P_PAN,11);lock_set(2,1,P_LEVEL,110);lk_bank_t bank=mo.b;
    uint16_t remix[REMIX_SCOPES];memcpy(remix,(const void *)seq_remix_config,sizeof remix);
    uint8_t expect_m[4][NTRK],expect_w[4][NTRK];
    for(unsigned slot=0;slot<4;slot++) {
        trk[2].p[P_E6]=(int16_t)(slot&1?3:2);trk[6].p[P_E6]=(int16_t)(slot&1?1:3);acid_wave_resolve();
        for(unsigned k=0;k<NTRK;k++) {
            trk[k].p[P_AMORPH]=(int16_t)((slot*31+k*9)%128);
            expect_m[slot][k]=(uint8_t)trk[k].p[P_AMORPH];expect_w[slot][k]=(uint8_t)trk[k].p[P_E6];
            trk[k].p[P_SLEN]=1;
        }
        project_save(slot);assert(proj_ok(&proj_slot[slot]));
        assert(sizeof(project_t)==3840 && __builtin_offsetof(project_t,rsv_mo)==3252 && sizeof proj_slot==15360);
    }
    for(unsigned slot=0;slot<4;slot++) {
        assert(proj_load(slot,1));transport_req=0;
        for(unsigned k=0;k<NTRK;k++)assert(trk[k].p[P_AMORPH]==expect_m[slot][k]);
        assert(trk[2].p[P_E6]==expect_w[slot][2] && trk[6].p[P_E6]==expect_w[slot][6]);
        assert(tb_live[2].cfg.seed==base && tb_live[2].cfg.scale==6 && tb_live[6].cfg.scale==18);
        assert(!memcmp(remix,(const void *)seq_remix_config,sizeof remix) && bank_events_eq(&bank,&mo.b));
    }
    power_on(0);assert(trk[2].p[P_AMORPH]==expect_m[3][2]);
    project_t old=proj_slot[0];old.t[2].step[2].rsv=0;old.t[2].step[1].rsv=0;
    old.sum=proj_sum(&old);proj_slot[0]=old;assert(proj_load(0,1));transport_req=0;
    assert(trk[2].p[P_AMORPH]==64 && trk[2].p[P_LEVEL]==106);
    old.t[2].p[P_E6]=old.t[6].p[P_E6]=3;old.sum=proj_sum(&old);proj_slot[0]=old;
    assert(proj_load(0,1));transport_req=0;assert(trk[2].p[P_E6]==3 && trk[6].p[P_E6]==0);
    project_t saved[4];memcpy(saved,proj_slot,sizeof saved);
    int16_t livew[2]={trk[2].p[P_E6],trk[6].p[P_E6]},livem[2]={trk[2].p[P_AMORPH],trk[6].p[P_AMORPH]};
    chain.cfg.count=4;chain.cfg.loop=1;
    for(unsigned r=0;r<4;r++)chain.cfg.row[r]=(chain_row_t){(uint8_t)r,1};
    assert(!chain_prepare());seq_start();
    for(unsigned r=0;r<8;r++) {
        chain.row=(uint8_t)(r%4);chain_apply(0);
        assert(trk[2].p[P_AMORPH]==(r%4==0?64:expect_m[r%4][2]));
        assert(trk[6].p[P_AMORPH]==expect_m[r%4][6]);
        assert(!(trk[2].p[P_E6]==3 && trk[6].p[P_E6]==3));
        for(unsigned k=0;k<NTRK;k++)seq_tick(&trk[k],0);
        for(unsigned k=0;k<2;k++) {
            unsigned n=k?6:2;
            for(unsigned c=0;c<64;c++)assert(seq_source_step(&trk[n],c)==remix_source_step((remix[n]>>4)&15,remix[n]&15,c));
        }
    }
    chain.row=2;chain_apply(0);drum_hit(&trk[6],96);
    assert(trk[6].v[0].active && (trk[6].v[0].k[2]&3u)==3u);
    chain.row=0;chain_apply(0);
    assert(trk[2].p[P_E6]==3 && trk[6].p[P_E6]==0 && !trk[6].v[0].active);
    drum_hit(&trk[2],96);assert((trk[2].v[0].k[2]&3u)==3u);
    seq_stop();transport_req=0;
    assert(trk[2].p[P_E6]==livew[0] && trk[6].p[P_E6]==livew[1] && trk[2].p[P_AMORPH]==livem[0] && trk[6].p[P_AMORPH]==livem[1]);
    assert(!memcmp(saved,proj_slot,sizeof saved));
    puts("C PASS: WAVE4/MORPH ACID-only SOUND slots, independent physical values, explicit single owner/UI/preset/runtime rejection, project/boot/Song round-trip, deterministic conflicting loads/legacy64, ORB/TB-3PO/locks/Motion metadata retained; FDRF/noinit unchanged");
}
static double seconds(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec*1e-9;}
static void audio_half(int32_t *pcm)
{
    for(unsigned offset=0;offset<HALF_FRAMES;offset+=CTL)mix_block(pcm+2u*offset,CTL);
}
static double cost(unsigned wave)
{
    fresh();drum_tracks_init();drum_set_model(&trk[2],DM_ACID);drum_set_model(&trk[6],DM_ACID);
    trk[2].p[P_E6]=(int16_t)wave;trk[2].p[P_AMORPH]=96;trk[6].p[P_E6]=1;acid_wave_resolve();
    for(unsigned k=0;k<NTRK;k++) {
        trk[k].p[P_SRC]=0;trk[k].p[P_SLEN]=16;trk[k].p[P_SDIV]=2;
        trk[k].p[P_DLY]=20;trk[k].p[P_REV]=15;trk[k].p[P_LEVEL]=90;
        for(unsigned j=0;j<16;j++) {
            trk[k].step[j].on=(k==2||k==6)?j%4!=1:j%(k%3+2)==0;
            trk[k].step[j].nt=(int8_t)((k==2||k==6)?-27+(j%3)*3:0);
            trk[k].step[j].acc=j%4==0;trk[k].step[j].slide=(k==2||k==6)&&j%8==2;
        }
    }
    song.g[G_BPM]=140;seq_start();int32_t pcm[HALF_FRAMES*2];
    for(unsigned i=0;i<128;i++)audio_half(pcm);
    double start=seconds(),worst=0;uint64_t peak=0,clipped=0;
    for(unsigned i=0;i<2048;i++) {
        if(wave==3 && i%16==0)trk[2].p[P_AMORPH]=(int16_t)(i/16%2?0:127);
        double a=seconds();audio_half(pcm);double elapsed=seconds()-a;if(elapsed>worst)worst=elapsed;
        for(unsigned j=0;j<HALF_FRAMES*2;j++) {unsigned x=(unsigned)abs(pcm[j]);if(x>peak)peak=x;clipped+=x>=32767u;}
    }
    double total=seconds()-start;seq_stop();transport_req=0;
    printf("D cost: wave%u, six drums + two ACID/FX, %.3f us/256-frame half mean, %.3f us host maximum, peak %llu, clipped samples %llu\n",wave,total*1e6/2048,worst*1e6,(unsigned long long)peak,(unsigned long long)clipped);
    assert(peak<=32768u);
    return total;
}
int main(int argc,char **argv)
{
    setvbuf(stdout,0,_IOLBF,0);assert(argc==2);
    if(!strcmp(argv[1],"A"))fastslow();
    if(!strcmp(argv[1],"B"))waves();
    if(!strcmp(argv[1],"C"))ui_storage();
    if(!strcmp(argv[1],"D")){double a=cost(1),b=cost(3);printf("D PASS: fixed workload host Morph/Square ratio %.3fx; host timing is not FM-1 CPU headroom\n",b/a);}
    return 0;
}
