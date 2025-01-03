#pragma once

/**
 * @file lut.h
 * @author Xiaojintao
 * @brief Look up table for pseudo.
 * @version 0.1
 * @date 2024-12-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// clang-format off
#define RGB2Y(r, g, b) ((( 66 * r + 129 * g +  25 * b + 128) >> 8) +  16)
#define RGB2U(r, g, b) (((-38 * r -  74 * g + 112 * b + 128) >> 8) + 128)
#define RGB2V(r, g, b) (((112 * r -  94 * g -  18 * b + 128) >> 8) + 128)
// clang-format on

struct YUV420P_LUT
{
    uint8_t* y;
    uint8_t* u;
    uint8_t* v;
    int size;
};

enum LUT_TYPE
{
    LUT_IRONBOW_FORWARD = 0,
    LUT_IRONBOW_REVERSE,
    LUT_LAVA_FORWARD,
    LUT_LAVA_REVERSE,
    LUT_RAINBOW_FORWARD,
    LUT_RAINBOW_REVERSE,
    LUT_RAINBOWHC_FORWARD,
    LUT_RAINBOWHC_REVERSE,
    LUT_TYPE_COUNT
};

int Init_LUT(int type, const char* bin_file);
void Free_LUT(int type);
void Free_All_LUTs(void);

const struct YUV420P_LUT* Get_LUT(int type);

#ifdef __cplusplus
}
#endif