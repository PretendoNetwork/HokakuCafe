#include "ios_exploit.h"

#ifndef TIRAMISU
#include <coreinit/foreground.h>
#include <proc_ui/procui.h>
#include <sysapp/launch.h>
#endif

int main(int argc, char **argv)
{
#ifndef TIRAMISU
    // init procui
    ProcUIInit(&OSSavesDone_ReadyToRelease);
#endif

    // run the ios exploit
    ExecuteIOSExploit();

#ifndef TIRAMISU
    // exit to the menu as soon as possible
    ProcUIStatus status;
    while ((status = ProcUIProcessMessages(TRUE)) != PROCUI_STATUS_EXITING) {
        if(status == PROCUI_STATUS_RELEASE_FOREGROUND) {
            ProcUIDrawDoneRelease();
        }

        if(status != PROCUI_STATUS_IN_FOREGROUND) {
            continue;
        }

        SYSLaunchMenu();
    }

    // shutdown procui
    ProcUIShutdown();
#endif

    return 0;
}
