#include "motor.h"

Motor::Motor()
    : UART("/dev/ttyS6", 115200, 1024)
{
    // do nothing
}