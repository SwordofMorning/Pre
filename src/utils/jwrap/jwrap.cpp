#include "jwrap.h"

// clang-format off
const std::map<int, std::string> JWrap::m_code_map = 
{
    {CODE_ENUM::PSEUDO, "pseudo"},
    {CODE_ENUM::GAS_ENHANCEMENT, "gas_enhancement"},
    {CODE_ENUM::AUTOFOCUS_IR, "autofocus_ir"}
};
// clang-format on

JWrap::JWrap(std::string p_json)
{
    this->Parse(p_json);
}

int JWrap::Parse(std::string p_json)
{
    cJSON* root = cJSON_Parse(p_json.c_str());
    if (!root)
    {
        litelog.log.error("Invalid JSON format");
        return -1;
    }

    // Parse code object
    cJSON* code = cJSON_GetObjectItem(root, "code");
    if (!code)
    {
        cJSON_Delete(root);
        litelog.log.error("Missing code object");
        return -1;
    }

    // Get code enum
    cJSON* code_enum = cJSON_GetObjectItem(code, "enum");
    if (code_enum)
        m_code_enum = code_enum->valueint;

    // Get code name
    cJSON* code_name = cJSON_GetObjectItem(code, "name");
    if (code_name)
        m_code_name = code_name->valuestring;

    // Get method
    cJSON* method = cJSON_GetObjectItem(root, "method");
    if (method)
        m_method = method->valuestring;

    // Parse data based on code enum
    cJSON* data = cJSON_GetObjectItem(root, "data");
    if (data)
    {
        switch (m_code_enum)
        {
        case PSEUDO:
        case GAS_ENHANCEMENT: {
            cJSON* value = cJSON_GetObjectItem(data, "value");
            if (value)
            {
                m_value = std::string(value->valuestring);
            }
            break;
        }
        case AUTOFOCUS_IR: {
            AutoFocusDataIR af_data;
            cJSON* type = cJSON_GetObjectItem(data, "type");
            cJSON* enable = cJSON_GetObjectItem(data, "enable");
            if (type)
                af_data.type = type->valuestring;
            if (enable)
                af_data.enable = enable->valuestring;
            m_value = af_data;
            break;
        }
        }
    }

    cJSON_Delete(root);
    return Validate();
}

int JWrap::Validate() const
{
    if (m_code_map.find(m_code_enum) == m_code_map.end())
    {
        litelog.log.error("Invalid code enum");
        return -1;
    }

    if (m_code_map.at(m_code_enum) != m_code_name)
    {
        litelog.log.error("Code name doesn't match enum");
        return -1;
    }

    if (m_method.empty())
    {
        litelog.log.error("Method cannot be empty");
        return -1;
    }
    return 0;
}

std::string JWrap::CreateReturnJson(std::string p_status)
{
    cJSON* root = cJSON_CreateObject();

    // Add code object
    cJSON* code = cJSON_CreateObject();
    cJSON_AddNumberToObject(code, "enum", m_code_enum);
    cJSON_AddStringToObject(code, "name", m_code_name.c_str());
    cJSON_AddItemToObject(root, "code", code);

    // Add method
    cJSON_AddStringToObject(root, "method", m_method.c_str());

    // Add Status
    cJSON_AddStringToObject(root, "status", p_status.c_str());

    // PrintUnformatted to delete
    char* json_str = cJSON_PrintUnformatted(root);
    std::string result(json_str);
    free(json_str);
    cJSON_Delete(root);

    return result;
}

int JWrap::GetCodeEnum() const
{
    return m_code_enum;
}

std::string JWrap::GetCodeName() const
{
    return m_code_name;
}

std::string JWrap::GetMethod() const
{
    return m_method;
}

std::variant<std::string, JWrap::AutoFocusDataIR> JWrap::GetValue() const
{
    return m_value;
}