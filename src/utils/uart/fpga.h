#pragma once

/**
 * @file fpga.h
 * @author Xiaojintao
 * @brief FPGA communication.
 * @version 0.1
 * @date 2024-12-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "uartclass.h"

class FPGA : public UART
{
private:
    uint8_t Calculate_Checksum(const std::vector<uint8_t>& data, size_t begin, size_t length);

public:
    FPGA();

    void NUC();

    void Set_Gas_Enhancement(int mode);
};