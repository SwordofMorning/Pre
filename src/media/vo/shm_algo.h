#pragma once

/**
 * @file shm_algo.h
 * @author Xiaojintao
 * @brief Share memory for Algorithm Backend program.
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
#define ALGO_SEM_KEY 0x0020
// RGB Buffer, used for algorithm backend.
#define ALGO_YUV_KEY 0x0021

// YUV420SP while UV is 128, for algo backend.
#define SHM_OUT_ALGO_SIZE (v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height * 3 / 2)

/**
 * @brief Init SHM, create shm and allocate buffer.
 * 
 * @return success or not.
 */
int SHM_ALGO_Init();

/**
 * @brief Process data, frame_sync -> algo_in -> algo_out -> SHM.
 * 
 * @return int 
 */
int SHM_ALGO_Process();

/**
 * @brief Delete SHM and free buffer.
 * 
 */
void SHM_ALGO_Exit();