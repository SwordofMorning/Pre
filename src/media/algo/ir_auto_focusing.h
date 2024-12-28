#include "../../config/config.h"
#include "../../utils/uart/motor.h"
#include <string>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <opencv2/opencv.hpp>

void ir_auto_focusing_by_image_continuous(Motor& ir_motor, int x, int y);