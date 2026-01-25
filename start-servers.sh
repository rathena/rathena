#!/bin/bash
# ========================================
# Ragnarok Online Server Startup Script
# Network: 192.168.0.100
# ========================================

set -e

BASE_DIR="/home/lot399/RagnarokOnlineServer/rathena-AI-world"
cd "$BASE_DIR"

echo "========================================"
echo "Ragnarok Online Server Startup"
echo "========================================"
echo "Network: 192.168.0.100"
echo "Time: $(date)"
echo ""

# Function to check if a process is running
is_running() {
    pgrep -f "$1" > /dev/null 2>&1
}

# Function to wait for port
wait_for_port() {
    local port=$1
    local timeout=30
    local counter=0
    echo "  Waiting for port $port..."
    while ! nc -z 192.168.0.100 $port 2>/dev/null; do
        sleep 1
        counter=$((counter + 1))
        if [ $counter -ge $timeout ]; then
            echo "  ✗ Timeout waiting for port $port"
            return 1
        fi
    done
    echo "  ✓ Port $port is open"
    return 0
}

# Stop any existing servers
echo "Step 1: Stopping existing servers..."
pkill -f "login-server" 2>/dev/null && echo "  ✓ Stopped login-server" || echo "  - login-server not running"
pkill -f "char-server" 2>/dev/null && echo "  ✓ Stopped char-server" || echo "  - char-server not running"
pkill -f "map-server" 2>/dev/null && echo "  ✓ Stopped map-server" || echo "  - map-server not running"
sleep 2
echo ""

# Verify database connectivity
echo "Step 2: Verifying database connectivity..."
if mysql -h 192.168.0.100 -u ragnarok -pfOsh2AQ5Y74mv8M9wbDA0g2Cb19ya6uZ -e "USE ragnarok; SELECT 1;" > /dev/null 2>&1; then
    echo "  ✓ Database connection successful"
else
    echo "  ✗ Database connection failed"
    exit 1
fi
echo ""

# Start Login Server
echo "Step 3: Starting Login Server..."
if is_running "login-server"; then
    echo "  - Login server already running"
else
    nohup ./login-server > log/login-server.log 2>&1 &
    sleep 2
    if is_running "login-server"; then
        echo "  ✓ Login server started (PID: $(pgrep -f login-server))"
        wait_for_port 6900
    else
        echo "  ✗ Failed to start login server"
        echo "  Check log/login-server.log for details"
        tail -20 log/login-server.log
        exit 1
    fi
fi
echo ""

# Start Character Server
echo "Step 4: Starting Character Server..."
if is_running "char-server"; then
    echo "  - Character server already running"
else
    nohup ./char-server > log/char-server.log 2>&1 &
    sleep 2
    if is_running "char-server"; then
        echo "  ✓ Character server started (PID: $(pgrep -f char-server))"
        wait_for_port 6121
    else
        echo "  ✗ Failed to start character server"
        echo "  Check log/char-server.log for details"
        tail -20 log/char-server.log
        exit 1
    fi
fi
echo ""

# Start Map Server
echo "Step 5: Starting Map Server..."
if is_running "map-server"; then
    echo "  - Map server already running"
else
    nohup ./map-server > log/map-server.log 2>&1 &
    sleep 3
    if is_running "map-server"; then
        echo "  ✓ Map server started (PID: $(pgrep -f map-server))"
        wait_for_port 5121
    else
        echo "  ✗ Failed to start map server"
        echo "  Check log/map-server.log for details"
        tail -20 log/map-server.log
        exit 1
    fi
fi
echo ""

# Verify all services
echo "Step 6: Verifying all services..."
netstat -tuln | grep -E ":(6900|6121|5121|3306)" | grep "192.168.0.100"
echo ""

echo "========================================"
echo "Server Status Summary"
echo "========================================"
if is_running "login-server"; then
    echo "✓ Login Server: RUNNING (192.168.0.100:6900)"
else
    echo "✗ Login Server: STOPPED"
fi

if is_running "char-server"; then
    echo "✓ Character Server: RUNNING (192.168.0.100:6121)"
else
    echo "✗ Character Server: STOPPED"
fi

if is_running "map-server"; then
    echo "✓ Map Server: RUNNING (192.168.0.100:5121)"
else
    echo "✗ Map Server: STOPPED"
fi

echo "✓ Database Server: RUNNING (192.168.0.100:3306)"
echo ""
echo "All servers started successfully!"
echo "Check individual log files for detailed information:"
echo "  - log/login-server.log"
echo "  - log/char-server.log"
echo "  - log/map-server.log"
echo ""
