# AI IPC Service

Database-based Inter-Process Communication (IPC) polling service for rAthena NPC scripts.

## Overview

The AI IPC Service replaces HTTP-based NPC script commands (`httpget`, `httppost`) with a database-based IPC mechanism. This approach:

- **Eliminates external dependencies**: No need for curl, zmq, or hiredis libraries
- **Leverages existing infrastructure**: Uses the same MySQL database as rAthena
- **Provides reliability**: Database transactions ensure message delivery
- **Enables scalability**: Multiple service instances can process requests concurrently
- **Supports async operations**: NPC scripts can fire-and-forget or poll for responses

## Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        DATABASE-BASED IPC ARCHITECTURE                       │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐    ┌────────────┐ │
│  │  NPC Script  │───▶│ Native C++   │───▶│   MySQL DB   │◀──▶│  This      │ │
│  │  Commands    │    │ BUILDIN_FUNC │    │ (ai_requests │    │  Service   │ │
│  │              │◀───│              │◀───│ ai_responses)│    │  (Python)  │ │
│  └──────────────┘    └──────────────┘    └──────────────┘    └────────────┘ │
│                                                                              │
│  Flow:                                                                       │
│  1. NPC script calls ai_db_request()                                        │
│  2. C++ inserts request into ai_requests table                              │
│  3. This service polls and processes requests                               │
│  4. This service writes response to ai_responses table                      │
│  5. NPC script calls ai_db_response() to retrieve result                    │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Quick Start

### Prerequisites

- Python 3.11+ (3.9+ minimum)
- MySQL/MariaDB database with ai_ipc_tables.sql schema applied
- rAthena server with native IPC commands (ai_db_request, ai_db_response)

### Installation

```bash
# Navigate to service directory
cd rathena-AI-world/tools/ai_ipc_service

# Create virtual environment
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install dependencies
pip install -r requirements.txt

# Copy and configure settings
cp config.example.yaml config.yaml
# Edit config.yaml with your database credentials
```

### Running the Service

```bash
# Using Python module
python -m ai_ipc_service.main

# Or directly
python main.py

# With custom config file
python main.py --config /path/to/config.yaml

# With environment variables
DB_HOST=localhost DB_PASSWORD=secret python main.py
```

### Docker (Optional)

```dockerfile
FROM python:3.11-slim

WORKDIR /app
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY . .
CMD ["python", "-m", "ai_ipc_service.main"]
```

```bash
docker build -t ai-ipc-service .
docker run -e DB_HOST=host.docker.internal -e DB_PASSWORD=secret ai-ipc-service
```

## Configuration

### Configuration File (config.yaml)

```yaml
database:
  host: localhost
  port: 3306
  user: ragnarok
  password: ragnarok
  database: ragnarok
  pool_size: 5

polling:
  interval_ms: 100      # How often to poll for new requests
  batch_size: 50        # Max requests per poll
  worker_count: 4       # Concurrent request processors
  max_retries: 3        # Retry failed requests

ai_service:
  base_url: http://localhost:8000  # External AI service URL
  timeout_seconds: 10

logging:
  level: INFO           # DEBUG, INFO, WARNING, ERROR
  format: json          # json or text
```

### Environment Variables

Environment variables override config file settings:

| Variable | Config Path | Default |
|----------|-------------|---------|
| `AI_IPC_CONFIG` | - | config.yaml |
| `DB_HOST` | database.host | localhost |
| `DB_PORT` | database.port | 3306 |
| `DB_USER` | database.user | ragnarok |
| `DB_PASSWORD` | database.password | - |
| `DB_NAME` | database.database | ragnarok |
| `DB_POOL_SIZE` | database.pool_size | 5 |
| `POLL_INTERVAL_MS` | polling.interval_ms | 100 |
| `BATCH_SIZE` | polling.batch_size | 50 |
| `WORKER_COUNT` | polling.worker_count | 4 |
| `AI_SERVICE_BASE_URL` | ai_service.base_url | http://localhost:8000 |
| `AI_SERVICE_TIMEOUT` | ai_service.timeout_seconds | 10 |
| `LOG_LEVEL` | logging.level | INFO |
| `LOG_FORMAT` | logging.format | json |

## Request Handlers

### Health Check Handler
Handles request types: `health_check`, `health`

Returns service status information:
```json
{
  "status": "ok",
  "service": "ai_ipc_service",
  "version": "1.0.0",
  "timestamp": "2025-11-27T00:00:00.000000",
  "uptime_seconds": 3600
}
```

### HTTP Proxy Handler
Handles request types: `http_get`, `http_post`, `httpget`, `httppost`, `get`, `post`

Provides backward compatibility with httpget/httppost NPC commands:
- Reconstructs full URL from endpoint and configured base URL
- Forwards request to external service
- Returns JSON response

Request format:
```json
{
  "body": {"key": "value"},
  "headers": {"X-Custom": "header"},
  "timeout": 10
}
```

### AI Dialogue Handler
Handles request types: `dialogue`, `ai_dialogue`, `npc_dialogue`

Generates AI-powered NPC dialogue:
```json
{
  "npc_id": "merchant_001",
  "npc_name": "Alice the Merchant",
  "player_message": "Hello!",
  "context": {
    "player_level": 50,
    "player_class": "Knight"
  }
}
```

Response format:
```json
{
  "status": "ok",
  "npc_id": "merchant_001",
  "response": "Welcome, brave Knight!",
  "emotion": "friendly",
  "actions": [{"type": "emote", "value": "wave"}],
  "menu_options": ["Buy Items", "Sell Items", "Leave"]
}
```

## Database Schema

The service uses two main tables (created by `ai_ipc_tables.sql`):

### ai_requests
| Column | Type | Description |
|--------|------|-------------|
| id | BIGINT | Primary key |
| request_type | VARCHAR(50) | Handler routing key |
| endpoint | VARCHAR(255) | API endpoint path |
| request_data | TEXT | JSON payload |
| status | ENUM | pending/processing/completed/failed/timeout/cancelled |
| priority | TINYINT | 1=highest, 10=lowest |
| source_npc | VARCHAR(100) | NPC that made request |
| source_map | VARCHAR(50) | Map name |
| created_at | TIMESTAMP | Creation time |
| expires_at | TIMESTAMP | Expiration time |

### ai_responses
| Column | Type | Description |
|--------|------|-------------|
| id | BIGINT | Primary key |
| request_id | BIGINT | FK to ai_requests |
| response_data | TEXT | JSON response |
| http_status | SMALLINT | Status code (200, 400, 500) |
| error_message | VARCHAR(500) | Error details |
| processing_time_ms | INT | Processing duration |

## Development

### Project Structure

```
ai_ipc_service/
├── __init__.py           # Package initialization
├── main.py               # Entry point, polling loop
├── config.py             # Configuration management
├── config.example.yaml   # Example configuration
├── database.py           # Async MySQL operations
├── processor.py          # Request processing logic
├── handlers/
│   ├── __init__.py       # Handler registry
│   ├── base.py           # Base handler class
│   ├── health_check.py   # Health check handler
│   ├── http_proxy.py     # HTTP proxy handler
│   └── ai_dialogue.py    # AI dialogue handler
├── requirements.txt      # Python dependencies
└── README.md             # This file
```

### Adding a Custom Handler

1. Create a new handler file in `handlers/`:

```python
# handlers/my_handler.py
from .base import BaseHandler

class MyHandler(BaseHandler):
    async def handle(self, request: dict) -> dict:
        data = self.get_request_data(request)
        # Process request...
        return self.create_success_response(data={"result": "processed"})
```

2. Register in `handlers/__init__.py`:

```python
from .my_handler import MyHandler

HANDLER_REGISTRY["my_type"] = MyHandler

__all__ = [..., "MyHandler"]
```

### Testing

```bash
# Install test dependencies
pip install pytest pytest-asyncio pytest-cov

# Run tests
pytest tests/

# With coverage
pytest --cov=ai_ipc_service tests/
```

## Production Deployment

### Systemd Service

```ini
# /etc/systemd/system/ai-ipc-service.service
[Unit]
Description=AI IPC Service for rAthena
After=network.target mysql.service

[Service]
Type=simple
User=ragnarok
WorkingDirectory=/path/to/ai_ipc_service
Environment=DB_PASSWORD=your_password
ExecStart=/path/to/venv/bin/python -m ai_ipc_service.main
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```

```bash
sudo systemctl enable ai-ipc-service
sudo systemctl start ai-ipc-service
sudo systemctl status ai-ipc-service
```

### Supervisor

```ini
# /etc/supervisor/conf.d/ai-ipc-service.conf
[program:ai-ipc-service]
command=/path/to/venv/bin/python -m ai_ipc_service.main
directory=/path/to/ai_ipc_service
user=ragnarok
environment=DB_PASSWORD="your_password"
autostart=true
autorestart=true
stderr_logfile=/var/log/ai-ipc-service.err.log
stdout_logfile=/var/log/ai-ipc-service.out.log
```

### Monitoring

The service logs processing statistics every 100 polls:
```
Status: processed=1234, failed=5, rate=12.34/s
```

Final statistics on shutdown:
```
Final Statistics:
  Requests processed: 12345
  Requests failed: 23
  Requests expired: 5
  Batches processed: 500
  Uptime: 3600 seconds
  Avg rate: 3.43/s
```

## Troubleshooting

### Service won't start

1. Check database connectivity:
   ```bash
   mysql -h localhost -u ragnarok -p ragnarok -e "SELECT 1"
   ```

2. Verify tables exist:
   ```bash
   mysql -h localhost -u ragnarok -p ragnarok -e "SHOW TABLES LIKE 'ai_%'"
   ```

3. Check logs for errors:
   ```bash
   python -m ai_ipc_service.main 2>&1 | head -50
   ```

### Requests not being processed

1. Check for pending requests:
   ```sql
   SELECT COUNT(*) FROM ai_requests WHERE status = 'pending';
   ```

2. Check for processing requests (stuck):
   ```sql
   SELECT * FROM ai_requests WHERE status = 'processing' AND updated_at < NOW() - INTERVAL 5 MINUTE;
   ```

3. Reset stuck requests:
   ```sql
   UPDATE ai_requests SET status = 'pending' WHERE status = 'processing' AND updated_at < NOW() - INTERVAL 5 MINUTE;
   ```

### High latency

1. Reduce polling interval:
   ```yaml
   polling:
     interval_ms: 50  # From 100
   ```

2. Increase batch size and workers:
   ```yaml
   polling:
     batch_size: 100   # From 50
     worker_count: 8   # From 4
   ```

3. Increase database pool size:
   ```yaml
   database:
     pool_size: 10  # From 5
   ```

## License

This project is part of the rAthena AI World project.

## Version History

- **1.0.0** (2025-11-27): Initial release
  - Database polling service
  - Health check, HTTP proxy, and AI dialogue handlers
  - YAML/environment configuration
  - Graceful shutdown handling
  - Structured logging support