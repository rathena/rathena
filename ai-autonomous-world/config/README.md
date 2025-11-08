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
1. Create an Azure OpenAI resource in Azure Portal
2. Deploy a model (e.g., GPT-4)
3. Get your API key and endpoint from Azure Portal
4. Update the configuration:
   ```yaml
   llm:
     default_provider: "azure_openai"
     azure_openai:
       enabled: true
       api_key: "your-actual-key"
       endpoint: "https://your-resource.openai.azure.com/"
       deployment_name: "your-deployment-name"
   ```

### OpenAI
1. Sign up at https://platform.openai.com/
2. Create an API key
3. Update the configuration:
   ```yaml
   llm:
     default_provider: "openai"
     openai:
       enabled: true
       api_key: "sk-..."
   ```

### DeepSeek
1. Sign up at https://platform.deepseek.com/
2. Create an API key
3. Update the configuration:
   ```yaml
   llm:
     default_provider: "deepseek"
     deepseek:
       enabled: true
       api_key: "your-deepseek-key"
   ```

### Ollama (Local, Free)
1. Install Ollama: https://ollama.ai/
2. Pull a model: `ollama pull llama2`
3. Update the configuration:
   ```yaml
   llm:
     default_provider: "ollama"
     ollama:
       enabled: true
       base_url: "http://localhost:11434"
       model: "llama2"
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

