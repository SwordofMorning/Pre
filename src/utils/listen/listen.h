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

class Listen
{
private:
    mqtt_client* m_mqtt_handle;
public:
    Listen();

    void operator()();
};