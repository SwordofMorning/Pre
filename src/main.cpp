#include <iostream>
#include "../others/version/version.h"
#include "./utils/log/litelog.h"
#include "./media/vi/v4l2_dvp.h"

void init_modules()
{
    /* Init Modules */
    litelog.init("Pre");

    litelog.log.notice("Start!");
    litelog.log.notice("Branch: %s", __GIT_BRANCH__);
    litelog.log.notice("Commit ID: %s", __GIT_COMMIT_ID__);
    litelog.log.notice("User: %s", __GIT_USER__);
}

void exit_modules()
{
    litelog.log.notice("End!");
    litelog.exit();
}

void execute()
{
    DVP_Streaming();
}

int main()
{
    init_modules();

    execute();

    exit_modules();

    return 0;
}