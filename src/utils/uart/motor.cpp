#include "motor.h"

Motor::Motor()
    : UART("/dev/ttyS6", 115200, 1024)
    , m_dev_ir(0x02)
    , m_dev_vis_zoom(0x03)
    , m_dev_vis_focus(0x04)
    , m_dev_shutter(0x06)
{
    m_receive_callback = [this](const uint8_t* data, size_t len) -> int {
        if (data[0] != 0x24)
            return -1;

        if (data[1] == 0x02 || data[1] == 0x03 || data[1] == 0x04)
            return this->Parse_Move(data, len);
        else if (data[1] == 0x06)
            return this->Parse_Shutter(data, len);

        return -2;
    };
}

uint8_t Motor::Calculate_Checksum(const std::vector<uint8_t>& data, size_t length)
{
    uint8_t checksum = 0;
    for (size_t i = 0; i < length; ++i)
        checksum ^= data[i];
    return checksum;
}

uint8_t Motor::Calculate_Checksum(const uint8_t* data, size_t length)
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

int Motor::Shutter(uint8_t operation)
{
    // clang-format off
    std::vector<uint8_t> cmd = {
        // label
        0x24,
        // device
        m_dev_shutter,
        // command length
        0x01, 0x00,
        // operation
        operation
    };
    // clang-format on

    cmd.push_back(Calculate_Checksum(cmd, cmd.size()));
    this->Send(cmd);

    return 0;
}

int Motor::Shutter_Open()
{
    return this->Shutter(0x00);
}

int Motor::Shutter_Close()
{
    return this->Shutter(0x02);
}

int Motor::Parse_Move(const uint8_t* data, size_t len)
{
    if (len != 10)
        return -1;
    if (data[0] != 0x24 || data[8] != 0xFE)
        return -2;
    if (data[9] != this->Calculate_Checksum(data, len - 1))
        return -3;

    int32_t tmp = (data[4] << (8 * 0)) | (data[5] << (8 * 1)) | (data[6] << (8 * 2)) | (data[7] << (8 * 3));
    litelog.log.debug("Motor move [%x] with position on: %d", data[1], tmp);

    if (data[1] == 0x02)
        m_step_ir_cur = tmp;
    else if (data[1] == 0x03)
        m_dev_vis_zoom = tmp;
    else if (data[1] == 0x04)
        m_dev_vis_focus = tmp;
    return 0;
}

int Motor::Parse_Shutter(const uint8_t* data, size_t len)
{
    return 0;
}