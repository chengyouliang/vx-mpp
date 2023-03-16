/* Copyright (C) 2008-2020 Allegro DVT2.  All rights reserved. */
/*************************************************************************//*!
   \addtogroup Decoder_Settings
   @{
   \file
*****************************************************************************/
#pragma once

/*************************************************************************//*!
   \brief Decoder synchronization mode
*****************************************************************************/
typedef enum AL_e_DecUnit
{
  AL_AU_UNIT = 0, /*< decode at the Access Unit level (frame) */
  AL_VCL_NAL_UNIT = 1, /*< decode at the NAL Unit level (slice) */
  AL_DEC_UNIT_MAX_ENUM,
}AL_EDecUnit;

/*@}*/
