/* SPDX-License-Identifier: GPL-3.0-only */
/* RC10 targeted production-code checks. No hardware is contacted. */
static void rc10_watchdog(void);
#define UI_HOST_WATCHDOG_TICK() rc10_watchdog()
#define main rc10_boot_main
#include "boot_test.c"
#undef main
#include <assert.h>
#include <time.h>
#include "fixtures/acid_rc9_baseline.h"
static unsigned calibration_tick;
static void rc10_watchdog(void)
{
    if(calibration_tick){calibration_tick++;fm1_ms+=30001;fm1_in.buttons=0;}
}
static void input(void){ui_input();ui_leds();}
static void tap(unsigned b){press(b);input();release_all();input();}
static void knob(unsigned k,int d){if(d){turn(EN_K1+k,d);input();}}
static void fresh(void){fresh_flash();wipe_ram();ui_host_init();drum_tracks_init();home_screen();acc_oct=0;acid_wave_conflict=0;}
static void acid_pair(void){drum_set_model(&trk[2],DM_ACID);drum_set_model(&trk[6],DM_ACID);track_select(2);}
static void sound(unsigned n){ui.home=0;ui.page=(uint8_t)(page_first(FAM_SND)+n);page_entered();}
static void next(track_t *t){seq_tick(t,step_samples(t,div_period((uint32_t)t->p[P_SDIV],t->seq_rem),t->seq_cnt)-t->seq_pos);}
static int close_to(double a,double b,double e){return fabs(a-b)<=e;}
static void configuration(void)
{
    fresh();assert(TP[P_LEVEL].def==104);
    unsigned id=preset_first_of(DM_ACID);static const int16_t knobs[8]={-12,50,80,30,60,64,1,40};
    assert(PRE_UID[id]==152 && str_eq(PRE_FACTORY[id].name,"FASTSLOW") && str_eq(PRE_FACTORY[id+1].name,"SQUELCH"));
    for(unsigned j=0;j<8;j++)assert(PRE_FACTORY[id].k[j]==knobs[j] && DMODELS[DM_ACID].edit[j].def==knobs[j]);
    drum_set_model(&trk[2],DM_ACID);assert(trk[2].p[P_LEVEL]==104 && trk[2].p[P_E0]==-12 && trk[2].p[P_AMORPH]==64);
    trk[6].p[P_LEVEL]=81;drum_set_model(&trk[6],DM_ACID);assert(trk[6].p[P_LEVEL]==81);
    trk[2].p[P_LEVEL]=106;assert(!preset_load(&trk[2],id+1));assert(trk[2].p[P_LEVEL]==106);
    assert(!preset_load(&trk[2],id) && trk[2].p[P_LEVEL]==106 && trk[2].p[P_AMORPH]==64);
    for(unsigned k=0;k<NTRK;k++)if(k!=2 && k!=6)assert(trk[k].p[P_LEVEL]==104 && trk[k].p[P_MODEL]==KIT_DEF[k][0]);
    track_select(2);sound(0);trk[2].p[P_E6]=0;acid_wave_guard(&trk[2]);
    knob(1,1);assert(trk[2].p[P_E6]==1);knob(1,1);assert(trk[2].p[P_E6]==3);knob(1,1);assert(trk[2].p[P_E6]==3);
    knob(1,-1);assert(trk[2].p[P_E6]==1);knob(1,-1);assert(!trk[2].p[P_E6]);
    assert(str_eq(N_ACIDWAVE[0],"SAW") && str_eq(N_ACIDWAVE[1],"PULSE") && str_eq(N_ACIDWAVE[2],"PULSE") && str_eq(N_ACIDWAVE[3],"MORPH"));
    trk[2].p[P_E6]=2;knob(1,1);assert(trk[2].p[P_E6]==3);knob(1,-1);assert(trk[2].p[P_E6]==1);
    uint32_t last=0;
    for(unsigned m=0;m<128;m++){
        uint32_t duty=acid_pwm_duty(m*65536u);assert(duty>last);last=duty;
        if(!m)assert(close_to(duty/4294967296.0,.05,1e-8));
        if(m==64)assert(duty==0x80000000u);
        if(m==127)assert(close_to(duty/4294967296.0,.95,1e-8));
    }
    for(unsigned n=0;n<128;n+=7){
        uint32_t inc=acid_inc((int)n*16);
        for(unsigned i=0;i<4096;i++)assert(acid_pwm_wave(i<<20,inc,0x80000000u,inc,32768)==rc9_acid_plain_wave(i<<20,inc,1));
    }
    /* Preserve every old routing ID, then append the ACID-only destination. */
    static const unsigned old[19]={255,P_E0,P_E1,P_E2,P_E3,P_E4,P_E5,P_E6,P_E7,P_LEVEL,P_PAN,255,P_RDECAY,P_RMIX,P_RTONE,P_RSTRCT,P_RPOS,P_FCUT,P_FRESO};
    for(unsigned d=0;d<19;d++)assert(lfo_dest_param(d)==old[d]);
    assert(lfo_dest_param(19)==P_AMORPH);
    trk[2].p[P_E6]=3;acid_wave_guard(&trk[2]);
    trk[6].p[P_E6]=3;assert(acid_wave_guard(&trk[6])!=3 && acid_wave_conflict && trk[2].p[P_E6]==3);
    puts("A-F PASS: LEVEL104/user levels, FASTSLOW UID152/TUNE-12/default64, three visible choices/stable IDs, monotonic 5/50/95% duty and legacy Square oscillator exact at 50%");
}
static void lfo_pwm(void)
{
    fresh();acid_pair();
    for(unsigned track=0;track<2;track++)for(unsigned src=0;src<2;src++)for(int sign=-1;sign<=1;sign+=2){
        track_t *t=&trk[track?6:2];t->p[P_E6]=1;t->p[P_AMORPH]=64;
        for(unsigned s=0;s<4;s++){t->p[LS_ID[s][LS_DEST]]=0;t->p[LS_ID[s][LS_DEPTH]]=0;}
        for(unsigned l=0;l<2;l++){unsigned q=l?P_LFO2:P_LFO1;t->p[q+LF_WAVE]=LW_SINE;t->p[q+LF_MODE]=LM_HZ;t->p[q+LF_RATE]=50;t->p[q+LF_PHASE]=32;t->p[q+LF_MORPH]=0;t->p[q+LF_TRIG]=LT_FREE;}
        t->p[LS_ID[0][LS_SRC]]=(int16_t)src;t->p[LS_ID[0][LS_DEST]]=19;t->p[LS_ID[0][LS_DEPTH]]=(int16_t)(sign*16);
        lfo_track(t,0);assert(t->lnum==1 && t->lsave[0]==64 && (sign>0?t->p[P_AMORPH]>64:t->p[P_AMORPH]<64));
        lfo_restore_track(t);assert(t->p[P_AMORPH]==64);
    }
    track_t *t=&trk[2];
    for(unsigned s=0;s<4;s++){t->p[LS_ID[s][LS_SRC]]=(int16_t)(s&1);t->p[LS_ID[s][LS_DEST]]=19;t->p[LS_ID[s][LS_DEPTH]]=(int16_t)(s==3?-8:8);}
    for(unsigned i=0;i<2000;i++){
        int expected=64;lfo_track(t,CTL);
        for(unsigned s=0;s<4;s++){int m=t->lfo[t->p[LS_ID[s][LS_SRC]]].out*t->p[LS_ID[s][LS_DEPTH]]/64;expected+=m*127/32767;}
        assert(t->lnum==1 && t->p[P_AMORPH]==clamp(expected,0,127));lfo_restore_track(t);assert(t->p[P_AMORPH]==64);
    }
    t->p[P_E6]=0;lfo_track(t,CTL);assert(!t->lnum && t->p[P_AMORPH]==64);lfo_restore_track(t);
    t=&trk[0];t->p[P_AMORPH]=103;t->p[P_LFO1+LF_DEST]=19;t->p[P_LFO1+LF_DEPTH]=64;
    lfo_track(t,CTL);assert(!t->lnum && t->p[P_AMORPH]==103 && str_eq(lfo_dest_label(t,19),"--"));lfo_restore_track(t);
    assert(P_AMORPH!=P_LFO1+LF_MORPH && P_AMORPH!=P_LFO2+LF_MORPH);
    /* Active modulation reaches a smoothed actual pulse width. */
    t=&trk[2];t->p[P_E6]=1;t->p[P_E0]=0;t->p[P_AMORPH]=64;t->nt=-27;
    dvoice_t v={0};v.vel=96;v.active=1;acid_trigger(t,&v);uint32_t start=v.k[0];int32_t pcm[CTL];
    for(unsigned i=0;i<16;i++){t->p[P_AMORPH]=127;memset(pcm,0,sizeof pcm);acid_render(t,&v,pcm,CTL);assert(v.k[0]>=start && v.k[0]<=127u*65536u);start=v.k[0];}
    assert(v.k[0]==127u*65536u);
    for(unsigned i=0;i<16;i++){t->p[P_AMORPH]=0;memset(pcm,0,sizeof pcm);acid_render(t,&v,pcm,CTL);assert(v.k[0]<=start);start=v.k[0];}assert(v.k[0]==0);
    puts("G-I/J-LFO PASS: both LFOs/either ACID/all four slots, signed depth/summing/clamps, restored manual base/no drift; SAW/non-ACID inert, actual PWM finite smoothing");
}
static void ui_checks(void)
{
    fresh();acid_pair();trk[2].p[P_AMORPH]=112;preset_list_open();
    ui.pr.cur=(uint16_t)preset_first_of(DM_ACID);pl_audition(0,0);assert(trk[2].p[P_AMORPH]==64);
    tap(B_HOME);assert(!pl_on() && ui.home && trk[2].p[P_AMORPH]==112);
    sound(2);trk[2].p[P_E6]=0;trk[2].p[P_AMORPH]=112;trk[2].pre_ed=0;
    song.rec=1u<<2;seq_start();seq_tick(&trk[2],0);lk_bank_t bank=mo.b;
    knob(3,1);assert(trk[2].p[P_AMORPH]==112 && !trk[2].pre_ed && !memcmp(&bank,&mo.b,sizeof bank));
    ui.env=1;ui.held=0;ui.step_si[0]=0;ui.step_t0[0]=3;fm1_in.notes=1u<<STEP_KEY[0];
    knob(3,-1);assert(trk[2].p[P_AMORPH]==112 && !memcmp(&bank,&mo.b,sizeof bank));
    fm1_in.notes=0;memset(ui.step_t0,0,sizeof ui.step_t0);ui.env=0;seq_stop();transport_req=0;
    draw_columns();unsigned n=str_len(ui.col[3]);assert(n>=3 && ui.col[3][n-3]=='C');
    trk[2].p[P_E6]=1;knob(3,-1);assert(trk[2].p[P_AMORPH]==111);
    for(unsigned keys_on=0;keys_on<2;keys_on++)for(unsigned env_on=0;env_on<2;env_on++)for(unsigned play=0;play<2;play++)for(unsigned page=0;page<2;page++){
        fresh();acid_pair();trk[2].p[P_SLEN]=64;tap(B_ARP);if(page)tap(B_ARP);
        if(keys_on)tap(B_SCL);
        if(env_on)tap(B_ENV);
        /* Physical toggles retain their original mutual exclusion. Exercise
         * the requested simultaneous flags defensively without changing it. */
        if(keys_on && env_on){ui.keys=ui.env=1;song.seq_mode=seq_mode_now();}
        assert(ui.keys==keys_on && ui.env==env_on);
        if(play){seq_start();seq_tick(&trk[2],0);}fm1_ms=1000+100*keys_on+10*env_on+2*play+page;
        int octave=song.octave;unsigned oldbank=ui.bank;bank=mo.b;
        press(B_OCTUP);input();unsigned seed=tb_live[2].cfg.seed;assert(str_eq(ui.msg,"NEW LINE"));fm1_ms++;input();release_all();input();
        assert(tb_live[2].cfg.seed==seed && song.octave==octave && ui.bank==oldbank && !acc_oct && !memcmp(&bank,&mo.b,sizeof bank));
        if(play)next(&trk[2]);
        step_t expected[NSTEP];memcpy(expected,trk[2].step,sizeof expected);tb_state_t state=tb_live[2];uint32_t r=seed^fm1_ms;tb_mutate_steps(expected,&trk[2],&state,&r);
        press(B_OCTDN);input();assert(str_eq(ui.msg,"MUTATED") || str_eq(ui.msg,"NO CHANGE"));fm1_ms++;input();release_all();input();if(play)next(&trk[2]);
        assert(!memcmp(expected,trk[2].step,sizeof expected) && song.octave==octave && ui.bank==oldbank && !acc_oct && !memcmp(&bank,&mo.b,sizeof bank));
        if(play){seq_stop();transport_req=0;}
        /* Deliberately holding ENV still banks, even with KEYS on. */
        unsigned before=tb_live[2].cfg.seed;press(B_ENV);input();press(B_OCTUP);input();assert(tb_live[2].cfg.seed==before && ui.bank==1);
        fm1_in.buttons&=~(1u<<panel.btn[B_OCTUP]);input();release_all();input();assert(ui.bank==1 && tb_live[2].cfg.seed==before);
        ui.confirm=1;tap(B_OCTDN);assert(!ui.confirm && tb_live[2].cfg.seed==before);
        ui.ml.on=1;tap(B_OCTUP);assert(!ui.ml.on && tb_live[2].cfg.seed==before);
    }
    fresh();home_tracks();tap(B_GLO);assert(ui.menu==1);
    for(unsigned item=0;item<MI_COUNT;item++)if(!menu_submenu(item)){
        ui.menu_sel=(uint8_t)item;unsigned palette=settings.palette,spk=settings.lowcut,keys=ui.keys,zoom=settings.zoom;
        tap(B_SCL);assert(ui.menu==1 && ui.keys==keys && settings.palette==palette && settings.lowcut==spk && settings.zoom==zoom);
    }
    ui.menu_sel=MI_ABOUT;tap(B_SCL);assert(ui.menu==2 && !ui.about_top);tap(B_OCTDN);assert(ui.menu==1);
    ui.menu_sel=MI_ABOUT;tap(B_OCTUP);assert(ui.menu==2);turn(EN_SELECT,1);input();assert(ui.about_top==1);tap(B_HOME);assert(!ui.menu && ui.home && ui.hmode==1);
    for(unsigned button=0;button<2;button++){
        tap(B_GLO);ui.menu_sel=MI_PANEL;panel_t old=panel;calibration_tick=1;
        tap(button?B_OCTUP:B_SCL);assert(calibration_tick>1 && !memcmp(&panel,&old,sizeof old) && str_eq(ui.msg,"SETUP CANCELLED"));calibration_tick=0;
        tap(B_HOME);assert(ui.home && !ui.menu);
    }
    puts("J/M/N PASS: disabled SAW knob/dither/no locks/Motion; TB-3PO 16 page/KEYS/ENV/transport combinations exactly once, modified ENV bank navigation and priorities; SEL/OCT+ ABOUT/calibration entry and safe timeout, scalar SEL inert, HOME intact");
}
static void persistence(void)
{
    fresh();acid_pair();uint16_t remix[REMIX_SCOPES];for(unsigned k=0;k<REMIX_SCOPES;k++)remix[k]=(uint16_t)(seq_remix_config[k]=256u+k*13u);
    motion_add(2,0,P_PAN,11);lock_set(2,1,P_LEVEL,103);lk_bank_t locks=mo.b;
    uint32_t seed=tb_live[2].cfg.seed;
    for(unsigned raw=0;raw<4;raw++){
        trk[2].p[P_E6]=(int16_t)raw;trk[6].p[P_E6]=1;acid_wave_resolve();trk[2].p[P_AMORPH]=(int16_t)(raw*37);
        trk[2].p[P_LEVEL]=106;trk[2].p[P_E0]=0;trk[2].p[P_LFO1+LF_DEST]=19;trk[2].p[P_LFO2+LF_DEST]=19;
        trk[2].p[P_LFO1+LF_DEPTH]=17;trk[2].p[P_LFO2+LF_DEPTH]=-18;project_save(raw);
        assert(proj_ok(&proj_slot[raw]) && proj_slot[raw].t[2].step[2].rsv==0xA9);
    }
    project_t saved[4];memcpy(saved,proj_slot,sizeof saved);
    for(unsigned raw=0;raw<4;raw++){
        assert(proj_load(raw,1));transport_req=0;
        assert(trk[2].p[P_E6]==raw && trk[2].p[P_AMORPH]==raw*37 && trk[2].p[P_LEVEL]==106 && trk[2].p[P_E0]==0);
        assert(trk[2].p[P_LFO1+LF_DEST]==19 && trk[2].p[P_LFO2+LF_DEST]==19 && trk[2].p[P_LFO1+LF_DEPTH]==17 && trk[2].p[P_LFO2+LF_DEPTH]==-18);
        assert(tb_live[2].cfg.seed==seed && bank_events_eq(&locks,&mo.b) && !memcmp(remix,(const void *)seq_remix_config,sizeof remix));
    }
    /* All old identities, old tagged values and untagged earlier records. */
    for(unsigned marker=0;marker<2;marker++)for(unsigned raw=0;raw<4;raw++){
        project_t old=saved[0];old.t[2].p[P_E6]=(int16_t)raw;old.t[2].step[2].rsv=marker?0xA8:0;old.t[2].step[1].rsv=112;old.sum=proj_sum(&old);proj_slot[0]=old;
        assert(proj_load(0,1));transport_req=0;assert(trk[2].p[P_E6]==raw);
        int expected=raw==1?64:raw==2?28:marker?112:64;
        assert(trk[2].p[P_AMORPH]==expected && trk[2].p[P_LEVEL]==106 && trk[2].p[P_E0]==0);
        assert(!memcmp(&old,&proj_slot[0],sizeof old));
    }
    memcpy(proj_slot,saved,sizeof saved);assert(proj_load(3,1));transport_req=0;
    int livewave=trk[2].p[P_E6],livebase=trk[2].p[P_AMORPH];
    chain.cfg.count=4;chain.cfg.loop=1;for(unsigned i=0;i<4;i++)chain.cfg.row[i]=(chain_row_t){(uint8_t)i,1};
    assert(!chain_prepare());seq_start();
    for(unsigned i=0;i<8;i++){
        chain.row=(uint8_t)(i%4);chain_apply(0);assert(trk[2].p[P_E6]==i%4 && trk[2].p[P_AMORPH]==(int)(i%4*37));
        lfo_track(&trk[2],CTL);if(i%4)assert(trk[2].lnum==1);else assert(!trk[2].lnum);lfo_restore_track(&trk[2]);assert(trk[2].p[P_AMORPH]==(int)(i%4*37));
    }
    seq_stop();transport_req=0;assert(trk[2].p[P_E6]==livewave && trk[2].p[P_AMORPH]==livebase && !memcmp(saved,proj_slot,sizeof saved));
    assert(sizeof(project_t)==3840 && sizeof proj_slot==15360 && __builtin_offsetof(project_t,rsv_mo)==3252);
    puts("E/O/P PASS: raw WAVE0/1/2/3 legacy and current tagged semantics, 25%->24.6875% migration, original levels/tuning/UIDs, routing19 project/Song restore/base, existing ORB/TB/Motion/locks and fixed FDRF/noinit footprint");
}
typedef struct { double sum, sq, peak;unsigned n; } stats_t;
static void sample(stats_t *s,double x){s->sum+=x;s->sq+=x*x;if(fabs(x)>s->peak)s->peak=fabs(x);s->n++;}
static double rms_stat(const stats_t *s){return sqrt(s->sq/s->n);}
static double dc_stat(const stats_t *s){return s->sum/s->n;}
static void measure_voice(unsigned note,unsigned cut,unsigned res,unsigned m,int old,stats_t *osc,stats_t *audio)
{
    track_t t=trk[0];dvoice_t v={0};int16_t p[8]={0,50,80,30,60,64,3,40};memcpy(t.p+P_E0,p,sizeof p);
    t.p[P_MODEL]=DM_ACID;t.model=DM_ACID;t.nt=(int8_t)((int)note-60);t.p[P_AMORPH]=(int16_t)m;t.p[P_E2]=(int16_t)cut;t.p[P_E3]=(int16_t)res;t.wow=0;
    uint32_t inc=acid_inc((int32_t)note*16),recip=(uint32_t)((1ull<<48)/inc);
    acid_morph_mix_t a;rc9_acid_morph_mix_t b;
    acid_morph_prepare(&a,(int32_t)m*65536);rc9_acid_morph_prepare(&b,(int32_t)m*65536);
    for(unsigned i=0;i<65536;i++){
        int32_t x=old?rc9_acid_morph_eval(&b,i<<16,inc,recip):acid_morph_eval(&a,i<<16,inc,recip);
        sample(osc,(double)x/QONE);
    }
    v.vel=96;v.active=1;
    if(old)rc9_acid_trigger(&t,&v);else acid_trigger(&t,&v);
    for(unsigned j=0;j<1024;j++){
        int32_t out[CTL]={0};if(old)rc9_acid_render(&t,&v,out,CTL);else acid_render(&t,&v,out,CTL);
        if(j>=64)for(unsigned i=0;i<CTL;i++){assert(abs(out[i])<131072);sample(audio,out[i]);}
    }
}
static void audio_measurements(void)
{
    fresh();
    /* With feedback disabled, the ladder/output path is exactly RC9 for
     * Saw and Morph. The bass correction is not a global gain increase. */
    for(unsigned mode=0;mode<2;mode++){
        track_t a=trk[0],b;dvoice_t av={0},bv={0};
        a.model=DM_ACID;a.p[P_MODEL]=DM_ACID;a.p[P_E0]=0;a.p[P_E3]=0;
        a.p[P_E6]=(int16_t)(mode?3:0);a.p[P_AMORPH]=112;a.nt=-27;b=a;
        av.vel=bv.vel=96;av.active=bv.active=1;acid_trigger(&a,&av);rc9_acid_trigger(&b,&bv);
        for(unsigned j=0;j<1024;j++){
            int32_t x[CTL]={0},y[CTL]={0};acid_render(&a,&av,x,CTL);rc9_acid_render(&b,&bv,y,CTL);
            assert(!memcmp(x,y,sizeof x));
        }
    }
    unsigned notes[]={24,33,45},cuts[]={80,40,110},res[]={30,90,120},values[]={0,32,64,96,112,127};
    FILE *f=fopen("build/rc10-audio-measurements.csv","w");assert(f);
    fprintf(f,"engine,note,cut,res,morph,osc_rms,osc_peak,osc_dc,postfilter_rms,postfilter_peak,postfilter_mean\n");
    double minratio=100,maxdc=0;
    for(unsigned ni=0;ni<3;ni++)for(unsigned ci=0;ci<3;ci++)for(unsigned ri=0;ri<3;ri++){
        double mids[2]={0},ends[2]={0};
        for(unsigned mi=0;mi<6;mi++){
            stats_t osc[2]={{0}},voice[2]={{0}};
            for(unsigned old=0;old<2;old++){
                measure_voice(notes[ni],cuts[ci],res[ri],values[mi],(int)old,&osc[old],&voice[old]);
                assert(rms_stat(&osc[old])>.55 && rms_stat(&osc[old])<.59 && fabs(dc_stat(&osc[old]))<1e-5);
                fprintf(f,"%s,%u,%u,%u,%u,%.9f,%.9f,%.9f,%.6f,%.0f,%.6f\n",old?"RC9":"RC10",notes[ni],cuts[ci],res[ri],values[mi],rms_stat(&osc[old]),osc[old].peak,dc_stat(&osc[old]),rms_stat(&voice[old]),voice[old].peak,dc_stat(&voice[old]));
                if(mi==2)mids[old]=rms_stat(&voice[old]);
                if(mi==5)ends[old]=rms_stat(&voice[old]);
                if(fabs(dc_stat(&osc[old]))>maxdc)maxdc=fabs(dc_stat(&osc[old]));
            }
            assert(osc[0].sum==osc[1].sum && osc[0].sq==osc[1].sq && osc[0].peak==osc[1].peak);
        }
        for(unsigned old=0;old<2;old++){double r=ends[old]/mids[old];if(r<minratio)minratio=r;assert(r>.75);}
    }
    fclose(f);
    /* A moving parameter cannot accidentally disable the active oscillator. */
    track_t t=trk[0];dvoice_t v={0};t.p[P_MODEL]=DM_ACID;t.model=DM_ACID;t.p[P_E0]=0;t.p[P_E6]=3;t.p[P_AMORPH]=0;t.nt=-27;v.vel=96;v.active=1;acid_trigger(&t,&v);
    stats_t sweep={0};
    for(unsigned j=0;j<2048;j++){
        t.p[P_AMORPH]=(int16_t)(j<128?j:127);int32_t out[CTL]={0};acid_render(&t,&v,out,CTL);
        assert(v.active && (v.k[2]&3u)==3u);
        if(j>=128)for(unsigned i=0;i<CTL;i++)sample(&sweep,out[i]);
    }
    assert(rms_stat(&sweep)>100);
    printf("K/L offline PASS: before/after 162 note/filter/Morph cases, oscillator bit-identical and DC<=%.9f; min127/64 post-filter RMS ratio %.5f (both engines), active sweep never silenced. Hardware symptom not reproduced/root unconfirmed.\n",maxdc,minratio);
    puts("Measurements: build/rc10-audio-measurements.csv; effective MIDI24/33/45, TUNE0, VEL96, held gate, DECAY50/ENVMD60/ACCNT64; oscillator 65536 phase points; raw post-filter frames2048..32767 (30720 samples), before track/master gains/DC blocker.");
}
static double seconds(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec*1e-9;}
static void half(int32_t *out){for(unsigned i=0;i<HALF_FRAMES;i+=CTL)mix_block(out+2*i,CTL);}
static void performance(void)
{
    fresh();acid_pair();
    for(unsigned k=0;k<NTRK;k++){
        trk[k].p[P_SRC]=0;trk[k].p[P_SLEN]=16;trk[k].p[P_SDIV]=2;trk[k].p[P_LEVEL]=(int16_t)((k==2||k==6)?104:88);trk[k].p[P_DLY]=20;trk[k].p[P_REV]=15;
        for(unsigned j=0;j<16;j++){trk[k].step[j].on=(k==2||k==6)?1:j%(k%3+2)==0;trk[k].step[j].nt=(int8_t)((k==2||k==6)?-27+(j%3)*3:0);trk[k].step[j].acc=j%4==0;trk[k].step[j].slide=(k==2||k==6)&&j%8==2;}
    }
    trk[2].p[P_E6]=3;trk[6].p[P_E6]=1;acid_wave_resolve();
    for(unsigned k=2;k<=6;k+=4){
        track_t *t=&trk[k];t->p[P_E7]=127;t->p[P_E2]=120;t->p[P_E3]=120;t->p[P_E4]=127;t->p[P_E5]=127;t->p[P_AMORPH]=64;
        for(unsigned l=0;l<2;l++){unsigned q=l?P_LFO2:P_LFO1;t->p[q+LF_WAVE]=LW_SINE;t->p[q+LF_MODE]=LM_HZ;t->p[q+LF_RATE]=100;t->p[q+LF_PHASE]=(int16_t)(l?64:0);t->p[q+LF_MORPH]=0;t->p[q+LF_TRIG]=LT_FREE;}
        for(unsigned s=0;s<4;s++){t->p[LS_ID[s][LS_SRC]]=(int16_t)(s&1);t->p[LS_ID[s][LS_DEST]]=19;t->p[LS_ID[s][LS_DEPTH]]=(int16_t)(s&1?-16:16);}
    }
    song.g[G_BPM]=150;seq_start();int32_t out[HALF_FRAMES*2];for(unsigned i=0;i<128;i++)half(out);
    double start=seconds(),peak_time=0;unsigned peak=0,clip=0,acid_blocks=0;uint32_t lo[2]={UINT32_MAX,UINT32_MAX},hi[2]={0};
    for(unsigned i=0;i<4096;i++){
        double a=seconds();half(out);double dt=seconds()-a;if(dt>peak_time)peak_time=dt;
        for(unsigned j=0;j<HALF_FRAMES*2;j++){unsigned v=(unsigned)abs(out[j]);if(v>peak)peak=v;clip+=v>=32767u;}
        acid_blocks+=trk[2].v[0].active && trk[6].v[0].active;
        for(unsigned j=0;j<2;j++){track_t *t=&trk[j?6:2];assert(t->p[P_AMORPH]==64);uint32_t m=t->v[0].k[0];if(m<lo[j])lo[j]=m;if(m>hi[j])hi[j]=m;}
    }
    double elapsed=seconds()-start;assert(acid_blocks>4000 && !clip && peak<=32767 && lo[0]<hi[0] && lo[1]<hi[1]);
    seq_stop();transport_req=0;
    printf("Q host PASS: 6 drums + 2 ACID, one Morph/one PWM, both dual-LFO/four-slot A.MOR, CUT120/RES120/ENV127/ACC127, 150BPM: mean %.3fus/256 frames, max %.3fus, peak %u, clips%u, both ACID active %u/4096; no FM-1 realtime claim.\n",elapsed*1e6/4096,peak_time*1e6,peak,clip,acid_blocks);
}
static void alias_dump(const char *path)
{
    FILE *f=fopen(path,"wb");assert(f);
    unsigned bins[]={13,97,389},values[]={0,64,127};
    for(unsigned k=0;k<3;k++)for(unsigned m=0;m<3;m++){
        uint32_t inc=bins[k]<<19,ph=0,duty=acid_pwm_duty(values[m]*65536u);int gain=acid_pwm_gain(values[m]*65536u);
        for(unsigned i=0;i<8192;i++){
            double d=(double)duty/4294967296.0,naive=((ph+duty)<ph?2*(1-d):-2*d)*gain/32768.0;
            double filtered=(double)acid_pwm_wave(ph,inc,duty,inc,gain)/QONE;
            float pair[2]={(float)naive,(float)filtered};fwrite(pair,sizeof pair,1,f);ph+=inc;
        }
    }
    fclose(f);
}
int main(int argc,char **argv)
{
    setvbuf(stdout,0,_IOLBF,0);assert(argc>=2);
    if(!strcmp(argv[1],"config"))configuration();
    if(!strcmp(argv[1],"lfo"))lfo_pwm();
    if(!strcmp(argv[1],"ui"))ui_checks();
    if(!strcmp(argv[1],"storage"))persistence();
    if(!strcmp(argv[1],"audio"))audio_measurements();
    if(!strcmp(argv[1],"cost"))performance();
    if(!strcmp(argv[1],"alias")){assert(argc==3);alias_dump(argv[2]);}
    return 0;
}
