/**
  @file src/components/ffmpeg/omx_videodec_component.h
  
  This component implements an H.264 / MPEG-4 AVC video decoder. 
  The H.264 / MPEG-4 AVC Video decoder is based on the FFmpeg software library.

  Copyright (C) 2007-2008 STMicroelectronics
  Copyright (C) 2007-2008 Nokia Corporation and/or its subsidiary(-ies).

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA
  02110-1301  USA

  $Date$
  Revision $Rev$
  Author $Author$
*/

#ifndef _OMX_VIDEODEC_COMPONENT_H_
#define _OMX_VIDEODEC_COMPONENT_H_

#include <OMX_Types.h>
#include <OMX_Component.h>
#include <OMX_Core.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <omx_base_filter.h>
#include <string.h>
#include <vdec.h>


#define VIDEO_DEC_BASE_NAME "OMX.st.video_decoder"
#define VIDEO_DEC_MPEG4_NAME "OMX.st.video_decoder.mpeg4"
#define VIDEO_DEC_H264_NAME "OMX.st.video_decoder.avc"
#define VIDEO_DEC_MPEG4_ROLE "video_decoder.mpeg4"
#define VIDEO_DEC_H264_ROLE "video_decoder.avc"

/** Video Decoder component private structure.
  */
DERIVEDCLASS(omx_videodec_component_PrivateType, omx_base_filter_PrivateType)
#define omx_videodec_component_PrivateType_FIELDS omx_base_filter_PrivateType_FIELDS \
  /** @param semaphore for avcodec access syncrhonization */\
  tsem_t* avCodecSyncSem; \
  /** @param config Reference to VDecConfig structure */ \
  VDecConfig config; \
  /** @param dec Reference to VDecHandle structure */ \
  VDecHandle dec;\
  /** @param video_coding_type Field that indicate the supported video format of video decoder */ \
  OMX_U32 video_coding_type;   \
  /** @param avcodecReady boolean flag that is true when the video coded has been initialized */ \
  OMX_BOOL avcodecReady; 
ENDCLASS(omx_videodec_component_PrivateType)

/* Component private entry points declaration */
OMX_ERRORTYPE omx_videodec_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,OMX_STRING cComponentName);
OMX_ERRORTYPE omx_videodec_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_videodec_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_videodec_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_videodec_component_MessageHandler(OMX_COMPONENTTYPE*,internalRequestMessageType*);

void omx_videodec_component_BufferMgmtCallback(
  OMX_COMPONENTTYPE *openmaxStandComp,
  OMX_BUFFERHEADERTYPE* inputbuffer,
  OMX_BUFFERHEADERTYPE* outputbuffer);

OMX_ERRORTYPE omx_videodec_component_GetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_INOUT OMX_PTR ComponentParameterStructure);

OMX_ERRORTYPE omx_videodec_component_SetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_IN  OMX_PTR ComponentParameterStructure);

OMX_ERRORTYPE omx_videodec_component_ComponentRoleEnum(
  OMX_IN OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_U8 *cRole,
  OMX_IN OMX_U32 nIndex);

void SetInternalVideoParameters(OMX_COMPONENTTYPE *openmaxStandComp);

OMX_ERRORTYPE omx_videodec_component_SetConfig(
  OMX_HANDLETYPE hComponent,
  OMX_INDEXTYPE nIndex,
  OMX_PTR pComponentConfigStructure);

OMX_ERRORTYPE omx_videodec_component_GetExtensionIndex(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_STRING cParameterName,
  OMX_OUT OMX_INDEXTYPE* pIndexType);

#endif
