#include "common.hpp"
#include "msg_protocol.hpp"
#include "heartbeat_shared.hpp"
#include "gpio.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include <cstring>
#include <atomic>
#include <chrono>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>


#define RED_GPIO    17
#define YELLOW_GPIO 27
#define GREEN_GPIO  22


static void blink_led(int gpio, std::atomic_bool& enabled, int delay_ms);


int main()
{
    int wd_index = start_watchdog_heartbeat("LedService");
    if (wd_index == -1)
    {
        std::cerr << "Failed to register with Watchdog\n";
        return 1;
    }
    
    std::atomic<bool> redOn{false}, yellowOn{false}, greenOn{false};

    std::thread redThread(blink_led, RED_GPIO, std::ref(redOn), 100);
    std::thread yellowThread(blink_led, YELLOW_GPIO, std::ref(yellowOn), 300);
    std::thread greenThread(blink_led, GREEN_GPIO, std::ref(greenOn), 600);

    redThread.detach();
    yellowThread.detach();
    greenThread.detach();

    mkfifo(LEDS_PIPE_PATH, 0666);

    int fd = open(LEDS_PIPE_PATH, O_RDONLY);
    if (fd == -1) {
        std::cerr << "Failed to open pipe for reading.\n";
        return 1;
    }
    
    char buffer[32];
    while (true)
    {
        ssize_t len = read(fd, buffer, sizeof(buffer) - 1);
        if (len > 0)
        {
            buffer[len] = '\0';  // null terminate
            std::string line(buffer);
            int num = stoi(line);
            if (num < MEDIUM_LEN)
            {
                redOn = true;
                yellowOn = false;
                greenOn = false;
                std::cout << "[LED] RED\n";
            }
            else if (num < FAR_LEN)
            {
                redOn = false;
                yellowOn = true;
                greenOn = false;
                std::cout << "[LED] YELLOW\n";
            }
            else
            {
                redOn = false;
                yellowOn = false;
                greenOn = true;
                std::cout << "[LED] GREEN\n";
            }
        }
        else if (len == 0)
        {
            // Writer closed the pipe
            close(fd);
            fd = open(LEDS_PIPE_PATH, O_RDONLY);
            if (fd == -1)
            {
                break;
            }
        }
        else
        {
            std::cerr << "Read error.\n";
            break;
        }
    }

    close(fd);
    return 0;
}


static void blink_led(int gpio, std::atomic_bool& enabled, int delay_ms)
{
    GPIO led("gpiochip0", gpio);
    while (true)
    {
        if (enabled)
        {
            led.write(0);
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
            led.write(1);
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        }
        else
        {
            led.write(1);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}