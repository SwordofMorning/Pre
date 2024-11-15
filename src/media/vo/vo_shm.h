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

// 定义共享内存key
#define ALGO_SHM_YUV_KEY 0x0010
#define ALGO_SHM_FLOAT_KEY 0x0011
#define ALGO_SEM_KEY 0x0012

// YUV420P buffer size
#define SHM_OUT_YUV_SIZE (v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height * 3 / 2)
// Float buffer size
#define SHM_OUT_FLOAT_SIZE (v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height * sizeof(float))

// 初始化算法模块
int SHM_Init();

// 处理一帧数据
int SHM_Process();

// 退出算法模块
void SHM_Exit();