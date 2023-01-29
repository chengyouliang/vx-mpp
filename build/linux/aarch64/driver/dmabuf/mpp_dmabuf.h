#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/dma-buf.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>

#include "mpp_dma_alloc.h"

#define MPP_DMA_IOCTL_ALLOC     _IOWR('V', 2, struct mpp_dma_info)
#define DRM_DMA_IOCTL_FREE      _IOWR('V', 3, struct mpp_dma_info)
#define DRM_DMA_IOCTL_IMPORT    _IOWR('V', 4, struct mpp_dma_info)


struct mpp_dma_info {
	u32 fd;
	u32 size;
	void *hander;
};
int mpp_setup_dma_cdev(struct cdev *pcdev, struct platform_device *pdev,struct class *module_class,int major,  int minor, const char *device_name);
int mpp_del_dma_cdev(struct cdev *pcdev, struct class *module_class,int major,  int minor);