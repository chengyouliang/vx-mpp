/* Copyright (C) 2008-2020 Allegro DVT2.  All rights reserved. */
/**************************************************************************//*!
   \addtogroup Driver
   @{
   \file
 *****************************************************************************/
#pragma once
#include "lib_rtos/types.h"

typedef enum
{
  DRIVER_SUCCESS,
  DRIVER_ERROR_UNKNOWN,
  DRIVER_ERROR_NO_MEMORY,
  DRIVER_ERROR_CHANNEL,
  DRIVER_TIMEOUT,
}AL_EDriverError;

#define AL_POLL_MSG 0xfffffffc

/*************************************************************************//*!
    \brief Interfaces with a device.
    The device can either be the interface of a kernel driver like al5e, al5r or al5d
    or it could also be a socket, this is implementation dependant.
    \see AL_GetHardwareDriver for the kernel driver implementation
*****************************************************************************/
typedef struct AL_t_driver AL_TDriver;
typedef struct
{
  int (* pfnOpen)(AL_TDriver* driver, const char* device);
  void (* pfnClose)(AL_TDriver* driver, int fd);
  AL_EDriverError (* pfnPostMessage)(AL_TDriver* driver, int fd, long unsigned int messageId, void* data, bool isBlocking);
}AL_DriverVtable;

struct AL_t_driver
{
  const AL_DriverVtable* vtable;
};

static AL_INLINE
int AL_Driver_Open(AL_TDriver* driver, const char* device)
{
  return driver->vtable->pfnOpen(driver, device);
}

static AL_INLINE
void AL_Driver_Close(AL_TDriver* driver, int fd)
{
  driver->vtable->pfnClose(driver, fd);
}

static AL_INLINE
AL_EDriverError AL_Driver_PostMessage(AL_TDriver* driver, int fd, long unsigned int messageId, void* data)
{
  return driver->vtable->pfnPostMessage(driver, fd, messageId, data, true);
}

static AL_INLINE
AL_EDriverError AL_Driver_PostMessage2(AL_TDriver* driver, int fd, long unsigned int messageId, void* data, bool isBlocking)
{
  return driver->vtable->pfnPostMessage(driver, fd, messageId, data, isBlocking);
}

/*@}*/

