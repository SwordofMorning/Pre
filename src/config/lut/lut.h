#pragma once

#include <stdint.h>

#define RGB2Y(r, g, b) ((( 66 * r + 129 * g +  25 * b + 128) >> 8) +  16)
#define RGB2U(r, g, b) (((-38 * r -  74 * g + 112 * b + 128) >> 8) + 128)
#define RGB2V(r, g, b) (((112 * r -  94 * g -  18 * b + 128) >> 8) + 128)

struct YUV420P_LUT
{
    uint8_t y[65536];
    uint8_t u[65536];
    uint8_t v[65536];
};

extern struct YUV420P_LUT lava_lut;

void Init_Lava_LUT();