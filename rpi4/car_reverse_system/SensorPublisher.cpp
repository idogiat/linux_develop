#include <iostream>
#include <vector>
#include <memory>
#include <fstream>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>

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
                // std::this_thread::sleep_for(std::chrono::seconds(1));
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
    void update(int data) override {
        if (data < 10) 
        {
            std::cout << "[LED] RED\n";
        } else if (data < 20)
        {
            std::cout << "[LED] YELLOW\n";
        }
        else
        {
            std::cout << "[LED] GREEN\n";
        }
    }
};

// Example observer: Buzzer service
class BuzzerService : public Observer {
public:
    void update(int data) override {
        if (data < 20) {
            std::cout << "[Buzzer] Beep!\n";
        }
    }
};

int main() {
    SensorPublisher sensor;

    auto led = std::make_shared<LedService>();
    auto buzzer = std::make_shared<BuzzerService>();

    sensor.registerObserver(led);
    sensor.registerObserver(buzzer);

    sensor.start("/dev/distance0");

    std::cout << "Sensor monitoring started. Press Enter to stop.\n";
    std::cin.get();  // Wait for user to press Enter

    sensor.stop();
    std::cout << "Sensor monitoring stopped.\n";

    return 0;
}