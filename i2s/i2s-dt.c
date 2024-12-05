	if (i2s_tdm->clk_trcm == TRCM_TX) {
			mclk = i2s_tdm->mclk_tx;
		} else if (i2s_tdm->clk_trcm == TRCM_RX) {
			mclk = i2s_tdm->mclk_rx;
		} else if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			mclk = i2s_tdm->mclk_tx;
		} else {
			mclk = i2s_tdm->mclk_rx;
		}

		err = clk_set_rate( mclk, 24576000);
#1		DBGOUT("i2s: %s clk_set_rate_48 err=%d\n", __func__, (int)err);

		mclk_rate = clk_get_rate(mclk);
#2		DBGOUT("i2s: %s clk_get_rate =%d\n", __func__, (int)mclk_rate);

		err = clk_set_rate(mclk, DEFAULT_MCLK_FS * params_rate(params));
#3		DBGOUT("i2s: %s clk_set_rate err=%d\n", __func__, (int)err);
		if (err)
			return err;

		mclk_rate = clk_get_rate(mclk);
		bclk_rate = i2s_tdm->frame_width * params_rate(params);
======
Попробовал в i2s dt так
	clocks = <&mclk_i2s0_8ch_in>, <&mclk_i2s0_8ch_in>, <&cru HCLK_I2S0_8CH>;
	clock-names = "mclk_tx", "mclk_rx", "hclk";

!!! gate для SCLK_I2S0_8CH_TX(RX) выключены, можно было-бы в кодеке включить

#1 err = 0
#2 корректно возвращает rate
#3 (родной код) нестандартная частота, err=-22

При загрузке
[    0.863385] i2s8d: loading out-of-tree module taints kernel.
[    0.877346] mycodec_probe: mycodec_probe
[    0.888959] i2s_tdm_set_sysclk: rockchip_i2s_tdm_set_sysclk
[    0.898725] rockchip-thermal ff1f0000.tsadc: Missing tshut mode property, using default (cru)

И получаем при проигрывании aplay 44100 или 48000
[   45.990289] mycodec_hw_params: mycodec_hw_params
[   45.990814] i2s: rockchip_i2s_tdm_hw_params clk_set_rate_48 err=0
[   45.991444] i2s: rockchip_i2s_tdm_hw_params clk_get_rate =24576000
[   45.992059] i2s: rockchip_i2s_tdm_hw_params clk_set_rate err=-22
[   45.992613] rockchip-i2s-tdm ff300000.i2s: ASoC: error at snd_soc_dai_hw_params on ff300000.i2s: -22

при проигрывании aplay 88200 или 96000 все ОК ( DEFAULT_MCLK_FS=256 * 2 * fs = 512 fs gen rate )
[  218.595768] mycodec_hw_params: mycodec_hw_params
[  218.596240] i2s: rockchip_i2s_tdm_hw_params clk_set_rate_48 err=0
[  218.596783] i2s: rockchip_i2s_tdm_hw_params clk_get_rate =24576000
[  218.597358] i2s: rockchip_i2s_tdm_hw_params clk_set_rate err=0
[  218.597942] i2s: rockchip_i2s_tdm_hw_params bclk_rate=5644800

Сначала вызов кодек, потом драйвер !

Бельмо (((
Выходит без правки драйвера никак ((((( (в новых версиях еще чего-нибудь добавят)

В драйвере i2s (без tdm) видимо freq совпадал с чатотой генов, в противном случае тоже выдывал бы error
static int rockchip_i2s_set_sysclk(struct snd_soc_dai *cpu_dai, int clk_id,
	ret = clk_set_rate(i2s->mclk, freq);
	if (ret)
		dev_err(i2s->dev, "Fail to set mclk %d\n", ret);

