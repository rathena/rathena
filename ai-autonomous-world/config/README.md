# AI Service Configuration Guide

## Quick Start

1. **Copy the configuration file:**
   ```bash
   cp ai-service-config.yaml ai-service-config.yaml.example
   ```

2. **Edit `ai-service-config.yaml` and replace placeholder values:**
   - `YOUR_AZURE_OPENAI_API_KEY_HERE` → Your actual Azure OpenAI API key
   - `YOUR_OPENAI_API_KEY_HERE` → Your actual OpenAI API key
   - `YOUR_POSTGRES_PASSWORD_HERE` → Your PostgreSQL password
   - Update endpoints and deployment names as needed

3. **Choose your LLM provider:**
   - Set `llm.default_provider` to your preferred provider
   - Enable the corresponding provider section
   - Disable unused providers

## LLM Provider Setup

### Azure OpenAI (Recommended for Production)

**IMPORTANT: Never commit API keys to version control!**

1. Create an Azure OpenAI resource in Azure Portal
2. Deploy a model (e.g., GPT-4)
3. Get your API key and endpoint from Azure Portal
4. **Set your credentials in the `.env` file** (located at `ai-autonomous-world/ai_service/.env`):
   ```bash
   # Azure OpenAI Configuration
   AZURE_OPENAI_API_KEY=your-actual-api-key-here
   AZURE_OPENAI_ENDPOINT=https://your-resource.openai.azure.com
   AZURE_OPENAI_DEPLOYMENT_NAME=gpt-4.1-nano
   AZURE_OPENAI_API_VERSION=2024-02-15-preview
   ```
5. The YAML config file uses placeholders that reference these environment variables
6. **Never edit the YAML file to add your API key** - it may be committed to Git

### OpenAI
1. Sign up at https://platform.openai.com/
2. Create an API key
3. **Set your credentials in the `.env` file**:
   ```bash
   # OpenAI Configuration
   PRIMARY_LLM_PROVIDER=openai
   OPENAI_API_KEY=sk-your-actual-api-key-here
   OPENAI_MODEL=gpt-4-turbo-preview
   OPENAI_TEMPERATURE=0.7
   OPENAI_MAX_TOKENS=2000
   OPENAI_TIMEOUT=30
   OPENAI_ENABLED=true
   ```

### DeepSeek (Cost-Effective Alternative)
1. Sign up at https://platform.deepseek.com/
2. Create an API key
3. **Set your credentials in the `.env` file**:
   ```bash
   # DeepSeek Configuration
   PRIMARY_LLM_PROVIDER=deepseek
   DEEPSEEK_API_KEY=sk-your-actual-api-key-here
   DEEPSEEK_BASE_URL=https://api.deepseek.com/v1
   DEEPSEEK_MODEL=deepseek-chat
   DEEPSEEK_TEMPERATURE=0.7
   DEEPSEEK_MAX_TOKENS=2000
   DEEPSEEK_TIMEOUT=30
   DEEPSEEK_ENABLED=true
   ```

### Ollama (Local, Free)
1. Install Ollama: https://ollama.ai/
2. Pull a model: `ollama pull llama2`
3. **Set your configuration in the `.env` file**:
   ```bash
   # Ollama Configuration (Local)
   PRIMARY_LLM_PROVIDER=ollama
   OLLAMA_BASE_URL=http://localhost:11434
   OLLAMA_MODEL=llama2
   OLLAMA_TEMPERATURE=0.7
   OLLAMA_TIMEOUT=60
   OLLAMA_ENABLED=true
   ```

### Google Gemini
1. Sign up at https://ai.google.dev/
2. Create an API key
3. **Set your credentials in the `.env` file**:
   ```bash
   # Google Gemini Configuration
   PRIMARY_LLM_PROVIDER=google_gemini
   GOOGLE_API_KEY=your-actual-api-key-here
   GOOGLE_GEMINI_MODEL=gemini-pro
   GOOGLE_GEMINI_TEMPERATURE=0.7
   GOOGLE_GEMINI_MAX_TOKENS=2000
   GOOGLE_GEMINI_TIMEOUT=30
   GOOGLE_GEMINI_ENABLED=true
   ```

### Anthropic Claude
1. Sign up at https://console.anthropic.com/
2. Create an API key
3. **Set your credentials in the `.env` file**:
   ```bash
   # Anthropic Claude Configuration
   PRIMARY_LLM_PROVIDER=anthropic
   ANTHROPIC_API_KEY=sk-ant-your-actual-api-key-here
   ANTHROPIC_MODEL=claude-3-opus-20240229
   ANTHROPIC_TEMPERATURE=0.7
   ANTHROPIC_MAX_TOKENS=2000
   ANTHROPIC_TIMEOUT=30
   ANTHROPIC_ENABLED=true
   ```

## Database Configuration

### DragonflyDB (Redis-compatible)
**Set your configuration in the `.env` file**:
```bash
# DragonflyDB Configuration
DRAGONFLY_HOST=localhost
DRAGONFLY_PORT=6379
DRAGONFLY_DB=0
DRAGONFLY_PASSWORD=  # Leave empty if no authentication
DRAGONFLY_MAX_CONNECTIONS=50
DRAGONFLY_SOCKET_TIMEOUT=5
DRAGONFLY_SOCKET_CONNECT_TIMEOUT=5
```

### PostgreSQL
**Set your configuration in the `.env` file**:
```bash
# PostgreSQL Configuration
POSTGRES_HOST=localhost
POSTGRES_PORT=5432
POSTGRES_DB=ai_world_memory
POSTGRES_USER=ai_world_user
POSTGRES_PASSWORD=your-secure-password-here
POSTGRES_MIN_CONNECTIONS=5
POSTGRES_MAX_CONNECTIONS=20
```

## PostgreSQL Setup

### Requirements
- PostgreSQL 17
- Extensions: pgvector, TimescaleDB, Apache AGE

### Installation (Ubuntu/Debian)
```bash
# Install PostgreSQL 17
sudo apt-get update
sudo apt-get install postgresql-17

# Install pgvector
sudo apt-get install postgresql-17-pgvector

# Install TimescaleDB
sudo apt-get install timescaledb-2-postgresql-17

# Install Apache AGE (follow official guide)
# https://age.apache.org/age-manual/master/intro/setup.html
```

### Database Setup
```bash
# Create database
sudo -u postgres createdb ai_world_memory

# Enable extensions
sudo -u postgres psql ai_world_memory << EOF
CREATE EXTENSION IF NOT EXISTS vector;
CREATE EXTENSION IF NOT EXISTS timescaledb;
CREATE EXTENSION IF NOT EXISTS age;
EOF
```

### Configuration
Update `ai-service-config.yaml`:
```yaml
postgres:
  host: "localhost"
  port: 5432
  database: "ai_world_memory"
  user: "postgres"
  password: "your-secure-password"
```

## Environment Variables (Alternative to YAML)

You can also configure using environment variables:

```bash
# LLM Configuration
export AI_LLM_PROVIDER="azure_openai"
export AZURE_OPENAI_API_KEY="your-key"
export AZURE_OPENAI_ENDPOINT="https://your-resource.openai.azure.com/"

# Database Configuration
export DRAGONFLY_HOST="localhost"
export DRAGONFLY_PORT="6379"
export POSTGRES_HOST="localhost"
export POSTGRES_PASSWORD="your-password"

# Service Configuration
export AI_SERVICE_PORT="8000"
export AI_SERVICE_DEBUG="false"
export AI_SERVICE_ENVIRONMENT="production"
```

## Security Best Practices

1. **Never commit API keys to version control**
   - Add `ai-service-config.yaml` to `.gitignore`
   - Use environment variables in production

2. **Use different keys for different environments**
   - Development: Lower-tier API keys
   - Production: Enterprise-grade keys with rate limits

3. **Rotate API keys regularly**
   - Set up key rotation schedule
   - Monitor API usage

4. **Secure PostgreSQL**
   - Use strong passwords
   - Enable SSL connections
   - Restrict network access

## Troubleshooting

### "LLM provider not configured" error
- Check that your API key is set correctly
- Verify the provider is enabled in the config
- Test API key with a simple curl request

### "Cannot connect to PostgreSQL" error
- Verify PostgreSQL is running: `sudo systemctl status postgresql`
- Check connection settings in config
- Verify database exists: `sudo -u postgres psql -l`

### "Redis connection failed" error
- Verify DragonflyDB is running: `ps aux | grep dragonfly`
- Check port 6379 is not in use by another service
- Test connection: `redis-cli ping`

## Performance Tuning

### For High Load (1000+ concurrent users)
```yaml
performance:
  http_pooling:
    max_connections: 500
  redis_caching:
    max_cache_size: 50000
  batch_processing:
    default_batch_size: 20

llm:
  azure_openai:
    timeout: 60
    max_tokens: 1000  # Reduce for faster responses
```

### For Quality (Lower load, better responses)
```yaml
autonomous_features:
  llm_optimization:
    mode: "quality"
  universal_consciousness:
    reasoning_depth: "deep"

llm:
  azure_openai:
    temperature: 0.9
    max_tokens: 4000
```

