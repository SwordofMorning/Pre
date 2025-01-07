#pragma once

#include <iostream>
#include <vector>
#include <stdint.h>
#include "../../../config/config.h"

class Diff
{
public:
    /**
     * @brief Process frame difference
     * 
     * @param input Input raw data (uint16_t)
     * @param output Output difference data (uint16_t)
     * @param width Image width
     * @param height Image height
     * @param rate Difference rate (0.0-1.0)
     */
    void Process_Raw(uint16_t* input, uint16_t* output, size_t width, size_t height, float rate);

    /**
     * @brief Process frame difference with statistics
     * 
     * @param input Input raw data (uint16_t)
     * @param output Output difference data (uint16_t)
     * @param width Image width
     * @param height Image height
     * @param rate Difference rate (0.0-1.0)
     * @param percentile_min Min percentile threshold (0.0-1.0)
     * @param percentile_max Max percentile threshold (0.0-1.0)
     */
    void Process_Raw_Stats(uint16_t* input, uint16_t* output, size_t width, size_t height, float rate, float percentile_min = 0.02f, float percentile_max = 0.98f);
};