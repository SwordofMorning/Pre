#include "diff.h"
#include <algorithm>
#include <cstring>

Diff::Diff()
    : m_checksum(0)
{
    // do nothing
}

uint16_t Diff::Checksum(uint16_t* data)
{
    uint16_t sum = 0;
    for (int i = 0; i < 10; ++i)
    {
        sum += data[i];
    }
    return sum;
}

bool Diff::Process_Raw(uint16_t* input, uint16_t* output, size_t width, size_t height, float rate)
{
    /*
    uint16_t current_checksum = this->Checksum(input);
    if (current_checksum == m_checksum)
        return false;
    m_checksum = current_checksum;
    */

    DiffCL_Diff(&diff_cl, input, output, width, height, rate);
    return true;
}

bool Diff::Process_Raw_Stats(uint16_t* input, uint16_t* output, size_t width, size_t height, float rate, float percentile_min, float percentile_max)
{
    /*
    uint16_t current_checksum = this->Checksum(input);
    if (current_checksum == m_checksum)
        return false;
    m_checksum = current_checksum;
    */

    size_t total_size = width * height;
    std::vector<uint16_t> temp_diff(total_size);

    // 1. 计算差分
    DiffCL_Diff(&diff_cl, input, temp_diff.data(), width, height, rate);

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

    return true;
}

bool Diff::Process_Raw_Stats_CV(uint16_t* input, uint16_t* output, size_t width, size_t height, float rate, float percentile_min, float percentile_max)
{
    /*
    uint16_t current_checksum = this->Checksum(input);
    if (current_checksum == m_checksum)
        return false;
    m_checksum = current_checksum;
    */

    size_t total_size = width * height;
    std::vector<uint16_t> temp_diff(total_size);

    // 1. 计算差分
    DiffCL_Diff(&diff_cl, input, temp_diff.data(), width, height, rate);

    // 2. 转换为OpenCV矩阵
    cv::Mat diff_mat(height, width, CV_16U, temp_diff.data());
    cv::Mat result_mat(height, width, CV_16U, output);

    // 3. 计算分位数
    double min_val, max_val;
    cv::Mat sorted;
    cv::sort(diff_mat.reshape(1, 1), sorted, cv::SORT_ASCENDING);
    min_val = sorted.at<uint16_t>(total_size * percentile_min);
    max_val = sorted.at<uint16_t>(total_size * percentile_max);

    // 4. 应用阈值（限制范围）
    cv::threshold(diff_mat, diff_mat, min_val, min_val, cv::THRESH_TOZERO);
    cv::threshold(diff_mat, diff_mat, max_val, max_val, cv::THRESH_TRUNC);

    // 5. 复制结果到输出
    diff_mat.copyTo(result_mat);

    return true;
}

bool Diff::Process_Raw_Stats_Vague(uint16_t* input, uint16_t* output, size_t width, size_t height, float rate, float percentile_min, float percentile_max)
{
    size_t total_size = width * height;
    std::vector<uint16_t> temp_diff(total_size);

    // 1. 计算差分
    DiffCL_Diff(&diff_cl, input, temp_diff.data(), width, height, rate);

    // 2. 降采样并查找阈值
    const int sample_step = 4;
    std::vector<uint16_t> sampled_diff;
    sampled_diff.reserve(total_size / (sample_step * sample_step));

    // 降采样
    for (size_t y = 0; y < height; y += sample_step)
        for (size_t x = 0; x < width; x += sample_step)
            sampled_diff.push_back(temp_diff[y * width + x]);

    size_t sampled_size = sampled_diff.size();
    size_t min_idx = size_t(sampled_size * percentile_min);
    size_t max_idx = size_t(sampled_size * percentile_max);

    // 使用快速选择算法找到百分位值
    uint16_t min_val = quick_select(sampled_diff, 0, sampled_size - 1, min_idx);
    uint16_t max_val = quick_select(sampled_diff, 0, sampled_size - 1, max_idx);

    // 3. 应用阈值
    for (size_t i = 0; i < total_size; i++)
    {
        uint16_t val = temp_diff[i];
        val = std::clamp(val, min_val, max_val);
        output[i] = uint16_t((float)(val - min_val) * 65535.0f / (max_val - min_val));
    }

    return true;
}

bool Diff::Process_Raw_Stats_CV_Vague(uint16_t* input, uint16_t* output, size_t width, size_t height, float rate, float percentile_min, float percentile_max)
{
    size_t total_size = width * height;
    std::vector<uint16_t> temp_diff(total_size);

    // 1. 计算差分
    DiffCL_Diff(&diff_cl, input, temp_diff.data(), width, height, rate);

    // 2. 转换为OpenCV矩阵
    cv::Mat diff_mat(height, width, CV_16U, temp_diff.data());
    cv::Mat result_mat(height, width, CV_16U, output);

    // 2. 降采样并查找阈值
    const int sample_step = 4;
    std::vector<uint16_t> sampled_diff;
    sampled_diff.reserve(total_size / (sample_step * sample_step));

    // 降采样
    for (size_t y = 0; y < height; y += sample_step)
    {
        for (size_t x = 0; x < width; x += sample_step)
        {
            sampled_diff.push_back(temp_diff[y * width + x]);
        }
    }

    size_t sampled_size = sampled_diff.size();
    size_t min_idx = size_t(sampled_size * percentile_min);
    size_t max_idx = size_t(sampled_size * percentile_max);

    // 使用快速选择算法找到百分位值
    uint16_t min_val = quick_select(sampled_diff, 0, sampled_size - 1, min_idx);
    uint16_t max_val = quick_select(sampled_diff, 0, sampled_size - 1, max_idx);

    // 4. 应用阈值（限制范围）
    cv::threshold(diff_mat, diff_mat, min_val, min_val, cv::THRESH_TOZERO);
    cv::threshold(diff_mat, diff_mat, max_val, max_val, cv::THRESH_TRUNC);

    // 5. 复制结果到输出
    diff_mat.copyTo(result_mat);

    return true;
}