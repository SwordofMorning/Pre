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
#include "../../config/lut/lut.h"
#include "../../config/config.h"
#include "../vo/vo_gst.h"

class AdaptiveMapper
{
private:
    static constexpr int HISTORY_SIZE = 30;  // 保持30帧的历史记录
    static constexpr float ALPHA = 0.1f;     // 平滑系数
    static constexpr int MIN_RANGE = 1000;   // 最小映射范围
    
    // 历史数据
    float smoothed_min;
    float smoothed_max;
    bool initialized;

    // 辅助函数：计算当前帧的统计信息
    void calculateFrameStats(uint16_t* input, int size, uint16_t& min_val, uint16_t& max_val) {
        // 忽略极值点，使用百分位数
        std::vector<uint16_t> sorted_data(input, input + size);
        std::sort(sorted_data.begin(), sorted_data.end());
        
        // 使用1%和99%的百分位数作为范围
        int lower_idx = size * 0.01;
        int upper_idx = size * 0.99;
        
        min_val = sorted_data[lower_idx];
        max_val = sorted_data[upper_idx];
    }

public:
    AdaptiveMapper() : smoothed_min(0), smoothed_max(65535), initialized(false) {}

    void updateRange(uint16_t* input, int width, int height) {
        uint16_t frame_min, frame_max;
        calculateFrameStats(input, width * height, frame_min, frame_max);

        if (!initialized) {
            smoothed_min = frame_min;
            smoothed_max = frame_max;
            initialized = true;
        } else {
            // 指数平滑更新
            smoothed_min = (1 - ALPHA) * smoothed_min + ALPHA * frame_min;
            smoothed_max = (1 - ALPHA) * smoothed_max + ALPHA * frame_max;
            
            // 确保最小映射范围
            if (smoothed_max - smoothed_min < MIN_RANGE) {
                float center = (smoothed_max + smoothed_min) / 2;
                smoothed_min = center - MIN_RANGE / 2;
                smoothed_max = center + MIN_RANGE / 2;
            }
        }
    }

    float getScale() const {
        return 255.0f / (smoothed_max - smoothed_min);
    }

    float getMin() const { return smoothed_min; }
    float getMax() const { return smoothed_max; }
};