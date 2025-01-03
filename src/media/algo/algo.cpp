#include "algo.h"

static Pseudo ps;
static Filter fl;

int Process_One_Frame()
{
    /* ----- Section 1 : Color ----- */

    uint8_t* y = shm_out_yuv;
    uint8_t* uv = y + v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height;

#if __SHOW_TIME_CONSUME__
    struct timespec start_pseudo, end_pseudo;
    clock_gettime(CLOCK_MONOTONIC, &start_pseudo);
#endif

    ps(algo_in, y, uv, v4l2_ir_dvp_valid_width, v4l2_ir_dvp_valid_height);

#if __SHOW_TIME_CONSUME__
    clock_gettime(CLOCK_MONOTONIC, &end_pseudo);
    double pseudo_time_ms = ((end_pseudo.tv_sec - start_pseudo.tv_sec) * 1e9 + (end_pseudo.tv_nsec - start_pseudo.tv_nsec)) / 1e6;

    struct timespec start_filter, end_filter;
    clock_gettime(CLOCK_MONOTONIC, &start_filter);
#endif

    fl.Mean_NV12(y, v4l2_ir_dvp_valid_width, v4l2_ir_dvp_valid_height, 5);

#if __SHOW_TIME_CONSUME__
    clock_gettime(CLOCK_MONOTONIC, &end_filter);
    double filter_time_ms = ((end_filter.tv_sec - start_filter.tv_sec) * 1e9 + (end_filter.tv_nsec - start_filter.tv_nsec)) / 1e6;

    printf("Processing Time - Pseudo: %.2f ms, Filter: %.2f ms, Total: %.2f ms\n",
           pseudo_time_ms, filter_time_ms, pseudo_time_ms + filter_time_ms);
#endif

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