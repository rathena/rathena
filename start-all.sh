#!/bin/bash
cd "$(dirname "$0")"

echo "Starting all services..."

# Start databases
echo "--- Starting PostgreSQL ---"
sudo systemctl start postgresql

echo "--- Starting Redis ---"
sudo systemctl start redis-server

sleep 2

# Start ML inference
echo "--- Starting ML Inference Service ---"
sudo systemctl start ml-inference

sleep 3

# Start game servers
echo "--- Starting Login Server ---"
./login-server > /dev/null 2>&1 &
echo $! > /tmp/login.pid
sleep 2

echo "--- Starting Char Server ---"
./char-server > /dev/null 2>&1 &
echo $! > /tmp/char.pid
sleep 2

echo "--- Starting Map Server ---"
./map-server > /dev/null 2>&1 &
echo $! > /tmp/map.pid
sleep 3

echo ""
echo "All services started"
echo ""
ps aux | grep -E "login-server|char-server|map-server" | grep -v grep
