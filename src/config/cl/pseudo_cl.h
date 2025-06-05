#pragma once

/**
 * @file pseudo.h
 * @author Xiaojintao
 * @brief OpenCL kernel constructor for pseudo color.
 * @version 0.1
 * @date 2024-12-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "cl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel_black_hot;
    cl_kernel kernel_white_hot;
    cl_kernel kernel_color_map;
    cl_kernel kernel_isotherms;

    cl_mem d_input;
    cl_mem d_y_out;
    cl_mem d_uv_out;
    cl_mem d_lut_y;
    cl_mem d_lut_u;
    cl_mem d_lut_v;
    cl_mem d_temps;
    cl_mem d_uv_maps;

    bool initialized;
} PseudoCL;

extern PseudoCL pseudo_cl;

// Init OpenCL
bool PseudoCL_Init(PseudoCL* cl, int width, int height);

// Cleanup OpenCL
void PseudoCL_Cleanup(PseudoCL* cl);

/**
 * @brief Pseudo interface via OpenCL.
 * 
 * @param cl OpenCL handle.
 * @param input Original Data from sensor.
 * @param y_out Luma of output data.
 * @param uv_out Chroma ouf output data.
 * @param width Width of picture.
 * @param height Height of picture.
 * @param pseudo_type enum of pseudo, eg. PSEUDO_IRONBOW_FORWARD.
 * @param lut pesudo lookup table.
 * @param scale 16bits to 8bits scale params, i.e. max of valid data in 16bits array.
 * @param min_val min of valid data in 16bits array.
 * @param scale_min colar bar min scale params.
 * @param scale_max colar bar max scale params.
 * @return success or not.
 */
int PseudoCL_ProcessNV12(PseudoCL* cl,
                         uint16_t* input,
                         uint8_t* y_out,
                         uint8_t* uv_out,
                         int width,
                         int height,
                         int pseudo_type,
                         const struct YUV420P_LUT* lut,
                         float scale,
                         float min_val,
                         float scale_min,
                         float scale_max);

int PseudoCL_ProcessIsotherms(PseudoCL* cl,
                              uint16_t* input,
                              uint8_t* y_out,
                              uint8_t* uv_out,
                              int width,
                              int height,
                              const struct YUV420P_LUT* lut,
                              float scale,
                              float min_val,
                              float* temps,
                              float threshold_min,
                              float threshold_max,
                              uint8_t* uv_maps);

#ifdef __cplusplus
}
#endif