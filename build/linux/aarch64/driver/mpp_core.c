#include <linux/uaccess.h>
#include <linux/dma-buf.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/module.h>

#include "dmabuf/mpp_dmabuf.h"

#define CODEC_NR_DEVS 6


static struct class *module_class;
static int codec_major;
static int codec_nr_devs = CODEC_NR_DEVS;
static struct cdev dma_cdev;

static int mpp_core_init(void)
{
    dev_t dev = 0;
	int err;
	printk("%s %d\n",__FUNCTION__,__LINE__);
    module_class = class_create(THIS_MODULE, "mpp_core_class");
	printk("%s %d\n",__FUNCTION__,__LINE__);
	if (codec_major == 0) {
		printk("%s %d\n",__FUNCTION__,__LINE__);
		err = alloc_chrdev_region(&dev, 0, codec_nr_devs, "vanxum_codec");
		codec_major = MAJOR(dev);

		if (err) {
			printk("Allegro codec: can't get major %d\n",
				 codec_major);
			return err;
		}
	}
	printk("%s %d\n",__FUNCTION__,__LINE__);
    mpp_setup_dma_cdev(&dma_cdev,module_class,codec_major,1,"mpp_dma");

    return 0;
}

static void mpp_core_exit(void)
{
      mpp_del_dma_cdev(&dma_cdev,module_class,codec_major,1);
}

module_init(mpp_core_init);
module_exit(mpp_core_exit);

MODULE_LICENSE("Dual BSD/GPL");