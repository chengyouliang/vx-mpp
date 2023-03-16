/* Copyright (C) 2008-2020 Allegro DVT2.  All rights reserved. */
/**************************************************************************//*!
   \addtogroup Decoder_Settings
   @{
   \file
 *****************************************************************************/
#pragma once

#include "lib_common/PicFormat.h"

/*************************************************************************//*!
   \brief Decoder frame output configuration
*****************************************************************************/
typedef struct AL_t_DecOutputSettings
{
  bool bCustomFormat; /*!< Should the frame buffers returned to the user be in a different format from the one used internally in the decoder */
  bool bFrameBufferCompression; /*!< Should the frame buffers returned to the user be compressed */
  AL_EFbStorageMode eFBStorageMode; /*!< Specifies the storage mode of the frame buffers returned to the user */
}AL_TDecOutputSettings;

/*@}*/

