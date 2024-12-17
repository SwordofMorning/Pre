#pragma once

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