#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Save sensor data as binary file.
#define __DVP_SAVE__ 0

// Statistics DVP FPS.
#define __DVP_FPS__ 1

/**
 * @brief DVP Continuous capture or not.
 * @note 1, capture data in while(1);
 * @note 0, capture a fixed number of frames, specified by `__DVP_CAPTURE_FRAMES__`.
 */
#define __DVP_CONTINUOUS_CAPTURE__ 0

// Captured number of frames on noncontinuous dvp mode.
#define __DVP_CAPTURE_FRAMES__ 100

#ifdef __cplusplus
}
#endif