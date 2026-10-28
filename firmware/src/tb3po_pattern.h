/* SPDX-License-Identifier: GPL-3.0-only */
/* TB-3PO data subset, unchanged from X0X-RMX v1.1.0 pattern.h. */
#pragma once
#include <stdint.h>
#define NSTEPS 64
enum { G_REST, G_NOTE, G_TIE };                             /* 303 step gate */

/* 303 step: one byte of note, one of flags */
#define BS_GATE_MASK 0x03u
#define BS_ACCENT 0x04u
#define BS_SLIDE 0x08u                /* slide INTO the next step (303 semantics) */
typedef struct {
    uint8_t note;                     /* MIDI note */
    uint8_t flags;                    /* gate | BS_ACCENT | BS_SLIDE */
} bstep_t;

/* TB-3PO settings of one 303 line: the line is reproducible from these and the seed */
typedef struct {
    uint8_t density, accent, slide;   /* probabilities 0..100 % */
    uint8_t oct_range;                /* 1..3 */
    uint8_t root;                     /* 0 = C .. 11 = B */
    uint8_t scale;                    /* tb3po.h SCALES */
    uint8_t base_oct;                 /* octave of the root: note = 12 * (base_oct + 1) + root */
    uint8_t mutate_bars;              /* auto-mutate every N bars, 0 = off */
    uint32_t seed;
} tb3po_cfg_t;

typedef struct {
    bstep_t step[NSTEPS];
    uint8_t len;                      /* 1..64 */
    uint8_t rate;                     /* RATE_* */
    uint8_t dir;                      /* DIR_* */
    uint8_t transpose;                /* semitones + 24 (24 = none), applied when played */
    tb3po_cfg_t gen;
} bpart_t;


static inline int bstep_gate(const bstep_t *s) { return s->flags & BS_GATE_MASK; }
