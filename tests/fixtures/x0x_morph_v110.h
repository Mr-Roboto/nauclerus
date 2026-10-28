/* SPDX-License-Identifier: GPL-3.0-only */
/* X0X-RMX v1.1.0 a4e78f3a6e16fa310b7614e5876d07e16a0020f9,
 * bass303.c Morph evaluator. Only private symbol/libm adapter names differ. */
/* Read-only mathematical evaluator from pinned X0X-RMX v1.1.0. */
static float reference_rsqrt(float x){return 1.0f/sqrtf(x);}
static float reference_clamp(float x,float lo,float hi){return x<lo?lo:x>hi?hi:x;}
typedef struct {
    float shark, trap, notch, dc, jump, slope[5];
} reference_bass303_morph_mix_t;

/* Signed event reference_distance avoids rounding frac(phase+.5) near one and
 * amplifying that rounding by 1/d. Equivalent to the documented residuals
 * for d<.5, including coincident supports from different events. */
static inline float reference_distance(float p, float event)
{
    float t = p - event;
    if (t < -0.5f) t += 1.0f;
    if (t > 0.5f) t -= 1.0f;
    return t;
}
/* Four equal boxes of phase width d convolve to a cubic B-spline kernel.
 * tail(r) is its integral from r*d to infinity, r >= 0. The BLEP residual
 * for a 2-high step is +/-2*tail. The BLAMP is d*integral_r^infinity tail.
 * Use the far piece's small powers near support boundaries, avoiding the
 * cancellation in a generic truncated-power implementation. */
static inline void reference_cubic_residual(float t, float d, int need_step, float *step, float *slope)
{
    float r = fabsf(t), tail, ramp;
    *step = *slope = 0.0f;
    if (r >= 2.0f * d) return;
    r /= d;
    if (r >= 1.0f) {
        float x = 2.0f - r, x2 = x * x, x4 = x2 * x2;
        tail = need_step ? x4 * (1.0f / 24.0f) : 0.0f;
        ramp = x4 * x * (1.0f / 120.0f);
    } else {
        float r2 = r * r;
        tail = need_step ? 0.5f + r * (-2.0f / 3.0f
               + r2 * (1.0f / 3.0f - r * 0.125f)) : 0.0f;
        ramp = 7.0f / 30.0f + r * (-0.5f + r * (1.0f / 3.0f
               + r2 * (-1.0f / 12.0f + r * (1.0f / 40.0f))));
    }
    *step = t < 0.0f ? 2.0f * tail : -2.0f * tail;
    *slope = d * ramp;
}
static float reference_cubic_active_correction(float t, float d, float jump, float slope)
{
    float b, r, y;
    reference_cubic_residual(t, d, jump != 0.0f, &b, &r);
    y = jump * b + slope * r;
    /* For d>.25 the support exceeds a half-cycle. Periodic images of the
     * SAME event can overlap, in addition to overlaps between different events.
     * For 0<d<.5 only this adjacent image can contribute. */
    if (d > 0.25f) {
        reference_cubic_residual(t < 0.0f ? t + 1.0f : t - 1.0f, d, jump != 0.0f, &b, &r);
        y += jump * b + slope * r;
    }
    return y;
}
static inline float reference_cubic_correction(float t, float d, float jump, float slope)
{
    /* Keep the very common unsupported event cheap even if the compiler
     * chooses not to inline the larger polynomial/image helper. A nearest
     * image outside support implies every other image is outside as well. */
    if (fabsf(t) >= 2.0f * d) return 0.0f;
    return reference_cubic_active_correction(t, d, jump, slope);
}
static float reference_precise_compensation(float variance)
{
    float c;
    if (variance == 1.0f) return 1.0f;
    c = reference_rsqrt(variance);
    /* One additional Newton step, outside the sample loop. The exact unity
     * branch makes converging endpoint weights use exact endpoint gain. */
    return c * (1.5f - 0.5f * variance * c * c);
}
static void reference_morph_prepare(reference_bass303_morph_mix_t *v, float m)
{
    const float gs = 0.6454972243679028f, gt = 0.7071067811865475f;
    const float gn = 0.8295613557843402f;
    float x, w, c;
    int endpoint;
    m = reference_clamp(m, 0.0f, 127.0f);
    endpoint = m == 0.0f ? 0 : m == 64.0f ? 1 : m == 127.0f ? 2 : -1;
    v->shark = v->trap = v->notch = 0.0f;
    if (endpoint >= 0) {
        if (endpoint == 0) v->shark = gs;
        else if (endpoint == 1) v->trap = gt;
        else { v->trap = gn; v->notch = gn; }
    } else if (m < 64.0f) {
        x = m / 64.0f;
        w = x * x * (3.0f - 2.0f * x);
        c = reference_precise_compensation((1.0f - w) * (1.0f - w) + w * w
                     + 2.0f * 0.3423265984407288f * w * (1.0f - w));
        v->shark = c * (1.0f - w) * gs;
        v->trap = c * w * gt;
    } else {
        x = (m - 64.0f) / 63.0f;
        w = x * x * (3.0f - 2.0f * x);
        c = reference_precise_compensation((1.0f - w) * (1.0f - w) + w * w
                     + 2.0f * 0.9532062476387964f * w * (1.0f - w));
        v->trap = c * ((1.0f - w) * gt + w * gn);
        v->notch = c * w * gn;
    }
    v->dc = v->notch * 0.125f;
    v->jump = 1.5f * v->shark;
    v->slope[0] = 6.0f * v->shark - 8.0f * v->trap - 8.0f * v->notch;
    v->slope[1] = 16.0f * v->notch;
    v->slope[2] = -8.0f * v->trap - 8.0f * v->notch;
    v->slope[3] = v->slope[4] = 8.0f * v->trap;
}
static float reference_morph_eval(const reference_bass303_morph_mix_t *v, float phase, float d)
{
    float u = phase < 0.5f ? phase + 0.5f : phase - 0.5f;
    float y = v->dc;
    if (v->shark != 0.0f) y += v->shark * (1.0f - 3.0f * u * u);
    if (v->trap != 0.0f) {
        float t = u < 0.25f ? 1.0f : u < 0.5f ? 3.0f - 8.0f * u
                  : u < 0.75f ? -1.0f : 8.0f * u - 7.0f;
        y += v->trap * t;
    }
    if (v->notch != 0.0f && u < 0.25f)
        y -= v->notch * (u < 0.125f ? 8.0f * u : 2.0f - 8.0f * u);
    {
        float t = reference_distance(phase, 0.5f);
        if (v->jump != 0.0f) y -= v->shark * d * d;
        y += reference_cubic_correction(t, d, v->jump, v->slope[0]);
        if (v->slope[1] != 0.0f)
            y += reference_cubic_correction(reference_distance(phase, 0.625f), d, 0.0f, v->slope[1]);
        if (v->slope[2] != 0.0f)
            y += reference_cubic_correction(reference_distance(phase, 0.75f), d, 0.0f, v->slope[2]);
        if (v->slope[3] != 0.0f) {
            y += reference_cubic_correction(reference_distance(phase, 0.0f), d, 0.0f, v->slope[3]);
            y += reference_cubic_correction(reference_distance(phase, 0.25f), d, 0.0f, v->slope[4]);
        }
    }
    return y;
}
