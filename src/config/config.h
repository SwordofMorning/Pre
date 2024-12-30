#pragma once

/**
 * @file src/config/config.h
 * @author Xiaojintao
 * @brief config file of all.
 * @version 0.1
 * @date 2024-08-05
 */

#include <pthread.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <stdarg.h>
#include "./include/device.h"
#include "./include/enum.h"
#include "./include/global.h"
#include "./include/debug.h"
#include "./lut/lut.h"
#include "./lut/pseudo.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CLEAR(x) memset(&(x), 0, sizeof(x))

/**
 * @brief Config all parameters.
 */
void Config_Init();

/**
 * @brief Clear resource before exit program.
 */
void Config_Exit();

#ifdef __cplusplus
}
#endif