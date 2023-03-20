/**
  @file test/components/video/omxvideodectest.c
  
  Test application that uses a OpenMAX component, a generic video decoder. 
  The application receives an video stream (.m4v or .264) decoded by a multiple format decoder component.
  The decoded output is seen by a yuv viewer.
  
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


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

#include <OMX_Core.h>
#include <OMX_Component.h>
#include <OMX_Types.h>
#include <OMX_Video.h>
#include <tsemaphore.h>
#include <tsemaphore.h>
#include <omx_comp_debug_levels.h>


#define COMPONENT_NAME_BASE "OMX.h26x.video_encoder"
#define COMPONENT_NAME_BASE_LEN 20


/* Application's private data */
typedef struct appPrivateType{
  tsem_t* decoderEventSem;
  tsem_t* eofSem;
  FILE *fd;
  FILE *outfile;
  OMX_HANDLETYPE videodechandle;
}appPrivateType;

appPrivateType* appPriv;

#define BUFFER_IN_SIZE 2*8192
int buffer_in_size = (BUFFER_IN_SIZE*2*1024); 

static OMX_BOOL bEOS = OMX_FALSE;

/** Callbacks implementation of the video decoder component*/
OMX_ERRORTYPE videodecEventHandler(
  OMX_OUT OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_PTR pAppData,
  OMX_OUT OMX_EVENTTYPE eEvent,
  OMX_OUT OMX_U32 Data1,
  OMX_OUT OMX_U32 Data2,
  OMX_OUT OMX_PTR pEventData) {
  OMX_ERRORTYPE err = OMX_ErrorNone;

  DEBUG(DEB_LEV_SIMPLE_SEQ, "Hi there, I am in the %s callback\n", __func__);
  if(eEvent == OMX_EventCmdComplete) {
    if (Data1 == OMX_CommandStateSet) {
      DEBUG(DEB_LEV_SIMPLE_SEQ, "State changed in ");
      switch ((int)Data2) {
        case OMX_StateInvalid:
          DEBUG(DEB_LEV_SIMPLE_SEQ, "OMX_StateInvalid\n");
          break;
        case OMX_StateLoaded:
          DEBUG(DEB_LEV_SIMPLE_SEQ, "OMX_StateLoaded\n");
          break;
        case OMX_StateIdle:
          DEBUG(DEB_LEV_SIMPLE_SEQ, "OMX_StateIdle\n");
          break;
        case OMX_StateExecuting:
          DEBUG(DEB_LEV_SIMPLE_SEQ, "OMX_StateExecuting\n");
          break;
        case OMX_StatePause:
          DEBUG(DEB_LEV_SIMPLE_SEQ, "OMX_StatePause\n");
          break;
        case OMX_StateWaitForResources:
          DEBUG(DEB_LEV_SIMPLE_SEQ, "OMX_StateWaitForResources\n");
          break;
      }    
      tsem_up(appPriv->decoderEventSem);
    } else if (OMX_CommandPortEnable || OMX_CommandPortDisable) {
      DEBUG(DEB_LEV_SIMPLE_SEQ, "In %s Received Port Enable/Disable Event\n",__func__);
      tsem_up(appPriv->decoderEventSem);
    } 
  } else if(eEvent == OMX_EventPortSettingsChanged) {
     tsem_up(appPriv->decoderEventSem);
  }
  return err; 
}

OMX_ERRORTYPE videodecEmptyBufferDone(
  OMX_OUT OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_PTR pAppData,
  OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer) {
  OMX_ERRORTYPE err = OMX_ErrorNone;
  int data_read;
  static int iBufferDropped=0;

  DEBUG(DEB_LEV_FULL_SEQ, "Hi there, I am in the %s callback.\n", __func__);

  data_read = fread(pBuffer->pBuffer, 1, buffer_in_size, appPriv->fd);
  pBuffer->nFilledLen = data_read;
  pBuffer->nOffset = 0;
  if (data_read <= 0) {
    DEBUG(DEB_LEV_SIMPLE_SEQ, "In the %s no more input data available\n", __func__);
    iBufferDropped++;
    if(iBufferDropped>=2) {
      bEOS=OMX_TRUE;
      return OMX_ErrorNone;
    }
    pBuffer->nFilledLen=0;
    pBuffer->nFlags = OMX_BUFFERFLAG_EOS;
    err = OMX_EmptyThisBuffer(hComponent, pBuffer);
    return OMX_ErrorNone;
  }
  pBuffer->nFilledLen = data_read;
  if(!bEOS) {
    DEBUG(DEB_LEV_FULL_SEQ, "Empty buffer %p\n", pBuffer);
    err = OMX_EmptyThisBuffer(hComponent, pBuffer);
  } else {
    DEBUG(DEB_LEV_FULL_SEQ, "In %s Dropping Empty This buffer to Audio Dec\n", __func__);
  }
  return err;
}


OMX_ERRORTYPE videodecFillBufferDone(
  OMX_OUT OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_PTR pAppData,
  OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer) {
  OMX_ERRORTYPE err = OMX_ErrorNone;

  fwrite(pBuffer->pBuffer, 1,  pBuffer->nFilledLen, appPriv->outfile);    
  pBuffer->nFilledLen = 0;
  if(pBuffer->nFlags == OMX_BUFFERFLAG_EOS) {
      DEBUG(DEB_LEV_ERR, "In %s: eos=%x Calling Empty This Buffer\n", __func__, (int)pBuffer->nFlags);
      bEOS = OMX_TRUE;
  }
  if(!bEOS) {
    err = OMX_FillThisBuffer(hComponent, pBuffer);
  }
  return err;  
}


OMX_CALLBACKTYPE videodeccallbacks = { 
    .EventHandler = videodecEventHandler,
    .EmptyBufferDone = videodecEmptyBufferDone,
    .FillBufferDone = videodecFillBufferDone
  };

int main(int argc, char** argv) {
  int err;
  char *full_component_name;
  /** used with video decoder */
  OMX_BUFFERHEADERTYPE *pInBuffer[2], *pOutBuffer[2];

  err = OMX_Init();
  if (err != OMX_ErrorNone) {
    printf("The OpenMAX core can not be initialized. Exiting...\n");
    exit(1);
  } else {
    printf("Omx core is initialized \n");
  }
    /* Initialize application private data */
  appPriv = malloc(sizeof(appPrivateType));  
  appPriv->decoderEventSem = malloc(sizeof(tsem_t));
  tsem_init(appPriv->decoderEventSem, 0);

  appPriv->fd = fopen("test.h264", "rb");
  if(appPriv->fd == NULL) {
    DEBUG(DEB_LEV_ERR, "Error in opening input file \n");
    exit(1);
  }
  appPriv->outfile = fopen("test.yuv", "wb");
  if(appPriv->outfile == NULL) {
      DEBUG(DEB_LEV_ERR, "Error in opening output file \n");
      exit(1);
  }
  full_component_name = malloc(sizeof(char*) * OMX_MAX_STRINGNAME_SIZE);
  strcpy(full_component_name, COMPONENT_NAME_BASE);

  printf("The component selected for decoding is %s\n", full_component_name);

  /** getting video decoder handle */
  err = OMX_GetHandle(&appPriv->videodechandle, full_component_name, NULL, &videodeccallbacks);
  if(err != OMX_ErrorNone){
    printf("No video decoder component found. Exiting...\n");
    exit(1);
  } else {
    printf("Found The component for decoding is %s\n", full_component_name);
  }
   /** sending command to video decoder component to go to idle state */
  pInBuffer[0] = pInBuffer[1] = NULL;
  err = OMX_SendCommand(appPriv->videodechandle, OMX_CommandStateSet, OMX_StateIdle, NULL);
  if(err != OMX_ErrorNone){
    printf("OMX_CommandStateSet error...\n");
    exit(1);
  }
  tsem_down(appPriv->decoderEventSem);
  err = OMX_AllocateBuffer(appPriv->videodechandle, &pInBuffer[0], 0, NULL, buffer_in_size);
  err = OMX_AllocateBuffer(appPriv->videodechandle, &pInBuffer[1], 0, NULL, buffer_in_size);
  if(err != OMX_ErrorNone){
    printf("OMX_CommandStateSet error...\n");
    exit(1);
  }
  pOutBuffer[0] = pOutBuffer[1] = NULL;
  err = OMX_AllocateBuffer(appPriv->videodechandle, &pOutBuffer[0], 1, NULL, buffer_in_size);
  err = OMX_AllocateBuffer(appPriv->videodechandle, &pOutBuffer[1], 1, NULL, buffer_in_size);
  if(err != OMX_ErrorNone){
    printf("OMX_CommandStateSet error...\n");
    exit(1);
  }
  printf("%s %d\n",__FUNCTION__,__LINE__);
  /** sending command to video decoder component to go to executing state */
  err = OMX_SendCommand(appPriv->videodechandle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
  tsem_down(appPriv->decoderEventSem);

  printf("%s %d\n",__FUNCTION__,__LINE__);
  err = OMX_SendCommand(appPriv->videodechandle, OMX_CommandPortEnable, 1, NULL);
  if(err != OMX_ErrorNone) {
    DEBUG(DEB_LEV_ERR,"video decoder port enable failed\n");
    exit(1);
  }
  tsem_down(appPriv->decoderEventSem);
  printf("%s %d\n",__FUNCTION__,__LINE__);
  err = OMX_SendCommand(appPriv->videodechandle, OMX_CommandPortEnable, 0, NULL);
  if(err != OMX_ErrorNone) {
    DEBUG(DEB_LEV_ERR,"video decoder port enable failed\n");
    exit(1);
  }
  tsem_down(appPriv->decoderEventSem);
  printf("%s %d\n",__FUNCTION__,__LINE__);
  err = OMX_FillThisBuffer(appPriv->videodechandle, pOutBuffer[0]);
  err = OMX_FillThisBuffer(appPriv->videodechandle, pOutBuffer[1]);
  printf("%s %d\n",__FUNCTION__,__LINE__);
  int data_read;
  data_read = fread(pInBuffer[0]->pBuffer, 1, buffer_in_size, appPriv->fd);
  pInBuffer[0]->nFilledLen = data_read;
  pInBuffer[0]->nOffset = 0;
  printf("%s %d\n",__FUNCTION__,__LINE__);
  /** in non tunneled case use the 2nd input buffer for input read and procesing
    * in tunneled case, it will be used afterwards
    */
  data_read = fread(pInBuffer[1]->pBuffer, 1, buffer_in_size, appPriv->fd);
  pInBuffer[1]->nFilledLen = data_read;
  pInBuffer[1]->nOffset = 0;
  printf("%s %d\n",__FUNCTION__,__LINE__);
  DEBUG(DEB_LEV_PARAMS, "Empty first  buffer %p\n", pInBuffer[0]->pBuffer);
  err = OMX_EmptyThisBuffer(appPriv->videodechandle, pInBuffer[0]);
  DEBUG(DEB_LEV_PARAMS, "Empty second buffer %p\n", pInBuffer[1]->pBuffer);
  err = OMX_EmptyThisBuffer(appPriv->videodechandle, pInBuffer[1]);

  while(1) {
    if('Q' == toupper(getchar())) {
      bEOS = OMX_TRUE;
      break;
    }
  }
  return 0;
}

