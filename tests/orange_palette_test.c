/* SPDX-License-Identifier: GPL-3.0-only
 * Nauclerus 1.1: verify vivid ORANGE LCD palettes and unchanged theme IDs.
 * Uses the firmware's actual two-tone color definitions.
 */
#include "ui_host.h"

static unsigned failures;
static void verify(const char *what, int condition)
{
    printf("%s  %s\n", condition ? "ok  " : "FAIL", what);
    failures += !condition;
}

int main(void)
{
    ui_host_init();

    verify("stored ORANGE and ORANGE INV theme IDs remain 0 and 1",
           theme_index(THEME_TAG | 0u) == 0u &&
           theme_index(THEME_TAG | 1u) == 1u);

    theme_set(THEME_TAG | 0u);
    verify("ORANGE: vivid RGB565 #FF6500 background, near-black #120804 ink",
           C_BG == RGB(0xff, 0x65, 0x00) &&
           C_INK == RGB(0x12, 0x08, 0x04));
    verify("ORANGE differs from prior muted #D45E31",
           C_BG != RGB(0xd4, 0x5e, 0x31));

    theme_set(THEME_TAG | 1u);
    verify("ORANGE INV: vivid RGB565 #FF6500 ink, near-black #100604 background",
           C_BG == RGB(0x10, 0x06, 0x04) &&
           C_INK == RGB(0xff, 0x65, 0x00));

    verify("theme menu order and other palettes remain available",
           theme_step(THEME_TAG | 0u, +1) == (THEME_TAG | 1u) &&
           theme_index(THEME_TAG | 2u) == 2u &&
           THEMES[2].bg == RGB(0x24, 0x45, 0xc8) &&
           (sizeof THEMES / sizeof THEMES[0]) == 14u);
    printf(failures ? "orange_palette_test: %u FAILED\n" :
                      "orange_palette_test: all checks passed\n", failures);
    return failures ? 1 : 0;
}
