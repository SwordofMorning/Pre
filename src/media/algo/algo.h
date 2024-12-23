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
#include "../../config/lut/lut.h"
#include "../../config/config.h"
#include "../vo/gst.h"
#include "pseudo_am.h"

int Process_One_Frame();

void Pseudo_420P(uint16_t* input, uint8_t* y_out, uint8_t* u_out, uint8_t* v_out, int width, int height);

void Pseudo_NV12(uint16_t* input, uint8_t* y_out, uint8_t* uv_out, int width, int height);