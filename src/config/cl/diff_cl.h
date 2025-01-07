#pragma once

#include <stdlib.h>
#include <string.h>
#include "cl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel_diff;
    cl_mem d_current;    // 当前帧
    cl_mem d_last;       // 上一帧
    cl_mem d_output;     // 输出
    uint16_t* last_frame;  // CPU端存储上一帧
    bool initialized;
} DiffCL;

extern DiffCL diff_cl;

bool DiffCL_Init(DiffCL* cl, int width, int height);
void DiffCL_Cleanup(DiffCL* cl);
int DiffCL_Process(DiffCL* cl, uint16_t* input, uint16_t* output, int width, int height, float rate);

#ifdef __cplusplus
}
#endif