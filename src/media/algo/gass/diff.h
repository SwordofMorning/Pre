#pragma once

#include <iostream>
#include <vector>
#include <stdint.h>
#include <opencv2/opencv.hpp>
#include "../../../config/config.h"

class Diff
{
private:
    // checksum of last frame.
    uint16_t m_checksum;

    /**
     * @brief Calculate checksum for first 10 pixels
     * 
     * @param data Input data
     * @return uint16_t Calculated checksum
     */
    uint16_t Checksum(uint16_t* data);

    uint16_t quick_select(std::vector<uint16_t>& arr, int left, int right, int k);

    int partition(std::vector<uint16_t>& arr, int left, int right, int pivot_idx);

public:
    Diff();

    /**
     * @brief Process frame difference
     * 
     * @param input Input raw data (uint16_t)
     * @param output Output difference data (uint16_t)
     * @param width Image width
     * @param height Image height
     * @param rate Difference rate (0.0-1.0)
     * @return true if frame processed, false if duplicate frame
     */
    bool Process_Raw(uint16_t* input, uint16_t* output, size_t width, size_t height, float rate);

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
     * @return true if frame processed, false if duplicate frame
     */
    bool Process_Raw_Stats(uint16_t* input, uint16_t* output, size_t width, size_t height, float rate, float percentile_min = 0.02f, float percentile_max = 0.98f);

    /**
     * @brief Process frame difference with statistics via OpenCV
     * 
     * @param input Input raw data (uint16_t)
     * @param output Output difference data (uint16_t)
     * @param width Image width
     * @param height Image height
     * @param rate Difference rate (0.0-1.0)
     * @param percentile_min Min percentile threshold (0.0-1.0)
     * @param percentile_max Max percentile threshold (0.0-1.0)
     * @return true if frame processed, false if duplicate frame
     */
    bool Process_Raw_Stats_CV(uint16_t* input, uint16_t* output, size_t width, size_t height, float rate, float percentile_min = 0.02f, float percentile_max = 0.98f);

    /**
     * @brief Process frame difference with statistics in vague mode
     * 
     * @param input Input raw data (uint16_t)
     * @param output Output difference data (uint16_t)
     * @param width Image width
     * @param height Image height
     * @param rate Difference rate (0.0-1.0)
     * @param percentile_min Min percentile threshold (0.0-1.0)
     * @param percentile_max Max percentile threshold (0.0-1.0)
     * @return true if frame processed, false if duplicate frame
     */
    bool Process_Raw_Stats_Vague(uint16_t* input, uint16_t* output, size_t width, size_t height, float rate, float percentile_min = 0.02f, float percentile_max = 0.98f);


    /**
     * @brief Process frame difference with statistics in vague mode via OpenCV
     * 
     * @param input Input raw data (uint16_t)
     * @param output Output difference data (uint16_t)
     * @param width Image width
     * @param height Image height
     * @param rate Difference rate (0.0-1.0)
     * @param percentile_min Min percentile threshold (0.0-1.0)
     * @param percentile_max Max percentile threshold (0.0-1.0)
     * @return true if frame processed, false if duplicate frame
     */
    bool Process_Raw_Stats_CV_Vague(uint16_t* input, uint16_t* output, size_t width, size_t height, float rate, float percentile_min = 0.02f, float percentile_max = 0.98f);
};