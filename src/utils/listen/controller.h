#pragma once

#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include <unordered_map>
#include <string>
#include <memory>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <functional>
#include <map>
#include "../mqtt/mqtt_client.h"
#include "../log/litelog.h"
#include "../jwrap/jwrap.h"
#include "../../config/config.h"

class Controller
{
protected:
    enum class PseudoMode
    {
        IRONBOW_FORWARD,
        IRONBOW_REVERSE,
        LAVA_FORWARD,
        LAVA_REVERSE,
        RAINBOW_FORWARD,
        RAINBOW_REVERSE,
        RAINBOWHC_FORWARD,
        RAINBOWHC_REVERSE,
        BLACK_HOT,
        WHITE_HOT
    };

    enum ErrorCode
    {
        SUCCESS = 0,
        OUT_OF_BOUNDS = -1,
        OBJ_NOT_FOUND = -2,
        INVALID_PARAMETER = -3
    };

protected:
    static const std::map<std::string, PseudoMode> string_to_mode;
    static const std::map<PseudoMode, int> mode_to_pseudo;

    int Set_Pseudo(PseudoMode mode) const;
    int Set_Pseudo(const std::variant<std::string, JWrap::IrAutoFocusData>& value);
    int Set_Gas_Enhancement(const std::variant<std::string, JWrap::IrAutoFocusData>& value);
    int Set_IR_Focus(const std::variant<std::string, JWrap::IrAutoFocusData>& value);
};