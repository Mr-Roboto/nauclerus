/* SPDX-License-Identifier: GPL-3.0-only
 * Nauclerus 1.1: PROJECT/TOOLS GO action confirmation usability.
 * Uses the production UI and host render, not a simulated confirmation handler.
 */
#include "ui_host.h"

static unsigned fails;
static void chk(const char *what, int ok) {
    printf("%s  %s\n", ok ? "ok  " : "FAIL", what);
    fails += !ok;
}

int main(void)
{
    unsigned i;
    ui_host_init();
    open_family(FAM_SAVE);
    chk("PROJECT opens for the saved project actions",
        str_eq(cur_page()->title, "PROJECT"));

    turn(EN_K4, 1);
    ui_frame();
    chk("first SAVE turn arms without saving, ~4 s remain",
        ui.arm == G_SAVE && ui.arm_t >= 260u && ui.arm_t <= UI_GO_CONFIRM_FRAMES);
    chk("AGAIN: SAVE popup stays visible for the whole arm window",
        str_eq(ui.msg, "AGAIN: SAVE") && ui.msg_t == ui.arm_t);
    for (i = 0; i < 220u; i++)
        ui_frame();
    chk("SAVE remains armed after more than 3 seconds",
        ui.arm == G_SAVE && ui.arm_t > 0 && ui.msg_t == ui.arm_t);
    for (i = 0; i < 100u; i++)
        ui_frame();
    chk("expired SAVE silently disarms and hides its prompt",
        ui.arm == 0u && ui.arm_t == 0u && ui.msg_t == 0u);

    open_family(FAM_SAVE);                      /* PROJECT -> TOOLS */
    chk("TOOLS page opens for clear and init actions",
        str_eq(cur_page()->title, "TOOLS"));
    trk[0].step[0].on = 1u;
    turn(EN_K1, 1);                            /* CLRSQ first turn */
    ui_frame();
    chk("first CLRSQ turn arms but retains the pattern",
        ui.arm == G_CLRSEQ && trk[0].step[0].on);
    for (i = 0; i < 220u; i++)
        ui_frame();
    turn(EN_K1, 1);                            /* second turn within 4 s */
    ui_frame();
    chk("late second turn clears, disarms and shows completion",
        !trk[0].step[0].on && !ui.arm && !ui.arm_t && str_eq(ui.msg, "PATTERN CLEARED"));

    song.g[G_BPM] = 137;
    turn(EN_K4, 1);                            /* INIT ALL first turn */
    ui_frame();
    for (i = 0; i < 300u; i++)
        ui_frame();
    chk("INIT ALL expires without changing the live project",
        song.g[G_BPM] == 137 && ui.arm == 0);
    turn(EN_K4, 1);
    ui_frame();
    chk("first INIT ALL turn after timeout only re-arms",
        song.g[G_BPM] == 137 && ui.arm == G_INITALL);

    printf(fails ? "confirm_window_test: %u FAILED\n" :
                   "confirm_window_test: all checks passed\n", fails);
    return fails ? 1 : 0;
}
