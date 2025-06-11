#include "main.hpp"

void init()
{
    Config_Init();
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
    usleep(100 * 1000);
    vo_thread = std::thread(vo_thread_func);
    usleep(100 * 1000);

    Motor motor;
    motor.Start();
    FPGA fpga;
    fpga.Start();

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