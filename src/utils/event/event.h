#pragma once

#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <poll.h>
#include <string>
#include <thread>
#include <atomic>
#include "../../config/config.h"
#include "../uart/motor.h"
#include "../uart/fpga.h"
#include "../../media/algo/af/ir_auto_focusing.h"
#include "../../media/algo/af/af_ir.h"

class EventListener
{
public:
    EventListener(Motor& p_motor, FPGA& p_fpga);
    ~EventListener();

    // 启动监听
    bool Start();
    // 停止监听
    void Stop();

private:
    // 监听线程函数
    void ListenThread();
    // 处理单个设备的监听
    void ProcessDevice(const std::string& device);
    // 打印按键信息
    void PrintKeyEvent(const std::string& device, int code, int value);

    std::thread listen_thread_;
    std::atomic<bool> running_;

    Motor& m_motor;
    FPGA& m_fpga;
    AF_IR m_af_ir;
};