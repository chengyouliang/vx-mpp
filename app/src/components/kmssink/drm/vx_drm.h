/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * DRM driver for vanxum Tequila SoC
 * LCDC REGS
 *
 * Copyright 2021 VANXUM Electronics.
 * Ruinan Duan <duanruinan@vanxum.com>
 *
 */

#ifndef VX_DRM_H
#define VX_DRM_H

#include "drm.h"

#ifdef __cplusplus
extern "C" {
#endif

#define	VX_DUMB_CREATE_FLAGS_TILED_ALIGN	(1U << 0)

enum drm_vx_degamma_mode {
	VX_DEGAMMA_MODE_DISABLED = 0,
	/* TODO VX_DEGAMMA_MODE_BT601, */
	VX_DEGAMMA_MODE_BT709,
	VX_DEGAMMA_MODE_BT2020,
};

enum drm_vx_lcdc_output_fmt {
	VX_LCDC_OUTPUT_FMT_RGB888 = 0,
	VX_LCDC_OUTPUT_FMT_RGB101010,
};

enum drm_vx_hdmi_output_bpc_capability {
	VX_HDMI_OUTPUT_8BPC_CAPABILITY = 0,
	VX_HDMI_OUTPUT_10BPC_CAPABILITY,
};

enum drm_vx_hdmi_input_fmt {
	VX_HDMI_INPUT_FMT_RGB888 = 0,
	VX_HDMI_INPUT_FMT_RGB101010,
};

enum drm_vx_hdmi_output_fmt {
	VX_HDMI_OUTPUT_FMT_RGB888 = 0,
	VX_HDMI_OUTPUT_FMT_RGB101010,
};

struct drm_vx_watermark {
	__u32 watermark;
	__u16 qos_low;
	__u16 qos_high;
};

struct drm_vx_color_key {
	/* color key range */
	__u32 color_key_min;
	__u32 color_key_max;
	__u32 en; /* enable or not */
};

struct drm_vx_clear_color {
	__u32 color; /* ARGB8888 clear color */
	__u32 en; /* enable or not */
};

#define TREED_LUT_SIZE (17*17*17)

union drm_vx_3d_lut {
	union {
		__u32 all;
		struct {
			__u32 b:10;
			__u32 g:10;
			__u32 r:10;
			__u32 :2;
		} bit;
	} lut[TREED_LUT_SIZE];
};

#ifdef __cplusplus
}
#endif

#endif

