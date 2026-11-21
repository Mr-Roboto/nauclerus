/* SPDX-License-Identifier: GPL-3.0-only
 * MIDI Clock Out / TRS->USB Thru, adapted selectively from DEADACTIVE/Hortator.
 * Nauclerus standard build: USB MIDI output only; TRS connector is MIDI input.
 * The schedule uses the audio half IRQ's TIMER4 time (not wall-clock timer).
 * USB clocks never go back to the USB input that supplied the original pulses.
 * The updater's SysEx traffic suspends SYNC and THRU; no extra BLE/Editor code.
 */
#define MOUT_Q 16u                                  /* the scheduled Start / Clock / Stop */
#define MOUT_TPS_Q16 35665605u                      /* TIMER4 ticks a sample (24 MHz / 44.1 kHz), Q16 */
#define MOUT_HALF_TICKS ((uint32_t)(((uint64_t)HALF_FRAMES * MOUT_TPS_Q16) >> 16))
#define MOUT_TICK 2400u                             /* TIMER5: 100 us */
#define MOUT_PAUSE_MS 3000u                         /* after an update's last SysEx */
#define MOUT_CLOCK_ROOM 8u                          /* a Clock only while the USB ring holds fewer packets: a computer not
                                                     * reading gets no backlog of stale Clocks, the keys' notes keep room */
#define SY_ALL SY_USB                         /* Standard Nauclerus 1.1: USB output only */

static volatile uint8_t midi_thru;                  /* GLOBAL 3/7 THRU (settings: project.c, the palette word's bit 14) */
static struct {
    uint32_t due[MOUT_Q];                           /* TIMER4 tick */
    uint8_t st[MOUT_Q];                             /* F8 FA FC */
    volatile uint32_t w, r;                         /* the audio ISR writes w, TIMER5 r */
    uint32_t t0, blk, r_ticks;                      /* the half's IRQ, its samples before this block, the last render */
    uint32_t pos, pair, pulses;                     /* samples into the 1/8 (gclk.cnt / 2: pair), Clocks counted */
    uint8_t j, run;                                 /* the 1/8's next Clock (0..12); the own clock's transport running */
    uint8_t sub, owed;                              /* main.c fm1_timer5_irq's sub and owed (USB), mirrored */
    uint8_t sx_seen;
    uint32_t fw, sx_ms;                             /* the input ring read up to here; the last SysEx seen */
    uint32_t sent, drops, fwd;                      /* console: packets out, refused, forwarded */
} mout;

static uint32_t mout_port(uint32_t src)            /* the output an input's port is (midi_in_source) */
{
    return src == 1u ? SY_USB : src == 3u ? (uint32_t)SY_ALL & 2u : 0u;
}

/* one packet to ports (SY_ bits); a Clock (rt) only where the USB ring has room for it */
static void mout_send(uint32_t pkt, uint32_t ports, int rt)
{
    if (ports & SY_USB) {
        if (!rt || mo_w - mo_r < MOUT_CLOCK_ROOM)
            midi_out_event(pkt);                    /* USB-MIDI: no TRS output on the FM-1 */
        else
            mout.drops++;
    }
    mout.sent++;
}

/* ---- the audio ISR ---- */
static void mout_half(uint32_t t0)                  /* audio.c: a half's render begins (its IRQ's tick) */
{
    mout.t0 = t0;
    mout.blk = 0;
}
static void mout_half_done(uint32_t t1)             /* .. and ends */
{
    mout.r_ticks = t1 - mout.t0;
}
static void mout_push(uint32_t st, uint32_t off)    /* st at sample off of this block */
{
    uint32_t w = mout.w;
    if (w - mout.r >= MOUT_Q) {
        mout.drops++;
        return;
    }
    mout.due[w % MOUT_Q] = mout.t0 + (uint32_t)(((uint64_t)(HALF_FRAMES + mout.blk + off) * MOUT_TPS_Q16) >> 16);
    mout.st[w % MOUT_Q] = (uint8_t)st;
    RING_PUBLISH();
    mout.w = w + 1u;
}
static void mout_clock(uint32_t off)
{
    mout.pulses++;
    if (song.g[G_SYNC])
        mout_push(0xF8u, off);
}
/* the Clocks of this block (adv samples, after grids_tick): the 1/8's j-th at j / 24 of a beat into it */
static void mout_pulses(uint32_t adv)
{
    uint32_t beat = beat_samples(), pair = gclk.cnt >> 1, base = 0, p, x;
    if (pair != mout.pair) {                        /* a 1/8 began in this block, gclk.pos samples before its end: the
                                                     * last 1/8's Clocks not sent yet first (a tempo change) */
        uint32_t e = adv > gclk.pos ? adv - gclk.pos : 0u;
        for (; mout.j < 12u; mout.j++) {
            x = mout.j * beat / 24u;
            x = x > mout.pos ? x - mout.pos : 0u;
            mout_clock(x < e ? x : e);
        }
        mout.pair = pair;
        mout.j = 0;
        mout.pos = 0;
        base = e;
        p = gclk.pos;
    } else {
        p = mout.pos + adv;
    }
    for (; mout.j < 12u; mout.j++) {
        x = mout.j * beat / 24u;
        if (x >= p)
            break;
        mout_clock(base + (x > mout.pos ? x - mout.pos : 0u));
    }
    mout.pos = p;
}
/* events_block, every block (n samples); run: the steps moved (seq_n = n on the own clock) */
static __attribute__((noinline)) void mout_block(uint32_t n, uint32_t run)
{
    if (run && mout.run && song.playing && !song.g[G_CLOCK])
        mout_pulses(n);
    mout.blk += n;
}
static __attribute__((noinline)) void mout_start(void)   /* seq_start (block start) */
{
    if (song.g[G_CLOCK]) {                          /* following: the source's Start goes through as it is */
        mout.run = 0;
        return;
    }
    if (song.g[G_SYNC]) {
        if (mout.run)
            mout_push(0xFCu, 0);                    /* PLAY while playing: from the top */
        mout_push(0xFAu, 0);
    }
    mout.run = 1;
    mout.pair = 0xFFFFFFFFu;                        /* the first 1/8 begins with the first block */
    mout.j = 12;
    mout.pos = 0;
}
static __attribute__((noinline)) void mout_stop(void)   /* seq_stop */
{
    if (mout.run && song.g[G_SYNC])
        mout_push(0xFCu, 0);
    mout.run = 0;
}

/* ---- TIMER5, outside the render ---- */
static int mout_paused(void)
{
#if FELUCCA_OTA
    /* While an update uses SysEx or its reply ring, suppress clocks and THRU.
     * This firmware has no editor SysEx exception: conservative and safe. */
    if (usb.ota_req || usb.sx_on || sx_collect || sx_ready || sx_busy || so_w != so_r) {
        mout.sx_ms = fm1_ms;
        mout.sx_seen = 1;
        return 1;
    }
    return mout.sx_seen && fm1_ms - mout.sx_ms < MOUT_PAUSE_MS;
#else
    return 0;
#endif
}
/* the input ring's new packets (USB, TRS, BLE: usb_app.c, midi_uart.c, bt_midi.c fill it), as they are read: the
 * followed source's realtime to the SYNC outputs, THRU's channel messages to the other outputs. Never back to their
 * port. (The ring's writers run in TIMER5 or with the IRQs off: none runs during this.) */
static void mout_scan(void)
{
    uint32_t sync = (uint32_t)song.g[G_SYNC] & SY_ALL, clk = (uint32_t)song.g[G_CLOCK];
    while (mout.fw != mi_w) {
        uint32_t at, pkt, src;
        if (mi_w - mout.fw > MQ)
            mout.fw = mi_w - MQ;                    /* (overwritten: lost) */
        at = mout.fw % MQ;
        pkt = midi_in_q[at];
        src = midi_in_source[at] ? midi_in_source[at] : 1u;
        mout.fw++;
        if (((pkt >> 8) & 0xFFu) >= 0xF8u) {
            if (clk && clk == src && sync) {
                mout_send(pkt, sync & ~mout_port(src), 1);
                mout.fwd++;
            }
        } else if (midi_thru) {
            mout_send(pkt, SY_ALL & ~mout_port(src), 0);
            mout.fwd++;
        }
    }
}
/* the Clocks due nearer to this USB poll than to the next send (the next poll, or the first after the coming render) */
static void mout_release(uint32_t now)
{
    uint32_t q = now + (1u + (5u - mout.sub % 5u) % 5u) * MOUT_TICK, t_next = mout.t0 + MOUT_HALF_TICKS, lim;
    uint32_t sync = (uint32_t)song.g[G_SYNC] & SY_ALL;
    if ((int32_t)(t_next - now) > 0 && (int32_t)(q - t_next) >= 0)
        q = t_next + mout.r_ticks + MOUT_TICK;
    lim = now + (q - now) / 2u;
    while (mout.r != mout.w) {
        uint32_t i = mout.r % MOUT_Q;
        if ((int32_t)(mout.due[i] - lim) > 0)
            break;
        if (sync)
            mout_send(0x0Fu | (uint32_t)mout.st[i] << 8, sync, 1);
        RING_PUBLISH();
        mout.r++;
    }
}
/* every TIMER5 tick, before main.c's handler (felucca.c); in_audio: nested in the render */
static void mout_tick(uint32_t now, uint32_t in_audio)
{
    uint32_t poll;
    if (mout.sub % 5u == 0u)                        /* as main.c: USB every 5th tick, owed while the render runs */
        mout.owed = 1;
    if (++mout.sub == 10u)
        mout.sub = 0;
    if (in_audio)
        return;
    poll = mout.owed;
    mout.owed = 0;
    if (mout_paused()) {
        mout.r = mout.w;
        mout.fw = mi_w;
        return;
    }
    mout_scan();
    if (poll && mout.r != mout.w)
        mout_release(now);
}
