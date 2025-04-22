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
#include <vector>
#include <opencv2/opencv.hpp>
#include "../../../config/config.h"

class Vignetting
{
private:
    uint16_t m_background[640 * 512] = {0};
    pthread_mutex_t m_mutex;

    uint16_t m_background_rad[640 * 512] = {0};

    void RadialSmooth(uint16_t* data, size_t width, size_t height)
    {
        // 计算图像中心
        float center_x = width / 2.0f;
        float center_y = height / 2.0f;
        
        // 计算最大半径（图像中心到角落的距离）
        float max_radius = std::sqrt(center_x * center_x + center_y * center_y);
        
        // 创建径向距离数组和累加器
        const int RADIUS_BINS = 128;      // 滤波分区数量
        const int SMOOTH_WINDOW = 67;    // 滤波相邻区数量

        std::vector<float> radius_sum(RADIUS_BINS, 0);
        std::vector<int> radius_count(RADIUS_BINS, 0);
        
        // 第一遍：计算每个径向区间的平均值
        for (size_t y = 0; y < height; ++y)
        {
            for (size_t x = 0; x < width; ++x)
            {
                // 计算到中心的距离
                float dx = x - center_x;
                float dy = y - center_y;
                float radius = std::sqrt(dx * dx + dy * dy);
                
                // 计算半径所属的区间
                int bin = int(radius * RADIUS_BINS / max_radius);
                if (bin >= RADIUS_BINS) bin = RADIUS_BINS - 1;
                
                // 累加值和计数
                radius_sum[bin] += data[y * width + x];
                radius_count[bin]++;
            }
        }
        
        // 计算每个区间的平均值
        std::vector<float> radius_avg(RADIUS_BINS);
        for (int i = 0; i < RADIUS_BINS; ++i)
        {
            radius_avg[i] = radius_count[i] > 0 ? 
                           radius_sum[i] / radius_count[i] : 0;
        }
        
        // 对半径平均值进行平滑处理
        std::vector<float> smoothed_avg = radius_avg;
        for (int i = SMOOTH_WINDOW; i < RADIUS_BINS - SMOOTH_WINDOW; ++i)
        {
            float sum = 0;
            for (int j = -SMOOTH_WINDOW; j <= SMOOTH_WINDOW; ++j)
            {
                sum += radius_avg[i + j];
            }
            smoothed_avg[i] = sum / (2 * SMOOTH_WINDOW + 1);
        }
        
        // 第二遍：应用平滑后的值
        for (size_t y = 0; y < height; ++y)
        {
            for (size_t x = 0; x < width; ++x)
            {
                float dx = x - center_x;
                float dy = y - center_y;
                float radius = std::sqrt(dx * dx + dy * dy);
                
                int bin = int(radius * RADIUS_BINS / max_radius);
                if (bin >= RADIUS_BINS) bin = RADIUS_BINS - 1;
                
                // 使用线性插值获取平滑值
                float smooth_value = smoothed_avg[bin];
                if (bin < RADIUS_BINS - 1)
                {
                    float t = (radius * RADIUS_BINS / max_radius) - bin;
                    smooth_value = (1 - t) * smoothed_avg[bin] + 
                                 t * smoothed_avg[bin + 1];
                }
                
                data[y * width + x] = static_cast<uint16_t>(smooth_value);
            }
        }
    }

public:
    Vignetting()
    {
        pthread_mutex_init(&m_mutex, NULL);
    }

    ~Vignetting()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    void Save(uint16_t* background, size_t width, size_t height)
    {
        if (!background || width * height > 640 * 512)
            return;

        pthread_mutex_lock(&m_mutex);
        std::memcpy(m_background, background, width * height * sizeof(uint16_t));
        
        // 对背景进行径向平滑
        RadialSmooth(m_background, width, height);
        
        pthread_mutex_unlock(&m_mutex);
    }

    void Cut(uint16_t* input, uint16_t* output, size_t width, size_t height)
    {
        if (!input || !output || width * height > 640 * 512)
            return;

        pthread_mutex_lock(&m_mutex);
        
        for (size_t i = 0; i < width * height; ++i)
        {
            output[i] = (input[i] >= m_background[i]) ? 
                       (input[i] - m_background[i]) : 
                       (m_background[i] - input[i]);
        }
        
        pthread_mutex_unlock(&m_mutex);
    }

    Vignetting(const Vignetting&) = delete;
    Vignetting& operator=(const Vignetting&) = delete;
};

extern Vignetting vignetting;