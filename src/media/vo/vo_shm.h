#pragma once

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "../../config/config.h"
#include "../algo/algo.h"
#include "../../utils/log/litelog.h"

// YUV Buffer, used for IR image data.
#define ALGO_SHM_YUV_KEY 0x0010
// Float Buffer, used for temperature data.
#define ALGO_SHM_FLOAT_KEY 0x0011
// YUV Buffer, used for Visible image data.
#define ALGO_SHM_CSI_KEY 0x0012
// Signal Key
#define ALGO_SEM_KEY 0x0020

// YUV420P buffer size
#define SHM_OUT_YUV_SIZE (v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height * 3 / 2)
// Float buffer size
#define SHM_OUT_FLOAT_SIZE (v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height * sizeof(float))
// UYVY buffer size
#define SHM_OUT_CSI_SIZE (v4l2_vis_csi_width * v4l2_vis_csi_height * 2)

/**
 * @brief Init SHM, create shm and allocate buffer.
 * 
 * @return success or not.
 */
int SHM_Init();

/**
 * @brief Process data, frame_sync -> algo_in -> algo_out -> SHM.
 * 
 * @return int 
 */
int SHM_Process();

/**
 * @brief Delete SHM and free buffer.
 * 
 */
void SHM_Exit();