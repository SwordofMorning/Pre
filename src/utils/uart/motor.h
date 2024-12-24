#pragma once

#include "uartclass.h"
#include "../log/litelog.h"

class Motor : public UART
{
private:
    int32_t m_step_ir_cur;
    int32_t m_step_vis_focus_cur;
    int32_t m_step_vis_zoom_cur;

    uint8_t m_dev_ir;
    uint8_t m_dev_vis_zoom;
    uint8_t m_dev_vis_focus;
    uint8_t m_dev_shutter;

    /**
     * @brief XOR for data.
     * @return checksum
     */
    uint8_t Calculate_Checksum(const std::vector<uint8_t>& data, size_t length);
    uint8_t Calculate_Checksum(const uint8_t* data, size_t length);

    int Move(uint8_t dev, int32_t steps);
    int Shutter(uint8_t operation);

    int Parse_Move(const uint8_t* data, size_t len);
    int Parse_Shutter(const uint8_t* data, size_t len);

public:
    Motor();

    int Move_IR(int32_t steps);
    int Move_Vis_Zoom(int32_t steps);
    int Move_Vis_Focus(int32_t steps);
    int Shutter_Open();
    int Shutter_Close();
};