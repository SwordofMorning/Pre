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

#include "../mqtt/mqtt_client.h"
#include "../log/litelog.h"
#include "../jwrap/jwrap.h"

class Listen
{
private:
    std::string m_topic;
    std::string m_target;

    mqtt_client* m_mqtt_handle;

private:
    int Set_Pseudo();
    int Set_Gas_Enhancement();
    int Set_IR_Focus();

public:
    Listen();

    void operator()();
};