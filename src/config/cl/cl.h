#pragma once

#include <CL/cl.h>
#include <stdint.h>
#include <stdbool.h>
#include "../lut/lut.h"
#include "../include/global.h"

char* read_kernel_source(const char* filename);