#!/bin/bash

SERVICE_NAMES=("reverse-core" "reverse-led" "reverse-buzzer")

for SERVICE in "${SERVICE_NAMES[@]}"; do
    sudo systemctl stop "$SERVICE.service"
    sudo systemctl disable "$SERVICE.service"
    sudo systemctl status "$SERVICE.service"
done

echo "GPIO Removed:"
sudo lsof /dev/gpiochip0