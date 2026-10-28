/* SPDX-License-Identifier: GPL-3.0-only */
/* Real panel/UI, production transport, project flash and boot. No test-only
 * Remix setter is enabled: activation goes through the shipped confirm/enable paths. */
#define main remix_original_boot_main
#include "boot_test.c"
#undef main
#include <assert.h>

static uint32_t duration(track_t *t) {return step_samples(t,div_period((uint32_t)t->p[P_SDIV],t->seq_rem),t->seq_cnt);}
static void tick_next(track_t *t) {seq_tick(t,duration(t)-t->seq_pos);}
static void input(void) {ui_input();ui_leds();}
static void tap_button(unsigned b) {press(b);input();release_all();input();}
static void knob(unsigned k,int d) {if(d){turn(EN_K1+k,d);input();}}
static void open_remix(void)
{
    for(unsigned i=0;i<NPAGES && (ui.home || cur_page()->graph!=GR_REMIX);i++)tap_button(B_SEQ);
    assert(!ui.home && cur_page()->graph==GR_REMIX);
}
static void fresh(void)
{
    fresh_flash();wipe_ram();ui_host_init();
    memset(proj_slot,0,sizeof proj_slot);memset(lk_slot,0,sizeof lk_slot);
    for(unsigned k=0;k<NTRK;k++) {
        old_set_model(&trk[k],DM_K808);trk[k].p[P_SLEN]=8;trk[k].p[P_SDIV]=2;trk[k].p[P_SRC]=0;
        trk[k].p[P_LEVEL]=90;trk[k].p[P_PAN]=-7;
        memset(trk[k].step,0,sizeof trk[k].step);
    }
    open_remix();
}
static void select_track(unsigned k)
{
    while(song.sel!=k){turn(EN_ALGO,song.sel<k ? 1 : -1);input();}
}
static void select_pattern(unsigned bank,unsigned pattern)
{
    remix_draft_sync(); unsigned scope=seq_remix_scope(song.sel);
    knob(0,(int)bank-(ui.remix.selection[scope]>>4));
    knob(1,(int)pattern-(ui.remix.selection[scope]&15));
}
static unsigned source(unsigned state,unsigned chronological)
{
    return state&256u ? (unsigned)remix_source_step((state>>4)&15u,state&15u,chronological) : chronological;
}
static void routing(void)
{
    fresh();
    step_t before[NTRK][NSTEP];
    for(unsigned k=0;k<NTRK;k++)memcpy(before[k],trk[k].step,sizeof before[k]);
    assert(seq_remix_apply(0,258));
    seq_start();
    for(unsigned k=0;k<NTRK;k++) {
        seq_tick(&trk[k],0);
        assert(seq_remix_scope(k)==REMIX_DRUM && seq_remix_effective(k)==258);
        for(unsigned c=0;c<64;c++)assert(seq_source_step(&trk[k],c)==source(258,c));
    }
    unsigned acid=preset_first_of(DM_ACID);
    assert(!preset_load(&trk[1],acid) && !preset_load(&trk[6],acid));
    assert(seq_remix_apply(1,260));tick_next(&trk[1]);
    assert(seq_remix_effective(1)==260 && seq_remix_effective(6)==0);
    assert(seq_remix_apply(6,287));tick_next(&trk[6]);
    assert(seq_remix_effective(1)==260 && seq_remix_effective(6)==287);
    assert(preset_load(&trk[0],acid)==-1 && acid_tracks(0)==2);
    assert(seq_remix_apply(3,293));
    for(unsigned k=0;k<NTRK;k++)if(k!=1 && k!=6) {
        uint32_t count=trk[k].seq_cnt;tick_next(&trk[k]);
        assert(trk[k].seq_cnt==count+1 && seq_remix_effective(k)==293);
    }
    assert(seq_remix_effective(1)==260 && seq_remix_effective(6)==287);
    select_track(1);trk[1].gate_n=GATE_HELD;trk[1].slide_in=1;
    model_step(1);                         /* real normal model editor */
    assert(trk[1].p[P_MODEL]!=DM_ACID && !trk[1].gate_n && !trk[1].slide_in);
    assert(seq_remix_effective(1)==293 && seq_remix_config[1]==260);
    assert(seq_remix_apply(3,37));tick_next(&trk[1]);
    assert(seq_remix_effective(1)==37 && seq_source_step(&trk[1],7)==7);
    model_step(-1);
    assert(trk[1].p[P_MODEL]==DM_ACID && seq_remix_effective(1)==260);
    assert(!preset_load(&trk[1],preset_first_of(DM_K808)));
    assert(seq_remix_effective(1)==37 && seq_remix_config[1]==260);
    assert(!preset_load(&trk[1],acid) && seq_remix_effective(1)==260);
    assert(seq_remix_apply(0,258));
    trk[2].p[P_SRC]=1;tick_next(&trk[2]);
    assert(trk[2].p[P_SRC]==1 && seq_source_step(&trk[2],3)==3);
    for(unsigned k=0;k<NTRK;k++)assert(!memcmp(before[k],trk[k].step,sizeof before[k]));
    seq_stop();
    puts("A PASS: shared Drum, independent Bass on arbitrary tracks, model/preset routing, two-ACID cap, timing and Grids bypass");
}
static void open_page(unsigned family,unsigned offset)
{
    ui.home=0;ui.page=(uint8_t)(page_first(family)+offset);page_entered();
}
static void grids_setup(unsigned mode)
{
    song.g[G_GMODE]=(int16_t)mode;song.g[G_GX]=49;song.g[G_GY]=83;song.g[G_GCHAOS]=101;
    song.g[G_GFILL1]=65;song.g[G_GFILL2]=84;song.g[G_GFILL3]=110;
    song.g[G_GLEN1]=3;song.g[G_GLEN2]=5;song.g[G_GLEN3]=11;
    song.g[G_SWING]=47;
}
static void grids_write_check(void)
{
    static track_t before[NTRK];
    for(unsigned mode=0;mode<2;mode++) {
        fresh();grids_setup(mode);
        trk[0].p[P_SRC]=1;trk[1].p[P_SRC]=2;trk[2].p[P_SRC]=3;trk[3].p[P_SRC]=1;
        assert(!preset_load(&trk[5],preset_first_of(DM_ACID)));trk[5].p[P_SRC]=3;
        for(unsigned k=0;k<NTRK;k++) {
            trk[k].p[P_SSWING]=(int16_t)(k+13);
            for(unsigned n=0;n<64;n++)trk[k].step[n]=(step_t){.on=1,.acc=1,.cond=4,.rat=3,.slide=1,.tie=1,.nt=12,.rsv=7};
        }
        assert(seq_remix_apply(0,258));
        motion_add(0,5,P_LEVEL,44);lock_set(0,5,P_LEVEL,101);
        lk_bank_t bank=mo.b;chain.cfg.count=1;chain.cfg.row[0]=(chain_row_t){2,3};
        chain_config_t config=chain.cfg;int16_t globals[G_COUNT];memcpy(globals,song.g,sizeof globals);
        uint16_t remix[REMIX_SCOPES];for(unsigned k=0;k<REMIX_SCOPES;k++)remix[k]=seq_remix_config[k];
        uint8_t expected[64];grids_start();
        for(unsigned n=0;n<64;n++)expected[n]=(uint8_t)grids_step();
        for(unsigned n=0;n<11;n++)grids_step();
        uint8_t runtime[sizeof grids],clock[sizeof gclk];
        memcpy(runtime,&grids,sizeof grids);memcpy(clock,&gclk,sizeof gclk);
        memcpy(before,trk,sizeof before);grids_write_steps();
        assert(str_eq(ui.msg,"GRIDS WRITTEN"));
        assert(!memcmp(runtime,&grids,sizeof grids) && !memcmp(clock,&gclk,sizeof gclk));
        assert(!memcmp(globals,song.g,sizeof globals) && !memcmp(&config,&chain.cfg,sizeof config));
        assert(!memcmp(&bank,&mo.b,sizeof bank));
        for(unsigned k=0;k<REMIX_SCOPES;k++)assert(seq_remix_config[k]==remix[k]);
        for(unsigned k=0;k<NTRK;k++) {
            track_t want=before[k];
            if(k<4) {
                unsigned ch=(unsigned)before[k].p[P_SRC]-1;
                want.p[P_SLEN]=64;want.p[P_SDIV]=3;want.p[P_SRC]=0;
                memset(want.step,0,sizeof want.step);
                for(unsigned n=0;n<64;n++) {
                    want.step[n].on=(expected[n]>>ch)&1u;
                    want.step[n].acc=(expected[n]>>(ch+3))&1u;
                }
            }
            assert(!memcmp(&want,&trk[k],sizeof want));
        }
        /* Repeat from a different live phase: the snapshot is deterministic. */
        step_t first[4][NSTEP];for(unsigned k=0;k<4;k++) {
            memcpy(first[k],trk[k].step,sizeof first[k]);trk[k].p[P_SRC]=before[k].p[P_SRC];
        }
        for(unsigned n=0;n<7;n++)grids_step();memcpy(runtime,&grids,sizeof grids);
        grids_write_steps();assert(!memcmp(runtime,&grids,sizeof grids));
        for(unsigned k=0;k<4;k++)assert(!memcmp(first[k],trk[k].step,sizeof first[k]));
        trk[0].p[P_SRC]=1;memcpy(before,trk,sizeof before);
        song.playing=1;grids_write_steps();assert(str_eq(ui.msg,"STOP TO WRITE") && !memcmp(before,trk,sizeof before));
        song.playing=0;transport_req=1;grids_write_steps();
        assert(str_eq(ui.msg,"STOP TO WRITE") && !memcmp(before,trk,sizeof before));transport_req=0;
        chain.armed=1;grids_write_steps();assert(!memcmp(before,trk,sizeof before));chain.armed=0;
        for(unsigned k=0;k<NTRK;k++)if(k!=5)trk[k].p[P_SRC]=0;
        memcpy(before,trk,sizeof before);grids_write_steps();
        assert(str_eq(ui.msg,"NO GRIDS TRACKS") && !memcmp(before,trk,sizeof before));
    }
    puts("A PASS: MAP/EUCLID 64-step triggers/accents, assigned channels/duplicate tracks, defaults, deterministic state preservation, unrelated tracks/settings and stopped-only safeguards");
}
static void grids_remix_check(void)
{
    fresh();grids_setup(0);trk[0].p[P_SRC]=1;
    assert(seq_remix_apply(0,258));grids_write_steps();
    assert(trk[0].p[P_SRC]==0 && trk[0].p[P_SLEN]==64 && trk[0].p[P_SDIV]==3 && seq_remix_config[REMIX_DRUM]==258);
    step_t stored[NSTEP];memcpy(stored,trk[0].step,sizeof stored);
    seq_start();seq_tick(&trk[0],0);unsigned changed=0;
    for(unsigned n=0;n<64;n++) {
        unsigned mapped=(unsigned)remix_source_step(0,2,n);
        assert(seq_source_step(&trk[0],n)==mapped);
        assert(seq_steps(&trk[0])[mapped].on==stored[mapped].on && seq_steps(&trk[0])[mapped].acc==stored[mapped].acc);
        changed+=mapped!=n;
    }
    assert(changed && !memcmp(stored,trk[0].step,sizeof stored));
    assert(seq_remix_apply(0,2));tick_next(&trk[0]);
    for(unsigned n=0;n<64;n++)assert(seq_source_step(&trk[0],n)==n);
    assert(!memcmp(stored,trk[0].step,sizeof stored));seq_stop();
    puts("B PASS: written SRC=STEPS uses existing global Drum mapping across four blocks; OFF restores chronology without changing data");
}
static void controls(void)
{
    fresh();select_pattern(1,2);knob(2,1);
    assert(seq_remix_config[REMIX_DRUM]==256 && ui.remix.selection[REMIX_DRUM]==18);
    knob(3,1);assert(seq_remix_config[REMIX_DRUM]==274 && !(ui.remix.dirty&(1u<<REMIX_DRUM)));
    select_pattern(2,3);knob(2,-1);knob(2,1);
    assert(seq_remix_config[REMIX_DRUM]==274 && ui.remix.selection[REMIX_DRUM]==35);
    tap_button(B_OCTUP);assert(seq_remix_config[REMIX_DRUM]==291);
    seq_start();seq_tick(&trk[0],0);select_pattern(3,4);knob(3,1);
    assert(seq_remix_pending[REMIX_DRUM]==(512u|308u) && seq_remix_effective(0)==291);
    tick_next(&trk[0]);assert(seq_remix_effective(0)==308);
    select_pattern(4,5);tap_button(B_OCTUP);
    assert(seq_remix_pending[REMIX_DRUM]==(512u|325u));tick_next(&trk[0]);assert(seq_remix_effective(0)==325);
    seq_stop();transport_req=0;
    /* Both ARP pages dispatch the same WRITE action. */
    for(unsigned page=0;page<2;page++) {
        open_page(FAM_GRIDS,page);trk[0].p[P_SRC]=1;tap_button(B_OCTUP);
        assert(trk[0].p[P_SRC]==0 && str_eq(ui.msg,"GRIDS WRITTEN"));
    }
    open_page(FAM_SAVE,0);song.g[G_SLOT]=1;ui.arm=0;trk[0].p[P_LEVEL]=70;
    tap_button(B_OCTUP);assert(ui.arm==G_SAVE && !project_used(0));
    tap_button(B_OCTUP);assert(project_used(0) && proj_slot[0].t[0].p[P_LEVEL]==70);
    trk[0].p[P_LEVEL]=11;tap_button(B_OCTDN);assert(ui.arm==G_LOAD && trk[0].p[P_LEVEL]==11);
    tap_button(B_OCTDN);assert(trk[0].p[P_LEVEL]==70 && str_eq(ui.msg,"LOADED"));transport_req=0;
    trk[0].p[P_LEVEL]=22;tap_button(B_OCTUP);assert(proj_slot[0].t[0].p[P_LEVEL]==70);
    knob(3,1);assert(proj_slot[0].t[0].p[P_LEVEL]==22); /* shared arm, existing K4 SAVE */
    trk[0].p[P_LEVEL]=33;knob(2,1);assert(trk[0].p[P_LEVEL]==33 && ui.arm==G_LOAD);
    tap_button(B_OCTDN);assert(trk[0].p[P_LEVEL]==22);transport_req=0;
    project_t saved=proj_slot[0];seq_start();tap_button(B_OCTUP);tap_button(B_OCTUP);
    assert(str_eq(ui.msg,"STOP TO SAVE") && !memcmp(&saved,&proj_slot[0],sizeof saved));seq_stop();transport_req=0;
    chain.armed=1;tap_button(B_OCTDN);tap_button(B_OCTDN);
    assert(str_eq(ui.msg,"STOP TO LOAD") && !memcmp(&saved,&proj_slot[0],sizeof saved));chain.armed=0;
    ui.keys=ui.env=0;ui.env_t0=0;open_page(FAM_SEQ,0);trk[0].p[P_SLEN]=64;ui.bank=0;
    tap_button(B_OCTUP);assert(ui.bank==1);open_page(FAM_SND,0);tap_button(B_OCTUP);assert(cur_fam()==FAM_LAY);
    puts("C PASS: Remix K4/OCT+ share stopped/queued confirmation, K3 keeps draft separate; ARP WRITE dispatch and PROJECT shortcuts retain two-action confirmation and transport/Song safeguards");
}
static void persistence(void)
{
    fresh();
    for(unsigned k=0;k<NTRK;k++) {
        assert(!preset_load(&trk[k],preset_first_of(DM_ACID)));
        assert(seq_remix_apply(k,(k&1u ? 256u : 0u)|(k*29u+7u)));
        assert(!preset_load(&trk[k],preset_first_of(DM_K808)));
        trk[k].p[P_SLEN]=1;
    }
    assert(!preset_load(&trk[1],preset_first_of(DM_ACID)) && !preset_load(&trk[6],preset_first_of(DM_ACID)));
    motion_add(0,0,P_LEVEL,40);lock_set(0,0,P_LEVEL,100);mo.b.on=255;
    uint16_t expected[4][REMIX_SCOPES];lk_bank_t bank=mo.b;
    for(unsigned slot=0;slot<4;slot++) {
        for(unsigned k=0;k<REMIX_SCOPES;k++)
            expected[slot][k]=(uint16_t)(seq_remix_config[k]=((slot+k)&1u ? 256u : 0u)|((slot*61u+k*29u+7u)&255u));
        project_save(slot);assert(proj_ok(&proj_slot[slot]) && str_eq(ui.msg,"SAVED"));
        assert(proj_slot[slot].rsv_mo[8]==2 && sizeof(project_t)==3840 && sizeof proj_slot==15360);
    }
    for(unsigned slot=0;slot<4;slot++) {
        seq_remix_defaults();assert(proj_load(slot,1));transport_req=0;
        for(unsigned k=0;k<REMIX_SCOPES;k++)assert(seq_remix_config[k]==expected[slot][k]);
        assert(bank_events_eq(&bank,&mo.b));
    }
    power_on(0);                         /* last saved slot restored through actual boot path */
    for(unsigned k=0;k<REMIX_SCOPES;k++)assert(seq_remix_config[k]==expected[3][k]);
    uint16_t live[REMIX_SCOPES];for(unsigned k=0;k<REMIX_SCOPES;k++)live[k]=seq_remix_config[k];
    chain.cfg.count=4;chain.cfg.loop=1;
    for(unsigned row=0;row<4;row++)chain.cfg.row[row]=(chain_row_t){row,1};
    transport_req=0;assert(chain_prepare()==0);seq_start();
    int32_t pcm[HALF_FRAMES*2];unsigned transitions=0, prior=chain.slot;
    for(unsigned block=0;block<1000 && transitions<4;block++) {
        mix_block(pcm,HALF_FRAMES);
        for(unsigned k=0;k<NTRK;k++) {
            unsigned scope=seq_remix_scope(k);
            assert(seq_remix_effective(k)==expected[chain.slot][scope]);
            assert(seq_source_step(&trk[k],0)==source(expected[chain.slot][scope],0));
        }
        assert(mo.src==chain.src[chain.slot].m);
        if(chain.slot!=prior){transitions++;prior=chain.slot;}
    }
    assert(transitions==4);seq_stop();transport_req=0;
    for(unsigned k=0;k<REMIX_SCOPES;k++)assert(seq_remix_config[k]==live[k]);
    project_t legacy=proj_slot[0];memset(legacy.rsv_mo,0,sizeof legacy.rsv_mo);
    memcpy(legacy.rsv_mo,PROJ_REMIX_HEAD,12);legacy.rsv_mo[8]=1;legacy.rsv_mo[9]=16;
    for(unsigned k=0;k<NTRK;k++) {
        unsigned code=k==1 ? 256u : k==6 ? 1u : k*17u;
        legacy.rsv_mo[12+2*k]=(uint8_t)code;legacy.rsv_mo[13+2*k]=(uint8_t)(code>>8);
    }
    uint32_t hash=proj_hash(legacy.rsv_mo,28);
    for(unsigned j=0;j<4;j++)legacy.rsv_mo[28+j]=(uint8_t)(hash>>(8*j));
    legacy.sum=proj_sum(&legacy);proj_slot[0]=legacy;assert(proj_load(0,1));transport_req=0;
    for(unsigned k=0;k<NTRK;k++)assert(seq_remix_config[k]==seq_remix_state(k==1 ? 256u : k==6 ? 1u : k*17u));
    assert(seq_remix_config[REMIX_DRUM]==0); /* never infer global Drum from old per-track data */
    for(unsigned kind=0;kind<3;kind++) {
        project_t p=legacy;
        if(kind==0)memset(p.rsv_mo,0,sizeof p.rsv_mo);
        if(kind==1)p.rsv_mo[8]=9;
        if(kind==2)p.rsv_mo[28]^=1;
        p.sum=proj_sum(&p);proj_slot[0]=p;assert(proj_load(0,1));transport_req=0;
        for(unsigned k=0;k<REMIX_SCOPES;k++)assert(!seq_remix_config[k]);
    }
    puts("C PASS: global/Bass retained ON and OFF selections, A-D SAVE/LOAD/boot/Song, lock bank, fixed layout and v1/invalid legacy defaults");
}
static int same_track_content(const track_t *a,const track_t *b)
{
    for(unsigned k=0;k<NTRK;k++)
        if(memcmp(a[k].p,b[k].p,sizeof a[k].p) || memcmp(a[k].step,b[k].step,sizeof a[k].step) || a[k].pre!=b[k].pre || a[k].pre_ed!=b[k].pre_ed) return 0;
    return 1;
}
static void quick_assign_check(void)
{
    static track_t expected[NTRK];
    fresh();
    for(unsigned k=0;k<NTRK;k++) {
        trk[k].p[P_SRC]=(int16_t)((k+2u)%4u);
        trk[k].step[k].on=1;trk[k].step[k].acc=1;
    }
    assert(!preset_load(&trk[0],preset_first_of(DM_ACID)));
    uint16_t remix[REMIX_SCOPES];memcpy(remix,(const void *)seq_remix_config,sizeof remix);
    select_track(1); /* ARP routes an ACID selection to TB-3PO, not GRIDS. */
    open_page(FAM_GRIDS,1);memcpy(expected,trk,sizeof expected);
    knob(3,-1);assert(same_track_content(expected,trk));
    for(unsigned k=0;k<3;k++)expected[k].p[P_SRC]=(int16_t)(k+1);
    knob(3,1);
    assert(str_eq(ui.msg,"GRIDS T1-3 ASSIGNED") && same_track_content(expected,trk));
    knob(3,1);assert(same_track_content(expected,trk));
    assert(!memcmp(remix,(const void *)seq_remix_config,sizeof remix));
    for(unsigned k=0;k<3;k++)trk[k].p[P_SRC]=0;
    memcpy(expected,trk,sizeof expected);
    open_page(FAM_GRIDS,0);knob(3,1);
    assert(same_track_content(expected,trk));
    open_page(FAM_GRIDS,1);
    chain.armed=1;knob(3,1);
    assert(str_eq(ui.msg,"STOP TO EDIT") && same_track_content(expected,trk));
    chain.armed=0;chain.running=1;knob(3,1);
    assert(str_eq(ui.msg,"STOP TO EDIT") && same_track_content(expected,trk));
    chain.running=0;
    puts("A PASS: page-2 CW assignment, CCW/no other-page assignment, idempotence, Song restrictions; only T1-3 SRC changed, all steps/models/other parameters and Remix unchanged");
}
static void remix_led_check(void)
{
    fresh();trk[0].p[P_SLEN]=64;ui.bank=1;ui.env=1;
    assert(oct_leds()==0);
    select_pattern(1,2);assert(oct_leds()==OCT_BR_UP);
    knob(2,1);assert(seq_remix_config[REMIX_DRUM]==256 && oct_leds()==OCT_BR_UP);
    tap_button(B_OCTUP);
    assert(seq_remix_config[REMIX_DRUM]==274 && oct_leds()==0);
    select_pattern(2,3);assert(oct_leds()==OCT_BR_UP);
    knob(3,1);assert(seq_remix_config[REMIX_DRUM]==291 && oct_leds()==0);
    ui.keys=1;select_pattern(3,4);assert(oct_leds()==OCT_BR_UP);
    tap_button(B_OCTUP);assert(seq_remix_config[REMIX_DRUM]==308 && oct_leds()==0);ui.keys=0;
    seq_start();seq_tick(&trk[0],0);select_pattern(4,5);
    assert(oct_leds()==OCT_BR_UP);knob(3,1);
    assert(seq_remix_pending[REMIX_DRUM]==(512u|325u) && oct_leds()==0);
    seq_stop();transport_req=0;
    assert(!preset_load(&trk[1],preset_first_of(DM_ACID)));select_track(1);
    select_pattern(5,6);assert(oct_leds()==OCT_BR_UP);
    select_track(0);assert(oct_leds()==0);
    select_track(1);assert(oct_leds()==OCT_BR_UP);
    tap_button(B_OCTUP);assert((seq_remix_config[1]&255u)==86u && oct_leds()==0);
    select_track(0);ui.keys=ui.env=0;ui.env_t0=0;
    open_page(FAM_SEQ,0);ui.bank=1;
    assert(oct_leds()==(OCT_BR_DN|OCT_BR_UP));
    tap_button(B_OCTUP);assert(ui.bank==2);
    puts("B PASS: only OCT+ breathes for dirty Remix drafts; OCT+/K4 confirmation clears it, queued state stays clear, K3 keeps draft separate, Bass/Drum scope and existing STEP LEDs/navigation preserved");
}
static void grids_led_check(void)
{
    fresh();
    for(unsigned page=0;page<2;page++) {
        open_page(FAM_GRIDS,page);
        for(unsigned k=0;k<NTRK;k++)trk[k].p[P_SRC]=0;
        assert(oct_leds()==0);
        for(unsigned src=1;src<=3;src++) {
            trk[7].p[P_SRC]=(int16_t)src;
            assert(oct_leds()==OCT_BR_UP);
            song.playing=1;assert(oct_leds()==0);
            tap_button(B_OCTUP);assert(str_eq(ui.msg,"STOP TO WRITE") && trk[7].p[P_SRC]==src);
            song.playing=0;transport_req=1;assert(oct_leds()==0);transport_req=0;
        }
        assert(!preset_load(&trk[7],preset_first_of(DM_ACID)));
        assert(oct_leds()==0); /* ACID is never an eligible WRITE target. */
        assert(!preset_load(&trk[7],preset_first_of(DM_K808)));
        assert(oct_leds()==OCT_BR_UP);
        chain.armed=1;assert(oct_leds()==0);chain.armed=0;
        chain.running=1;assert(oct_leds()==0);chain.running=0;
        ui.confirm=1;assert(oct_leds()==(OCT_LIT_DN|OCT_BR_UP));ui.confirm=0;
        ui.menu=2;assert(oct_leds()==OCT_LIT_DN);ui.menu=0;
        ui.mute=1;assert(oct_leds()==0);ui.mute=0;
        ui.layer=1;assert(oct_leds()==0);ui.layer=0;
        ui.ml.on=1;assert(oct_leds()==(OCT_LIT_DN|OCT_BR_UP));ui.ml.on=0;
        ui.keys=1;song.octave=-1;assert(oct_leds()==OCT_LIT_DN);ui.keys=0;song.octave=0;
        ui.env=1;acc_oct=1;assert(oct_leds()==OCT_LIT_UP);ui.env=0;acc_oct=0;
        trk[7].p[P_SRC]=0;assert(oct_leds()==0);
    }
    open_page(FAM_SND,0);assert(oct_leds()==OCT_BR_UP);
    open_page(FAM_LAY,0);assert(oct_leds()==OCT_LIT_DN);
    open_page(FAM_SEQ,0);trk[0].p[P_SLEN]=64;ui.bank=1;
    assert(oct_leds()==(OCT_BR_DN|OCT_BR_UP));
    open_remix();assert(oct_leds()==0);select_pattern(1,1);assert(oct_leds()==OCT_BR_UP);
    puts("A PASS: both GRIDS pages indicate eligible stopped WRITE only; PLAY/queued PLAY/Song/ACID/no targets stay dark; special-mode priorities and other page LEDs preserved");
}
static void red_themes_check(void)
{
    static const theme_t old[] = {
        {"ORANGE",RGB(233,111,37),RGB(50,19,8)}, {"ORANGE INV",RGB(22,9,4),RGB(244,133,50)},
        {"BLUE",RGB(36,69,200),RGB(234,241,255)}, {"WHITE",RGB(255,255,255),RGB(0,0,0)},
        {"BLUE INV",RGB(244,248,255),RGB(36,69,200)}, {"WHITE INV",RGB(0,0,0),RGB(255,255,255)},
        {"YELLOW",RGB(255,208,0),RGB(23,23,23)}, {"YELLOW INV",RGB(23,23,23),RGB(255,208,0)},
        {"VIOLET",RGB(117,36,207),RGB(255,255,255)}, {"VIOLET INV",RGB(255,255,255),RGB(117,36,207)}
    };
    static const unsigned order[]={0,1,2,4,3,5,6,7,8,9,10,11};
    fresh();assert(NTHEMES==12);
    for(unsigned k=0;k<10;k++) {
        assert(str_eq(THEMES[k].name,old[k].name) && THEMES[k].bg==old[k].bg && THEMES[k].ink==old[k].ink);
        assert(theme_index(THEME_TAG|k)==k);
    }
    assert(str_eq(THEMES[10].name,"RED") && str_eq(THEMES[11].name,"RED INV"));
    assert(THEMES[10].bg==0xd800 && THEMES[10].ink==0xffff);
    assert(THEMES[11].bg==0xffff && THEMES[11].ink==0xd800);
    ui.menu=1;ui.menu_sel=MI_COLOR;settings.palette=THEME_TAG;theme_set(settings.palette);
    for(unsigned k=1;k<=NTHEMES;k++) {
        tap_button(B_OCTUP);unsigned id=order[k%NTHEMES];
        assert(settings.palette==(THEME_TAG|id) && C_BG==THEMES[id].bg && C_INK==THEMES[id].ink);
    }
    knob(0,-1);assert(settings.palette==(THEME_TAG|11u) && C_BG==0xffff && C_INK==0xd800);
    knob(0,-1);assert(settings.palette==(THEME_TAG|10u) && C_BG==0xd800 && C_INK==0xffff);
    knob(0,1);knob(0,1);assert(settings.palette==THEME_TAG);
    puts("B PASS: appended RED/RED INV IDs 10/11, RGB565 D800/FFFF exact reversal, menu selection/wrap both directions; accepted RC13 IDs/names/colors retained");
}
static void home_glo_check(void)
{
    fresh();home_screen();
    tap_button(B_HOME);assert(!ui.home && cur_fam()==FAM_MIX && !ui.menu);
    tap_button(B_HOME);assert(ui.home && !ui.menu);
    press(B_GLO);input();assert(!ui.menu && !ui.mute);
    release_all();input();assert(ui.menu==1 && !ui.mute);
    tap_button(B_GLO);assert(!ui.menu && ui.home);
    press(B_HOME);input();
    host_ticks+=701u*1000u*FM1_TICKS_PER_US;input();
    assert(!ui.menu && !ui.home && cur_fam()==FAM_GLO);
    unsigned page=ui.page;input();assert(ui.page==page);
    release_all();input();assert(!ui.menu && !ui.home && ui.page==page);
    press(B_HOME);input();host_ticks+=701u*1000u*FM1_TICKS_PER_US;input();
    assert(ui.home && !ui.menu);
    release_all();input();assert(ui.home && !ui.menu);
    press(B_HOME);input();host_ticks+=701u*1000u*FM1_TICKS_PER_US;input();
    release_all();input();assert(!ui.home && cur_fam()==FAM_GLO);
    tap_button(B_HOME);assert(ui.page==page_first(FAM_GLO)+1);page=ui.page;
    press(B_GLO);input();host_ticks+=401u*1000u*FM1_TICKS_PER_US;input();
    assert(ui.mute && !ui.menu && ui.page==page);
    release_all();input();assert(!ui.mute && !ui.menu && ui.page==page && !ui.home);
    tap_button(B_GLO);assert(ui.menu==1);
    press(B_GLO);input();host_ticks+=401u*1000u*FM1_TICKS_PER_US;input();
    release_all();input();assert(ui.menu==1 && !ui.mute); /* no late menu-closing tap */
    tap_button(B_OCTDN);assert(!ui.menu); /* existing BACK handler */
    tap_button(B_GLO);assert(ui.menu==1);
    press(B_HOME);input();host_ticks+=701u*1000u*FM1_TICKS_PER_US;input();
    assert(!ui.menu && cur_fam()==FAM_GLO);page=ui.page;
    release_all();input();assert(!ui.menu && !ui.home && ui.page==page);
    ui.confirm=1;tap_button(B_GLO);assert(ui.confirm==1 && !ui.menu);
    tap_button(B_OCTDN);assert(!ui.confirm);
    unsigned prior=trk[2].p[P_MUTE];
    press(B_GLO);input();kb_mute_hit=1u<<KEY_TRK_KEY[2];input();
    assert(ui.mute && trk[2].p[P_MUTE]==!prior);
    release_all();input();assert(!ui.menu && !ui.mute && ui.page==page);
    puts("PASS: HOME tap HOME/TRACKS, HOME hold GLOBAL navigation, GLO tap menu toggle, GLO hold MUTE; no hold-release taps, BACK/dialog priority and GLO+key mute preserved");
}
static void tb_assert_part(const track_t *t,const bpart_t *b)
{
    for(unsigned i=0;i<NSTEP;i++) {
        assert(t->step[i].nt==(int)b->step[i].note-60);
        assert(t->step[i].on==(bstep_gate(&b->step[i])==G_NOTE));
        assert(t->step[i].acc==!!(b->step[i].flags&BS_ACCENT));
        assert(t->step[i].slide==!!(b->step[i].flags&BS_SLIDE));
        assert(!t->step[i].tie && !t->step[i].cond && !t->step[i].rat);
    }
}
static void tb_step_check(void)
{
    fresh();unsigned acid=preset_first_of(DM_ACID);
    assert(!preset_load(&trk[2],acid) && !preset_load(&trk[6],acid));
    assert(preset_load(&trk[0],acid)==-1); /* retain two-voice limit */
    select_track(2);
    uint16_t remix[REMIX_SCOPES];
    assert(seq_remix_apply(2,274) && seq_remix_apply(6,291));
    memcpy(remix,(const void *)seq_remix_config,sizeof remix);
    lock_set(2,3,P_LEVEL,110);motion_add(2,2,P_PAN,13);lk_bank_t bank=mo.b;
    static const unsigned lengths[]={1,8,16,18,24,40,64};
    for(unsigned l=0;l<sizeof lengths/sizeof lengths[0];l++) {
        static track_t other[NTRK];memcpy(other,trk,sizeof other);
        trk[2].p[P_SLEN]=(int16_t)lengths[l];trk[2].p[P_SDIV]=3;trk[2].p[P_SRC]=1;
        for(unsigned j=0;j<NSTEP;j++) {
            trk[2].step[j].tie=1;trk[2].step[j].rat=3;trk[2].step[j].cond=56;
        }
        fm1_ms=7919u+l;tb_manual(1);
        bpart_t b={0};b.len=(uint8_t)lengths[l];b.gen=tb_live[2].cfg;tb3po_generate(&b);
        tb_assert_part(&trk[2],&b);
        assert(trk[2].p[P_SLEN]==lengths[l] && trk[2].p[P_SDIV]==3 && !trk[2].p[P_SRC]);
        for(unsigned k=0;k<NTRK;k++)if(k!=2)assert(!memcmp(&other[k],&trk[k],sizeof trk[k]));
    }
    tb3po_cfg_t keep=tb_live[2].cfg;
    select_track(6);fm1_ms+=99;tb_manual(1);
    assert(!memcmp(&keep,&tb_live[2].cfg,sizeof keep) && tb_live[6].cfg.seed!=keep.seed);
    tb3po_cfg_t cfg=tb_live[6].cfg;
    assert(!preset_load(&trk[6],preset_first_of(DM_K808)) && !preset_load(&trk[6],acid));
    assert(!memcmp(&cfg,&tb_live[6].cfg,sizeof cfg));
    /* Mutating hand-edited data leaves all inactive bytes and cell anchors intact. */
    select_track(2);trk[2].p[P_SLEN]=18;
    trk[2].step[0].nt=1;trk[2].step[0].on=1; /* deliberately outside MPEN */
    step_t before[NSTEP];memcpy(before,trk[2].step,sizeof before);
    fm1_ms+=123;tb_manual(0);
    for(unsigned j=0;j<18;j++)if(j%4<2) {
        assert(before[j].nt==trk[2].step[j].nt && before[j].on==trk[2].step[j].on);
    }
    assert(!memcmp(before+18,trk[2].step+18,sizeof(step_t)*(64-18)));
    assert(!memcmp(remix,(const void *)seq_remix_config,sizeof remix) && bank_events_eq(&bank,&mo.b));
    puts("B PASS: production MIDI/articulation adapter, clean plain steps, partial lengths, LEN/DIV retained, arbitrary T3/T7 independent ACID settings/seeds, model switching, hand edits/locks/Motion/Remix retained");
}
static void tb_ui_check(void)
{
    fresh();tap_button(B_ARP);assert(cur_fam()==FAM_GRIDS);
    tap_button(B_ARP);assert(cur_page()->id[0]==G_GFILL1);
    assert(!preset_load(&trk[2],preset_first_of(DM_ACID)));
    assert(!preset_load(&trk[6],preset_first_of(DM_ACID)));
    select_track(2);assert(cur_fam()==FAM_TB3PO && cur_page()->id[0]==0);
    assert(oct_leds()==(OCT_BR_UP|OCT_BR_DN));
    step_t before[NSTEP];memcpy(before,trk[2].step,sizeof before);
    knob(0,5);assert(tb_live[2].cfg.density==75 && tb_live[2].stale && !memcmp(before,trk[2].step,sizeof before));
    tap_button(B_ARP);assert(cur_page()->id[0]==4);
    knob(1,21);assert(tb_live[2].cfg.scale==21);knob(1,-21);assert(!tb_live[2].cfg.scale);
    turn(EN_SELECT,-1);input();assert(cur_page()->id[0]==0);
    fm1_ms=123;tap_button(B_OCTUP);assert(str_eq(ui.msg,"NEW LINE") && !tb_live[2].stale);
    uint32_t seed=tb_live[2].cfg.seed;fm1_ms++;
    tap_button(B_OCTUP);assert(tb_live[2].cfg.seed!=seed);
    ui_draw(); /* actual production drawing, no synthetic UI */
    assert(seq_remix_apply(2,0) && seq_remix_apply(6,291));
    trk[2].p[P_SLEN]=18;seq_start();seq_tick(&trk[2],0);
    for(unsigned c=0;c<18;c++)assert(seq_source_step(&trk[2],c)==c);
    memcpy(before,trk[2].step,sizeof before);fm1_ms++;
    tap_button(B_OCTUP);assert(tb_pending_valid[2] && !memcmp(before,trk[2].step,sizeof before));
    tick_next(&trk[2]);assert(!tb_pending_valid[2] && memcmp(before,trk[2].step,sizeof before));
    bpart_t b={0};b.len=18;b.gen=tb_live[2].cfg;tb3po_generate(&b);tb_assert_part(&trk[2],&b);
    assert(seq_remix_apply(2,274));tick_next(&trk[2]);
    assert(seq_remix_effective(2)==274 && seq_remix_effective(6)==291);
    for(unsigned c=0;c<64;c++)assert(seq_source_step(&trk[2],c)==source(274,c));
    unsigned pos=seq_source_step(&trk[2],trk[2].seq_idx);
    if(trk[2].step[pos].on)assert(trk[2].nt==trk[2].step[pos].nt);
    /* OFF never mutates; ON uses exactly the source routine/random stream per wrap. */
    memcpy(before,trk[2].step,sizeof before);
    for(unsigned j=0;j<18*3;j++)tick_next(&trk[2]);
    assert(!memcmp(before,trk[2].step,sizeof before));
    tb_live[2].cfg.mutate_bars=1;
    while(trk[2].seq_idx!=17)tick_next(&trk[2]);
    step_t expected[NSTEP];memcpy(expected,trk[2].step,sizeof expected);
    tb_state_t expected_state=tb_live[2];tb_mutate_steps(expected,&trk[2],&expected_state,&expected_state.rng);
    uint32_t bars=tb_live[2].bars;tick_next(&trk[2]);
    assert(tb_live[2].bars==bars+1 && tb_live[2].rng==expected_state.rng && !memcmp(expected,trk[2].step,sizeof expected));
    step_t other[NSTEP];memcpy(other,trk[6].step,sizeof other);
    fm1_ms+=71;tap_button(B_OCTDN);
    assert(tb_pending_valid[2] && (str_eq(ui.msg,"MUTATED") || str_eq(ui.msg,"NO CHANGE")));
    tick_next(&trk[2]);assert(!tb_pending_valid[2] && !memcmp(other,trk[6].step,sizeof other));
    fm1_ms+=15;tap_button(B_OCTUP);assert(tb_pending_valid[2]);
    seq_stop();transport_req=0;assert(!tb_pending_valid[2] && !trk[2].gate_n);
    chain.armed=1;memcpy(before,trk[2].step,sizeof before);tap_button(B_OCTUP);
    assert(str_eq(ui.msg,"STOP TO EDIT") && !memcmp(before,trk[2].step,sizeof before));chain.armed=0;
    ui.confirm=1;tap_button(B_OCTDN);assert(!ui.confirm && !memcmp(before,trk[2].step,sizeof before));
    select_track(0);assert(cur_fam()==FAM_GRIDS);
    puts("C PASS: context ARP/two pages, parameter staging/stale marker, NEW/manual/auto mutation, atomic next-step/STOP handoff, gate safety, ORB routing/independence, Song/dialog priority and Grids routing");
}
static void tb_persist_check(void)
{
    fresh();tb3po_cfg_t expected[4][NTRK];
    for(unsigned k=0;k<NTRK;k++) {
        assert(tb_live[k].cfg.density==70 && tb_live[k].cfg.accent==40 && tb_live[k].cfg.slide==25);
        assert(tb_live[k].cfg.oct_range==2 && tb_live[k].cfg.root==9 && !tb_live[k].cfg.scale && tb_live[k].cfg.base_oct==1 && !tb_live[k].cfg.mutate_bars);
    }
    assert(!preset_load(&trk[2],preset_first_of(DM_ACID)) && !preset_load(&trk[6],preset_first_of(DM_ACID)));
    lock_set(2,1,P_LEVEL,109);motion_add(2,0,P_PAN,11);lk_bank_t locks=mo.b;
    uint16_t remix[REMIX_SCOPES];for(unsigned k=0;k<REMIX_SCOPES;k++)remix[k]=(uint16_t)(seq_remix_config[k]=256u+k*13u);
    for(unsigned slot=0;slot<4;slot++) {
        for(unsigned k=0;k<NTRK;k++) {
            tb_live[k].cfg.density=(uint8_t)(slot*20+k);tb_live[k].cfg.scale=(uint8_t)((slot*8+k)%22);
            tb_live[k].cfg.seed=10000+slot*100+k;tb_live[k].cfg.mutate_bars=(uint8_t)(k+slot);
            expected[slot][k]=tb_live[k].cfg;trk[k].p[P_SLEN]=1;
            trk[k].step[0].on=1;trk[k].step[0].nt=(int8_t)(slot*2+k);
        }
        trk[2].p[P_SRC]=1;
        project_save(slot);assert(proj_ok(&proj_slot[slot]));
    }
    assert(sizeof(project_t)==3840 && sizeof proj_slot==15360 && __builtin_offsetof(project_t,rsv_mo)==3252);
    for(unsigned slot=0;slot<4;slot++) {
        assert(proj_load(slot,1));transport_req=0;
        for(unsigned k=0;k<NTRK;k++) {
            assert(!memcmp(&tb_live[k].cfg,&expected[slot][k],sizeof(tb3po_cfg_t)));
            assert(!tb_live[k].rng && !tb_live[k].bars && !tb_pending_valid[k]);
            assert(trk[k].step[0].nt==(int)(slot*2+k));
        }
        assert(bank_events_eq(&locks,&mo.b));
        assert(!memcmp(remix,(const void *)seq_remix_config,sizeof remix));
    }
    power_on(0);
    for(unsigned k=0;k<NTRK;k++)assert(!memcmp(&tb_live[k].cfg,&expected[3][k],sizeof(tb3po_cfg_t)));
    project_t saved[4];memcpy(saved,proj_slot,sizeof saved);
    tb_state_t live[NTRK];memcpy(live,tb_live,sizeof live);
    chain.cfg.count=4;chain.cfg.loop=1;
    for(unsigned row=0;row<4;row++)chain.cfg.row[row]=(chain_row_t){(uint8_t)row,2};
    assert(!chain_prepare());seq_start();
    for(unsigned row=0;row<8;row++) {
        chain.row=(uint8_t)(row%4);chain_apply(0);
        for(unsigned k=0;k<NTRK;k++) {
            assert(!memcmp(&tb_effective(k)->cfg,&expected[row%4][k],sizeof(tb3po_cfg_t)));
            assert(seq_steps(&trk[k])==tb_song_steps[row%4][k]);
        }
        for(unsigned k=0;k<NTRK;k++)seq_tick(&trk[k],0);
        tick_next(&trk[2]); /* auto mutation uses slot-owned RAM, not project steps */
        assert(!memcmp(saved,proj_slot,sizeof saved));
    }
    seq_stop();transport_req=0;
    assert(!memcmp(live,tb_live,sizeof live));
    /* Real one-repeat transitions must count the outgoing slot's final wrap,
     * including return visits; they must never mutate checksum-covered slots. */
    for(unsigned row=0;row<4;row++)chain.cfg.row[row].repeat=1;
    assert(!chain_prepare());seq_start();
    for(unsigned k=0;k<NTRK;k++)seq_tick(&trk[k],0);
    for(unsigned row=0;row<8;row++) {
        unsigned old=chain.slot;uint32_t bars=tb_song[old][2].bars;
        uint32_t advance=duration(&trk[chain.ref])-trk[chain.ref].seq_pos;
        chain_tick(advance);
        assert(chain.slot==(old+1u)%4u && tb_song[old][2].bars==bars+1u);
        for(unsigned k=0;k<NTRK;k++) {
            seq_tick(&trk[k],advance);
            assert(!memcmp(&tb_effective(k)->cfg,&expected[chain.slot][k],sizeof(tb3po_cfg_t)));
        }
        assert(!memcmp(saved,proj_slot,sizeof saved));
    }
    assert(tb_song[0][2].rng!=0 && chain.src[0].timing[2][3]==0 && saved[0].t[2].p[P_SRC]==1);
    seq_stop();transport_req=0;assert(trk[2].p[P_SRC]==1);
    assert(!memcmp(live,tb_live,sizeof live));
    for(unsigned kind=0;kind<3;kind++) {
        project_t legacy=saved[0];
        if(kind==0)memset(legacy.rsv_mo+PROJ_TB_OFFSET,0,PROJ_TB_BYTES);
        if(kind==1)legacy.rsv_mo[PROJ_TB_OFFSET+8]=2;
        if(kind==2)legacy.rsv_mo[PROJ_TB_OFFSET+108]^=1;
        legacy.sum=proj_sum(&legacy);proj_slot[0]=legacy;
        assert(proj_load(0,1));transport_req=0;
        for(unsigned k=0;k<NTRK;k++) {
            tb3po_cfg_t def;tb3po_defaults(&def,0xBEEFu+k);
            assert(!memcmp(&tb_live[k].cfg,&def,sizeof def));
            for(unsigned j=0;j<NSTEP;j++) {
                step_t want=legacy.t[k].step[j];want.rsv=0;
                assert(!memcmp(&want,&trk[k].step[j],sizeof want));
            }
        }
    }
    puts("D PASS: eight-track settings/seeds, A-D SAVE/LOAD/power-on/Song restore, independent runtime, saved steps/Remix/Motion/locks retained, fixed FDRF/noinit layout and legacy/invalid defaults");
}
int main(int argc,char **argv)
{
    setvbuf(stdout,0,_IOLBF,0);
    if(argc==1 || !strcmp(argv[1],"tb-steps"))tb_step_check();
    if(argc==1 || !strcmp(argv[1],"tb-ui"))tb_ui_check();
    if(argc==1 || !strcmp(argv[1],"tb-persist"))tb_persist_check();
    if(argc==1 || !strcmp(argv[1],"home-glo"))home_glo_check();
    if(argc==1 || !strcmp(argv[1],"grids-led"))grids_led_check();
    if(argc==1 || !strcmp(argv[1],"red-themes"))red_themes_check();
    if(argc==1 || !strcmp(argv[1],"grids"))grids_write_check();
    if(argc==1 || !strcmp(argv[1],"orb"))grids_remix_check();
    if(argc==1 || !strcmp(argv[1],"controls"))controls();
    if(argc==1 || !strcmp(argv[1],"assign"))quick_assign_check();
    if(argc==1 || !strcmp(argv[1],"led"))remix_led_check();
    return 0;
}
