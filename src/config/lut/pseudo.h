// pseudo_cl.h
#pragma once
#include <CL/cl.h>
#include <stdint.h>
#include <stdbool.h>
#include "lut.h"
#include "../include/global.h"

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

    cl_mem d_input;
    cl_mem d_y_out;
    cl_mem d_uv_out;
    cl_mem d_lut_y;
    cl_mem d_lut_u;
    cl_mem d_lut_v;

    bool initialized;

    struct {
        cl_mem y;
        cl_mem u;
        cl_mem v;
        int current_type;
        bool initialized;
    } persistent_lut;

    cl_event write_event;
    cl_event kernel_event;
    cl_event read_events[2];
} PseudoCL;

extern PseudoCL cl_processor;

// 初始化OpenCL环境
bool PseudoCL_Init(PseudoCL* cl, int width, int height);

// 清理OpenCL资源
void PseudoCL_Cleanup(PseudoCL* cl);

// 处理图像
int PseudoCL_ProcessNV12(PseudoCL* cl,
                         uint16_t* input,
                         uint8_t* y_out,
                         uint8_t* uv_out,
                         int width,
                         int height,
                         int pseudo_type,
                         const struct YUV420P_LUT* lut,
                         float scale,
                         float min_val);

#ifdef __cplusplus
}
#endif