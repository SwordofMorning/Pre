#pragma once

/**
 * @file shm_vo.h
 * @author Xiaojintao
 * @brief Share memory for Video Out program.
 * @version 0.1
 * @date 2024-12-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */

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

// Signal Key
#define VO_SEM_KEY 0x0010
// YUV Buffer, used for IR image data.
#define VO_YUV_KEY 0x0011
// Float Buffer, used for temperature data.
#define VO_FLOAT_KEY 0x0012
// YUV Buffer, used for visible image data.
#define VO_CSI_KEY 0x0013

// YUV420SP | NV12, for IR data.
#define SHM_OUT_YUV_SIZE (v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height * 3 / 2)
// Float, for Temp data.
#define SHM_OUT_FLOAT_SIZE (v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height * sizeof(float))
// YUV420SP | NV12, for Vis data.
#define SHM_OUT_CSI_SIZE (v4l2_vis_csi_width * v4l2_vis_csi_height * V4L2_VIS_CSI_PIX_FMT_SCALE)

/**
 * @brief Init SHM, create shm and allocate buffer.
 * 
 * @return success or not.
 */
int SHM_VO_Init();

/**
 * @brief Process data, frame_sync -> algo_in -> algo_out -> SHM.
 * 
 * @return int 
 */
int SHM_VO_Process();

/**
 * @brief Delete SHM and free buffer.
 * 
 */
void SHM_VO_Exit();