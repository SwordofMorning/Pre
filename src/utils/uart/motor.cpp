#include "motor.h"

Motor::Motor()
    : UART("/dev/ttyS6", 115200, 1024)
    , m_dev_ir(0x02)
    , m_dev_vis_zoom(0x03)
    , m_dev_vis_focus(0x04)
    , m_dev_shutter(0x06)
    , m_continuous_running(false)
    , m_continuous_direction(0)
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

Motor::~Motor()
{
    Move_IR_Stop();
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

    litelog.log.debug("Move: [%x][%x][%x][%x][%x][%x][%x][%x][%x]", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6], cmd[7], cmd[8]);

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
    litelog.log.debug("Open Shutter!");
    return this->Shutter(0x00);
}

int Motor::Shutter_Close()
{
    litelog.log.debug("Close Shutter!");
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
        m_step_vis_zoom_cur = tmp;
    else if (data[1] == 0x04)
        m_step_vis_focus_cur = tmp;
    return 0;
}

int Motor::Parse_Shutter(const uint8_t* data, size_t len)
{
    return 0;
}

void Motor::ContinuousMoveThread()
{
    while (m_continuous_running)
    {
        int direction = m_continuous_direction.load();
        if (direction != 0)
        {
            int32_t steps = CONTINUOUS_STEP_SIZE * direction;

            std::lock_guard<std::mutex> lock(m_continuous_mutex);
            Move_IR(steps);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(CONTINUOUS_INTERVAL_MS));
    }
}

bool Motor::Move_IR_Start(Direction direction)
{
    if (m_continuous_running)
    {
        Move_IR_Stop();
    }

    m_continuous_direction = static_cast<int>(direction);
    m_continuous_running = true;

    try
    {
        m_continuous_thread = std::thread(&Motor::ContinuousMoveThread, this);
        return true;
    }
    catch (const std::exception& e)
    {
        litelog.log.error("Failed to start continuous movement: %s", e.what());
        m_continuous_running = false;
        m_continuous_direction = 0;
        return false;
    }
}

void Motor::Move_IR_Stop()
{
    m_continuous_running = false;
    m_continuous_direction = 0;

    if (m_continuous_thread.joinable())
    {
        m_continuous_thread.join();
    }
}

int32_t Motor::Get_Step_IR_Cur()
{
    return m_step_ir_cur;
}