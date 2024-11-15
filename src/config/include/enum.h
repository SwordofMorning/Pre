#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Pseudo color enum.
enum
{
    // Iron bow i.e. iron red.
    PSEUDO_COLOR_IRON_BOW = 0,
    // White hot, white means high temperature.
    PSEUDO_COLOR_WHITE_HOT,
    // Black hot, black means high temperature.
    PSEUDO_COLOR_BLACK_HOT,
    // Rainbow.
    PSEUDO_COLOR_RAINBOW,
    // Rainbow High Contrast.
    PSEUDO_COLOR_RAINBOW_HC,
    // Fotric 8bits iron bow.
    PSEUDO_COLOR_IRON_BOW_F8,
    // Fotric 10bits iron bow.
    PSEUDO_COLOR_IRON_BOW_F10,
    // Nums of Pseudo color.
    PSEUDO_COLOR_NUMS,
};

/**
 * @brief Gas Enhancement Coefficient, which is used for FPGA's communication.
 * @attention Begin with 1, not 0!
 * @note use GAS_ENHANCEMENT_MIN < gas_enhancement < GAS_ENHANCEMENT_MAX to verify that it is valid.
 */
enum
{
    GAS_ENHANCEMENT_MIN = 0,
    GAS_ENHANCEMENT_NONE,
    GAS_ENHANCEMENT_2,
    GAS_ENHANCEMENT_3,
    GAS_ENHANCEMENT_4,
    GAS_ENHANCEMENT_5,
    GAS_ENHANCEMENT_6,
    GAS_ENHANCEMENT_7,
    GAS_ENHANCEMENT_8,
    GAS_ENHANCEMENT_9,
    GAS_ENHANCEMENT_10,
    GAS_ENHANCEMENT_11,
    GAS_ENHANCEMENT_12,
    GAS_ENHANCEMENT_13,
    GAS_ENHANCEMENT_14,
    GAS_ENHANCEMENT_15,
    GAS_ENHANCEMENT_16,
    GAS_ENHANCEMENT_MAX,
};

enum
{
    // Take a photo.
    CAPTURE_STATE_PHOTO_ONCE = 0,
    // Start record video.
    CAPTURE_STATE_VIDEO_START,
    // End record video.
    CAPTURE_STATE_VIDEO_End,
    // Nums of capture state.
    CAPTURE_STATE_NUMS,
};

#ifdef __cplusplus
}
#endif