/* SPDX-License-Identifier: GPL-3.0-only
 * Focused Nauclerus 1.1 tempo-delay regression: exercise the production
 * parameter descriptor/encoder ordering and FX time calculation on the host.
 * No hardware, flash, installer or project writes.
 */
#include "drum_host.h"

static unsigned failures;

static void verify(const char *description, int passed)
{
    printf("%s  %s\n", passed ? "ok  " : "FAIL", description);
    failures += !passed;
}

int main(void)
{
    static const char *const legacy[10] = {
        "1/4", "1/8", "1/16", "1/32", "8T", "16T",
        "1/2", "1/1", "2BAR", "4BAR"
    };
    static const uint8_t ordered[15] = {
        9, 8, 7, 6, 11, 0, 12, 10, 1, 13, 4, 2, 5, 3, 14
    };
    const uint32_t quarter = (uint32_t)FS * 60u / 120u;
    const uint32_t samples[15] = {
        quarter, quarter / 2u, quarter / 4u, quarter / 8u,
        quarter / 3u, quarter / 6u, quarter * 2u, quarter * 4u,
        quarter * 8u, quarter * 16u,
        quarter * 2u / 3u, quarter * 3u / 2u,
        quarter * 3u / 4u, quarter * 3u / 8u, quarter / 12u
    };
    const param_desc_t *d = &GP[G_DTIME];
    unsigned i;
    int old_ok = 1, timing_ok = 1, order_ok = 1, rank_ok = 1;

    host_init_live();                          /* new project: uses the actual production initializer */
    verify("new project defaults TIME to dotted eighth 8D (stored ID 12)",
           d->def == 12 && song.g[G_DTIME] == 12 && strcmp(N_DTIME[song.g[G_DTIME]], "8D") == 0);
    verify("saved regular eighth keeps its distinct stored ID 1",
           strcmp(N_DTIME[1], "1/8") == 0 && div_samples(1) == quarter / 2u);
    song.g[G_CLOCK] = 0;
    song.g[G_BPM] = 120;

    verify("TIME uses 15 dedicated delay selections",
           d->names == N_DTIME && d->min == 0 && d->max == 14);
    for (i = 0; i < 10u; i++)
        old_ok &= strcmp(N_DIV[i], legacy[i]) == 0 &&
                  strcmp(N_DTIME[i], legacy[i]) == 0;
    verify("legacy division IDs 0..9 and names are unchanged", old_ok);
    verify("LCD dotted labels are separate and fit four characters",
           strcmp(N_DTIME[11], "4D") == 0 &&
           strcmp(N_DTIME[12], "8D") == 0 &&
           strcmp(N_DTIME[13], "16D") == 0);

    for (i = 0; i < 15u; i++) {
        timing_ok &= div_samples(i) == samples[i];
        if (i) {
            order_ok &= samples[ordered[i - 1u]] >= samples[ordered[i]];
            order_ok &= param_turn(d, ordered[i - 1u], 1) == ordered[i];
            order_ok &= param_turn(d, ordered[i], -1) == ordered[i - 1u];
        }
        rank_ok &= enum_rank(d, ordered[i]) == (int32_t)i;
    }
    verify("all 15 delay IDs compute the expected time at 120 BPM", timing_ok);
    verify("encoder steps run longest to shortest and reverse correctly", order_ok);
    verify("all 15 displayed ranks including the last are reachable", rank_ok);
    verify("encoder clamps at the first and last division",
           param_turn(d, ordered[0], -1) == ordered[0] &&
           param_turn(d, ordered[14], 1) == ordered[14]);

    song.g[G_BPM] = 40;
    song.g[G_DTIME] = 11; /* dotted quarter, 2.25s: beyond ring */
    verify("40 BPM dotted quarter clamps to existing ring length",
           div_samples(11) > DLY_LEN - 1u &&
           delay_samples() == DLY_LEN - 1u);
    song.g[G_DTIME] = 12;
    verify("40 BPM dotted eighth fits without clamp",
           delay_samples() == ((uint32_t)FS * 60u / 40u) * 3u / 4u);
    song.g[G_DTIME] = 13;
    verify("40 BPM dotted sixteenth fits without clamp",
           delay_samples() == ((uint32_t)FS * 60u / 40u) * 3u / 8u);
    verify("existing delay ring capacity stays 65,536 samples", DLY_LEN == 65536u);

    printf(failures ? "delay_time_test: %u FAILED\n" :
                      "delay_time_test: all checks passed\n", failures);
    return failures ? 1 : 0;
}
