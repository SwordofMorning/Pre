#pragma once

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
#include "../../../config/config.h"

class Filter
{
public:
    /**
     * @brief Mean filter For NV12 image.
     * 
     * @param yuv NV12 data.
     * @param width image width.
     * @param height image height.
     * @param windows_size filter windows size.
     */
    void Mean_NV12(uint8_t* yuv, size_t width, size_t height, size_t windows_size);
};