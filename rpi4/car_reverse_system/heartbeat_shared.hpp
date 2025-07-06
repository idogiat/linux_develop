#pragma once
#include <time.h>

#define SHM_NAME     "/watchdog_shm"
#define MAX_PROCESSES 8


struct HeartbeatEntrys,
{
    pid_t pid;
    time_t last_heartbeat;
    char name[32];
    char path[128];
};


struct HeartbeatTable
{
    HeartbeatEntry entries[MAX_PROCESSES];
};


int start_watchdog_heartbeat(const std::string& name);
