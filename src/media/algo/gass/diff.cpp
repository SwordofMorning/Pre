#include "diff.h"
#include <algorithm>
#include <cstring>

void Diff::Process_Raw(uint16_t* input, uint16_t* output, size_t width, size_t height, float rate)
{
    DiffCL_Process(&diff_cl, input, output, width, height, rate);
}

void Diff::Process_Raw_Stats(uint16_t* input, uint16_t* output, size_t width, size_t height, float rate, float percentile_min, float percentile_max)
{
    size_t total_size = width * height;
    std::vector<uint16_t> temp_diff(total_size);

    // 1. 计算差分
    DiffCL_Process(&diff_cl, input, temp_diff.data(), width, height, rate);

    // 2. 计算阈值
    std::vector<uint16_t> sorted_diff = temp_diff;
    std::sort(sorted_diff.begin(), sorted_diff.end());

    size_t min_idx = size_t(total_size * percentile_min);
    size_t max_idx = size_t(total_size * percentile_max);
    uint16_t min_val = sorted_diff[min_idx];
    uint16_t max_val = sorted_diff[max_idx];

    // 3. 应用阈值
    for (size_t i = 0; i < total_size; i++)
    {
        if (temp_diff[i] < min_val)
            temp_diff[i] = min_val;
        if (temp_diff[i] > max_val)
            temp_diff[i] = max_val;

        // 归一化到16位范围
        output[i] = uint16_t((float)(temp_diff[i] - min_val) * 65535.0f / (max_val - min_val));
    }
}