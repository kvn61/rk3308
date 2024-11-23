	clocks = <&cru SCLK_I2S0_8CH_TX>, <&cru SCLK_I2S0_8CH_RX>, <&cru HCLK_I2S0_8CH>, 
		<&cru SCLK_I2S0_8CH_TX_SRC>, <&cru SCLK_I2S0_8CH_RX_SRC>, <&cru PLL_VPLL0>, <&cru PLL_VPLL1>;
	clock-names = 	"mclk_tx", "mclk_rx", "hclk",
			"mclk_tx_src", "mclk_rx_src", "mclk_root0", "mclk_root1";
