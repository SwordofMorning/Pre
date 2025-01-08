#pragma once

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <opencv2/opencv.hpp>
#include "../../../config/config.h"

class Filter
{
public:
    /**
     * @brief Mean filter For NV12 image.
     * 
     * @param yuv NV12 data.
     * @param width image width.
     * @param height image height.
     * @param windows_size filter windows size.
     */
    void Mean_NV12(uint8_t* yuv, size_t width, size_t height, size_t windows_size);

    void Bilateral_NV12(uint8_t* yuv, size_t width, size_t height, float sigma_space, float sigma_range);

    /**
     * @brief Median filter for 16-bit image data
     * 
     * @param data Input/Output 16-bit data
     * @param width Image width
     * @param height Image height
     * @param ksize Kernel size (must be odd)
     * @return true if successful, false if invalid parameters
     */
    bool Median_16U(uint16_t* data, size_t width, size_t height, int ksize = 3);
};