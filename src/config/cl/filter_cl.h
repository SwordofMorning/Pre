#pragma once

#include "cl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel_mean_filter;
    cl_kernel kernel_bilateral_filter;
    cl_mem d_input;
    cl_mem d_output;
    bool initialized;
} FilterCL;

extern FilterCL filter_cl;

bool FilterCL_Init(FilterCL* cl, int width, int height);
void FilterCL_Cleanup(FilterCL* cl);
int FilterCL_ProcessMean(FilterCL* cl, uint8_t* input, uint8_t* output, int width, int height, int window_size);
int FilterCL_ProcessBilateral(FilterCL* cl, uint8_t* input, uint8_t* output, int width, int height, float sigma_space, float sigma_range);

#ifdef __cplusplus
}
#endif