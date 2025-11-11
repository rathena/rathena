#!/bin/bash

echo "Starting AI Service..."

cd /home/lot399/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world/ai-service

# Activate virtual environment
source ../venv/bin/activate

# Start the service
python main.py

