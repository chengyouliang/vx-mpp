/**
  @file src/components/kms/omx_kms_sink_component.c
  
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

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA
 
  $Date: 2023/3/23 +0530 (Tue, 02 Sep 2008) $
  Revision $Rev: 593 $
  Author $Author: chengyouliang $
*/

#include <omxcore.h>
#include <omx_kms_sink_component.h>




/** Returns a time value in milliseconds based on a clock starting at
 *  some arbitrary base. Given a call to GetTime that returns a value
 *  of n a subsequent call to GetTime made m milliseconds later should 
 *  return a value of (approximately) (n+m). This method is used, for
 *  instance, to compute the duration of call. */
long GetTime() {
    struct timeval now;
    gettimeofday(&now, NULL);
    return ((long)now.tv_sec) * 1000 + ((long)now.tv_usec) / 1000;
}

/** The Constructor
 * 
 * @param openmaxStandComp is the handle to be constructed
 * @param cComponentName is the name of the constructed component
 * 
 */ 
OMX_ERRORTYPE omx_kms_sink_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,OMX_STRING cComponentName) {
  OMX_ERRORTYPE err = OMX_ErrorNone;  
  omx_kms_sink_component_PortType *pPort;
  omx_kms_sink_component_PrivateType* omx_kms_sink_component_Private;

  if (!openmaxStandComp->pComponentPrivate) {
    DEBUG(DEB_LEV_FUNCTION_NAME, "In %s, allocating component\n", __func__);
    openmaxStandComp->pComponentPrivate = calloc(1, sizeof(omx_kms_sink_component_PrivateType));
    if(openmaxStandComp->pComponentPrivate == NULL) {
      return OMX_ErrorInsufficientResources;
    }
  } else {
    DEBUG(DEB_LEV_FUNCTION_NAME, "In %s, Error Component %p Already Allocated\n", __func__, openmaxStandComp->pComponentPrivate);
  }

  omx_kms_sink_component_Private = openmaxStandComp->pComponentPrivate;
  omx_kms_sink_component_Private->ports = NULL;
  
  /** we could create our own port structures here
    * fixme maybe the base class could use a "port factory" function pointer?  
    */
  err = omx_base_sink_Constructor(openmaxStandComp, cComponentName);

  omx_kms_sink_component_Private->sPortTypesParam[OMX_PortDomainVideo].nStartPortNumber = 0;
  omx_kms_sink_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts = 1;

  /** Allocate Ports and call port constructor. */  
  if ((omx_kms_sink_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts )  && !omx_kms_sink_component_Private->ports) {
    omx_kms_sink_component_Private->ports = calloc((omx_kms_sink_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts ), sizeof(omx_base_PortType *));
    if (!omx_kms_sink_component_Private->ports) {
      return OMX_ErrorInsufficientResources;
    }
    omx_kms_sink_component_Private->ports[0] = calloc(1, sizeof(omx_kms_sink_component_PortType));
    if (!omx_kms_sink_component_Private->ports[0]) {
      return OMX_ErrorInsufficientResources;
    }
    base_video_port_Constructor(openmaxStandComp, &omx_kms_sink_component_Private->ports[0], 0, OMX_TRUE);
  }

  pPort = (omx_kms_sink_component_PortType *) omx_kms_sink_component_Private->ports[OMX_BASE_SINK_INPUTPORT_INDEX];

  /** Domain specific section for the allocated port. */

  pPort->sPortParam.format.video.nFrameWidth = 1920;
  pPort->sPortParam.format.video.nFrameHeight = 1080;
  pPort->sPortParam.format.video.nBitrate = 0;
  pPort->sPortParam.format.video.xFramerate = 25;
  pPort->sPortParam.format.video.eColorFormat = DRM_FORMAT_NV12;

  //  Figure out stride, slice height, min buffer size

  pPort->sPortParam.format.video.nStride = calcStride(pPort->sPortParam.format.video.nFrameWidth, pPort->sPortParam.format.video.eColorFormat);
  pPort->sPortParam.format.video.nSliceHeight = pPort->sPortParam.format.video.nFrameHeight;  //  No support for slices yet
  pPort->sPortParam.nBufferSize = (OMX_U32) abs(pPort->sPortParam.format.video.nStride) * pPort->sPortParam.format.video.nSliceHeight;

  pPort->sVideoParam.eColorFormat = OMX_COLOR_FormatYUV420Planar;
  //pPort->sVideoParam.xFramerate = 25;

  DEBUG(DEB_LEV_PARAMS, "In %s, bSize=%d stride=%d\n", __func__,(int)pPort->sPortParam.nBufferSize,(int)pPort->sPortParam.format.video.nStride);
  
  /** Set configs */
  setHeader(&pPort->omxConfigCrop, sizeof(OMX_CONFIG_RECTTYPE));  
  pPort->omxConfigCrop.nPortIndex = OMX_BASE_SINK_INPUTPORT_INDEX;
  pPort->omxConfigCrop.nLeft = pPort->omxConfigCrop.nTop = 0;
  pPort->omxConfigCrop.nWidth = pPort->omxConfigCrop.nHeight = 0;

  setHeader(&pPort->omxConfigRotate, sizeof(OMX_CONFIG_ROTATIONTYPE));  
  pPort->omxConfigRotate.nPortIndex = OMX_BASE_SINK_INPUTPORT_INDEX;
  pPort->omxConfigRotate.nRotation = 0;  //Default: No rotation (0 degrees)

  setHeader(&pPort->omxConfigMirror, sizeof(OMX_CONFIG_MIRRORTYPE));  
  pPort->omxConfigMirror.nPortIndex = OMX_BASE_SINK_INPUTPORT_INDEX;
  pPort->omxConfigMirror.eMirror = OMX_MirrorNone;  //Default: No mirroring

  setHeader(&pPort->omxConfigScale, sizeof(OMX_CONFIG_SCALEFACTORTYPE));  
  pPort->omxConfigScale.nPortIndex = OMX_BASE_SINK_INPUTPORT_INDEX;
  pPort->omxConfigScale.xWidth = pPort->omxConfigScale.xHeight = 0x10000;  //Default: No scaling (scale factor = 1)

  setHeader(&pPort->omxConfigOutputPosition, sizeof(OMX_CONFIG_POINTTYPE));  
  pPort->omxConfigOutputPosition.nPortIndex = OMX_BASE_SINK_INPUTPORT_INDEX;
  pPort->omxConfigOutputPosition.nX = pPort->omxConfigOutputPosition.nY = 0; //Default: No shift in output position (0,0)

  /** set the function pointers */
  omx_kms_sink_component_Private->destructor = omx_kms_sink_component_Destructor;
  omx_kms_sink_component_Private->BufferMgmtCallback = omx_kms_sink_component_BufferMgmtCallback;
  openmaxStandComp->SetParameter = omx_kms_sink_component_SetParameter;
  openmaxStandComp->GetParameter = omx_kms_sink_component_GetParameter;
  omx_kms_sink_component_Private->messageHandler = omx_kms_sink_component_MessageHandler;

  omx_kms_sink_component_Private->bIskmsInit = OMX_FALSE;
  if(!omx_kms_sink_component_Private->kmsSyncSem) {
    omx_kms_sink_component_Private->kmsSyncSem = calloc(1,sizeof(tsem_t));
    if(omx_kms_sink_component_Private->kmsSyncSem == NULL) {
      return OMX_ErrorInsufficientResources;
    }
    tsem_init(omx_kms_sink_component_Private->kmsSyncSem, 0);
  }
  return err;
}

/** The Destructor 
 */
OMX_ERRORTYPE omx_kms_sink_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp) {
  omx_kms_sink_component_PrivateType* omx_kms_sink_component_Private = openmaxStandComp->pComponentPrivate;
  OMX_U32 i;
 
  /* frees port/s */
  if (omx_kms_sink_component_Private->ports) {
    for (i=0; i < (omx_kms_sink_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts); i++) {
      if(omx_kms_sink_component_Private->ports[i])
        omx_kms_sink_component_Private->ports[i]->PortDestructor(omx_kms_sink_component_Private->ports[i]);
    }
    free(omx_kms_sink_component_Private->ports);
    omx_kms_sink_component_Private->ports=NULL;
  }

  if(omx_kms_sink_component_Private->kmsSyncSem) {
    tsem_deinit(omx_kms_sink_component_Private->kmsSyncSem); 
    free(omx_kms_sink_component_Private->kmsSyncSem);
    omx_kms_sink_component_Private->kmsSyncSem = NULL;
  }

  omx_base_sink_Destructor(openmaxStandComp);

  return OMX_ErrorNone;
}

/** The initialization function 
  * This function opens the frame buffer device and allocates memory for display 
  * also it finds the frame buffer supported display formats
  */
OMX_ERRORTYPE omx_kms_sink_component_Init(OMX_COMPONENTTYPE *openmaxStandComp) {
  omx_kms_sink_component_PrivateType* omx_kms_sink_component_Private = openmaxStandComp->pComponentPrivate;
  omx_kms_sink_component_PortType* pPort = (omx_kms_sink_component_PortType *) omx_kms_sink_component_Private->ports[OMX_BASE_SINK_INPUTPORT_INDEX];
  int yuv_width  = pPort->sPortParam.format.video.nFrameWidth;
  int yuv_height = pPort->sPortParam.format.video.nFrameHeight;
  int yuv_Format = pPort->sPortParam.format.video.eColorFormat;
  printf("%s %d\n",__FUNCTION__,__LINE__);
  if (IMG_FMT_NV12 == OMX_COLOR_FormatYUV420Planar)
  {
      yuv_Format = IMG_FMT_NV12;
  }
  unsigned int err,i;
  printf("%s %d\n",__FUNCTION__,__LINE__);
  omx_kms_sink_component_Private->pdev  = drm_device_create(yuv_width,yuv_height,1,yuv_Format);
  if (!omx_kms_sink_component_Private->pdev)
  {
      DEBUG(DEB_LEV_ERR, "In %s, device_create error\n", __func__);
      return OMX_ErrorInsufficientResources;
  }
   printf("%s %d\n",__FUNCTION__,__LINE__);
  /*Signal kms Initialized*/
  tsem_up(omx_kms_sink_component_Private->kmsSyncSem);

  return OMX_ErrorNone;
}

/** The deinitialization function 
  * It deallocates the frame buffer memory, and closes frame buffer
  */
OMX_ERRORTYPE omx_kms_sink_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp) {
  omx_kms_sink_component_PrivateType* omx_kms_sink_component_Private = openmaxStandComp->pComponentPrivate;

  omx_kms_sink_component_Private->bIskmsInit = OMX_FALSE;
  drm_device_destroy(omx_kms_sink_component_Private->pdev);
  return OMX_ErrorNone;
}


/** buffer management callback function 
  * takes one input buffer and displays its contents 
  */
void omx_kms_sink_component_BufferMgmtCallback(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* pInputBuffer) {
  omx_kms_sink_component_PrivateType* omx_kms_sink_component_Private = openmaxStandComp->pComponentPrivate;
  OMX_S32 ret;
  ret = drm_show(omx_kms_sink_component_Private->pdev,pInputBuffer->pBuffer,pInputBuffer->nFilledLen);
  if (ret < 0)
  {
     DEBUG(DEB_LEV_ERR, "In %s, drm_show error\n", __func__);
  }
  pInputBuffer->nFilledLen = 0;
}


OMX_ERRORTYPE omx_kms_sink_component_SetConfig(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nIndex,
  OMX_IN  OMX_PTR pComponentConfigStructure) {

  OMX_ERRORTYPE err = OMX_ErrorNone;
  OMX_U32 portIndex;
  OMX_CONFIG_RECTTYPE *omxConfigCrop;
  OMX_CONFIG_ROTATIONTYPE *omxConfigRotate;
  OMX_CONFIG_MIRRORTYPE *omxConfigMirror;
  OMX_CONFIG_SCALEFACTORTYPE *omxConfigScale;
  OMX_CONFIG_POINTTYPE *omxConfigOutputPosition;

  /* Check which structure we are being fed and make control its header */
  OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
  omx_kms_sink_component_PrivateType* omx_kms_sink_component_Private = openmaxStandComp->pComponentPrivate;
  omx_kms_sink_component_PortType *pPort;
  if (pComponentConfigStructure == NULL) {
    return OMX_ErrorBadParameter;
  }

  DEBUG(DEB_LEV_SIMPLE_SEQ, "   Setting parameter %i\n", nIndex);

  switch (nIndex) {
    case OMX_IndexConfigCommonInputCrop:
      omxConfigCrop = (OMX_CONFIG_RECTTYPE*)pComponentConfigStructure;
      portIndex = omxConfigCrop->nPortIndex;
      if ((err = checkHeader(pComponentConfigStructure, sizeof(OMX_CONFIG_RECTTYPE))) != OMX_ErrorNone) { 
        break;
      }
      if (portIndex == OMX_BASE_SINK_INPUTPORT_INDEX) {
        pPort = (omx_kms_sink_component_PortType *) omx_kms_sink_component_Private->ports[portIndex];
        pPort->omxConfigCrop.nLeft = omxConfigCrop->nLeft;
        pPort->omxConfigCrop.nTop = omxConfigCrop->nTop;
        pPort->omxConfigCrop.nWidth = omxConfigCrop->nWidth;
        pPort->omxConfigCrop.nHeight = omxConfigCrop->nHeight;
      } else {
        return OMX_ErrorBadPortIndex;
      }
      break;
    case OMX_IndexConfigCommonRotate:
      omxConfigRotate = (OMX_CONFIG_ROTATIONTYPE*)pComponentConfigStructure;
      portIndex = omxConfigRotate->nPortIndex;
      if ((err = checkHeader(pComponentConfigStructure, sizeof(OMX_CONFIG_ROTATIONTYPE))) != OMX_ErrorNone) { 
        break;
      }
      if (portIndex == 0) {
        pPort = (omx_kms_sink_component_PortType *) omx_kms_sink_component_Private->ports[portIndex];
        if (omxConfigRotate->nRotation != 0) {
          //  Rotation not supported (yet)
          return OMX_ErrorUnsupportedSetting;
        }
        pPort->omxConfigRotate.nRotation = omxConfigRotate->nRotation;
      } else {
        return OMX_ErrorBadPortIndex;
      }
      break;
    case OMX_IndexConfigCommonMirror:
      omxConfigMirror = (OMX_CONFIG_MIRRORTYPE*)pComponentConfigStructure;
      portIndex = omxConfigMirror->nPortIndex;
      if ((err = checkHeader(pComponentConfigStructure, sizeof(OMX_CONFIG_MIRRORTYPE))) != OMX_ErrorNone) { 
        break;
      }
      if (portIndex == 0) {
        if (omxConfigMirror->eMirror == OMX_MirrorBoth || omxConfigMirror->eMirror == OMX_MirrorHorizontal)  {
          //  Horizontal mirroring not yet supported
          return OMX_ErrorUnsupportedSetting;
        }
        pPort = (omx_kms_sink_component_PortType *) omx_kms_sink_component_Private->ports[portIndex];
        pPort->omxConfigMirror.eMirror = omxConfigMirror->eMirror;
      } else {
        return OMX_ErrorBadPortIndex;
      }
      break;
    case OMX_IndexConfigCommonScale:
      omxConfigScale = (OMX_CONFIG_SCALEFACTORTYPE*)pComponentConfigStructure;
      portIndex = omxConfigScale->nPortIndex;
      if ((err = checkHeader(pComponentConfigStructure, sizeof(OMX_CONFIG_SCALEFACTORTYPE))) != OMX_ErrorNone) { 
        break;
      }
      if (portIndex == 0) {
        if (omxConfigScale->xWidth != 0x10000 || omxConfigScale->xHeight != 0x10000)  {
          //  Scaling not yet supported
          return OMX_ErrorUnsupportedSetting;
        }
        pPort = (omx_kms_sink_component_PortType *) omx_kms_sink_component_Private->ports[portIndex];
        pPort->omxConfigScale.xWidth = omxConfigScale->xWidth;
        pPort->omxConfigScale.xHeight = omxConfigScale->xHeight;
      } else {
        return OMX_ErrorBadPortIndex;
      }
      break;
    case OMX_IndexConfigCommonOutputPosition:
      omxConfigOutputPosition = (OMX_CONFIG_POINTTYPE*)pComponentConfigStructure;
      portIndex = omxConfigOutputPosition->nPortIndex;
      if ((err = checkHeader(pComponentConfigStructure, sizeof(OMX_CONFIG_POINTTYPE))) != OMX_ErrorNone) { 
        break;
      }
      if (portIndex == 0) {
        pPort = (omx_kms_sink_component_PortType *) omx_kms_sink_component_Private->ports[portIndex];
        pPort->omxConfigOutputPosition.nX = omxConfigOutputPosition->nX;
        pPort->omxConfigOutputPosition.nY = omxConfigOutputPosition->nY;
      } else {
        return OMX_ErrorBadPortIndex;
      }
      break;
    default: // delegate to superclass
      return omx_base_component_SetConfig(hComponent, nIndex, pComponentConfigStructure);
  }
  return err;
}



OMX_ERRORTYPE omx_kms_sink_component_GetConfig(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nIndex,
  OMX_INOUT OMX_PTR pComponentConfigStructure) {

  OMX_CONFIG_RECTTYPE *omxConfigCrop;
  OMX_CONFIG_ROTATIONTYPE *omxConfigRotate;
  OMX_CONFIG_MIRRORTYPE *omxConfigMirror;
  OMX_CONFIG_SCALEFACTORTYPE *omxConfigScale;
  OMX_CONFIG_POINTTYPE *omxConfigOutputPosition;

  OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
  omx_kms_sink_component_PrivateType* omx_kms_sink_component_Private = openmaxStandComp->pComponentPrivate;
  omx_kms_sink_component_PortType *pPort;
  if (pComponentConfigStructure == NULL) {
    return OMX_ErrorBadParameter;
  }
  DEBUG(DEB_LEV_SIMPLE_SEQ, "   Getting configuration %i\n", nIndex);
  /* Check which structure we are being fed and fill its header */
  switch (nIndex) {
    case OMX_IndexConfigCommonInputCrop:
      omxConfigCrop = (OMX_CONFIG_RECTTYPE*)pComponentConfigStructure;
      setHeader(omxConfigCrop, sizeof(OMX_CONFIG_RECTTYPE));
      if (omxConfigCrop->nPortIndex == 0) {
        pPort = (omx_kms_sink_component_PortType *)omx_kms_sink_component_Private->ports[omxConfigCrop->nPortIndex];
        memcpy(omxConfigCrop, &pPort->omxConfigCrop, sizeof(OMX_CONFIG_RECTTYPE));
      } else {
        return OMX_ErrorBadPortIndex;
      }
    break;    
    case OMX_IndexConfigCommonRotate:
      omxConfigRotate = (OMX_CONFIG_ROTATIONTYPE*)pComponentConfigStructure;
      setHeader(omxConfigRotate, sizeof(OMX_CONFIG_ROTATIONTYPE));
      if (omxConfigRotate->nPortIndex == 0) {
        pPort = (omx_kms_sink_component_PortType *)omx_kms_sink_component_Private->ports[omxConfigRotate->nPortIndex];
        memcpy(omxConfigRotate, &pPort->omxConfigRotate, sizeof(OMX_CONFIG_ROTATIONTYPE));
      } else {
        return OMX_ErrorBadPortIndex;
      }
      break;    
    case OMX_IndexConfigCommonMirror:
      omxConfigMirror = (OMX_CONFIG_MIRRORTYPE*)pComponentConfigStructure;
      setHeader(omxConfigMirror, sizeof(OMX_CONFIG_MIRRORTYPE));
      if (omxConfigMirror->nPortIndex == 0) {
        pPort = (omx_kms_sink_component_PortType *)omx_kms_sink_component_Private->ports[omxConfigMirror->nPortIndex];
        memcpy(omxConfigMirror, &pPort->omxConfigMirror, sizeof(OMX_CONFIG_MIRRORTYPE));
      } else {
        return OMX_ErrorBadPortIndex;
      }
      break;      
    case OMX_IndexConfigCommonScale:
      omxConfigScale = (OMX_CONFIG_SCALEFACTORTYPE*)pComponentConfigStructure;
      setHeader(omxConfigScale, sizeof(OMX_CONFIG_SCALEFACTORTYPE));
      if (omxConfigScale->nPortIndex == 0) {
        pPort = (omx_kms_sink_component_PortType *)omx_kms_sink_component_Private->ports[omxConfigScale->nPortIndex];
        memcpy(omxConfigScale, &pPort->omxConfigScale, sizeof(OMX_CONFIG_SCALEFACTORTYPE));
      } else {
        return OMX_ErrorBadPortIndex;
      }
      break;    
    case OMX_IndexConfigCommonOutputPosition:
      omxConfigOutputPosition = (OMX_CONFIG_POINTTYPE*)pComponentConfigStructure;
      setHeader(omxConfigOutputPosition, sizeof(OMX_CONFIG_POINTTYPE));
      if (omxConfigOutputPosition->nPortIndex == 0) {
        pPort = (omx_kms_sink_component_PortType *)omx_kms_sink_component_Private->ports[omxConfigOutputPosition->nPortIndex];
        memcpy(omxConfigOutputPosition, &pPort->omxConfigOutputPosition, sizeof(OMX_CONFIG_POINTTYPE));
      } else {
        return OMX_ErrorBadPortIndex;
      }
      break;    
    default: // delegate to superclass
      return omx_base_component_GetConfig(hComponent, nIndex, pComponentConfigStructure);
  }
  return OMX_ErrorNone;
}



OMX_ERRORTYPE omx_kms_sink_component_SetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_IN  OMX_PTR ComponentParameterStructure) {

  OMX_ERRORTYPE err = OMX_ErrorNone;
  OMX_PARAM_PORTDEFINITIONTYPE *pPortDef;
  OMX_VIDEO_PARAM_PORTFORMATTYPE *pVideoPortFormat;
  OMX_U32 portIndex;

  /* Check which structure we are being fed and make control its header */
  OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
  omx_kms_sink_component_PrivateType* omx_kms_sink_component_Private = openmaxStandComp->pComponentPrivate;
  omx_kms_sink_component_PortType *pPort;
  
  if (ComponentParameterStructure == NULL) {
    return OMX_ErrorBadParameter;
  }

  DEBUG(DEB_LEV_SIMPLE_SEQ, "   Setting parameter %i\n", nParamIndex);
  switch(nParamIndex) {
     case OMX_IndexParamPortDefinition:
      pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE*) ComponentParameterStructure;
      portIndex = pPortDef->nPortIndex;
      err = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
      if(err!=OMX_ErrorNone) { 
        DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,err); 
        break;
      }
      
      if(portIndex > (omx_kms_sink_component_Private->sPortTypesParam[OMX_PortDomainVideo].nPorts)) {
        return OMX_ErrorBadPortIndex;
      }

      if(portIndex == 0) {

        pPort = (omx_kms_sink_component_PortType *) omx_kms_sink_component_Private->ports[portIndex];
      
        pPort->sPortParam.nBufferCountActual = pPortDef->nBufferCountActual;
        //  Copy stuff from OMX_VIDEO_PORTDEFINITIONTYPE structure
        if(pPortDef->format.video.cMIMEType != NULL) {
          strcpy(pPort->sPortParam.format.video.cMIMEType , pPortDef->format.video.cMIMEType);
        }
        pPort->sPortParam.format.video.nFrameWidth  = pPortDef->format.video.nFrameWidth;
        pPort->sPortParam.format.video.nFrameHeight = pPortDef->format.video.nFrameHeight;
        pPort->sPortParam.format.video.nBitrate     = pPortDef->format.video.nBitrate;
        pPort->sPortParam.format.video.xFramerate   = pPortDef->format.video.xFramerate;
        pPort->sPortParam.format.video.bFlagErrorConcealment = pPortDef->format.video.bFlagErrorConcealment; 
        pPort->sVideoParam.eColorFormat             = pPortDef->format.video.eColorFormat;;
        pPort->sPortParam.format.video.eColorFormat = pPortDef->format.video.eColorFormat;

        //  Figure out stride, slice height, min buffer size
        pPort->sPortParam.format.video.nStride = calcStride(pPort->sPortParam.format.video.nFrameWidth, pPort->sVideoParam.eColorFormat);
        pPort->sPortParam.format.video.nSliceHeight = pPort->sPortParam.format.video.nFrameHeight;  //  No support for slices yet
        // Read-only field by spec

        pPort->sPortParam.nBufferSize = (OMX_U32) abs(pPort->sPortParam.format.video.nStride) * pPort->sPortParam.format.video.nSliceHeight;
        pPort->omxConfigCrop.nWidth = pPort->sPortParam.format.video.nFrameWidth;
        pPort->omxConfigCrop.nHeight = pPort->sPortParam.format.video.nFrameHeight;
      } 
      break;

    case OMX_IndexParamVideoPortFormat:
      //  FIXME: How do we handle the nIndex member?
      pVideoPortFormat = (OMX_VIDEO_PARAM_PORTFORMATTYPE*)ComponentParameterStructure;
      portIndex = pVideoPortFormat->nPortIndex;
      err = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pVideoPortFormat, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
      if(err!=OMX_ErrorNone) { 
        DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,err); 
        break;
      }
      pPort = (omx_kms_sink_component_PortType *) omx_kms_sink_component_Private->ports[portIndex];
      if(portIndex != 0) {
        return OMX_ErrorBadPortIndex;
      }
      if (pVideoPortFormat->eCompressionFormat != OMX_VIDEO_CodingUnused)  {
        //  No compression allowed
        return OMX_ErrorUnsupportedSetting;
      }
#if 0
      if(pVideoPortFormat->xFramerate > 0) {
        nFrameProcessTime = 1000000 / pVideoPortFormat->xFramerate;
      }
      pPort->sVideoParam.xFramerate         = pVideoPortFormat->xFramerate;
#endif
      pPort->sVideoParam.eCompressionFormat = pVideoPortFormat->eCompressionFormat;
      pPort->sVideoParam.eColorFormat       = pVideoPortFormat->eColorFormat;
      pPort->sPortParam.format.video.eColorFormat = pVideoPortFormat->eColorFormat;
      //  Figure out stride, slice height, min buffer size
      pPort->sPortParam.format.video.nStride      = calcStride(pPort->sPortParam.format.video.nFrameWidth, pPort->sVideoParam.eColorFormat);
      pPort->sPortParam.format.video.nSliceHeight = pPort->sPortParam.format.video.nFrameHeight;  //  No support for slices yet
      pPort->sPortParam.nBufferSize               = (OMX_U32) abs(pPort->sPortParam.format.video.nStride) * pPort->sPortParam.format.video.nSliceHeight;
      break;
    
    default: /*Call the base component function*/
      return omx_base_component_SetParameter(hComponent, nParamIndex, ComponentParameterStructure);
  }
  return err;
}


OMX_ERRORTYPE omx_kms_sink_component_GetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_INOUT OMX_PTR ComponentParameterStructure) {

  OMX_VIDEO_PARAM_PORTFORMATTYPE *pVideoPortFormat; 
  OMX_ERRORTYPE err = OMX_ErrorNone;
  OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
  omx_kms_sink_component_PrivateType* omx_kms_sink_component_Private = openmaxStandComp->pComponentPrivate;
  omx_kms_sink_component_PortType *pPort = (omx_kms_sink_component_PortType *) omx_kms_sink_component_Private->ports[OMX_BASE_SINK_INPUTPORT_INDEX];  
  printf("%s %d\n",__FUNCTION__,__LINE__);

  if (ComponentParameterStructure == NULL) {
    return OMX_ErrorBadParameter;
  }
  DEBUG(DEB_LEV_SIMPLE_SEQ, "   Getting parameter %i\n", nParamIndex);
  /* Check which structure we are being fed and fill its header */
  switch(nParamIndex) {
    case OMX_IndexParamVideoInit:
      if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_PORT_PARAM_TYPE))) != OMX_ErrorNone) { 
        break;
      }
      memcpy(ComponentParameterStructure, &omx_kms_sink_component_Private->sPortTypesParam[OMX_PortDomainVideo], sizeof(OMX_PORT_PARAM_TYPE));
      break;    
    
    case OMX_IndexParamVideoPortFormat:
      pVideoPortFormat = (OMX_VIDEO_PARAM_PORTFORMATTYPE*)ComponentParameterStructure;
      if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE))) != OMX_ErrorNone) { 
        break;
      }
      if (pVideoPortFormat->nPortIndex < 1) {
        memcpy(pVideoPortFormat, &pPort->sVideoParam, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
      } else {
        return OMX_ErrorBadPortIndex;
      }
      break;  
      
    default: /*Call the base component function*/
      return omx_base_component_GetParameter(hComponent, nParamIndex, ComponentParameterStructure);
  }
  return err;
}


OMX_ERRORTYPE omx_kms_sink_component_MessageHandler(OMX_COMPONENTTYPE* openmaxStandComp,internalRequestMessageType *message) {

  omx_kms_sink_component_PrivateType* omx_kms_sink_component_Private = (omx_kms_sink_component_PrivateType*)openmaxStandComp->pComponentPrivate;
  OMX_ERRORTYPE err;
  OMX_STATETYPE eState;

  DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s\n", __func__);
  eState = omx_kms_sink_component_Private->state; //storing current state

  if (message->messageType == OMX_CommandStateSet){
    if ((message->messageParam == OMX_StateExecuting ) && (omx_kms_sink_component_Private->state == OMX_StateIdle)) {
      DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s sink component from loaded to idle \n", __func__);
      err = omx_kms_sink_component_Init(openmaxStandComp);
      if(err!=OMX_ErrorNone) { 
        DEBUG(DEB_LEV_ERR, "In %s Video Sink Init Failed Error=%x\n",__func__,err); 
        return err;
      }
    } 
  }
  // Execute the base message handling
  err = omx_base_component_MessageHandler(openmaxStandComp,message);

  if (message->messageType == OMX_CommandStateSet) {
    if ((message->messageParam == OMX_StateIdle ) && (omx_kms_sink_component_Private->state == OMX_StateIdle) && eState == OMX_StateExecuting) {
      err = omx_kms_sink_component_Deinit(openmaxStandComp);
      if(err!=OMX_ErrorNone) { 
        DEBUG(DEB_LEV_ERR, "In %s Video Sink Deinit Failed Error=%x\n",__func__,err); 
        return err;
      }
    }
  }
  return err;
}

/**  This function takes two inputs - 
  * @param width is the input picture width
  * @param omx_pxlfmt is the input openmax standard pixel format
  * It calculates the byte per pixel needed to display the picture with the input omx_pxlfmt
  * The output stride for display is thus omx_fbdev_sink_component_Private->product of input width and byte per pixel
  */
OMX_S32 calcStride(OMX_U32 width, OMX_COLOR_FORMATTYPE omx_pxlfmt) {
  OMX_U32 stride;
  OMX_U32 bpp; // bit per pixel

  switch(omx_pxlfmt) {
    case OMX_COLOR_FormatMonochrome:
      bpp = 1;
      break;
    case OMX_COLOR_FormatL2:
      bpp = 2;
    case OMX_COLOR_FormatL4:
      bpp = 4;
      break;
    case OMX_COLOR_FormatL8:
    case OMX_COLOR_Format8bitRGB332:
    case OMX_COLOR_FormatRawBayer8bit:
    case OMX_COLOR_FormatRawBayer8bitcompressed:
      bpp = 8;  
      break;
    case OMX_COLOR_FormatRawBayer10bit:
      bpp = 10;
      break;
    case OMX_COLOR_FormatYUV411Planar:
    case OMX_COLOR_FormatYUV411PackedPlanar:
    case OMX_COLOR_Format12bitRGB444:
    case OMX_COLOR_FormatYUV420Planar:
    case OMX_COLOR_FormatYUV420PackedPlanar:
    case OMX_COLOR_FormatYUV420SemiPlanar:
    case OMX_COLOR_FormatYUV444Interleaved:
      bpp = 12;
      break;
    case OMX_COLOR_FormatL16:
    case OMX_COLOR_Format16bitARGB4444:
    case OMX_COLOR_Format16bitARGB1555:
    case OMX_COLOR_Format16bitRGB565:
    case OMX_COLOR_Format16bitBGR565:
    case OMX_COLOR_FormatYUV422Planar:
    case OMX_COLOR_FormatYUV422PackedPlanar:
    case OMX_COLOR_FormatYUV422SemiPlanar:
    case OMX_COLOR_FormatYCbYCr:
    case OMX_COLOR_FormatYCrYCb:
    case OMX_COLOR_FormatCbYCrY:
    case OMX_COLOR_FormatCrYCbY:
      bpp = 16;
      break;
    case OMX_COLOR_Format18bitRGB666:
    case OMX_COLOR_Format18bitARGB1665:
      bpp = 18;
      break;
    case OMX_COLOR_Format19bitARGB1666:
      bpp = 19;
      break;
    case OMX_COLOR_FormatL24:
    case OMX_COLOR_Format24bitRGB888:
    case OMX_COLOR_Format24bitBGR888:
    case OMX_COLOR_Format24bitARGB1887:
      bpp = 24;
      break;
    case OMX_COLOR_Format25bitARGB1888:
      bpp = 25;
      break;
    case OMX_COLOR_FormatL32:
    case OMX_COLOR_Format32bitBGRA8888:
    case OMX_COLOR_Format32bitARGB8888:
      bpp = 32;
      break;
    default:
      bpp = 0;
      break;
  }
  stride = (width * bpp) >> 3; // division by 8 to get byte per pixel value
  return (OMX_S32) stride;
}
