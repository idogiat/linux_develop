#!/bin/bash

set -e

# Paths
PROJECT_DIR="$(dirname "$(realpath "$0")")"
SENSOR_PUBLISHER="$PROJECT_DIR/SensorPublisher"


# Start sensor_publisher
echo "[INFO] Starting sensor_publisher..."
"$SENSOR_PUBLISHER" &
PUBLISHER_PID=$!

# Wait for both processes
wait $PUBLISHER_PID
