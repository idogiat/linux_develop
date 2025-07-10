#!/bin/bash


set -e # stop if error raised

PROJECT_DIR="$(pwd)"
USER_NAME="$(whoami)"
SERVICE_NAME="reverse-core.service"
SERVICE_PATH="/etc/systemd/system/$SERVICE_NAME"
ENV_FILE="/etc/reverse_project.env"
SERVICE_NAMES=("reverse-core" "reverse-led" "reverse-buzzer")


echo "[0/6] make kernel module"
make  -C "$PROJECT_DIR/kernel"

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
for SERVICE in "${SERVICE_NAMES[@]}"; do
    TEMPLATE_FILE="services/${SERVICE}.service.template"
    TARGET_FILE="/etc/systemd/system/${SERVICE}.service"

    echo "[*] Installing $SERVICE..."

    sed "s|<PROJECT_DIR>|$PROJECT_DIR|g" "$TEMPLATE_FILE" > "$TARGET_FILE" 
done

sudo systemctl daemon-reload


echo "[5/6] Apply Services..."
for SERVICE in "${SERVICE_NAMES[@]}"; do
    sudo systemctl enable "$SERVICE.service"
    sudo systemctl restart "$SERVICE.service"
done


echo "[6/6] Done"
for SERVICE in "${SERVICE_NAMES[@]}"; do
    sudo systemctl status "$SERVICE.service"
done

echo "GPIO In Used:"
sudo lsof /dev/gpiochip0
