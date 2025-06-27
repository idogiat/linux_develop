#include "common.hpp"
#include "gpio.hpp" 
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <csignal>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define BUZZER_GPIO 12

std::atomic<int> currentZone(-1);
std::atomic<bool> stopFlag(false);
std::thread buzzerThread;


void beep(GPIO& gpio, int frequency, int durationMs) {
    int half_period_us = 1000000 / (2 * frequency);
    int cycles = frequency * durationMs / 1000;

    for (int i = 0; i < cycles; ++i)
    {
        gpio.write(true);
        usleep(half_period_us);
        gpio.write(false);
        usleep(half_period_us);
    }
}

void buzzer_loop() {
    GPIO buzzer("gpiochip0", BUZZER_GPIO);

    while (!stopFlag) {
        int zone = currentZone;

        int delayBetweenBeeps = 1000;
        std::pair<int, int> tones = {300, 350}; // zone 2: far

        if (zone == 1) {
            delayBetweenBeeps = 500;
            tones = {500, 600};
        } else if (zone == 0) {
            delayBetweenBeeps = 200;
            tones = {800, 1000};
        } else if (zone == -1 || zone > 2) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        beep(buzzer, tones.first, 100);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        beep(buzzer, tones.second, 100);
        std::this_thread::sleep_for(std::chrono::milliseconds(delayBetweenBeeps));
    }

    buzzer.write(false);
}

void handle_sigint(int) {
    std::cout << "\n[Buzzer] SIGINT received. Cleaning up...\n";
    stopFlag = true;
    if (buzzerThread.joinable()) buzzerThread.join();
    unlink(BUZZER_PIPE_PATH);
    exit(0);
}

int main() {
    std::signal(SIGINT, handle_sigint);
    mkfifo(BUZZER_PIPE_PATH, 0666);

    buzzerThread = std::thread(buzzer_loop);

    int fd = open(BUZZER_PIPE_PATH, O_RDONLY);
    if (fd == -1) {
        std::cerr << "[Buzzer] Failed to open pipe\n";
        return 1;
    }

    char buf[128];
    while (true) {
        ssize_t len = read(fd, buf, sizeof(buf) - 1);
        if (len > 0) {
            buf[len] = '\0';
            int val = std::stoi(buf);

            int zone;
            if (val < 10) zone = 0;
            else if (val < 20) zone = 1;
            else zone = 2;

            currentZone = zone;
            std::cout << "[Buzzer] Distance: " << val << " ? Zone " << zone << "\n";
        }
    }

    close(fd);
    return 0;
}