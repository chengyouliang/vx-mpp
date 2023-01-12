/*
 * Copyright 2017 Rockchip Electronics Co. LTD
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

#ifndef __MPP_QUEUE_H__
#define __MPP_QUEUE_H__

#include "mpp_list.h"

class MppQueue: public mpp_list
{
private:
    sem_t mQueuePending;
    int mFlushFlag;
public:
    MppQueue(node_destructor func);
    ~MppQueue();
    OMX_S32 push(void *data, OMX_S32 size);
    OMX_S32 pull(void *data, OMX_S32 size);
    OMX_S32 flush();
};

#endif