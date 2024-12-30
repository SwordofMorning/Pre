#pragma once

#include "af.h"

class AF_IR : public AF
{
private:
    int m_step_length;

    /**
     * @brief Preprocess Image data.
     * 
     * @note 1. read IR raw data;
     * @note 2. create u16 mat;
     * @note 3. median filter;
     * @note 4. normalize.
     * 
     * @return cv::Mat 8bit IR mat.
     */
    virtual cv::Mat Create_8bit_Mat() override;

public:
    AF_IR(const int& p_iw, const int& p_ih, const int& p_steps, Motor& p_motor);

    /**
     * @brief Auto focus.
     * 
     * @param x Focus point x-axis coordinate.
     * @param y Focus point y-axis coordinate.
     */
    virtual void Focus(const int& x, const int& y) override;
};