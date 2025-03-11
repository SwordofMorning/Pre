#include "pseudo_am.h"

PseudoAdaptiveMapper PAM_mapper;

PseudoAdaptiveMapper::PseudoAdaptiveMapper()
    : smoothed_min(0)
    , smoothed_max(65535)
    , initialized(false)
{
    buffer.reserve(1920 * 1080 / SAMPLE_STRIDE);
}

int partition(uint16_t* arr, int left, int right, int pivotIndex)
{
    uint16_t pivotValue = arr[pivotIndex];
    std::swap(arr[pivotIndex], arr[right]);
    int storeIndex = left;

    for (int i = left; i < right; i++)
    {
        if (arr[i] <= pivotValue)
        {
            std::swap(arr[storeIndex], arr[i]);
            storeIndex++;
        }
    }
    std::swap(arr[right], arr[storeIndex]);
    return storeIndex;
}

uint16_t quickSelect(uint16_t* arr, int left, int right, int k)
{
    if (left == right)
        return arr[left];
    int pivotIndex = left + (right - left) / 2;
    pivotIndex = partition(arr, left, right, pivotIndex);

    if (k == pivotIndex)
        return arr[k];
    else if (k < pivotIndex)
        return quickSelect(arr, left, pivotIndex - 1, k);
    else
        return quickSelect(arr, pivotIndex + 1, right, k);
}

void PseudoAdaptiveMapper::CalculateFrameStats(uint16_t* input, int size, uint16_t& min_val, uint16_t& max_val)
{
    int sample_size = size / SAMPLE_STRIDE;
    buffer.resize(sample_size);

    for (int i = 0; i < sample_size; i++)
    {
        buffer[i] = input[i * SAMPLE_STRIDE];
    }

    int lower_pos = sample_size * 0.01;
    int upper_pos = sample_size * 0.99;

    min_val = quickSelect(buffer.data(), 0, sample_size - 1, lower_pos);
    max_val = quickSelect(buffer.data(), 0, sample_size - 1, upper_pos);
}

void PseudoAdaptiveMapper::CalculateFrameStats_HIST(uint16_t* input, int size, uint16_t& min_val, uint16_t& max_val)
{
    // Initialize counter and temporary array
    const int HIST_SIZE = 65536;
    const int BIN_SIZE = 256;
    const int SAMPLES_PER_BIN = HIST_SIZE / BIN_SIZE;

    // Histogram array
    int histogram[BIN_SIZE] = {0};
    int total_samples = 0;

    /* Loop 1: Create a histogram */
    for (int i = 0; i < size; i += SAMPLE_STRIDE)
    {
        uint16_t value = input[i];
        int bin = value / SAMPLES_PER_BIN;
        // Make sure not to cross the boundary
        bin = std::min(bin, BIN_SIZE - 1);
        histogram[bin]++;
        total_samples++;
    }

    // Calculate target position: 1%
    int target_min = total_samples * 0.01;
    // Calculate target position: 99%
    int target_max = total_samples * 0.99;

    /* Loop 2: Find min */
    int count = 0;
    int min_bin = 0;
    for (int i = 0; i < BIN_SIZE; i++)
    {
        count += histogram[i];
        if (count >= target_min)
        {
            min_bin = i;
            break;
        }
    }

    /* Loop 3: Find max */
    count = 0;
    int max_bin = BIN_SIZE - 1;
    for (int i = BIN_SIZE - 1; i >= 0; i--)
    {
        count += histogram[i];
        if (count >= total_samples - target_max)
        {
            max_bin = i;
            break;
        }
    }

    // Convert bins back to actual values
    min_val = min_bin * SAMPLES_PER_BIN;
    max_val = (max_bin + 1) * SAMPLES_PER_BIN - 1;

    // Refine the search: find a more precise value within a certain interval
    uint16_t refined_min = 65535;
    uint16_t refined_max = 0;

    /* Loop 4: Refine */
    for (int i = 0; i < size; i += SAMPLE_STRIDE)
    {
        uint16_t value = input[i];
        if (value >= min_val && value <= min_val + SAMPLES_PER_BIN)
        {
            refined_min = std::min(refined_min, value);
        }
        if (value >= max_val - SAMPLES_PER_BIN && value <= max_val)
        {
            refined_max = std::max(refined_max, value);
        }
    }

    // Use the refined value
    if (refined_min != 65535)
        min_val = refined_min;
    if (refined_max != 0)
        max_val = refined_max;
}

void PseudoAdaptiveMapper::UpdateRange(uint16_t* input, int width, int height)
{
    uint16_t frame_min, frame_max;
    CalculateFrameStats_HIST(input, width * height, frame_min, frame_max);

    if (!initialized)
    {
        smoothed_min = frame_min;
        smoothed_max = frame_max;
        initialized = true;
    }
    else
    {
        smoothed_min = (1 - ALPHA) * smoothed_min + ALPHA * frame_min;
        smoothed_max = (1 - ALPHA) * smoothed_max + ALPHA * frame_max;

        if (smoothed_max - smoothed_min < MIN_RANGE)
        {
            float center = (smoothed_max + smoothed_min) / 2;
            smoothed_min = center - MIN_RANGE / 2;
            smoothed_max = center + MIN_RANGE / 2;
        }
    }
}

float PseudoAdaptiveMapper::GetScale() const
{
    return 255.0f / (smoothed_max - smoothed_min);
}

float PseudoAdaptiveMapper::GetMin() const
{
    return smoothed_min;
}

float PseudoAdaptiveMapper::GetMax() const
{
    return smoothed_max;
}