#pragma once

/**
 * @file controller.h
 * @author Xiaojintao
 * @brief MQTT command listener.
 * @version 0.1
 * @date 2024-12-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "controller.h"

class Listen : public Controller
{
private:
    std::string m_topic;
    std::string m_target;

    mqtt_client* m_mqtt_handle;

public:
    Listen();

    void operator()();
};