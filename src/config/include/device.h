#pragma once

/**
 * @file device.h
 * @author Xiaojintao
 * @brief 
 * @version 0.1
 * @date 2024-08-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include "_vis_cmaera.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================================== */
/* ======================================== IR Camera ======================================== */
/* =========================================================================================== */

// DVP interface device name i.e. IR FPGA sensor input.
#define V4L2_IR_DVP_DEVICE_NAME "/dev/video0"
// DVP interface input pixel format, capture as gray8.
#define V4L2_IR_DVP_PIX_FMT V4L2_PIX_FMT_GREY
// Numbers of DVP V4L2 request buffers.
#define V4L2_IR_DVP_REQ_COUNT 5

// FPGA's output mode.
enum
{
    // 640*512*16bits model, with 640*2, height 513.
    V4L2_IR_DVP_MODE_640 = 0,
    // 320*256*16bits model, with 320*2, height 258.
    V4L2_IR_DVP_MODE_320,
    // Nums of fpga's modes.
    V4L2_IR_DVP_MODE_NUMS,
};

// Capture with in 640 mode.
#define V4L2_IR_DVP_CAPTURE_WIDTH_640 1280
// Capture height in 640 mode.
#define V4L2_IR_DVP_CAPTURE_HEIGHT_640 513
// Capture with in 320 mode.
#define V4L2_IR_DVP_CAPTURE_WIDTH_320 640
// Capture height in 320 mode.
#define V4L2_IR_DVP_CAPTURE_HEIGHT_320 258

// V4L2 crop 640
#define V4L2_IR_DVP_CAPTURE_CROP_640 "v4l2-ctl -d /dev/video0 --set-crop top=0,left=0,width=1280,height=513"
// V4L2 crop 320
#define V4L2_IR_DVP_CAPTURE_CROP_320 "v4l2-ctl -d /dev/video0 --set-crop top=0,left=0,width=640,height=258"

// Valid with in 640 mode.
#define V4L2_IR_DVP_VALID_WIDTH_640 640
// Valid height in 640 mode.
#define V4L2_IR_DVP_VALID_HEIGHT_640 512
// Valid with in 320 mode.
#define V4L2_IR_DVP_VALID_WIDTH_320 320
// Valid height in 320 mode.
#define V4L2_IR_DVP_VALID_HEIGHT_320 256

// Use to specific which mode we choose.
extern int v4l2_ir_dvp_mode;
// IR capture width, only have: 640*2 = 1280 or 320*2 = 640, statistics as uint8.
extern int v4l2_ir_dvp_capture_width;
// IR capture height, only have: 513 in 640 mode, or 258 in 320 mode, statistics as uint8.
extern int v4l2_ir_dvp_capture_height;
// IR valid width, only have: 640 or 320, statistics as uint16.
extern int v4l2_ir_dvp_valid_width;
// IR valid height, only have: 512 or 256, statistics as uint16.
extern int v4l2_ir_dvp_valid_height;

// DVP's file descriptor.
extern int v4l2_ir_dvp_fd;
// DVP's nplanes
extern int v4l2_ir_dvp_nplanes;
// To specific which buffer we current use in V4L2 capture queue.
extern int v4l2_ir_dvp_buffer_global_index;
// To specific buffer length.
extern int v4l2_ir_dvp_buffer_global_length;

// DVP's capture buffer as V4L2's queue.
struct V4L2_IR_DVP_Buffer
{
    void* start;
    size_t length;
};

// Point to DVP's capture buffer.
extern struct V4L2_IR_DVP_Buffer* v4l2_ir_dvp_buffer_global;

/* ============================================================================================ */
/* ======================================== Vis Camera ======================================== */
/* ============================================================================================ */

#if (VISIBLE_CAMERA_MODE == VISIBLE_CAMERA_MODE_HGD_IMX335)
#define V4L2_VIS_CSI_DEVICE_NAME "/dev/video22"
#define V4L2_VIS_CSI_PIX_FMT V4L2_PIX_FMT_NV12
// It means NV12 have 1.5 times size of pixels
#define V4L2_VIS_CSI_PIX_FMT_SCALE 1.5
#elif (VISIBLE_CAMERA_MODE == VISIBLE_CAMERA_MODE_NGD)
#define V4L2_VIS_CSI_DEVICE_NAME "/dev/video11"
#define V4L2_VIS_CSI_PIX_FMT V4L2_PIX_FMT_UYVY
// It means UYVY have 2 times size of pixels
#define V4L2_VIS_CSI_PIX_FMT_SCALE 2
#endif

#define V4L2_VIS_CSI_REQ_COUNT 5

enum V4L2_VIS_CSI_MODE
{
    V4L2_VIS_CSI_MODE_NGD_2688x1520at25 = 0,
    V4L2_VIS_CSI_MODE_HGD_2592x1944at30 = 1,
    V4L2_VIS_CSI_MODE_NUMS,
};

#define V4L2_VIS_CSI_CAPTURE_WIDTH_2688 2688
#define V4L2_VIS_CSI_CAPTURE_HEIGHT_1520 1520
#define V4L2_VIS_CSI_CAPTURE_WIDTH_2592 2592
#define V4L2_VIS_CSI_CAPTURE_HEIGHT_1944 1944

extern int v4l2_vis_csi_fd;
extern int v4l2_vis_csi_nplanes;
extern int v4l2_vis_csi_buffer_global_index;
extern int v4l2_vis_csi_buffer_global_length;

struct V4L2_VIS_CSI_Buffer
{
    void* start;
    size_t length;
};

extern struct V4L2_VIS_CSI_Buffer* v4l2_vis_csi_buffer_global;

extern int v4l2_vis_csi_mode;
extern int v4l2_vis_csi_width;
extern int v4l2_vis_csi_height;

#ifdef __cplusplus
}
#endif
