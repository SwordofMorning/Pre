#pragma once

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