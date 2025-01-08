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
#include "include/device.h"
#include "include/enum.h"
#include "include/global.h"
#include "include/debug.h"
#include "lut/lut.h"
#include "cl/pseudo_cl.h"
#include "cl/filter_cl.h"
#include "cl/diff_cl.h"
#include "../../others/version/version.h"
#include "../utils/log/litelog.h"

/**
 * @file config.h
 * @author Xiaojintao
 * @brief Total config for program.
 * @version 0.1
 * @date 2024-12-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */

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