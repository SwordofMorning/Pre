#pragma once

#include "uartclass.h"

class Motor : public UART
{
private:
    // int m_step_ir_max;
    // int m_step_ir_cur;
    // int m_step_vis_focus_max;
    // int m_step_vis_focus_cur;
    // int m_step_vis_zoom_max;
    // int m_step_vis_zoom_cur;
    // bool m_shutter;

    uint8_t m_dev_ir;
    uint8_t m_dev_vis_zoom;
    uint8_t m_dev_vis_focus;

    /**
     * @brief XOR for data.
     * @return checksum
     */
    uint8_t Calculate_Checksum(const std::vector<uint8_t>& data, size_t length);

public:
    Motor();

    int Move(uint8_t dev, int32_t steps);
    int Move_IR(int32_t steps);
    int Move_Vis_Zoom(int32_t steps);
    int Move_Vis_Focus(int32_t steps);
};