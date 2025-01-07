#include "filter.h"

void Filter::Mean_NV12(uint8_t* yuv, size_t width, size_t height, size_t window_size)
{
    size_t y_size = width * height;

    std::vector<uint8_t> temp_y(y_size);

    // Only on Y
    FilterCL_ProcessMean(&filter_cl, yuv, temp_y.data(), width, height, window_size);

    std::memcpy(yuv, temp_y.data(), y_size);
}

void Filter::Bilateral_NV12(uint8_t* yuv, size_t width, size_t height, float sigma_space, float sigma_range)
{
    size_t y_size = width * height;
    std::vector<uint8_t> temp_y(y_size);

    // Only on Y
    FilterCL_ProcessBilateral(&filter_cl, yuv, temp_y.data(), width, height, sigma_space, sigma_range);

    std::memcpy(yuv, temp_y.data(), y_size);
}