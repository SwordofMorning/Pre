#pragma once

/**
 * @file _vis_camera.h
 * @author Xiaojintao
 * @brief Config for visible camera.
 * @version 0.1
 * @date 2024-12-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifdef __cplusplus
extern "C" {
#endif

enum VISIBLE_CAMERA_MODES
{
    VISIBLE_CAMERA_MODE_HGD_IMX335 = 1,
    VISIBLE_CAMERA_MODE_NGD = 2,
    VISIBLE_CAMERA_MODE_NUMS,
};

#define VISIBLE_CAMERA_MODE VISIBLE_CAMERA_MODE_HGD_IMX335

#ifdef __cplusplus
}
#endif