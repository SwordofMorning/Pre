#pragma once

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