# Distance Sensing System with Multi-Process Architecture on Raspberry Pi

## Overview
This project implements a modular, multi-process system on a Raspberry Pi to measure distances using the HC-SR04 ultrasonic sensor. It reacts to distance changes using LEDs and a buzzer, and is designed to be robust, extendable, and maintainable. The system includes a kernel module, inter-process communication, and process isolation per hardware component.

## Features
- Kernel module for real-time HC-SR04 distance sensing
- IPC via Named Pipes Broadcast  (Observer Design Pattern)
- Multiple user-space processes for individual components:
  - LED indicators (green, yellow, red)
  - Buzzer alert
- Separation of concerns and scalable design
- Running as linux services. 
- Clean modular C++17 codebase.

---

## Class Design

### Core Components
| Class | Responsibility |
|-------|----------------|
| `hc_sr04_driver` | Reads current distance from `/dev/distance0` |
| `SensorPublisher` | Reads distance and publishes to IPC sockets |
| `LedProcess` | Receives level, controls LED (color-specific) |
| `BuzzerProcess` | Receives level, controls buzzer sound |
| `msg_protocol` | Maps raw cm to enums (`FAR`, `MEDIUM`, `CLOSE`) |


## Diagram

```
                           +-----------------------+
                           |   /dev/distance0      |
                           |   (Kernel Module)     |
                           +-----------+-----------+
                                       |
                                       v
                           +------------------------+
                           |    SensorPublisher     |
                           |   (Master Process)     |
                           +-----------+------------+
                                       |
                              Named Pipes / Broadcast
                                       |
         +-----------------------------+-----------------------------+
         |                                                           |
         v                                                           v
  +-------------------+                                   +--------------------+
  |   LED Service     |                                   |   Buzzer Service   |
  |   (reverse-led)   |                                   | (reverse-buzzer)   |
  +-------------------+                                   +--------------------+

```



## HC-SR04 conection GPIO 
[example](https://pimylifeup.com/raspberry-pi-distance-sensor/)

| Component        | Raspberry Pi GPIO (BCM) | Pin Number | Description            |
| ---------------- | ----------------------- | ---------- | ---------------------- |
| **HC-SR04 VCC**  | 5V Power                | Pin 2 or 4 | Power supply           |
| **HC-SR04 GND**  | GND                     | Pin 6 or 9 | Ground                 |
| **HC-SR04 TRIG** | GPIO 23                 | Pin 16     | Trigger pin (output)   |
| **HC-SR04 ECHO** | GPIO 24                 | Pin 18     | Echo pin (input)       |
| **Buzzer**       | GPIO 12                 | Pin 32     | Buzzer signal (output) |
| **LED - Red**    | GPIO 17                 | Pin 11     | Close range indicator  |
| **LED - Yellow** | GPIO 27                 | Pin 13     | Medium range indicator |
| **LED - Green**  | GPIO 22                 | Pin 15     | Far range indicator    |


## how to run
```
sudo ./install.sh
```
- compile kernel module, and services.
- add permitions
- load services and start them.

```
./uninstall.sh
```
- stop services

to cleanup build files run `make clean`