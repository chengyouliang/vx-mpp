#include <linux/device.h>

#include "mpp_dma_alloc.h"

#define MPP_DMA_IOCTL_ALLOC     _IOWR('V', 1, struct mpp_dma_info)
#define DRM_DMA_IOCTL_FREE      _IOWR('V', 2, struct mpp_dma_info)
#define DRM_DMA_IOCTL_IMPORT    _IOWR('V', 3, struct mpp_dma_info)
#define DRM_DMA_FD_TO_HANDLE    _IOWR('V', 4, struct mpp_dma_info)

struct mpp_dma_info {
	u32 fd;
	u32 size;
	u32 hander;
};
