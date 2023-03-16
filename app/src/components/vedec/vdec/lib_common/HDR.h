/* Copyright (C) 2008-2020 Allegro DVT2.  All rights reserved. */
/**************************************************************************//*!
   \addtogroup Buffers
   @{
   \file
 *****************************************************************************/

#pragma once

#include "lib_rtos/types.h"
/*************************************************************************//*!
   \brief Colour Description identifer (See ISO/IEC 23091-4 or ITU-T H.273)
*****************************************************************************/
typedef enum e_ColourDescription
{
  AL_COLOUR_DESC_RESERVED,
  AL_COLOUR_DESC_UNSPECIFIED,
  AL_COLOUR_DESC_BT_470_NTSC,
  AL_COLOUR_DESC_BT_601_NTSC,
  AL_COLOUR_DESC_BT_601_PAL,
  AL_COLOUR_DESC_BT_709,
  AL_COLOUR_DESC_BT_2020,
  AL_COLOUR_DESC_SMPTE_240M,
  AL_COLOUR_DESC_SMPTE_ST_428,
  AL_COLOUR_DESC_SMPTE_RP_431,
  AL_COLOUR_DESC_SMPTE_EG_432,
  AL_COLOUR_DESC_EBU_3213,
  AL_COLOUR_DESC_GENERIC_FILM,
  AL_COLOUR_DESC_MAX_ENUM,
}AL_EColourDescription;

/************************************//*!
   \brief Transfer Function identifer
****************************************/
typedef enum e_TransferCharacteristics
{
  AL_TRANSFER_CHARAC_BT_709 = 1,
  AL_TRANSFER_CHARAC_UNSPECIFIED = 2,
  AL_TRANSFER_CHARAC_BT_470_SYSTEM_M = 4,
  AL_TRANSFER_CHARAC_BT_470_SYSTEM_B = 5,
  AL_TRANSFER_CHARAC_BT_601 = 6,
  AL_TRANSFER_CHARAC_SMPTE_240M = 7,
  AL_TRANSFER_CHARAC_LINEAR = 8,
  AL_TRANSFER_CHARAC_LOG = 9,
  AL_TRANSFER_CHARAC_LOG_EXTENDED = 10,
  AL_TRANSFER_CHARAC_IEC_61966_2_4 = 11,
  AL_TRANSFER_CHARAC_BT_1361 = 12,
  AL_TRANSFER_CHARAC_IEC_61966_2_1 = 13,
  AL_TRANSFER_CHARAC_BT_2020_10B = 14,
  AL_TRANSFER_CHARAC_BT_2020_12B = 15,
  AL_TRANSFER_CHARAC_BT_2100_PQ = 16,
  AL_TRANSFER_CHARAC_SMPTE_428 = 17,
  AL_TRANSFER_CHARAC_BT_2100_HLG = 18,
  AL_TRANSFER_CHARAC_MAX_ENUM,
}AL_ETransferCharacteristics;

/*******************************************************************************//*!
   \brief Matrix coefficient identifier used for luma/chroma computation from RGB
***********************************************************************************/
typedef enum e_ColourMatrixCoefficients
{
  AL_COLOUR_MAT_COEFF_GBR = 0,
  AL_COLOUR_MAT_COEFF_BT_709 = 1,
  AL_COLOUR_MAT_COEFF_UNSPECIFIED = 2,
  AL_COLOUR_MAT_COEFF_USFCC_CFR = 4,
  AL_COLOUR_MAT_COEFF_BT_601_625 = 5,
  AL_COLOUR_MAT_COEFF_BT_601_525 = 6,
  AL_COLOUR_MAT_COEFF_BT_SMPTE_240M = 7,
  AL_COLOUR_MAT_COEFF_BT_YCGCO = 8,
  AL_COLOUR_MAT_COEFF_BT_2100_YCBCR = 9,
  AL_COLOUR_MAT_COEFF_BT_2020_CLS = 10,
  AL_COLOUR_MAT_COEFF_SMPTE_2085 = 11,
  AL_COLOUR_MAT_COEFF_CHROMA_DERIVED_NCLS = 12,
  AL_COLOUR_MAT_COEFF_CHROMA_DERIVED_CLS = 13,
  AL_COLOUR_MAT_COEFF_BT_2100_ICTCP = 14,
  AL_COLOUR_MAT_COEFF_MAX_ENUM,
}AL_EColourMatrixCoefficients;

/*@}*/

