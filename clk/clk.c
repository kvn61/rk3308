#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <byteswap.h>

//#include <periphery/mmio.h>
#include "mmio.h"

#define GRF (0xff000000)
#define CRU (0xff500000)
#define I2S80 (0xff300000)
#define GPIO2 (0xff450000)

#define BV(r,s,m) ( ((r)>>(s)) & (m) )

// CRU
#define CLKSEL_52_R (0x01d0)
#define CLKSEL_53_R (0x01d4)
#define CLKSEL_54_R (0x01d8)
#define CLKSEL_55_R (0x01dc)
#define CLKGATE_10_R (0x0328)
#define CLKGATE_11_R (0x032c)
#define CLKGATE_13_R (0x0334)

//GRF
#define SOC_2_R (0x0308)

// I2S
#define TXCR_R (0x0000)
#define CKR_R (0x0008)
#define CLKDIV_R (0x0038)

//GPIO DDR
#define DDR_R (0x0004)
// mclk gpio2_a4 bit 4

//gate10_12 gate10_13 sel52_10_2 sel52_12_1 gate10_14 gate10_15 gate13_4
//BV(g10,12,1) BV(g10,13,1) BV(s52,10,3) BV(s52,12,1) BV(g10,14,1) BV(g10,15,1) BV(g13,4,1)

//gate11_0  gate11_1  sel54_10_2 sel54_12_1 gate11_2  gate11_3  gate13_5
//BV(g11,0,1) BV(g11,1,1) BV(s54,10,3) BV(s54,12,1) BV(g11,2,1) BV(g11,3,1) BV(g13,5,1)

//soc2_10
//BV(soc2,10,1)

int main(void) {
    mmio_t *mmio;
    uint32_t g10, g11, g13, s52, s54, soc2, gpio2_a4;
    uint32_t txcr, ckr, clkdiv;

    mmio = mmio_new();
    if (mmio_open(mmio, CRU, 0x10000) < 0) {
        fprintf(stderr, "mmio_open(): %s\n", mmio_errmsg(mmio));
        exit(1);
    }

        if (mmio_read32(mmio, CLKSEL_52_R, &s52) < 0) {
            fprintf(stderr, "mmio_read32(): %s\n", mmio_errmsg(mmio));
            exit(1);
        }
        if (mmio_read32(mmio, CLKSEL_54_R, &s54) < 0) {
            fprintf(stderr, "mmio_read32(): %s\n", mmio_errmsg(mmio));
            exit(1);
        }
        if (mmio_read32(mmio, CLKGATE_10_R, &g10) < 0) {
            fprintf(stderr, "mmio_read32(): %s\n", mmio_errmsg(mmio));
            exit(1);
        }
        if (mmio_read32(mmio, CLKGATE_11_R, &g11) < 0) {
            fprintf(stderr, "mmio_read32(): %s\n", mmio_errmsg(mmio));
            exit(1);
        }
        if (mmio_read32(mmio, CLKGATE_13_R, &g13) < 0) {
            fprintf(stderr, "mmio_read32(): %s\n", mmio_errmsg(mmio));
            exit(1);
        }

    mmio_close(mmio);

    mmio = mmio_new();
    if (mmio_open(mmio, GRF, 0x10000) < 0) {
        fprintf(stderr, "mmio_open(): %s\n", mmio_errmsg(mmio));
        exit(1);
    }
        if (mmio_read32(mmio, SOC_2_R, &soc2) < 0) {
            fprintf(stderr, "mmio_read32(): %s\n", mmio_errmsg(mmio));
            exit(1);
        }
    mmio_close(mmio);

    mmio = mmio_new();
    if (mmio_open(mmio, I2S80, 0x10000) < 0) {
        fprintf(stderr, "mmio_open(): %s\n", mmio_errmsg(mmio));
        exit(1);
    }

        if (mmio_read32(mmio, TXCR_R, &txcr) < 0) {
            fprintf(stderr, "mmio_read32(): %s\n", mmio_errmsg(mmio));
            exit(1);
        }
        if (mmio_read32(mmio, CKR_R, &ckr) < 0) {
            fprintf(stderr, "mmio_read32(): %s\n", mmio_errmsg(mmio));
            exit(1);
        }
        if (mmio_read32(mmio, CLKDIV_R, &clkdiv) < 0) {
            fprintf(stderr, "mmio_read32(): %s\n", mmio_errmsg(mmio));
            exit(1);
        }

    mmio_close(mmio);

    mmio = mmio_new();
    if (mmio_open(mmio, GPIO2, 0x10000) < 0) {
        fprintf(stderr, "mmio_open(): %s\n", mmio_errmsg(mmio));
        exit(1);
    }
        if (mmio_read32(mmio, DDR_R, &gpio2_a4) < 0) {
            fprintf(stderr, "mmio_read32(): %s\n", mmio_errmsg(mmio));
            exit(1);
        }
    mmio_close(mmio);


    printf("gate10_12 gate10_13 sel52_10_2 sel52_12_1 gate10_14 gate10_15 gate13_4\n");
    printf("%-9u %-9u %-10u %-10u %-9u %-9u %u\n" \
        ,BV(g10,12,1), BV(g10,13,1), BV(s52,10,3), BV(s52,12,1), BV(g10,14,1), BV(g10,15,1), BV(g13,4,1));

    printf("gate11_0 gate11_1 sel54_10_2 sel54_12_1 gate11_2 gate11_3 gate13_5\n");
    printf("%-8u %-8u %-10u %-10u %-8u %-8u %u\n" \
        ,BV(g11,0,1), BV(g11,1,1), BV(s54,10,3), BV(s54,12,1), BV(g11,2,1), BV(g11,3,1), BV(g13,5,1));

    printf("soc2_10 :   ");
    if( BV(soc2,10,1) ) printf("RX_OUT\n"); else printf("TX_OUT\n");

    printf("TXCR\n");
    printf("TCSR HWT SJM FBM IBM TFS VDW\n");
    printf("%-4u %-3u %-3u %-3u %-3u %-3u %-3u\n", BV(txcr,15,3), BV(txcr,14,1), BV(txcr,12,1), BV(txcr,11,1), BV(txcr,9,3), BV(txcr,5,3), BV(txcr,0,31)+1);

    printf("TX_MAP SDIO 3 2 1 0 = %-u %-u %-u %-u\n", BV(txcr,29,3), BV(txcr,27,3), BV(txcr,25,3), BV(txcr,23,3));

    printf("TRCM %-u  CKR TXlrck div %-u  CKR RXlrck div %-u   CLKDIV TXbclk div %-u   CLKDIV RXbclk div %-u\n", \
            BV(ckr,28,3), BV(ckr,0,255)+1, BV(ckr,8,255)+1, BV(clkdiv,0,255), BV(clkdiv,8,255) );

    printf("gpio2_a4  %-u\n", BV(gpio2_a4,4,1) );

    return 0;
}

