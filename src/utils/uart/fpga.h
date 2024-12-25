#pragma once

#include "uartclass.h"

class FPGA : public UART
{
public:
    FPGA();

    void NUC();
};