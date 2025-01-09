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
    cl_kernel kernel_tm;
    cl_mem d_input;
    cl_mem d_output;
    bool initialized;
} TMCL;

extern TMCL tm_cl;

bool TMCL_Init(TMCL* cl, int width, int height);
void TMCL_Cleanup(TMCL* cl);
int TMCL_Process(TMCL* cl, uint16_t* input, float* output, int width, int height, float a, float b, float c);

#ifdef __cplusplus
}
#endif