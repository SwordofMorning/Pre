#include "fpga.h"

FPGA::FPGA()
    : UART("/dev/ttyS5", 115200, 1024)
{
    // do nothing
}

void FPGA::NUC()
{
    // clang-format off
    std::vector<uint8_t> cmd = {
        0x55, 0xAA, 0x10, 0x09, 0x00, 0x01, 0x04, 0x00, 0x0A, 0x07, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00
    };
    // clang-format on

    this->Send(cmd);
}