#include <iostream>
#include <thread>
#include "../others/version/version.h"
#include "./utils/log/litelog.h"
#include "./media/vi/v4l2_dvp.h"
#include "./media/vo/vo_shm.h"

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
    algo_init();
    while (vo_running)
    {
        pthread_mutex_lock(&v4l2_ir_dvp_share_buffer_mutex);
        while (!v4l2_ir_dvp_share_buffer_updated && vo_running)
        {
            pthread_cond_wait(&v4l2_ir_dvp_share_buffer_cond, 
                            &v4l2_ir_dvp_share_buffer_mutex);
        }
        if (!vo_running)
        {
            pthread_mutex_unlock(&v4l2_ir_dvp_share_buffer_mutex);
            break;
        }
        v4l2_ir_dvp_share_buffer_updated = 0;
        pthread_mutex_unlock(&v4l2_ir_dvp_share_buffer_mutex);
        
        algo_process();
    }
    algo_exit();
}

void init()
{
    Config_Init();

    litelog.init("Pre");
    litelog.log.notice("Start!");
    litelog.log.notice("Branch: %s", __GIT_BRANCH__);
    litelog.log.notice("Commit ID: %s", __GIT_COMMIT_ID__);
    litelog.log.notice("User: %s", __GIT_USER__);
}

void exit()
{
    Config_Exit();

    dvp_running = false;
    if (dvp_thread.joinable())
        dvp_thread.join();

    vo_running = false;
    if (vo_thread.joinable())
        vo_thread.join();
    
    litelog.log.notice("End!");
    litelog.exit();
}

void execute()
{
    dvp_thread = std::thread(dvp_thread_func);
    sleep(1);
    vo_thread = std::thread(vo_thread_func);
    while (1)
    {
        // do nothing
        usleep(10);
    }
}

int main()
{
    init();

    execute();

    exit();

    return 0;
}