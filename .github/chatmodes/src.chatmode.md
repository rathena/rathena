# rAthena Source Code (src/) - ChatMode

## Overview
O diretório `src/` contém todo o código-fonte C++ do servidor rAthena, organizado em módulos específicos para cada servidor e funcionalidades compartilhadas. A arquitetura segue o padrão de três servidores independentes que se comunicam via TCP/IP.

## Estrutura de Servidores

### 1. Login Server (`src/login/`)
**Responsabilidade:** Autenticação de contas e gerenciamento de logins
- **Arquivos principais:**
  - `account.cpp/hpp` - Engine de persistência para dados de conta (SQL/TXT)
  - `ipban.cpp/hpp` - Sistema de banimento automático por IP
  - `login.cpp/hpp` - Módulo principal do login-server, lógica de autenticação
  - `loginclif.cpp/hpp` - Client Interface - comunicação cliente ⟷ login-server
  - `loginchrif.cpp/hpp` - Char Interface - comunicação char-server ⟷ login-server  
  - `logincnslif.cpp/hpp` - Console Interface - comandos do console
  - `loginlog.cpp/hpp` - Sistema de logs específico do login-server

### 2. Character Server (`src/char/`)
**Responsabilidade:** Gerenciamento de personagens, persistência de dados e inter-server communication
- **Arquivos principais:**
  - `char.cpp/hpp` - Processo principal do char-server, gerenciamento de personagens
  - `char_clif.cpp/hpp` - Client Interface - seleção/criação de personagens
  - `char_mapif.cpp/hpp` - Map Interface - comunicação char ⟷ map-server
  - `char_logif.cpp/hpp` - Login Interface - comunicação char ⟷ login-server
  - `char_cnslif.cpp/hpp` - Console Interface - comandos do console
  - `char_restock.cpp/hpp` - Sistema de restock de itens
  - `inter.cpp/hpp` - Módulo principal inter-server, coordena comunicação entre map-servers

- **Submódulos Inter-server (int_*):**
  - `int_achievement.cpp/hpp` - Sistema de conquistas/achievements
  - `int_auction.cpp/hpp` - Sistema de leilões
  - `int_clan.cpp/hpp` - Sistema de clãs
  - `int_elemental.cpp/hpp` - Dados de elementais (Sorcerer)
  - `int_guild.cpp/hpp` - Sistema de guildas, WoE, castelos
  - `int_homun.cpp/hpp` - Sistema de homunculus (Alchemist)
  - `int_mail.cpp/hpp` - Sistema de correio interno
  - `int_mercenary.cpp/hpp` - Sistema de mercenários
  - `int_party.cpp/hpp` - Sistema de grupos/parties
  - `int_pet.cpp/hpp` - Sistema de pets/bichinhos
  - `int_quest.cpp/hpp` - Sistema de missões/quests
  - `int_storage.cpp/hpp` - Kafra storage, guild storage

### 3. Map Server (`src/map/`)
**Responsabilidade:** Lógica de jogo em tempo real, NPCs, skills, combat, world simulation
- **Core Systems:**
  - `map.cpp/hpp` - Gerenciamento de mapas, blocklists, células, spawning
  - `clif.cpp/hpp` - Client Interface - toda comunicação cliente ⟷ map-server
  - `chrif.cpp/hpp` - Char Interface - comunicação char ⟷ map-server
  - `intif.cpp/hpp` - Inter Interface - requests para inter-server via char-server

- **Player & Unit Systems:**
  - `pc.cpp/hpp` - Player Character - lógica principal de jogadores (1997+ linhas)
  - `pc_groups.cpp/hpp` - Sistema de grupos/permissões de jogadores
  - `unit.cpp/hpp` - Sistema de unidades (movimento, ações, AI base)
  - `path.cpp/hpp` - Pathfinding e cálculos de movimento

- **Combat & Battle Systems:**
  - `battle.cpp/hpp` - Cálculos de dano, fórmulas de combate, battle_config
  - `skill.cpp/hpp` - Sistema de habilidades, casting, cooldowns, efeitos
  - `status.cpp/hpp` - Status changes, buffs/debuffs, cálculos de stats
  - `mob.cpp/hpp` - Monstros, AI, spawning, drops

- **Game Content Systems:**
  - `npc.cpp/hpp` - NPCs, scripts, eventos, shops
  - `script.cpp/hpp` - Interpretador de scripts NPC (2400+ linhas)
  - `quest.cpp/hpp` - Sistema de missões e objetivos
  - `achievement.cpp/hpp` - Sistema de conquistas
  - `instance.cpp/hpp` - Instâncias/dungeons privadas

- **Social & Guild Systems:**
  - `guild.cpp/hpp` - Sistema de guildas local (map-server side)
  - `party.cpp/hpp` - Sistema de grupos/parties local
  - `clan.cpp/hpp` - Sistema de clãs
  - `chat.cpp/hpp` - Sistema de chat rooms
  - `channel.cpp/hpp` - Sistema de canais de chat

- **Economy & Trading:**
  - `trade.cpp/hpp` - Sistema de trocas entre jogadores
  - `vending.cpp/hpp` - Sistema de vendas (merchant stalls)
  - `buyingstore.cpp/hpp` - Sistema de lojas de compra
  - `cashshop.cpp/hpp` - Loja de cash/premium items
  - `searchstore.cpp/hpp` - Busca de lojas de vendas
  - `storage.cpp/hpp` - Kafra storage, cart, guild storage

- **Companion Systems:**
  - `pet.cpp/hpp` - Sistema de pets/bichinhos
  - `homunculus.cpp/hpp` - Sistema de homunculus (Alchemist)
  - `mercenary.cpp/hpp` - Sistema de mercenários
  - `elemental.cpp/hpp` - Sistema de elementais (Sorcerer)

- **Special Systems:**
  - `atcommand.cpp/hpp` - Comandos GM (@commands) e implementações
  - `battleground.cpp/hpp` - Sistema de campo de batalha/PvP
  - `duel.cpp/hpp` - Sistema de duelos
  - `mail.cpp/hpp` - Sistema de correio
  - `log.cpp/hpp` - Sistema de logs detalhado
  - `mapreg.cpp/hpp` - Variáveis globais persistentes do map-server

- **Utility & Support:**
  - `itemdb.cpp/hpp` - Database de itens em memória
  - `date.cpp/hpp` - Sistema de data/tempo do jogo
  - `navi.cpp/hpp` - Sistema de navegação
  - `ping.cpp/hpp` - Sistema de ping/latência
  - `emotes.cpp/hpp` - Sistema de emoticons
  - `deposit.cpp/hpp` - Sistema de depósito
  - `customrate.cpp/hpp` - Sistema de rates customizadas
  - `restock.cpp/hpp` - Sistema de restock automático

- **Communication Protocols:**
  - `clif_obfuscation.hpp` - Ofuscação de packets
  - `clif_packetdb.hpp` - Database de packets do cliente
  - `clif_shuffle.hpp` - Shuffle de packets para segurança
  - `packets.hpp` - Definições de estruturas de packets
  - `packets_struct.hpp` - Estruturas específicas de packets

### 4. Web Server (`src/web/`)
**Responsabilidade:** API HTTP/REST para integrações web e aplicações externas
- **Arquivos principais:**
  - `web.cpp/hpp` - Servidor HTTP principal, routing, middleware
  - `auth.cpp/hpp` - Sistema de autenticação web
  - `sqllock.cpp/hpp` - Sistema de locks para queries SQL thread-safe
  - `webutils.cpp/hpp` - Utilitários e helpers para web
  - `webcnslif.hpp` - Interface de console para web-server

- **Controllers (API Endpoints):**
  - `charconfig_controller.cpp/hpp` - API para configurações de personagem
  - `emblem_controller.cpp/hpp` - API para emblemas de guilda
  - `merchantstore_controller.cpp/hpp` - API para lojas de mercadores
  - `partybooking_controller.cpp/hpp` - API para party booking system
  - `userconfig_controller.cpp/hpp` - API para configurações de usuário

- **Core Web Infrastructure:**
  - `http.hpp` - Definições e estruturas HTTP
  - Integração com httplib (3rdparty) para servidor HTTP
  - Sistema RESTful para comunicação com aplicações web

### 5. Tools (`src/tool/`)
**Responsabilidade:** Ferramentas de conversão, cache e manutenção de dados
- **Arquivos principais:**
  - `csv2yaml.cpp/hpp` - Converte databases CSV/TXT legados para YAML
  - `yaml2sql.cpp` - Converte databases YAML para tabelas SQL
  - `yamlupgrade.cpp/hpp` - Atualiza formato de databases YAML
  - `mapcache.cpp` - Gera cache de mapas (.gat → map_cache.dat)
  - `yaml.hpp` - Utilitários e helpers para processamento YAML

- **Funcionalidades:**
  - Conversão automática de formatos legados
  - Geração de cache para otimização de performance
  - Upgrade automático de schemas de database
  - Validação de integridade de dados

## Módulos Comuns (`src/common/`)
**Responsabilidade:** Funcionalidades compartilhadas entre todos os servidores, core engine

### Core Engine Systems
- `core.cpp/hpp` - Entrada principal, inicialização, event loops, crash handling
- `socket.cpp/hpp` - Engine de sockets TCP/IP, conexões, buffers
- `timer.cpp/hpp` - Sistema de timers de alta precisão, event scheduling
- `malloc.cpp/hpp` - Memory manager customizado, leak detection, debug
- `showmsg.cpp/hpp` - Sistema de mensagens coloridas no console, logging levels

### Database & Storage Systems
- `db.cpp/hpp` - Database engine genérico (hashtables, trees, arrays)
- `database.cpp/hpp` - Modern database interface, YAML integration
- `sql.cpp/hpp` - MySQL/MariaDB proxy, connection pooling, prepared statements
- `ers.cpp/hpp` - Entry Reusage System - pool de objetos para performance

### Network & Communication
- `packets.hpp` - Estruturas de packets, headers, serialização
- `cli.cpp/hpp` - Command Line Interface, argumentos, console input
- `msg_conf.cpp/hpp` - Sistema de mensagens localizadas, configuração

### Utilities & Helpers
- `strlib.cpp/hpp` - String manipulation, StringBuf, parsing utilities
- `utils.cpp/hpp` - Utility functions, file operations, path handling
- `utilities.cpp/hpp` - Modern C++ utilities, helpers, extensions
- `random.cpp/hpp` - Secure random number generation, MT19937
- `nullpo.cpp/hpp` - Null pointer checking, debug assertions

### Cryptography & Security
- `md5calc.cpp/hpp` - MD5 hashing para senhas e checksums
- `des.cpp/hpp` - Modified DES algorithm para packet encryption

### Configuration & Data Processing
- `conf.cpp/hpp` - Interface para libconfig, parsing de arquivos .conf
- `mmo.hpp` - Core structures (1347 linhas), defines, enums, game constants
- `mapindex.cpp/hpp` - Map indexing system, map_cache.dat processing
- `grfio.cpp/hpp` - GRF file reader, data extraction, decompression

### Platform Abstraction
- `cbasetypes.hpp` - Cross-platform type definitions, compiler adaptations
- `winapi.cpp/hpp` - Windows API adaptations, compatibility layer

## Configuration System (`src/config/`)
**Responsabilidade:** Configurações de compilação e features do servidor

### Core Configuration Files
- `core.hpp` - Configurações principais (AUTOLOOTITEM_SIZE, MAX_SUGGESTIONS, OFFICIAL_WALKPATH)
- `renewal.hpp` - Configurações específicas do modo Renewal vs Pre-Renewal
- `secure.hpp` - Configurações de segurança e proteção
- `packets.hpp` - Definições de versão de packets (PACKETVER)
- `const.hpp` - Constantes globais do sistema

### Class System Configuration
- `classes/` - Configurações específicas do sistema de classes
  - Definições de job IDs, skill trees, stat bonuses

## Customization System (`src/custom/`)
**Responsabilidade:** Sistema de hooks para customizações sem modificar o core

### Hook Files (Include-based Extension System)
- `defines_pre.hpp` - Definições customizadas carregadas antes do core
- `defines_post.hpp` - Definições customizadas carregadas após o core
- `atcommand.inc` - Implementações de @commands customizados
- `atcommand_def.inc` - Definições de @commands customizados
- `script.inc` - Implementações de script commands customizados
- `script_def.inc` - Definições de script commands customizados
- `battle_config_init.inc` - Inicialização de configurações de battle customizadas
- `battle_config_struct.inc` - Estruturas de battle_config customizadas

### Customization Philosophy
- **Non-invasive**: Modificações sem alterar arquivos do core
- **Hook-based**: Sistema de includes para injetar código
- **Compile-time**: Customizações integradas na compilação
- **Modular**: Cada funcionalidade em arquivo separado

## Convenções de Nomenclatura

### Prefixos de Função
- `pc_*` - Funções relacionadas a jogadores (player character)
- `mob_*` - Funções relacionadas a monstros
- `npc_*` - Funções relacionadas a NPCs
- `skill_*` - Funções relacionadas a habilidades
- `status_*` - Funções relacionadas a status/buffs
- `battle_*` - Funções relacionadas a combate
- `guild_*` - Funções relacionadas a guildas
- `party_*` - Funções relacionadas a grupos

### Prefixos de Estrutura
- `s_*` - Estruturas (ex: `s_quest_db`)
- `e_*` - Enums (ex: `e_race`)

### Prefixos de Constantes
- `SC_*` - Status changes (ex: `SC_BLESSING`)
- `SP_*` - Bonus parameters (ex: `SP_ATK_RATE`)
- Skill IDs seguem padrão `CLASSE_NOME` (ex: `AL_TELEPORT`)

## Estruturas de Dados Principais

### Unit Data Structures (Block List System)
- `sd` - **map_session_data** - Session data de jogadores (cliente conectado)
- `md` - **mob_data** - Mob data (monstros, mercenários)
- `nd` - **npc_data** - NPC data (NPCs, shops, scripts)
- `hd` - **homun_data** - Homunculus data (companheiro alchemist)
- `pd` - **pet_data** - Pet data (bichinhos de estimação)
- `ed` - **elemental_data** - Elemental data (companheiro sorcerer)
- `bl` - **block_list** - Base structure para todos os objetos no mapa
- `ud` - **unit_data** - Dados de movimento e ações de qualquer unidade

### Game State Structures
- `st` - **script_state** - Stack de execução de script NPC (2400+ linhas em script.hpp)
- `sc` - **status_change** - Container de todos os status effects ativos
- `sce` - **status_change_entry** - Entry individual de um status effect
- `skill_lv` - Nível de habilidade, `skill_id` - ID da habilidade
- `item` - Estrutura de item com id, amount, attributes, cards

### Network & System
- `fd` - **File descriptor** - Socket/arquivo handle
- `WFIFOHEAD(fd, size)` - Prepara buffer de envio
- `WFIFOL/W/B(fd, pos)` - Write Long/Word/Byte no buffer
- `RFIFOREST(fd)` - Bytes restantes no buffer de recebimento
- `session[fd]` - Session data do socket

## Fluxo de Comunicação
1. **Cliente ⟷ Login Server** - Autenticação
2. **Login ⟷ Char Server** - Validação de conta
3. **Cliente ⟷ Char Server** - Seleção de personagem
4. **Char ⟷ Map Server** - Transferência de dados do personagem
5. **Cliente ⟷ Map Server** - Gameplay em tempo real

## Compilação
O projeto suporta três métodos de compilação:
1. **Configure + Makefile** (Linux/POSIX)
2. **CMake** (Cross-platform)
3. **Visual Studio Solution** (Windows)

## Estrutura de Banco de Dados
Cada servidor tem suas tabelas específicas:
- **Login:** `login_db`, `reg_db` (variáveis de conta)
- **Char:** `char_db`, `inventory_db`, `guild_db`, `party_db`, etc.
- **Map:** `mapreg_db` (variáveis globais), `vending_db`, etc.

## Implementação de Comandos

### AtCommands (Comandos GM) - `src/map/atcommand.cpp`
```cpp
// Definição da função do comando
ACMD_FUNC(exemplo) {
    // sd = jogador que executou o comando
    // command = nome do comando usado
    // message = argumentos passados
    
    clif_displaymessage(fd, "Comando executado!");
    return 0; // sucesso
}

// Registro do comando (em atcommand.cpp)
ACMD_DEF(exemplo)           // Comando básico
ACMD_DEFR(exemplo2, 1)      // Com restrição (1 = não funciona no console)
ACMD_DEF2("alias", exemplo) // Com alias/nome alternativo
```

### Script Commands (Comandos de Script) - `src/map/script.cpp`
```cpp
// Definição da função do comando
BUILDIN_FUNC(exemplo_script) {
    // st = script state (contexto de execução)
    // Parâmetros acessados via script_getnum(), script_getstr()
    
    int valor = script_getnum(st, 2);        // 2º parâmetro como int
    const char* texto = script_getstr(st, 3); // 3º parâmetro como string
    
    // Retornar valor para o script
    script_pushint(st, resultado);
    return 0;
}

// Registro do comando (em script.cpp)
BUILDIN_DEF(exemplo_script, "is")    // int + string como parâmetros
BUILDIN_DEF(exemplo2, "i?")          // int + parâmetro opcional
BUILDIN_DEF2("alias", exemplo_script, "is") // Com alias
```

### Argumentos de Script Commands
- `i` - integer (int32)
- `s` - string (const char*)
- `l` - label (script label)
- `r` - reference (variável por referência)
- `v` - variable (nome de variável)
- `?` - parâmetro opcional
- `*` - número variável de parâmetros

## Packet System & Network Communication

### Packet Structure (`src/common/packets.hpp`)
```cpp
struct PACKET {
    int16 packetType;    // ID do packet
    int16 packetLength;  // Tamanho do packet
} __attribute__((packed));

// Exemplo de packet específico
struct PACKET_CA_LOGIN {
    int16 packetType;
    uint32 version;
    char userid[24];
    char passwd[24];
    uint8 clienttype;
} __attribute__((packed));
```

### Packet Communication Flow
- **CA_** - Client to Account (Login) server
- **AC_** - Account (Login) to Client
- **CH_** - Client to Character server  
- **HC_** - Character to Client
- **CZ_** - Client to Zone (Map) server
- **ZC_** - Zone (Map) to Client
- **AH_** - Account to Character server
- **HA_** - Character to Account server
- **HZ_** - Character to Zone server
- **ZH_** - Zone to Character server

### Network Buffer Operations
```cpp
// Envio de dados (Write FIFO)
WFIFOHEAD(fd, packet_len);           // Prepara buffer
WFIFOW(fd, 0) = PACKET_ID;           // Escreve packet ID
WFIFOL(fd, 2) = data;                // Escreve dados
WFIFOSET(fd, packet_len);            // Envia packet

// Recebimento de dados (Read FIFO)
int cmd = RFIFOW(fd, 0);             // Lê packet ID
int data = RFIFOL(fd, 2);            // Lê dados
RFIFOSKIP(fd, packet_len);           // Confirma processamento
```

## Database Integration

### SQL Operations (`src/common/sql.cpp`)
```cpp
// Query simples
if (SQL_ERROR == Sql_Query(sql_handle, "SELECT `name` FROM `char` WHERE `char_id`='%d'", char_id)) {
    Sql_ShowDebug(sql_handle);
    return false;
}

// Prepared statements (recomendado)
SqlStmt* stmt = SqlStmt_Malloc(sql_handle);
SqlStmt_Prepare(stmt, "INSERT INTO `inventory` (`char_id`,`nameid`,`amount`) VALUES(?,?,?)");
SqlStmt_BindParam(stmt, 0, SQLDT_INT, &char_id, sizeof(char_id));
SqlStmt_BindParam(stmt, 1, SQLDT_INT, &item_id, sizeof(item_id));
SqlStmt_BindParam(stmt, 2, SQLDT_INT, &amount, sizeof(amount));
SqlStmt_Execute(stmt);
SqlStmt_Free(stmt);
```

### Memory Management (`src/common/malloc.cpp`)
```cpp
// Alocação segura com debug
void* ptr = aMalloc(size);           // malloc com debug
void* ptr = aCalloc(num, size);      // calloc com debug
ptr = aRealloc(ptr, new_size);       // realloc com debug
aFree(ptr);                          // free com debug

// Strings
char* str = aStrdup("texto");        // duplica string
char* str = aStrndup("texto", 5);    // duplica n caracteres
```

## Timer System (`src/common/timer.cpp`)
```cpp
// Adicionar timer
int timer_id = add_timer(gettick() + 5000, timer_callback, account_id, data);

// Função de callback
int timer_callback(int tid, t_tick tick, int id, intptr_t data) {
    // Processar timer
    return 0; // 0 = não repetir, valor > 0 = repetir após X ms
}

// Remover timer
delete_timer(timer_id, timer_callback);

// Timer de NPC (intervalos fixos)
add_timer_interval(gettick() + 1000, npc_timer_callback, 0, 0, 1000);
```

## Debugging e Logs
- `ShowMessage()` - Mensagem normal no console
- `ShowStatus()` - Status/informação (verde)
- `ShowInfo()` - Informação (azul)  
- `ShowNotice()` - Notice (amarelo)
- `ShowWarning()` - Warning (magenta)
- `ShowError()` - Error (vermelho)
- `ShowFatalError()` - Fatal error + crash
- `ShowDebug()` - Debug (apenas em debug builds)
- Sistema de logs integrado para cada servidor
- Verificações `nullpo_ret()` para debug de ponteiros nulos
- Gerenciamento de memória com detecção de vazamentos
- Sistema de mensagens coloridas no console

## Performance Considerations
- Use `ers_alloc()` para objetos frequently allocated/freed
- Prefer `StringBuf` para string building
- Cache database queries quando possível
- Use timers ao invés de loops para operações periódicas
- Minimize allocations em hot paths (combat, movement)