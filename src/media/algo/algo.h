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
#include "../../config/lut/lut.h"
#include "../../config/config.h"
#include "../vo/vo_gst.h"

int Process_One_Frame();

void Pseudo(uint16_t* input, uint8_t* y_out, uint8_t* u_out, uint8_t* v_out, int width, int height);