# Distance Sensing System with Multi-Process Architecture on Raspberry Pi

## Overview
This project implements a modular, multi-process system on a Raspberry Pi to measure distances using the HC-SR04 ultrasonic sensor. It reacts to distance changes using LEDs and a buzzer, and is designed to be robust, extendable, and maintainable. The system includes a kernel module, inter-process communication, and process isolation per hardware component.

## Features
- Kernel module for real-time HC-SR04 distance sensing
- Multiple user-space processes for individual components:
  - LED indicators (green, yellow, red)
  - Buzzer alert
  - (Planned) Display screen
- IPC via Unix Domain Sockets (or FIFO/Message Queues)
- Separation of concerns and scalable design
- Clean modular C++17 codebase

---

## Class Design

### Core Components
| Class | Responsibility |
|-------|----------------|
| `DistanceReader` | Reads current distance from `/dev/distance0` |
| `DistanceClassifier` | Maps raw cm to enums (`FAR`, `MEDIUM`, `CLOSE`) |
| `SensorPublisher` | Reads distance and publishes to IPC sockets |
| `LedProcess` | Receives level, controls LED (color-specific) |
| `BuzzerProcess` | Receives level, controls buzzer sound |
| `UnixSocketServer/Client` | Handles IPC communication |
| `Logger` | Handles logging per process |
| `CommonDefs` | Contains constants and enum definitions |

### CommonDefs Example
```cpp
enum class DistanceLevel {
    FAR,
    MEDIUM,
    CLOSE
};

const int FAR_THRESHOLD = 100;
const int MEDIUM_THRESHOLD = 50;
```


           +--------------------+
           |  Kernel Module     |
           |  /dev/distance0    |
           +---------+----------+
                     |
                     v
           +--------------------+
           |  SensorPublisher   |  <-- Reads distance, broadcasts to clients
           +---------+----------+
                     |
         +-----------+----------+-------------------+
         |           |          |                   |
         v           v          v                   v
+--------------+ +-------------+ +------------+ +------------+
| led_green    | | led_yellow  | | led_red    | | buzzer     |
| process      | | process     | | process    | | process    |
+--------------+ +-------------+ +------------+ +------------+
     |                |               |             |
     v                v               v             v
    [GPIO]          [GPIO]         [GPIO]        [GPIO:PWM]

