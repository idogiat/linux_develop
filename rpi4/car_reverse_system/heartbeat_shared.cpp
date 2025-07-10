#include "heartbeat_shared.hpp"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <thread>
#include <iostream>


int start_watchdog_heartbeat(const std::string& name)
{
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("shm_open");
        return -1;
    }

    void* shm_ptr = mmap(NULL, sizeof(HeartbeatTable), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        perror("mmap");
        return -1;
    }

    HeartbeatTable* table = static_cast<HeartbeatTable*>(shm_ptr);
    pid_t my_pid = getpid();

    char exe_path[128] = {0};
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len != -1) exe_path[len] = '\0';

    int my_index = -1;
    for (int i = 0; i < MAX_PROCESSES; ++i)
    {
        if (table->entries[i].pid == 0)
        {
            table->entries[i].pid = my_pid;
            table->entries[i].last_heartbeat = time(NULL);
            strncpy(table->entries[i].name, name.c_str(), sizeof(table->entries[i].name) - 1);
            strncpy(table->entries[i].path, exe_path, sizeof(table->entries[i].path) - 1);
            my_index = i;
            break;
        }
    }

    if (my_index == -1)
    {
        std::cerr << "[WatchdogClient] No slot available\n";
        return -1;
    }

    std::thread heartbeat_thread([table, my_index]()
    {
        while (true)
        {
            table->entries[my_index].last_heartbeat = time(NULL);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    heartbeat_thread.detach();

    return my_index;
}


void unregister_watchdog_client(int index)
{
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("shm_open");
        return;
    }

    void* shm_ptr = mmap(NULL, sizeof(HeartbeatTable), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        perror("mmap");
        close(shm_fd);
        return;
    }

    HeartbeatTable* table = static_cast<HeartbeatTable*>(shm_ptr);
    if (index >= 0 && index < MAX_PROCESSES)
    {
        table->entries[index].pid = 0;
        table->entries[index].last_heartbeat = 0;
        memset(table->entries[index].name, 0, sizeof(table->entries[index].name));
        memset(table->entries[index].path, 0, sizeof(table->entries[index].path));
        std::cout << "[WatchdogClient] Unregistered from index " << index << std::endl;
    }

    munmap(shm_ptr, sizeof(HeartbeatTable));
    close(shm_fd);
}
