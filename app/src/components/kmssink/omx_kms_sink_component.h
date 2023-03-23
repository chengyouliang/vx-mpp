/**
  @file src/components/kms/omx_kms_sink_component.h
  
  OpenMAX kms sink component. 

  Copyright (C) 2023-2024 vanxum

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.
 
  $Date: 2023/3/23 +0530 (Tue, 02 Sep 2008) $
  Revision $Rev: 593 $
  Author $Author: chengyouliang $
*/

#ifndef _OMX_kms_SINK_COMPONENT_H_
#define _OMX_kms_SINK_COMPONENT_H_

#include <OMX_Types.h>
#include <OMX_Component.h>
#include <OMX_Core.h>
#include <OMX_Video.h>
#include <OMX_IVCommon.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/resource.h>

#include <omx_base_video_port.h>
#include <omx_base_sink.h>

/**  Filename of devnode for framebuffer device
  *  Should somehow be passed from client
  */
#define FBDEV_FILENAME  "/dev/fb0" 

/** FBDEV sink port component port structure.
  */
DERIVEDCLASS(omx_kms_sink_component_PortType, omx_base_video_PortType)
#define omx_kms_sink_component_PortType_FIELDS omx_base_video_PortType_FIELDS \
  /** @param omxConfigCrop Crop rectangle of image */ \
  OMX_CONFIG_RECTTYPE omxConfigCrop; \
  /** @param omxConfigRotate Set rotation angle of image */ \
  OMX_CONFIG_ROTATIONTYPE omxConfigRotate; \
  /** @param omxConfigMirror Set mirroring of image */ \
  OMX_CONFIG_MIRRORTYPE omxConfigMirror; \
  /** @param omxConfigScale Set scale factors */ \
  OMX_CONFIG_SCALEFACTORTYPE omxConfigScale; \
  /** @param omxConfigOutputPosition Top-Left offset from intermediate buffer to output buffer */ \
  OMX_CONFIG_POINTTYPE omxConfigOutputPosition;
ENDCLASS(omx_kms_sink_component_PortType)

/** FBDEV sink port component private structure.
  * see the define above
  * @param fd The file descriptor for the framebuffer 
  * @param vscr_info The fb_var_screeninfo structure for the framebuffer 
  * @param fscr_info The fb_fix_screeninfo structure for the framebuffer
  * @param scr_data Pointer to the mmapped memory for the framebuffer 
  * @param fbpxlfmt frame buffer pixel format
  * @param fbwidth frame buffer display width 
  * @param fbheight frame buffer display height 
  * @param fbbpp frame buffer pixel depth
  * @param fbstride frame buffer display stride 
  * @param xScale the scale of the media clock
  * @param eState the state of the media clock
  * @param product frame buffer memory area 
  * @param frameDropFlag the flag active on scale change indicates that frames are to be dropped 
  * @param dropFrameCount counts the number of frames dropped 
  */
DERIVEDCLASS(omx_kms_sink_component_PrivateType, omx_base_sink_PrivateType)
#define omx_kms_sink_component_PrivateType_FIELDS omx_base_sink_PrivateType_FIELDS \
  OMX_BOOL                    bIskmsInit;\
  tsem_t*                     kmsSyncSem; \
  long                        old_time; \
  long                        new_time;
ENDCLASS(omx_kms_sink_component_PrivateType)

/* Component private entry points declaration */
OMX_ERRORTYPE omx_kms_sink_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,OMX_STRING cComponentName);
OMX_ERRORTYPE omx_kms_sink_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_kms_sink_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_kms_sink_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_kms_sink_component_MessageHandler(OMX_COMPONENTTYPE* , internalRequestMessageType*);

void omx_kms_sink_component_BufferMgmtCallback(
  OMX_COMPONENTTYPE *openmaxStandComp,
  OMX_BUFFERHEADERTYPE* pInputBuffer);

OMX_ERRORTYPE omx_kms_sink_component_port_SendBufferFunction(
  omx_base_PortType *openmaxStandPort,
  OMX_BUFFERHEADERTYPE* pBuffer);

/* to handle the communication at the clock port */
OMX_BOOL omx_kms_sink_component_ClockPortHandleFunction(
  omx_kms_sink_component_PrivateType* omx_kms_sink_component_Private,
  OMX_BUFFERHEADERTYPE* inputbuffer);

OMX_ERRORTYPE omx_kms_sink_component_SetConfig(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nIndex,
  OMX_IN  OMX_PTR pComponentConfigStructure);

OMX_ERRORTYPE omx_kms_sink_component_GetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_INOUT OMX_PTR ComponentParameterStructure);

OMX_ERRORTYPE omx_kms_sink_component_SetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_IN  OMX_PTR ComponentParameterStructure);

OMX_ERRORTYPE omx_kms_sink_component_GetConfig(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nIndex,
  OMX_INOUT OMX_PTR pComponentConfigStructure);

/** function prototypes of some internal functions */

OMX_S32 calcStride(OMX_U32 width, OMX_COLOR_FORMATTYPE omx_pxlfmt);

/** Returns a time value in milliseconds based on a clock starting at
 *  some arbitrary base. Given a call to GetTime that returns a value
 *  of n a subsequent call to GetTime made m milliseconds later should 
 *  return a value of (approximately) (n+m). This method is used, for
 *  instance, to compute the duration of call. */
long GetTime();

#endif
