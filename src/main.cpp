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

    vo_running = false;
    if (vo_thread.joinable())
        vo_thread.join();

    listen_running = false;
    if (listen_thread.joinable())
        listen_thread.join();

    litelog.log.notice("End!");
    litelog.exit();
}

void execute()
{
    dvp_thread = std::thread(dvp_thread_func);
    sleep(1);
    vo_thread = std::thread(vo_thread_func);
    listen_thread = std::thread(listen_thread_func);
    while (1)
    {
        usleep(200 * 1000);
    }
}

int main()
{
    init();
    execute();
    exit();

    return 0;
}