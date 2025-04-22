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

class Vignetting
{
private:
    uint16_t m_background[640 * 512] = {0};
    pthread_mutex_t m_mutex;

public:
    Vignetting()
    {
        // 初始化互斥锁
        pthread_mutex_init(&m_mutex, NULL);
    }

    ~Vignetting()
    {
        // 销毁互斥锁
        pthread_mutex_destroy(&m_mutex);
    }

    void Save(uint16_t* background, size_t width, size_t height)
    {
        if (!background || width * height > 640 * 512)
            return;

        pthread_mutex_lock(&m_mutex);
        std::memcpy(m_background, background, width * height * sizeof(uint16_t));
        pthread_mutex_unlock(&m_mutex);
    }

    void Cut(uint16_t* input, uint16_t* output, size_t width, size_t height)
    {
        if (!input || !output || width * height > 640 * 512)
            return;

        pthread_mutex_lock(&m_mutex);
        
        // 逐像素处理
        for (size_t i = 0; i < width * height; ++i)
        {
            // 计算差值的绝对值
            output[i] = (input[i] >= m_background[i]) ? 
                       (input[i] - m_background[i]) : 
                       (m_background[i] - input[i]);
        }
        
        pthread_mutex_unlock(&m_mutex);
    }

    // 禁用拷贝构造和赋值操作符
    Vignetting(const Vignetting&) = delete;
    Vignetting& operator=(const Vignetting&) = delete;
};

extern Vignetting vignetting;