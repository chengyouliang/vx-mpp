/*
 * 
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "mpp_mem.h"
#include "mpp_log.h"
#include "os_mem.h"
#include "mpp_env.h"
#include "allocator_dma.h"

static OMX_U32 dma_debug = 0;

#define DMA_FUNCTION                (0x00000001)
#define DMA_DEVICE                  (0x00000002)
#define DMA_CLIENT                  (0x00000004)
#define DMA_IOCTL                   (0x00000008)

#define dma_dbg(flag, fmt, ...)     _mpp_dbg_f(dma_debug, flag, fmt, ## __VA_ARGS__)
#define dma_dbg_func(fmt, ...)      dma_dbg(DMA_FUNCTION, fmt, ## __VA_ARGS__)


typedef struct {
    OMX_U32  alignment;
    OMX_S32  dma_device;
    OMX_U32  flags;
} allocator_ctx_dma;

static const char *dev_dma = "/dev/mpp_dma";


static int dma_ioctl(int fd, int req, void *arg)
{
    int ret;
    printf("dma_ioctl %x with code %d: %s\n", req);
    do {
        ret = ioctl(fd, req, arg);
    } while (ret == -1 && (errno == EINTR || errno == EAGAIN));

    dma_dbg(DMA_IOCTL, "dma_ioctl %x with code %d: %s\n", req,
            ret, strerror(errno));

    return ret;
}

static MPP_RET allocator_dma_open(void **ctx, MppAllocatorCfg *cfg)
{
    MPP_RET ret = MPP_OK;
    OMX_S32 fd;
    allocator_ctx_dma *p;

    dma_dbg_func("enter\n");

    if (NULL == ctx) {
        mpp_err_f("does not accept NULL input\n");
        return MPP_ERR_NULL_PTR;
    }

    *ctx = NULL;

    mpp_env_get_u32("dma_debug", &dma_debug, 0);

    fd = open(dev_dma, O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        mpp_err_f("open %s failed!\n", dev_dma);
        return MPP_ERR_UNKNOW;
    }

    dma_dbg(DMA_DEVICE, "open drm dev fd %d\n", fd);

    p = mpp_malloc(allocator_ctx_dma, 1);
    if (NULL == p) {
        close(fd);
        mpp_err_f("failed to allocate context\n");
        return MPP_ERR_MALLOC;
    } else {
        /*
         * default drm use cma, do nothing here
         */
        p->alignment    = cfg->alignment;
        p->flags        = cfg->flags;
        p->dma_device   = fd;
        *ctx = p;
    }

    dma_dbg_func("leave dev %d\n", fd);

    return MPP_OK;
}

static MPP_RET allocator_dma_alloc(void *ctx, MppBufferInfo *info)
{
    allocator_ctx_dma *p;
    int ret;
    if (!ctx || !info) {
        mpp_err_f("found NULL context input\n");
        return MPP_ERR_VALUE;
    }
    p = (allocator_ctx_dma *)ctx;

    dma_dbg_func("dev %d alloc alignment %d size %d\n", p->dma_device,
                 p->alignment, info->size);
    struct mpp_dma_info mppdma_info;
    mppdma_info.size = (info->size + p->alignment - 1) & (~( p->alignment - 1));
    printf("%s %d 0x%.8X\n", __FUNCTION__,__LINE__,MPP_DMA_IOCTL_ALLOC);
    ret = dma_ioctl(p->dma_device,MPP_DMA_IOCTL_ALLOC,(void *)&mppdma_info);
    if (ret) {
        mpp_err_f("drm_alloc failed ret %d\n", ret);
        return ret;
    }
    return ret;
}

static MPP_RET allocator_dma_free(void *ctx, MppBufferInfo *info)
{
    allocator_ctx_dma *p;
    int ret;
    if (!ctx || !info) {
        mpp_err_f("found NULL context input\n");
        return MPP_ERR_VALUE;
    }
     p = (allocator_ctx_dma *)ctx;

    dma_dbg_func("dev %d alloc alignment %d size %d\n", p->dma_device,
                 p->alignment, info->size);
    struct mpp_dma_info mppdma_info;
    mppdma_info.hander = info->hnd;
    ret = dma_ioctl(p->dma_device,DRM_DMA_IOCTL_FREE,(void *)&mppdma_info);
    if (ret) {
        mpp_err_f("drm_alloc failed ret %d\n", ret);
        return ret;
    }

    return ret;
}

static MPP_RET allocator_dma_import(void *ctx, MppBufferInfo *info)
{
    allocator_ctx_dma *p;
    int ret;
    if (!ctx || !info) {
        mpp_err_f("found NULL context input\n");
        return MPP_ERR_VALUE;
    }
    p = (allocator_ctx_dma *)ctx;

    dma_dbg_func("dev %d alloc alignment %d size %d\n", p->dma_device,
                 p->alignment, info->size);
    struct mpp_dma_info mppdma_info;
    mppdma_info.hander = info->hnd;
    ret = dma_ioctl(p->dma_device,DRM_DMA_IOCTL_IMPORT,(void *)&mppdma_info);
    if (ret) {
        mpp_err_f("dma alloc failed ret %d\n", ret);
        return ret;
    }
    return ret;
}

static MPP_RET allocator_dma_mmap(void *ctx, MppBufferInfo *info)
{
    void *ptr = NULL;
    unsigned long offset = 0L;
    mpp_assert(ctx);
    mpp_assert(info->size);
    mpp_assert(info->fd >= 0);

    if (info->ptr)
        return MPP_OK;

    /*
     * It is insecure to access the first memory page,
     * usually system doesn't allow this behavior.
     */
    ptr = mmap(NULL, info->size, PROT_READ | PROT_WRITE,
               MAP_SHARED, info->fd, offset);
    if (ptr == MAP_FAILED)
        return MPP_ERR_NULL_PTR;

    info->ptr = ptr;

    return MPP_OK;
}

static MPP_RET allocator_dma_close(void *ctx)
{
    if (ctx) {
        mpp_free(ctx);
        return MPP_OK;
    }
    mpp_err_f("found NULL context input\n");
    return MPP_ERR_VALUE;
}

os_allocator allocator_dma = {
    .open  =  allocator_dma_open,
    .close = allocator_dma_close,
    .alloc  =  allocator_dma_alloc,
    .free   =   allocator_dma_free,
    .import = allocator_dma_import,
    .release = allocator_dma_free,
    .mmap = allocator_dma_mmap,
};
