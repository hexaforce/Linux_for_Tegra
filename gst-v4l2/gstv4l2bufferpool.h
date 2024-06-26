/* GStreamer
 *
 * Copyright (C) 2001-2002 Ronald Bultje <rbultje@ronald.bitfreak.net>
 *               2006 Edgard Lima <edgard.lima@gmail.com>
 *               2009 Texas Instruments, Inc - http://www.ti.com/
 * Copyright (c) 2018-2021, NVIDIA CORPORATION. All rights reserved.
 *
 * gstv4l2bufferpool.h V4L2 buffer pool class
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __GST_V4L2_BUFFER_POOL_H__
#define __GST_V4L2_BUFFER_POOL_H__

#include <gst/gst.h>

/* The structures are renamed as the name conflicts with the
 * OSS v4l2 library structures. */
#ifdef USE_V4L2_TARGET_NV
#define GstV4l2BufferPool GstNvV4l2BufferPool
#define GstV4l2BufferPoolClass GstNvV4l2BufferPoolClass
#endif

typedef struct _GstV4l2BufferPool GstV4l2BufferPool;
typedef struct _GstV4l2BufferPoolClass GstV4l2BufferPoolClass;
typedef struct _GstV4l2Meta GstV4l2Meta;

#include "gstv4l2object.h"
#include "gstv4l2allocator.h"

G_BEGIN_DECLS

#define GST_TYPE_V4L2_BUFFER_POOL      (gst_v4l2_buffer_pool_get_type())
#define GST_IS_V4L2_BUFFER_POOL(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_V4L2_BUFFER_POOL))
#define GST_V4L2_BUFFER_POOL(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_V4L2_BUFFER_POOL, GstV4l2BufferPool))
#define GST_V4L2_BUFFER_POOL_CAST(obj) ((GstV4l2BufferPool*)(obj))

/* This flow return is used to indicated that the last buffer of a
 * drain or a resoltuion change has been found. This should normally
 * only occure for mem-2-mem devices. */
#define GST_V4L2_FLOW_LAST_BUFFER GST_FLOW_CUSTOM_SUCCESS

/* This flow return is used to indicated that the returned buffer was marked
 * with the error flag and had no payload. This error should be recovered by
 * simply waiting for next buffer. */
#define GST_V4L2_FLOW_CORRUPTED_BUFFER GST_FLOW_CUSTOM_SUCCESS_1

struct _GstV4l2BufferPool
{
  GstBufferPool parent;

  GstV4l2Object *obj;        /* the v4l2 object */
  gint video_fd;             /* a dup(2) of the v4l2object's video_fd */
  GstPoll *poll;             /* a poll for video_fd */
  GstPollFD pollfd;
  gboolean can_poll_device;

  gboolean empty;
  GCond empty_cond;

  GstV4l2Allocator *vallocator;
  GstAllocator *allocator;
  GstAllocationParams params;
  GstBufferPool *other_pool;
  guint size;
  GstVideoInfo caps_info;   /* Default video information */

  gboolean add_videometa;    /* set if video meta should be added */
  gboolean enable_copy_threshold; /* If copy_threshold should be set */

  guint min_latency;         /* number of buffers we will hold */
  guint max_latency;         /* number of buffers we can hold */
  guint num_queued;          /* number of buffers queued in the driver */
  guint num_allocated;       /* number of buffers allocated */
  guint copy_threshold;      /* when our pool runs lower, start handing out copies */

  gboolean streaming;
  gboolean flushing;

  GstBuffer *buffers[VIDEO_MAX_FRAME];

  /* signal handlers */
  gulong group_released_handler;

  /* Control to warn only once on buggy feild driver bug */
  gboolean has_warned_on_buggy_field;

#ifdef USE_V4L2_TARGET_NV
  gboolean enable_dynamic_allocation; /* If dynamic_allocation should be set */
#endif
};

struct _GstV4l2BufferPoolClass
{
  GstBufferPoolClass parent_class;
};

GType gst_v4l2_buffer_pool_get_type (void);

GstBufferPool *     gst_v4l2_buffer_pool_new     (GstV4l2Object *obj, GstCaps *caps);

GstFlowReturn       gst_v4l2_buffer_pool_process (GstV4l2BufferPool * bpool, GstBuffer ** buf);

void                gst_v4l2_buffer_pool_set_other_pool (GstV4l2BufferPool * pool,
                                                         GstBufferPool * other_pool);
void                gst_v4l2_buffer_pool_copy_at_threshold (GstV4l2BufferPool * pool,
                                                            gboolean copy);

gboolean            gst_v4l2_buffer_pool_flush   (GstBufferPool *pool);

#ifdef USE_V4L2_TARGET_NV
void
gst_v4l2_buffer_pool_enable_dynamic_allocation (GstV4l2BufferPool * pool,
                                                gboolean enable_dynamic_allocation);
gint
get_motion_vectors (GstV4l2Object *obj, guint32 bufferIndex,
            v4l2_ctrl_videoenc_outputbuf_metadata_MV *enc_mv_metadata);
#endif

G_END_DECLS

#endif /*__GST_V4L2_BUFFER_POOL_H__ */
