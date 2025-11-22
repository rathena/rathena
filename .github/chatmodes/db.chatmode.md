# rAthena Database (db/) - ChatMode

## Overview
O diretório `db/` contém todas as bases de dados do servidor rAthena em formato YAML moderno, substituindo os antigos formatos TXT/CSV. O sistema utiliza estrutura hierárquica com Headers, Body e Footers, suportando imports e modo Renewal/Pre-Renewal.

## Estrutura de Diretórios

### Diretório Principal (`db/`)
Contém as definições base dos databases, que fazem referência aos arquivos específicos nas subpastas.

### Version-Specific Directories
- **`re/`** - Databases para modo Renewal (252,548+ linhas só no item_db_equip.yml)
  - Balanceamento moderno, fórmulas de dano atualizadas
  - Recursos avançados (enchantments, random options, etc.)
- **`pre-re/`** - Databases para modo Pre-Renewal  
  - Balanceamento clássico, fórmulas originais
  - Sistema simplificado de stats e mecânicas

### Import System
- **`import/`** - Overrides e customizações do servidor
  - **Filosofia**: Modifique apenas o que precisa, sem tocar nos arquivos base
  - Suporte a databases customizados (item_db_bg.yml, mob_db_bg.yml, etc.)
- **`import-tmpl/`** - Templates de exemplo para imports

### Special Directories
- **`rsm-mod/`** - Modificações específicas do servidor
- **`generator/`** - Scripts e tools para geração automática de dados

## Item Database System

### Arquivos Principais
- **`item_db.yml`** - Database master com imports (102 linhas de configuração)
- **`item_cash.yml`** - Itens exclusivos da loja de cash/premium
- **`item_combos.yml`** - Sistema de combinações item+carta ou item+item

### Specialized Item Systems  
- **`item_enchant.yml`** - Sistema de encantamentos avançados
- **`item_group_db.yml`** - Grupos para random drops, gift boxes, etc.
- **`item_packages.yml`** - Pacotes/boxes que contêm múltiplos itens
- **`item_randomopt_db.yml`** - Random options (stats aleatórios em equipamentos)
- **`item_randomopt_group.yml`** - Grupos de random options
- **`item_refining_db.yml`** - Sistema de refinamento (+1 a +20)
- **`item_reform.yml`** - Sistema de reforma/upgrade de itens
- **`item_trade_db.yml`** - Restrições de trade (untradeable, etc.)
- **`item_vending_db.yml`** - Configurações específicas para vending
- **`item_forge_options_groups.yml`** - Grupos de opções para forja
- **`item_forge_options_rank.yml`** - Rankings/levels de opções de forja

### Item Categories (Split files no re/ e pre-re/)
- **`item_db_equip.yml`** - Equipamentos (armas, armaduras, acessórios)
- **`item_db_etc.yml`** - Itens diversos (materiais, quest items, etc.)
- **`item_db_usable.yml`** - Itens consumíveis (poções, comidas, etc.)

## Monster Database System

### Core Monster Files
- **`mob_db.yml`** - Database master de monstros (91 linhas de configuração + imports)
- **`mob_chat_db.yml`** - Sistema de falas/mensagens de monstros
- **`mob_item_ratio.yml`** - Modificadores de taxa de drop por monstro
- **`mob_summon.yml`** - Sistema de invocação e summoning de monstros

### Monster Behavior & Skills
- **`mob_skill_db.txt`** - Habilidades e AI de monstros (ainda em formato TXT)
- **`mob_avail.yml`** - Disponibilidade e spawning de monstros por mapa

### Advanced Monster Systems
- **`size_fix.yml`** - Modificadores de dano baseados no tamanho (Small/Medium/Large)
- **`attr_fix.yml`** - Tabela de modificadores elementais (91 elementos vs 91 elementos)

## Skill Database System

### Core Skill Files
- **`skill_db.yml`** - Database principal de habilidades (152 linhas de config)
- **`skill_tree.yml`** - Árvore de habilidades por classe/job
- **`guild_skill_tree.yml`** - Habilidades exclusivas de guildas
- **`spellbook_db.yml`** - Sistema de livros de magias (Warlock)

### Skill Behavior & Restrictions
- **`skill_changematerial_db.txt`** - Materiais alternativos para skills
- **`skill_damage_db.txt`** - Modificadores de dano específicos por skill  
- **`skill_nocast_db.txt`** - Restrições de casting (mapflags, situações)

## Character & Job System

### Core Job/Class Files
- **`job_stats.yml`** - Stats base por classe (STR, AGI, VIT, INT, DEX, LUK)
- **`job_exp.yml`** - Tabelas de experiência base/job por classe
- **`job_basepoints.yml`** - Pontos de status base distribuídos por job
- **`job_aspd.yml`** - ASPD (Attack Speed) base por classe
- **`statpoint.yml`** - Pontos de atributo ganhos por level base

### Experience & Level Systems
- **`exp_guild.yml`** - Tabela de experiência de guildas
- **`exp_homun.yml`** - Tabela de experiência de homunculus  
- **`level_penalty.yml`** - Penalidades de EXP por diferença de level

### Job Restrictions
- **`job_noenter_map.txt`** - Mapas restritos por classe
- **`item_noequip.txt`** - Restrições de equipamento por job

## Pet & Companion System

### Companion Databases
- **`pet_db.yml`** - Pets/bichinhos de estimação completos
- **`homunculus_db.yml`** - Homunculus (companheiros Alchemist)
- **`mercenary_db.yml`** - Mercenários (NPCs de aluguel)
- **`elemental_db.yml`** - Elementais (companheiros Sorcerer)

## Quest & Achievement System

### Quest System
- **`quest_db.yml`** - Missões com objetivos e recompensas (59 linhas de config)
- **`achievement_db.yml`** - Sistema moderno de conquistas
- **`achievement_level_db.yml`** - Níveis e progressão de achievements

## Instance & Special Content

### Instance System
- **`instance_db.yml`** - Dungeons/instâncias privadas (51 linhas de config)
  - TimeLimit, IdleTimeout, NoNpc, NoMapFlag, Destroyable
- **`battleground_db.yml`** - Sistema de campo de batalha PvP
- **`castle_db.yml`** - Castelos para War of Emperium (WoE)

## Crafting & Production System

### Core Crafting
- **`create_arrow_db.yml`** - Sistema de criação de flechas (Archer)
- **`produce_db.txt`** - Produção de itens por classes (Blacksmith, etc.)

### Advanced Systems (Renewal)
- **`laphine_synthesis.yml`** - Sistema de síntese Laphine (equipamentos especiais)
- **`laphine_upgrade.yml`** - Sistema de upgrade Laphine
- **`refine.yml`** - Configurações de refinamento (+1 a +20)

### Special Skill Systems
- **`abra_db.yml`** - Database para Abracadabra (Sage skill - efeitos aleatórios)
- **`magicmushroom_db.yml`** - Efeitos da habilidade Magic Mushroom

## Utility & Misc Systems

### Core System Files
- **`const.yml`** - Constantes de script e sistema (54 linhas de definições)
- **`status.yml`** - Definições de status effects e buffs/debuffs
- **`status_disabled.txt`** - Status effects desabilitados no servidor

### Map & World Systems
- **`map_cache.dat`** - Cache de mapas compilado (binary)
- **`map_index.txt`** - Índice de todos os mapas do servidor
- **`map_drops.yml`** - Drops específicos por mapa
- **`GeoIP.dat`** - Database de geolocalização por IP (legacy)

### Player Experience Systems
- **`attendance.yml`** - Sistema de presença diária/recompensas
- **`deposit_db.yml`** - Sistema de depósito de itens
- **`enchantgrade.yml`** - Graus e níveis de encantamento
- **`stylist.yml`** - Sistema de mudança de visual/cores

### Security & Anti-Cheat
- **`captcha_db.yml`** - Sistema anti-bot com captcha
- **`reputation.yml`** - Sistema de reputação de jogadores
- **`reputation_group.yml`** - Grupos de reputação

### Premium Systems
- **`cash_emotes_db.yml`** - Emotes exclusivos de cash shop
- **`title_bonus.yml`** - Bônus estatísticos de títulos

## Formato YAML Moderno

### Estrutura Base (3 seções)
```yaml
Header:
  Type: ITEM_DB / MOB_DB / SKILL_DB / COMBO_DB / etc.
  Version: 1

Body:
  - Id: 1001
    AegisName: "Red_Potion"  
    Name: "Red Potion"
    # ... campos específicos

Footer:
  Imports:
  - Path: db/pre-re/item_db.yml
    Mode: Prerenewal
  - Path: db/re/item_db.yml  
    Mode: Renewal
  - Path: db/import/item_db.yml
```

### Sistema de Imports Hierárquico
1. **Base file** (`db/item_db.yml`) - Define estrutura e imports
2. **Mode-specific** (`db/re/` ou `db/pre-re/`) - Dados específicos do modo
3. **Custom imports** (`db/import/`) - Overrides do servidor (carregados por último)

### Item Database Structure (Campos Completos)
```yaml
- Id: 501                      # ID único do item
  AegisName: "Red_Potion"      # Nome interno (sem espaços) - usado em scripts
  Name: "Red Potion"           # Nome de exibição no cliente
  Type: "Healing"              # Tipo principal (Healing/Usable/Etc/Armor/Weapon/Card/etc.)
  SubType: 0                   # Subtipo (Weapon/Ammo/Card específico)
  Buy: 50                      # Preço de compra NPC (Default: 0)
  Sell: 25                     # Preço de venda NPC (Default: Buy/2)
  Weight: 70                   # Peso (cada 10 = 1 peso de jogo)
  Attack: 0                    # ATK físico (armas)
  MagicAttack: 0               # MATK (armas mágicas)
  Defense: 0                   # DEF (armaduras)
  Range: 1                     # Alcance de ataque (armas)
  Slots: 0                     # Slots para cartas
  Jobs:                        # Restrições de classe
    Alchemist: true
    Blacksmith: true
    # ... outras classes específicas
    # OU: All: true para todas
  Classes:                     # Restrições de nível de classe  
    All: true                  # Normal/Upper/Baby/Third/etc.
  Gender: "Both"               # Both/Male/Female
  Locations:                   # Slots de equipamento (armaduras)
    Right_Hand: true           # Mão direita
    Head_Top: true             # Cabeça superior
    # ... outros slots
  WeaponLevel: 4               # Nível da arma (1-4)
  ArmorLevel: 1                # Nível da armadura (1-2)  
  EquipLevelMin: 40            # Level mínimo para equipar
  EquipLevelMax: 99            # Level máximo para equipar (Default: 0 = sem limite)
  Refineable: true             # Pode ser refinado (+1 a +20)
  Gradable: false              # Pode ter grade (A/B/C/D)
  View: 1                      # Sprite ID de exibição
  AliasName: null              # Nome alternativo para o cliente
  Flags:                       # Flags especiais
    BuyingStore: true          # Disponível em buying stores
    DeadBranch: false          # Pode sair de Dead Branch
    Container: false           # É um container
    UniqueId: false            # Tem ID único por instância
    BindOnEquip: false         # Bind ao equipar
    DropAnnounce: false        # Anuncia quando dropado
    NoConsume: false           # Não é consumido ao usar
    DropEffect: None           # Efeito visual ao dropar
  Stack:                       # Sistema de stack
    Amount: 100                # Quantidade máxima por stack
    Inventory: true            # Pode stackar no inventário
    Cart: true                 # Pode stackar no cart
    Storage: true              # Pode stackar no storage
    GuildStorage: true         # Pode stackar no guild storage
  NoUse:                       # Restrições de uso
    Override: 100              # Level de GM que ignora restrições
    Sitting: false             # Não pode usar sentado
  Trade:                       # Restrições de trade
    Override: 100              # Level de GM que ignora restrições  
    NoDrop: false              # Não pode dropar
    NoTrade: false             # Não pode trocar
    TradePartner: false        # Só pode trocar com parceiro
    NoSell: false              # Não pode vender
    NoCart: false              # Não pode colocar no cart
    NoStorage: false           # Não pode colocar no storage
    NoGuildStorage: false      # Não pode colocar no guild storage
    NoMail: false              # Não pode enviar por mail
    NoAuction: false           # Não pode colocar em leilão
  Script: |                    # Script executado ao usar/equipar
    .@r = getrefine();
    bonus bUnbreakableWeapon;
    bonus bBaseAtk,50+BaseLevel;
    if (eaclass()&EAJL_THIRD && BaseJob == Job_Alchemist) {
       bonus bHit,20;
       bonus bAspdRate,10;
    }
  EquipScript: |               # Script executado ao equipar (apenas)
    bonus bStr,2;
  UnEquipScript: |             # Script executado ao desequipar
    heal 0,0;
```

### Monster Database Structure (Campos Completos)
```yaml
- Id: 1001                     # ID único do monstro
  AegisName: "SCORPION"        # Nome interno para scripts (uso em mapas/spawns)
  Name: "Scorpion"             # Nome de exibição em inglês
  JapaneseName: "スコーピオン"  # Nome em japonês (Default: usa Name)
  Level: 16                    # Level do monstro
  Hp: 153                      # HP total
  Sp: 1                        # SP total (para skills)
  BaseExp: 54                  # EXP base concedida
  JobExp: 40                   # EXP de job concedida
  MvpExp: 0                    # EXP adicional para MVP (Default: 0)
  Attack: 33                   # ATK mínimo (Pre-RE) / ATK base (RE)
  Attack2: 40                  # ATK máximo (Pre-RE) / MATK base (RE)
  Defense: 10                  # DEF física (reduz dano físico)
  MagicDefense: 10             # MDEF mágica (reduz dano mágico)
  Resistance: 0                # RES física (Renewal - reduz dano físico %)
  MagicResistance: 0           # MRES mágica (Renewal - reduz dano mágico %)
  Str: 12                      # Força (afeta ATK)
  Agi: 15                      # Agilidade (afeta FLEE)
  Vit: 10                      # Vitalidade (afeta DEF adicional)
  Int: 5                       # Inteligência (afeta MATK)
  Dex: 19                      # Destreza (afeta HIT)
  Luk: 5                       # Sorte (afeta Perfect Dodge/Lucky Flee)
  AttackRange: 1               # Alcance de ataque físico
  SkillRange: 10               # Alcance máximo para usar skills
  ChaseRange: 12               # Distância máxima de perseguição
  Size: "Small"                # Tamanho (Small/Medium/Large)
  Race: "Insect"               # Raça (Formless/Undead/Brute/Plant/Insect/Fish/Demon/Demi-Human/Angel/Dragon)
  Element: "Fire"              # Elemento base
  ElementLevel: 1              # Nível do elemento (1-4)
  WalkSpeed: 200               # Velocidade de movimento (ms por célula)
  AttackDelay: 1564            # Delay entre ataques (ms)
  AttackMotion: 864            # Duração da animação de ataque (ms)
  DamageMotion: 576            # Duração da animação de recebimento de dano (ms)
  DamageTaken: 100             # Modificador de dano recebido (%)
  Ai: 01                       # Tipo de AI (comportamento)
  Class: "Normal"              # Classe (Normal/Boss/Guardian/Battlefield)
  Modes:                       # Comportamentos especiais
    CanMove: true              # Pode se mover
    Looter: false              # Coleta itens do chão
    Aggressive: false          # Ataca players próximos
    Assist: false              # Ajuda outros mobs da mesma raça
    CastSensorIdle: false      # Ataca quem usa magia próximo
    Boss: false                # É considerado boss
    Plant: false               # Comportamento de planta
    CanAttack: true            # Pode atacar
    Detector: false            # Detecta players ocultos
    CastSensorChase: false     # Persegue quem usa magia
    ChangeChase: false         # Muda alvo durante chase
    Angry: false               # Fica com raiva ao ser atacado
    ChangeTargetMelee: false   # Muda alvo em combate corpo a corpo
    ChangeTargetChase: false   # Muda alvo durante perseguição
    TargetWeak: false          # Prefere alvos mais fracos
    RandomTarget: false        # Escolhe alvos aleatoriamente
    IgnoreMelee: false         # Ignora ataques corpo a corpo
    IgnoreMagic: false         # Ignora ataques mágicos
    IgnoreRanged: false        # Ignora ataques ranged
    Mvp: false                 # É um MVP
    IgnoreMisc: false          # Ignora skills misc
    KnockBackImmune: false     # Imune a knockback
    TeleportBlock: false       # Bloqueia teleporte próximo
    FixedItemDrop: false       # Drops com taxa fixa (ignoram modificadores)
    IgnoreElement: false       # Ignora modificadores elementais
    IgnoreDefense: false       # Ignora DEF do alvo
    IgnoreFlee: false          # Ignora FLEE do alvo
    IgnoreMDefense: false      # Ignora MDEF do alvo
  MvpDrops:                    # Drops exclusivos de MVP
    - Item: "Old_Card_Album"
      Rate: 5500
      RandomOptionGroup: "Old_Card_Album"
  Drops:                       # Lista de drops normais
    - Item: "Scorpion_Tail"
      Rate: 5500               # Taxa de drop (0-10000, onde 10000 = 100%)
      StealProtected: false    # Protegido contra Steal
      RandomOptionGroup: "Weapon_Rental"
    - Item: "Zeny"
      Rate: 10000
      Amount: [40, 90]         # Range de quantidade (min, max)
```

### Skill Database Structure (Sistema Complexo - 152 linhas de config)
```yaml
- Id: 1                        # ID único da skill
  Name: "NV_BASIC"             # Nome Aegis da skill (usado em scripts)
  Description: "Basic Skill"   # Descrição da skill
  MaxLevel: 9                  # Nível máximo da skill
  Type: "Passive"              # Tipo de skill (None/Weapon/Magic/Misc/Passive)
  TargetType: "Passive"        # Tipo de alvo (Passive/Enemy/Place/Self/Friend/Trap/etc.)
  DamageFlags:                 # Propriedades de dano
    NoDamage: true             # Não causa dano
    Splash: false              # Tem área de efeito
    SplitDamage: false         # Divide dano entre alvos
    IgnoreCards: false         # Ignora efeitos de cartas
    IgnoreElement: false       # Ignora modificadores elementais
    IgnoreDefense: false       # Ignora DEF
    IgnoreFlee: false          # Ignora FLEE (sempre acerta)
    IgnoreLongCard: false      # Ignora cartas de longa distância
  Flags:                       # Flags de comportamento da skill
    IsQuest: false             # É skill de quest
    IsWedding: false           # É skill de casamento
    IsSpirit: false            # É skill de spirit sphere
    IsGuild: false             # É skill de guilda
    IsSong: false              # É song/dance skill
    IsEnsemble: false          # Requer ensemble (2 players)
    IsNpc: false               # É skill de NPC
    IsParty: false             # Afeta party
    IsShow: true               # Mostra animação
    AllowReproduce: false      # Pode ser reproduzida (Plagiarism)
  Range:                       # Alcance da skill por nível
    - Level: 1
      Size: 0                  # Alcance no nível 1
    - Level: 9  
      Size: 0                  # Alcance no nível 9
  Hit: "Normal"                # Tipo de hit (Normal/Critical/Lucky/etc.)
  HitCount:                    # Número de hits por nível
    - Level: 1
      Count: 0
  Element:                     # Elemento da skill por nível
    - Level: 1
      Element: "Neutral"
  SplashArea:                  # Área de efeito por nível  
    - Level: 1
      Area: 0
  ActiveInstance:              # Instâncias ativas simultâneas
    - Level: 1
      Max: 0
  Knockback:                   # Knockback por nível
    - Level: 1
      Amount: 0
  CastTime:                    # Tempo de cast por nível
    - Level: 1
      Time: 0
  AfterCastActDelay:           # Delay após cast
    - Level: 1
      Time: 0
  AfterCastWalkDelay:          # Delay de movimento após cast
    - Level: 1
      Time: 0
  Duration1:                   # Duração primária do efeito
    - Level: 1
      Time: 0
  Duration2:                   # Duração secundária (se aplicável)
    - Level: 1  
      Time: 0
  Cooldown:                    # Cooldown da skill por nível
    - Level: 1
      Time: 0
  FixedCastTime:               # Tempo de cast fixo (Renewal)
    - Level: 1
      Time: 0
  CastTimeFlags:               # Flags do tempo de cast
    IgnoreDex: false           # Ignora DEX para redução de cast
    IgnoreStatusEffect: false  # Ignora status que afetam cast
    IgnoreItemBonus: false     # Ignora bônus de itens no cast
  CastDelayFlags:              # Flags do delay após cast
    IgnoreDex: false           # Ignora DEX para redução de delay
    IgnoreStatusEffect: false  # Ignora status que afetam delay
    IgnoreItemBonus: false     # Ignora bônus de itens no delay
  Requires:                    # Requisitos para usar a skill
    HpCost:                    # Custo de HP por nível
      - Level: 1
        Amount: 0
    SpCost:                    # Custo de SP por nível
      - Level: 1
        Amount: 0
    HpRateCost:               # Custo de HP em % por nível
      - Level: 1
        Amount: 0
    SpRateCost:               # Custo de SP em % por nível
      - Level: 1
        Amount: 0
    MaxHpTrigger:             # HP máximo necessário para usar
      - Level: 1
        Amount: 0
    ZenyCost:                 # Custo em Zeny por nível
      - Level: 1
        Amount: 0
    WeaponTypes:              # Tipos de arma necessários
      All: true               # Todas as armas OU lista específica
    AmmoTypes:                # Tipos de munição necessários
      All: false
    AmmoAmount:               # Quantidade de munição por nível
      - Level: 1
        Amount: 0
    State: "None"             # Estado necessário (Moveable/NotOverWeight/etc.)
    Status:                   # Status necessários/proibidos
      - Status: "NONE"
        Mode: "Required"      # Required/NotRequired
    SpiritSphereCost:         # Custo em Spirit Spheres por nível
      - Level: 1
        Amount: 0
    ItemCost:                 # Itens consumidos por nível
      - Level: 1
        Items:
          - Item: "Red_Gemstone"
            Amount: 1
    Equipment:                # Equipamentos necessários
      - Item: "Some_Item"
        Equipped: true        # Deve estar equipado
  Unit:                       # Configurações de skill unit (ground skills)
    Id: 0                     # ID da skill unit
    AlternateId: 0            # ID alternativo
    Layout: 0                 # Layout da área de efeito
    Range: 0                  # Alcance da unit
    Interval: 0               # Intervalo entre ativações (ms)
    Target: "None"            # Tipo de alvo da unit
    Flag: "UF_NONE"           # Flags especiais da unit
```

## Tipos de Dados e Enumerações

### Item Types (Tipos Principais)
- **`Healing`** - Itens de cura (poções, comidas)
- **`Usable`** - Itens usáveis consumíveis  
- **`Etc`** - Itens diversos (materiais, quest items, etc.)
- **`Armor`** - Equipamentos (armaduras, acessórios, sapatos)
- **`Weapon`** - Armas de todas as categorias
- **`Card`** - Cartas para inserir em slots
- **`PetEgg`** - Ovos de pets/bichinhos
- **`PetArmor`** - Equipamentos para pets
- **`Ammo`** - Munições (flechas, balas, shurikens)
- **`DelayConsume`** - Consumo com delay (itemskill items)
- **`ShadowGear`** - Equipamentos sombra (sistema especial)
- **`Cash`** - Itens de cash shop (requer confirmação)

### Weapon SubTypes
**Melee Weapons:**
- `Fist`, `Dagger`, `1hSword`, `2hSword`, `1hSpear`, `2hSpear`
- `1hAxe`, `2hAxe`, `Mace`, `Staff`, `2hStaff`
- `Knuckle`, `Musical`, `Whip`, `Book`, `Katar`, `Huuma`

**Ranged Weapons:**
- `Bow`, `Revolver`, `Rifle`, `Gatling`, `Shotgun`, `Grenade`

### Ammo SubTypes
- `Arrow`, `Dagger`, `Bullet`, `Shell`, `Grenade`
- `Shuriken`, `Kunai`, `CannonBall`, `ThrowWeapon`

### Card SubTypes
- `Normal` - Cartas normais (Default)
- `Enchant` - Cartas de encantamento especiais

### Equipment Locations (Slots de Equipamento)
- `Head_Top`, `Head_Mid`, `Head_Low` - Cabeça (superior/meio/inferior)
- `Right_Hand`, `Left_Hand`, `Both_Hand` - Mãos
- `Armor` - Armadura corporal
- `Shoes` - Sapatos
- `Garment` - Manto/capa
- `Right_Accessory`, `Left_Accessory` - Acessórios
- `Costume_Head_Top`, `Costume_Head_Mid`, `Costume_Head_Low` - Visuais
- `Costume_Garment` - Manto visual
- `Shadow_Armor`, `Shadow_Weapon`, `Shadow_Shield` - Shadow gear
- `Shadow_Shoes`, `Shadow_Right_Accessory`, `Shadow_Left_Accessory`

### Monster Races (10 categorias)
- **`Formless`** - Sem forma definida (slimes, spirits)
- **`Undead`** - Mortos-vivos (skeletons, zombies)
- **`Brute`** - Bestas/animais selvagens
- **`Plant`** - Plantas e vegetação
- **`Insect`** - Insetos e artrópodes
- **`Fish`** - Peixes e criaturas aquáticas
- **`Demon`** - Demônios e seres malignos
- **`Demi-Human`** - Humanoides
- **`Angel`** - Anjos e seres divinos
- **`Dragon`** - Dragões e reptilianos

### Monster Elements (10 elementos)
- **`Neutral`** - Neutro (sem elemento)
- **`Water`** - Água (Level 1-4)
- **`Earth`** - Terra (Level 1-4)
- **`Fire`** - Fogo (Level 1-4)
- **`Wind`** - Vento (Level 1-4)
- **`Poison`** - Veneno (Level 1-4)
- **`Holy`** - Sagrado (Level 1-4)
- **`Dark`** - Sombrio (Level 1-4)
- **`Ghost`** - Fantasma (Level 1-4)
- **`Undead`** - Morto-vivo (Level 1-4)

### Monster Sizes (3 categorias)
- **`Small`** - Pequeno (modificadores específicos de dano)
- **`Medium`** - Médio (tamanho padrão)
- **`Large`** - Grande (modificadores específicos de dano)

### Monster Classes
- **`Normal`** - Monstros normais
- **`Boss`** - Chefes/bosses (imune a algumas skills)
- **`Guardian`** - Guardiões (WoE guardians)
- **`Battlefield`** - Monstros de battleground

### Skill Types
- **`None`** - Sem tipo específico
- **`Weapon`** - Skill baseada em arma física
- **`Magic`** - Skill mágica
- **`Misc`** - Skill diversa/especial
- **`Passive`** - Skill passiva (sempre ativa)

### Skill Target Types
- **`Passive`** - Skill passiva (sem alvo)
- **`Enemy`** - Alvos inimigos
- **`Place`** - Local/ground target
- **`Self`** - Próprio usuário
- **`Friend`** - Aliados
- **`Trap`** - Skills tipo armadilha

### Job/Class Restrictions (Lista Completa)
**First Classes:**
- `Acolyte`, `Archer`, `Mage`, `Merchant`, `Swordman`, `Thief`, `Novice`, `SuperNovice`

**Second Classes:**
- `Priest`, `Monk`, `Hunter`, `Bard`, `Dancer`, `Wizard`, `Sage`, `Blacksmith`, `Alchemist`
- `Knight`, `Crusader`, `Assassin`, `Rogue`

**Transcendent Classes:**
- `High_Priest`, `Champion`, `Sniper`, `Clown`, `Gypsy`, `High_Wizard`, `Professor`
- `Whitesmith`, `Creator`, `Lord_Knight`, `Paladin`, `Assassin_Cross`, `Stalker`

**Third Classes:**
- `Arch_Bishop`, `Sura`, `Ranger`, `Minstrel`, `Wanderer`, `Warlock`, `Sorcerer`
- `Mechanic`, `Geneticist`, `Rune_Knight`, `Royal_Guard`, `Guillotine_Cross`, `Shadow_Chaser`

**Extended Classes:**
- `Taekwon`, `Star_Gladiator`, `Soul_Linker`, `Gunslinger`, `Ninja`, `Rebellion`, `Summoner`

## Conversão e Manutenção

### Ferramentas de Conversão
- `csv2yaml.exe` - Converte CSV antigo para YAML
- `yaml2sql.exe` - Converte YAML para SQL
- `yamlupgrade.exe` - Atualiza formato YAML

### Validação
- Verificação automática de sintaxe YAML
- Validação de IDs únicos
- Verificação de referências cruzadas

## Importação e Prioridade
- Arquivos `*_re.yml` são para modo Renewal
- Arquivos `*2.yml` são para importações/customizações
- Sistema de herança permite sobrescrever dados base

## Relacionamentos entre Databases
- Items podem referenciar mobs (drops)
- Skills podem referenciar items (requisitos)
- Quests podem referenciar items/mobs
- Jobs determinam access a skills/items

## Database Tools & Conversion

### Conversion Tools (src/tool/)
- **`csv2yaml.exe`** - Converte databases CSV/TXT legacy para YAML moderno
  - Suporta conversão em lote com prompts
  - Define `CONVERT_ALL` para conversão automática
- **`yaml2sql.exe`** - Converte databases YAML para tabelas SQL
  - Útil para servidores que preferem SQL databases
  - Mantém compatibilidade com sistemas antigos
- **`yamlupgrade.exe`** - Atualiza schemas YAML para versões mais recentes
  - Executado automaticamente quando versão é incompatível
  - Migra estruturas antigas para novas

### Validation & Testing
- **Syntax Validation** - Parser YAML automático detecta erros de sintaxe
- **Reference Checking** - Validação de referências entre databases
- **ID Collision Detection** - Detecta IDs duplicados
- **Import Chain Validation** - Verifica ordem e dependências de imports

## Performance & Optimization

### Loading Strategy
- **Startup Loading** - Databases carregadas completamente na inicialização
- **Memory Caching** - Dados mantidos em memória para acesso rápido
- **Lazy Loading** - Alguns dados carregados sob demanda
- **Smart Indexing** - Índices por ID, AegisName, e outros campos chave

### Development Features
- **Runtime Reload** - `@reloaditemdb`, `@reloadmobdb`, etc. (GM commands)
- **Hot Swapping** - Modificações em import/ são aplicadas sem restart
- **Debug Mode** - Logs detalhados de loading e parsing
- **Incremental Updates** - Apenas arquivos modificados são reprocessados

### Optimization Tips
- **Import Strategy** - Use import/ apenas para overrides necessários
- **File Organization** - Mantenha files grandes (252k+ linhas) organizados
- **Memory Usage** - Monitore uso de RAM com databases grandes
- **Startup Time** - Databases grandes aumentam tempo de inicialização

## Customization Best Practices

### Import System Usage
```yaml
# ❌ WRONG: Modificar arquivos em db/re/ ou db/pre-re/
# ✅ CORRECT: Criar overrides em db/import/

# db/import/item_db.yml
Header:
  Type: ITEM_DB  
  Version: 1

Body:
  # Override apenas os itens que você modificou
  - Id: 501
    AegisName: "Red_Potion"
    Name: "Super Red Potion"  # Modificação
    Script: |
      itemheal 100,0;         # Heal aumentado
```

### Development Workflow
1. **Backup Original** - Sempre mantenha backup dos files originais
2. **Use Import System** - Modificações apenas em `db/import/`
3. **Test Incremental** - Teste mudanças uma de cada vez
4. **Document Changes** - Comente o que foi modificado e por quê
5. **Version Control** - Use git para rastrear mudanças customizadas

### Common Pitfalls
- **❌ Modificar files base** - Dificulta updates do rAthena
- **❌ IDs conflitantes** - Use ranges customizados (ex: 60000+)
- **❌ Referencias quebradas** - Sempre valide AegisNames
- **❌ Scripts complexos** - Mantenha scripts simples e legíveis
- **❌ Overrides desnecessários** - Override apenas o que realmente muda

### Custom ID Ranges (Recomendações)
- **Items**: 60000-69999 (custom items)
- **Monsters**: 4000-4999 (custom mobs)  
- **Skills**: 8000-8999 (custom skills)
- **Quests**: 70000+ (custom quests)
- **Achievements**: 280000+ (custom achievements)

## Debugging & Troubleshooting

### Common Errors
```bash
# Syntax Error
"ERROR: Failed to parse YAML file"
-> Verifique indentação e aspas

# Missing Reference  
"WARNING: Unknown item 'Custom_Sword'"
-> Verifique AegisName no item_db

# ID Collision
"ERROR: Duplicate ID 1001 found"
-> Use IDs únicos para items customizados

# Import Order
"ERROR: Cannot find prerequisite"  
-> Verifique ordem dos imports no Footer
```

### Debug Commands (GM)
```bash
@reloaditemdb        # Recarrega item database
@reloadmobdb         # Recarrega monster database  
@reloadskilldb       # Recarrega skill database
@reloadscript        # Recarrega scripts (para quest_db changes)
@iteminfo <id>       # Info detalhada de item
@mobinfo <id>        # Info detalhada de monstro
@skillinfo <id>      # Info detalhada de skill
```

### Performance Monitoring
- **Memory Usage** - Monitore RAM usage após reload
- **Load Time** - Tempo de carregamento na inicialização
- **Query Performance** - Velocidade de busca por ID/Name
- **File Size Impact** - Impacto de files grandes no performance

### Production Deployment
- **Test Environment** - Sempre teste em servidor de desenvolvimento
- **Gradual Rollout** - Deploy mudanças gradualmente
- **Backup Strategy** - Mantenha backups de databases funcionais
- **Rollback Plan** - Tenha plano para reverter mudanças problemáticas
- **Monitor Logs** - Observe logs para erros após deployment