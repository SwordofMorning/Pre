#pragma once

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
#include <pthread.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <stdarg.h>

#include "device.h"
#include "enum.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========== V4L2 ========== */

// Share memory of dvp captured data.
extern uint16_t v4l2_ir_dvp_share_buffer[V4L2_IR_DVP_VALID_WIDTH_640 * V4L2_IR_DVP_VALID_HEIGHT_640];
// Mutex of dvp shm.
extern pthread_mutex_t v4l2_ir_dvp_share_buffer_mutex;
// Cond of dvp shm.
extern pthread_cond_t v4l2_ir_dvp_share_buffer_cond;
// Count of dvp shm.
extern int v4l2_ir_dvp_share_buffer_updated;

/* ========== SHM ========== */

extern uint16_t* algo_in;
extern uint8_t* algo_out_yuv;
extern float* algo_out_float;

extern int shmid_yuv;
extern int shmid_float;
extern int semid;

extern uint8_t* shm_yuv;
extern float* shm_float;

#define SHM_FRAME_BUFFER_SIZE 8
#define IR_TARGET_FPS 25

struct FrameSync
{
    pthread_mutex_t mutex;
    pthread_cond_t producer_cond;
    pthread_cond_t consumer_cond;
    int write_pos;
    int read_pos;
    int frame_count;
    bool buffer_full;
    struct timeval last_frame_time;
    // Loop Buffer
    uint16_t* frame_buffer[SHM_FRAME_BUFFER_SIZE];
};
extern struct FrameSync frame_sync;

/* ========== Config ========== */

// Gas Enhancement Param.
extern int gas_enhancement;

// To specific which pseudo color mode we use.
extern int pseudo_color;

#ifdef __cplusplus
}
#endif