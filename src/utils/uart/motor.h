#pragma once

/**
 * @file motor.h
 * @author Xiaojintao
 * @brief Motor driver.
 * @version 0.1
 * @date 2024-12-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "uartclass.h"
#include "../log/litelog.h"

class Motor : public UART
{
private:
    int32_t m_step_ir_cur;
    int32_t m_step_vis_focus_cur;
    int32_t m_step_vis_zoom_cur;

    const uint8_t m_dev_ir;
    const uint8_t m_dev_vis_zoom;
    const uint8_t m_dev_vis_focus;
    const uint8_t m_dev_shutter;

    /* Continuous Move */
    std::thread m_continuous_thread;
    std::atomic<bool> m_continuous_running;
    std::atomic<int> m_continuous_direction;
    std::mutex m_continuous_mutex;
    static constexpr int32_t CONTINUOUS_STEP_SIZE = 100;
    static constexpr int CONTINUOUS_INTERVAL_MS = 50;

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

    void ContinuousMoveThread();

public:
    Motor();
    virtual ~Motor();

    enum class Direction
    {
        FORWARD = 1,
        BACKWARD = -1,
        STOP = 0
    };

    int Move_IR(int32_t steps);
    int Move_Vis_Zoom(int32_t steps);
    int Move_Vis_Focus(int32_t steps);
    int Shutter_Open();
    int Shutter_Close();

    bool Move_IR_Start(Direction direction);
    void Move_IR_Stop();

    /**
     * @brief Get Current IR Step.
     * @return m_step_ir_cur 
     */
    int32_t Get_Step_IR_Cur();
};