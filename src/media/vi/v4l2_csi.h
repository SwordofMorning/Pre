#pragma once

/**
 * @file v4l2_dvp.h
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
#include "../../config/config.h"
#include "../../utils/log/litelog.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start DVP camera streaming, restore data in v4l2_ir_dvp_buffer_global.
 * 
 * @return nothing now.
 */
int CSI_Streaming();

#ifdef __cplusplus
}
#endif