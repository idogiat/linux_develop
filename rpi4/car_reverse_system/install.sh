#!/bin/bash


set -e # stop if error raised

PROJECT_DIR="$(pwd)"
USER_NAME="$(whoami)"
SERVICE_NAME="reverse-core.service"
SERVICE_PATH="/etc/systemd/system/$SERVICE_NAME"
ENV_FILE="/etc/reverse_project.env"

echo "$PROJECT_DIR"
echo "$USER_NAME"

echo "[0/6] make kernel module"

if lsmod | grep -q "hc_sr04_driver"; then
    echo "kernel module already running..."
else
    make  -C "$PROJECT_DIR/kernel"
    sudo insmod "$PROJECT_DIR/kernel/hc_sr04_driver.ko"
fi

echo "[1/6] make project"
make  -C "$PROJECT_DIR" all


echo "[2/6] create named pipes..."
LEDS_PIPE_PATH="/tmp/leds_pipe"
BUZZER_PIPE_PATH="/tmp/buzzer_pipe"

[[ -p "$LEDS_PIPE_PATH" ]] || mkfifo "$LEDS_PIPE_PATH"
[[ -p "$BUZZER_PIPE_PATH" ]] || mkfifo "$BUZZER_PIPE_PATH"
sudo chmod 666 "$LEDS_PIPE_PATH"
sudo chmod 666 "$BUZZER_PIPE_PATH"


echo "[3/6] add permitions..."
chmod +x "$PROJECT_DIR/reverse_core.sh"
chmod +x "$PROJECT_DIR/SensorPublisher"
chmod +x "$PROJECT_DIR/WatchdogManager"
chmod +x "$PROJECT_DIR/LedService"
chmod +x "$PROJECT_DIR/BuzzerService"


echo "[4/6] install systemd file and loading..."
SERVICE_TEMPLATE="reverse-core.service.template"
SERVICE_FILE="/etc/systemd/system/reverse-core.service"
sed -e "s|<PROJECT_DIR>|$PROJECT_DIR|g" \
    -e "s|<USER_NAME>|$USER_NAME|g" \
    "$PROJECT_DIR/$SERVICE_TEMPLATE" > "$SERVICE_FILE" 

sudo systemctl daemon-reload


echo "[5/6] Apply Services..."
sudo systemctl enable "$SERVICE_NAME"
sudo systemctl restart "$SERVICE_NAME"

echo "[6/6] Done"
sudo systemctl status "$SERVICE_NAME"




