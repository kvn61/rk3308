// i2s or codec DT
dsd-gpios = <&gpioX Y GPIO_ACTIVE_HIGH>;

// in code
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>

struct gpio_desc *dsd_pin;

probe 
		dsd_pin = devm_gpiod_get_optional(&pdev->dev,
				"dsd", GPIOD_OUT_LOW);
		if (IS_ERR(dsd_pin))
			return PTR_ERR(dsd_pin);

hw_params
	if( params_format(params) == SNDRV_PCM_FORMAT_DSD_U32_LE ) {
		gpiod_set_value(dsd_pin, 1);
		dev_dbg(??->dev, "set enable to 1");
	} else
		gpiod_set_value(dsd_pin, 0);


