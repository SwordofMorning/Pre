#include "tm.h"

void Temperature_Measurement::Quadratic(uint16_t* input, float* output, int width, int height, float a, float b, float c)
{
    TMCL_Process(&tm_cl, input, output, width, height, a, b, c);
}

void Temperature_Measurement::Exp(uint16_t* input, float* output, int width, int height, float A, float B, float emi)
{
    TMCL_Process_Exp(&tm_cl, input, output, width, height, A, B, emi);
}