#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <byteswap.h>

//#include <periphery/mmio.h>
#include "mmio.h"

#define CRU (0xff500000)

#define BV(r,s,m) ( ((r)>>(s)) & (m) )

// CRU
#define CLKGATE_13_R (0x0334)


int main(void) {
    mmio_t *mmio;
    uint32_t g13;

    mmio = mmio_new();
    if (mmio_open(mmio, CRU, 0x10000) < 0) {
        fprintf(stderr, "mmio_open(): %s\n", mmio_errmsg(mmio));
        exit(1);
    }

        if (mmio_read32(mmio, CLKGATE_13_R, &g13) < 0) {
            fprintf(stderr, "mmio_read32(): %s\n", mmio_errmsg(mmio));
            exit(1);
        }
        g13 |= 0x300030;
        if (mmio_write32(mmio, CLKGATE_13_R, g13) < 0) {
            fprintf(stderr, "mmio_write32(): %s\n", mmio_errmsg(mmio));
            exit(1);
        }
        if (mmio_read32(mmio, CLKGATE_13_R, &g13) < 0) {
            fprintf(stderr, "mmio_read32(): %s\n", mmio_errmsg(mmio));
            exit(1);
        }

    mmio_close(mmio);

    printf("gate13_4 = %u    gate13_5 = %u\n" , BV(g13,4,1), BV(g13,5,1));

    return 0;
}

