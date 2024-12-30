#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <byteswap.h>

//#include <periphery/mmio.h>
#include "mmio.h"

#define I2S80 (0xff300000)

#define BV(r,s,m) ( ((r)>>(s)) & (m) )

// I2S
#define INTCR_R (0x0014)
#define INTSR_R (0x0018)

int main(void) {
    mmio_t *mmio;
    uint32_t intcr, intsr, level;

    mmio = mmio_new();
    if (mmio_open(mmio, I2S80, 0x10000) < 0) {
        fprintf(stderr, "mmio_open(): %s\n", mmio_errmsg(mmio));
        exit(1);
    }

        if (mmio_read32(mmio, INTCR_R, &intcr) < 0) {
            fprintf(stderr, "mmio_read32(): %s\n", mmio_errmsg(mmio));
            exit(1);
        }
        if (mmio_read32(mmio, INTSR_R, &intsr) < 0) {
            fprintf(stderr, "mmio_read32(): %s\n", mmio_errmsg(mmio));
            exit(1);
        }

    mmio_close(mmio);

    printf( "level=%u   Enabled: TXUI=%u  TXEI=%u\n", BV(intcr,4,31), BV(intcr,1,1), BV(intcr,0,1) );
    printf( "level=%u   Status: TXUI=%u  TXEI=%u\n", BV(intcr,4,31), BV(intsr,1,1), BV(intsr,0,1) );

    return 0;
}
