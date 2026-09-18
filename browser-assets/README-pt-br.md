# roBrowser Legacy Remote Client (Node.js)

Servidor de cliente remoto para o [roBrowserLegacy](https://github.com/MrAntares/roBrowserLegacy) que serve assets do Ragnarok Online a partir de arquivos GRF via HTTP. Jogadores podem jogar diretamente no navegador sem precisar ter o client completo instalado localmente.

Com o **Modo Servidor Unificado**, este unico processo Node.js substitui tres servicos separados — servindo assets do jogo, arquivos estaticos e fazendo proxy de conexoes WebSocket para o rAthena — tudo em uma unica porta.

---

## Sumario

- [Recursos](#recursos)
- [Arquitetura](#arquitetura)
  - [Modo Unificado (Padrao)](#modo-unificado-padrao)
  - [Modo Separado (Legado)](#modo-separado-legado)
- [Instalacao e Configuracao](#instalacao-e-configuracao)
  - [1. Instalar Dependencias](#1-instalar-dependencias)
  - [2. Adicionar Arquivos do Client Ragnarok](#2-adicionar-arquivos-do-client-ragnarok)
  - [3. Configurar Ambiente](#3-configurar-ambiente)
  - [4. Preparar para Inicializacao Otimizada](#4-preparar-para-inicializacao-otimizada-recomendado)
  - [5. Executar o Servidor](#5-executar-o-servidor)
- [Modo Servidor Unificado](#modo-servidor-unificado)
  - [Proxy WebSocket Embutido](#proxy-websocket-embutido)
  - [Servidor de Arquivos Estaticos Embutido](#servidor-de-arquivos-estaticos-embutido)
  - [Voltar ao Modo Separado](#voltar-ao-modo-separado)
- [Recursos de Performance](#recursos-de-performance)
  - [Cache LRU de Arquivos](#cache-lru-de-arquivos)
  - [Aquecimento de Cache](#aquecimento-de-cache)
  - [Indice de Arquivos GRF](#indice-de-arquivos-grf)
  - [Headers de Cache HTTP](#headers-de-cache-http)
  - [Compressao de Respostas](#compressao-de-respostas)
  - [Auto-Extracao para Disco](#auto-extracao-para-disco)
- [Variaveis de Ambiente](#variaveis-de-ambiente)
- [Endpoints da API](#endpoints-da-api)
- [Testes](#testes)
- [Scripts NPM](#scripts-npm)
- [Suporte a Encoding de Nomes Coreanos](#suporte-a-encoding-de-nomes-coreanos)
- [Estrutura de Diretorios](#estrutura-de-diretorios)
- [Solucao de Problemas](#solucao-de-problemas)
- [Licenca](#licenca)
- [Autores](#autores)

---

## Recursos

- **Modo Servidor Unificado** — processo unico substitui wsproxy + live-server + servidor de assets
- **Proxy WebSocket embutido** — conecta WebSocket do navegador ao TCP do rAthena (substitui wsproxy standalone)
- **Servidor de arquivos estaticos embutido** — serve arquivos do roBrowserLegacy (substitui live-server)
- **Cache LRU de arquivos** com tamanho configuravel (ate 5000 arquivos / 1GB+)
- **Aquecimento de cache** — pre-carrega assets mais usados na inicializacao
- **Indexacao de arquivos GRF** — buscas O(1) em todos os arquivos GRF
- **Headers de cache HTTP** (ETag, Cache-Control) para cache do navegador
- **Compressao Gzip/Deflate** para respostas baseadas em texto
- **Suporte a encoding de nomes coreanos** (CP949/EUC-KR) com deteccao/correcao de mojibake
- **Sistema de mapeamento de paths** para conversao de encoding (path coreano → path GRF)
- **Auto-extracao** — salva arquivos GRF no disco para acesso mais rapido nas proximas vezes
- **Log de arquivos ausentes** com notificacoes
- **API REST** para health checks, estatisticas de cache e busca de arquivos
- Cross-Origin Resource Sharing (CORS)

---

## Arquitetura

### Modo Unificado (Padrao)

Um unico processo Node.js em uma unica porta faz tudo:

```
Navegador ──HTTP──→ Express (:3338)
                      ├── /applications/pwa/*  → Arquivos estaticos do roBrowserLegacy
                      ├── /data/*              → Assets do GRF
                      ├── /api/*               → Health, estatisticas de cache, busca
                      └── /ws/*                → Proxy WebSocket → rAthena TCP

Docker (rAthena)
  ├── MariaDB  :3306
  ├── Login    :6900  ←──┐
  ├── Char     :6121  ←──┤ TCP via proxy /ws/
  └── Map      :5121  ←──┘
```

**Antes (3 processos Node.js):**

| Processo | Porta | Funcao |
|----------|-------|--------|
| wsproxy | 5999 | Bridge WebSocket → TCP |
| RemoteClient-JS | 3338 | Servidor de assets GRF |
| live-server | 8000 | Servidor de arquivos estaticos |

**Depois (1 processo Node.js):**

| Processo | Porta | Funcao |
|----------|-------|--------|
| RemoteClient-JS | 3338 | Tudo |

### Modo Separado (Legado)

Defina `ENABLE_WSPROXY=false` e `ENABLE_STATIC_SERVE=false` no `.env` para rodar no modo legado com processos separados.

---

## Instalacao e Configuracao

### 1. Instalar Dependencias

```bash
npm install
```

### 2. Adicionar Arquivos do Client Ragnarok

Coloque seus arquivos GRF no diretorio `resources/`:

```text
resources/
├── DATA.INI          # OBRIGATORIO - lista os arquivos GRF a carregar
├── data.grf          # Arquivo GRF principal
├── rdata.grf         # Arquivo GRF adicional
└── *.grf             # Outros arquivos GRF
```

**Compatibilidade de GRF:** Este projeto funciona com GRF versoes **0x200** e **0x300** sem criptografia DES.

Para garantir compatibilidade, reempacote seus GRFs usando [GRF Builder/Editor](https://github.com/Tokeiburu/GRFEditor):
1. Abra seu arquivo `.grf` no GRF Builder
2. Va em: **File → Options → Repack type → Decrypt**
3. Clique: **Tools → Repack**
4. Aguarde completar e substitua o arquivo original

### 3. Configurar Ambiente

Copie o arquivo de exemplo e ajuste conforme necessario:

```bash
cp .env.example .env
```

Veja [Variaveis de Ambiente](#variaveis-de-ambiente) para todas as opcoes.

### 4. Preparar para Inicializacao Otimizada (Recomendado)

```bash
# Preparacao completa (valida config, gera mapeamento de paths, constroi indice)
npm run setup

# Preparacao rapida (pula validacao profunda de encoding)
npm run setup:quick
```

### 5. Executar o Servidor

```bash
# Modo desenvolvimento (logs detalhados, middleware de debug, relatorio de validacao)
npm start

# Modo producao (logs minimos, sem middleware de debug, inicializacao silenciosa)
npm run start:prod
```

**Saida em desenvolvimento:**

```text
Starting roBrowser Remote Client... [development]

📋 VALIDATION REPORT
================================================================================
✓ INFO:
  Node.js: v18.12.0
  Valid GRF: data.grf (version 0x200, no DES)
================================================================================

Static serve enabled: D:\projeto\roBrowserLegacy
WebSocket proxy enabled on /ws/ (allowed: 127.0.0.1:6900, 127.0.0.1:6121, 127.0.0.1:5121)
Client initialized in 1250ms (450,000 files indexed)
File index built in 320ms
Added 12000 mojibake path mappings for roBrowser compatibility

Server ready on http://localhost:3338 | Game: http://localhost:3338/applications/pwa/index.html | WS Proxy: /ws/

Warming cache (up to 500 files)...
Cache warmed with 500 files in 3200ms
```

**Saida em producao:**

```text
Starting roBrowser Remote Client... [production]
Client initialized in 1250ms (450,000 files indexed)
Server ready on http://localhost:3338 | Game: http://localhost:3338/applications/pwa/index.html | WS Proxy: /ws/
Cache warmed with 500 files in 3200ms
```

### Desenvolvimento vs Producao

| Recurso | Desenvolvimento | Producao |
|---------|-----------------|----------|
| Log de requisicoes | Toda requisicao logada | Desabilitado |
| Relatorio de validacao | Relatorio completo no startup | Apenas em caso de erros |
| Conexoes WS proxy | Logadas | Silencioso |
| Detalhes do indice de arquivos | Logados | Silencioso |
| Log de arquivos ausentes | Logado no console | Apenas em arquivo |
| Info de inicializacao | Detalhado | Resumo em uma linha |
| Erros e avisos | Sempre exibidos | Sempre exibidos |

Altere o modo por:
- `npm start` (desenvolvimento) / `npm run start:prod` (producao)
- Ou mude `NODE_ENV=production` no `.env`

---

## Modo Servidor Unificado

### Proxy WebSocket Embutido

Quando `ENABLE_WSPROXY=true`, o servidor embute um proxy WebSocket-para-TCP que substitui o pacote standalone [wsproxy](https://github.com/herenow/wsProxy).

**Como funciona:**
1. Navegador conecta via WebSocket em `ws://localhost:3338/ws/127.0.0.1:6900`
2. Servidor extrai o destino (`127.0.0.1:6900`) do path da URL
3. Servidor abre uma conexao TCP com o rAthena
4. Pacotes sao transferidos bidirecionalmente: `WS ↔ TCP`

**Seguranca:** Apenas conexoes para destinos na whitelist sao permitidas (padrao):
- `127.0.0.1:6900` (Servidor de Login)
- `127.0.0.1:6121` (Servidor de Char)
- `127.0.0.1:5121` (Servidor de Map)

**Destinos personalizados (Docker/Kubernetes):**

Quando o rAthena roda em outro host (ex: containers Docker), configure os destinos permitidos via variavel de ambiente `WS_ALLOWED_TARGETS`:

```env
# Docker Desktop (macOS/Windows):
WS_ALLOWED_TARGETS=host.docker.internal:6900,host.docker.internal:6121,host.docker.internal:5121

# Servidor rAthena remoto:
WS_ALLOWED_TARGETS=10.0.0.5:6900,10.0.0.5:6121,10.0.0.5:5121
```

Quando nao definido, usa os destinos localhost padrao (`127.0.0.1:6900/6121/5121`).

**Configuracao do roBrowserLegacy** (`Config.local.js`):
```js
remoteClient: 'http://127.0.0.1:3338/',  // OBRIGATORIO - veja abaixo
socketProxy: 'ws://127.0.0.1:3338/ws/'  // modo unificado
// socketProxy: 'ws://127.0.0.1:5999/'  // modo separado (legado)
```

**O `remoteClient` nao e opcional.** O roBrowserLegacy vem com
`remoteClient: 'https://grf.robrowser.com/'` como padrao
(`applications/pwa/Config.js`), e esse unico valor decide de onde vem cada
asset. Sem defini-lo, o cliente carrega tudo do CDN publico em silencio — o jogo
roda, nada parece quebrado, e este servidor nunca e usado. A barra final e
obrigatoria: o cliente concatena direto (`remoteClient + url` e
`remoteClient + 'batch'`).

### Servidor de Arquivos Estaticos Embutido

Quando `ENABLE_STATIC_SERVE=true`, o servidor serve os arquivos do roBrowserLegacy via middleware static do Express, substituindo a necessidade do `live-server`.

**Acesse o jogo em:** `http://localhost:3338/applications/pwa/index.html`

A variavel `ROBROWSER_PATH` aponta para o diretorio do roBrowserLegacy (padrao: `../roBrowserLegacy`).

### Voltar ao Modo Separado

Para voltar a arquitetura legada de 3 processos, atualize seu `.env`:

```env
ENABLE_WSPROXY=false
ENABLE_STATIC_SERVE=false
```

E reverta o `Config.local.js`:
```js
socketProxy: 'ws://127.0.0.1:5999/'
```

Depois inicie o wsproxy e live-server separadamente como antes.

---

## Recursos de Performance

### Cache LRU de Arquivos

Cache LRU (Least Recently Used) em memoria para conteudo de arquivos com operacoes get/set O(1).

```env
CACHE_MAX_FILES=5000        # Max arquivos em cache (padrao: 5000)
CACHE_MAX_MEMORY_MB=1024    # Max memoria em MB (padrao: 1024)
```

- Arquivos maiores que 10% da memoria maxima nao sao cacheados
- Remocao automatica quando os limites sao atingidos
- Estatisticas de cache disponiveis em `/api/cache-stats`

**Guia de dimensionamento:**

| Tamanho GRF | Arquivos Recomendados | Memoria Recomendada (MB) |
|-------------|----------------------|--------------------------|
| < 500MB | 2000 | 512 |
| 500MB - 2GB | 5000 | 1024 |
| > 2GB | 10000 | 2048 |

### Aquecimento de Cache

Pre-carrega assets frequentemente acessados no cache na inicializacao, para que o primeiro jogador a conectar tenha tempos de carregamento rapidos.

```env
CACHE_WARM_UP=true          # Ativar/desativar aquecimento
CACHE_WARM_UP_LIMIT=500     # Max arquivos a pre-carregar
```

**Categorias de assets pre-carregados (em ordem de prioridade):**
1. Texturas de UI/interface
2. Telas de loading e imagens de cartas
3. Dados do mapa padrao de spawn (prontera)
4. Formatos de mapa comuns (`.gat`, `.rsw`)
5. Sprites de jogadores (todas as classes)
6. Arquivos de paleta (`.pal`)
7. Arquivos de configuracao Lua/Lub

O aquecimento roda **apos** o servidor estar pronto e nao bloqueia requisicoes.

### Indice de Arquivos GRF

Na inicializacao, o servidor constroi um indice unificado de todos os arquivos GRF para buscas O(1):

- Paths normalizados (case-insensitive, direcao das barras)
- Variantes de path mojibake para compatibilidade com roBrowser
- Integracao com mapeamento de paths para resolucao Coreano → GRF
- Estatisticas do indice disponiveis via `/api/cache-stats`

### Headers de Cache HTTP

Assets estaticos do jogo recebem headers de cache apropriados para cache no navegador:

| Header | Valor | Proposito |
|--------|-------|-----------|
| ETag | Hash MD5 | Validacao de conteudo |
| Cache-Control | `max-age=86400, immutable` | Cache de 1 dia para assets do jogo |
| 304 Not Modified | — | Pular re-download se nao mudou |

### Compressao de Respostas

- Compressao Gzip/Deflate para respostas baseadas em texto (JSON, XML, HTML, JS)
- So comprime respostas maiores que 1KB
- Deteccao automatica de content-type e negociacao de encoding

### Auto-Extracao para Disco

Quando `CLIENT_AUTOEXTRACT=true` (padrao em `src/config/configs.js`), arquivos extraidos dos GRFs sao salvos no sistema de arquivos local. Nas proximas requisicoes, os arquivos sao servidos do disco ao inves de re-extrair do GRF — significativamente mais rapido para acessos repetidos.

---

## Variaveis de Ambiente

| Variavel | Padrao | Descricao |
|----------|--------|-----------|
| `PORT` | `3338` | Porta do servidor |
| `CLIENT_PUBLIC_URL` | `http://localhost:8000` | Origem CORS permitida |
| `NODE_ENV` | `development` | Ambiente Node (`development` ou `production`) |
| `CACHE_MAX_FILES` | `5000` | Max arquivos no cache LRU |
| `CACHE_MAX_MEMORY_MB` | `1024` | Max memoria do cache (MB) |
| `CACHE_WARM_UP` | `true` | Ativar aquecimento de cache na inicializacao |
| `CACHE_WARM_UP_LIMIT` | `500` | Max arquivos a pre-carregar no aquecimento |
| `ENABLE_WSPROXY` | `true` | Embutir proxy WebSocket (substitui wsproxy) |
| `WS_ALLOWED_TARGETS` | `127.0.0.1:6900,...` | Pares `host:port` separados por virgula para allowlist do proxy WS |
| `ENABLE_STATIC_SERVE` | `true` | Servir arquivos estaticos do roBrowserLegacy (substitui live-server) |
| `ROBROWSER_PATH` | `../roBrowserLegacy` | Caminho para o diretorio do roBrowserLegacy |

---

## Endpoints da API

| Metodo | Rota | Descricao |
|--------|------|-----------|
| GET | `/` | Retorna `index.html` |
| GET | `/api/health` | Liveness. Detalhe completo (validacao, cache, indice, arquivos ausentes) em desenvolvimento; so status e contagens em producao |
| GET | `/api/cache-stats` | Estatisticas de cache e indice. Apenas em desenvolvimento — 404 em producao |
| GET | `/api/missing-files` | Lista de arquivos nao encontrados. Apenas em desenvolvimento — 404 em producao |
| GET | `/*` | Serve qualquer arquivo do client (do disco, cache ou GRF) |
| POST | `/search` | Busca arquivos por filtro regex |
| GET | `/list-files` | Lista todos os arquivos disponiveis |
| WS | `/ws/{host}:{port}` | Proxy WebSocket para TCP (quando `ENABLE_WSPROXY=true`) |

### Exemplos de Uso

```bash
# Verificar saude do sistema
curl http://localhost:3338/api/health

# Verificar performance do cache
curl http://localhost:3338/api/cache-stats

# Verificar arquivos ausentes
curl http://localhost:3338/api/missing-files

# Buscar arquivos por regex
curl -X POST http://localhost:3338/search \
  -H "Content-Type: application/json" \
  -d '{"filter": "sprite.*\\.spr"}'
```

**Exemplo de resposta de estatisticas do cache:**

```json
{
  "cache": {
    "size": 500,
    "maxSize": 5000,
    "memoryUsedMB": "384.50",
    "maxMemoryMB": "1024",
    "hits": 12500,
    "misses": 500,
    "hitRate": "96.15%"
  },
  "index": {
    "totalFiles": 450000,
    "grfCount": 3,
    "indexBuilt": true
  }
}
```

---

## Testes

```bash
npm test
```

A suite roda no test runner embutido do Node — sem framework para instalar. **Nao precisa do cliente
Ragnarok**: os fixtures GRF em `tests/fixtures/` somam menos de 2 KB e sao arquivos sinteticos sob
licenca MIT (ver `tests/fixtures/README.md`). O CI roda o mesmo comando em todo pull request.

O que ela cobre, na ordem do que doeria mais se quebrasse:

- **Contencao de caminho** em `Client.getFile()` — o funil de `POST /batch` e da rota curinga `GET /*`.
  Traversal, caminhos absolutos, byte NUL, e a raiz do repositorio nao ser doc-root, mais o caminho
  feliz para a contencao nao virar bloqueio geral.
- **Contencao no middleware de raw import**, tanto no handler `?raw` quanto no reescritor de imports.
- **Parsing de header GRF** 0x200 e 0x300, contra headers sinteticos e arquivos reais de cada versao,
  mais os tres casos de rejeicao.
- **Gating em producao** dos endpoints de diagnostico. Sao assercoes estruturais sobre o `index.js`, nao
  chamadas HTTP — os handlers vivem dentro de `startServer()`, que exige um GRF real. O arquivo diz isso
  no topo e registra o resultado obtido com o servidor no ar.

## Scripts NPM

| Script | Descricao |
|--------|-----------|
| `npm start` | Iniciar o servidor (desenvolvimento, detalhado) |
| `npm run start:prod` | Iniciar o servidor (producao, logs minimos) |
| `npm test` | Executar a suite de regressao (nao precisa do cliente Ragnarok) |
| `npm run setup` | Otimizacao completa pre-inicializacao |
| `npm run setup:quick` | Pre-inicializacao rapida (pula validacao profunda) |
| `npm run doctor` | Executar validacao de diagnostico |
| `npm run doctor:deep` | Validacao profunda com verificacao de encoding |
| `npm run debug-grf` | Debug do carregamento de arquivos GRF |
| `npm run convert:encoding` | Gerar path-mapping.json |
| `npm run validate:grf` | Validar um unico arquivo GRF |
| `npm run validate:all` | Validar todos os GRFs em resources/ |
| `npm run validate:encoding` | Validar encoding com iconv-lite |
| `npm run test:mojibake` | Testar deteccao de mojibake |

---

## Suporte a Encoding de Nomes Coreanos

Muitos arquivos GRF do Ragnarok contem nomes de arquivo em coreano codificados em CP949/EUC-KR. Quando lidos em sistemas nao-coreanos, aparecem como mojibake (caracteres embaralhados).

**O problema:**
```
Cliente solicita: /data/texture/유저인터페이스/t_배경3-3.tga
GRF contem:       /data/texture/À¯ÀúÀÎÅÍÆäÀÌ½º/t_¹è°æ3-3.tga
```

**A solucao:**

O servidor lida com isso automaticamente atraves de:
1. **Indexacao mojibake** — constroi o indice GRF com variantes em Unicode coreano e mojibake
2. **Decodificacao em tempo real** — decodifica paths mojibake de volta para Unicode coreano na requisicao
3. **Mapeamento de paths** — `path-mapping.json` opcional para mapeamentos explicitos Coreano → path GRF

```bash
# Validacao profunda de encoding
npm run doctor:deep

# Gerar path-mapping.json
npm run convert:encoding
```

---

## Estrutura de Diretorios

```text
roBrowserLegacy-RemoteClient-JS/
│
├── index.js                    # Servidor principal (Express + WS proxy + static serve)
├── start-prod.js               # Launcher de producao (define NODE_ENV=production)
├── index.html                  # Pagina inicial servida na raiz do servidor
├── doctor.js                   # Ferramenta de diagnostico
├── prepare.js                  # Script de otimizacao pre-inicializacao
├── package.json                # Dependencias e scripts do projeto
├── .env                        # Configuracao de ambiente
├── .env.example                # Template de ambiente
├── path-mapping.json           # Mapeamentos de conversao de encoding gerados
│
├── src/                        # Codigo-fonte da aplicacao
│   ├── config/
│   │   └── configs.js          # Configuracoes do client e servidor
│   ├── controllers/
│   │   ├── clientController.js # Operacoes de arquivo, cache, indexacao, aquecimento
│   │   └── grfController.js    # Extracao GRF usando @chicowall/grf-loader
│   ├── middlewares/
│   │   └── debugMiddleware.js  # Middleware de log de debug (apenas dev)
│   ├── routes/
│   │   └── index.js            # Rotas com headers de cache HTTP
│   ├── utils/
│   │   ├── bmpUtils.js         # Conversao BMP para PNG
│   │   ├── logger.js           # Utilitario de log (respeita NODE_ENV)
│   │   └── LRUCache.js         # Implementacao do cache LRU
│   └── validators/
│       └── startupValidator.js # Validacao de inicializacao e encoding
│
├── tools/                      # Ferramentas CLI para validacao e conversao
│   ├── validate-grf.mjs        # Validacao de GRF unico
│   ├── validate-all-grfs.mjs   # Validacao em lote de GRFs
│   ├── validate-grf-iconv.mjs  # Validacao de encoding com iconv-lite
│   ├── convert-encoding.mjs    # Gerar path-mapping.json
│   └── test-mojibake.mjs       # Testar deteccao de mojibake
│
├── logs/                       # Arquivos de log
│   └── missing-files.log       # Log de arquivos ausentes
│
├── resources/                  # ARQUIVOS DO CLIENT RAGNAROK
│   ├── DATA.INI                # Arquivo de configuracao do client (obrigatorio)
│   └── *.grf                   # Arquivos GRF do client
│
├── BGM/                        # Musicas de fundo do jogo
├── data/                       # Arquivos de dados do client (auto-extraidos)
├── System/                     # Arquivos de sistema do client
└── AI/                         # Scripts de IA para homunculos/mercenarios
```

---

## Solucao de Problemas

### Problemas de Encoding

Se arquivos nao sao encontrados devido a problemas de encoding:

1. Execute validacao profunda: `npm run doctor:deep`
2. Gere mapeamento de paths: `npm run convert:encoding`
3. Reinicie o servidor

### Arquivos Ausentes

O servidor registra arquivos ausentes em `logs/missing-files.log`. Verifique:

- Endpoint `/api/missing-files` para arquivos ausentes recentes
- Saida do console para alertas de arquivos ausentes (dispara apos 10+ arquivos ausentes)

### Problemas de Performance

1. Verifique hit rate do cache: `curl http://localhost:3338/api/cache-stats`
2. Aumente tamanho do cache via `.env` (veja [Variaveis de Ambiente](#variaveis-de-ambiente))
3. Ative aquecimento de cache: `CACHE_WARM_UP=true`
4. Execute `npm run setup` para pre-construir indices

### Proxy WebSocket Nao Funciona

1. Verifique `ENABLE_WSPROXY=true` no `.env`
2. Confira se `Config.local.js` tem `socketProxy: 'ws://127.0.0.1:3338/ws/'`
3. Assets vindo de `grf.robrowser.com` em vez deste servidor: falta `remoteClient` no `Config.local.js`
4. Garanta que o rAthena esta rodando (login:6900, char:6121, map:5121)
5. Verifique os logs do servidor por mensagens `WS proxy blocked connection`
6. Para Docker/rAthena remoto: defina `WS_ALLOWED_TARGETS` no `.env` (veja [Variaveis de Ambiente](#variaveis-de-ambiente))

### Problemas Comuns

| Problema | Solucao |
|----------|---------|
| Dependencias nao instaladas | Execute `npm install` |
| GRF incompativel | Reempacote com GRF Builder (versao 0x200, sem DES) |
| DATA.INI ausente | Crie `resources/DATA.INI` |
| Problemas de encoding | Execute `npm run convert:encoding` |
| Acesso lento a arquivos | Aumente cache, ative aquecimento, execute `npm run setup` |
| Conexao WS proxy recusada | Verifique se rAthena esta rodando, confira portas |
| Arquivos estaticos nao servidos | Confira se `ROBROWSER_PATH` aponta para o diretorio do roBrowserLegacy |

---

## Licenca

GNU GPL V3

## Autores

- Vincent Thibault
- Francisco Wallison
