/* SPDX-License-Identifier: GPL-3.0-only */
/* Production project preparation, song, sequencer, Motion/locks and audio. */
#define DRUM_REMIX_TEST 1
#define main remix_existing_boot_main
#include "boot_test.c"
#undef main
#include <assert.h>
#include <inttypes.h>

static const unsigned LENGTHS[] = {1,8,16,24,40,64};
static void set_code(unsigned k, unsigned code)
{
    assert(seq_remix_test_set(k, code ? (code-1)/16 : 0, code ? (code-1)%16 : 0, code != 0));
}
static unsigned slot_code(unsigned s, unsigned k)
{
    k=trk[k].p[P_MODEL]==DM_ACID ? k : 7;
    return s == 0 && k == 0 ? 1 : s == 3 && k == 7 ? 256 : (s*61+k*29+2)%256+1;
}
static unsigned mapped(unsigned code, unsigned c)
{
    return code ? (unsigned)remix_source_step((code-1)/16,(code-1)%16,c) : c;
}
static uint32_t period(track_t *t)
{
    return step_samples(t,div_period((uint32_t)t->p[P_SDIV],t->seq_rem),t->seq_cnt);
}
static uint64_t fold(uint64_t h, uint32_t v) { return (h^v)*UINT64_C(1099511628211); }
static uint64_t state_hash(uint64_t h)
{
    h=fold(h,song.playing); h=fold(h,song.rec); h=fold(h,song.tick);
    h=fold(h,chain.running); h=fold(h,chain.row); h=fold(h,chain.slot);
    h=fold(h,chain.left); h=fold(h,chain.b24); h=fold(h,chain.ref);
    for(unsigned k=0;k<NTRK;k++) {
        track_t *t=&trk[k];
        const uint32_t v[]={t->seq_idx,t->seq_cnt,t->seq_pos,t->seq_rem,t->seq_par,t->rng,
            t->rat_n,t->rat_k,t->rat_len,t->rat_vel,(uint8_t)t->rat_nt,t->gate_n,t->slide_in,
            (uint8_t)t->nt,t->v[0].age,t->v[0].vel,t->lnum,t->rskip};
        for(unsigned i=0;i<sizeof v/sizeof v[0];i++) h=fold(h,v[i]);
        for(unsigned p=0;p<P_COUNT;p++) {
            h=fold(h,(uint16_t)t->p[p]); h=fold(h,(uint16_t)mo.base[k][p]);
            h=fold(h,(uint16_t)mo.under[k][p]);
        }
        for(unsigned w=0;w<MO_W;w++) { h=fold(h,mo.act[k][w]); h=fold(h,mo.lact[k][w]); }
    }
    return h;
}
/* mode: OFF, A1, original active, or materialized active played OFF.
 * The oracle only copies events using the production mapper. Both sides run
 * the real Song timing, sequencer, Motion/Lock engine, DSP and mixer. */
static void setup(unsigned mode, unsigned acid, unsigned grids, unsigned loop)
{
    fresh_flash(); wipe_ram(); flash_ok=0;
    song.g[G_BPM]=239;
    for(unsigned k=0;k<NTRK;k++) {
        old_set_model(&trk[k],acid && k<2 ? DM_ACID : DM_K808);
        trk[k].p[P_LEVEL]=90; trk[k].p[P_PAN]=-7; trk[k].p[P_E1]=64;
    }
    for(unsigned slot=0;slot<4;slot++) {
        memset(&mo.b,0,sizeof mo.b);
        for(unsigned k=0;k<NTRK;k++) {
            step_t original[NSTEP]; lk_bank_t bank; memset(&bank,0,sizeof bank);
            track_t *t=&trk[k]; unsigned code=mode>=2 ? slot_code(slot,k) : mode;
            t->p[P_SLEN]=(int16_t)(1+(k+slot)%8); t->p[P_SDIV]=(int16_t)(2+(k+slot)%2);
            t->p[P_SSWING]=(int16_t)((k+slot)*5%43); t->p[P_SRC]=grids && k==7 ? 1 : 0;
            set_code(k,mode==3 ? 0 : code);
            for(unsigned s=0;s<NSTEP;s++) {
                memset(&original[s],0,sizeof original[s]);
                original[s].on=(s+slot)%5!=2; original[s].nt=(int8_t)((int)(s%24)-12);
                original[s].acc=s%2; original[s].rat=s%4;
                original[s].tie=acid && k<2 && s%7==3;
                original[s].slide=acid && k<2 && s%3==0;
                original[s].cond=s%4==0 ? 22 : s%4==1 ? COND_1SHOT : s%4==2 ? 11 : 0;
                if(s%3==1) assert(lk_put(&bank,lk_key(k,s,P_LEVEL),30+(int)slot*9+(int)s%17,0)>=0);
                if(s%4==1) assert(lk_put(&bank,lk_key(k,s,P_LEVEL),100+(int)slot,1)>=0);
                if(s%7==1) assert(lk_put(&bank,lk_key(k,s,P_PAN),(int)slot*8-12,1)>=0);
                if(k<2 && s%5==1) {
                    assert(lk_put(&bank,lk_key(k,s,P_E1),30+(int)slot,0)>=0);
                    assert(lk_put(&bank,lk_key(k,s,P_E1),100+(int)slot,1)>=0);
                }
            }
            for(unsigned c=0;c<NSTEP;c++) {
                unsigned s=mode==3 && !t->p[P_SRC] ? mapped(code,c) : c;
                t->step[c]=original[s];
                unsigned i=lk_seek(&bank,k,s);
                for(;i<bank.count && (bank.key[i]>>7)==(k<<6|s);i++)
                    assert(lk_put(&mo.b,lk_key(k,c,bank.key[i]&127),bank.val[i],lk_is_lock(&bank,i))>=0);
            }
        }
        mo.b.on=255; project_save(slot); assert(proj_ok(&proj_slot[slot]));
    }
    /* The edited project's data and selections deliberately differ from every slot. */
    memset(&mo.b,0,sizeof mo.b);
    for(unsigned k=0;k<NTRK;k++) {
        trk[k].p[P_SLEN]=13; trk[k].p[P_SDIV]=1; trk[k].p[P_SSWING]=17; trk[k].p[P_SRC]=0;
        memset(trk[k].step,0,sizeof trk[k].step); trk[k].step[1].on=1; trk[k].step[1].nt=19;
        set_code(k,200+k);
    }
    /* Exercise modulation round the row's held/locked parameter as well. */
    trk[0].p[P_LFO1+LF_DEST]=2; trk[0].p[P_LFO1+LF_DEPTH]=10;
    trk[0].p[P_LFO1+LF_TRIG]=LT_FREE; trk[0].p[P_LFO1+LF_MODE]=LM_HZ; trk[0].p[P_LFO1+LF_RATE]=127;
    static const uint8_t slots[16]={0,1,2,3,0,2,1,3,3,0,1,2,0,3,2,1};
    chain.cfg.count=16; chain.cfg.loop=(uint8_t)loop;
    for(unsigned r=0;r<16;r++) chain.cfg.row[r]=(chain_row_t){slots[r],(uint8_t)(1+r%3)};
    song.rec=255;
    assert(chain_prepare()==0);
}
typedef struct { uint64_t audio,state; unsigned blocks,rows,repeats,midblocks; } result_t;
static result_t run(unsigned mode,unsigned acid,unsigned grids,unsigned loop)
{
    setup(mode,acid,grids,loop);
    static project_t saved[4]; static lk_bank_t banks[4]; step_t live[NTRK][NSTEP];
    memcpy(saved,proj_slot,sizeof saved); memcpy(banks,lk_slot,sizeof banks);
    for(unsigned k=0;k<NTRK;k++) memcpy(live[k],trk[k].step,sizeof live[k]);
    result_t out={UINT64_C(14695981039346656037),UINT64_C(14695981039346656037),0,0,0,0};
    unsigned previous_row=99, previous_left=0;
    while((song.playing || transport_req) && out.blocks<150000) {
        int32_t l[CTL],r[CTL]; render_mix(l,r,CTL); out.blocks++;
        for(unsigned i=0;i<CTL;i++) { out.audio=fold(out.audio,(uint32_t)l[i]); out.audio=fold(out.audio,(uint32_t)r[i]); }
        out.state=state_hash(out.state);
        if(chain.running) {
            if(chain.row!=previous_row) {
                if(previous_row!=99 && trk[0].seq_pos>0 && trk[0].seq_pos<CTL) out.midblocks++;
                out.rows++; previous_row=chain.row;
            } else if(chain.left!=previous_left) out.repeats++;
            previous_left=chain.left;
#ifndef DRUM_REMIX_OFF_PROBE
            unsigned slot=chain.cfg.row[chain.row].slot;
            assert(chain.slot==slot && mo.src==&lk_slot[slot] && seq_steps(&trk[0])==chain.src[slot].step[0]);
            for(unsigned k=0;k<NTRK;k++) {
                unsigned code=mode==2 ? slot_code(slot,k) : mode==1 ? 1 : 0;
                assert(seq_remix_effective(k)==seq_remix_state(code));
                unsigned c=trk[k].seq_idx;
                assert(seq_source_step(&trk[k],c)==(trk[k].p[P_SRC] ? c : mapped(code,c)));
                assert(seq_remix_config[seq_remix_scope(k)]==seq_remix_state(200+(trk[k].p[P_MODEL]==DM_ACID ? k : 7)) && !song.rec);
            }
#endif
        }
        if(loop && out.rows==33) break; /* two complete traversals, then row 1 */
    }
    assert(out.blocks<150000 && out.rows==(loop ? 33u : 16u) && out.repeats>0 && out.midblocks>0);
    seq_stop();
    for(unsigned k=0;k<NTRK;k++) {
        assert(seq_remix_config[seq_remix_scope(k)]==seq_remix_state(200+(trk[k].p[P_MODEL]==DM_ACID ? k : 7)) && !seq_remix[k] && trk[k].p[P_SLEN]==13 && song.rec==255);
        assert(!memcmp(live[k],trk[k].step,sizeof live[k]) && !lock_held(k,P_LEVEL) && !lock_held(k,P_E1));
        assert(*motion_slot(&trk[k],P_LEVEL)==90 && *motion_slot(&trk[k],P_PAN)==-7 && *motion_slot(&trk[k],P_E1)==64);
    }
    assert(!memcmp(saved,proj_slot,sizeof saved) && !memcmp(banks,lk_slot,sizeof banks));
    /* Ordinary PLAY uses the retained edited project, never the last song slot. */
    seq_start();
    for(unsigned k=0;k<NTRK;k++) assert(seq_source_step(&trk[k],1)==mapped(200+(trk[k].p[P_MODEL]==DM_ACID ? k : 7),1) && seq_steps(&trk[k])==trk[k].step);
    seq_stop(); assert(chain_prepare()==0); seq_start();
    assert(chain.row==0 && chain.slot==0);
#ifndef DRUM_REMIX_OFF_PROBE
    for(unsigned k=0;k<NTRK;k++)
        assert(seq_remix_effective(k)==seq_remix_state(mode==2 ? slot_code(0,k) : mode==1 ? 1 : 0));
#endif
    seq_stop(); return out;
}
#ifndef DRUM_REMIX_OFF_PROBE
static void same(result_t a,result_t b)
{
    assert(a.audio==b.audio && a.state==b.state && a.blocks==b.blocks && a.rows==b.rows && a.repeats==b.repeats && a.midblocks==b.midblocks);
}
static void test_materialized(void)
{
    for(unsigned acid=0;acid<2;acid++) for(unsigned loop=0;loop<2;loop++) {
        same(run(0,acid,1,loop),run(1,acid,1,loop));
        same(run(2,acid,1,loop),run(3,acid,1,loop));
    }
    puts("remix song: OFF/A1 and active/materialized audio + state identical; A-D, all 16 rows, repeats, returns, two loops, Grids, LFO/Motion/Lock, drums/ACID and mid-block transitions passed");
}
static void test_all_selections(void)
{
    for(unsigned leni=0;leni<6;leni++) for(unsigned acid=0;acid<2;acid++) {
        fresh_flash(); wipe_ram(); flash_ok=0;
        for(unsigned k=0;k<NTRK;k++) {
            old_set_model(&trk[k],acid && k<2 ? DM_ACID : DM_K808); trk[k].p[P_SLEN]=(int16_t)LENGTHS[leni];
            trk[k].p[P_SRC]=0; trk[k].p[P_SDIV]=(int16_t)((leni+k)%10); trk[k].p[P_SSWING]=37;
            for(unsigned s=0;s<NSTEP;s++) {
                memset(&trk[k].step[s],0,sizeof(step_t)); trk[k].step[s].on=1; trk[k].step[s].nt=(int8_t)(s%24-12);
                trk[k].step[s].rat=s%4; trk[k].step[s].acc=s%2;
            }
            for(unsigned s=0;s<NSTEP;s++) {
                motion_add(k,s,P_LEVEL,30+(int)s); lock_set(k,s,P_LEVEL,100);
            }
        }
        /* Every code on every track, distributed differently across all four slots. */
        for(unsigned seed=0;seed<=256;seed++) {
            for(unsigned slot=0;slot<4;slot++) {
                for(unsigned k=0;k<NTRK;k++) set_code(k,(seed+slot*13+k*37)%257);
                project_save(slot);
            }
            chain.cfg.count=4;
            for(unsigned slot=0;slot<4;slot++) chain.cfg.row[slot]=(chain_row_t){(uint8_t)slot,1};
            assert(chain_prepare()==0); transport_req=0; seq_start();
            for(unsigned slot=0;slot<4;slot++) {
                chain.row=(uint8_t)slot; chain_apply(0);
                for(unsigned k=0;k<NTRK;k++) {
                    track_t *t=&trk[k]; unsigned code=(seed+slot*13+(acid && k<2 ? k : 7)*37)%257;
                    assert(seq_remix_effective(k)==seq_remix_state(code));
                    for(unsigned c=0;c<LENGTHS[leni];c++) {
                        seq_tick(t,c ? period(t)-t->seq_pos : 0);
                        unsigned source=mapped(code,c);
                        assert(t->seq_idx==c && seq_source_step(t,c)==source && t->nt==(int)(source%24)-12);
                        assert(t->rat_n==source%4+1 && t->rat_len==period(t) && t->rat_vel==(source%2 ? 127u : 96u));
                        assert(t->p[P_LEVEL]==100 && mo.under[k][P_LEVEL]==30+(int)source);
                        unsigned nxt=(c+1)%LENGTHS[leni];
                        assert(seq_next(t)==&chain.src[slot].step[k][mapped(code,nxt)]);
                        assert(!memcmp(chain.src[slot].step[k],proj_slot[slot].t[k].step,sizeof trk[k].step));
                    }
                }
            }
            seq_stop();
        }
    }
    puts("remix song: OFF/all 256 codes on every track and slot; six lengths, four blocks, all divisions, swing, mapped notes/ratchets/Motion-before-Lock and wrapped lookahead passed");
}
static void extension_hash(project_t *p)
{
    uint32_t h=proj_hash(p->rsv_mo,30);
    for(unsigned i=0;i<4;i++) p->rsv_mo[30+i]=(uint8_t)(h>>(8*i));
    p->sum=proj_sum(p);
}
static void test_metadata(void)
{
    setup(2,0,0,0); seq_stop(); transport_req=0;
    project_t valid=proj_slot[0];
    for(unsigned kind=0;kind<7;kind++) {
        proj_slot[0]=valid; uint8_t *d=proj_slot[0].rsv_mo;
        if(kind==0) memset(d,0,510);
        if(kind==1) d[8]=3;
        if(kind==2) d[0]^=1;
        if(kind==3) d[11]=1;
        if(kind==4) { d[12]=0;d[13]=2; }
        if(kind==5) { d[12]=255;d[13]=255; }
        extension_hash(&proj_slot[0]);
        if(kind==6) { d[30]^=1;proj_slot[0].sum=proj_sum(&proj_slot[0]); }
        assert(proj_ok(&proj_slot[0]) && chain_prepare()==0); transport_req=0; seq_start();
        for(unsigned k=0;k<NTRK;k++) for(unsigned c=0;c<64;c++) assert(seq_source_step(&trk[k],c)==c);
        seq_stop();
    }
    proj_slot[0]=valid; proj_slot[0].rsv_mo[12]^=1;
    assert(chain_prepare()==3); /* Outer corruption still refuses the slot. */
    assert(fdre_migrates()); transport_req=0;
    chain.cfg.count=1;chain.cfg.row[0]=(chain_row_t){1,1};
    for(unsigned k=0;k<NTRK;k++) set_code(k,256);
    assert(chain_prepare()==0); seq_start();
    for(unsigned k=0;k<NTRK;k++) assert(!seq_remix_effective(k));
    seq_stop();
    puts("remix song: absent/unknown/malformed/corrupt extension loads OFF, corrupt project still refused, converted FDRE OFF with its original bank, live configuration untouched");
}
static void test_transition_gates(void)
{
    setup(2,1,0,0); seq_stop(); transport_req=0;
    chain.cfg.count=2; chain.cfg.row[0]=(chain_row_t){1,1}; chain.cfg.row[1]=(chain_row_t){2,1};
    /* Explicit mapped NOTE -> TIE -> SLIDE -> rest, including repeated sources. */
    for(unsigned slot=1;slot<=2;slot++) {
        project_t *p=&proj_slot[slot]; p->t[0].p[P_SLEN]=5; p->t[0].p[P_SDIV]=2;p->t[0].p[P_SSWING]=0;
        memset(p->t[0].step,0,sizeof p->t[0].step);
        p->t[0].step[0].on=1;p->t[0].step[0].slide=1;p->t[0].step[0].nt=-12;
        unsigned code=slot==1 ? 3 : 5, tie=mapped(code,1), last=mapped(code,4);
        p->t[0].step[tie].tie=1;p->t[0].step[tie].slide=1;p->t[0].step[tie].nt=15;
        p->t[0].step[2].on=1;p->t[0].step[2].nt=7;
        p->t[0].step[last].on=1;p->t[0].step[last].slide=1;
        if(last) p->t[0].step[last].nt=-7; /* A3 repeats the opening source zero here. */
        assert(lk_put(&lk_slot[slot],lk_key(0,last,P_LEVEL),40,0)>=0);
        assert(lk_put(&lk_slot[slot],lk_key(0,last,P_LEVEL),110,1)>=0);
        lk_seal(&lk_slot[slot],slot);p->lk_n=lk_slot[slot].count;p->lk_link=proj_link(&lk_slot[slot]);
        p->rsv_mo[12]=(uint8_t)(slot==1 ? 2 : 4);p->rsv_mo[13]=1;extension_hash(p);
    }
    assert(chain_prepare()==0); transport_req=0;seq_start();
    track_t *t=&trk[0]; seq_tick(t,0); uint32_t age=t->v[0].age;
    assert(t->nt==-12 && t->gate_n==GATE_HELD);
    seq_tick(t,period(t)); assert(t->v[0].age==age && t->gate_n==GATE_HELD);
    seq_tick(t,period(t)); assert(t->v[0].age==age && t->nt==7 && !t->slide_in);
    seq_tick(t,period(t)); assert(!t->gate_n);
    seq_tick(t,period(t)); assert(t->gate_n==GATE_HELD && lock_held(0,P_LEVEL) && mo.under[0][P_LEVEL]==40);
    /* Force the existing reference to its last chronological event, 7 samples
     * before the boundary. chain_tick must install the new row before seq_tick. */
    chain.ref=0; t->seq_pos=period(t)-7;
    chain_tick(CTL); assert(chain.row==1 && chain.slot==2 && chain.cut && !t->gate_n && !t->slide_in);
    for(unsigned k=0;k<NTRK;k++) {
        assert(seq_remix_effective(k)==chain.src[2].remix[seq_remix_scope(k)]);
        assert(!lock_held(k,P_LEVEL) && !motion_moved(k,P_LEVEL));
    }
    seq_tick(t,CTL); assert(t->seq_idx==0 && t->seq_pos==CTL-7 && t->nt==-12 && t->v[0].age>age);
    assert(seq_next(t)==&chain.src[2].step[0][9]);
    seq_start(); assert(chain.running && chain.slot==1 && chain.row==0 && !t->gate_n);
    assert(seq_remix_effective(0)==seq_remix_state(3));
    seq_tick(t,0);assert(seq_next(t)==&chain.src[1].step[0][5]);
    seq_stop(); assert(chain_prepare()==0);seq_start();assert(chain.slot==1 && chain.row==0);
    seq_stop();
    puts("remix song: mapped ACID NOTE/TIE/SLIDE/rest and new-row lookahead; old gates/locks/Motion released, selection installed before first hit, 7-sample mid-block cut and restart passed");
}
int main(void)
{
    setvbuf(stdout,0,_IOLBF,0);
    test_all_selections(); test_metadata(); test_transition_gates(); test_materialized();
    puts("remix song: all production-code checks passed"); return 0;
}
#else
int main(void)
{
    for(unsigned acid=0;acid<2;acid++) for(unsigned loop=0;loop<2;loop++) {
        result_t r=run(0,acid,1,loop);
        printf("OFF acid=%u loop=%u audio=%016" PRIx64 " state=%016" PRIx64 " blocks=%u rows=%u repeats=%u midblocks=%u\n",
               acid,loop,r.audio,r.state,r.blocks,r.rows,r.repeats,r.midblocks);
    }
    return 0;
}
#endif
