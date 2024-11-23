#pragma once

#include <stdint.h>

#define RGB2Y(r, g, b) ((( 66 * r + 129 * g +  25 * b + 128) >> 8) +  16)
#define RGB2U(r, g, b) (((-38 * r -  74 * g + 112 * b + 128) >> 8) + 128)
#define RGB2V(r, g, b) (((112 * r -  94 * g -  18 * b + 128) >> 8) + 128)

#define COLOR_MAP_SIZE 625  // lava.csv的实际颜色数量
struct YUV420P_LUT {
    uint8_t y[COLOR_MAP_SIZE];
    uint8_t u[COLOR_MAP_SIZE];
    uint8_t v[COLOR_MAP_SIZE];
};

extern struct YUV420P_LUT lava_lut;

void Init_Lava_LUT();