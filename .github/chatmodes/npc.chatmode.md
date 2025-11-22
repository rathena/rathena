# rAthena NPCs (npc/) - ChatMode

## Overview
O diretório `npc/` contém todos os scripts de NPCs, portais, monstros e eventos do rAthena em arquivos `.txt`, organizados por funcionalidade e localização. O sistema utiliza arquivos de configuração `.conf` para controlar quais scripts são carregados.

## Sistema de Carregamento (Scripts Configuration)

### Core Configuration Files (280+ linhas no scripts_athena.conf)
- **`scripts_athena.conf`** - Arquivo principal com todos os scripts oficiais
  - Organizado por seções: Airports, Cities, Merchants, Jobs, Quests, etc.
  - Usa sintaxe: `npc: npc/path/file.txt`
  - Scripts comentados com `//npc:` não são carregados
- **`scripts_custom.conf`** - Scripts customizados do servidor
- **`scripts_guild.conf`** - Scripts específicos de guildas e WoE
- **`scripts_jobs.conf`** - Scripts de mudança de classe
- **`scripts_mapflags.conf`** - Configurações de mapas (PvP, GvG, etc.)
- **`scripts_monsters.conf`** - Spawns de monstros
- **`scripts_test.conf`** - Scripts de desenvolvimento e teste
- **`scripts_warps.conf`** - Sistema de portais/teletransporte

### Loading Order & Priority
1. **mapflag/** - Configurações de mapa primeiro (flags de comportamento)
2. **warps/** - Portais básicos entre mapas
3. **mobs/** - Spawns de monstros básicos
4. **cities/** - NPCs de cidades (27 cidades diferentes)
5. **merchants/**, **kafras/** - Serviços básicos (26 tipos de comerciantes)
6. **jobs/** - Sistema de classes (2508+ linhas só no knight.txt)
7. **quests/** - Sistema de missões (organizados por região)
8. **instances/**, **events/** - Conteúdo especial
9. **custom/** - Customizações específicas do servidor

## Location-based NPCs

### Cities (`cities/` - 27 cidades)
**Arquivo por cidade com NPCs informativos, guaridas, e serviços básicos**
- `prontera.txt` (731 linhas) - Capital principal, NPCs informativos
- `geffen.txt` - Cidade dos magos, NPCs mágicos
- `payon.txt` - Vila dos arqueiros, NPCs tradicionais
- `alberta.txt` - Porto comercial, NPCs de comércio
- `aldebaran.txt` - Cidade dos alquimistas
- `morocc.txt` - Cidade do deserto, NPCs árabes
- `yuno.txt`, `lighthalzen.txt`, `einbroch.txt` - Cidades avançadas
- `amatsu.txt`, `ayothaya.txt`, `louyang.txt`, `gonryun.txt` - Cidades orientais
- `hugel.txt`, `rachel.txt`, `veins.txt` - Cidades de expansão
- `brasilis.txt`, `moscovia.txt` - Cidades especiais
- E outras: `comodo.txt`, `izlude.txt`, `umbala.txt`, `niflheim.txt`, etc.

### Airports (`airports/` - Sistema de Airships)
- `airships.txt` - NPCs dos dirigíveis
- `einbroch.txt`, `hugel.txt`, `izlude.txt`, `lighthalzen.txt`, `rachel.txt`, `yuno.txt`
- Sistema de transporte aéreo entre cidades principais

### Kafra Corporation (`kafras/`)
- **Kafra Services**: Armazenamento, teletransporte, cart rental
- **Cool Event Corp**: Serviços alternativos aos Kafras
- NPCs distribuídos por todas as cidades principais

### Warp System (`warps/` - Portais organizados por região)
- **`cities/`** - Portais dentro das cidades (ex: `prontera.txt` - 79 linhas de warps)
- **`fields/`** - Portais entre campos e dungeons
- **`dungeons/`** - Portais internos das dungeons
- **`other/`** - Portais especiais e eventos
- **Sintaxe**: `mapa,x,y,tamanho warp nome destino_mapa,dest_x,dest_y`

## Functional NPCs (Por Sistema)

### Merchants (`merchants/` - 26 tipos especializados)
**Sistema completo de comércio e serviços**
- `shops.txt` (355 linhas) - Lojas básicas organizadas por cidade
  - Tool Dealers, Weapon Dealers, Armor Dealers por cidade
  - Sintaxe: `mapa,x,y,dir shop Nome sprite,item:preço,item:preço`
- `refine.txt` - NPCs de refinamento (+1 a +20)
- `alchemist.txt` - Comerciantes de materiais de alquimia
- `hair_dyer.txt`, `clothes_dyer.txt` - Tintureiros
- `socket_enchant.txt`, `socket_enchant2.txt` - Encantadores
- `cash_trader.txt` - Comerciante de itens premium
- `advanced_refiner.txt` - Refinamento avançado
- `buying_shops.txt` - Sistema de lojas de compra
- E outros especializados: `gemstone.txt`, `kunai_maker.txt`, `inn.txt`

### Job System (`jobs/` - Hierarquia completa de classes)
**Estrutura organizada por nível de classe:**
- **`novice/`** - `supernovice.txt` (Super Novice job change)
- **`1-1e/`** - Extended first classes (Gunslinger, Ninja, etc.)
- **`2-1/`** - Second classes: `knight.txt` (2508 linhas), `priest.txt`, `wizard.txt`, `blacksmith.txt`, `hunter.txt`, `assassin.txt`
- **`2-1a/`** - Alternate second classes
- **`2-1e/`** - Extended second classes
- **`2-2/`** - Advanced second classes (Crusader, Monk, Sage, etc.)
- **`2-2a/`** - Alternate advanced classes
- **`2-2e/`** - Extended advanced classes
- **`valkyrie.txt`** - Transcendent/Rebirth system

### Quest System (`quests/` - Missões organizadas por região/tipo)
**Sistema massivo de missões (50+ arquivos)**
- **Regional Quests**: `quests_prontera.txt`, `quests_geffen.txt`, `quests_payon.txt`, etc.
- **First Class Tutorials**: `first_class/tu_acolyte.txt`, `tu_archer.txt`, etc.
- **Equipment Quests**: `newgears/2004_headgears.txt`, `newgears/2005_headgears.txt`
- **Epic Quests**: `seals/` (God Item Quests), `the_sign_quest.txt`, `thana_quest.txt`
- **Special Quests**: `kiel_hyre_quest.txt`, `airship_quests.txt`, `cooking_quest.txt`
- **Skill Quests**: `skills/` (skill-specific quests)
- **Class-specific**: `gunslinger_quests.txt`, `ninja_quests.txt`

## Special Content & Advanced Systems

### Instance System (`instances/`)
- **Private Dungeons**: Scripts de instâncias/dungeons privadas
- Sistema de dungeons instanciadas para parties/guilds
- Controle de tempo, dificuldade, e recompensas especiais

### Battleground System (`battleground/`)
- **PvP Organizado**: Scripts de campo de batalha estruturado
- Modos: Conquest, Capture the Flag, Death Match, Stone Control
- Sistemas: Flavius, Tierra, KvM (Kreiger von Midgard)
- **Desabilitado por padrão** (comentado no scripts_athena.conf)

### Events (`events/`)
- **Eventos Sazonais**: Christmas, Halloween, Valentine's Day
- **Eventos Especiais**: Scripts de eventos temporários
- **Event Controllers**: NPCs para controle de eventos

### Guild System (`guild/` e `guild2/`)
- **`guild/`** - Sistema básico de guildas e War of Emperium (WoE)
- **`guild2/`** - Sistema estendido e WoE Second Edition
- **Castle Scripts**: NPCs de castelos, economics, defesas
- **Agit Controllers**: Controle de horários e mechanics do WoE

### Other Content (`other/`)
- **Utility NPCs**: Funcionalidades especiais não categorizadas
- **Arena System**: Sistema de arena PvP
- **Global Functions**: Funções compartilhadas entre scripts

## Monster & Map Configuration

### Monster Spawns (`mobs/` - Sistema básico)
- `towns.txt` (40 linhas) - Spawns em cidades (Tarou em Einbech, Phen em Jawaii)
- `pvp.txt` - Spawns específicos para mapas PvP
- `jail.txt` - Monstros na prisão
- **Nota**: Spawns principais estão nos databases YAML, não em scripts NPC

### Map Configuration (`mapflag/` - 24 tipos de configuração)
**Sistema completo de flags de mapa:**
- **PvP System**: `pvp.txt` (101 linhas), `pvp_noguild.txt`, `pvp_noparty.txt`
- **GvG System**: `gvg.txt`, `gvg_noparty.txt`
- **Teleport Control**: `noteleport.txt`, `nowarp.txt`, `nowarpto.txt`, `noreturn.txt`
- **Skill Restrictions**: `noskill.txt`, `noicewall.txt`, `skill_damage.txt`, `skill_duration.txt`
- **Economy Control**: `nobranch.txt`, `nomemo.txt`
- **Special Modes**: `battleground.txt`, `nightmare.txt`, `night.txt`
- **Utility Flags**: `town.txt`, `nosave.txt`, `nopenalty.txt`
- **Customization**: `restricted.txt`, `partylock.txt`, `jail.txt`

## Customization & Development

### Custom Scripts (`custom/` - 40+ arquivos customizados)
**Sistema robusto de customizações não-invasivas:**
- **Core Systems**: `jobmaster.txt`, `resetnpc.txt`, `healer.txt`, `warper.txt`
- **Economy**: `itemmall.txt`, `emprestimos.txt`, `deposit.txt`
- **Events**: `events/` (subfolder), `woe_controller.txt`
- **Utility**: `build_manager.txt`, `card_remover.txt`, `stylist.txt`
- **Custom Features**: `mining/`, `mission/`, `mvplog/`, `guildhouse/`
- **Special Systems**: `championMobs.txt`, `Gramps.txt`, `platinum_skills.txt`

### CreativeSD Specific (`creativesd/`)
- Scripts específicos e customizações do servidor CreativeSD
- Modificações e adições exclusivas

### Development & Testing (`test/`)
- **NPCs de Teste**: Scripts para testing de funcionalidades
- **Debug NPCs**: NPCs para desenvolvimento e debugging
- **Sandbox**: Ambiente controlado para testes

### Version Control (`pre-re/` e `re/`)
- **`pre-re/`** - Scripts específicos para modo Pre-Renewal
  - Balanceamento clássico, mechanics originais
- **`re/`** - Scripts específicos para modo Renewal  
  - Balanceamento moderno, novas mechanics

### Special Files
- **`barters.yml`** - Sistema de troca/barter em formato YAML (híbrido)
- Scripts que mesclam funcionalidades de NPC com databases YAML

## Sintaxe de Scripts NPC (linguagem própria do rAthena)

### Estrutura Básica de NPC
```npc
// Comentários de linha única
/* Comentários de bloco */
<mapa>,<x>,<y>,<direção>	script	<nome>	<sprite_id>,{
	// código do script entre chaves
	mes "[Nome do NPC]";
	mes "Mensagem do NPC";
	close;
}
```

### Exemplo Real (baseado em prontera.txt)
```npc
prontera,101,288,3	script	Shuger#pront	98,{
	mes "[Shuger]";
	mes "Outside the safety of the city, there is a pink beast known as ^000077Poring^000000.";
	next;
	mes "[Shuger]";
	mes "Though it's cute in appearance and does not actively harm people, Poring is known to absorb items that are on the ground into its own body.";
	next;
	mes "[Shuger]";
	mes "So if there's something on the ground that you want to pick up, be careful lest it be consumed by a Poring. Then again... Porings are pretty weak...";
	close;
}
```

### Sistema de Duplicates (Reutilização de NPCs)
```npc
// NPC base/template
-	script	Guard#pront::prtguard	105,{
	mes "[Prontera Guard]";
	mes "Welcome to Prontera.";
	close;
}

// Duplicates do NPC base em diferentes posições
prontera,223,99,1	duplicate(prtguard)	Guard#2pront	105
prontera,229,104,5	duplicate(prtguard)	Guard#3pront	105
prontera,47,339,5	duplicate(prtguard)	Guard#4pront	105
```

### Tipos de NPCs (6 tipos principais + especiais)

#### 1. Script NPCs (NPCs interativos)
```npc
<mapa>,<x>,<y>,<direção>	script	<nome>	<sprite>,{
	// lógica personalizada do NPC
	mes "[Nome]";
	mes "Mensagem";
	close;
}
```

#### 2. Shop NPCs (Lojas básicas)
```npc
// Exemplo real de alberta_in
alberta_in,182,97,0	shop	Tool Dealer#alb2	73,1750:-1,611:-1,501:-1,502:-1,503:-1
// Formato: item_id:preço (-1 = preço padrão do database)
```

#### 3. Cashshop NPCs (Lojas premium)
```npc
<mapa>,<x>,<y>,<direção>	cashshop	<nome>	<sprite>,<item1>:<preço_cash>,<item2>:<preço_cash>
// Preços em Cash Points ao invés de Zeny
```

#### 4. Warp NPCs (Portais/Teletransporte)
```npc
// Exemplo real de prontera.txt
prontera,156,22,0	warp	prt001	3,2,prt_fild08,170,375
// Formato: mapa,x,y,0 warp nome range_x,range_y,destino_mapa,dest_x,dest_y
```

#### 5. Monster Spawns (Invocação de monstros)
```npc
// Exemplo real de towns.txt
einbech,0,0	monster	Tarou	1175,5,1800000,1500000
// Formato: mapa,x,y monster nome mob_id,quantidade,respawn_time,respawn_variance
```

#### 6. Duplicates (Clones de NPCs existentes)
```npc
// Exemplo real de prontera.txt
prontera,223,99,1	duplicate(prtguard)	Guard#2pront	105
// Duplica NPC existente com novo nome e posição
```

#### 7. Mapflag NPCs (Configurações de mapa)
```npc
// Exemplo de pvp.txt
pvp_y_1-1	mapflag	pvp
// Define comportamentos especiais do mapa
```

#### 8. Special NPCs (Tipos especiais)
```npc
// Floating NPCs (sem sprite visível)
-	script	HiddenScript	-1,{
	// Scripts de background/eventos
}

// Ontouch NPCs (ativados ao pisar)
mapa,x,y,range	script	TouchNPC	-1,{
	// Ativado quando player pisa na área
}
```

## Comandos de Script Essenciais (baseado na análise real dos NPCs)

### Diálogo e Interface (Comandos mais usados nos NPCs)
- `mes "[Nome]"` - Exibe mensagem com nome do NPC
- `mes "texto"` - Exibe mensagem simples
- `next` - Próxima página de diálogo (usado em 95% dos NPCs longos)
- `close` - Fecha janela e termina script
- `close2` - Fecha janela mas continua script (usado em warps/eventos)
- `end` - Termina execução sem fechar janela
- `menu "Opção 1",L_Label1,"Opção 2",L_Label2` - Menu com labels
- `select("Op1:Op2:Op3")` - Menu simples com @menu como resultado
- `input @variavel` - Input numérico do jogador
- `input$ @variavel$` - Input de string do jogador

### Controle de Fluxo (Padrões encontrados no código)
```npc
// Condicionais (muito comum em job changes)
if (BaseJob != Job_Swordman) {
    mes "Você não é Swordman!";
    close;
}

// Loops (comum em NPCs de troca)
for(.@i = 0; .@i < getarraysize(.@items); .@i++) {
    // processa cada item
}

// Labels e jumps (padrão em todos os NPCs com menu)
L_Menu:
    menu "Opção 1",L_Op1,"Sair",L_End;
L_Op1:
    mes "Você escolheu opção 1";
    goto L_Menu;
L_End:
    close;
```

### Sistema de Variáveis (hierarquia de escopo)
```npc
// Temporárias de sessão (perdidas ao deslogar)
@temp_var = 100;           // inteiro temporário
@temp_string$ = "texto";   // string temporária

// Permanentes de personagem (salvas no save)
var_permanente = 50;       // valor salvo no char
var_string$ = "salvo";     // string salva no char

// Permanentes de conta (compartilhadas entre chars)
#conta_var = 1;           // valor da conta
#conta_string$ = "teste"; // string da conta

// Globais de servidor (compartilhadas entre todos)
$global_var = 999;        // inteiro global
$global_string$ = "info"; // string global
$@event_var = 10;         // temporária global (eventos)

// Locais de script (existem apenas durante execução)
.@local_var = 5;          // local ao script atual
.var_npc = 100;           // permanente do NPC específico
```

### Items e Economia (baseado em shops reais)
```npc
// Dar items (comum em quests e job changes)
getitem 501,10;                    // 10 Red Potions
getitem "Red_Potion",10;           // usando nome

// Remover items (verificações de quest)
delitem 501,5;                     // remove 5 Red Potions
if (countitem(501) < 5) {          // verifica se tem 5
    mes "Você precisa de 5 Red Potions";
    close;
}

// Verificar peso (obrigatório antes de dar items)
if (!checkweight(501,10)) {
    mes "Você não tem espaço suficiente!";
    close;
}

// Manipular Zeny (muito comum)
if (Zeny < 1000) {
    mes "Você precisa de 1000 Zeny";
    close;
}
Zeny -= 1000;  // remove zeny
// ou: set Zeny, Zeny - 1000;
```

### Player Status e Progressão
```npc
// Experiência (usado em job NPCs e quests)
getexp 10000,5000;              // 10k base, 5k job exp
// Comum em job changes:
if (JobLevel < 40) {
    mes "Você precisa ser Job Level 40!";
    close;
}

// Mudança de classe (sistema complexo nos job NPCs)
jobchange Job_Knight;           // muda para Knight
// Verificações comuns:
if (BaseJob != Job_Swordman) close;
if (SkillPoint > 0) close;
if (checkweight(1201,1) == 0) close;

// Status effects e buffs
sc_start SC_BLESSING,300000,10; // blessing nível 10 por 5min
heal 1000,500;                  // cura 1000 HP, 500 SP
percentheal 100,100;            // cura 100% HP/SP
```

### Mapas e Movimento
```npc
// Teletransporte (usado em todos os warps)
warp "prontera",156,191;        // warp simples
if (BaseLevel < 20) {           // verificação comum
    mes "Level muito baixo!";
    close;
}

// Warps em área (usado em eventos)
areawarp "prontera",0,0,400,400,"payon",70,100;

// Mapflags (configurações de mapa)
setmapflag "pvp_y_1-1",mf_pvp;     // habilita PvP
removemapflag "prontera",mf_nowarp; // remove restrição
```

### Sistema de Quests (baseado nos NPCs de quest)
```npc
// Quest tracking (usado extensivamente)
if (checkquest(60000) == -1) {     // quest não iniciada
    setquest 60000;                // inicia quest
    mes "Quest iniciada!";
}

if (checkquest(60000) == 1) {      // quest em progresso
    mes "Complete a quest primeiro!";
    close;
}

if (checkquest(60000) == 2) {      // quest completável
    completequest 60000;           // completa quest
    getexp 50000,25000;            // recompensa
    erasequest 60000;              // remove da lista
}
```

### Timers e Eventos (sistema avançado)
```npc
// Timers de NPC (usado em eventos)
OnTimer30000:                      // a cada 30 segundos
    announce "Mensagem automática!",bc_all;
    end;

OnInit:                           // ao carregar script
    initnpctimer;                 // inicia timer automático
    end;

// Eventos personalizados
addtimer 60000,"NPC::OnDelayedEvent";  // timer único
```

### Shops Avançados (baseado nos merchants/)
```npc
// Shops dinâmicos
-	shop	DynamicShop	-1,501:-1
OnInit:
    npcshopitem "DynamicShop",501,50;  // adiciona item com preço
    npcshopitem "DynamicShop",502,100;
    end;

// Verificações de compra
OnBuyItem:
    if (BaseLevel < 10) {
        mes "Level muito baixo para comprar!";
        close;
    }
    end;
```

## Mapflags Importantes

### PvP/GvG Flags
- `pvp` - Permite PvP
- `gvg` - Guild vs Guild
- `gvg_castle` - Castelo GvG
- `battleground` - Campo de batalha

### Teleport/Save Flags
- `noteleport` - Proíbe teletransporte
- `nosave` - Não salva posição
- `noreturn` - Não permite retorno
- `nowarp` - Proíbe warp
- `nowarpto` - Proíbe warp para o mapa

### Skill/Magic Flags
- `nomemo` - Proíbe memo
- `noteleport` - Proíbe skills de teleporte
- `nobranch` - Proíbe Dead Branch
- `noicewall` - Proíbe Ice Wall

### Economy Flags
- `noloot` - Proíbe loot
- `nodrop` - Proíbe drop
- `novending` - Proíbe vending
- `nopvp` - Desabilita PvP

## Organização de Arquivos

### Convenções de Nomenclatura (padrões reais encontrados)
- **Por localização**: `prontera.txt`, `geffen.txt`, `payon.txt` (cities/)
- **Por função**: `shops.txt`, `refine.txt`, `hair_dyer.txt` (merchants/)
- **Por sistema**: `knight.txt`, `priest.txt`, `wizard.txt` (jobs/)
- **Por tipo**: `pvp.txt`, `gvg.txt`, `noteleport.txt` (mapflag/)
- **Hierárquico**: `cities/prontera.txt`, `jobs/2-1/knight.txt`

### Estrutura de Comentários (formato padrão rAthena)
```npc
//===== rAthena Script =====================================
//= Prontera NPCs
//===== By: ================================================
//= rAthena Dev Team
//===== Current Version: ===================================
//= 2.1
//===== Description: =======================================
//= NPCs for the city of Prontera.
//===== Additional Comments: ===============================
//= 1.0 First version
//= 2.0 Updated structure
//= 2.1 Added new NPCs
//==========================================================
```

### Loading Order (baseado no scripts_athena.conf real - 280 linhas)
```conf
// 1. MAP CONFIGURATION (flags e comportamentos)
npc: npc/mapflag/pvp.txt           // 101 linhas - mapas PvP
npc: npc/mapflag/gvg.txt           // mapas GvG
npc: npc/mapflag/noteleport.txt    // restrições teleporte
// ... 24 tipos de mapflag diferentes

// 2. BASIC INFRASTRUCTURE
npc: npc/warps/cities/prontera.txt // 79 linhas - portais da cidade
npc: npc/warps/fields/fild01.txt   // portais entre campos
npc: npc/warps/dungeons/prt_maze.txt // portais de dungeons

// 3. MONSTERS & SPAWNS
npc: npc/mobs/towns.txt            // 40 linhas - spawns básicos

// 4. CORE SERVICES (27 cidades)
npc: npc/cities/prontera.txt       // 731 linhas - NPCs informativos
npc: npc/cities/geffen.txt         // cidade dos magos
npc: npc/cities/payon.txt          // vila dos arqueiros
// ... todas as 27 cidades

// 5. COMMERCIAL SERVICES
npc: npc/merchants/shops.txt       // 355 linhas - lojas básicas
npc: npc/merchants/refine.txt      // refinadores
npc: npc/kafras/kafra_prontera.txt // serviços kafra

// 6. GAMEPLAY SYSTEMS
npc: npc/jobs/novice/supernovice.txt     // sistema de classes
npc: npc/jobs/2-1/knight.txt      // 2508 linhas - Knight job
npc: npc/quests/quests_prontera.txt      // sistema de quests

// 7. ADVANCED CONTENT
npc: npc/instances/nyd_01.txt      // dungeons instanciadas
npc: npc/events/christmas.txt      // eventos sazonais

// 8. CUSTOMIZATIONS (último para não conflitar)
npc: npc/custom/jobmaster.txt      // NPCs customizados
npc: npc/custom/warper.txt         // warper personalizado
```

### Scripts Configuration Files (sistema modular)
```conf
// scripts_athena.conf - Principal (280+ linhas)
import: conf/import/scripts_conf.txt

// Arquivos especializados:
// scripts_custom.conf    - Customizações
// scripts_guild.conf     - Sistema de guildas
// scripts_jobs.conf      - Classes específicas
// scripts_mapflags.conf  - Configurações de mapa
// scripts_monsters.conf  - Spawns especiais
// scripts_test.conf      - Scripts de desenvolvimento
// scripts_warps.conf     - Sistema de portais
```

## Debug e Desenvolvimento

### Comandos de Debug (usados nos scripts reais)
```npc
// Console debugging (visível apenas no servidor)
debugmes "Player " + strcharinfo(0) + " executou ação X";
debugmes "Variável @temp tem valor: " + @temp;

// Logging específico (salvo em arquivos de log)
logmes "Quest completada: " + getquestinfo(60000,0);

// Anúncios para testing
announce "Teste: " + strcharinfo(0) + " mudou para Knight", bc_all;
mapannounce "prontera", "Player testando sistema", bc_map;

// Verificação de estado para debug
if (.debug) {
    mes "DEBUG: BaseJob=" + BaseJob + " JobLevel=" + JobLevel;
    mes "DEBUG: Quest status=" + checkquest(60000);
}
```

### Testing NPCs (encontrados em custom/ e test/)
```npc
// NPC de teste completo (padrão encontrado)
prontera,150,150,4	script	GM Tester	1_M_LIBRARYMASTER,{
	if (getgmlevel() < 99) {
		mes "Acesso negado!";
		close;
	}
	
	menu "Test Jobs",L_Jobs,"Test Items",L_Items,"Test Quests",L_Quests;
	
L_Jobs:
	mes "Testando job change...";
	jobchange Job_Knight;
	close;
	
L_Items:
	getitem 501,100;  // teste de items
	mes "100 potions adicionadas";
	close;
	
L_Quests:
	setquest 60000;   // teste de quest
	mes "Quest de teste iniciada";
	close;
}

// Sistema de reload automático
OnInit:
	if (.debug_mode) {
		addtimer 30000, strnpcinfo(3) + "::OnReload";
	}
	end;

OnReload:
	debugmes "Recarregando script de teste...";
	donpcevent strnpcinfo(3) + "::OnInit";
	end;
```

### System Reloading (comandos GM para desenvolvimento)
```npc
// @reloadscript - recarrega todos os NPCs
// @reloadnpc <nome> - recarrega NPC específico
// @loadnpc <arquivo> - carrega arquivo específico
// @unloadnpc <nome> - descarrega NPC

// Exemplo de NPC que se auto-recarrega
OnWhisperGlobal:
	if (getgmlevel() < 99) end;
	if (@whispervar0$ == "reload") {
		atcommand "@reloadnpc " + strnpcinfo(0);
	}
	end;
```

## Boas Práticas (baseadas nos NPCs oficiais)

### Performance (otimizações encontradas no código)
```npc
// ❌ Evitar - loops infinitos e pesados
while (1) {
    // operação pesada sem break
}

// ✅ Melhor - loops controlados
for(.@i = 0; .@i < 100; .@i++) {
    if (condition) break;  // sempre ter saída
}

// ✅ Cache de dados (padrão nos job NPCs)
OnInit:
    .job_name$ = jobname(Job_Knight);
    .required_level = 40;
    .required_items[0] = 1010;  // cache de requirements
    end;

// ✅ Usar close2 para non-blocking (warps)
mes "Teleportando...";
close2;
warp "prontera",156,191;
end;

// ❌ Evitar variáveis globais desnecessárias
$temp_var = 1;  // ocupam memória permanentemente

// ✅ Preferir variáveis locais/temporárias
.@temp_var = 1;  // liberadas ao fim do script
@temp_var = 1;   // liberadas ao deslogar
```

### Manutenibilidade (padrões dos scripts oficiais)
```npc
// ✅ Estrutura clara com comments descritivos
//===== Knight Job Change NPC =================================
// Handles job change from Swordman to Knight
// Requirements: JobLevel 40+, specific items, no skill points
//=============================================================

// ✅ Nomes de variáveis significativos
.@knight_level_req = 40;
.@test_room_map$ = "job_knt";
.@test_monsters[0] = 1002;  // Poring para teste

// ✅ Validações organizadas em funções
function Check_Requirements {
    if (BaseJob != Job_Swordman) return 0;
    if (JobLevel < 40) return 0;
    if (SkillPoint > 0) return 0;
    if (checkweight(1201,1) == 0) return 0;
    return 1;
}

// ✅ Labels organizados hierarquicamente
L_Main:          // menu principal
L_Requirements:  // verifica requisitos
L_Test:          // teste prático
L_JobChange:     // mudança de classe
L_End:           // finalização
```

### Segurança (validações encontradas nos NPCs)
```npc
// ✅ Validação de input rigorosa (padrão dos job NPCs)
if (BaseJob != Job_Swordman) {
    mes "[Knight Master]";
    mes "Você não é um Swordman!";
    close;
}

// ✅ Verificação de skill points (previne bugs)
if (SkillPoint > 0) {
    mes "Você ainda tem skill points não utilizados!";
    mes "Use todos antes de mudar de classe.";
    close;
}

// ✅ Verificação de peso antes de dar items
if (!checkweight(1201,1)) {
    mes "Seu inventário está muito pesado!";
    close;
}

// ✅ Rate limiting em NPCs críticos
if (gettimetick(2) < @last_action + 60) {
    mes "Aguarde 1 minuto antes de usar novamente.";
    close;
}
@last_action = gettimetick(2);

// ✅ Prevenção de exploit de duplicação
if (@trading_lock) {
    mes "Você já está em uma transação!";
    close;
}
@trading_lock = 1;
// ... operações críticas ...
@trading_lock = 0;

// ✅ Validação de mapas críticos (GvG, eventos)
if (getmapflag(strcharinfo(3), mf_gvg)) {
    mes "Este serviço não está disponível durante GvG!";
    close;
}
```

### Error Handling (tratamento de erros robusto)
```npc
// ✅ Verificações múltiplas antes de ações críticas
function Job_Change_Knight {
    // Verificações em ordem de importância
    if (!Check_Requirements()) {
        mes "Você não atende aos requisitos.";
        return 0;
    }
    
    if (!Check_Items()) {
        mes "Você não possui os items necessários.";
        return 0;
    }
    
    if (!Check_Test_Completion()) {
        mes "Você precisa completar o teste primeiro.";
        return 0;
    }
    
    // Ação crítica apenas após todas as validações
    jobchange Job_Knight;
    return 1;
}
```