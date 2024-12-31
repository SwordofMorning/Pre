#pragma once

/**
 * @file uartwrap.h
 * @author Xiaojintao
 * @brief Uart C lang interface.
 * @version 0.1
 * @date 2024-12-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */

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