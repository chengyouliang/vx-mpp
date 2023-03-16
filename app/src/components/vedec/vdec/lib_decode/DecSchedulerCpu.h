/* Copyright (C) 2008-2020 Allegro DVT2.  All rights reserved. */
/**************************************************************************//*!
   \addtogroup Decoder_API
   @{
   \file
 *****************************************************************************/
#pragma once

typedef struct AL_i_DecScheduler AL_IDecScheduler;

#include "lib_common/Allocator.h"
#include "lib_ip_ctrl/IpCtrl.h"

/*************************************************************************//*!
    \brief Interfaces with a scheduler that runs on the same process.
    Its main usage is to interface with the scheduler if everything is running in the same process
   \param[in] pIpCtrl The interface the scheduler will use to read and write the IP registers
   \param[in] pDmaAllocator a dma allocator that will be used to create work buffers and to map some of the buffer that are sent to the scheduler.
*****************************************************************************/
AL_IDecScheduler* AL_DecSchedulerCpu_Create(AL_TIpCtrl* pIpCtrl, AL_TAllocator* pDmaAllocator);

/*@}*/

