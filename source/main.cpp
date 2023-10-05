#include "ios_exploit.h"
#include "common/config.h"
#include "ini.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <coreinit/cache.h>

#ifndef SETUP_MODULE
#include <coreinit/foreground.h>
#include <proc_ui/procui.h>
#include <sysapp/launch.h>
#endif

static int configHandler(void* user, const char* section, const char* name, const char* value)
{
    Configuration_t* config = (Configuration_t*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("General", "Enabled")) {
        if (strcmp(value, "True") == 0) {
            config->enabled = 1;
        } else if (strcmp(value, "False") == 0) {
            config->enabled = 0;
        } else {
            return 0;
        }
    } else if (MATCH("Net", "Mode")) {
        if (strcmp(value, "ALL") == 0) {
            config->mode = MODE_ALL;
        } else if (strcmp(value, "IPV4") == 0) {
            config->mode = MODE_IPV4;
        } else if (strcmp(value, "TCP") == 0) {
            config->mode = MODE_TCP;
        } else if (strcmp(value, "UDP") == 0) {
            config->mode = MODE_UDP;
        } else if (strcmp(value, "PRUDP") == 0) {
            config->mode = MODE_PRUDP;
        } else {
            return 0;
        }
    } else if (MATCH("Net", "MaxPacketSize")) {
        config->maxPacketSize = strtol(value, NULL, 0);
    } else {
        return 0;  /* unknown section/name, error */
    }

    return 1;
}

int main(int argc, char **argv)
{
#ifndef SETUP_MODULE
    // init procui
    ProcUIInit(&OSSavesDone_ReadyToRelease);
#endif

    // create folder in case it doesn't exist
    mkdir("/vol/external01/HokakuCafe/", 0700);

    // parse config file
    Configuration_t config;
    const char *configPath = "/vol/external01/HokakuCafe/config.ini";
    if (ini_parse(configPath, configHandler, &config) < 0) {
        // create default config
        FILE* f = fopen(configPath, "w");
        if (f) {
            fputs("[General]\n"
                  "Enabled=True\n\n"
                  "[Net]\n"
                  "Mode=PRUDP ; Can be either ALL, IPV4, TCP, UDP or PRUDP\n"
                  "MaxPacketSize=0x800\n", f);
            fclose(f);
        }

        config.enabled = 1;
        config.mode = MODE_PRUDP;
        config.maxPacketSize = 0x800;
    }

    if (config.enabled) {
        // write config to mem
        memcpy((void*)0xF4160000, &config, sizeof(config));
        DCStoreRange((void*)0xF4160000, sizeof(config));

        // run the ios exploit
        ExecuteIOSExploit();
    }

#ifndef SETUP_MODULE
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
