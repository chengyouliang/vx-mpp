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
//读Buffer，直到找到完整的一帧数据才返回
//返回值意义：
// 0-- 已经找到完整的一帧
// 1-- 找到一个帧的开头标志，但是没有确定帧的大小（说明输入的Buffer里不够一帧数据）
// 2-- 没有找到帧的开头标志（可能输入的Buffer数据太少，或者数据非法）
//
OMX_U32 ue_v(unsigned char *nal)
{
  if (!nal)
  {
    return -1;
  }
  return (*nal & 31);
}

OMX_U32 h264_parse(OMX_U32 buffer_start_addr, OMX_U32 buffer_size, OMX_U8 *src_mem, OMX_U32 *frame_start_addr,  OMX_U32 *frame_size, OMX_U8 is_first_seq_header, MPEG4_CONFIG_DATA *conf_data)
{
	OMX_U32 i ;
	OMX_U32 num_start_code ;
	OMX_U32 hd_start_code ;
	int rbsp_base;
 
	OMX_U8  tmp_0, tmp_1 ;
	OMX_U8  *tmp_mem ;
 
	i              = 0 ;
	num_start_code = 0 ;//帧头的计数（帧头的起始码为：0x000001或0x00000001, 注意一帧并不等同于一个NAL，这里的帧是指I/B/P或SEI/SPS/PPS帧，而NAL是帧的组成部分），
	hd_start_code  = 0 ; //SEI/SPS/PPS帧的计数器
	//找到一帧的条件是num_start_code = 2，即找到两个帧头，第一个帧头到第二个帧头前的数据为一帧。当第一个帧头后跟I/B/P时，num_start_code=1；当第二个帧头后跟I/B/P或SEI/SPS/PPS帧，则num_start_code = 2
	//如果第一个帧头后跟SEI/SPS/PPS，num_start_code并不增加，但hd_start_code会增加1，并且记录当前的地址为起始地址；直到遇到下一个帧头为I/B/P帧时，num_start_code才加1
 
	//u8* src_mem = (u8*)de_emulation_prevention(srcmem); 
	OMX_U32 nal_count = 0;
	while(1)
	{
		tmp_mem = src_mem + i;
 
		if (((*(tmp_mem)==0x00) && (*(tmp_mem+1)==0x00) && (*(tmp_mem+2)==0x00) && (*(tmp_mem+3)==0x01)) ||  //NAL前的起始码00000001||000001
			((*(tmp_mem)==0x00) && (*(tmp_mem+1)==0x00) && (*(tmp_mem+2)==0x01)))
 
		{
			rbsp_base = buffer_start_addr + i;
      
			if((*(tmp_mem)==0x00) && (*(tmp_mem+1)==0x00) && (*(tmp_mem+2)==0x00) && (*(tmp_mem+3)==0x01))
			{
				i++;
				tmp_mem = src_mem + i;
			}
 
			nal_count++;
 
			tmp_0 = *(tmp_mem+3) & 0x1f ; 
      printf("%s %d  tmp_0 %d  %x\n",__FUNCTION__,__LINE__,tmp_0,*(tmp_mem+3));
			if (tmp_0==0x06 || tmp_0==0x07 || tmp_0==0x08)  //=== SEI，SPS, PPS, ，i_nal_type的值等于0x7表示这个nalu是个sps数据包
			{
				hd_start_code++ ;
				if (num_start_code==0 && hd_start_code==1) 
					*frame_start_addr = rbsp_base;
				if (num_start_code==1) num_start_code++ ;
			}
			
			if (tmp_0==0x01 || tmp_0==0x05)    //=== non-IDR picture, IDR picture，0x01为non-IDR片，0x05为I片
			{
				unsigned char  umpt,*temp;

				temp = tmp_mem+4;
				umpt = ue_v(temp);
 
			
				if (is_first_seq_header) //遇到nal_unit_type为1，5的分片时，并且is_first_seq_header=true，表示只获取视频头信息（H264：SEI+SPS+PPS），马上返回
				{                        //说明：is_first_seq_header一般在第一次调用VsParser函数时才传入true，目的是提取文件的视频头信息，但是这里有个问题：
					                    //如果文件的前几帧或第一帧不是I帧，比如是P帧，而P帧前面是没有SPS/PPS的，所以返回的可能就是SEI信息里。
					*frame_size     = rbsp_base - *frame_start_addr ;
					num_start_code = 2 ;
 
					break ;
				}

				tmp_1 = *(tmp_mem+4) & 0x80 ;  //== first_mb_in_slice，这个标志用来确定帧与帧的边界。判断该字节第一个bit是否为1，如果是1，就是一帧的第一片。
				if (tmp_1==0x80) //帧的第一个分片
				{ 
					//这里可能是进入函数后遇到的第一个分片，也可能是下一个帧的第一个分片，是哪种情况由num_start_code 和 hd_start_code的值确定
					num_start_code++ ;
					if (num_start_code==1 && hd_start_code==0) *frame_start_addr = rbsp_base;
				}
 
				if (num_start_code == 1) //找到第一个I/B/P帧的开始头（函数还不能返回）
				{
					umpt = ue_v(temp); //slice_type
					FrameType f_type = NONE;
					OMX_U8 slice_type = umpt;
					switch(slice_type)
					{
					case 2:case 7:
					case 4:case 9:
						f_type = I_Frame;
						//ATLTRACE("I Frame\n");
						if(conf_data) conf_data->cType = 'I';
						break;
					case 0:case 5:
					case 3:case 8:
						f_type = P_Frame;
						//ATLTRACE("P Frame \n");
						if(conf_data) conf_data->cType = 'P';
						break;
					case 1:case 6:
						f_type = B_Frame;
						//ATLTRACE("B Frame \n");
						if(conf_data) conf_data->cType = 'B';
						break;
					default:
						f_type = NONE;
						//ATLTRACE("NON I P B Frame\n");
						if(conf_data) conf_data->cType = 0;
						break;
					}
					
					//if(f_type != NONE) // 检查到帧类型
					//{
					//	  if(conf_data && conf_data->bGetWH == 0) //如果不是获取分辨率则跳出
					//	    break;
					//}
				}
 
			}
 
			if (num_start_code==2) //找到两个I/B/P帧的帧头，或者第一个是I/B/P帧的帧头，第2个是SEI/SPS/PPS帧的帧头，函数可以返回了
			{
				*frame_size = rbsp_base - *frame_start_addr;
				break ;
			}
 
			i+=3;
		}
		else
		{
			i++ ;
		}
		if (i>=buffer_size-4) break ;
	}
 
	//ATLTRACE("nal_count: %d, num_start_code: %d, hd_start_code: %d \n", nal_count - 1, num_start_code, hd_start_code); //打印变量信息
 
	if (num_start_code==2) return(0) ;   
	if (num_start_code==1) return(1) ;   
	else                   return(2) ;  
}

// 返回值意义：
// 0: one frame size is successfully determined AND there are still more frames
// 1,2: the size of the last frame is successfully determined. 1，2表示不够一帧数据，需要读取更多，或者是到了文件尾部
// 3: error (invalid standard)
 
OMX_U32 VsParser(SSBSIP_MFC_CODEC_TYPE standard, OMX_U32 buffer_start_addr, OMX_U32 buffer_size, /*u8* src_mem,*/
	OMX_U32 *frame_start_addr, OMX_U32 *frame_size,  OMX_U8 is_first_seq_header, MPEG4_CONFIG_DATA * conf_data)
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
								   frame_start_addr,  frame_size, is_first_seq_header, conf_data);
			if(ret_value==1||ret_value==2)	*frame_size=buffer_size;
			break ;
 
		default :
			ret_value = 3 ;
			break ;
	}
	return ret_value ;
}

void *InitRawFrameExtract(char* pFilename, MPEG4_CONFIG_DATA * pconf_data)
{
	SSBSIP_MFC_CODEC_TYPE eMode;
	char* pFileExt;
	OMX_U32 uFrameInfo[2];
	OMX_U32 uRemainSz, uRdPtr, uRet, uOneFrameAddr, uOneFrameSize, uFrameCnt, uFileOffset;
  OMX_U8 parseHeader;
	
	unsigned int parser_loop_cnt=0;
	int stuff_byte;
	CtxType *pCtx;
  eMode = H264_DEC;
	pCtx = (CtxType *)malloc(sizeof(CtxType));
	pCtx->lastFrameNum = -1;
	pCtx->fin = fopen(pFilename,"rb");
	if ( pCtx->fin==NULL )
		return NULL;
 
	pCtx->info = (frameInfo*)malloc(INFO_BUFF_SIZE);
	pCtx->frameNum = 0;
 
	uRemainSz = fread(pCtx->aInputBuf,1,STREAM_BUF_SIZE,pCtx->fin);	
	uRdPtr = (OMX_U32)pCtx->aInputBuf;
	uFrameCnt = 0;
	uFileOffset = 0;
  parseHeader  = 1;
	uRet = VsParser(eMode, uRdPtr, uRemainSz, &uOneFrameAddr, &uOneFrameSize, parseHeader, pconf_data); //第一次调用VsParser 第6个参数：parseHeader = true，表示获取视频头信息（H264：SEI+SPS+PPS）
 
	if (uRet == 1) //没有获取到一帧，已读到文件尾部
	{
		pCtx->info[pCtx->frameNum].offset = uFileOffset;
		pCtx->info[pCtx->frameNum].size= uRemainSz;
		pCtx->frameNum++;
		fclose(pCtx->fin);
		return pCtx;
	}
	else if (uRet!=0)
	{
		fclose(pCtx->fin);
		return NULL;
	}
 
	stuff_byte = uOneFrameAddr - uRdPtr;
	uRemainSz -= uOneFrameSize+stuff_byte;
	uRdPtr = uOneFrameAddr+uOneFrameSize;
	pCtx->info[pCtx->frameNum].offset = uFileOffset;
	pCtx->info[pCtx->frameNum].size= uOneFrameSize+stuff_byte;
	pCtx->frameNum++;
 
	uFileOffset += uOneFrameSize+stuff_byte;
	printf("\nStart Parsing.........\n");
 
 
	while (1) {
    //每次从文件读入一个块的数据到uRdPtr指向的缓冲区地址，然后调用VsParser提取出一帧数据出来，如果返回不足一帧，则继续读取下一个块。
		//VsParser返回0表示已经获取到完整的一帧，并记录当前帧的地址和帧的大小，下次读该帧后面的数据；
		//返回1或2表示数据不足或者缓冲区里找不到一帧的数据，这时候则需要读入文件的下一个块，如果已经读到文件尾部，则跳出循环。
		uRet = VsParser(eMode, uRdPtr, uRemainSz, &uOneFrameAddr, &uOneFrameSize, 0, pconf_data);
		if ( (uRet == 0) ) { /* Normal case. */
			uRdPtr = uOneFrameAddr + uOneFrameSize;
			uRemainSz -= uOneFrameSize;
 
			pCtx->info[pCtx->frameNum].offset = uFileOffset;
			pCtx->info[pCtx->frameNum].size= uOneFrameSize;
 
			pCtx->frameNum++;
			uFileOffset += uOneFrameSize;
			printf("Frame info--- size: %d, type: %c \n", uOneFrameSize, pconf_data->cType);
		}
		else if ((uRet == 1) || (uRet == 2)) {  /* Last frame or insufficient stream. */
 
			uOneFrameSize = uRemainSz;
			fseek(pCtx->fin, uFileOffset, SEEK_SET);
			uRemainSz = fread(pCtx->aInputBuf,1,STREAM_BUF_SIZE,pCtx->fin);	
			uRdPtr = (OMX_U32)pCtx->aInputBuf;
 
			if(uRemainSz==uOneFrameSize) {
				pCtx->info[pCtx->frameNum].offset = uFileOffset;
				pCtx->info[pCtx->frameNum].size= uOneFrameSize;
				pCtx->lastFrameNum = (uRet == 1) ? pCtx->frameNum : pCtx->frameNum - 1;
				break;
			}
			continue;
		}
		else
		{
			printf("[ERR] Parsing Error!!\n");
			return NULL;
		}
 
		parser_loop_cnt++;
	}
	printf(">> VsParser loop count : %d <<\n", parser_loop_cnt);
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
  CtxType *pctx = (CtxType *)InitRawFrameExtract("test.h264",&config_data);
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

