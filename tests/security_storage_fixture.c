/* SPDX-License-Identifier: GPL-3.0-only */
/* Actual production project/lock saves -> simulated flash for the real loader. */
#define main existing_boot_tests
#include "boot_test.c"
#undef main
#include <assert.h>

int main(int argc, char **argv)
{
    assert(argc == 2);
    fresh_flash(); wipe_ram();
    for (unsigned k=0;k<NTRK;k++)
        for (unsigned n=0;n<NSTEP;n++) {
            trk[k].step[n].on = (n+k)%3 != 0;
            lock_set(k,n,P_E0,(int)((n+k)%128));
            lock_set(k,n,P_E1,(int)((n*3+k)%128));
        }
    assert(mo.b.count == 1024);
    for (unsigned pass=0;pass<2;pass++)
        for (unsigned slot=0;slot<4;slot++) {
            for (unsigned k=0;k<NTRK;k++) assert(seq_remix_apply(k,(slot*61+k*19+pass)%257));
            project_save(slot);
            assert(proj_ok(&proj_slot[slot]) && str_eq(ui.msg,"SAVED"));
            for (unsigned copy=0;copy<2;copy++) {
                uint32_t off=st_sector(OBJ_LOCKS0+slot,copy);
                /* The loader's record probe cannot match an erased lock-bank tail. */
                for (unsigned i=0;i<80;i++) assert(hflash[off+0xF00+i]==0xFF);
            }
        }
    assert(!host_fl_stray);
    FILE *f=fopen(argv[1],"wb");assert(f);
    assert(fwrite(hflash,1,sizeof hflash,f)==sizeof hflash);assert(!fclose(f));
    puts("security storage: production A-D project/1024-lock saves, both copies and erased lock tails passed");
    return 0;
}
