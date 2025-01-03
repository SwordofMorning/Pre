#pragma once

/**
 * @file algo.h
 * @author Xiaojintao
 * @brief Algorithm interface.
 * @version 0.1
 * @date 2024-12-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <algorithm>
#include <cstring>
#include <cmath>
#include "../../config/lut/lut.h"
#include "../../config/lut/pseudo.h"
#include "../../config/config.h"
#include "pseudo/pseudo_am.h"
#include "pseudo/pseudo.h"

int Process_One_Frame();