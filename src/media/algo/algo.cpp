#include "algo.h"

static Diff diff;
static Pseudo pseudo;
static Filter filter;
static Temperature_Measurement tm;
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

    struct timespec start_temp, end_temp;
    clock_gettime(CLOCK_MONOTONIC, &start_temp);
#endif

    /* =================================== */
    /* ===== Section 2 : Temperature ===== */
    /* =================================== */

    float a = 4.095005068288752e-09;
    float b = 0.000681535692189997;
    float c = 5.249889753750205;

    tm(algo_in, shm_out_float, v4l2_ir_dvp_valid_width, v4l2_ir_dvp_valid_height, a, b, c);

#if __SHOW_TIME_CONSUME__
    clock_gettime(CLOCK_MONOTONIC, &end_temp);
    double temp_time_ms = ((end_temp.tv_sec - start_temp.tv_sec) * 1e9 + (end_temp.tv_nsec - start_temp.tv_nsec)) / 1e6;
    double total_time_ms = diff_time_ms + pseudo_time_ms + pseudo_time_ms + temp_time_ms;
    // clang-format off
    printf("Processing Time - Diff: %.2f ms, Pseudo: %.2f ms, Filter: %.2f ms, Temp: %.2f, Total: %.2f ms\n", 
        diff_time_ms, pseudo_time_ms, filter_time_ms, temp_time_ms, total_time_ms);
    printf("TM: [%.2f, %.2f %.2f]\n", shm_out_float[640 * 255 + 320], shm_out_float[640 * 255 + 321], shm_out_float[640 * 255 + 322]);
    // clang-format on
#endif
    return 0;
}