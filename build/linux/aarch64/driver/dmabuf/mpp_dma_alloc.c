#include <linux/module.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>

#include "mpp_dma_alloc.h"

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("chengyouliang");
MODULE_DESCRIPTION("vanxum Common");

struct mpp_dma_buffer *mpp_alloc_dma(struct device *dev, size_t size)
{
	printk("%s %d\n",__FUNCTION__,__LINE__);
	struct mpp_dma_buffer *buf =
		kmalloc(sizeof(struct mpp_dma_buffer),
			GFP_KERNEL);

	if (!buf)
		return NULL;
	printk("%s %d %d %p\n",__FUNCTION__,__LINE__,1024,dev);
	buf->size = 1024;
	buf->cpu_handle = dma_alloc_coherent(dev, buf->size,
					     &buf->dma_handle,
					     GFP_KERNEL | GFP_DMA);

	printk("%s %d\n",__FUNCTION__,__LINE__);
	if (!buf->cpu_handle) {
		kfree(buf);
		return NULL;
	}
	printk("%s %d\n",__FUNCTION__,__LINE__);
	return buf;
}

void mpp_free_dma(struct device *dev, struct mpp_dma_buffer *buf)
{
	if (buf)
		dma_free_coherent(dev, buf->size, buf->cpu_handle,
				  buf->dma_handle);
	kfree(buf);
}


