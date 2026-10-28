/* SPDX-License-Identifier: GPL-3.0-only */
/* Focused RC12 checks using production routing, UI and project implementations. */
#define main rc12_boot_checks_not_run
#include "boot_test.c"
#undef main
#include <assert.h>
static void fresh12(void){fresh_flash();wipe_ram();ui_host_init();drum_tracks_init();home_screen();}
static void tick12(void){ui_input();}
static void tap12(unsigned b){press(b);tick12();release_all();tick12();}
static int32_t expected12(int code,int x,int depth)
{
    if(code<2)return x*depth/64;
    int magnitude=depth<0?-depth:depth;
    int oriented=depth<0?-x:x;
    int amount=(oriented+32767)*magnitude/128;
    return code<4?amount:-amount;
}
static void arithmetic12(void)
{
    for(int code=0;code<6;code++)for(int d=-64;d<=64;d++)for(int x=-32767;x<=32767;x++){
        int m=lfo_route_amount(code,x,d);
        assert(m==expected12(code,x,d));
        if(code>=2 && code<4)assert(m>=0);
        if(code>=4)assert(m<=0);
    }
    puts("PASS arithmetic: all 6 codes x 129 depths x 65535 Q15 values; exact old BIP rounding, UNI directions/endpoints");
}
static void setup_lfos12(track_t *t)
{
    for(unsigned l=0;l<2;l++){
        unsigned q=l?P_LFO2:P_LFO1;t->p[q+LF_WAVE]=LW_SINE;t->p[q+LF_MODE]=LM_HZ;
        t->p[q+LF_RATE]=64;t->p[q+LF_PHASE]=(int16_t)(l?96:32);t->p[q+LF_MORPH]=0;t->p[q+LF_TRIG]=LT_FREE;
    }
    for(unsigned s=0;s<4;s++){t->p[LS_ID[s][LS_DEST]]=0;t->p[LS_ID[s][LS_DEPTH]]=0;}
}
static void routing12(void)
{
    fresh12();drum_set_model(&trk[2],DM_ACID);drum_set_model(&trk[6],DM_ACID);
    unsigned dests[]={3,4,9,17,18,19};
    for(unsigned ti=0;ti<2;ti++)for(unsigned slot=0;slot<4;slot++)for(unsigned code=0;code<6;code++)for(int sign=-1;sign<=1;sign+=2)for(unsigned di=0;di<6;di++){
        track_t *t=&trk[ti?6:2];setup_lfos12(t);t->p[P_E6]=1;
        unsigned pid=lfo_dest_param(dests[di]);const param_desc_t *pd=track_desc(t,pid);
        int base=(pd->min+pd->max)/2;t->p[pid]=(int16_t)base;
        t->p[LS_ID[slot][LS_SRC]]=(int16_t)code;t->p[LS_ID[slot][LS_DEST]]=(int16_t)dests[di];t->p[LS_ID[slot][LS_DEPTH]]=(int16_t)(sign*64);
        lfo_track(t,CTL);assert(t->lon==(1u<<(code&1)) && t->lnum==1);
        int amount=expected12(code,t->lfo[code&1].out,sign*64);int v=clamp(base+amount*(pd->max-pd->min)/32767,pd->min,pd->max);
        assert(t->p[pid]==v);
        if(code==2||code==3)assert(v>=base);if(code>=4)assert(v<=base);
        lfo_restore_track(t);assert(t->p[pid]==base);
        int live;assert(lfo_live(t,pid,&live) && live==v);assert(lfo_slot_now(t,slot)==amount);
        /* Boundary clamping does not change the stored base. */
        t->p[pid]=(int16_t)(code>=4?pd->min:pd->max);base=t->p[pid];
        lfo_track(t,CTL);assert(t->p[pid]>=pd->min && t->p[pid]<=pd->max);lfo_restore_track(t);assert(t->p[pid]==base);
    }
    track_t *t=&trk[2];setup_lfos12(t);t->p[P_E6]=1;t->p[P_AMORPH]=64;
    const int codes[]={2,3,4,1},depths[]={32,-17,9,-23};
    for(unsigned s=0;s<4;s++){t->p[LS_ID[s][LS_SRC]]=codes[s];t->p[LS_ID[s][LS_DEST]]=19;t->p[LS_ID[s][LS_DEPTH]]=depths[s];}
    for(unsigned i=0;i<2000;i++){
        lfo_track(t,CTL);int sum=0;for(unsigned s=0;s<4;s++)sum+=expected12(codes[s],t->lfo[codes[s]&1].out,depths[s])*127/32767;
        assert(t->lnum==1 && t->p[P_AMORPH]==clamp(64+sum,0,127));lfo_restore_track(t);assert(t->p[P_AMORPH]==64);
    }
    for(unsigned s=0;s<4;s++)assert(!motion_param(LS_ID[s][LS_SRC]));
    puts("PASS routing: both ACID tracks, both LFOs/all 4 slots, engine/FX cutoff+resonance, level/PWM, signed depth, clamping, live meters, mixed accumulation and 2000-block restoration; SRC is not Motion/lock recordable");
}
static void ui12(void)
{
    fresh12();track_select(2);open_family(FAM_LFO);for(unsigned i=0;i<3;i++)tap12(B_LFO);
    assert(cur_page()->graph==GR_LDEST);
    static const char *names[]={"L1BI","L2BI","L1U+","L2U+","L1U-","L2U-"};
    for(unsigned s=0;s<4;s++){
        song.lslot=(uint8_t)s;unsigned id=LS_ID[s][LS_SRC];TSEL->p[id]=0;
        for(unsigned code=0;code<6;code++){
            if(code){turn(EN_K2,1);tick12();}assert(TSEL->p[id]==code && !strcmp(N_LSRC[code],names[code]));
            unsigned a=P_LFO1+LF_MODE,b=P_LFO2+LF_MODE;TSEL->p[a]=TSEL->p[b]=LM_SYNC;
            tap12(B_OCTUP);assert(TSEL->p[code&1?b:a]==LM_HZ && TSEL->p[code&1?a:b]==LM_SYNC);
        }
        for(unsigned code=0;code<6;code++){
            unsigned cc=code*127u/5u;ml_set(TSEL,id,cc);assert(TSEL->p[id]==code);
        }
    }
    puts("PASS UI/MIDI Learn: 4 slots x six visible source/mode choices, K2 edits, parity-correct OCT rate-mode selection and unchanged MIDI parameter IDs");
}
static void storage12(void)
{
    fresh12();
    for(unsigned k=0;k<NTRK;k++)for(unsigned s=0;s<4;s++)assert(trk[k].p[LS_ID[s][LS_SRC]]==(int)(s&1));
    for(unsigned id=0;id<PRE_NFACT;id++){
        assert(!preset_load(&trk[0],id));for(unsigned s=0;s<4;s++)assert(trk[0].p[LS_ID[s][LS_SRC]]==(int)(s&1));
    }
    drum_set_model(&trk[2],DM_ACID);track_t *t=&trk[2];setup_lfos12(t);t->p[P_E6]=1;
    motion_add(2,0,P_PAN,11);lock_set(2,1,P_LEVEL,103);lk_bank_t locks=mo.b;step_t steps[NSTEP];memcpy(steps,t->step,sizeof steps);
    for(unsigned slot=0;slot<4;slot++){
        for(unsigned k=0;k<NTRK;k++)for(unsigned s=0;s<4;s++)trk[k].p[LS_ID[s][LS_SRC]]=(int16_t)((slot+k+s)%6);
        project_save(slot);assert(proj_ok(&proj_slot[slot]));
    }
    project_t saved[4];memcpy(saved,proj_slot,sizeof saved);
    for(unsigned slot=0;slot<4;slot++){
        assert(proj_load(slot,1));transport_req=0;
        for(unsigned k=0;k<NTRK;k++){
            for(unsigned s=0;s<4;s++)assert(trk[k].p[LS_ID[s][LS_SRC]]==(int)((slot+k+s)%6));
            assert(!memcmp(trk[k].p,saved[slot].t[k].p,sizeof saved[slot].t[k].p));
        }
        assert(bank_events_eq(&locks,&mo.b) && !memcmp(steps,trk[2].step,sizeof steps));
    }
    /* A real FDRE conversion retains old source0/1 exactly. */
    for(unsigned k=0;k<NTRK;k++)for(unsigned s=0;s<4;s++)trk[k].p[LS_ID[s][LS_SRC]]=(int16_t)(s&1);
    static project_ve_t ve;fdre_of_live(&ve);assert(st_save(OBJ_PROJECT0,&ve,sizeof ve)==0);memset(&proj_slot[0],0,sizeof proj_slot[0]);
    assert(proj_load(0,1));transport_req=0;for(unsigned k=0;k<NTRK;k++)for(unsigned s=0;s<4;s++)assert(trk[k].p[LS_ID[s][LS_SRC]]==(int)(s&1));
    memcpy(proj_slot,saved,sizeof saved);assert(proj_load(3,1));transport_req=0;
    int16_t sources[4];for(unsigned s=0;s<4;s++)sources[s]=trk[2].p[LS_ID[s][LS_SRC]];
    setup_lfos12(&trk[2]);trk[2].p[P_LFO1+LF_DEST]=9;trk[2].p[P_LFO1+LF_DEPTH]=24;
    chain.cfg.count=4;chain.cfg.loop=1;for(unsigned s=0;s<4;s++)chain.cfg.row[s]=(chain_row_t){(uint8_t)s,1};
    assert(!chain_prepare());seq_start();
    for(unsigned r=0;r<8;r++){
        chain.row=(uint8_t)(r%4);chain_apply(0);
        for(unsigned s=0;s<4;s++)assert(trk[2].p[LS_ID[s][LS_SRC]]==sources[s]);
        int base=trk[2].p[P_LEVEL];lfo_track(&trk[2],CTL);assert(trk[2].p[P_LEVEL]>=0 && trk[2].p[P_LEVEL]<=127);lfo_restore_track(&trk[2]);assert(trk[2].p[P_LEVEL]==base);
    }
    seq_stop();transport_req=0;assert(!memcmp(saved,proj_slot,sizeof saved));
    assert(sizeof(project_t)==3840 && sizeof proj_slot==15360 && P_LSRC1==61 && P_LS4DEPTH==68 && P_NSTORED==69);
    puts("PASS persistence/presets/Song: every factory preset/default BIP; all six source codes across 8 tracks/A-D round-trip; FDRE legacy 0/1; Motion/locks/steps retained; Song preserves live routing (existing ownership), restores modulated bases and never rewrites slots; FDRF3840/IDs unchanged");
}
int main(int argc,char **argv)
{
    assert(argc==2);setvbuf(stdout,0,_IOLBF,0);
    if(!strcmp(argv[1],"arithmetic"))arithmetic12();
    else if(!strcmp(argv[1],"routing"))routing12();
    else if(!strcmp(argv[1],"ui"))ui12();
    else if(!strcmp(argv[1],"storage"))storage12();
    else return 2;
    return 0;
}
