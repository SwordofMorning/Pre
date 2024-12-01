#pragma once

#include <iostream>
#include <thread>
#include "../others/version/version.h"
#include "./utils/log/litelog.h"
#include "./media/vi/v4l2_dvp.h"
#include "./media/vo/vo_shm.h"
#include "./utils/listen/listen.h"

std::thread dvp_thread;
bool dvp_running = true;
std::thread vo_thread;
bool vo_running = true;
std::thread listen_thread;
bool listen_running = true;

void dvp_thread_func()
{
    DVP_Streaming();
}

void vo_thread_func()
{
    SHM_Init();
    while (vo_running)
    {
        SHM_Process();
    }
    SHM_Exit();
}

void listen_thread_func()
{
    Listen li;
    li();
}