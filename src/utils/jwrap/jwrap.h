#pragma once

#include "cjson/cJSON.h"
#include <string>
#include <map>
#include <stdexcept>
#include <variant>
#include "../log/litelog.h"

class JWrap
{
public:
    enum CODE_ENUM
    {
        PSEUDO = 510,
        GAS_ENHANCE = 514,
        IR_AUTOFOCUS = 523,
    };

    struct IrAutoFocusData
    {
        std::string type;
        std::string enable;
    };

private:
    static const std::map<int, std::string> m_code_map;

    int m_code_enum;
    std::string m_code_name;
    std::string m_method;
    std::variant<std::string, IrAutoFocusData> m_value;

private:
    int Parse(std::string p_json);
    int Validate() const;

public:
    JWrap(std::string p_json);

    int GetCodeEnum() const;
    std::string GetCodeName() const;
    std::string GetMethod() const;
    std::variant<std::string, IrAutoFocusData> GetValue() const;

    std::string CreateReturnJson(std::string p_status);
};