#pragma once

#include<string>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "../../../config/config.h"
#include "../../../utils/uart/motor.h"

class AF
{
protected:
    int m_image_width;
    int m_image_height;
    int m_motor_total_steps;

    Motor& m_motor;
    
    /**
     * @brief Preprocess image data, from raw to 8bit grey.
     * 
     * @return cv::Mat 8bit image mat.
     */
    virtual cv::Mat Create_8bit_Mat() = 0;

public:
    AF(const int& p_iw, const int& p_ih, const int& p_steps, Motor& p_motor);

    /**
     * @brief Auto focus.
     * 
     * @param x Focus point x-axis coordinate.
     * @param y Focus point y-axis coordinate.
     */
    virtual void Focus(const int& x, const int& y) = 0;
};