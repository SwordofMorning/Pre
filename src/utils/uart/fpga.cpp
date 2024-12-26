#include "fpga.h"

FPGA::FPGA()
    : UART("/dev/ttyS5", 115200, 1024)
{
    // do nothing
}

uint8_t FPGA::Calculate_Checksum(const std::vector<uint8_t>& data, size_t begin, size_t end)
{
    uint8_t checksum = 0;
    for (size_t i = begin; i < end; ++i)
        checksum += data[i];
    return checksum;
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

void FPGA::Set_Gas_Enhancement(int mode)
{
    // clang-format off
    std::vector<uint8_t> cmd = {
        0x55, 0xAA, 0x10, 0x09, 0x00, 0x01, 0x04, 0x00, 0x8F, 0x07, 0x01, 0x00, 0x00, 0x00, 0xA5, 0x00
    };
    // clang-format on

    cmd[10] = mode;
    cmd[14] = Calculate_Checksum(cmd, 3, 14);

    for (auto i : cmd)
    {
        printf("[%x]", i);
    }
    std::cout << std::endl;
}