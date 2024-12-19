#include "motor.h"

Motor::Motor()
    : UART("/dev/ttyS6", 115200, 1024)
{
    m_dev_ir = 0x02;
    m_dev_vis_zoom = 0x03;
    m_dev_vis_focus = 0x04;
}

uint8_t Motor::Calculate_Checksum(const std::vector<uint8_t>& data, size_t length)
{
    uint8_t checksum = 0;
    for (size_t i = 0; i < length; ++i)
        checksum ^= data[i];
    return checksum;
}

int Motor::Move(uint8_t dev, int32_t steps)
{
    // clang-format off
    std::vector<uint8_t> cmd = {
        // label
        0x24,
        // device
        dev,
        // command length
        0x04, 0x00,
        // steps
        0x00, 0x00, 0x00, 0x00,
    };
    // clang-format on

    // Convert steps to little-endian byte order
    cmd[4] = static_cast<uint8_t>(steps & 0xFF);
    cmd[5] = static_cast<uint8_t>((steps >> 8) & 0xFF);
    cmd[6] = static_cast<uint8_t>((steps >> 16) & 0xFF);
    cmd[7] = static_cast<uint8_t>((steps >> 24) & 0xFF);
    cmd.push_back(Calculate_Checksum(cmd, cmd.size()));

    for (auto i : cmd)
    {
        printf("[%x]", i);
    }

    this->Send(cmd);

    return 0;
}

int Motor::Move_IR(int32_t steps)
{
    return Move(m_dev_ir, steps);
}

int Motor::Move_Vis_Zoom(int32_t steps)
{
    return Move(m_dev_vis_zoom, steps);
}

int Motor::Move_Vis_Focus(int32_t steps)
{
    return Move(m_dev_vis_focus, steps);
}