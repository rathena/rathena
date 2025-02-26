# rAthena AI World

## Overview
Advanced AI-driven gameplay enhancement and P2P network security system for rAthena, providing intelligent NPCs, dynamic world evolution, and robust protection.

## ✨ Implemented Features

### AI Agents

#### AI Legends System
- 36 unique AI Legend characters representing each final job class
- Progressive character development from novice to final job classes
- Contrasting MBTI personalities for unique character interactions
- Detailed backstories and relationships between characters
- MVP battle assistance for the 5 weakest parties
- 1v1 PvP duels with same-class players
- Global chat interactions between AI characters
- Secret skills that can be taught to worthy players

#### The Nameless Beggar
- Mysterious master character connecting all AI Legends
- Daily city-roaming system visiting different locations
- Food request and reward system encouraging player interaction
- Feeding streak system rewarding consistent interaction
- Special event triggered by collecting 100 Ancient Scroll Fragments
- Epic grand PK fight between all AI Legends and their master lasting 8-18 minutes
- Master's victory through perfect skill combinations from all job classes
- Narrative connection revealing all AI Legends as the Beggar's disciples

#### Mayor AI Agent
- Weekly server statistics analysis
- Dynamic event creation based on player behavior
- Player retention and acquisition strategies
- Comprehensive reporting on server health and trends
- Tailored events for different player segments
- Event creation system with various types and difficulties
- Integration with other AI systems for coordinated experiences

#### AI Integration
- Support for Azure OpenAI, OpenAI, and DeepSeek V3 providers
- LangChain integration for context memory management
- Dynamic configuration systems for each AI character
- Fallback mechanisms for offline operation

### Network Security
- Thread-safe packet monitoring
- Resource usage tracking
- Performance metrics collection
- Host statistics monitoring
- SQL injection prevention
- Memory safety improvements
- Basic DDoS protection

### Monitoring System
- Real-time connection tracking
- Resource usage monitoring
- Performance metrics collection
- Security event logging
- Host status tracking

### Development Tools
- Cross-platform build system
- Comprehensive test suite
- Memory leak detection
- Thread safety verification
- Performance testing

## 🚀 Getting Started

### Prerequisites
- CMake 3.15+
- C++17 compliant compiler
- MySQL 5.7+
- libconfig
- nlohmann-json
- Azure OpenAI, OpenAI, or DeepSeek API key (for AI features)

### Building

#### Linux
```bash
# Build and test
./test_build.sh

# Build only
mkdir build && cd build
cmake -DBUILD_TESTING=ON ..
make
```

#### Windows
```batch
# Build and test
test_build.bat

# Build only
mkdir build && cd build
cmake -DBUILD_TESTING=ON -G "Visual Studio 16 2019" ..
cmake --build . --config Release
```

## 🔧 Configuration

### AI Providers Configuration
```ini
ai_providers: {
    azure_openai: {
        enabled: true
        api_key: "your-api-key"
        endpoint: "your-endpoint"
        deployment: "gpt-4o"
        api_version: "2023-05-15"
    }
    openai: {
        enabled: true
        api_key: "your-api-key"
        model: "gpt-4o"
    }
    deepseek: {
        enabled: true
        api_key: "your-api-key"
        model: "deepseek-v3"
    }
}
```

### AI Legends Configuration
```ini
ai_legends_enabled: true
ai_legends_provider: "azure_openai"
ai_legends_model: "gpt-4o"
ai_legends_power_advantage: 8
```

### Beggar Configuration
```ini
ai_beggar_enabled: true
ai_beggar_provider: "azure_openai"
ai_beggar_model: "gpt-4o"
ai_beggar_cities: ["prontera", "geffen", "payon", "morocc", "alberta"]
```

### Mayor Configuration
```ini
ai_mayor_enabled: true
ai_mayor_provider: "azure_openai"
ai_mayor_model: "gpt-4o"
ai_mayor_analysis_frequency: "weekly"
```

### Network Monitor Settings
```ini
network_monitor: {
    enabled: true
    update_interval: 60
    cleanup_interval: 300
}
```

### Security Settings
```ini
security: {
    ddos_protection: {
        enabled: true
        packet_rate: 1000
        connection_rate: 100
    }
}
```

## 📊 Monitoring

### AI System Monitoring
- Character interactions
- Player engagement metrics
- Event participation
- AI response performance
- Memory usage statistics

### Performance Metrics
- Packet rates
- Bandwidth usage
- Connection counts
- Resource utilization
- Security events

### Host Statistics
- CPU usage
- Memory usage
- Network usage
- Connected players
- System status

## 🔒 Security Features

### Protection
- SQL injection prevention
- Memory safety
- Thread safety
- Resource management
- Input validation

### Monitoring
- Security event logging
- Resource tracking
- Connection monitoring
- Performance tracking
- Alert generation

## 🧪 Testing

### Running Tests
```bash
# Full test suite
cd build/src/test
./network_tests

# Specific tests
./network_tests --gtest_filter=NetworkMonitorTest.*
./network_tests --gtest_filter=SyncTest.*
./network_tests --gtest_filter=P2PConfigParserTest.*
```

### Memory Testing
```bash
# Linux (Valgrind)
valgrind --leak-check=full ./network_tests

# Windows (Dr. Memory)
drmemory.exe -light -- network_tests.exe
```

## 📚 Documentation

### AI Systems
- [AI Legends System](docs/AI_LEGENDS.md)
- [AI Legends Profiles](docs/AI_LEGENDS_PROFILES.md)
- [Nameless Beggar System](docs/AI_BEGGAR.md)
- [Mayor Agent System](docs/AI_MAYOR.md)
- [AI Development Roadmap](docs/AI_DEVELOPMENT_ROADMAP.md)
- [AI Agents Overview](docs/AI_AGENTS.md)
- [AI Implementation Summary](docs/AI_IMPLEMENTATION_SUMMARY.md)
- [AI Agents Optimization](docs/AI_AGENTS_OPTIMIZATION.md)

### Network & Security
- [Security Review](docs/SECURITY_REVIEW.md)
- [P2P Hosting Guide](docs/P2P_HOSTING.md)
- [Future Features](docs/FUTURE_FEATURES.md)
- [Feature Details](docs/FEATURE_DETAILS.md)

## 🔜 Upcoming Features

### AI Systems
- Cross-Class Synthesis system
- Dynamic World Evolution
- Legend Bloodlines system
- Economic Ecosystem
- Social Dynamics system
- Advanced Combat Mechanics
- Personal Housing system
- Time Manipulation system
- Guild Evolution system
- Dimensional Warfare

### Security
- TLS implementation
- Advanced DDoS protection
- Certificate management
- Packet encryption

### Monitoring
- Real-time dashboard
- Advanced analytics
- Predictive monitoring
- Custom metrics

### Performance
- Advanced caching
- Load balancing
- Dynamic scaling
- Resource optimization

## 🤝 Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Run tests (`./test_build.sh` or `test_build.bat`)
4. Commit changes (`git commit -m 'Add amazing feature'`)
5. Push to branch (`git push origin feature/amazing-feature`)
6. Open a Pull Request

## 📄 License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.
