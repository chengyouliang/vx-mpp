/* Copyright (C) 2008-2020 Allegro DVT2.  All rights reserved. */
#pragma once

#include <linux/types.h>

#define AL_CMD_UNBLOCK_CHANNEL _IO('q', 1)

#define SCHED_SEARCH_SC			_IOW('q', 2, struct al5_sched_info)
#define SCHED_CREATE_CHANNEL	_IOWR('q', 3, struct al5_sched_info)
#define SCHED_DESTROY_CHANNEL	_IOW('q', 4, struct al5_sched_info)
#define SCHED_DECODE_FRAME		_IOW('q', 5, struct al5_sched_info)
#define SCHED_DECODE_SLICE		_IOW('q', 6, struct al5_sched_info)
#define SCHED_DECODE_JPEG		_IOW('q', 7, struct al5_sched_info)

#define AL_CMD_IP_WAIT_CB          _IOWR('q', 12, struct cb_info)
#define GET_DMA_FD                  _IOWR('q', 13, struct al5_dma_info)
#define GET_DMA_MMAP                _IOWR('q', 26, struct al5_dma_info)
#define GET_DMA_PHY                 _IOWR('q', 18, struct al5_dma_info)


struct al5_dma_info {
	__u32 fd;
	__u32 size;
	__u32 phy_addr;
};

struct al5_sched_info {
	int  id;
	void *arg1;
	void *arg2;
	void *arg3;
};

struct parse_info {
	int frame_id;
	int parsing_id;
};

struct dec_info {
	__u8 frm_id;
	__u8 mv_id;
	__u16 pic_state;
	__u32 crc;
};

struct sc_info {
	__u32 num_sc;
	__u32 num_bytes;
};

struct cb_info {
	__u16 cb_type;
	__u16 id;
	union {
		struct parse_info parse;
		struct dec_info dec;
		struct sc_info sc;
	};
};