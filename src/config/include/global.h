#pragma once

/**
 * @file global.h
 * @author Xiaojintao
 * @brief Global variable define.
 * @version 0.1
 * @date 2024-12-31
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

#define FRAME_SYNC_BUFFER_SIZE 8
#define IR_TARGET_FPS 25

struct FrameSync16
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
    uint16_t* frame_buffer[FRAME_SYNC_BUFFER_SIZE];
};
extern struct FrameSync16 frame_sync_dvp;

struct FrameSync8
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
    uint8_t* frame_buffer[FRAME_SYNC_BUFFER_SIZE];
};
extern struct FrameSync8 frame_sync_csi;

/* ========== SHM ========== */

extern uint16_t* algo_in;
extern uint8_t* shm_out_yuv;
extern float* shm_out_float;
extern uint8_t* shm_out_algo;

extern int shmid_yuv;
extern int shmid_float;
extern int shmid_csi;
extern int shmid_algo;
extern int semid_vo;
extern int semid_ab;

// IR yuv data
extern uint8_t* shm_yuv;
// IR temperature data
extern float* shm_float;
// Visible yuv data
extern uint8_t* shm_vis;
// IR algo data.
extern uint8_t* shm_algo;

/* ========== Config ========== */

struct UserConfig
{
    int pseudo;
    int gas_enhancement;
    bool in_focus;
    bool mean_filter;
    bool gas_enhancement_software;
};
extern struct UserConfig usr;

struct TempParams
{
    float a;
    float b;
    float c;
};

extern struct TempParams temp_param;

#ifdef __cplusplus
}
#endif