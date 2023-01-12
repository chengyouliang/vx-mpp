#include "mpp_dmabuf.h"
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

MODULE_LICENSE("Dual BSD/GPL");
static struct class *module_class;



static int mpp_core_init(void)
{
        printk(KERN_ALERT "Hello, world\n");
        return 0;
}
static void mpp_core_exit(void)
{

        printk(KERN_ALERT "Goodbye, world\n");
        module_class = class_create(THIS_MODULE, "mpp_core_class");

}
module_init(mpp_core_init);

module_exit(mpp_core_exit);