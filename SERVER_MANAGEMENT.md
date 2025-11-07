# 🔧 rAthena AI World - Server Management Guide

Quick reference for starting, stopping, and restarting all server components.

---

## 🚀 Quick Restart Commands

### **Restart Everything (Full Server Restart)**

```bash
# Stop all services
sudo systemctl stop rathena-map rathena-char rathena-login ai-world p2p-coordinator

# Start all services in correct order
sudo systemctl start rathena-login && sleep 2
sudo systemctl start rathena-char && sleep 2
sudo systemctl start rathena-map
sudo systemctl start ai-world p2p-coordinator

# Verify all services are running
sudo systemctl status 'rathena-*' ai-world p2p-coordinator
```

### **Restart Only Game Servers (rAthena)**

```bash
# Restart in correct order
sudo systemctl restart rathena-login && sleep 2
sudo systemctl restart rathena-char && sleep 2
sudo systemctl restart rathena-map

# Check status
sudo systemctl status 'rathena-*'
```

### **Restart Only AI Services**

```bash
sudo systemctl restart ai-world p2p-coordinator
sudo systemctl status ai-world p2p-coordinator
```

### **Restart Individual Services**

```bash
# Login server
sudo systemctl restart rathena-login

# Character server
sudo systemctl restart rathena-char

# Map server
sudo systemctl restart rathena-map

# AI World service
sudo systemctl restart ai-world

# P2P Coordinator
sudo systemctl restart p2p-coordinator
```

---

## 🛑 Stop Server

### **Stop All Services**

```bash
sudo systemctl stop rathena-map rathena-char rathena-login ai-world p2p-coordinator
```

### **Stop Only Game Servers**

```bash
sudo systemctl stop rathena-map rathena-char rathena-login
```

### **Stop Only AI Services**

```bash
sudo systemctl stop ai-world p2p-coordinator
```

---

## ▶️ Start Server

### **Start All Services (Correct Order)**

```bash
# 1. Ensure databases are running
sudo systemctl start postgresql mariadb dragonfly

# 2. Start rAthena servers (in order)
sudo systemctl start rathena-login && sleep 2
sudo systemctl start rathena-char && sleep 2
sudo systemctl start rathena-map

# 3. Start AI services
sudo systemctl start ai-world p2p-coordinator
```

### **Start Only Game Servers**

```bash
sudo systemctl start rathena-login && sleep 2
sudo systemctl start rathena-char && sleep 2
sudo systemctl start rathena-map
```

### **Start Only AI Services**

```bash
sudo systemctl start ai-world p2p-coordinator
```

---

## 📊 Check Server Status

### **Check All Services**

```bash
sudo systemctl status 'rathena-*' ai-world p2p-coordinator
```

### **Quick Status Check (One-liner)**

```bash
sudo systemctl is-active postgresql mariadb dragonfly rathena-login rathena-char rathena-map ai-world p2p-coordinator
```

### **Check Individual Service**

```bash
sudo systemctl status rathena-login
sudo systemctl status rathena-char
sudo systemctl status rathena-map
sudo systemctl status ai-world
sudo systemctl status p2p-coordinator
```

### **Check Database Services**

```bash
sudo systemctl status postgresql mariadb dragonfly
```

---

## 📝 View Logs

### **Real-time Logs (All Services)**

```bash
sudo journalctl -f
```

### **Real-time Logs (Specific Service)**

```bash
# Login server
sudo journalctl -u rathena-login -f

# Character server
sudo journalctl -u rathena-char -f

# Map server
sudo journalctl -u rathena-map -f

# AI World service
sudo journalctl -u ai-world -f

# P2P Coordinator
sudo journalctl -u p2p-coordinator -f
```

### **View Last 50 Lines**

```bash
sudo journalctl -u rathena-map -n 50 --no-pager
```

### **View Logs Since Boot**

```bash
sudo journalctl -u rathena-map -b
```

### **View Logs for Specific Time**

```bash
# Last hour
sudo journalctl -u rathena-map --since "1 hour ago"

# Last 30 minutes
sudo journalctl -u rathena-map --since "30 minutes ago"

# Today
sudo journalctl -u rathena-map --since today
```

---

## 🔍 Check Listening Ports

### **Check All Game Ports**

```bash
sudo ss -tlnp | grep -E ":(6900|6121|5121|8000|8001)"
```

Expected output:
```
0.0.0.0:6900    # Login server
0.0.0.0:6121    # Char server
0.0.0.0:5121    # Map server
0.0.0.0:8000    # AI World service
0.0.0.0:8001    # P2P Coordinator
```

### **Check Database Ports**

```bash
sudo ss -tlnp | grep -E ":(5432|3306|6379)"
```

---

## 🔄 Reload Configuration

### **Reload NPC Scripts (Without Restart)**

In-game with GM account:
```
@reloadscript
```

Or via console:
```bash
# This requires the map server to be running
# Connect to map server console if available
```

### **Reload Item Database**

In-game with GM account:
```
@reloaditemdb
```

### **Reload Monster Database**

In-game with GM account:
```
@reloadmobdb
```

---

## 🚨 Troubleshooting

### **Service Won't Start**

```bash
# Check service status for errors
sudo systemctl status rathena-map

# View detailed logs
sudo journalctl -u rathena-map -n 100 --no-pager

# Check if port is already in use
sudo ss -tlnp | grep 5121

# Reset failed state
sudo systemctl reset-failed rathena-map

# Try starting again
sudo systemctl start rathena-map
```

### **Database Connection Issues**

```bash
# Check database services
sudo systemctl status postgresql mariadb dragonfly

# Test MariaDB connection
mysql -h 127.0.0.1 -u ragnarok -p'ragnarok' ragnarok -e "SELECT 1;"

# Test PostgreSQL connection
psql -h localhost -U ai_world_user -d ai_world_memory -c "SELECT 1;"

# Restart databases if needed
sudo systemctl restart postgresql mariadb dragonfly
```

### **Players Can't Connect**

```bash
# Check if login server is running
sudo systemctl status rathena-login

# Check if port is listening
sudo ss -tlnp | grep 6900

# Check firewall
sudo ufw status

# View login server logs
sudo journalctl -u rathena-login -f
```

---

## 🔧 Advanced Management

### **Enable/Disable Auto-start on Boot**

```bash
# Enable (already enabled by default)
sudo systemctl enable rathena-login rathena-char rathena-map ai-world p2p-coordinator

# Disable
sudo systemctl disable rathena-login rathena-char rathena-map ai-world p2p-coordinator
```

### **Reload Systemd Configuration**

After editing service files:
```bash
sudo systemctl daemon-reload
```

### **View Service Configuration**

```bash
systemctl cat rathena-map
```

---

## 📋 Common Scenarios

### **After Configuration Changes**

```bash
# Restart affected service
sudo systemctl restart rathena-map

# Or restart all
sudo systemctl restart rathena-login rathena-char rathena-map
```

### **After Database Changes**

```bash
# Usually no restart needed, but if required:
sudo systemctl restart rathena-login rathena-char rathena-map
```

### **After AI Service Code Changes**

```bash
sudo systemctl restart ai-world
```

### **After System Reboot**

All services start automatically. Verify with:
```bash
sudo systemctl is-active postgresql mariadb dragonfly rathena-login rathena-char rathena-map ai-world p2p-coordinator
```

---

## 🎯 One-Command Solutions

### **Full Server Restart (Copy-Paste Ready)**

```bash
sudo systemctl stop rathena-map rathena-char rathena-login ai-world p2p-coordinator && \
sleep 2 && \
sudo systemctl start rathena-login && sleep 2 && \
sudo systemctl start rathena-char && sleep 2 && \
sudo systemctl start rathena-map && \
sudo systemctl start ai-world p2p-coordinator && \
echo "=== Server Restart Complete ===" && \
sudo systemctl is-active rathena-login rathena-char rathena-map ai-world p2p-coordinator
```

### **Quick Status Check (Copy-Paste Ready)**

```bash
echo "=== Service Status ===" && \
sudo systemctl is-active postgresql mariadb dragonfly rathena-login rathena-char rathena-map ai-world p2p-coordinator && \
echo "" && \
echo "=== Listening Ports ===" && \
sudo ss -tlnp | grep -E ":(6900|6121|5121|8000|8001)"
```

---

**For more detailed information, see `DEPLOYMENT_COMPLETE_GUIDE.md`**

