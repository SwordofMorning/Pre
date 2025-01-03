#pragma once

/**
 * @file pseudo_am.h
 * @author Xiaojintao
 * @brief Pseudo Adaptive Mapper.
 * @version 0.1
 * @date 2024-12-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */

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
#include "../../../config/config.h"

class PseudoAdaptiveMapper
{
private:
    static constexpr int HISTORY_SIZE = 30;
    static constexpr float ALPHA = 0.1f;
    static constexpr int MIN_RANGE = 1000;
    static constexpr int SAMPLE_STRIDE = 4;

    // History data
    float smoothed_min;
    float smoothed_max;
    bool initialized;
    std::vector<uint16_t> buffer;

    // Auxiliary function: calculate statistics for the current frame
    void CalculateFrameStats(uint16_t* input, int size, uint16_t& min_val, uint16_t& max_val);

public:
    PseudoAdaptiveMapper();

    void UpdateRange(uint16_t* input, int width, int height);

    float GetScale() const;
    float GetMin() const;
    float GetMax() const;
};

extern PseudoAdaptiveMapper PAM_mapper;