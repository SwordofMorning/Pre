#pragma once

#include <iostream>
#include <thread>
#include "../others/version/version.h"
#include "./utils/log/litelog.h"
#include "./media/vi/v4l2_dvp.h"
#include "./media/vi/v4l2_csi.h"
#include "./media/vo/shm_vo.h"
#include "./media/vo/shm_algo.h"
#include "./media/vo/gst.h"
#include "./utils/listen/listen.h"
#include "./utils/event/event.h"
#include "./utils/uart/motor.h"

std::thread dvp_thread;
bool dvp_running = true;
std::thread csi_thread;
bool csi_running = true;
std::thread vo_thread;
bool vo_running = true;
std::thread gst_thread;
bool ab_running = true;
std::thread ab_thread;

void dvp_thread_func()
{
    DVP_Streaming();
}

void csi_thread_func()
{
    CSI_Streaming();
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

// i.e. algo backend
void ab_thread_func()
{
    SHM_ALGO_Init();
    while (vo_running)
    {
        SHM_ALGO_Process();
    }
    SHM_ALGO_Exit();
}

void gst_thread_func()
{
    VO_GST_Streaming();
}