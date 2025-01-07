#pragma once

/**
 * @file debug.h
 * @author Xiaojintao
 * @brief Debug option for program.
 * @version 0.1
 * @date 2024-12-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifdef __cplusplus
extern "C" {
#endif

// Save sensor data as binary file.
#define __DVP_SAVE__ 0

// Statistics DVP FPS.
#define __DVP_FPS__ 0

/**
 * @brief DVP Continuous capture or not.
 * @note 1, capture data in while(1);
 * @note 0, capture a fixed number of frames, specified by `__DVP_CAPTURE_FRAMES__`.
 */
#define __DVP_CONTINUOUS_CAPTURE__ 1

// Captured number of frames on noncontinuous dvp mode.
#define __DVP_CAPTURE_FRAMES__ 100

#define __SHOW_TIME_CONSUME__ 1

#ifdef __cplusplus
}
#endif