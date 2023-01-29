#include <linux/uaccess.h>
#include <linux/dma-buf.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/property.h>
#include "dmabuf/mpp_dmabuf.h"


#define CODEC_NR_DEVS 6
#define DRV_NAME "vanxum_codec"

static struct class *module_class;
static int codec_major;
static int codec_nr_devs = CODEC_NR_DEVS;
static struct cdev dma_cdev;

static struct platform_device mpp_codec_device = {
	.name = DRV_NAME,
	.id = 0,
};


static int mpp_code_probe(struct platform_device *pdev)
{
	dev_t dev = 0;
	int err;
    module_class = class_create(THIS_MODULE, "mpp_codec_class");
	if (codec_major == 0) {
		err = alloc_chrdev_region(&dev, 0, codec_nr_devs, "vanxum_codec");
		codec_major = MAJOR(dev);

		if (err) {
			printk("Allegro codec: can't get major %d\n",
				 codec_major);
			return err;
		}
	}
    mpp_setup_dma_cdev(&dma_cdev,pdev,module_class,codec_major,1,"mpp_dma");
	return 0;
}

static int mpp_code_remove(struct platform_device *pdev)
{
	mpp_del_dma_cdev(&dma_cdev,module_class,codec_major,1);
	class_destroy(module_class);
	return 0;
}

static const struct of_device_id mpp_codec_table[] = {
	{ .compatible =  "vanxum_codec", },
	{}
};

MODULE_DEVICE_TABLE(of, mpp_codec_table);

static struct platform_driver mpp_codec_driver = {
	.driver = {
		.name = DRV_NAME,
		.of_match_table = of_match_ptr(mpp_codec_table),
	},
	.probe = mpp_code_probe,
	.remove = mpp_code_remove,
};

static int  __init mpp_codec_init(void)
{
	platform_driver_register(&mpp_codec_driver);
	return platform_device_register(&mpp_codec_device);
}

static void mpp_codec_exit(void)
{
	 platform_driver_unregister(&mpp_codec_driver);
	 platform_device_unregister(&mpp_codec_device);
}

module_init(mpp_codec_init);
module_exit(mpp_codec_exit);

MODULE_LICENSE("Dual BSD/GPL");