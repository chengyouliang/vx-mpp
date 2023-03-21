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

typedef enum 
{
    NONE,
    I_Frame,
    P_Frame,
    B_Frame,
}FrameType;

typedef enum 
{
  MPEG4_DEC,
	DIVX311_DEC,
	DIVX502_DEC,	
	DIVX503_DEC,
  DIVX412_DEC,
  MPEG2_DEC,
	XVID_DEC,
  H263_DEC,
  H264_DEC,
} SSBSIP_MFC_CODEC_TYPE;

typedef struct
{
	OMX_U8  bGetWH; //是否獲取分辨率
  OMX_S32  width, height; //分辨率
	OMX_S8  cType; //幀類型（'I' 'P' 'B'）
} MPEG4_CONFIG_DATA;

typedef struct
{
  OMX_U32 offset;
  OMX_U32 size;
  OMX_U32 naltype;
}frameInfo;

#define STREAM_BUF_SIZE (4*1024)
typedef struct
{
  frameInfo *info;
  OMX_U32 frameNum;
  OMX_U32 lastFrameNum;
  FILE *fin;
  OMX_U8 aInputBuf[STREAM_BUF_SIZE];
}CtxType;


#define INFO_BUFF_SIZE 20000



static OMX_U32 g_frame_num = 0;

OMX_U32 h264_parse(OMX_U32 buffer_start_addr, OMX_U32 buffer_size, OMX_U8 *src_mem,CtxType* pctx)
{
	OMX_U32 i ;
  OMX_U8 *pbase,*pnext,*ptmp;
  int len;
  pbase = src_mem;
  ptmp = src_mem;
  i = 0;
	while(i < buffer_size)
	{
		ptmp = src_mem + i;
    printf("%s %d %d\n",__FUNCTION__,__LINE__,i);
		if (((*(ptmp)==0x00) && (*(ptmp+1)==0x00) && (*(ptmp+2)==0x00) && (*(ptmp+3)==0x01)) ||  //NAL前的起始码00000001||000001
			((*(ptmp)==0x00) && (*(ptmp+1)==0x00) && (*(ptmp+2)==0x01)))
		{
      
      if((*(ptmp)==0x00) && (*(ptmp+1)==0x00) && (*(ptmp+2)==0x01))
      {
          len = 3;
      }
      else
      {
          len = 4;
      }
      printf("%s %d %x\n",__FUNCTION__,__LINE__,*(ptmp + len +1));
      i += len;
      pnext = ptmp;
      if (pnext != pbase)
      {
         if ( pctx->frameNum == 0)
         {
              pctx->info[pctx->frameNum].offset  = 0;
         }
         else
         {
            pctx->info[pctx->frameNum].offset += pctx->info[pctx->frameNum -1].offset + pctx->info[pctx->frameNum  -1].size;
         }
         pctx->info[pctx->frameNum].size = pnext - pbase;
         pctx->info[pctx->frameNum].naltype = *(pbase + len +1);
         printf("%x \n",pctx->info[pctx->frameNum].naltype);
         pctx->frameNum ++;
         pbase = pnext;
      }
		}
		else
		{
			i++ ;
		}
	}
  return 1;
}
// 返回值意义：
// 0: one frame size is successfully determined AND there are still more frames
// 1,2: the size of the last frame is successfully determined. 1，2表示不够一帧数据，需要读取更多，或者是到了文件尾部
// 3: error (invalid standard)
 
OMX_U32 VsParser(SSBSIP_MFC_CODEC_TYPE standard, OMX_U32 buffer_start_addr, OMX_U32 buffer_size, /*u8* src_mem,*/
	CtxType * pctx)
{
	OMX_U32 ret_value;
	
	OMX_U8 *src_mem;
	
	src_mem = (OMX_U8 *)buffer_start_addr;
	
	switch(standard)
	{
		case MPEG4_DEC :
		case DIVX311_DEC :	
		case DIVX412_DEC :
		case DIVX502_DEC :	
		case DIVX503_DEC :
		case XVID_DEC:
		case H263_DEC :
#if 0
			ret_value = mpeg4_parse( buffer_start_addr, buffer_size, src_mem,
											  frame_start_addr,  frame_size, is_first_seq_header, conf_data);
			if(ret_value==1||ret_value==2)	*frame_size=buffer_size;			
#endif
			break ;
		case MPEG2_DEC :
#if 0
			ret_value = mpeg2_parse(buffer_start_addr, buffer_size, src_mem,
									frame_start_addr,  frame_size, is_first_seq_header, conf_data);
			if(ret_value==1||ret_value==2)	*frame_size=buffer_size;
#endif
			break ;
		case H264_DEC :
			ret_value = h264_parse(buffer_start_addr, buffer_size, src_mem,
								   pctx);
			break ;
 
		default :
			ret_value = 3 ;
			break ;
	}
	return ret_value ;
}

void *InitRawFrameExtract(char* pFilename)
{
	SSBSIP_MFC_CODEC_TYPE eMode;
	char* pFileExt;
	OMX_U32 uFrameInfo[2];
	OMX_U32 uRemainSz,fileoffset;
  frameInfo *ptmpinfo;

	CtxType *pCtx;
  eMode = H264_DEC;
	pCtx = (CtxType *)malloc(sizeof(CtxType));
	pCtx->lastFrameNum = -1;
	pCtx->fin = fopen(pFilename,"rb");
	if ( pCtx->fin==NULL )
		return NULL;
 
	pCtx->info = (frameInfo*)malloc(INFO_BUFF_SIZE);
	pCtx->frameNum = 0;
  fileoffset = 0;
  while(1)
  {  
      fseek(pCtx->fin, fileoffset, SEEK_SET);
	    uRemainSz = fread(pCtx->aInputBuf,1,STREAM_BUF_SIZE,pCtx->fin);
      if (uRemainSz <= 0)
      {
         break;
      }
      VsParser(eMode, (OMX_U32)pCtx->aInputBuf, uRemainSz, pCtx); //第一次调用VsParser 第6个参数：parseHeader = true，表示获取视频头信息（H264：SEI+SPS+PPS）
      ptmpinfo =  (pCtx->info + pCtx->frameNum -1);
      fileoffset = ptmpinfo->offset +  ptmpinfo->size;
  }
  fseek(pCtx->fin, 0, SEEK_SET);
	return (void *)pCtx;	
}

void delRawFrameExtract(CtxType *pCtx)
{
    if (!pCtx)
    {
        return;
    }
    if(pCtx->info)
    {
       free(pCtx->info);
    }
    fclose(pCtx->fin);
    free(pCtx);
}



int GetNextFrame(CtxType *pCtx,unsigned char *pFrameBuf, unsigned int *pSize)
{
	int isLastFrame;
  OMX_U32 uFileOffset;
  if (!pCtx || !pFrameBuf  || !pSize)
  {
      return -1;
  }
  if (g_frame_num > pCtx->lastFrameNum)
  {
     return 0;
  }
  *pSize = pCtx->info[g_frame_num].size;
  uFileOffset = pCtx->info[g_frame_num].offset;
  fseek(pCtx->fin, uFileOffset, SEEK_SET);
  fread(pFrameBuf,1,*pSize,pCtx->fin);
	return *pSize;
}

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

#define BUFFER_IN_SIZE (4*1024)
#define BUFFER_OUT_SIZE (4*1024*1024)

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

  data_read = fread(pBuffer->pBuffer, 1, BUFFER_IN_SIZE, appPriv->fd);
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
  MPEG4_CONFIG_DATA config_data;
  CtxType *pctx = (CtxType *)InitRawFrameExtract("test.h264");
  if (pctx == NULL)
  {
      DEBUG(DEB_LEV_ERR, "Error in InitRawFrameExtract \n");
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
  err = OMX_AllocateBuffer(appPriv->videodechandle, &pInBuffer[0], 0, NULL, BUFFER_IN_SIZE);
  err = OMX_AllocateBuffer(appPriv->videodechandle, &pInBuffer[1], 0, NULL, BUFFER_IN_SIZE);
  if(err != OMX_ErrorNone){
    printf("OMX_CommandStateSet error...\n");
    exit(1);
  }
  pOutBuffer[0] = pOutBuffer[1] = NULL;
  err = OMX_AllocateBuffer(appPriv->videodechandle, &pOutBuffer[0], 1, NULL, BUFFER_OUT_SIZE);
  err = OMX_AllocateBuffer(appPriv->videodechandle, &pOutBuffer[1], 1, NULL, BUFFER_OUT_SIZE);
  if(err != OMX_ErrorNone){
    printf("OMX_CommandStateSet error...\n");
    exit(1);
  }

  /** sending command to video decoder component to go to executing state */
  err = OMX_SendCommand(appPriv->videodechandle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
  tsem_down(appPriv->decoderEventSem);

  err = OMX_SendCommand(appPriv->videodechandle, OMX_CommandPortEnable, 1, NULL);
  if(err != OMX_ErrorNone) {
    DEBUG(DEB_LEV_ERR,"video decoder port enable failed\n");
    exit(1);
  }
  tsem_down(appPriv->decoderEventSem);
  err = OMX_SendCommand(appPriv->videodechandle, OMX_CommandPortEnable, 0, NULL);
  if(err != OMX_ErrorNone) {
    DEBUG(DEB_LEV_ERR,"video decoder port enable failed\n");
    exit(1);
  }
  tsem_down(appPriv->decoderEventSem);
  err = OMX_FillThisBuffer(appPriv->videodechandle, pOutBuffer[0]);
  err = OMX_FillThisBuffer(appPriv->videodechandle, pOutBuffer[1]);
  int data_read;
  data_read = fread(pInBuffer[0]->pBuffer, 1, BUFFER_IN_SIZE, appPriv->fd);
  pInBuffer[0]->nFilledLen = data_read;
  pInBuffer[0]->nOffset = 0;
  /** in non tunneled case use the 2nd input buffer for input read and procesing
    * in tunneled case, it will be used afterwards
    */
  data_read = fread(pInBuffer[1]->pBuffer, 1, BUFFER_IN_SIZE, appPriv->fd);
  pInBuffer[1]->nFilledLen = data_read;
  pInBuffer[1]->nOffset = 0;
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

