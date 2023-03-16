/* Copyright (C) 2008-2020 Allegro DVT2.  All rights reserved. */
#pragma once

/*************************************************************************//*!
   \brief Video Mode
*****************************************************************************/
typedef enum AL_e_VideoMode
{
  AL_VM_PROGRESSIVE, /*!< Progressive */
  AL_VM_MAX_ENUM,
}AL_EVideoMode;

static AL_INLINE bool AL_IS_INTERLACED(AL_EVideoMode eVideoMode)
{
  (void)eVideoMode;
  bool bIsInterlaced = false;
  return bIsInterlaced;
}

/*************************************************************************//*!
   \brief Sequence Mode
*****************************************************************************/
typedef enum AL_e_SequenceMode
{
  AL_SM_UNKNOWN, /*!< unknown */
  AL_SM_PROGRESSIVE, /*!< progressive */
  AL_SM_MAX_ENUM,
}AL_ESequenceMode;

