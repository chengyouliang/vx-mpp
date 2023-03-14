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


#define COMPONENT_NAME_BASE "OMX.st.video_decoder"
#define BASE_ROLE "video_decoder.avc"
#define COMPONENT_NAME_BASE_LEN 20



/** Callbacks implementation of the video decoder component*/
OMX_ERRORTYPE videodecEventHandler(
  OMX_OUT OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_PTR pAppData,
  OMX_OUT OMX_EVENTTYPE eEvent,
  OMX_OUT OMX_U32 Data1,
  OMX_OUT OMX_U32 Data2,
  OMX_OUT OMX_PTR pEventData) {

#if 0
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
    DEBUG(DEB_LEV_SIMPLE_SEQ, "\n port settings change event handler in %s \n", __func__);
    if(Data2 == 0) {

      /** before setting port parameters , first check if tunneled case 
        * if so, then disable the output port of video decoder
        * and generate dummy up signal, so that in main thread
        * the color conv and sink comp ports are disabled and 
        * port parameter settings changes are done properly
        */
      if(flagSetupTunnel) {
        DEBUG(DEB_LEV_SIMPLE_SEQ,"Sending Port Disable Command\n");
        err = OMX_SendCommand(appPriv->videodechandle, OMX_CommandPortDisable, 1, NULL);
        if(err != OMX_ErrorNone) {
          DEBUG(DEB_LEV_ERR,"video decoder port disable failed\n");
          exit(1);
        }

        /** disable the ports */
        if(flagIsColorConvRequested) {
          err = OMX_SendCommand(appPriv->colorconv_handle, OMX_CommandPortDisable, OMX_ALL, NULL);
          if(err != OMX_ErrorNone) {
            DEBUG(DEB_LEV_ERR,"color conv all port disable failed\n");
            exit(1);
          }
        } 
        
        if(flagIsSinkRequested) {
          err = OMX_SendCommand(appPriv->video_sink_handle, OMX_CommandPortDisable, 0, NULL);
          if(err != OMX_ErrorNone) {
            DEBUG(DEB_LEV_ERR,"video sink input port disable failed\n");
            exit(1);
          }
        }
        // dummy up signal - caught in main thread to resume processing from there
        tsem_up(appPriv->decoderEventSem);
      }

      /** in non tunneled case, if color converter and sink component are selected 
        * then set their port parameters according to input tream characteristics and 
        * send command to them to go to idle state and executing state
        */
      if(!flagSetupTunnel) {
        if(flagIsColorConvRequested) {
          setPortParameters();
          pOutBufferColorConv[0] = pOutBufferColorConv[1] = NULL;
          err = OMX_SendCommand(appPriv->colorconv_handle, OMX_CommandStateSet, OMX_StateIdle, NULL);

          /** in non tunneled case, using buffers in color conv input port, allocated by video dec component output port */
          err = OMX_UseBuffer(appPriv->colorconv_handle, &pInBufferColorConv[0], 0, NULL, buffer_out_size, pOutBuffer[0]->pBuffer);
          if(err != OMX_ErrorNone) {
            DEBUG(DEB_LEV_ERR, "Unable to use the video dec comp allocate buffer\n");
            exit(1);
          }
          err = OMX_UseBuffer(appPriv->colorconv_handle, &pInBufferColorConv[1], 0, NULL, buffer_out_size, pOutBuffer[1]->pBuffer);
          if(err != OMX_ErrorNone) {
            DEBUG(DEB_LEV_ERR, "Unable to use the video dec comp allocate buffer\n");
            exit(1);
          }

          /** allocating buffers in the color converter compoennt output port */
          omx_colorconvPortDefinition.nPortIndex = 1;
          setHeader(&omx_colorconvPortDefinition, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
          err = OMX_GetParameter(appPriv->colorconv_handle, OMX_IndexParamPortDefinition, &omx_colorconvPortDefinition);
          outbuf_colorconv_size = omx_colorconvPortDefinition.nBufferSize;
          DEBUG(DEB_LEV_SIMPLE_SEQ, " outbuf_colorconv_size : %d \n", (int)outbuf_colorconv_size);

          err = OMX_AllocateBuffer(appPriv->colorconv_handle, &pOutBufferColorConv[0], 1, NULL, outbuf_colorconv_size);
          if(err != OMX_ErrorNone) {
            DEBUG(DEB_LEV_ERR, "Unable to allocate buffer in color conv\n");
            exit(1);
          }
          err = OMX_AllocateBuffer(appPriv->colorconv_handle, &pOutBufferColorConv[1], 1, NULL, outbuf_colorconv_size);
          if(err != OMX_ErrorNone) {
            DEBUG(DEB_LEV_ERR, "Unable to allocate buffer in colro conv\n");
            exit(1);
          }

          DEBUG(DEB_LEV_FULL_SEQ, "Before locking on idle wait semaphore (line %d)\n",__LINE__);
          tsem_down(appPriv->colorconvEventSem);
        } else {
          setPortParameters_xvideo();
        }
        if(flagIsSinkRequested) {
          err = OMX_SendCommand(appPriv->video_sink_handle, OMX_CommandStateSet, OMX_StateIdle, NULL);

          DEBUG(DEB_ALL_MESS, "Sent idle command\n");

          if(flagIsColorConvRequested) {
            err = OMX_UseBuffer(appPriv->video_sink_handle, &pInBufferSink[0], 0, NULL, outbuf_colorconv_size, pOutBufferColorConv[0]->pBuffer);
            if(err != OMX_ErrorNone) {
              DEBUG(DEB_LEV_ERR, "Unable to use the color conv comp allocate buffer\n");
              exit(1);
            }
            err = OMX_UseBuffer(appPriv->video_sink_handle, &pInBufferSink[1], 0, NULL, outbuf_colorconv_size, pOutBufferColorConv[1]->pBuffer);
            if(err != OMX_ErrorNone) {
              DEBUG(DEB_LEV_ERR, "Unable to use the color conv comp allocate buffer\n");
              exit(1);
            }
          } else {
            err = OMX_UseBuffer(appPriv->video_sink_handle, &pInBufferSink[0], 0, NULL, buffer_out_size, pOutBuffer[0]->pBuffer);
            if(err != OMX_ErrorNone) {
              DEBUG(DEB_LEV_ERR, "Unable to use the color conv comp allocate buffer\n");
              exit(1);
            }
            DEBUG(DEB_LEV_FULL_SEQ, "In %s pInBufferSink[0]->pBuffer=%x pBuffer=%x\n", __func__,(int)pInBufferSink[0]->pBuffer,(int)pOutBuffer[0]->pBuffer);
            err = OMX_UseBuffer(appPriv->video_sink_handle, &pInBufferSink[1], 0, NULL, buffer_out_size, pOutBuffer[1]->pBuffer);
            if(err != OMX_ErrorNone) {
              DEBUG(DEB_LEV_ERR, "Unable to use the color conv comp allocate buffer\n");
              exit(1);
            }
            DEBUG(DEB_LEV_FULL_SEQ, "In %s pInBufferSink[1]->pBuffer=%x pBuffer=%x\n", __func__,(int)pInBufferSink[1]->pBuffer,(int)pOutBuffer[1]->pBuffer);
          }

          DEBUG(DEB_LEV_FULL_SEQ, "Before locking on idle wait semaphore (line %d)\n",__LINE__);
          tsem_down(appPriv->videoSinkEventSem);
        }

        if(flagIsColorConvRequested) {
          err = OMX_SendCommand(appPriv->colorconv_handle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
          tsem_down(appPriv->colorconvEventSem);
        }
        if(flagIsSinkRequested) {    
          err = OMX_SendCommand(appPriv->video_sink_handle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
          tsem_down(appPriv->videoSinkEventSem);
        }

        if(flagIsColorConvRequested) { 
          err = OMX_FillThisBuffer(appPriv->colorconv_handle, pOutBufferColorConv[0]);
          err = OMX_FillThisBuffer(appPriv->colorconv_handle, pOutBufferColorConv[1]);
          DEBUG(DEB_LEV_SIMPLE_SEQ, "---> After fill this buffer function calls to the color conv output buffers\n");
        }
      }
    }
  } else if(eEvent == OMX_EventBufferFlag) {
    DEBUG(DEB_LEV_ERR, "In %s OMX_BUFFERFLAG_EOS\n", __func__);
    if((int)Data2 == OMX_BUFFERFLAG_EOS) {
      tsem_up(appPriv->eofSem);
    }
  } else {
    DEBUG(DEB_LEV_SIMPLE_SEQ, "Param1 is %i\n", (int)Data1);
    DEBUG(DEB_LEV_SIMPLE_SEQ, "Param2 is %i\n", (int)Data2);
  }
#endif
  return 0; 
}

OMX_ERRORTYPE videodecEmptyBufferDone(
  OMX_OUT OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_PTR pAppData,
  OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer) {
#if 0
  OMX_ERRORTYPE err;
  int data_read;
  static int iBufferDropped=0;

  DEBUG(DEB_LEV_FULL_SEQ, "Hi there, I am in the %s callback.\n", __func__);

  data_read = fread(pBuffer->pBuffer, 1, buffer_in_size, fd);
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
    DEBUG(DEB_LEV_FULL_SEQ, "Empty buffer %x\n", (int)pBuffer);
    err = OMX_EmptyThisBuffer(hComponent, pBuffer);
  } else {
    DEBUG(DEB_LEV_FULL_SEQ, "In %s Dropping Empty This buffer to Audio Dec\n", __func__);
  }
#endif
  return OMX_ErrorNone;
}


OMX_ERRORTYPE videodecFillBufferDone(
  OMX_OUT OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_PTR pAppData,
  OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer) {

#if 0
  OMX_ERRORTYPE err;
  OMX_STATETYPE eState;

  if(pBuffer != NULL) {
    if(!bEOS) {
      /** if there is color conv component in processing state then send this buffer, in non tunneled case 
        * else in non tunneled case, write the output buffer contents in the specified output file
        */
      if(flagIsColorConvRequested && (!flagSetupTunnel)) {
        OMX_GetState(appPriv->colorconv_handle,&eState);
        if(eState == OMX_StateExecuting || eState == OMX_StatePause) {
          if(pInBufferColorConv[0]->pBuffer == pBuffer->pBuffer) {
            pInBufferColorConv[0]->nFilledLen = pBuffer->nFilledLen;
            err = OMX_EmptyThisBuffer(appPriv->colorconv_handle, pInBufferColorConv[0]);
          } else {
            pInBufferColorConv[1]->nFilledLen = pBuffer->nFilledLen;
            err = OMX_EmptyThisBuffer(appPriv->colorconv_handle, pInBufferColorConv[1]);
          }
          if(err != OMX_ErrorNone) {
            DEBUG(DEB_LEV_ERR, "In %s Error %08x Calling FillThisBuffer\n", __func__,err);
          }
        } else {
          err = OMX_FillThisBuffer(hComponent, pBuffer);
        }
      } else if(flagIsSinkRequested) {
        //DEBUG(DEB_LEV_ERR, "In %s Calling FillThisBuffer Size=%d\n", __func__,err,pBuffer->nFilledLen);
        //DEBUG(DEB_LEV_ERR, "In %s pBuffer[0]=%x pBuffer[1]=%x pBuffer=%x Len=%d\n", __func__,pInBufferSink[0]->pBuffer,pInBufferSink[1]->pBuffer,pBuffer->pBuffer,pBuffer->nFilledLen);
        if(pInBufferSink[0]->pBuffer == pBuffer->pBuffer) {
          pInBufferSink[0]->nFilledLen = pBuffer->nFilledLen;
          //DEBUG(DEB_LEV_ERR, "In %s Calling OMX_EmptyThisBuffer 0 Size=%d\n", __func__,pBuffer->nFilledLen);
          err = OMX_EmptyThisBuffer(appPriv->video_sink_handle, pInBufferSink[0]);
        } else {
          pInBufferSink[1]->nFilledLen = pBuffer->nFilledLen;
          //DEBUG(DEB_LEV_ERR, "In %s Calling OMX_EmptyThisBuffer 1 Size=%d\n", __func__,pBuffer->nFilledLen);
          err = OMX_EmptyThisBuffer(appPriv->video_sink_handle, pInBufferSink[1]);
        }
        if(err != OMX_ErrorNone) {
          DEBUG(DEB_LEV_ERR, "In %s Error %08x Calling FillThisBuffer\n", __func__,err);
        }
      } else if((pBuffer->nFilledLen > 0) && (!flagSetupTunnel)) {
          fwrite(pBuffer->pBuffer, 1,  pBuffer->nFilledLen, outfile);    
          pBuffer->nFilledLen = 0;
      } else {
          DEBUG(DEB_LEV_ERR, "In %s Empty buffer in Else\n", __func__);
      }
      if(pBuffer->nFlags == OMX_BUFFERFLAG_EOS) {
        DEBUG(DEB_LEV_ERR, "In %s: eos=%x Calling Empty This Buffer\n", __func__, (int)pBuffer->nFlags);
        bEOS = OMX_TRUE;
      }
      if(!bEOS && !flagIsColorConvRequested && !flagIsSinkRequested && !flagSetupTunnel) {
        err = OMX_FillThisBuffer(hComponent, pBuffer);
      }
    } else {
      DEBUG(DEB_LEV_ERR, "In %s: eos=%x Dropping Empty This Buffer\n", __func__,(int)pBuffer->nFlags);
    }
  } else {
    DEBUG(DEB_LEV_ERR, "Ouch! In %s: had NULL buffer to output...\n", __func__);
  }
#endif;
  return OMX_ErrorNone;  
}





OMX_CALLBACKTYPE videodeccallbacks = { 
    .EventHandler = videodecEventHandler,
    .EmptyBufferDone = videodecEmptyBufferDone,
    .FillBufferDone = videodecFillBufferDone
  };

int main(int argc, char** argv) {

  int err;
  printf("%s %d\n",__FUNCTION__,__LINE__);
  char *full_component_name;
  err = OMX_Init();
  printf("%s %d\n",__FUNCTION__,__LINE__);
  OMX_HANDLETYPE videodechandle;
  if (err != OMX_ErrorNone) {
    printf("The OpenMAX core can not be initialized. Exiting...\n");
    exit(1);
  } else {
    printf("Omx core is initialized \n");
  }

  full_component_name = malloc(sizeof(char*) * OMX_MAX_STRINGNAME_SIZE);
  strcpy(full_component_name, COMPONENT_NAME_BASE);

  printf("The component selected for decoding is %s\n", full_component_name);

  /** getting video decoder handle */
  err = OMX_GetHandle(&videodechandle, full_component_name, NULL, &videodeccallbacks);
  if(err != OMX_ErrorNone){
    printf("No video decoder component found. Exiting...\n");
    exit(1);
  } else {
    printf("Found The component for decoding is %s\n", full_component_name);
  }
  return 0;
}

