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

/* ========== SHM ========== */

extern uint16_t* algo_in;
extern uint8_t* shm_out_yuv;
extern float* shm_out_float;

extern int shmid_yuv;
extern int shmid_float;
extern int semid_vo;

// IR yuv data
extern uint8_t* shm_yuv;
// IR temperature data
extern float* shm_float;

/* ========== Config ========== */

struct TempParamsQuadratic
{
    float a;
    float b;
    float c;
};

struct TempParamsLn
{
    float a;
    float b;
    float epsilon;
};

struct TempParams
{
    struct TempParamsQuadratic quadratic;
    struct TempParamsLn ln;
};

struct UserConfig
{
    // mutex of UserConfig
    pthread_mutex_t mutex;

    // @see: enum PSEUDO
    int pseudo;
    // @see: enum GAS_ENHANCEMENT
    int gas_enhancement;
    // @see: in_focus or not, avoid repeated focus
    bool in_focus;
    // mean filter enable or not
    bool mean_filter;
    // software GE enable or not
    bool gas_enhancement_software;
    // temperature measurements
    struct TempParams tm;

    // Color Bar max, percentage [0, 1]
    float color_bar_max;
    // Color Bar min, percentage [0, 1]
    float color_bar_min;

    // ture, enable isothermal mode; otherwise use normal pseudo mode
    bool isothermal;
    // isothermal threshold max, float tempreatures
    float isothermal_threshold_max;
    // isothermal threshold min, float tempreatures
    float isothermal_threshold_min;
    // isothermal uv map, i.e. lt[u][v], med[u][v], gt[u][v], 3 groups of uv
    uint8_t isothermal_uv_map[6];
};
extern struct UserConfig usr;

#ifdef __cplusplus
}
#endif