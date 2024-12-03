#include "controller.h"

// clang-format off
const std::map<std::string, Controller::PseudoMode> Controller::string_to_mode =
{
    {"ironbow_forward", PseudoMode::IRONBOW_FORWARD},
    {"ironbow_reverse", PseudoMode::IRONBOW_REVERSE},
    {"lava_forward", PseudoMode::LAVA_FORWARD},
    {"lava_reverse", PseudoMode::LAVA_REVERSE},
    {"rainbow_forward", PseudoMode::RAINBOW_FORWARD},
    {"rainbow_reverse", PseudoMode::RAINBOW_REVERSE},
    {"rainbowhc_forward", PseudoMode::RAINBOWHC_FORWARD},
    {"rainbowhc_reverse", PseudoMode::RAINBOWHC_REVERSE},
    {"black_hot", PseudoMode::BLACK_HOT},
    {"white_hot", PseudoMode::WHITE_HOT}
};

const std::map<Controller::PseudoMode, int> Controller::mode_to_pseudo =
{
    {PseudoMode::IRONBOW_FORWARD, PSEUDO_IRONBOW_FORWARD},
    {PseudoMode::IRONBOW_REVERSE, PSEUDO_IRONBOW_REVERSE},
    {PseudoMode::LAVA_FORWARD, PSEUDO_LAVA_FORWARD},
    {PseudoMode::LAVA_REVERSE, PSEUDO_LAVA_REVERSE},
    {PseudoMode::RAINBOW_FORWARD, PSEUDO_RAINBOW_FORWARD},
    {PseudoMode::RAINBOW_REVERSE, PSEUDO_RAINBOW_REVERSE},
    {PseudoMode::RAINBOWHC_FORWARD, PSEUDO_RAINBOWHC_FORWARD},
    {PseudoMode::RAINBOWHC_REVERSE, PSEUDO_RAINBOWHC_REVERSE},
    {PseudoMode::BLACK_HOT, PSEUDO_BLACK_HOT},
    {PseudoMode::WHITE_HOT, PSEUDO_WHITE_HOT}
};

int Controller::Set_Pseudo(PseudoMode mode) const
{
    auto it = mode_to_pseudo.find(mode);
    if (it != mode_to_pseudo.end())
    {
        usr.pseudo = it->second;
        litelog.log.info("Set pseudo mode_int: {%x}.", it->second);
        return Controller::ErrorCode::SUCCESS;
    }
    litelog.log.info("Set pseudo out of boundary.");
    return Controller::ErrorCode::OUT_OF_BOUNDS;
}

int Controller::Set_Pseudo(const std::variant<std::string, JWrap::AutoFocusDataIR>& value)
{
    return std::visit([this](const auto& val) -> int
    {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, std::string>)
        {
            auto it = string_to_mode.find(val);
            if (it != string_to_mode.end())
            {
                litelog.log.info("Set pseudo mode_str: {%s}.", it->first.c_str());
                return Set_Pseudo(it->second);
            }
            litelog.log.error("Unknown pseudo mode: {%s}.", val);
            return Controller::ErrorCode::OBJ_NOT_FOUND;
        }
        litelog.log.error("Invalid value type for pseudo mode.");
        return Controller::ErrorCode::INVALID_PARAMETER;
    }, value);
}

int Controller::Set_Gas_Enhancement(const std::variant<std::string, JWrap::AutoFocusDataIR>& value)
{
    return std::visit([this](const auto& val) -> int
    {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, std::string>)
        {
            int p_enhance = atoi(val.c_str());
            if (GAS_ENHANCEMENT_MIN < p_enhance && p_enhance < GAS_ENHANCEMENT_MAX)
            {
                usr.gas_enhancement = p_enhance;
                litelog.log.info("Set gas enhancement: {%d}.", p_enhance);
                return Controller::ErrorCode::SUCCESS;
            }
            litelog.log.error("Gas enhancement parameter {%d}, out of boundary.", p_enhance);
            return Controller::ErrorCode::OBJ_NOT_FOUND;
        }
        litelog.log.error("Invalid value type for gas enhancement mode");
        return Controller::ErrorCode::INVALID_PARAMETER;
    }, value);
}

int Controller::Set_IR_Focus(const std::variant<std::string, JWrap::AutoFocusDataIR>& value)
{
    return std::visit([this](const auto& val) -> int
    {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, JWrap::AutoFocusDataIR>)
        {
            // Enable Autofocus
            if (val.enable == "1")
            {
                // Oneshot
                if (val.type == "single")
                {
                    // @todo
                    litelog.log.info("Set IR autofocus: {%s}{%s}.", val.enable.c_str(), val.type.c_str());
                    return Controller::ErrorCode::SUCCESS;
                }
                // Continue
                else if (val.type == "continue")
                {
                    // @todo
                    litelog.log.info("Set IR autofocus: {%s}{%s}.", val.enable.c_str(), val.type.c_str());
                    return Controller::ErrorCode::SUCCESS;
                }
                return Controller::ErrorCode::INVALID_PARAMETER;
            }
            // Disable Autofocus
            else if (val.enable == "0")
            {
                // @todo
                litelog.log.info("Set IR autofocus: {%s}{%s}.", val.enable.c_str(), val.type.c_str());
                return Controller::ErrorCode::SUCCESS;
            }
            return Controller::ErrorCode::INVALID_PARAMETER;
        }
        litelog.log.error("Invalid value type for autofocus mode");
        return Controller::ErrorCode::INVALID_PARAMETER;
    }, value);
}

// clang-format on