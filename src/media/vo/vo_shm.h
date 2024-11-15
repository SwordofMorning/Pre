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

// 定义共享内存key
#define ALGO_SHM_YUV_KEY 0x0010
#define ALGO_SHM_FLOAT_KEY 0x0011
#define ALGO_SEM_KEY 0x0012

// 图像尺寸定义
#define ALGO_WIDTH 640
#define ALGO_HEIGHT 512

// YUV420P buffer size
#define ALGO_YUV_SIZE (ALGO_WIDTH * ALGO_HEIGHT * 3 / 2)
// Float buffer size
#define ALGO_FLOAT_SIZE (ALGO_WIDTH * ALGO_HEIGHT * sizeof(float))

// 初始化算法模块
int algo_init();

// 处理一帧数据
int algo_process();

// 退出算法模块
void algo_exit();