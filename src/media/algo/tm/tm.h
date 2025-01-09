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
#include <opencv2/opencv.hpp>
#include "../../../config/config.h"

class Temperature_Measurement
{
public:
    void operator()(uint16_t* input, float* output, int width, int height, float a, float b, float c);
};