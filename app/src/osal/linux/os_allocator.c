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

#if defined(__gnu_linux__)
#include "mpp_log.h"

#include "allocator_drm.h"
#include "allocator_dma.h"
#include "allocator_ion.h"
#include "allocator_std.h"

/*
 * Linux only support MPP_BUFFER_TYPE_NORMAL so far
 * we can support MPP_BUFFER_TYPE_V4L2 later
 */

MPP_RET os_allocator_get(os_allocator *api, MppBufferType type)
{
    MPP_RET ret = MPP_OK;
    switch (type) {
    case MPP_BUFFER_TYPE_NORMAL : {
        *api = allocator_std;
    } break;
    case MPP_BUFFER_TYPE_ION : {
       *api = allocator_ion ;

    } break;
    case MPP_BUFFER_TYPE_DMA: {
        *api = allocator_dma;
    } break;
    case MPP_BUFFER_TYPE_DRM : {
        *api = allocator_drm ;
    } break;
    default : {
        ret = MPP_NOK;
    } break;
    }
    return ret;
}

#endif
