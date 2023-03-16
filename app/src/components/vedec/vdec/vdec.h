#ifndef __VDEC_H__
#define __VDEC_H__
#include <stdint.h>

#define vdec_fourcc(a, b, c, d) ((uint32_t)(a) | ((uint32_t)(b) << 8) | \
				 ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))
// Color Formats
#define VDEC_FORMAT_NV12		vdec_fourcc('N', 'V', '1', '2') /* 2x2 subsampled Cr:Cb plane */
#define VDEC_FORMAT_P010		vdec_fourcc('P', '0', '1', '0') /* 2x2 subsampled Cr:Cb plane 10 bits per channel */
#define VDEC_FORMAT_NV16		vdec_fourcc('N', 'V', '1', '6')
#define VDEC_FORMAT_P210		vdec_fourcc('P', '2', '1', '0')
#define VDEC_FORMAT_I444		vdec_fourcc('I', '4', '4', '4')
#define VDEC_FORMAT_I4AL		vdec_fourcc('I', '4', 'A', 'L')
#define VDEC_FORMAT_T608		vdec_fourcc('T', '6', '0', '8')
#define VDEC_FORMAT_T60A		vdec_fourcc('T', '6', '0', 'A')
#define VDEC_FORMAT_T628		vdec_fourcc('T', '6', '2', '8')
#define VDEC_FORMAT_T62A		vdec_fourcc('T', '6', '2', 'A')
#define VDEC_FORMAT_T648		vdec_fourcc('T', '6', '4', '8')
#define VDEC_FORMAT_T64A		vdec_fourcc('T', '6', '4', 'A')

/**********************************************************************************
                               Types
**********************************************************************************/
typedef void* VDecHandle;
typedef void* FrameHandle;

typedef enum _ChromaMode
{
	VDEC_CHROMA_MONO = 0, /*!< Monochrome */
	VDEC_CHROMA_4_0_0 = VDEC_CHROMA_MONO, /*!< 4:0:0 = Monochrome */
	VDEC_CHROMA_4_2_0 = 1, /*!< 4:2:0 chroma sampling */
	VDEC_CHROMA_4_2_2 = 2, /*!< 4:2:2 chroma sampling */
	VDEC_CHROMA_4_4_4 = 3, /*!< 4:4:4 chroma sampling */
	VDEC_CHROMA_MAX_ENUM, /* sentinel */
}VDEC_EChromaMode;

#define VDEC_CS_FLAGS(Flags) (((Flags) & 0xFFFF) << 8)
#define AVC_PROFILE_IDC_CAVLC_444 44 // not supported
#define AVC_PROFILE_IDC_BASELINE 66
#define AVC_PROFILE_IDC_MAIN 77
#define AVC_PROFILE_IDC_EXTENDED 88 // not supported
#define AVC_PROFILE_IDC_HIGH 100
#define AVC_PROFILE_IDC_HIGH10 110
#define AVC_PROFILE_IDC_HIGH_422 122
#define AVC_PROFILE_IDC_HIGH_444_PRED 244

#define VDEC_RExt_FLAGS(Flags) (((Flags) & 0xFFFF) << 8)
#define HEVC_PROFILE_IDC_MAIN 1
#define HEVC_PROFILE_IDC_MAIN10 2
#define HEVC_PROFILE_IDC_MAIN_STILL 3
#define HEVC_PROFILE_IDC_RExt 4

typedef enum _ECodec
{
	/* assign hardware standard value */
	VDEC_CODEC_AVC = 0,
	VDEC_CODEC_HEVC = 1,
	VDEC_CODEC_AV1 = 2,
	VDEC_CODEC_VP9 = 3,
	VDEC_CODEC_JPEG = 4,
	VDEC_CODEC_VVC = 5,
	VDEC_CODEC_INVALID, /* sentinel */
}VDEC_ECodec;

typedef enum __attribute__((aligned(4))) _VDEC_Profile
{
	VDEC_PROFILE_AVC = (VDEC_CODEC_AVC << 24),
	VDEC_PROFILE_AVC_CAVLC_444 = VDEC_PROFILE_AVC | AVC_PROFILE_IDC_CAVLC_444, // not supported
	VDEC_PROFILE_AVC_BASELINE = VDEC_PROFILE_AVC | AVC_PROFILE_IDC_BASELINE,
	VDEC_PROFILE_AVC_MAIN = VDEC_PROFILE_AVC | AVC_PROFILE_IDC_MAIN,
	VDEC_PROFILE_AVC_EXTENDED = VDEC_PROFILE_AVC | AVC_PROFILE_IDC_EXTENDED, // not supported
	VDEC_PROFILE_AVC_HIGH = VDEC_PROFILE_AVC | AVC_PROFILE_IDC_HIGH,
	VDEC_PROFILE_AVC_HIGH10 = VDEC_PROFILE_AVC | AVC_PROFILE_IDC_HIGH10,
	VDEC_PROFILE_AVC_HIGH_422 = VDEC_PROFILE_AVC | AVC_PROFILE_IDC_HIGH_422,
	VDEC_PROFILE_AVC_HIGH_444_PRED = VDEC_PROFILE_AVC | AVC_PROFILE_IDC_HIGH_444_PRED, // not supported

	VDEC_PROFILE_AVC_C_BASELINE = VDEC_PROFILE_AVC_BASELINE | VDEC_CS_FLAGS(0x0002),
	VDEC_PROFILE_AVC_PROG_HIGH = VDEC_PROFILE_AVC_HIGH | VDEC_CS_FLAGS(0x0010),
	VDEC_PROFILE_AVC_C_HIGH = VDEC_PROFILE_AVC_HIGH | VDEC_CS_FLAGS(0x0030),
	VDEC_PROFILE_AVC_HIGH10_INTRA = VDEC_PROFILE_AVC_HIGH10 | VDEC_CS_FLAGS(0x0008),
	VDEC_PROFILE_AVC_HIGH_422_INTRA = VDEC_PROFILE_AVC_HIGH_422 | VDEC_CS_FLAGS(0x0008),
	VDEC_PROFILE_AVC_HIGH_444_INTRA = VDEC_PROFILE_AVC_HIGH_444_PRED | VDEC_CS_FLAGS(0x0008), // not supported

	VDEC_PROFILE_HEVC = (VDEC_CODEC_HEVC << 24),
	VDEC_PROFILE_HEVC_MAIN = VDEC_PROFILE_HEVC | HEVC_PROFILE_IDC_MAIN,
	VDEC_PROFILE_HEVC_MAIN10 = VDEC_PROFILE_HEVC | HEVC_PROFILE_IDC_MAIN10,
	VDEC_PROFILE_HEVC_MAIN_STILL = VDEC_PROFILE_HEVC | HEVC_PROFILE_IDC_MAIN_STILL,
	VDEC_PROFILE_HEVC_RExt = VDEC_PROFILE_HEVC | HEVC_PROFILE_IDC_RExt,
	VDEC_PROFILE_HEVC_MONO = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0xFC80),
	VDEC_PROFILE_HEVC_MONO10 = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0xDC80),
	VDEC_PROFILE_HEVC_MONO12 = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0x9C80), // not supported
	VDEC_PROFILE_HEVC_MONO16 = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0x1C80), // not supported
	VDEC_PROFILE_HEVC_MAIN12 = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0x9880),
	VDEC_PROFILE_HEVC_MAIN_422 = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0xF080),
	VDEC_PROFILE_HEVC_MAIN_422_10 = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0xD080),
	VDEC_PROFILE_HEVC_MAIN_422_12 = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0x9080),
	VDEC_PROFILE_HEVC_MAIN_444 = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0xE080),
	VDEC_PROFILE_HEVC_MAIN_444_10 = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0xC080),
	VDEC_PROFILE_HEVC_MAIN_444_12 = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0x8080), // not supported
	VDEC_PROFILE_HEVC_MAIN_INTRA = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0xFA00),
	VDEC_PROFILE_HEVC_MAIN10_INTRA = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0xDA00),
	VDEC_PROFILE_HEVC_MAIN12_INTRA = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0x9A00), // not supported
	VDEC_PROFILE_HEVC_MAIN_422_INTRA = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0xF200),
	VDEC_PROFILE_HEVC_MAIN_422_10_INTRA = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0xD200),
	VDEC_PROFILE_HEVC_MAIN_422_12_INTRA = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0x9200), // not supported
	VDEC_PROFILE_HEVC_MAIN_444_INTRA = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0xE200),
	VDEC_PROFILE_HEVC_MAIN_444_10_INTRA = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0xC200),
	VDEC_PROFILE_HEVC_MAIN_444_12_INTRA = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0x8200), // not supported
	VDEC_PROFILE_HEVC_MAIN_444_16_INTRA = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0x0200), // not supported
	VDEC_PROFILE_HEVC_MAIN_444_STILL = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0xE300),
	VDEC_PROFILE_HEVC_MAIN_444_16_STILL = VDEC_PROFILE_HEVC_RExt | VDEC_RExt_FLAGS(0x0300), // not supported

	VDEC_PROFILE_JPEG = (VDEC_CODEC_JPEG << 24),

	VDEC_PROFILE_UNKNOWN = 0xFFFFFFFF
} VDEC_EProfile;

typedef struct {
	int32_t iWidth;
	int32_t iHeight;
	int iBitDepth; /*!< Stream's bit depth */
	int iLevel; /*!< Stream's level */
	VDEC_EChromaMode eChroma; /*!< Stream's chroma mode (400/420/422/444) */
	VDEC_EProfile eProfile; /*!< Stream's profile */
	//VDEC_ESequenceMode eSequenceMode; /*!< Stream's sequence mode */
}VDecPreAllocArgs;

typedef struct _VDecConfig{
	int 	 tiledFmt;		// 1: request decode output Tiled Color Format
	VDEC_ECodec  type;			// Video Decoder Type AVC, HEVC or JPEG
	int 	 splitInput;	        // bitstream input mode, 1: Spilt or 0: unSplit
	int	 latencyMode;	        // latency mode, 0:frame latency 1: slice latency
	int	 mutiChunk;		// 1: planes of frame in muti chunk  0: planes of frame in on chunk
	uint32_t zBufSize;		// bitstream internal buffer size, used for split mode

	VDecPreAllocArgs preAllocArgs;
}VDecConfig;

/**********************************************************************************
                               Interfaces
**********************************************************************************/

/***
 * 初始化视频解码器
 * 输入：
 *     VDecConfig *vConf 解码器配置参数
 * 输出：
 *     返回解码器的Handle; NULL:创建解码器失败
 */
VDecHandle createVideoDecoder(VDecConfig *vConf);

/***
 * 销毁视频解码器，释放申请或占用的资源
 */
void destroyVideoDecoder(VDecHandle vDec);

/***
 * 向解码器输入一帧码流
 * 输入：
 *     VDecHandle vDec 视频解码器Handle
 *     uint8_t *stream 待解码的视频码流包
 *     uint32_t size   码流大小
 *     0 为成功 其它失败
 */
int decodePutStream(VDecHandle vDec, uint8_t *stream, uint32_t size);

/***
 * 获取解码器的eventfd，作为异步获取解码数据的通知
 * 输入：
 *     VDecHandle vDec 视频解码器Handle
 * 输出：
 *     eventfd的值
 */
int decodeGetEventFd(VDecHandle vDec);

/***
 * 向解码器获取一帧解码数据
 * 输入：
 *     VDecHandle vDec   视频解码器Handle
 *     FrameHandle *frame 解码输出帧信息
 * 输出：
 *
 *     VMA_SUCCESS:为成功
 *     VDEC_FRAME_CHANGE:Frame Change，可以获取当前所有的Frame
 *     VDEC_END_OF_FRAME:为EOF
 */
int decodeGetFrameSync(VDecHandle vDec, FrameHandle *frame);
int decodeGetFrameAsync(VDecHandle vDec, FrameHandle *frame);

/***
 * 获取当前解码器内部Buffer
 * 输入：
 *     VDecHandle vDec   视频解码器Handle
 * 
 *  输出：
 *     FrameHandle数组指针，以NULL结尾
 */
FrameHandle *decodeGetInternalFrames(VDecHandle vDec);
/***
 * 将解码数据的Buffer归还给解码器
 * 输入：
 *     VDecHandle vDec   视频解码器Handle
 *     VideoFrame frame 解码输入帧信息
 */
void decodeReleaseFrame(VDecHandle vDec, FrameHandle frame);


int decodeGetFrameDim(FrameHandle frame, uint32_t *width, uint32_t *height);
int decodeGetFrameFourCC(FrameHandle frame, uint32_t *format);
int decodeGetFrameStride(FrameHandle frame, uint32_t stride[4]);
int decodeGetFrameFd(FrameHandle frame, int32_t fd[4]);
int decodeGetFrameData(FrameHandle frame, uint8_t *data[4]);

void vdec_frame_hook( FrameHandle frame , void *p ) ;
void* vdec_frame_unhook( FrameHandle frame ) ;
#endif
