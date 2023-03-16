/* Copyright (C) 2008-2020 Allegro DVT2.  All rights reserved. */
/*************************************************************************//*!
   \defgroup lib_ip_ctrl Ip Control

   The AL_TIpCtrl structure defines an interface object used to access the
   hardware ip registers and to register actions to execute when an interrupt
   is received from the hardware ip.
   The following API is an abstraction layer allowing various implementations
   of the ip control interface.
   The user can provide his own implementation if the provided ones don't fit
   his constraints

   @{
   \file
*****************************************************************************/
#pragma once

#include "lib_rtos/types.h"

enum {
	CB_ENDSTARTCODE,
	CB_DESTROYCHANNEL,
	CB_ENDPARSING,
	CB_ENDDECODING,
	CB_MAX_NUM
};
/*************************************************************************//*!
   \brief IP callback prototype
*****************************************************************************/
typedef void (* AL_PFN_IpCtrl_CallBack) (void* pUserData, void *cbInfo);

/*************************************************************************//*!
   \brief Generic interface to access the registers of the hardware ip.
*****************************************************************************/
typedef struct AL_t_IpCtrl AL_TIpCtrl;

typedef struct
{
  void (* pfnDestroy)(AL_TIpCtrl* pThis);
  void (* pfnRegisterCallBack)(AL_TIpCtrl* pThis, AL_PFN_IpCtrl_CallBack pfnCallBack, void* pUserData, uint8_t cbNum);
  void (* pfnCallSched)(AL_TIpCtrl* pThis, int cmd, void *args);
}AL_IpCtrlVtable;

struct AL_t_IpCtrl
{
  const AL_IpCtrlVtable* vtable;
};

/*********************************************************************//*!
   \brief Destroys an AL_TIpCtrl object
   \param[in] pIpCtrl pointer to a AL_TIpCtrl interface
*************************************************************************/
static AL_INLINE void AL_IpCtrl_Destroy(AL_TIpCtrl* pIpCtrl)
{
  if(!pIpCtrl)
    return;

  pIpCtrl->vtable->pfnDestroy(pIpCtrl);
}

/*********************************************************************//*!
   \brief Associates a callback function with an interrupt number
   To unregister a callback, register a NULL pointer as a callback.
   After calling this function, the user knows that the callback will never be called again.

   An implementation is required to finish the callback being unregistered if it
   is running before returning from this function.

   Implementations can raise a warning if the underlying IP tries to raise
   an interrupt which doesn't have a callback but shouldn't raise one if the
   interrupt was unregistered.

   A callback cannot be registered inside another callback.

   \param[in] pIpCtrl pointer to a TIpCtrl interface
   \param[in] pfnCallBack Pointer to the function to be called when
   corresponding interrupt is received.
   \param[in] pUserData   Pointer to the call back function parameter
   \param[in] uIntNum     Interrupt number associated to pfn_CallBack
*************************************************************************/
static AL_INLINE void AL_IpCtrl_RegisterCallBack(AL_TIpCtrl* pIpCtrl, AL_PFN_IpCtrl_CallBack pfnCallBack, void* pUserData, uint8_t cbNum)
{
  pIpCtrl->vtable->pfnRegisterCallBack(pIpCtrl, pfnCallBack, pUserData, cbNum);
}

static AL_INLINE void AL_IpCtrl_CallSched(AL_TIpCtrl* pIpCtrl, int cmd, void *args)
{
  pIpCtrl->vtable->pfnCallSched(pIpCtrl, cmd, args);
}

/*@}*/

