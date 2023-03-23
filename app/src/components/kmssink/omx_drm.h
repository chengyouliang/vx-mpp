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

#ifndef OMX_DRM_H
#define OMX_RMC_H

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <drm_fourcc.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <xf86drmMode.h>
#include <xf86drm.h>
#include <dc_sw_tb_utils.h>
#include <vx_drm.h>
#include <stdio.h>

#include <OMX_Types.h>
#include <OMX_Core.h>

enum IMG_FMT{
	IMG_FMT_ARGB8888 = 0,
	IMG_FMT_ABGR8888,
	IMG_FMT_RGBA8888,
	IMG_FMT_BGRA8888,
	IMG_FMT_XRGB8888,
	IMG_FMT_XBGR8888,
	IMG_FMT_RGBX8888,
	IMG_FMT_BGRX8888,
	IMG_FMT_ARGB2101010,
	IMG_FMT_ABGR2101010,
	IMG_FMT_RGBA1010102,
	IMG_FMT_BGRA1010102,
	IMG_FMT_NV12,
	IMG_FMT_NV12_TILED_4X4,
	IMG_FMT_NV12_10BIT_TILED_4X4,
	IMG_FMT_YUV444_PLANAR_TILED_4X4,
	IMG_FMT_YUV444_10BIT_PLANAR_TILED_4X4,
} ;

enum plane_type {
	PL_TP_PRIMARY = 0,
	PL_TP_CURSOR,
	PL_TP_OVERLAY,
};

struct dma_buf {
	OMX_U32 width, height;
	size_t size;
	OMX_U32 fourcc;
	OMX_U32 fb_id;

	OMX_U32 bpps[4];
	OMX_U32 handles[4];
	OMX_U32 offsets[4];
	OMX_U32 strides[4];
	OMX_U32 widths[4];
	OMX_U32 heights[4];
	OMX_U64 mods[4];
	size_t sizes[4]; 
	void *maps[4];
	OMX_S32 count_planes;
	OMX_U64 modifier;
};

struct drm_plane {
	OMX_U32 id;
	struct list_head link;

	struct drm_crtc *crtc;

	enum plane_type type;

	OMX_U32 possible_crtcs;

	s32 count_formats;
	OMX_U32 formats[128];

	OMX_U32 prop_crtc_id;
	OMX_U32 prop_fb_id;
	OMX_U32 prop_crtc_x;
	OMX_U32 prop_crtc_y;
	OMX_U32 prop_crtc_w;
	OMX_U32 prop_crtc_h;
	OMX_U32 prop_src_x;
	OMX_U32 prop_src_y;
	OMX_U32 prop_src_w;
	OMX_U32 prop_src_h;
	OMX_U32 prop_zpos;
	OMX_U32 prop_clear_color;
	OMX_U32 prop_pixel_blend_mode;
	OMX_U32 prop_alpha;
	OMX_U32 prop_color_key;

	OMX_U32 clear_color_blob_id;
	OMX_U32 color_key_blob_id;
};
struct drm_encoder {
	u32 id;
	struct list_head link;
	struct drm_connector *connector;
	struct drm_crtc *crtc;
	u32 possible_crtcs;
};
struct drm_crtc {
	OMX_U32 id;
	struct list_head link;
	OMX_S32 pipe;

	OMX_U32 prop_active;
	OMX_U32 prop_mode_id;
	OMX_U32 prop_bg_color;
	OMX_U32 mode_blob_id;

	struct drm_encoder *encoder;

	struct list_head planes;
	struct drm_plane *primary;
	struct drm_plane *overlay[4];
	OMX_S32 overlay_cnt;
	struct drm_plane *cursor;
};

struct drm_connector {
	u32 id;
	struct list_head link;

	struct drm_encoder *encoder;
	drmModeConnector *conn;

	s32 count_encoders;
	u32 encoders[64];

	u32 prop_crtc_id;
};

struct drm_dev {
	s32 fd;
	drmModeRes *res;
	drmModePlaneRes *pres;
    struct dma_buf *buf;
    struct drm_crtc *curcrtc;

	struct list_head crtcs;
	struct list_head planes;
	struct list_head encoders;
	struct list_head connectors;
};


#endif