#include "algo.h"

static Diff diff;
static Pseudo pseudo;
static Filter filter;
uint16_t g_diff_result[640 * 512] = {0};

int Process_One_Frame()
{
    /* ============================= */
    /* ===== Section 1 : Color ===== */
    /* ============================= */

    uint8_t* y = shm_out_yuv;
    uint8_t* uv = y + v4l2_ir_dvp_valid_width * v4l2_ir_dvp_valid_height;

    /* ----- Par 1 : Diff ----- */

#if __SHOW_TIME_CONSUME__
    struct timespec start_diff, end_diff;
    clock_gettime(CLOCK_MONOTONIC, &start_diff);
#endif

    if (usr.gas_enhancement_software)
    {
        if (!diff.Process_Raw_Stats(algo_in, g_diff_result, v4l2_ir_dvp_valid_width, v4l2_ir_dvp_valid_height, 0.98))
            return -1;
        filter.Median_16U(g_diff_result, v4l2_ir_dvp_valid_width, v4l2_ir_dvp_valid_height, 3);
    }

#if __SHOW_TIME_CONSUME__
    clock_gettime(CLOCK_MONOTONIC, &end_diff);
    double diff_time_ms = ((end_diff.tv_sec - start_diff.tv_sec) * 1e9 + (end_diff.tv_nsec - start_diff.tv_nsec)) / 1e6;

    struct timespec start_pseudo, end_pseudo;
    clock_gettime(CLOCK_MONOTONIC, &start_pseudo);
#endif

    /* ----- Par 2 : Pseudo Color ----- */

    if (usr.gas_enhancement_software)
        pseudo(g_diff_result, y, uv, v4l2_ir_dvp_valid_width, v4l2_ir_dvp_valid_height);
    else
        pseudo(algo_in, y, uv, v4l2_ir_dvp_valid_width, v4l2_ir_dvp_valid_height);

#if __SHOW_TIME_CONSUME__
    clock_gettime(CLOCK_MONOTONIC, &end_pseudo);
    double pseudo_time_ms = ((end_pseudo.tv_sec - start_pseudo.tv_sec) * 1e9 + (end_pseudo.tv_nsec - start_pseudo.tv_nsec)) / 1e6;

    struct timespec start_filter, end_filter;
    clock_gettime(CLOCK_MONOTONIC, &start_filter);
#endif

    /* ----- Par 3 : Filter ----- */

    if (usr.mean_filter)
        filter.Mean_NV12(y, v4l2_ir_dvp_valid_width, v4l2_ir_dvp_valid_height, 3);

#if __SHOW_TIME_CONSUME__
    clock_gettime(CLOCK_MONOTONIC, &end_filter);

    double filter_time_ms = ((end_filter.tv_sec - start_filter.tv_sec) * 1e9 + (end_filter.tv_nsec - start_filter.tv_nsec)) / 1e6;
    double total_time_ms = diff_time_ms + pseudo_time_ms + pseudo_time_ms;
    printf("Processing Time - Diff: %.2f ms, Pseudo: %.2f ms, Filter: %.2f ms, Total: %.2f ms\n", diff_time_ms, pseudo_time_ms, filter_time_ms, total_time_ms);
#endif

    /* =================================== */
    /* ===== Section 2 : Temperature ===== */
    /* =================================== */

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

    return 0;
}