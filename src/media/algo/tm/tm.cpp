#include "tm.h"

void Temperature_Measurement::operator()(uint16_t* input, float* output, int width, int height, float a, float b, float c)
{
    TMCL_Process(&tm_cl, input, output, width, height, a, b, c);
}