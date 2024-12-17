#pragma once

#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief uart init function.
 * 
 * @return success or fail.
 * @retval fd, success.
 * @retval -1, open port fail.
 * @retval -2, set opt fail.
 */
int UART_Init(char* dev_name, int baudrate);

/**
 * @brief release uart
 */
void UART_Exit(int fd);

#ifdef __cplusplus
}
#endif