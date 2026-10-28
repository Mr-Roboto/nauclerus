/* SPDX-License-Identifier: GPL-3.0-only */
/* Production DC/LOWCUT arithmetic versus an independent int64 floor-division
 * model; nominal Q15 master audio plus conservative pre-limiter headroom. */
#include "drum_host.h"
#include <assert.h>
#include <inttypes.h>
#include <limits.h>

/* Mixer clamp at full master: +/-524287 * 4 = +/-2097148 before
 * PERFORM/METRO. Exercise four times that envelope in the scalar DC test. */
#define HEADROOM 8388607
static uint32_t random_state=1;
static uint32_t random_word(void) { return random_state=random_state*1664525u+1013904223u; }
static int64_t floor_div(int64_t n,int64_t d)
{
    return n>=0 ? n/d : -((-n+d-1)/d);
}
/* Explicit model of a pi32v2 32-bit left shift, defined even for negative x.
 * The conversion back does not rely on an out-of-range unsigned-to-signed cast. */
static int32_t target_shift(int32_t x,unsigned sh)
{
    uint32_t u=(uint32_t)x<<sh;
    return (int32_t)(u<=INT32_MAX ? (int64_t)u : (int64_t)u-INT64_C(4294967296));
}
static int32_t reference_dc(int32_t x,int32_t *state,int32_t *error)
{
    int64_t scaled=(int64_t)x*64;
    int64_t e=scaled-*state+*error, d=floor_div(e,4096);
    assert(scaled>=INT32_MIN && scaled<=INT32_MAX && e>=INT32_MIN && e<=INT32_MAX);
    assert(target_shift(x,6)==scaled && target_shift((int32_t)d,12)==d*4096);
    int64_t next=(int64_t)*state+d, remainder=e-d*4096;
    assert(next>=INT32_MIN && next<=INT32_MAX && remainder>=0 && remainder<4096);
    *state=(int32_t)next; *error=(int32_t)remainder;
    return (int32_t)((int64_t)x-floor_div(next+32,64));
}
static int32_t reference_lc(int32_t x,int32_t *state,int32_t *error,unsigned sh)
{
    int64_t scale=INT64_C(1)<<sh;
    int64_t e=(int64_t)x-*state+*error, d=floor_div(e,scale);
    assert(e>=INT32_MIN && e<=INT32_MAX && target_shift((int32_t)d,sh)==d*scale);
    int64_t next=(int64_t)*state+d, remainder=e-d*scale;
    assert(next>=INT32_MIN && next<=INT32_MAX && remainder>=0 && remainder<scale);
    *state=(int32_t)next; *error=(int32_t)remainder;
    return (int32_t)((int64_t)x-next);
}
static void scalar_filters(void)
{
    static const int32_t samples[]={0,1,-1,31,-31,32,-32,63,-63,32767,-32768,
        2097148,-2097148,HEADROOM,-HEADROOM};
    int32_t dc=0,de=0,rdc=0,rde=0,lc=0,le=0,rlc=0,rle=0;
    for(unsigned i=0;i<2000000;i++) {
        int32_t x;
        if(i<150000) x=samples[(i/10000)%(sizeof samples/sizeof samples[0])];
        else if(i<300000) x=(i&1) ? HEADROOM : -HEADROOM;
        else if(i<1900000) x=(int32_t)(random_word()%(2u*HEADROOM+1u))-HEADROOM;
        else x=0;
        assert(dc_block(x,&dc,&de)==reference_dc(x,&rdc,&rde));
        assert(dc==rdc && de==rde && de>=0 && de<4096);
        unsigned sh=5+(i/17003)%2; /* coefficient changes with existing state/remainder */
        int32_t y=x*4;             /* second lowcut stage's conservative input envelope */
        assert(lowcut1(y,&lc,&le,sh)==reference_lc(y,&rlc,&rle,sh));
        assert(lc==rlc && le==rle && le>=0 && le<(1<<sh));
    }
    assert(dc==0 && lc==0);
    for(unsigned i=0;i<10000;i++) {
        assert(dc_block(0,&dc,&de)==0);
        assert(lowcut1(0,&lc,&le,5+(1999999u/17003u)%2)==0);
    }
    /* Explicitly signed updates and the exact accumulated error identity. */
    for(unsigned sh=5;sh<=6;sh++) for(int sign=-1;sign<=1;sign+=2) {
        int32_t state=sign*32767,err=(1<<sh)-1;
        int64_t start=state,total=0,first_error=err;
        for(unsigned i=0;i<100000;i++) {
            int32_t x=i<50000 ? -sign*32767 : 0;
            total+=(int64_t)x-state;
            lowcut1(x,&state,&err,sh);
        }
        assert(total+first_error==(int64_t)(state-start)*(1<<sh)+err && state==0);
    }
    puts("fx filters: 2M positive/negative/DC/noise/headroom/silence samples match int64 floor model and defined target shifts; error feedback, coefficient changes and decay-to-zero passed");
}
static uint64_t hash_sample(uint64_t h,int32_t x) { return (h^(uint32_t)x)*UINT64_C(1099511628211); }
static void speaker_modes(void)
{
    for(unsigned initial=0;initial<3;initial++) {
        host_init(); fx_lowcut=(uint8_t)initial; random_state=1;
        uint64_t hash=UINT64_C(14695981039346656037);
        int32_t tail_l=0,tail_r=0;
        for(unsigned i=0;i<600000;i++) {
            int32_t l=0,r=0;
            if(i<40000) l=r=32767;
            else if(i<80000) l=r=-32768;
            else if(i<120000) {l=32767;r=-32768;}
            else if(i<240000) {l=(i&1) ? 32767 : -32768;r=-l;}
            else if(i<400000) {
                l=(int32_t)(random_word()&65535)-32768;
                r=(int32_t)(random_word()&65535)-32768;
            }
            if(i==200000 || i==300000 || i==400000) fx_lowcut=(uint8_t)((fx_lowcut+1)%3);
            master_out(&l,&r);
            assert(l>=-32767 && l<=32767 && r>=-32767 && r<=32767);
            /* FLAT/LOWCUT settle to zero. BASS+ has separate quantized poles
             * without error feedback: preserve its existing steady tail. */
            if(i==550000) {tail_l=l;tail_r=r;}
            if(i>=550000) {
                assert(l==tail_l && r==tail_r);
                if(fx_lowcut!=2) assert(l==0 && r==0);
            }
            assert(dce_l>=0 && dce_l<4096 && dce_r>=0 && dce_r<4096);
            for(unsigned k=0;k<4;k++) assert(lce[k]>=0 && lce[k]<64);
            hash=hash_sample(hash,l);hash=hash_sample(hash,r);
            hash=hash_sample(hash,dc_l);hash=hash_sample(hash,dce_l);
            hash=hash_sample(hash,lc_l1);hash=hash_sample(hash,lce[0]);
        }
        printf("fx EQ initial=%u: 600000 frames, PCM/filter-state FNV64=%016" PRIx64 ", final tail %d/%d; silence/DC/polarity/mode transitions stable\n",initial,hash,tail_l,tail_r);
    }
}
int main(void)
{
    scalar_filters(); speaker_modes();
    puts("fx filters: all focused production-code checks passed");return 0;
}
