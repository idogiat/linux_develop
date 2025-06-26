#include "msg_protocol.hpp"
#include "common.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include <fstream>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <fcntl.h> 
#include <unistd.h>
#include <csignal>
#include <sys/stat.h>

// Observer interface
class Observer {
public:
    virtual void update(int data) = 0;
    virtual ~Observer() = default;
};

// Subject / Publisher
class SensorPublisher {
private:
    std::vector<std::weak_ptr<Observer>> observers;
    std::atomic<bool> running;
    std::thread sensorThread;
    std::mutex observerMutex;

public:
    void registerObserver(std::shared_ptr<Observer> obs) {
        std::lock_guard<std::mutex> lock(observerMutex);
        observers.push_back(obs);
    }

    void notifyObservers(int data) {
        std::lock_guard<std::mutex> lock(observerMutex);
        for (auto it = observers.begin(); it != observers.end(); ) {
            if (auto obs = it->lock()) {
                obs->update(data);
                ++it;
            } else {
                it = observers.erase(it);
            }
        }
    }

    void start(const std::string& filePath) {
        running = true;
        sensorThread = std::thread([this, filePath]() {
            int lastValue = -1;
            while (running) {
                std::ifstream file(filePath);
                int value = -1;
                if (file >> value) {
                    if (value != lastValue) {
                        lastValue = value;
                        notifyObservers(value);
                    }
                } else {
                    std::cerr << "[Sensor] Failed to read from " << filePath << "\n";
                }
            }
        });
    }

    void stop() {
        running = false;
        if (sensorThread.joinable()) {
            sensorThread.join();
        }
    }

    ~SensorPublisher() {
        stop();
    }
};

// Example observer: LED service
class LedService : public Observer {
public:
    LedService()
    {
        mkfifo(LEDS_PIPE_PATH, 0666);
    }
    
    ~LedService() {
        remove(LEDS_PIPE_PATH);
    }

    void update(int data) override {
        int fd = open(LEDS_PIPE_PATH, O_WRONLY | O_NONBLOCK);
        if (fd != -1)
        {
            std::string msg = std::to_string(data) + "\n";
            write(fd, msg.c_str(), msg.size());
            close(fd);
        }
        else
        {
            std::cerr << "[LED] Failed to write to pipe\n";
        }
    }
};

// Example observer: Buzzer service
class BuzzerService : public Observer {
public:

    BuzzerService()
    {
        mkfifo(BUZZER_PIPE_PATH, 0666);
    }

    ~BuzzerService()
    {
        remove(BUZZER_PIPE_PATH);
    }

    void update(int data) override {
        if (data < MEDIUM_LEN) {
            std::cout << "[Buzzer] Beep!\n";
        }

        int fd = open(BUZZER_PIPE_PATH, O_WRONLY | O_NONBLOCK);
        if (fd != -1)
        {
            std::string msg = std::to_string(data) + "\n";
            write(fd, msg.c_str(), msg.size());
            close(fd);
        }
    }
};


std::shared_ptr<LedService> led;
std::shared_ptr<BuzzerService> buzzer;

void handle_sigint(int) {
    std::cout << "\n[LED] Caught SIGINT, cleaning up...\n";
    led.reset();  // Triggers destructor
    buzzer.reset();
    exit(0);
}

int main() {
    std::signal(SIGINT, handle_sigint);

    SensorPublisher sensor;

    led = std::make_shared<LedService>();
    buzzer = std::make_shared<BuzzerService>();

    sensor.registerObserver(led);
    sensor.registerObserver(buzzer);

    sensor.start("/dev/distance0");

    std::cout << "Sensor monitoring started. Press Enter to stop.\n";
    std::cin.get();  // Wait for user to press Enter

    sensor.stop();
    std::cout << "Sensor monitoring stopped.\n";

    return 0;
}
