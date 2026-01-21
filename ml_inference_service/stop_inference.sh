#!/bin/bash
# Stop ML Inference Service

echo "=========================================="
echo "Stopping ML Inference Service"
echo "=========================================="

# Check if running via systemd
if systemctl is-active --quiet ml-inference 2>/dev/null; then
    echo "Stopping systemd service..."
    sudo systemctl stop ml-inference
    echo "✓ Service stopped"
    sudo systemctl status ml-inference --no-pager
else
    echo "Service not running via systemd"
    
    # Check for running Python process
    PID=$(pgrep -f "python.*main.py.*inference" || true)
    
    if [ -n "$PID" ]; then
        echo "Found running process (PID: $PID), sending SIGTERM..."
        kill -TERM $PID
        
        # Wait for graceful shutdown
        sleep 2
        
        # Check if still running
        if ps -p $PID > /dev/null 2>&1; then
            echo "Process still running, sending SIGKILL..."
            kill -KILL $PID
        fi
        
        echo "✓ Process terminated"
    else
        echo "No running inference service found"
    fi
fi

echo ""
echo "=========================================="
echo "Service stopped"
echo "=========================================="
