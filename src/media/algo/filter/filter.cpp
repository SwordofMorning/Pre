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

bool Filter::Median_16U(uint16_t* data, size_t width, size_t height, int ksize)
{
    if (!data || width == 0 || height == 0 || ksize % 2 == 0)
        return false;

    cv::Mat src(height, width, CV_16U, data);
    cv::Mat dst(height, width, CV_16U);
    cv::medianBlur(src, dst, ksize);

    memcpy(data, dst.data, width * height * sizeof(uint16_t));

    return true;
}