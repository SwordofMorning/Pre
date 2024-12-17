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
public:
    Motor();
    ~Motor();

    // 启动监听
    bool Start();
    // 停止监听
    void Stop();

    // 运算符重载，实现监听功能
    void operator()();

    // 设置接收回调函数
    void SetCallback(std::function<void(const uint8_t*, size_t)> callback) {
        receive_callback_ = callback;
    }

    bool Send(const std::vector<uint8_t>& data);

private:
    // 监听线程函数
    void ListenThread();

    int fd_;                     // 串口文件描述符
    std::thread listen_thread_;  // 监听线程
    std::atomic<bool> running_;  // 运行状态标志
    
    // 回调函数，用于处理接收到的数据
    std::function<void(const uint8_t*, size_t)> receive_callback_;

    static constexpr const char* DEVICE_NAME = "/dev/ttyS6";
    static constexpr int BAUDRATE = 115200;
    static constexpr int BUFFER_SIZE = 1024;

    std::mutex write_mutex_;
};