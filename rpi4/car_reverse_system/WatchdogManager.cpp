#include "heartbeat_shared.hpp"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <ctime>

int main()
{
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(HeartbeatTable));

    void* ptr = mmap(NULL, sizeof(HeartbeatTable), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("mmap");
        return 1;
    }

    HeartbeatTable* table = static_cast<HeartbeatTable*>(ptr);

    while (true)
    {
        time_t now = time(NULL);
        for (int i = 0; i < MAX_PROCESSES; ++i)
        {
            if (table->entries[i].pid == 0) continue;

            double seconds = difftime(now, table->entries[i].last_heartbeat);

            if (seconds > 5)
            {
                std::cout << "[WATCHDOG] Process " << table->entries[i].name
                          << " (PID " << table->entries[i].pid << ") not responding\n";
                kill(table->entries[i].pid, SIGKILL);
                sleep(1); // give time for OS to clean up

                pid_t new_pid = fork();
                if (new_pid == 0)
                {
                    execl(table->entries[i].path, table->entries[i].path, nullptr);
                    perror("execl failed");
                    _exit(1);
                }
                else if (new_pid > 0)
                {
                    table->entries[i].pid = new_pid;
                    table->entries[i].last_heartbeat = time(NULL);
                    std::cout << "[WATCHDOG] Restarted " << table->entries[i].name
                              << " with new PID " << new_pid << std::endl;
                }
                else
                {
                    perror("fork failed");
                }
            }
        }

        sleep(1);
    }

    return 0;
}