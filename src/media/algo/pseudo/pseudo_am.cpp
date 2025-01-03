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

void PseudoAdaptiveMapper::UpdateRange(uint16_t* input, int width, int height)
{
    uint16_t frame_min, frame_max;
    CalculateFrameStats(input, width * height, frame_min, frame_max);

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