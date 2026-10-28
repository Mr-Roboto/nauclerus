/* SPDX-License-Identifier: GPL-3.0-only */
/* RC9 integration: actual UI dispatch, production sequencer and flash harness. */
#define main rc9_boot_main
#include "boot_test.c"
#undef main
#include <assert.h>
static void input(void){ui_input();ui_leds();}
static void tap(unsigned b){press(b);input();release_all();input();}
static void knob(unsigned k,int n){if(n){turn(EN_K1+k,n);input();}}
static void fresh(void){fresh_flash();wipe_ram();ui_host_init();drum_tracks_init();home_screen();}
static void hold(unsigned b,unsigned ms)
{
    press(b);input();host_ticks+=ms*1000u*FM1_TICKS_PER_US;input();release_all();input();
}
static void next(track_t *t)
{
    seq_tick(t,step_samples(t,div_period((uint32_t)t->p[P_SDIV],t->seq_rem),t->seq_cnt)-t->seq_pos);
}
static void home_paths(void)
{
    fresh();home_tracks();assert(!ui.home && !ui.hmode);
    tap(B_GLO);assert(ui.menu==1 && !ui.mute);
    ui.menu_sel=MI_SPKEQ;uint32_t eq=settings.lowcut;
    press(B_HOME);input();press(B_OCTUP);fm1_in.buttons&=~(1u<<panel.btn[B_HOME]);input();
    assert(!ui.menu && ui.home && ui.hmode==1 && settings.lowcut==eq);
    assert(!ui.menu_sel && !ui.about_top);release_all();input();
    tap(B_GLO);assert(ui.menu==1);tap(B_GLO);assert(!ui.menu);
    tap(B_GLO);ui.menu_sel=MI_ABOUT;tap(B_OCTUP);assert(ui.menu==2);
    tap(B_HOME);assert(!ui.menu && ui.home && ui.hmode==1);
    hold(B_HOME,701);assert(!ui.home && cur_fam()==FAM_GLO && ui.page==page_first(FAM_GLO));
    for(unsigned n=1;n<=6;n++) {tap(B_HOME);assert(!ui.home && cur_fam()==FAM_GLO && ui.page==page_first(FAM_GLO)+n%3);}
    hold(B_HOME,701);assert(ui.home && ui.hmode==1 && !ui.menu);
    input();assert(ui.home); /* long release did not cycle/toggle */
    ui.confirm=1;step_t saved[NSTEP];memcpy(saved,trk[0].step,sizeof saved);
    press(B_HOME);input();press(B_OCTUP);fm1_in.buttons&=~(1u<<panel.btn[B_HOME]);input();
    assert(ui.home && !ui.confirm && !memcmp(saved,trk[0].step,sizeof saved));release_all();input();
    home_tracks();name_open(NK_RENAME,0);assert(name_on());tap(B_HOME);assert(!name_on() && ui.home && ui.hmode==1);
    home_tracks();preset_list_open();assert(pl_on());tap(B_HOME);assert(!pl_on() && ui.home && ui.hmode==1);
    home_tracks();ui.pr.pop=1;tap(B_HOME);assert(!pp_on() && ui.home && ui.hmode==1);
    ml_toggle();assert(ui.ml.on);tap(B_HOME);assert(!ui.ml.on && ui.home);
    press(B_GLO);input();host_ticks+=401u*1000u*FM1_TICKS_PER_US;input();assert(ui.mute);
    press(B_HOME);input();fm1_in.buttons&=~(1u<<panel.btn[B_HOME]);input();assert(!ui.mute && ui.home);
    input();assert(!ui.mute && !ui.menu);release_all();input();assert(!ui.menu);
    settings.perfpage=1;press(B_FX);input();host_ticks+=701u*1000u*FM1_TICKS_PER_US;input();assert(ui.layer && ui.pg_open);
    press(B_HOME);input();fm1_in.buttons&=~(1u<<panel.btn[B_HOME]);input();assert(ui.home && !ui.layer && !ui.pg_open);
    input();assert(!ui.layer && !ui.pg_open);release_all();input();assert(ui.home && !ui.layer);
    puts("PASS HOME: GLO/ABOUT/overlays escape to real HOME, no selected confirmation action, Settings short cycles/long exits, long-release suppression, GLO MUTE and PERF close safely");
}
static void acid_layout(void)
{
    static const uint8_t want[15]={P_MODEL,P_E6,P_E2,P_E3,P_E0,P_E1,P_E4,P_LEVEL,P_E7,P_E5,P_PAN,P_AMORPH,P_CHOKE,P_PRIO,P_NOTE};
    fresh();drum_set_model(&trk[2],DM_ACID);track_select(2);tap(B_EDIT);
    uint8_t ids[16];assert(snd_list(TSEL,ids)==15 && !memcmp(ids,want,sizeof want));
    uint32_t position;assert(fam_pages(FAM_SND,&position)==4);
    for(unsigned page=0;page<4;page++) {
        assert(cur_fam()==FAM_SND && ui.page==page_first(FAM_SND)+page && !page_hidden(ui.page));
        for(unsigned k=0;k<4;k++) {
            unsigned i=page*4+k;int16_t *ptr;
            const param_desc_t *d=page_desc(cur_page(),k,&ptr);
            if(i==15)assert(page_id(cur_page(),k)==0xFF && !d && !ptr);
            else assert(page_id(cur_page(),k)==want[i] && d && ptr==&TSEL->p[want[i]]);
        }
        if(page<3){turn(EN_SELECT,1);input();}
    }
    track_t before=*TSEL;knob(3,1);knob(3,-1);assert(!memcmp(&before,TSEL,sizeof before));
    turn(EN_SELECT,1);input();assert(ui.page==page_first(FAM_SND)+3);
    tap(B_EDIT);assert(ui.page==page_first(FAM_SND));
    knob(1,1);assert(TSEL->p[P_E6]==3); /* K2 WAVE, not a separate FILTER control */
    turn(EN_SELECT,1);input();int level=TSEL->p[P_LEVEL];knob(3,1);assert(TSEL->p[P_LEVEL]==level+1);
    turn(EN_SELECT,1);input();int morph=TSEL->p[P_AMORPH];knob(3,1);assert(TSEL->p[P_AMORPH]==morph+1);
    for(unsigned model=0;model<NMODELS;model++)if(model!=DM_ACID) {
        track_t probe=*TSEL;probe.p[P_MODEL]=(int16_t)model;uint8_t expected[16];unsigned n=0;
        expected[n++]=P_MODEL;
        for(unsigned j=0;j<8;j++)if(j<4 || (DMODELS[model].edit[j].label && DMODELS[model].edit[j].label[0]!='-'))expected[n++]=(uint8_t)(P_E0+j);
        expected[n++]=P_LEVEL;expected[n++]=P_PAN;expected[n++]=P_NOTE;expected[n++]=P_CHOKE;expected[n++]=P_PRIO;
        assert(snd_list(&probe,ids)==n && !memcmp(ids,expected,n));
    }
    puts("PASS ACID EDIT: exact four-page/15-parameter layout, empty K4 inactive, real WAVE/LEVEL/MORPH knobs and navigation; every non-ACID list unchanged");
}
static void tb_priority(void)
{
    for(unsigned keys_on=0;keys_on<2;keys_on++)for(unsigned running=0;running<2;running++)for(unsigned page=0;page<2;page++) {
        fresh();drum_set_model(&trk[2],DM_ACID);drum_set_model(&trk[6],DM_ACID);track_select(2);tap(B_ARP);
        if(page)tap(B_ARP);assert(cur_fam()==FAM_TB3PO && cur_page()->id[0]==4*page);
        if(keys_on)tap(B_SCL);assert(ui.keys==keys_on);song.octave=0;
        trk[2].p[P_SLEN]=18;trk[2].p[P_SRC]=0;
        step_t other[NSTEP];memcpy(other,trk[6].step,sizeof other);
        if(running){seq_start();seq_tick(&trk[2],0);}
        fm1_ms=1234+100*page+10*running+keys_on;
        press(B_OCTUP);input();uint32_t seed=tb_live[2].cfg.seed;
        assert(str_eq(ui.msg,"NEW LINE") && song.octave==0 && ui.keys==keys_on);
        fm1_ms++;input();release_all();input();assert(tb_live[2].cfg.seed==seed && song.octave==0);
        if(running){assert(tb_pending_valid[2]);next(&trk[2]);assert(!tb_pending_valid[2]);}
        bpart_t generated={0};generated.len=18;generated.gen=tb_live[2].cfg;tb3po_generate(&generated);
        for(unsigned j=0;j<NSTEP;j++)assert(trk[2].step[j].nt==(int)generated.step[j].note-60 && trk[2].step[j].on==(bstep_gate(&generated.step[j])==G_NOTE));
        step_t expected[NSTEP];memcpy(expected,trk[2].step,sizeof expected);tb_state_t g=tb_live[2];uint32_t r=g.cfg.seed^fm1_ms;
        tb_mutate_steps(expected,&trk[2],&g,&r);
        press(B_OCTDN);input();assert(str_eq(ui.msg,"MUTATED") || str_eq(ui.msg,"NO CHANGE"));
        fm1_ms++;input();release_all();input();assert(song.octave==0 && tb_live[2].cfg.seed==seed && ui.keys==keys_on);
        if(running){assert(tb_pending_valid[2]);next(&trk[2]);seq_stop();transport_req=0;}
        assert(!memcmp(expected,trk[2].step,sizeof expected) && !memcmp(other,trk[6].step,sizeof other));
        if(!keys_on)tap(B_SCL);tap(B_EDIT);assert(cur_fam()==FAM_SND && ui.keys);
        tap(B_OCTUP);assert(song.octave==1);tap(B_OCTDN);assert(song.octave==0);
        tap(B_ARP);uint32_t keep=tb_live[2].cfg.seed;
        ui.confirm=1;tap(B_OCTDN);assert(!ui.confirm && tb_live[2].cfg.seed==keep);
        ui.ml.on=1;tap(B_OCTUP);assert(!ui.ml.on && tb_live[2].cfg.seed==keep && !song.octave);
    }
    puts("PASS TB-3PO: NEW/MUTATE once per press, both pages, KEYS on/off, stopped/running and atomic step handoff; other track/KEYS flag retained, octave behavior elsewhere and dialog/MIDI-LEARN priority preserved");
}
/* Original attribution list from the verified RC8 package source. */
static const char *const rc8_credits[] = {
    "DRUM MACHINE ON FELUCCA", FELUCCA_VERSION, "FORK: DEADACTIVE",
    "FELUCCA: LEO KUROSHITA, H\xDCGELTON INSTRUMENTS", "GITHUB.COM/HUGELTON/FELUCCA",
    "MUTABLE INSTRUMENTS, EMILIE GILLET: PLAITS + STMLIB (DRUM ALGORITHMS, MIT), GRIDS (PATTERN ENGINE, GPL-3.0+), "
    "STREAMS (COMPRESSOR, MIT)",
    "DAISYSP, ELECTROSMITH + EMILIE GILLET: MODAL / MEMBRANE (SKIN, RESON; MIT)",
    "SAMPLES: VERSILIAN STUDIOS VSCO-2 CE / VCSL (CC0) + H\xDCGELTON SAMPLE PACK",
    "PIXEL FONT: DEADACTIVE",
    "MD VOICES MEASURED FROM JOE LANDERS' MACHINEDRUM EMULATOR, A FORK OF GEARMULATOR BY THE USUAL SUSPECTS (DSP56300 "
    "PROJECT): MEASUREMENTS ONLY, NO CODE OR DATA FROM IT",
    "MACHINEDRUM IS A TRADEMARK OF ELEKTRON. NOT AFFILIATED WITH ELEKTRON, JOE LANDERS OR THE USUAL SUSPECTS",
    "JIELI AC79 SDK (APACHE-2.0) IN THE PACKAGE",
    "RECOVERY TOOL: JL-UBOOT-TOOL BY KAGAIMIQ (ANDREY GRIGORYEV), MIT",
    "WEB FONT: TERMINUS (DIMITAR ZHEKOV, OFL)", "WEB ICONS: FUKIAI (H\xDCGELTON INSTRUMENTS, MIT)",
    "BOOT SCREEN STUDY: UNIFRAKTUR FONTS (J. 'MACH' WUST, PETER WIEGEL, OFL)",
    "GPL-3.0, NO WARRANTY"};
static void credits(void)
{
    fresh();unsigned count=sizeof AB/sizeof AB[0];
    assert(count==sizeof rc8_credits/sizeof rc8_credits[0]+1);
    for(unsigned j=0;j<count-1;j++)assert(str_eq(rc8_credits[j],AB[j<3?j:j+1]));
    assert(str_eq(AB[3],"NAUCLERUS: MR. ROBOTO - DEVELOPMENT, MODIFICATIONS + REMIX"));
    about_wrap();assert(ab_lines<AB_MAXL);
    unsigned visible=0;
    for(unsigned j=0;j<ab_lines;j++) {
        char b[64];ab_str(b,j);
        assert((ab_line[j].at ? AB_X+AB_IND:AB_X)+md_text_w(&MD_F3,2,b)<=AB_R);
        if(ab_line[j].c==3){assert(j<AB_ROWS);visible++;}
    }
    assert(visible==3 && ab_line[ab_lines-1].c==count-1 && str_eq(AB[count-1],"GPL-3.0, NO WARRANTY"));
    tap(B_GLO);ui.menu_sel=MI_ABOUT;tap(B_OCTUP);assert(ui.menu==2 && !ui.about_top);ui_draw();
    turn(EN_SELECT,1);input();assert(ui.about_top==1);tap(B_OCTDN);assert(ui.menu==1);
    printf("PASS ABOUT: Mr. Roboto visible on first view in three rows; all RC8 author/license entries preserved; %u/%u wrapped rows, display bounds, scroll and BACK intact\n",ab_lines,AB_MAXL);
}
int main(void)
{
    setvbuf(stdout,0,_IOLBF,0);
    home_paths();acid_layout();tb_priority();credits();return 0;
}
