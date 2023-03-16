/* Copyright (C) 2008-2020 Allegro DVT2.  All rights reserved. */
/**************************************************************************//*!
   \addtogroup lib_ip_ctrl
   @{
   \file
 *****************************************************************************/
#pragma once

#include "lib_rtos/types.h"
#include "lib_ip_ctrl/IpCtrl.h"

/*********************************************************************//*!
   \brief ip control interface implementation that access the hardware registers
   Checks the presence of the hardware ip, connect to the hardware driver.
   \param[in] deviceFile Specify the file descriptor associated to device
   \param[in] uIntReg Specify the interrupt register
   \param[in] uMskReg Specify the interrupt mask register
   \param[in] uIntMask Specify the interrupt mask Value
   \return Pointer on AL_TIpCtrl object
*************************************************************************/
AL_TIpCtrl* AL_Board_Create(const char* deviceFile, uint32_t uIntReg, uint32_t uMskReg, uint32_t uIntMask);

/*@}*/

