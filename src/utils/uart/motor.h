#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <unistd.h>
#include <iostream>
#include "uartwrap.h"

class Motor
{
private:
    static constexpr const char* DEVICE_NAME = "/dev/ttyS6";
    static constexpr int BAUDRATE = 115200;
    static constexpr int BUFFER_SIZE = 1024;

    int m_fd;
    std::thread m_listen_thread;
    std::atomic<bool> m_running; // 运行状态标志

private:
    // 监听线程函数
    void ListenThread();

    // 回调函数，用于处理接收到的数据
    std::function<void(const uint8_t*, size_t)> m_receive_callback;

    std::mutex m_write_mutex;

public:
    Motor();
    ~Motor();

    bool Start();
    void Stop();

    void operator()();

    void SetCallback(std::function<void(const uint8_t*, size_t)> callback);

    bool Send(const std::vector<uint8_t>& data);
};