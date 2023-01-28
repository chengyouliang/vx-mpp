#ifndef _MPP__DMA_ALLOC_H_
#define _MPP__DMA_ALLOC_H_

#include <linux/device.h>

struct mpp_dma_buffer {
	u32 size;
	dma_addr_t dma_handle;
	void *cpu_handle;
};

struct mpp_dma_buffer *mpp_alloc_dma(struct device *dev, size_t size);
void mpp_free_dma(struct device *dev, struct mpp_dma_buffer *buf);

#endif /* _AL_ALLOC_H_ */
