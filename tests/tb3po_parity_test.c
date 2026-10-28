/* SPDX-License-Identifier: GPL-3.0-only */
/* Compile against production and the read-only pinned reference; compare bytes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#ifndef TB3PO_SOURCE
#define TB3PO_SOURCE "../firmware/src/tb3po.c"
#endif
#include TB3PO_SOURCE
int main(void)
{
    for (unsigned scale=0;scale<TB3PO_NSCALES;scale++)
        for (unsigned len=1;len<=64;len++)
            for (unsigned seed=0;seed<8;seed++) {
                bpart_t b={0};uint32_t r=0;
                b.len=(uint8_t)len;tb3po_defaults(&b.gen,seed*7919u);
                b.gen.scale=(uint8_t)scale;b.gen.root=(uint8_t)(len%12);
                b.gen.base_oct=(uint8_t)(seed%5);b.gen.oct_range=(uint8_t)(1+seed%3);
                b.gen.density=seed==0?0:seed==1?100:70;
                b.gen.accent=b.gen.slide=seed==2?0:seed==3?100:40;
                tb3po_generate(&b);fwrite(b.step,sizeof b.step,1,stdout);
                for(unsigned m=0;m<8;m++) {
                    tb3po_mutate(&b,&r);fwrite(b.step,sizeof b.step,1,stdout);fwrite(&r,sizeof r,1,stdout);
                }
            }
    return ferror(stdout)!=0;
}
