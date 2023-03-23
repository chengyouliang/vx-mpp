/**
  @file src/components/kms/library_entry_point.c
  
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


#include "omx_drm.h"

static s32 get_prop_id(s32 fd, drmModeObjectProperties *props, const char *name)
{
	drmModePropertyPtr property;
	u32 i, id = 0;

	for (i = 0; i < props->count_props; i++) {
		property = drmModeGetProperty(fd, props->props[i]);
		if (!property)
			continue;
		if (!strcasecmp(property->name, name))
			id = property->prop_id;
		drmModeFreeProperty(property);
		if (id)
			return id;
	}

	return -1;
}
static s32 get_prop_value(s32 fd, drmModeObjectProperties *props,
			  const char *name, u32 *value)
{
	drmModePropertyPtr property;
	u32 i;

	for (i = 0; i < props->count_props; i++) {
		property = drmModeGetProperty(fd, props->props[i]);
		if (!property)
			continue;
		if (!strcasecmp(property->name, name)) {
			*value = props->prop_values[i];
			drmModeFreeProperty(property);
			return 0;
		}
		drmModeFreeProperty(property);
	}
	return -ENOENT;
}

static struct drm_plane *create_plane(struct drm_dev *dev, s32 index)
{
	struct drm_plane *plane;
	drmModePlane *pl;
	drmModeObjectProperties *props;
	u32 value = 0;
	s32 i, zpos;

	plane = calloc(1, sizeof(*plane));
	plane->id = dev->pres->planes[index];

	pl = drmModeGetPlane(dev->fd, plane->id);
	plane->possible_crtcs = pl->possible_crtcs;
	plane->count_formats = pl->count_formats;
	for (i = 0; i < plane->count_formats; i++) {
		/* printf("\t\t\t%4.4s\n", (char *)&plane->formats[i]); */
		plane->formats[i] = pl->formats[i];
	}
	drmModeFreePlane(pl);

	props = drmModeObjectGetProperties(dev->fd,
					   plane->id,
					   DRM_MODE_OBJECT_PLANE);

	plane->prop_crtc_id = get_prop_id(dev->fd, props, "CRTC_ID");
	plane->prop_fb_id = get_prop_id(dev->fd, props, "FB_ID");
	plane->prop_crtc_x = get_prop_id(dev->fd, props, "CRTC_X");
	plane->prop_crtc_y = get_prop_id(dev->fd, props, "CRTC_Y");
	plane->prop_crtc_w = get_prop_id(dev->fd, props, "CRTC_W");
	plane->prop_crtc_h = get_prop_id(dev->fd, props, "CRTC_H");
	plane->prop_src_x = get_prop_id(dev->fd, props, "SRC_X");
	plane->prop_src_y = get_prop_id(dev->fd, props, "SRC_Y");
	plane->prop_src_w = get_prop_id(dev->fd, props, "SRC_W");
	plane->prop_src_h = get_prop_id(dev->fd, props, "SRC_H");
	plane->prop_zpos = get_prop_id(dev->fd, props, "ZPOS");
	get_prop_value(dev->fd, props, "ZPOS", &value);
	zpos = (s32)value;
	plane->prop_clear_color = get_prop_id(dev->fd, props, "CLEAR_COLOR");
	plane->prop_color_key = get_prop_id(dev->fd, props, "COLOR_KEY");
	plane->prop_alpha = get_prop_id(dev->fd, props, "alpha");
	plane->prop_pixel_blend_mode = get_prop_id(dev->fd, props,
						   "pixel blend mode");

	get_prop_value(dev->fd, props, "type", &value);
	if (value == DRM_PLANE_TYPE_CURSOR)
		plane->type = PL_TP_CURSOR;
	else if (value == DRM_PLANE_TYPE_PRIMARY)
		plane->type = PL_TP_PRIMARY;
	else if (value == DRM_PLANE_TYPE_OVERLAY)
		plane->type = PL_TP_OVERLAY;
	printf("plane index: %d, id: %u, type: %d, zpos: %d\n", index, plane->id,
		plane->type, zpos);

	/* get_prop_value(dev->fd, props, "alpha", &value);
	get_prop_value(dev->fd, props, "pixel blend mode", &value); */

	drmModeFreeObjectProperties(props);

	return plane;
}

static struct drm_crtc *create_crtc(struct drm_dev *dev, s32 index)
{
	struct drm_crtc *crtc;
	drmModeObjectProperties *props;

	crtc = calloc(1, sizeof(*crtc));
	crtc->id = dev->res->crtcs[index];
	crtc->pipe = index;

	props = drmModeObjectGetProperties(dev->fd,
					   crtc->id,
					   DRM_MODE_OBJECT_CRTC);
	crtc->prop_active = get_prop_id(dev->fd, props, "ACTIVE");
	crtc->prop_mode_id = get_prop_id(dev->fd, props, "MODE_ID");
	crtc->prop_bg_color = get_prop_id(dev->fd, props, "BG_COLOR");
	drmModeFreeObjectProperties(props);

	INIT_LIST_HEAD(&crtc->planes);

	return crtc;
}

static struct drm_encoder *create_encoder(struct drm_dev *dev, s32 index)
{
	struct drm_encoder *encoder;
	drmModeEncoder *enc;

	encoder = calloc(1, sizeof(struct drm_encoder));
	encoder->id = dev->res->encoders[index];

	enc = drmModeGetEncoder(dev->fd, encoder->id);
	encoder->possible_crtcs = enc->possible_crtcs;

	drmModeFreeEncoder(enc);

	return encoder;
}

static struct drm_connector *create_connector(struct drm_dev *dev, s32 index)
{
	struct drm_connector *connector;
	drmModeConnector *conn;
	drmModeObjectProperties *props;

	connector = calloc(1, sizeof(*connector));
	connector->id = dev->res->connectors[index];

	conn = drmModeGetConnector(dev->fd, connector->id);
	connector->conn = conn;

	connector->count_encoders = conn->count_encoders;
	memcpy(&connector->encoders[0], &conn->encoders[0],
		connector->count_encoders * sizeof(conn->encoders[0]));

	props = drmModeObjectGetProperties(dev->fd,
					   connector->id,
					   DRM_MODE_OBJECT_CONNECTOR);
	connector->prop_crtc_id = get_prop_id(dev->fd, props, "CRTC_ID");
	drmModeFreeObjectProperties(props);

	return connector;
}


static s32 create_pipeline(struct drm_dev *dev, s32 index)
{
	struct drm_crtc *crtc;
	struct drm_plane *plane, *nplane;
	struct drm_encoder *encoder, *nencoder;
	struct drm_connector *connector, *nconnector;
	s32 i, overlay_cnt;

	list_for_each_entry(crtc, &dev->crtcs, link) {
		if (crtc->pipe == index)
			goto found_crtc;
	}

	fprintf(stderr, "cannot found crtc\n");
	return -ENODEV;

found_crtc:
	/* find primary plane */
    dev->curcrtc = crtc;
	list_for_each_entry_safe(plane, nplane, &dev->planes, link) {
		if (plane->type != PL_TP_PRIMARY)
			continue;
		if (plane->possible_crtcs & (1U << crtc->pipe)) {
			list_del(&plane->link);
			list_add_tail(&plane->link, &crtc->planes);
			crtc->primary = plane;
			plane->crtc = crtc;
			goto primary_found;
		}
	}

	fprintf(stderr, "cannot found primary plane\n");
	return -ENODEV;

primary_found:
	/* find overlay plane */
	overlay_cnt = 0;
	list_for_each_entry_safe(plane, nplane, &dev->planes, link) {
		/* printf("Plane Type: %u\n", plane->type);
		printf("Plane Possible CRTCS: %u\n", plane->possible_crtcs);
		printf("Current CRTC's pipe: %u\n", crtc->pipe); */
		if (plane->type != PL_TP_OVERLAY)
			continue;
		if (plane->possible_crtcs & (1U << crtc->pipe)) {
			list_del(&plane->link);
			list_add_tail(&plane->link, &crtc->planes);
			crtc->overlay[overlay_cnt] = plane;
			overlay_cnt++;
			plane->crtc = crtc;
		}
		if (overlay_cnt == 2)
			break;
	}
	/* printf("Found %d overlays\n", overlay_cnt); */
	crtc->overlay_cnt = overlay_cnt;

	/* find cursor plane */
	list_for_each_entry_safe(plane, nplane, &dev->planes, link) {
		if (plane->type != PL_TP_CURSOR)
			continue;
		if (plane->possible_crtcs & (1U << crtc->pipe)) {
			list_del(&plane->link);
			list_add_tail(&plane->link, &crtc->planes);
			crtc->cursor = plane;
			plane->crtc = crtc;
			goto cursor_found;
		}
	}

	fprintf(stderr, "cannot found cursor plane\n");
	return -ENODEV;

cursor_found:
	/* find encoder */
	list_for_each_entry_safe(encoder, nencoder, &dev->encoders, link) {
		if (encoder->possible_crtcs & (1U << crtc->pipe)) {
			list_del(&encoder->link);
			crtc->encoder = encoder;
			encoder->crtc = crtc;
			goto encoder_found;
		}
	}

	fprintf(stderr, "cannot found encoder\n");
	return -ENODEV;

encoder_found:
	/* find connector */
	list_for_each_entry_safe(connector, nconnector, &dev->connectors, link){
		for (i = 0; i < connector->count_encoders; i++) {
			if (connector->encoders[i] == encoder->id) {
				list_del(&connector->link);
				connector->encoder = encoder;
				encoder->connector = connector;
				goto established;
			}
		}
	}

	fprintf(stderr, "cannot found connector\n");
	return -ENODEV;

established:
	return 0;
}

struct dma_buf *dma_buf_create(s32 fd, u32 width, u32 height, u32 fourcc,
			       bool tiled4x4)
{
	struct dma_buf *buf;
	struct drm_mode_create_dumb create_arg;
	struct drm_mode_map_dumb map_arg;
	s32 i, ret;

	buf = calloc(1, sizeof(*buf));
	if (!buf) {
		fprintf(stderr, "cannot alloc buf\n");
		return NULL;
	}

	buf->width = width;
	buf->height = height;
	buf->fourcc = fourcc;

	switch (fourcc) {
	case DRM_FORMAT_XRGB8888:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_RGBX8888:
	case DRM_FORMAT_BGRX8888:
	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_BGRA8888:
		buf->bpps[0] = 32;
		buf->count_planes = 1;
		buf->widths[0] = width;
		buf->heights[0] = height;
		break;
	case DRM_FORMAT_ARGB2101010:
	case DRM_FORMAT_ABGR2101010:
	case DRM_FORMAT_RGBA1010102:
	case DRM_FORMAT_BGRA1010102:
		buf->bpps[0] = 32;
		buf->count_planes = 1;
		buf->widths[0] = width;
		buf->heights[0] = height;
		break;
	case DRM_FORMAT_YUV444:
		if (!tiled4x4) {
			return NULL;
		}
		buf->mods[0] = DRM_FORMAT_MOD_VX_4_4_TILE;
		buf->mods[1] = DRM_FORMAT_MOD_VX_4_4_TILE;
		buf->mods[2] = DRM_FORMAT_MOD_VX_4_4_TILE;
		buf->bpps[0] = buf->bpps[1] = buf->bpps[2] = 8;
		buf->count_planes = 3;
		buf->widths[0] = width;
		buf->widths[1] = width;
		buf->widths[2] = width;
		buf->heights[0] = height;
		buf->heights[1] = height;
		buf->heights[2] = height;
		break;
	case DRM_FORMAT_YUV444_10:
		if (!tiled4x4) {
			return NULL;
		}
		buf->mods[0] = DRM_FORMAT_MOD_VX_4_4_TILE;
		buf->mods[1] = DRM_FORMAT_MOD_VX_4_4_TILE;
		buf->mods[2] = DRM_FORMAT_MOD_VX_4_4_TILE;
		buf->bpps[0] = buf->bpps[1] = buf->bpps[2] = 10;
		buf->count_planes = 3;
		buf->widths[0] = width;
		buf->widths[1] = width;
		buf->widths[2] = width;
		buf->heights[0] = height;
		buf->heights[1] = height;
		buf->heights[2] = height;
		break;
	case DRM_FORMAT_NV12:
		if (tiled4x4) {
			buf->mods[0] = DRM_FORMAT_MOD_VX_4_4_TILE;
			buf->mods[1] = DRM_FORMAT_MOD_VX_4_4_TILE;
		}
		buf->count_planes = 2;
		buf->bpps[0] = 8;
		buf->bpps[1] = 8;
		buf->bpps[2] = 0;
		buf->widths[0] = width;
		buf->widths[1] = width;
		buf->heights[0] = height;
		buf->heights[1] = height / 2;
		break;
	case DRM_FORMAT_NV12_10:
		if (!tiled4x4) {
			return NULL;
		}
		buf->mods[0] = DRM_FORMAT_MOD_VX_4_4_TILE;
		buf->mods[1] = DRM_FORMAT_MOD_VX_4_4_TILE;
		buf->count_planes = 2;
		buf->bpps[0] = 10;
		buf->bpps[1] = 10;
		buf->bpps[2] = 0;
		buf->widths[0] = width;
		buf->widths[1] = width;
		buf->heights[0] = height;
		buf->heights[1] = height / 2;
		break;
	default:
		fprintf(stderr, "illegal format %d\n", __LINE__);
		return NULL;
	}

	for (i = 0; i < buf->count_planes; i++) {
		memset(&create_arg, 0, sizeof(create_arg));
		create_arg.bpp = buf->bpps[i];
		create_arg.width = buf->widths[i];
		create_arg.height = buf->heights[i];
		if (tiled4x4)
			create_arg.flags = VX_DUMB_CREATE_FLAGS_TILED_ALIGN;
		else
			create_arg.flags = 0;
		drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_arg);
		buf->handles[i] = create_arg.handle;
		buf->strides[i] = create_arg.pitch;
		printf("strides[%d]: %u\n", i, buf->strides[i]);

		memset(&map_arg, 0, sizeof(map_arg));
		map_arg.handle = create_arg.handle;
		drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map_arg);
		buf->sizes[i] = create_arg.size;
		printf("sizes[%d]: %lu\n", i, buf->sizes[i]);
		buf->maps[i] = mmap(NULL, create_arg.size, PROT_WRITE,
				MAP_SHARED,
				fd, map_arg.offset);
		printf("maps[%d]: %p\n", i, buf->maps[i]);

		
	}

	if (buf->mods[0]) {
		ret = drmModeAddFB2WithModifiers(fd, buf->width,
						 buf->height,
						 buf->fourcc,
						 buf->handles, buf->strides,
						 buf->offsets, (const uint64_t *)buf->mods,
						 &buf->fb_id,
						 DRM_MODE_FB_MODIFIERS);
	} else {
		ret = drmModeAddFB2(fd, buf->width, buf->height,
			buf->fourcc,
			buf->handles, buf->strides, buf->offsets,
			&buf->fb_id, 0);
	}

	if (ret) {
		fprintf(stderr, "failed to add fb %s\n", strerror(errno));
	}

	return buf;
}

int set_mode(struct drm_dev *dev,u32 width, u32 height) 
{
    
    struct drm_plane *primary;
    struct drm_crtc *crtc;
    int ret = 0;
	drmModeConnector *conn;
    struct drm_connector *connector;
    connector = crtc->encoder->connector;
	conn = connector->conn;
    int i;
	for (i = 0; i < conn->count_modes; i++) {
        if (conn->modes[i].hdisplay == width &&
            conn->modes[i].vdisplay == height) {
            break;
        }
    }
	if (i == conn->count_modes) {
		return -ENOENT;
	}
	printf("hdisplay %d \n",conn->modes[i].hdisplay);
	printf("vdisplay %d \n",conn->modes[i].vdisplay);
	printf("vrefresh %d \n",conn->modes[i].vrefresh);
	if (crtc->mode_blob_id == 0) {
		ret = drmModeCreatePropertyBlob(dev->fd,
			    &conn->modes[i],
				sizeof(conn->modes[i]), &crtc->mode_blob_id);
        if (ret) {
            fprintf(stderr, "failed to create mode\n");
            return ret;
        }
    }
    return ret;
}

void device_destroy(struct drm_dev *dev)
{
	struct drm_connector *connector, *nconnector;
	struct drm_encoder *encoder, *nencoder;
	struct drm_crtc *crtc, *ncrtc;
	struct drm_plane *plane, *nplane;

	if (!dev)
		return;

	list_for_each_entry_safe(plane, nplane, &dev->planes, link) {
		list_del(&plane->link);
		free(plane);
	}

	list_for_each_entry_safe(connector, nconnector, &dev->connectors, link){
		list_del(&connector->link);
		free(connector);
	}

	list_for_each_entry_safe(encoder, nencoder, &dev->encoders, link) {
		list_del(&encoder->link);
		free(encoder);
	}

	list_for_each_entry_safe(crtc, ncrtc, &dev->crtcs, link) {
		list_del(&crtc->link);
		free(crtc);
	}

	if (dev->pres) {
		drmModeFreePlaneResources(dev->pres);
		dev->pres = NULL;
	}

	if (dev->res) {
		drmModeFreeResources(dev->res);
		dev->res = NULL;
	}

	if (dev->fd > 0) {
		close(dev->fd);
		dev->fd = 0;
	}
	free(dev);
}



struct drm_dev *device_create(u32 width, u32 height,u32 index,u32 fourcc)
{
	s32 ret, i;
    struct drm_connector *connector;

	struct drm_dev *dev = calloc(1, sizeof(*dev));

	if (!dev)
		return NULL;

	dev->fd = open("/dev/dri/card0", O_RDWR, 0644);
	INIT_LIST_HEAD(&dev->crtcs);
	INIT_LIST_HEAD(&dev->encoders);
	INIT_LIST_HEAD(&dev->connectors);
	INIT_LIST_HEAD(&dev->planes);

	ret = drmSetClientCap(dev->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
	if (ret) {
		fprintf(stderr, "failed to set universal planes cap. %s\n",
			strerror(errno));
		return NULL;
	}
	ret = drmSetClientCap(dev->fd, DRM_CLIENT_CAP_ATOMIC, 1);
	if (ret) {
		fprintf(stderr, "failed to set atomic cap. %s\n",
			strerror(errno));
		return NULL;
	}
	dev->res = drmModeGetResources(dev->fd);
	if (!dev->res) {
		fprintf(stderr, "failed to get drm resources\n");
		device_destroy(dev);
		return NULL;
	}

	dev->pres = drmModeGetPlaneResources(dev->fd);
	if (!dev->pres) {
		fprintf(stderr, "failed to get drm plane resources\n");
		device_destroy(dev);
		return NULL;
	}

	for (i = 0; i < dev->res->count_connectors; i++) {
		struct drm_connector *connector;

		connector = create_connector(dev, i);
		list_add_tail(&connector->link, &dev->connectors);
	}

	for (i = 0; i < dev->res->count_encoders; i++) {
		struct drm_encoder *encoder;

		encoder = create_encoder(dev, i);
		list_add_tail(&encoder->link, &dev->encoders);
	}

	for (i = 0; i < dev->res->count_crtcs; i++) {
		struct drm_crtc *crtc;

		crtc = create_crtc(dev, i);
		list_add_tail(&crtc->link, &dev->crtcs);
	}

	for (i = 0; i < dev->pres->count_planes; i++) {
		struct drm_plane *plane;

		plane = create_plane(dev, i);
		list_add_tail(&plane->link, &dev->planes);
	}
    ret = create_pipeline(dev,index);
    if (ret < 0)    
    {
        return NULL;
    }
    
    ret  = set_mode(dev,width,height);
    if (ret < 0)   
    {
        return NULL;
    }
    dev->buf = dma_buf_create(dev->fd,width,height,fourcc,0);
    if (dev->buf  == NULL)
    {
        return NULL;
    }
	return dev;
}



int drm_show(struct drm_dev *dev,OMX_U8 *buf,OMX_U32 size)
{
    struct dma_buf *dmabuf = NULL;
	drmModeAtomicReq *req;
	struct drm_crtc *crtc;
	struct drm_connector *connector;
	struct drm_plane *primary;
	s32 ret = 0;
    u32 width;
    u32 height;
	u32 flags;
	s32 i, j;
	u32 zpos_base[2] = {0, 3};
    connector = crtc->encoder->connector;
	primary = crtc->primary;
	width = dev->buf->width;
    height = dev->buf->height;

    flags = DRM_MODE_ATOMIC_ALLOW_MODESET;
    
    req = drmModeAtomicAlloc();
    dmabuf = dev->buf;
    memcpy(dmabuf->maps[0],buf,size);
	ret = drmModeAtomicAddProperty(req, connector->id,
				       connector->prop_crtc_id, crtc->id);
	
	ret = drmModeAtomicAddProperty(req, crtc->id, crtc->prop_active,
					       1);
	
	ret = drmModeAtomicAddProperty(req, crtc->id,
					       crtc->prop_mode_id,
					       crtc->mode_blob_id);
	
	ret = drmModeAtomicAddProperty(req, primary->id,
					       primary->prop_crtc_id, crtc->id);
	
	ret = drmModeAtomicAddProperty(req, primary->id,
					       primary->prop_crtc_x, 0);
	ret = drmModeAtomicAddProperty(req, primary->id,
					       primary->prop_crtc_y, 0);
	ret = drmModeAtomicAddProperty(req, primary->id,
					       primary->prop_crtc_w,
					       width);
	ret = drmModeAtomicAddProperty(req, primary->id,
					       primary->prop_crtc_h,
					       height);

	ret = drmModeAtomicAddProperty(req, primary->id,
					       primary->prop_src_x, 0);
	ret = drmModeAtomicAddProperty(req, primary->id,
					       primary->prop_src_y, 0);
	ret = drmModeAtomicAddProperty(req, primary->id,
					       primary->prop_src_w,
					       width << 16);
	ret = drmModeAtomicAddProperty(req, primary->id,
					       primary->prop_src_h,
					       height << 16);

	ret = drmModeAtomicAddProperty(req, primary->id,
					       primary->prop_fb_id, dmabuf->fb_id);
	ret = drmModeAtomicAddProperty(req, primary->id,
					       primary->prop_zpos,
					       zpos_base[j] + 0);

	ret = drmModeAtomicCommit(dev->fd, req, flags, dev);
	if (ret) {
	    fprintf(stderr, "failed to commit (%s)\n",strerror(errno));
	}
    return ret;
}