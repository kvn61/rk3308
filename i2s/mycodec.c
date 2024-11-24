#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>

#include <sound/soc.h>

static struct snd_soc_dai_driver mycodec_dai = {
    .name = "mycodec-hifi",
    .playback = {
	.stream_name = "Mycodec Playback",
	.channels_min = 1,
	.channels_max = 2,
//	.rates = SNDRV_PCM_RATE_8000_384000,
	.rates = SNDRV_PCM_RATE_8000_768000,
	.formats = (SNDRV_PCM_FMTBIT_S16_LE |
	        SNDRV_PCM_FMTBIT_S20_3LE |
	        SNDRV_PCM_FMTBIT_S24_LE |
	        SNDRV_PCM_FMTBIT_S32_LE |
	        SNDRV_PCM_FMTBIT_DSD_U32_LE),
    },
    .capture = {
	.stream_name = "Mycodec Capture",
	.channels_min = 1,
	.channels_max = 2,
	.rates = SNDRV_PCM_RATE_8000_768000,
//	.rates = SNDRV_PCM_RATE_8000_384000,
//	.rates = SNDRV_PCM_RATE_KNOT,
	.formats = (SNDRV_PCM_FMTBIT_S16_LE |
	        SNDRV_PCM_FMTBIT_S20_3LE |
	        SNDRV_PCM_FMTBIT_S24_LE |
	        SNDRV_PCM_FMTBIT_S32_LE |
	        SNDRV_PCM_FMTBIT_DSD_U32_LE),
    },
};

static struct snd_soc_component_driver soc_component_dev_mycodec;// = {
//    .idle_bias_on	= 1,
//    .use_pmdown_time	= 1,
//    .endianness		= 1,
//};

static int mycodec_probe(struct platform_device *pdev)
{
    struct device_node *node = pdev->dev.of_node;
    u32 pch, cch;

    of_property_read_u32(node, "playback", &pch);
    of_property_read_u32(node, "capture", &cch);
    if( pch )
	mycodec_dai.playback.channels_max = pch;
    if( cch )
	mycodec_dai.capture.channels_max = cch;
	
    return devm_snd_soc_register_component(&pdev->dev, &soc_component_dev_mycodec,
	    &mycodec_dai, 1);
}

static const struct of_device_id mycodec_of_match[] = {
    { .compatible = "my,mycodec", },
    { }
};
MODULE_DEVICE_TABLE(of, mycodec_of_match);

static struct platform_driver mycodec_codec_driver = {
    .probe		= mycodec_probe,
    .driver		= {
	.name	= "mycodec-codec",
	.of_match_table = mycodec_of_match,
    },
};

module_platform_driver(mycodec_codec_driver);

MODULE_DESCRIPTION("ASoC MYCODEC codec driver");
MODULE_AUTHOR("KVN");
MODULE_LICENSE("GPL v2");