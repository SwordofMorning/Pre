#include "af.h"

AF::AF(const int& p_iw, const int& p_ih, const int& p_steps, Motor& p_motor)
    : m_image_width(p_iw)
    , m_image_height(p_ih)
    , m_motor_total_steps(p_steps)
    , m_motor(p_motor)
{
    // do nothing
}