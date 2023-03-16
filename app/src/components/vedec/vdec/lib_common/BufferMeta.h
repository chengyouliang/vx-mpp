/* Copyright (C) 2008-2020 Allegro DVT2.  All rights reserved. */
/**************************************************************************//*!
   \addtogroup Buffers
   @{
   \file
 *****************************************************************************/
#pragma once

#include "lib_rtos/types.h"

/*************************************************************************//*!
   \brief Tag identifying the metadata structure
*****************************************************************************/
typedef enum
{
  AL_META_TYPE_PIXMAP, /*< useful information related to the reconstructed picture */
  AL_META_TYPE_STREAM, /*< useful section of the buffer containing the bitstream */
  AL_META_TYPE_CIRCULAR, /*< circular buffer implementation inside the buffer */
  AL_META_TYPE_PICTURE, /*< useful information about the bitstream choices for the frame */
  AL_META_TYPE_HANDLE, /*< pointers to handles */
  AL_META_TYPE_SEI, /*< sei messages */
  AL_META_TYPE_RATECTRL, /*< rate-control statistics */
  AL_META_TYPE_MAX, /* sentinel */
  AL_META_TYPE_EXTENDED = 0x7F000000 /*< user can define their own metadatas after this value */
}AL_EMetaType;

AL_DEPRECATED_ENUM_VALUE(AL_EMetaType, AL_META_TYPE_SOURCE, AL_META_TYPE_PIXMAP, "Renamed. Use AL_META_TYPE_PIXMAP.");

typedef struct al_t_MetaData AL_TMetaData;
typedef bool (* AL_FCN_MetaDestroy) (AL_TMetaData* pMeta);
typedef AL_TMetaData* (* AL_FCN_MetaClone) (AL_TMetaData* pMeta);

/*************************************************************************//*!
   \brief Metadatas are used to add useful informations to a buffer. The user
   can also define his own metadata type and bind it to the buffer.
*****************************************************************************/
struct al_t_MetaData
{
  AL_EMetaType eType; /*< tag of the metadata */
  AL_FCN_MetaDestroy MetaDestroy; /*< custom deleter */
  AL_FCN_MetaClone MetaClone; /*< copy constructor */
};

static AL_INLINE AL_TMetaData* AL_MetaData_Clone(AL_TMetaData* pMeta)
{
  return pMeta->MetaClone(pMeta);
}

static AL_INLINE bool AL_MetaData_Destroy(AL_TMetaData* pMeta)
{
  return pMeta->MetaDestroy(pMeta);
}

/*@}*/

