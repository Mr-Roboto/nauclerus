/* SPDX-License-Identifier: GPL-3.0-only */
#include "remix.h"
#include "nauclerus_remix_table.h"

int remix_lookup(unsigned bank, unsigned pattern, unsigned slot)
{
    unsigned index;
    if (bank >= REMIX_BANKS || pattern >= REMIX_PATTERNS || slot >= REMIX_BLOCK)
        return -1;
    index = (bank * REMIX_PATTERNS + pattern) * REMIX_BLOCK + slot;
    return (remix_packed[index >> 1] >> ((index & 1u) * 4u)) & 15u;
}

int remix_source_step(unsigned bank, unsigned pattern, unsigned chronological)
{
    int source;
    if (chronological >= REMIX_STEPS)
        return -1;
    source = remix_lookup(bank, pattern, chronological & 15u);
    return source < 0 ? -1 : (int)(chronological & ~15u) + source;
}
