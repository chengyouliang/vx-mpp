#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "mpp_log.h"
#include "mpp_common.h"
#define   DMA_DEV  "/dev/mpp_dma"



#define MPP_DMA_IOCTL_ALLOC     _IOWR('V', 2, struct mpp_dma_info)
#define DRM_DMA_IOCTL_FREE      _IOWR('V', 3, struct mpp_dma_info)
#define DRM_DMA_IOCTL_IMPORT    _IOWR('V', 4, struct mpp_dma_info)

struct mpp_dma_info {
	OMX_U32 fd;
	OMX_U32 size;
	void *hander;
};



static int dma_ioctl(int fd, int req, void *arg)
{
    int ret;

    do {
        ret = ioctl(fd, req, arg);
    } while (ret == -1 && (errno == EINTR || errno == EAGAIN));

    mpp_log("dma_ioctl %x with code %d: %s\n", req,
            ret, strerror(errno));

    return ret;
}

int main()
{
    OMX_S32  fd,ret; 
    void *ptr = NULL; 
    unsigned long offset = 0L;
    char pchar[1023] = "hello world\n";
    fd = open(DMA_DEV, O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        mpp_err_f("open %s failed!\n", DMA_DEV);
        return -1;
    }

    struct mpp_dma_info mppdma_info;
    mppdma_info.size = 4*1024;
    printf("%s %d 0x%.8X\n", __FUNCTION__,__LINE__,MPP_DMA_IOCTL_ALLOC);
    ret = dma_ioctl(fd,MPP_DMA_IOCTL_ALLOC,(void *)&mppdma_info);
    if (ret) {
        mpp_err_f("drm_alloc failed ret %d\n", ret);
        return ret;
    }
    
    ptr = mmap(NULL, mppdma_info.size, PROT_READ | PROT_WRITE,
               MAP_SHARED, mppdma_info.fd, offset);
    if (ptr == MAP_FAILED)
    {
        mpp_err_f("drm_alloc failed ret %d\n", ret);
        return -1;
    }
    printf("%s %d %p\n",__FUNCTION__,__LINE__,ptr);
    memset(ptr,0, 4*1024);
    memcpy(ptr,pchar,4*1024);
     printf("%s %d %s\n",__FUNCTION__,__LINE__,ptr);
    ret = dma_ioctl(fd,DRM_DMA_IOCTL_FREE,(void *)&mppdma_info);
    if (ret) {
        mpp_err_f("drm_alloc free ret %d\n", ret);
        return ret;
    }
    mppdma_info.size = 8*1024;
    ret = dma_ioctl(fd,MPP_DMA_IOCTL_ALLOC,(void *)&mppdma_info);
    if (ret) {
        mpp_err_f("drm_alloc failed ret %d\n", ret);
        return ret;
    }
    ret = dma_ioctl(fd,DRM_DMA_IOCTL_FREE,(void *)&mppdma_info);
    if (ret) {
        mpp_err_f("drm_alloc free ret %d\n", ret);
        return ret;
    }
    close(fd);
}