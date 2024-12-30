#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <unistd.h>
#include <iostream>
#include <condition_variable>
#include "uartwrap.h"
#include "../log/litelog.h"

class UART
{
protected:
    const char* m_device_name;
    int m_baudrate;
    int m_buffer_size;

    int m_fd;
    std::thread m_listen_thread;
    std::atomic<bool> m_running;
    std::function<void(const uint8_t*, size_t)> m_receive_callback;
    std::mutex m_write_mutex;

protected:
    virtual void ListenThread();

public:
    UART(const char* device_name, int baudrate, int buffer_size);
    virtual ~UART();

    bool Start();
    void Stop();

    void SetCallback(std::function<void(const uint8_t*, size_t)> callback);

    virtual bool Send(const std::vector<uint8_t>& data);
};