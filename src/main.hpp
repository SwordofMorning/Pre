#pragma once

#include <iostream>
#include <thread>
#include "./utils/log/litelog.h"
#include "./media/vi/v4l2_dvp.h"
#include "./media/vi/v4l2_csi.h"
#include "./media/vo/shm_vo.h"
#include "./media/vo/shm_algo.h"
#include "./utils/listen/listen.h"
#include "./utils/event/event.h"
#include "./utils/uart/motor.h"
#include "./utils/uart/fpga.h"

std::thread dvp_thread;
bool dvp_running = true;
std::thread vo_thread;
bool vo_running = true;

void dvp_thread_func()
{
    DVP_Streaming();
}

void vo_thread_func()
{
    SHM_VO_Init();
    while (vo_running)
    {
        SHM_VO_Process();
    }
    SHM_VO_Exit();
}