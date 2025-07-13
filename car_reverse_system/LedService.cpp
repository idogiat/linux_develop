#include "common.hpp"
#include "msg_protocol.hpp"
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
#include <chrono>
#include <csignal>


#define RED_GPIO    17
#define YELLOW_GPIO 27
#define GREEN_GPIO  22


auto last_data_time = std::chrono::steady_clock::now();
std::atomic<bool> redOn{false}, yellowOn{false}, greenOn{false};


static void blink_led(int gpio, std::atomic_bool& enabled, int delay_ms);


void handle_sigint(int)
{
    std::cout << "\n[LED] Caught SIGINT, cleaning up...\n";
    redOn = false;
    yellowOn = false;
    greenOn = false;
    exit(0);
}


int main()
{
    std::signal(SIGINT, handle_sigint);
    std::signal(SIGTERM, handle_sigint);

    std::thread redThread(blink_led, RED_GPIO, std::ref(redOn), 100);
    std::thread yellowThread(blink_led, YELLOW_GPIO, std::ref(yellowOn), 300);
    std::thread greenThread(blink_led, GREEN_GPIO, std::ref(greenOn), 600);

    redThread.detach();
    yellowThread.detach();
    greenThread.detach();

    mkfifo(LEDS_PIPE_PATH, 0666);

    int fd = open(LEDS_PIPE_PATH, O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        std::cerr << "Failed to open pipe for reading.\n";
        return 1;
    }
    
    char buffer[32];
    while (true)
    {
        ssize_t len = read(fd, buffer, sizeof(buffer) - 1);
        if (len > 0) {
            buffer[len] = '\0';
            std::string line(buffer);

            try {
                int num = std::stoi(line);
                last_data_time = std::chrono::steady_clock::now(); 

                if (num < MEDIUM_LEN) {
                    redOn = true; yellowOn = false; greenOn = false;
                    std::cout << "[LED] RED\n";
                } else if (num < FAR_LEN) {
                    redOn = false; yellowOn = true; greenOn = false;
                    std::cout << "[LED] YELLOW\n";
                } else {
                    redOn = false; yellowOn = false; greenOn = true;
                    std::cout << "[LED] GREEN\n";
                }

            } catch (const std::exception& e) {
                std::cerr << "[LED] Failed to parse input: '" << line << "' - " << e.what() << "\n";
            }

        }
        else if (len == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                std::cerr << "[LED] Read error from pipe: " << strerror(errno) << "\n";
            }
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_data_time).count();

        if (elapsed > TIMEOUT) {
            redOn = false; yellowOn = false; greenOn = false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
