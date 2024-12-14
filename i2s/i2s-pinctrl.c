#include <linux/pinctrl/consumer.h>


  struct pinctrl *pinctrl;
	struct pinctrl_state *bclk_on;
	struct pinctrl_state *bclk_off;
};

-------------------------
static int i2s_pinctrl_select_bclk_on(struct rk_i2s_dev *i2s)
{
	int ret = 0;

	if (!IS_ERR(i2s->pinctrl) && !IS_ERR_OR_NULL(i2s->bclk_on))
		ret = pinctrl_select_state(i2s->pinctrl, i2s->bclk_on);

	if (ret)
		dev_err(i2s->dev, "bclk enable failed %d\n", ret);

	return ret;
}

static int i2s_pinctrl_select_bclk_off(struct rk_i2s_dev *i2s)
{

	int ret = 0;

	if (!IS_ERR(i2s->pinctrl) && !IS_ERR_OR_NULL(i2s->bclk_off))
		ret = pinctrl_select_state(i2s->pinctrl, i2s->bclk_off);

	if (ret)
		dev_err(i2s->dev, "bclk disable failed %d\n", ret);

	return ret;
}

------------------------------
PROBE
	i2s->pinctrl = devm_pinctrl_get(&pdev->dev);
	if (!IS_ERR(i2s->pinctrl)) {
		i2s->bclk_on = pinctrl_lookup_state(i2s->pinctrl, "bclk_on");
		if (!IS_ERR_OR_NULL(i2s->bclk_on)) {
			i2s->bclk_off = pinctrl_lookup_state(i2s->pinctrl, "bclk_off");
			if (IS_ERR_OR_NULL(i2s->bclk_off)) {
				dev_err(&pdev->dev, "failed to find i2s bclk_off\n");
				ret = -EINVAL;
				goto err_clk;
			}
		}
	} else {
		dev_dbg(&pdev->dev, "failed to find i2s pinctrl\n");
	}

	i2s_pinctrl_select_bclk_off(i2s);

=======================
DT

i2s:...{
    ...
		pinctrl-names = "bclk_on", "bclk_off";
		pinctrl-0 = <&i2s0_8ch_bus>;
		pinctrl-1 = <&i2s0_8ch_bus_bclk_off>;
    ...
};

	  pinctrl: pinctrl {
      ...
  		i2s0 {
  			i2s0_8ch_bus: i2s0-8ch-bus {
  				rockchip,pins =
  					<3 RK_PD0 1 &pcfg_pull_none>,
  					<3 RK_PD1 1 &pcfg_pull_none>,
  					<3 RK_PD2 1 &pcfg_pull_none>,
  					<3 RK_PD3 1 &pcfg_pull_none>,
  					<3 RK_PD4 1 &pcfg_pull_none>,
  					<3 RK_PD5 1 &pcfg_pull_none>,
  					<3 RK_PD6 1 &pcfg_pull_none>,
  					<3 RK_PD7 1 &pcfg_pull_none>,
  					<4 RK_PA0 1 &pcfg_pull_none>;
  			};
  
  			i2s0_8ch_bus_bclk_off: i2s0-8ch-bus-bclk-off {
  				rockchip,pins =
  					<3 RK_PD0 RK_FUNC_GPIO &pcfg_pull_none>,
  					<3 RK_PD1 1 &pcfg_pull_none>,
  					<3 RK_PD2 1 &pcfg_pull_none>,
  					<3 RK_PD3 1 &pcfg_pull_none>,
  					<3 RK_PD4 1 &pcfg_pull_none>,
  					<3 RK_PD5 1 &pcfg_pull_none>,
  					<3 RK_PD6 1 &pcfg_pull_none>,
  					<3 RK_PD7 1 &pcfg_pull_none>,
  					<4 RK_PA0 1 &pcfg_pull_none>;
  			};
  		};
      ...
  };
