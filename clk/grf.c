#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <byteswap.h>

//#include <periphery/mmio.h>
#include "mmio.h"

#define GRF (0xff000000)
#define CRU (0xff500000)
#define I2S80 (0xff300000)

#define GRF_SOC_CON2_R (0x0308)
#define GRF_SOC_CON2_M (1 << 10)

#define CRU_CLKGATE_CON13 (0x0334)


int main(void) {
    mmio_t *mmio;
    uint32_t grf_soc_con2;

    mmio = mmio_new();

    if (mmio_open(mmio, GRF, 0x10000) < 0) {
        fprintf(stderr, "mmio_open(): %s\n", mmio_errmsg(mmio));
        exit(1);
    }

    if (mmio_read32(mmio, GRF_SOC_CON2_R, &grf_soc_con2) < 0) {
        fprintf(stderr, "mmio_read32(): %s\n", mmio_errmsg(mmio));
        exit(1);
    }

    mmio_close(mmio);

    return 0;
}

