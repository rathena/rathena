#!/bin/bash

echo "=== Service Status ==="

echo -e "\n--- Game Servers ---"
ps aux | grep -E "login-server|char-server|map-server" | grep -v grep || echo "No game servers running"

echo -e "\n--- ML Inference Service ---"
sudo systemctl status ml-inference --no-pager | head -10

echo -e "\n--- Database Services ---"
echo "PostgreSQL: $(sudo systemctl is-active postgresql)"
echo "Redis: $(sudo systemctl is-active redis-server)"

echo -e "\n--- ML Metrics ---"
curl -s http://localhost:9090/metrics 2>/dev/null | grep -E "ml_service_up|ml_fallback_level|ml_requests_total" || echo "Metrics unavailable"

echo -e "\n--- Database IPC Status ---"
PGPASSWORD=ml_secure_password_2026 psql -h localhost -U ml_user -d ragnarok_ml -c "SELECT COUNT(*) as requests FROM ai_requests;" 2>/dev/null | tail -2 || echo "Database IPC unavailable"
