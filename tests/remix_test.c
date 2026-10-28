/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "remix.h"

int main(void)
{
    static const unsigned lengths[] = {1, 8, 16, 24, 40, 64};
    static const unsigned invalid[] = {16, 64, UINT_MAX};
    unsigned row = 0, values = 0, repeats = 0, beyond_length = 0;
    char line[256];
    FILE *f = fopen("assets/remix/remixes.txt", "r");
    assert(f);
    while (fgets(line, sizeof line, f)) {
        unsigned expected[16], slot, seen = 0, bank, pattern, n;
        char *p = line, *end;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\n' || !*p) continue;
        assert(row < 256);
        bank = row / 16;
        pattern = row % 16;
        for (slot = 0; slot < 16; slot++) {
            long v = strtol(p, &end, 10);
            assert(end != p && v >= 0 && v <= 15);
            p = end;
            expected[slot] = (unsigned)v;
            assert(remix_lookup(bank, pattern, slot) == v);
            if (seen & (1u << v)) repeats++;
            seen |= 1u << v;
            if (!row) assert(v == (long)slot);
            values++;
        }
        assert(expected[0] == 0);
        for (n = 0; n < sizeof lengths / sizeof lengths[0]; n++) {
            unsigned c;
            for (c = 0; c < lengths[n]; c++) {
                unsigned base = c & ~15u;
                int source = remix_source_step(bank, pattern, c);
                assert(source == (int)(base + expected[c & 15u]));
                assert(source >= (int)base && source < (int)(base + 16u));
                assert(source >= 0 && source < 64);
                if (!row) assert(source == (int)c);
                beyond_length += source >= (int)lengths[n];
            }
        }
        row++;
    }
    assert(!ferror(f) && row == 256 && values == 4096);
    fclose(f);
    assert(repeats > 0 && beyond_length > 0);
    for (unsigned i = 0; i < sizeof invalid / sizeof invalid[0]; i++) {
        unsigned v = invalid[i];
        assert(remix_lookup(v, 0, 0) == -1);
        assert(remix_lookup(0, v, 0) == -1);
        assert(remix_lookup(0, 0, v) == -1);
        assert(remix_source_step(v, 0, 0) == -1);
        assert(remix_source_step(0, v, 0) == -1);
    }
    assert(remix_source_step(0, 0, 64) == -1);
    assert(remix_source_step(0, 0, UINT_MAX) == -1);
    assert(remix_source_step(UINT_MAX, UINT_MAX, UINT_MAX) == -1);
    puts("remix: all 4096 values, A1, repeats, every bank/pattern, four blocks, lengths 1/8/16/24/40/64 and invalid inputs passed");
    return 0;
}
