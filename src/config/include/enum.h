#pragma once

/**
 * @file enum.h
 * @author Xiaojintao
 * @brief Enum for program.
 * @version 0.1
 * @date 2024-12-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifdef __cplusplus
extern "C" {
#endif

// Pseudo color enum.
enum PSEUDO
{
    PSEUDO_IRONBOW_FORWARD = 0,
    PSEUDO_IRONBOW_REVERSE,
    PSEUDO_LAVA_FORWARD,
    PSEUDO_LAVA_REVERSE,
    PSEUDO_RAINBOW_FORWARD,
    PSEUDO_RAINBOW_REVERSE,
    PSEUDO_RAINBOWHC_FORWARD,
    PSEUDO_RAINBOWHC_REVERSE,
    PSEUDO_WHITE_HOT,
    PSEUDO_BLACK_HOT,
    PSEUDO_NUMS,
};

/**
 * @brief Gas Enhancement Coefficient, which is used for FPGA's communication.
 * @attention Begin with 1, not 0!
 * @note use GAS_ENHANCEMENT_MIN < gas_enhancement < GAS_ENHANCEMENT_MAX to verify that it is valid.
 */
enum GAS_ENHANCEMENT
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

#ifdef __cplusplus
}
#endif