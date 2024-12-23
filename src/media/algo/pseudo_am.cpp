#include "pseudo_am.h"

PseudoAdaptiveMapper::PseudoAdaptiveMapper()
    : smoothed_min(0)
    , smoothed_max(65535)
    , initialized(false)
{
    // nothing
}

void PseudoAdaptiveMapper::CalculateFrameStats(uint16_t* input, int size, uint16_t& min_val, uint16_t& max_val)
{
    std::vector<uint16_t> sorted_data(input, input + size);
    std::sort(sorted_data.begin(), sorted_data.end());

    int lower_idx = size * 0.01;
    int upper_idx = size * 0.99;

    min_val = sorted_data[lower_idx];
    max_val = sorted_data[upper_idx];
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