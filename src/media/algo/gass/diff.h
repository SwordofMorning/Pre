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

        uint16_t quick_select(std::vector<uint16_t>& arr, int left, int right, int k) {
        if (left == right)
            return arr[left];

        int pivot_idx = left + (right - left) / 2;
        pivot_idx = partition(arr, left, right, pivot_idx);

        if (k == pivot_idx)
            return arr[k];
        else if (k < pivot_idx)
            return quick_select(arr, left, pivot_idx - 1, k);
        else
            return quick_select(arr, pivot_idx + 1, right, k);
    }

    int partition(std::vector<uint16_t>& arr, int left, int right, int pivot_idx) {
        uint16_t pivot_value = arr[pivot_idx];
        std::swap(arr[pivot_idx], arr[right]);
        int store_idx = left;

        for (int i = left; i < right; i++) {
            if (arr[i] < pivot_value) {
                std::swap(arr[store_idx], arr[i]);
                store_idx++;
            }
        }
        std::swap(arr[right], arr[store_idx]);
        return store_idx;
    }

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

    bool Process_Raw_Stats_CV(uint16_t* input, uint16_t* output, size_t width, size_t height, float rate, float percentile_min = 0.02f, float percentile_max = 0.98f);

    bool Process_Raw_Stats_Vague(uint16_t* input, uint16_t* output, size_t width, size_t height, float rate, float percentile_min = 0.02f, float percentile_max = 0.98f);

    bool Process_Raw_Stats_CV_Vague(uint16_t* input, uint16_t* output, size_t width, size_t height, float rate, float percentile_min = 0.02f, float percentile_max = 0.98f);
};