#!/bin/bash

set -e

# Paths
PROJECT_DIR="$(dirname "$(realpath "$0")")"

SENSOR_PUBLISHER="$PROJECT_DIR/SensorPublisher"
WATCHDOG="$PROJECT_DIR/WatchdogManager"


# Start sensor_publisher
echo "[INFO] Starting sensor_publisher..."
"$SENSOR_PUBLISHER" &
PUBLISHER_PID=$!

# Start watchdog with sensor_publisher PID
echo "[INFO] Starting watchdog..."
"$WATCHDOG" "$PUBLISHER_PID" &
WATCHDOG_PID=$!

# Wait for both processes
wait $PUBLISHER_PID
wait $WATCHDOG_PID