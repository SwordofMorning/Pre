#include "af_ir.h"

AF_IR::AF_IR(const int& p_iw, const int& p_ih, const int& p_steps, Motor& p_motor)
    : AF(p_iw, p_ih, p_steps, p_motor)
{
    m_step_length = 20;
}

cv::Mat AF_IR::Create_8bit_Mat()
{
    cv::Mat mat_16bit(m_image_height, m_image_width, CV_16U, algo_in);
    cv::Mat mat_8bit(m_image_height, m_image_width, CV_8U);

    cv::medianBlur(mat_16bit, mat_16bit, 3);
    cv::normalize(mat_16bit, mat_8bit, 0, 255, cv::NORM_MINMAX, CV_8U);

    return mat_8bit;
}

void AF_IR::Focus(const int& x, const int& y)
{
    std::cout << "1" << std::endl;

    /* Step 1 : Move motor to position 0 */
    m_motor.Move_IR_Start(Motor::Direction::BACKWARD);
    while (m_motor.Get_Step_IR_Cur() != 0)
    {
        std::cout << "cur: " << m_motor.Get_Step_IR_Cur() << std::endl;
        usleep(m_step_length * 1000);
    }

    std::cout << "2" << std::endl;

    /* Step 2 : Move motor to max position, and find clarity point */
    int32_t old_pos = 0;
    while (1)
    {
        m_motor.Move_IR(m_step_length);
        std::cout << "cur: " << m_motor.Get_Step_IR_Cur() << std::endl;
        usleep(m_step_length * 1000);
        if (old_pos == m_motor.Get_Step_IR_Cur() && old_pos > 2000)
            break;
        old_pos = m_motor.Get_Step_IR_Cur();
    }

    std::cout << "3" << std::endl;

    return;
}