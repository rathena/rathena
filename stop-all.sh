#!/bin/bash

echo "Stopping all services..."

# Stop game servers
echo "--- Stopping rAthena Servers ---"
pkill -TERM login-server 2>/dev/null && echo "Login server stopped" || echo "Login server not running"
pkill -TERM char-server 2>/dev/null && echo "Char server stopped" || echo "Char server not running"
pkill -TERM map-server 2>/dev/null && echo "Map server stopped" || echo "Map server not running"

sleep 2

# Stop ML inference
echo "--- Stopping ML Inference Service ---"
sudo systemctl stop ml-inference

echo ""
echo "All services stopped"
