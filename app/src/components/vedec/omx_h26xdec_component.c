/**
  @file src/components/ffmpeg/omx_videodec_component.c

  This component implements H.264 / MPEG-4 AVC video decoder.
  The H.264 / MPEG-4 AVC Video decoder is based on the FFmpeg software library.

  Copyright (C) 2007-2008 STMicroelectronics
  Copyright (C) 2007-2008 Nokia Corporation and/or its subsidiary(-ies)

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
#include <omxcore.h>
#include <omx_base_video_port.h>
#include "omx_h26xdec_component.h"
#include <OMX_Video.h>



#define VDEC_GENERAL_ERR            (-200)
#define VDEC_FRAME_NOT_READY        (VDEC_GENERAL_ERR-1)
#define VDEC_FRAME_CHANGE           (VDEC_GENERAL_ERR-2)
#define VDEC_END_OF_FRAME           (VDEC_GENERAL_ERR-3)
#define VDEC_DECODE_GO_AHEAD        (VDEC_GENERAL_ERR-4)
#define VMA_ERR_TIMEOUT             (-54)

/** Maximum Number of Video Component Instance*/

#define MAX_COMPONENT_VIDEODEC 4
/** Counter of Video Component Instance*/
static OMX_U32 noVideoDecInstance = 0;

#define DEFAULT_WIDTH 352
#define DEFAULT_HEIGHT 288

/** The output decoded color format */
#define OUTPUT_DECODED_COLOR_FMT OMX_COLOR_FormatYUV420Planar

/** define the max input buffer size */
#define DEFAULT_VIDEO_OUTPUT_BUF_SIZE (DEFAULT_WIDTH*DEFAULT_HEIGHT*3/2)  // YUV 420P

/** The Constructor of the video decoder component
  * @param openmaxStandComp the component handle to be constructed
  * @param cComponentName is the name of the constructed component
  */
OMX_ERRORTYPE omx_videodec_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,OMX_STRING cComponentName) {
   OMX_ERRORTYPE eError = OMX_ErrorNone;
 
  omx_videodec_component_PrivateType* omx_videodec_component_Private;
  omx_base_video_PortType *inPort,*outPort;
  OMX_U32 i;

  if (!openmaxStandComp->pComponentPrivate) {
    DEBUG(DEB_LEV_FUNCTION_NAME, "In %s, allocating component\n", __func__);
    openmaxStandComp->pComponentPrivate = calloc(1, sizeof(omx_videodec_component_PrivateType));
    if(openmaxStandComp->pComponentPrivate == NULL) {
      return OMX_ErrorInsufficientResources;
    }
  } else {
    DEBUG(DEB_LEV_FUNCTION_NAME, "In %s, Error Component %p Already Allocated\n", __func__,openmaxStandComp->pComponentPrivate);
  }

  omx_videodec_component_Private = openmaxStandComp->pComponentPrivate;
  omx_videodec_component_Private->ports = NULL;

  eError = omx_base_filter_Constructor(openmaxStandComp, cComponentName);

  omx_videodec_component_Private->sPortTypesParam[OMX_PortDomainVideo].nStartPortNumber = 0;
  omx_videodec_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts = 2;

  /** Allocate Ports and call port constructor. */
  if (omx_videodec_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts && !omx_videodec_component_Private->ports) {
    omx_videodec_component_Private->ports = calloc(omx_videodec_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts, sizeof(omx_base_PortType *));
    if (!omx_videodec_component_Private->ports) {
      return OMX_ErrorInsufficientResources;
    }
    for (i=0; i < omx_videodec_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts; i++) {
      omx_videodec_component_Private->ports[i] = calloc(1, sizeof(omx_base_video_PortType));
      if (!omx_videodec_component_Private->ports[i]) {
        return OMX_ErrorInsufficientResources;
      }
    }
  }

  base_video_port_Constructor(openmaxStandComp, &omx_videodec_component_Private->ports[0], 0, OMX_TRUE);
  base_video_port_Constructor(openmaxStandComp, &omx_videodec_component_Private->ports[1], 1, OMX_FALSE);

  /** here we can override whatever defaults the base_component constructor set
    * e.g. we can override the function pointers in the private struct
    */

  /** Domain specific section for the ports.
    * first we set the parameter common to both formats
    */
  //common parameters related to input port
  inPort = (omx_base_video_PortType *)omx_videodec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
  inPort->sPortParam.nBufferSize = DEFAULT_OUT_BUFFER_SIZE;
  inPort->sPortParam.format.video.xFramerate = 25;

  //common parameters related to output port
  outPort = (omx_base_video_PortType *)omx_videodec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
  outPort->sPortParam.format.video.eColorFormat = OUTPUT_DECODED_COLOR_FMT;
  outPort->sPortParam.nBufferSize = DEFAULT_VIDEO_OUTPUT_BUF_SIZE;
  outPort->sPortParam.format.video.xFramerate = 25;

  /** settings of output port parameter definition */
  outPort->sVideoParam.eColorFormat = OUTPUT_DECODED_COLOR_FMT;
  //outPort->sVideoParam.xFramerate = 25;

  /** now it's time to know the video coding type of the component */
  if(!strcmp(cComponentName, VIDEO_DEC_MPEG4_NAME)) {
    omx_videodec_component_Private->video_coding_type = OMX_VIDEO_CodingMPEG4;
  } else if(!strcmp(cComponentName, VIDEO_DEC_H264_NAME)) {
    omx_videodec_component_Private->video_coding_type = OMX_VIDEO_CodingAVC;
  } else if (!strcmp(cComponentName, VIDEO_DEC_BASE_NAME)) {
    omx_videodec_component_Private->video_coding_type = OMX_VIDEO_CodingUnused;
  } else {
    // IL client specified an invalid component name
    return OMX_ErrorInvalidComponentName;
  }

  if(!omx_videodec_component_Private->avCodecSyncSem) {
    omx_videodec_component_Private->avCodecSyncSem = calloc(1,sizeof(tsem_t));
    if(omx_videodec_component_Private->avCodecSyncSem == NULL) {
      return OMX_ErrorInsufficientResources;
    }
    tsem_init(omx_videodec_component_Private->avCodecSyncSem, 0);
  }
  //omx_videodec_component_Private->eOutFramePixFmt = PIX_FMT_YUV420P;

  if(omx_videodec_component_Private->video_coding_type == OMX_VIDEO_CodingMPEG4) {
    omx_videodec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingMPEG4;
  } else {
    omx_videodec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingAVC;
  }

  omx_videodec_component_Private->BufferMgmtCallback = omx_videodec_component_BufferMgmtCallback;

  /** initializing the codec context etc that was done earlier by ffmpeglibinit function */
  omx_videodec_component_Private->messageHandler = omx_videodec_component_MessageHandler;
  omx_videodec_component_Private->destructor = omx_videodec_component_Destructor;
  openmaxStandComp->SetParameter = omx_videodec_component_SetParameter;
  openmaxStandComp->GetParameter = omx_videodec_component_GetParameter;
  //openmaxStandComp->ComponentRoleEnum = omx_videodec_component_ComponentRoleEnum;
  
  noVideoDecInstance++;
  
  // set default config
  omx_videodec_component_Private->config.splitInput  = 0;
	omx_videodec_component_Private->config.tiledFmt    = 1;
	omx_videodec_component_Private->config.mutiChunk   = 0;
	omx_videodec_component_Private->config.latencyMode = 0;
	omx_videodec_component_Private->config.type        = VDEC_CODEC_AVC;

  if(noVideoDecInstance > MAX_COMPONENT_VIDEODEC) {
    return OMX_ErrorInsufficientResources;
  }
  return eError;
}


/** The destructor of the video decoder component
  */
OMX_ERRORTYPE omx_videodec_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp) {
  omx_videodec_component_PrivateType* omx_videodec_component_Private = openmaxStandComp->pComponentPrivate;
  OMX_U32 i;

  return OMX_ErrorNone;
}

/** The Initialization function of the video decoder
  */
OMX_ERRORTYPE omx_videodec_component_Init(OMX_COMPONENTTYPE *openmaxStandComp) {
  OMX_ERRORTYPE eError = OMX_ErrorNone;
  omx_videodec_component_PrivateType* omx_videodec_component_Private = (omx_videodec_component_PrivateType*)openmaxStandComp->pComponentPrivate;
  omx_videodec_component_Private->dec = createVideoDecoder(&omx_videodec_component_Private->config);
  if (omx_videodec_component_Private->dec == NULL)
  {
		  eError = OMX_ErrorHardware;
  }
  return eError;
}

/** The Deinitialization function of the video decoder
  */
OMX_ERRORTYPE omx_videodec_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp) {
  OMX_ERRORTYPE eError = OMX_ErrorNone;
  omx_videodec_component_PrivateType* omx_videodec_component_Private = (omx_videodec_component_PrivateType*)openmaxStandComp->pComponentPrivate;
  destroyVideoDecoder(omx_videodec_component_Private->dec);
  return eError;
}

static void omx_writePlane(OMX_BUFFERHEADERTYPE* dec_buf, uint8_t* data, uint32_t width, uint32_t height, uint32_t stride)
{
	if (width == stride) {
		memcpy(dec_buf+dec_buf->nFilledLen, data, width * height);
    dec_buf->nFilledLen += width * height;
	} else {
		for (uint32_t  h = 0; h < height; h++) {
      memcpy(dec_buf+dec_buf->nFilledLen, data,width);
      dec_buf->nFilledLen += width;
			data += stride;
		}
	}
}
static void omx_hander_to_buffer(OMX_BUFFERHEADERTYPE*dec_buf, FrameHandle frame)
{
	uint32_t format;
	uint32_t pitch[4];
	uint8_t *data[4];
	uint32_t width, height;
	uint64_t str_fmt;
	decodeGetFrameDim(frame, &width, &height);
	decodeGetFrameFourCC(frame, &format);
	decodeGetFrameData(frame, data);
	decodeGetFrameStride(frame, pitch);
	str_fmt = format;

	if (height % 2 != 0)
		height +=1;
	if (width % 2 != 0)
		width += 1;

	DEBUG(DEB_LEV_PARAMS, "format:%s width=%d height=%d stride=%d\n", (char *)&str_fmt, width, height, pitch[0]);
	switch (format){
	case VDEC_FORMAT_I444:
	case VDEC_FORMAT_I4AL:
		width = (format == VDEC_FORMAT_I4AL) ? width * 2 : width;
		omx_writePlane(dec_buf, data[0], width, height, pitch[0]);
		omx_writePlane(dec_buf, data[1], width, height, pitch[1]);
		omx_writePlane(dec_buf, data[2], width, height, pitch[2]);
		break;
	case VDEC_FORMAT_NV12:
	case VDEC_FORMAT_P010:
		width = (format == VDEC_FORMAT_P010) ? width * 2 : width;
		omx_writePlane(dec_buf, data[0], width, height, pitch[0]);
		omx_writePlane(dec_buf, data[1], width, height / 2, pitch[1]);
		break;
	case VDEC_FORMAT_NV16:
	case VDEC_FORMAT_P210:
		width = (format == VDEC_FORMAT_P210) ? width * 2 : width;
		omx_writePlane(dec_buf, data[0], width, height, pitch[0]);
		omx_writePlane(dec_buf, data[1], width, height, pitch[1]);
		break;
	case VDEC_FORMAT_T608:
	case VDEC_FORMAT_T60A:
		memcpy(dec_buf+dec_buf->nFilledLen, data[0], pitch[0] * height);
    dec_buf->nFilledLen += pitch[0] * height;
		memcpy(dec_buf+dec_buf->nFilledLen, data[1], pitch[1] * height / 2);
    dec_buf->nFilledLen +=pitch[1] * height / 2;
		break;
	case VDEC_FORMAT_T628:
	case VDEC_FORMAT_T62A:
  case VDEC_FORMAT_T648:
	case VDEC_FORMAT_T64A:
		memcpy(dec_buf+dec_buf->nFilledLen, data[0], pitch[0] * height);
    dec_buf->nFilledLen += pitch[0] * height;
		memcpy(dec_buf+dec_buf->nFilledLen, data[1], pitch[1] * height);
    dec_buf->nFilledLen += pitch[1] * height;
		break;
	default:
		DEBUG(DEB_LEV_ERR,"[%s] format not supported.\n", (char *)&str_fmt);
		break;
	}
}

/** This function is used to process the input buffer and provide one output buffer
  */
void omx_videodec_component_BufferMgmtCallback(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* pInputBuffer, OMX_BUFFERHEADERTYPE* pOutputBuffer) {
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    size_t n;
    omx_videodec_component_PrivateType* omx_videodec_component_Private = (omx_videodec_component_PrivateType*)openmaxStandComp->pComponentPrivate;
    pOutputBuffer->nFilledLen = 0;
		decodePutStream(omx_videodec_component_Private->dec, pInputBuffer->pBuffer, pInputBuffer->nSize);
    FrameHandle hFrame;
		n = decodeGetFrameSync(omx_videodec_component_Private->dec, &hFrame);
		if (n == (size_t)VDEC_FRAME_CHANGE)
			 n = decodeGetFrameSync(omx_videodec_component_Private->dec, &hFrame);
		if (n == (size_t)VMA_ERR_TIMEOUT) {
      DEBUG(DEB_LEV_ERR, "In %s  Video Decoder out\n",__func__);
      return;
		}
    omx_hander_to_buffer(pOutputBuffer,hFrame);
}

OMX_ERRORTYPE omx_videodec_component_SetParameter(
OMX_IN  OMX_HANDLETYPE hComponent,
OMX_IN  OMX_INDEXTYPE nParamIndex,
OMX_IN  OMX_PTR ComponentParameterStructure) {

  OMX_ERRORTYPE eError = OMX_ErrorNone;

  return eError;
}

OMX_ERRORTYPE omx_videodec_component_GetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_INOUT OMX_PTR ComponentParameterStructure) {

  return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_videodec_component_MessageHandler(OMX_COMPONENTTYPE* openmaxStandComp,internalRequestMessageType *message) {
    OMX_ERRORTYPE err;
  omx_videodec_component_PrivateType* omx_videodec_component_Private = (omx_videodec_component_PrivateType*)openmaxStandComp->pComponentPrivate;

  OMX_STATETYPE eCurrentState = omx_videodec_component_Private->state;

  DEBUG(DEB_LEV_FUNCTION_NAME, "In %s\n", __func__);

  if (message->messageType == OMX_CommandStateSet){
    if ((message->messageParam == OMX_StateExecuting ) && (omx_videodec_component_Private->state == OMX_StateIdle)) {
       //omx_videodec_component_Private->isFirstBuffer = OMX_TRUE;
    }
    else if ((message->messageParam == OMX_StateIdle ) && (omx_videodec_component_Private->state == OMX_StateLoaded)) {
      err = omx_videodec_component_Init(openmaxStandComp);
      if(err!=OMX_ErrorNone) {
        DEBUG(DEB_LEV_ERR, "In %s Video Decoder Init Failed Error=%x\n",__func__,err);
        return err;
      }
    } else if ((message->messageParam == OMX_StateLoaded) && (omx_videodec_component_Private->state == OMX_StateIdle)) {
      err = omx_videodec_component_Deinit(openmaxStandComp);
      if(err!=OMX_ErrorNone) {
        DEBUG(DEB_LEV_ERR, "In %s Video Decoder Deinit Failed Error=%x\n",__func__,err);
        return err;
      }
    }
  }
  // Execute the base message handling
  err =  omx_base_component_MessageHandler(openmaxStandComp,message);

  if (message->messageType == OMX_CommandStateSet){
   if ((message->messageParam == OMX_StateIdle  ) && (eCurrentState == OMX_StateExecuting)) {
      if (omx_videodec_component_Private->avcodecReady) {
         // omx_videodec_component_ffmpegLibDeInit(omx_videodec_component_Private);
        omx_videodec_component_Private->avcodecReady = OMX_FALSE;
      }
    }
  }
  return err;
}
OMX_ERRORTYPE omx_videodec_component_ComponentRoleEnum(
  OMX_IN OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_U8 *cRole,
  OMX_IN OMX_U32 nIndex) {
}