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
    void Pseudo_420P(uint16_t* input, uint8_t* y_out, uint8_t* u_out, uint8_t* v_out, int width, int height);
    void Pseudo_NV12(uint16_t* input, uint8_t* y_out, uint8_t* uv_out, int width, int height);
    void Pseudo_NV12_CL(uint16_t* input, uint8_t* y_out, uint8_t* uv_out, int width, int height);

public:
    void operator()(uint16_t* input, uint8_t* y_out, uint8_t* uv_out, int width, int height);
};