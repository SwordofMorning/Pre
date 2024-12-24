#include "main.hpp"

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
    csi_running = false;
    if (csi_thread.joinable())
        csi_thread.join();

    vo_running = false;
    if (vo_thread.joinable())
        vo_thread.join();
    ab_running = false;
    if (ab_thread.joinable())
        ab_thread.join();

    litelog.log.notice("End!");
    litelog.exit();
}

void execute()
{
    gst_thread = std::thread(gst_thread_func);
    sleep(1);
    dvp_thread = std::thread(dvp_thread_func);
    csi_thread = std::thread(csi_thread_func);
    sleep(1);
    vo_thread = std::thread(vo_thread_func);
    ab_thread = std::thread(ab_thread_func);
    sleep(1);

    Motor motor;
    motor.Start();
    EventListener el(motor);
    el.Start();

    Listen li;
    li();
}

int main()
{
    init();
    execute();
    exit();

    return 0;
}