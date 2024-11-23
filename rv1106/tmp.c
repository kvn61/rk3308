
=== Так было dt для rk3308 в старом кернел, в последних оставили 3 клока и убрали calibrate.
	clocks = <&cru SCLK_I2S0_8CH_TX>, <&cru SCLK_I2S0_8CH_RX>, <&cru HCLK_I2S0_8CH>, 
		<&cru SCLK_I2S0_8CH_TX_SRC>, <&cru SCLK_I2S0_8CH_RX_SRC>, <&cru PLL_VPLL0>, <&cru PLL_VPLL1>;
	clock-names = 	"mclk_tx", "mclk_rx", "hclk",
			"mclk_tx_src", "mclk_rx_src", "mclk_root0", "mclk_root1";

Ниже выписал clk для rv1106, там не так страшно ))), mux и gate, с моей картинкой doc/rk3308-cru.jpg сравните и имена сопоставить.

Вроде для одной сетки должно сработать
    i2s0_mclkin: i2s0_mclkin {
	compatible = "fixed-clock";
	#clock-cells = <0>;
	clock-frequency = <22579200>;
	status = "okay";
	clock-output-names = "i2s0_mclkin";
    };

		---это в i2s dt---
	my,mclk_external;
	assigned-clocks = <&cru SCLK_I2S0_8CH_TX>;
	assigned-clock-parents = <&i2s0_mclkin>;

my,mclk_external в i2s-tdm , чтобы не пытался установить rate (в mux тогда сбрасываеться на 0 и отключает внешний клок)


===drivers/clk/rockchip/clk-rv1106.c===

PNAME(clk_i2s0_8ch_tx_p)		= { "clk_i2s0_8ch_tx_src", "clk_i2s0_8ch_tx_frac", "i2s0_mclkin", "xin_osc0_half" };
PNAME(clk_i2s0_8ch_rx_p)		= { "clk_i2s0_8ch_rx_src", "clk_i2s0_8ch_rx_frac", "i2s0_mclkin", "xin_osc0_half" };
PNAME(i2s0_8ch_mclkout_p)		= { "mclk_i2s0_8ch_tx", "mclk_i2s0_8ch_rx", "xin_osc0_half" };

static struct rockchip_clk_branch rv1106_i2s0_8ch_tx_fracmux __initdata =
	MUX(CLK_I2S0_8CH_TX, "clk_i2s0_8ch_tx", clk_i2s0_8ch_tx_p, CLK_SET_RATE_PARENT,
			RV1106_CLKSEL_CON(19), 0, 2, MFLAGS);

static struct rockchip_clk_branch rv1106_i2s0_8ch_rx_fracmux __initdata =
	MUX(CLK_I2S0_8CH_RX, "clk_i2s0_8ch_rx", clk_i2s0_8ch_rx_p, CLK_SET_RATE_PARENT,
			RV1106_CLKSEL_CON(21), 0, 2, MFLAGS);

	COMPOSITE(CLK_I2S0_8CH_TX_SRC, "clk_i2s0_8ch_tx_src", mux_gpll_cpll_p, 0,
			RV1106_CLKSEL_CON(17), 7, 1, MFLAGS, 2, 5, DFLAGS,
			RV1106_CLKGATE_CON(1), 13, GFLAGS),
	COMPOSITE_FRACMUX(CLK_I2S0_8CH_TX_FRAC, "clk_i2s0_8ch_tx_frac", "clk_i2s0_8ch_tx_src", CLK_SET_RATE_PARENT,
			RV1106_CLKSEL_CON(18), 0,
			RV1106_CLKGATE_CON(1), 14, GFLAGS,
			&rv1106_i2s0_8ch_tx_fracmux),
	GATE(MCLK_I2S0_8CH_TX, "mclk_i2s0_8ch_tx", "clk_i2s0_8ch_tx", 0,
			RV1106_CLKGATE_CON(1), 15, GFLAGS),
	COMPOSITE(CLK_I2S0_8CH_RX_SRC, "clk_i2s0_8ch_rx_src", mux_gpll_cpll_p, 0,
			RV1106_CLKSEL_CON(19), 7, 1, MFLAGS, 2, 5, DFLAGS,
			RV1106_CLKGATE_CON(2), 0, GFLAGS),
	COMPOSITE_FRACMUX(CLK_I2S0_8CH_RX_FRAC, "clk_i2s0_8ch_rx_frac", "clk_i2s0_8ch_rx_src", CLK_SET_RATE_PARENT,
			RV1106_CLKSEL_CON(20), 0,
			RV1106_CLKGATE_CON(2), 1, GFLAGS,
			&rv1106_i2s0_8ch_rx_fracmux),
	GATE(MCLK_I2S0_8CH_RX, "mclk_i2s0_8ch_rx", "clk_i2s0_8ch_rx", 0,
			RV1106_CLKGATE_CON(2), 2, GFLAGS),
	MUX(I2S0_8CH_MCLKOUT, "i2s0_8ch_mclkout", i2s0_8ch_mclkout_p, CLK_SET_RATE_PARENT,
			RV1106_CLKSEL_CON(21), 2, 2, MFLAGS),

	GATE(HCLK_I2S0, "hclk_i2s0", "hclk_peri_root", 0,
			RV1106_PERICLKGATE_CON(6), 0, GFLAGS),
===drivers/clk/rockchip/clk-rv1106.c===

===include/dt-bindings/clock/rv1106-cru.h===
#define PLL_DPLL		2
#define PLL_CPLL		3

#define HCLK_I2S0		49

#define CLK_I2S0_8CH_TX_SRC	151
#define CLK_I2S0_8CH_TX_FRAC	152
#define CLK_I2S0_8CH_TX		153
#define CLK_I2S0_8CH_RX_SRC	154
#define CLK_I2S0_8CH_RX_FRAC	155
#define CLK_I2S0_8CH_RX		156
#define I2S0_8CH_MCLKOUT	157
#define MCLK_I2S0_8CH_RX	158
#define MCLK_I2S0_8CH_TX	159

===include/dt-bindings/clock/rv1106-cru.h===
