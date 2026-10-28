/* SPDX-License-Identifier: GPL-3.0-only */
/* Q24 adaptation of X0X-RMX v1.1.0 bass303.c / waveform_prototype.c,
 * a4e78f3a6e16fa310b7614e5876d07e16a0020f9. The centred geometries,
 * smoothstep, covariance gain compensation and cubic four-box BLEP/BLAMP
 * are retained; no floating-point audio loop or ladder filter is imported.
 * Phase is unsigned Q32; coefficients and samples are signed Q24.
 * All polynomial products use 64-bit intermediates; slopes stay below 16.
 * The reciprocal is prepared once per audio block, not divided per event. */
typedef struct {
    int32_t shark, trap, notch, dc, jump, slope[5];
} acid_morph_mix_t;
static int32_t acid_morph_comp(int32_t variance)
{
    int32_t c=QONE;
    uint32_t i;
    if (variance==QONE) return c;
    for (i=0;i<5u;i++) c=qm(c,(3*QONE-qm(variance,qm(c,c)))/2);
    return c;
}
/* Position Q16, with exact 0/64/127 landmarks. */
static void acid_morph_prepare(acid_morph_mix_t *v, int32_t pos)
{
    const int32_t gs=Q24(0.6454972243679028), gt=Q24(0.7071067811865475), gn=Q24(0.8295613557843402);
    int32_t x,w,c,a;
    v->shark=v->trap=v->notch=0;
    pos=qlim(pos,0,127*65536);
    if (!pos) v->shark=gs;
    else if (pos==64*65536) v->trap=gt;
    else if (pos==127*65536) v->trap=v->notch=gn;
    else {
        x=pos<64*65536 ? pos*4 : (pos-64*65536)*256/63;
        w=qm(qm(x,x),3*QONE-2*x);a=QONE-w;
        c=acid_morph_comp(qm(a,a)+qm(w,w)+qm(pos<64*65536 ? Q24(0.6846531968814576) : Q24(1.9064124952775928),qm(w,a)));
        if (pos<64*65536) {v->shark=qm(qm(c,a),gs);v->trap=qm(qm(c,w),gt);}
        else {v->trap=qm(c,qm(a,gt)+qm(w,gn));v->notch=qm(qm(c,w),gn);}
    }
    v->dc=v->notch/8;
    v->jump=v->shark*3/2;
    v->slope[0]=6*v->shark-8*v->trap-8*v->notch;
    v->slope[1]=16*v->notch;
    v->slope[2]=-8*v->trap-8*v->notch;
    v->slope[3]=v->slope[4]=8*v->trap;
}
static int32_t acid_cubic_image(uint32_t mag, int negative, uint32_t recip, int32_t d, int32_t jump, int32_t slope)
{
    int32_t r=(int32_t)(((uint64_t)mag*recip)>>24), tail=0, ramp;
    if (r>=2*QONE) return 0;
    if (r>=QONE) {
        int32_t x=2*QONE-r, x2=qm(x,x), x4=qm(x2,x2);
        if (jump) tail=x4/24;
        ramp=qm(x4,x)/120;
    } else {
        int32_t r2=qm(r,r);
        if (jump) tail=QONE/2+qm(r,-2*QONE/3+qm(r2,QONE/3-r/8));
        ramp=7*QONE/30+qm(r,-QONE/2+qm(r,QONE/3+qm(r2,-QONE/12+r/40)));
    }
    return qm(jump,negative ? 2*tail : -2*tail)+qm(slope,qm(d,ramp));
}
static int32_t acid_cubic_correction(uint32_t ph, uint32_t event, uint32_t inc, uint32_t recip, int32_t jump, int32_t slope)
{
    int32_t t=(int32_t)(ph-event), d=(int32_t)(inc>>8), y;
    uint32_t mag=t<0 ? 0u-(uint32_t)t : (uint32_t)t;
    if (mag>=2u*inc) return 0;
    y=acid_cubic_image(mag,t<0,recip,d,jump,slope);
    if (inc>0x40000000u) {
        uint64_t adjacent=0x100000000ull-mag;
        if (adjacent<2ull*inc)
            y+=acid_cubic_image((uint32_t)adjacent,t>=0,recip,d,jump,slope);
    }
    return y;
}
static int32_t acid_morph_eval(const acid_morph_mix_t *v, uint32_t ph, uint32_t inc, uint32_t recip)
{
    int32_t u=(int32_t)((ph+0x80000000u)>>8), y=v->dc;
    if (v->shark) y+=qm(v->shark,QONE-3*qm(u,u));
    if (v->trap) {
        int32_t t=u<QONE/4 ? QONE : u<QONE/2 ? 3*QONE-8*u : u<3*QONE/4 ? -QONE : 8*u-7*QONE;
        y+=qm(v->trap,t);
    }
    if (v->notch && u<QONE/4) y-=qm(v->notch,u<QONE/8 ? 8*u : 2*QONE-8*u);
    if (!inc) return y; /* ideal geometry, also useful for the LCD and host oracle */
    if (v->jump) y-=qm(v->shark,qm((int32_t)(inc>>8),(int32_t)(inc>>8)));
    y+=acid_cubic_correction(ph,0x80000000u,inc,recip,v->jump,v->slope[0]);
    if (v->slope[1]) y+=acid_cubic_correction(ph,0xA0000000u,inc,recip,0,v->slope[1]);
    if (v->slope[2]) y+=acid_cubic_correction(ph,0xC0000000u,inc,recip,0,v->slope[2]);
    if (v->slope[3]) {
        y+=acid_cubic_correction(ph,0,inc,recip,0,v->slope[3]);
        y+=acid_cubic_correction(ph,0x40000000u,inc,recip,0,v->slope[4]);
    }
    return y;
}
/* Preserve the original saw/square arithmetic, including rounding/order. */
static inline int32_t acid_plain_wave(uint32_t ph, uint32_t inc, uint32_t wave)
{
    int32_t x;
    if (wave==1u) {
        x=(int32_t)(ph>>7)-(int32_t)((ph+0x80000000u)>>7);
        x-=acid_blep(ph,inc)-acid_blep(ph+0x80000000u,inc);
    } else {
        x=(int32_t)(ph>>7)-QONE;
        x-=acid_blep(ph,inc);
    }
    return x;
}

/* RC10 variable pulse: 1 and 2 are compatible stored Pulse identities.
 * Quarter-power energy compensation limits narrow-pulse loss to 3.7 dB
 * without driving the pre-ladder signal above its existing +/-1.5 range.
 * This is a 256-byte coefficient table, not an oscillator wavetable. */
static const uint16_t ACID_PWM_GAIN_Q15[128] = {
    49632, 48115, 46824, 45707, 44727, 43858, 43079, 42377, 41740, 41157, 40623, 40131, 39676, 39253, 38859, 38492,
    38148, 37826, 37523, 37239, 36970, 36717, 36478, 36251, 36037, 35834, 35642, 35459, 35286, 35121, 34965, 34816,
    34675, 34541, 34414, 34292, 34178, 34068, 33965, 33867, 33774, 33686, 33603, 33524, 33450, 33380, 33315, 33254,
    33196, 33143, 33093, 33048, 33006, 32967, 32932, 32901, 32873, 32848, 32827, 32809, 32794, 32783, 32774, 32770,
    32768, 32770, 32775, 32783, 32795, 32810, 32828, 32850, 32876, 32905, 32937, 32973, 33013, 33057, 33104, 33155,
    33211, 33270, 33333, 33401, 33473, 33550, 33631, 33717, 33809, 33905, 34007, 34114, 34228, 34347, 34473, 34606,
    34746, 34893, 35048, 35212, 35384, 35565, 35757, 35959, 36172, 36397, 36636, 36888, 37156, 37440, 37743, 38065,
    38408, 38775, 39169, 39593, 40049, 40542, 41078, 41663, 42303, 43009, 43793, 44669, 45658, 46786, 48093, 49632,
};
static uint32_t acid_pwm_duty(uint32_t pos)
{
    if (pos<=64u*65536u)
        return 0x0CCCCCCDu+(uint32_t)(((uint64_t)pos*0x73333333u)>>22);
    return 0x80000000u+(uint32_t)(((uint64_t)(pos-64u*65536u)*0x73333333u)/(63u*65536u));
}
static int32_t acid_pwm_gain(uint32_t pos)
{
    uint32_t m=pos>>16, f=pos&65535u;
    int32_t a=ACID_PWM_GAIN_Q15[m];
    return m>=127u ? a : a+(int32_t)(((int64_t)((int32_t)ACID_PWM_GAIN_Q15[m+1u]-a)*f)>>16);
}
static inline int32_t acid_pwm_wave(uint32_t ph, uint32_t inc, uint32_t duty, uint32_t edge_inc, int32_t gain)
{
    /* Same two-saw phase convention as the former Square. At 50%, gain=1,
     * this is bit-identical to it, including both polyBLEP residuals. */
    int32_t x=(int32_t)(ph>>7)-(int32_t)((ph+duty)>>7);
    x-=acid_blep(ph,inc)-acid_blep(ph+duty,edge_inc);
    return (int32_t)(((int64_t)x*gain)>>15);
}
