#include "algo.h"

// static PseudoAdaptiveMapper mapper;
static Pseudo ps;

int Process_One_Frame()
{
    /* ----- Section 1 : Color ----- */

    uint8_t* y = shm_out_yuv;
    uint8_t* uv = y + v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height;

    ps(algo_in, y, uv, v4l2_ir_dvp_valid_width, v4l2_ir_dvp_valid_height);

    /* ----- Section 2 : Temp ----- */

#if 0
    // 寻找数据范围（用于float输出）
    uint16_t min_val = 65535;
    uint16_t max_val = 0;
    for (int i = 0; i < v4l2_ir_dvp_valid_height; i++)
    {
        for (int j = 0; j < v4l2_ir_dvp_valid_width; j++)
        {
            uint16_t val = algo_in[i * v4l2_ir_dvp_valid_width + j];
            if (val > max_val)
                max_val = val;
            if (val < min_val)
                min_val = val;
        }
    }

    float scale = 1.0f / (max_val - min_val);
    for (int i = 0; i < v4l2_ir_dvp_valid_height; i++)
    {
        for (int j = 0; j < v4l2_ir_dvp_valid_width; j++)
        {
            uint16_t val = algo_in[i * v4l2_ir_dvp_valid_width + j];
            shm_out_float[i * v4l2_ir_dvp_valid_width + j] = (float)(val - min_val) * scale;
        }
    }
#endif

    return 0;
}