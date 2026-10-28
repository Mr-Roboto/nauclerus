/* Immutable pre-RC10 DSP snapshot; private symbols prefixed only.
 * RC9 package SHA-256: 18eb2419885059ae8b119de0646fe5a24be533b0e01ddb5c11a0d0dac0e48823
 * dm_acid.c SHA-256: 8371f60ac58568e03170da292a7be5bcd85d2aa6e94d4634dbe7945052644f43
 * acid_wave.c SHA-256: 36b7c119badd164f310b79aa2ffc024c0117f7fb21f3f24eef9874c6d0225d1e */
/* SPDX-License-Identifier: GPL-3.0-only
 * Drum machine fork: 2026 DEADACTIVE */
/* ACID (spec docs/superpowers/specs/2026-10-09-acid-design.md §4): a 303-style mono bass voice. A band-limited saw or
 * square (polyBLEP) into a 24 dB 4-pole ladder (zero-delay feedback, Q24, a cubic soft clip on its input x - k y)
 * whose cutoff follows CUTOF + ENVMD x the filter envelope + the accent; the cutoff is computed per block at the
 * block's end and the ladder's coefficients ramp to it sample by sample (no zipper). The amp opens fast, holds while
 * the gate is open, releases in ~8 ms. ACC: more level and filter envelope (ACCNT), a shorter filter decay, and a
 * slow smoother per track (track_t.wow) that consecutive accents charge (the 303's "wow"). Slide (glide hook): the
 * pitch moves in the log domain over ~60 ms, nothing restarts. The sequencer drives the gate (seq.c rc9_acid_step, the
 * countdown in seq_tick); a hit no step drives (MIDI, the keys, Grids: drum_core.c drum_hit) closes after GATE of the
 * track's step; ACID_LIFE closes any gate still open, a backstop as the M1-C voices' lifetime: no note drones on.
 * One voice per track. Voice fields: ph[0] phase; x[0] pitch now, x[1] its target (1/16 semitone); x[2] flags
 * (bit 0 gate open, 1 attack done, 2 SQR, 3 the ramps below started); x[3] accent (Q15); env[0] amp, env[1] filter
 * envelope (Q24), k[1] its DECAY; f[0..3] the ladder; c[0].a1 / a2 / a3: G, 1 / (1 + k G^4) and the output gain the
 * last block ended on. */
#define ACID_ATT (QONE / 132)                           /* the amp's attack: ~3 ms linear to full */
#define ACID_REL 64268u                                 /* the amp's release per sample, Q16: ~8 ms to -60 dB */
#define ACID_GLIDE Q24(0.03)                            /* per block: a 12-semitone slide within 1/16 in ~60 ms */
#define ACID_WOW 65300u                                 /* track_t.wow's decay per block, Q16: ~200 ms */
#define ACID_LIFE LIFE_A                                /* the gate closes 5.5 s after the note (or its slide) */
#define ACID_KMAX Q24(4.4)                              /* RESO 127: the ladder's feedback past 4: it oscillates at
                                                         * every CUTOF (but the lowest, under a far higher note), the
                                                         * soft clip holds the amplitude */
#define ACID_GAIN 870                                   /* the output gain, Q8 (x3.4): the defaults level with TOM's */

static inline int32_t rc9_acid_clip(int32_t x)              /* x - 4/27 x^3, x held to +-1.5 (Q24): out +-1, smooth */
{
    x = qlim(x, -Q24(1.5), Q24(1.5));
    return x - qm(qm(qm(x, x), x), Q24(4.0 / 27.0));
}
static inline int32_t rc9_acid_blep(uint32_t ph, uint32_t inc)   /* polyBLEP residual of a -2 step at the wrap (Q24),
                                                              * ph / inc Q32 */
{
    int32_t t;
    if (ph < inc) {                                     /* just after the wrap: t = ph / inc in 0..1 */
        t = (int32_t)(((uint64_t)ph << 24) / inc);
        return t + t - qm(t, t) - QONE;
    }
    if (ph > 0xFFFFFFFFu - inc) {                       /* just before it: t in -1..0 */
        t = (int32_t)(((uint64_t)(ph - (0xFFFFFFFFu - inc)) << 24) / inc) - QONE;
        return qm(t, t) + t + t + QONE;
    }
    return 0;
}
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
} rc9_acid_morph_mix_t;
static int32_t rc9_acid_morph_comp(int32_t variance)
{
    int32_t c=QONE;
    uint32_t i;
    if (variance==QONE) return c;
    for (i=0;i<5u;i++) c=qm(c,(3*QONE-qm(variance,qm(c,c)))/2);
    return c;
}
/* Position Q16, with exact 0/64/127 landmarks. */
static void rc9_acid_morph_prepare(rc9_acid_morph_mix_t *v, int32_t pos)
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
        c=rc9_acid_morph_comp(qm(a,a)+qm(w,w)+qm(pos<64*65536 ? Q24(0.6846531968814576) : Q24(1.9064124952775928),qm(w,a)));
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
static int32_t rc9_acid_cubic_image(uint32_t mag, int negative, uint32_t recip, int32_t d, int32_t jump, int32_t slope)
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
static int32_t rc9_acid_cubic_correction(uint32_t ph, uint32_t event, uint32_t inc, uint32_t recip, int32_t jump, int32_t slope)
{
    int32_t t=(int32_t)(ph-event), d=(int32_t)(inc>>8), y;
    uint32_t mag=t<0 ? 0u-(uint32_t)t : (uint32_t)t;
    if (mag>=2u*inc) return 0;
    y=rc9_acid_cubic_image(mag,t<0,recip,d,jump,slope);
    if (inc>0x40000000u) {
        uint64_t adjacent=0x100000000ull-mag;
        if (adjacent<2ull*inc)
            y+=rc9_acid_cubic_image((uint32_t)adjacent,t>=0,recip,d,jump,slope);
    }
    return y;
}
static int32_t rc9_acid_morph_eval(const rc9_acid_morph_mix_t *v, uint32_t ph, uint32_t inc, uint32_t recip)
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
    y+=rc9_acid_cubic_correction(ph,0x80000000u,inc,recip,v->jump,v->slope[0]);
    if (v->slope[1]) y+=rc9_acid_cubic_correction(ph,0xA0000000u,inc,recip,0,v->slope[1]);
    if (v->slope[2]) y+=rc9_acid_cubic_correction(ph,0xC0000000u,inc,recip,0,v->slope[2]);
    if (v->slope[3]) {
        y+=rc9_acid_cubic_correction(ph,0,inc,recip,0,v->slope[3]);
        y+=rc9_acid_cubic_correction(ph,0x40000000u,inc,recip,0,v->slope[4]);
    }
    return y;
}
/* Preserve the original saw/square arithmetic, including rounding/order. */
static inline int32_t rc9_acid_plain_wave(uint32_t ph, uint32_t inc, uint32_t wave)
{
    int32_t x;
    if (wave==1u) {
        x=(int32_t)(ph>>7)-(int32_t)((ph+0x80000000u)>>7);
        x-=rc9_acid_blep(ph,inc)-rc9_acid_blep(ph+0x80000000u,inc);
    } else if (wave==2u) {
        /* 25% positive duty, analytic DC removal, then gain 2/3:
         * levels +1 and -1/3, ideal RMS equal to the saw (1/sqrt(3)). */
        x=ph<0x40000000u ? 3*QONE/2 : -QONE/2;
        x+=rc9_acid_blep(ph,inc)-rc9_acid_blep(ph+0xC0000000u,inc);
        x=x*2/3;
    } else {
        x=(int32_t)(ph>>7)-QONE;
        x-=rc9_acid_blep(ph,inc);
    }
    return x;
}

static uint8_t rc9_acid_wave_last[NTRK];
static volatile uint8_t rc9_acid_wave_conflict;
static uint32_t rc9_acid_wave_guard(track_t *t); /* presets.c: one physical Morph owner */

static uint32_t rc9_acid_inc(int32_t p16)                   /* pitch in 1/16 semitone (0 = MIDI 0) -> increment */
{
    static const uint32_t FINE[16] = {65536, 65773, 66011, 66250, 66489, 66730, 66971, 67213,   /* 2^(i / 192), Q16 */
                                      67456, 67700, 67945, 68191, 68438, 68685, 68933, 69183};
    p16 = qlim(p16, 0, 127 * 16);
    return (uint32_t)(((uint64_t)qnote_inc(p16 >> 4) * FINE[p16 & 15]) >> 16);
}
static int32_t rc9_acid_pitch(const track_t *t)             /* the target: C4 + the note + TUNE, 1/16 semitone */
{
    return (60 + t->nt + t->p[P_E0]) * 16;
}
static uint32_t rc9_acid_decay(int32_t p)                   /* DECAY 0..127 -> DECAY_K 34..121: 30 ms .. 3 s to -60 dB */
{
    return dk(34 + clamp(p, 0, 127) * 87 / 127);
}

/* ACID: TUNE DECAY CUTOF RESO, ENVMD ACCNT WAVE GATE */
static void rc9_acid_trigger(track_t *t, dvoice_t *v)
{
    const int16_t *p = &t->p[P_E0];
    int32_t acc = v->vel > 110u ? clamp(p[5], 0, 127) * 258 : 0;   /* ACCNT (Q15) on an accented note */
    v->x[0] = v->x[1] = rc9_acid_pitch(t);
    uint32_t wave=rc9_acid_wave_guard(t);
    v->k[0]=(uint32_t)clamp(t->p[P_AMORPH],0,127)*65536u; /* smoothed Morph Q16 */
    v->k[2]=wave | (wave<<2);                         /* current/old wave, fade samples in bits 8.. */
    v->x[2] = 1 | (wave == 1u ? 4 : 0);                         /* the gate open, the attack running; WAVE */
    v->x[3] = acc;
    v->ph[0] = 0;
    v->env[0] = 0;
    v->env[1] = ENV1;
    if (acc)
        t->wow = qmin(t->wow + (acc << 9) / 2, ENV1);      /* the wow charges on accents, across notes */
    v->k[1] = rc9_acid_decay(acc ? clamp(p[1], 0, 127) / 3 : p[1]);   /* DECAY; an accent: shorter */
    v->f[0] = v->f[1] = v->f[2] = v->f[3] = 0;
}

static void rc9_acid_glide(track_t *t, dvoice_t *v)          /* slide: a new target, nothing restarts */
{
    const int16_t *p = &t->p[P_E0];
    v->x[1] = rc9_acid_pitch(t);
    v->t = 0;                                            /* a new note: its lifetime starts */
    v->x[3] = v->vel > 110u ? clamp(p[5], 0, 127) * 258 : 0;   /* the slid-to step's accent, or none */
    if (v->x[3])
        t->wow = qmin(t->wow + (v->x[3] << 9) / 2, ENV1);
}

static void rc9_acid_gate(track_t *t, dvoice_t *v, int open)  /* open: the attack resumes from where the amp is */
{
    (void)t;
    v->x[2] = open ? ((v->x[2] | 1) & ~2) : (v->x[2] & ~1);
}

/* the ladder's coefficients at filter envelope e: G = g / (1 + g), g = tan(pi fc / fs); *inv = 1 / (1 + k G^4) */
static int32_t rc9_acid_coef(const track_t *t, const dvoice_t *v, int32_t e, int32_t k, int32_t *inv)
{
    const int16_t *p = &t->p[P_E0];
    int32_t cut, g, G, G2;
    /* cutoff in 1/16 semitones over MIDI 16 (~20 Hz): CUTOF (0..103 semitones: ~20 Hz .. ~8 kHz) + ENVMD x env (up
     * to 48) + the accent (env x ACCNT, up to 36; the track's wow, up to 12) */
    cut = clamp(p[2], 0, 127) * 103 * 16 / 127 +
          (int32_t)(((int64_t)clamp(p[4], 0, 127) * 16 * 48 / 127 * env_q15(e)) >> 15) +
          (int32_t)(((int64_t)v->x[3] * 16 * 36 / 32767 * env_q15(e)) >> 15) +
          (env_q15(t->wow) * 16 * 12 >> 15);
    cut = qlim(cut, 0, 111 * 16);                          /* ~20 Hz .. ~12.5 kHz (MIDI 127) */
    g = qtan_acc((int32_t)(rc9_acid_inc(cut + 16 * 16) >> 8)); /* tan(pi fc / fs): fc / fs is inc / 2^32, Q24 */
    G = qdiv(g, QONE + g);
    G2 = qm(G, G);
    *inv = qdiv(QONE, QONE + qm(k, qm(G2, G2)));
    return G;
}

static void rc9_acid_render(track_t *t, dvoice_t *v, int32_t *out, uint32_t n)
{
    uint32_t i, j, inc, kn = v->k[1];
    int32_t d, G, inv, G1, inv1, dG, dinv, k, gain, gain1, dgain, s0 = v->f[0], s1 = v->f[1], s2 = v->f[2],
                                                                        s3 = v->f[3];
    int32_t amp = v->env[0];
    uint32_t wave=rc9_acid_wave_guard(t), current=v->k[2]&3u, old=(v->k[2]>>2)&3u, fade=v->k[2]>>8;
    uint32_t recip=0;
    rc9_acid_morph_mix_t morph;
    if (wave!=current) {old=current;current=wave;fade=128u;}

    /* pitch: toward the target in the log domain, at least 1/16 semitone a block (no stall short of it) */
    d = v->x[1] - v->x[0];
    if (d) {
        int32_t s = (int32_t)(((int64_t)d * ACID_GLIDE + (1 << 23)) >> 24);
        v->x[0] += s ? s : d > 0 ? 1 : -1;
    }
    inc = rc9_acid_inc(v->x[0]);
    if (current==3u || (fade && old==3u)) {
        recip=(uint32_t)((1ull<<48)/inc);
        rc9_acid_morph_prepare(&morph,(int32_t)v->k[0]);
    }
    /* the ladder's coefficients at the block's end (the filter envelope n samples on: DECAY^n, n a power of two as
     * CTL) and the output gain (velocity x voice level x gain x the accent, <= 122332); each ramps from where the
     * last block ended (a new voice: from its value now) */
    for (j = n; j > 1u; j >>= 1)
        kn = (uint32_t)(((uint64_t)kn * kn) >> 16);
    k = clamp(t->p[P_E0 + 3], 0, 127) * (ACID_KMAX / 127);  /* RESO: 0 .. 4.4 (self-oscillation from ~4, 116) */
    G1 = rc9_acid_coef(t, v, mulq16(v->env[1], kn), k, &inv1);
    gain1 = (int32_t)(((int64_t)mulq15(vel_gain(v), VOICE_FS) * ACID_GAIN * (ENV1 + (v->x[3] << 9) / 2)) >> 32);
    if (!(v->x[2] & 8)) {
        v->x[2] |= 8;
        v->c[0].a1 = rc9_acid_coef(t, v, v->env[1], k, &v->c[0].a2);
        v->c[0].a3 = gain1;
    }
    G = v->c[0].a1;
    inv = v->c[0].a2;
    gain = v->c[0].a3;
    dG = (G1 - G) / (int32_t)n;                            /* per block: three 32-bit divides */
    dinv = (inv1 - inv) / (int32_t)n;
    dgain = (gain1 - gain) / (int32_t)n;
    for (i = 0; i < n; i++) {
        int32_t x, S, y, u, a, G2, G3;
        uint32_t ph = v->ph[0];
        if (current==3u || (fade && old==3u)) {
            int32_t target=clamp(t->p[P_AMORPH],0,127)*65536, pos=(int32_t)v->k[0];
            if (!(i&3u) && pos!=target) {
                int32_t step=(127*65536/221)*(int32_t)qmin(4u,n-i);
                pos+=qlim(target-pos,-step,step);
                v->k[0]=(uint32_t)pos;
                rc9_acid_morph_prepare(&morph,pos);
            }
        }
        x=current==3u ? rc9_acid_morph_eval(&morph,ph,inc,recip) : rc9_acid_plain_wave(ph,inc,current);
        if (fade) {
            int32_t prev=old==3u ? rc9_acid_morph_eval(&morph,ph,inc,recip) : rc9_acid_plain_wave(ph,inc,old);
            x=prev+(int32_t)(((int64_t)(x-prev)*(128u-fade))/128);
            fade--;
        }
        v->ph[0] = ph + inc;
        x >>= 1;                                         /* into the ladder at half: the soft clip stays near
                                                          * linear for the oscillator, the feedback can saturate */
        G += dG;
        inv += dinv;
        gain += dgain;
        G2 = qm(G, G);
        G3 = qm(G2, G);
        /* the ladder (TPT): y4 = (G^4 x + S) / (1 + k G^4), S = (1 - G)(G^3 s0 + G^2 s1 + G s2 + s3); the soft
         * clip on the ladder's input (x - k y, the feedback subtracted); each one-pole: v = (u - s) G, y = v + s,
         * s = y + v */
        S = qm(QONE - G, qm(G3, s0) + qm(G2, s1) + qm(G, s2) + s3);
        y = qm(qm(qm(G3, G), x) + S, inv);
        u = rc9_acid_clip(x - qm(k, y));
        a = qm(u - s0, G); u = a + s0; s0 = u + a;
        a = qm(u - s1, G); u = a + s1; s1 = u + a;
        a = qm(u - s2, G); u = a + s2; s2 = u + a;
        a = qm(u - s3, G); u = a + s3; s3 = u + a;
        /* the amp: attack, hold while the gate is open, release */
        if (v->x[2] & 1) {
            if (!(v->x[2] & 2)) {
                amp += ACID_ATT;
                if (amp >= ENV1) {
                    amp = ENV1;
                    v->x[2] |= 2;
                }
            }
        } else {
            amp = mulq16(amp, ACID_REL);
        }
        a = (int32_t)(((int64_t)qm(u, amp) * gain) >> 24);   /* as dm_put, the gain folded in */
        out[i] += a;
        v->last = a;
        env_step(v, 1);                                  /* the filter envelope: DECAY */
    }
    v->f[0] = s0; v->f[1] = s1; v->f[2] = s2; v->f[3] = s3;
    v->c[0].a1 = G1;                                     /* the next block ramps from these */
    v->c[0].a2 = inv1;
    v->c[0].a3 = gain1;
    v->env[0] = amp;
    v->k[2]=current | (old<<2) | (fade<<8);
    v->t += n;
    if (v->t >= ACID_LIFE)
        v->x[2] &= ~1;
    if (!(v->x[2] & 1) && amp < ENV_END)
        dm_end(v);
}

static const char *const N_RC9_ACIDWAVE[4] = {"SAW", "SQUARE", "PULSE", "MORPH"};
#define RC9_DM_ACID_DEF {"ACID", 1, 0, {PD("TUNE", F_SEMI, -24, 24, 0), PD("DECAY", F_INT, 0, 127, 50), \
    PD("CUTOF", F_INT, 0, 127, 80), PD("RESO", F_INT, 0, 127, 30), PD("ENVMD", F_INT, 0, 127, 60), \
    PD("ACCNT", F_INT, 0, 127, 64), PE("WAVE", N_RC9_ACIDWAVE, 1), PD("GATE", F_INT, 0, 127, 40)}, rc9_acid_trigger, \
    rc9_acid_render, 1, rc9_acid_glide, rc9_acid_gate}

static uint32_t rc9_acid_wave_guard(track_t *t){return (uint32_t)clamp(t->p[P_E6],0,3);}
