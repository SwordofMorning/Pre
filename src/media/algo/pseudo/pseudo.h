#pragma once

/**
 * @file pseudo.h
 * @author Xiaojintao
 * @brief Pseudo CPP wrapper.
 * @version 0.1
 * @date 2024-12-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "pseudo_am.h"

class Pseudo
{
private:
    void Pseudo_NV12_CL(uint16_t* input, uint8_t* y_out, uint8_t* uv_out, int width, int height, float cb_min, float cb_max);
    void Pseudo_Isotherms_CL(uint16_t* input, uint8_t* y_out, uint8_t* uv_out, int width, int height, float* temps, float threshold_min, float threshold_max, uint8_t* uv_maps);

public:
    void operator()(uint16_t* input, uint8_t* y_out, uint8_t* uv_out, int width, int height, float cb_min, float cb_max, float* temps);
};