/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once

enum { REMIX_BANKS = 16, REMIX_PATTERNS = 16, REMIX_BLOCK = 16, REMIX_STEPS = 64 };

/* Bank A..P and pattern 1..16 are internally 0..15. Invalid input returns -1.
 * Lookup returns a source offset 0..15; A1 is identity. No timing/state dependencies. */
int remix_lookup(unsigned bank, unsigned pattern, unsigned slot);

/* Chronological position 0..63 maps within its own 16-step block. Pattern length
 * is the caller's concern: never clamp sources to a partial block's playing length.
 * Invalid bank, pattern or chronological position returns -1. */
int remix_source_step(unsigned bank, unsigned pattern, unsigned chronological);
