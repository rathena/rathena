# Configuración del servidor

**Última revisión:** 2026-07-07  
**Fuente de verdad:** `conf/import/battle_conf.txt` (sobrescribe los defaults de `conf/battle/*.conf`)

Cadena de carga: `conf/battle_athena.conf` → archivos en `conf/battle/` → **`conf/import/battle_conf.txt`**

> En rAthena, `100` = 100% = rate **1x** oficial. `500` = 5x, `10000` = 100x.

---

## Información general

| Parámetro | Valor | Fuente |
|-----------|-------|--------|
| Episodio | 14.3 | Documentación OzRo |
| Modo | Renewal | Servidor |
| Nivel máximo | 175 / 60 | Tablas EXP renewal (`db/`) |
| Acceso | VPN privada, solo invitación | — |
| Jugadores objetivo | ~3 (familiar) | — |
| Pincode | Desactivado | `conf/import/char_conf.txt` |

---

## Experiencia (EXP)

| Parámetro | Valor | Multiplicador | Archivo |
|-----------|-------|---------------|---------|
| Base EXP | `500` | **5x** | import |
| Job EXP | `500` | **5x** | import |
| MVP EXP | `1000` | **10x** | import |
| Quest EXP (NPCs) | `300` | **3x** | import |
| Multi level-up | `true` | Sí | import |
| EXP bonus por atacante | `25` | +25% por cada atacante extra | import |
| EXP bonus máx. atacantes | `12` | Tope en 12 atacantes | `exp.conf` (default) |
| Shop EXP (Discount/Overcharge) | `100` | 1% del zeny recibido × skill lv | import |
| Heal EXP | `0` | Desactivado | `exp.conf` (default) |
| Resurrection EXP | `0` | Desactivado | `exp.conf` (default) |
| PvP EXP (mobs en mapas PvP) | `yes` | Sí | `exp.conf` (default) |

### Party

| Parámetro | Valor | Notas |
|-----------|-------|-------|
| Party even share bonus | `0` | **Sin** bonus extra por tamaño de party |
| Party share level range | `15` | `conf/inter_athena.conf` |
| Bloquear misma cuenta en party | `yes` | `party.conf` |
| Misma cuenta en party (multi-login) | Bloqueado por config | Conflicto con doble/triple cuenta en party |

### Penalizaciones al morir

| Parámetro | Valor | Efecto |
|-----------|-------|--------|
| Death penalty type | `1` | Pierde % del nivel actual |
| Death penalty base | `100` | **1%** base EXP |
| Death penalty job | `100` | **1%** job EXP |
| Zeny penalty | `100` | **1%** del zeny al morir (PvP) |
| Death penalty en max level | `0` | No pierde EXP al estar en max |

### UI / feedback

| Parámetro | Valor |
|-----------|-------|
| Mostrar EXP ganada | `yes` (`disp_experience`) |
| Mostrar zeny ganado | `yes` (`disp_zeny`) |

## Drops

### Rates por tipo de mob (valores efectivos)

Los rates `_boss` aplican a monstruos clase boss. Los rates `_mvp` **no están en import** → quedan en default `100` (1x).

#### Mobs normales

| Tipo de drop | Config | Multiplicador |
|--------------|--------|---------------|
| Common (etc) | `item_rate_common: 500` | **5x** |
| Healing | `item_rate_heal: 1000` | **10x** |
| Usable | `item_rate_use: 1000` | **10x** |
| Equipment | `item_rate_equip: 1500` | **15x** |
| Card | `item_rate_card: 10000` | **100x** |

#### Boss (no MVP)

| Tipo | Config | Multiplicador |
|------|--------|---------------|
| Common | `item_rate_common_boss: 500` | **5x** |
| Healing | `item_rate_heal_boss: 1000` | **10x** |
| Usable | `item_rate_use_boss: 1000` | **10x** |
| Equipment | `item_rate_equip_boss: 1500` | **15x** |
| Card | `item_rate_card_boss: 10000` | **100x** |

#### MVP

| Tipo | Config | Multiplicador | Nota |
|------|--------|---------------|------|
| Common | `item_rate_common_mvp` | **1x** (`100`) | Default, no override |
| Healing | `item_rate_heal_mvp` | **1x** | Default |
| Usable | `item_rate_use_mvp` | **1x** | Default |
| Equipment | `item_rate_equip_mvp` | **1x** | Default |
| Card | `item_rate_card_mvp` | **1x** | Default |
| MVP reward (inventario directo) | `item_rate_mvp: 500` | **5x** | import |

#### Otros

| Tipo | Config | Multiplicador |
|------|--------|---------------|
| Treasure (cofres WoE) | `item_rate_treasure: 500` | **5x** |
| Add drop / item group | `100` | 1x (default) |
| Drop cap | `9000` | 90% máx. efectivo |
| Drops logarítmicos | `no` | Lineales |
| Alchemist summon reward | `2` | Summons dropean (import) |

### Anuncios y loot

| Parámetro | Valor | Notas |
|-----------|-------|-------|
| Anuncio drops raros | `100` | Anuncia drops ≤ **1%** |
| Autoloot mercenario | `yes` | import |
| Autoloot homúnculo | `yes` | import |
| Item auto-get | `no` | Va al suelo |
| Floor item lifetime | `60000` ms | 60 s |
| First attack loot bonus | `30` | +30% prioridad loot |
| MVP to loot priority | `no` | MVP por daño+tank oficial |

### Doc antiguo vs. actual (drops)

| Doc antiguo | Actual |
|-------------|--------|
| Etc 10x | **5x** |
| Consumibles 20x | **10x** heal/use |
| Equip 30x | **15x** |
| Cards 200x (~2%) | **100x** en normal/boss; MVP cards **1x** |
| MVP items 5x | **5x** reward directo; drops en suelo MVP a 1x |
| Boss/MVP cards 100x | **100x** en boss; **1x** en MVP (no override) |
| Drop -50% si +15 niveles | **No verificado** en config (penalización renewal puede ser interna) |

---

## Monstruos y combate

| Parámetro | Valor | Notas |
|-----------|-------|-------|
| HP mobs | `200` | **×2** |
| HP MVP | `200` | **×2** |
| Zeny de mobs | `yes` | Pequeña cantidad por kill |
| Barra HP mobs | `no` | Oculta (excepto comportamiento MVP client) |
| MVP tomb | `yes` | Default `monster.conf` |
| Críticos de mobs | **No** | `enable_critical: 17` = PC + Mercenary; bit Mob no activo |
| mob_critical_rate | `100` | Sin efecto si mob no tiene críticos habilitados |
| Boss no se curan | **No verificado** | No encontrado en `conf/` |
| Flee penalty en mobs | `yes` | `agi_penalty_target: 3` (PC + Mob) |
| Def penalty por swarm | `no` | `vit_penalty_type: 0` |
| Falcon + Warg juntos | `yes` | `warg_can_falcon: yes` |
| Bone drop (PvP) | `1` | Solo mapas PvP |
| Devotion level diff | `15` | import |
| Skill drop items full | `yes` | import |

---

## Jugador y límites

| Parámetro | Valor | Fuente |
|-----------|-------|--------|
| Max ASPD (normal) | `190` | `player.conf` (doc viejo: 193) |
| Max ASPD (3rd) | `193` | `player.conf` (doc viejo: 195) |
| Max ASPD (extended/summoner) | `193` | `player.conf` |
| Max HP lv175 | `1100000` | `player.conf` |
| Restart HP al respawn | `50%` | import |
| Restart SP al respawn | `50%` | import |
| Player skillup limit | `no` | Sin límite de skill points por nivel |
| Ocultar versión servidor | `yes` | import |

---

## Mascotas

| Parámetro | Valor | Fuente |
|-----------|-------|--------|
| Pet equip required | `no` | import (override; default `yes`) |
| Pet attack support | `yes` | import |
| Pet damage support | `yes` | import |
| Pet attack EXP to master | `yes` | import |
| Pet level rate | `50` | **50%** velocidad de level pet |

---

## Economía y features

| Parámetro | Valor | Notas |
|-----------|-------|-------|
| Vending tax | `0%` | import (default sería 5% >100M) |
| Barter | `on` | import |
| Barter extended | `off` | import |
| Banking | `on` | `feature.conf` default |
| Buying store | `on` | default |
| Search stores | `on` | default |
| Instance reconnect | `yes` | import |
| Achievements | `off` | import |
| Attendance | `off` | import |
| Equip switch | `off` | import |
| Private airship | `off` | import |
| Roulette | `off` | import |
| Atcommand suggestions | `on` | import |

---

## Skills (selección)

| Parámetro | Valor |
|-----------|-------|
| Max heal modifier | `1` |
| Max heal lv | `11` |
| Emergency call cooldown | `15` s |
| Skip teleport lv1 menu | `yes` |
| Allow skill without day | `yes` |
| Allow ES magic on players | `yes` |
| Taekwon mission mob name | `1` (visible) |
| Cart revolution knockback | `no` |
| Refresh song | `yes` |
| Backstab bow penalty | `no` |
| Skill steal random options | `yes` |

---

## Personalizaciones del doc antiguo — estado

| Idea del doc viejo | Estado actual |
|--------------------|---------------|
| No requiere quest para primer job | `pendiente` — `jobmaster.txt` desactivado en `scripts_custom.conf` |
| Reborn platinum skills | `pendiente` — `platinum_skills.txt` desactivado; jobmaster tiene flag `.Platinum` |
| Mobs hacen críticos | **No activo** en config actual |
| Boss no se curan | **No encontrado** en config |
| Drop -50% con +15 niveles | **No encontrado** en config |
| ASPD 193 / 195 | **193** solo third; normal **190** |
| Bounty → Cash Points + Special Gold | `probar` — revisar scripts bounty |
| Cash shop items varios | `pendiente` — fuera de battle_conf |
| Merchant job EXP al vender | **Activo** (`shop_exp: 100`) |
| Quests repetibles | **Activo** — `npc/custom/ozro/repeteables/` |
| Bounty boards | **Activo** — 14 ciudades |

---

## Ajustes pendientes (del doc viejo)

```
- Duplicar precio Crimson (cuando se reactive la quest)
- Bajar precios hunting a la mitad
- Cambiar ratio compra/venta cash points (no 1:1)
- Deshabilitar banco/correo del menú → solo vía NPC
- Evaluar party_even_share_bonus para 3 jugadores en party real
- Evaluar rates _mvp (hoy a 1x) si MVP farming es muy lento
```

---

## Comentarios

<!-- Ajustes locales, ideas de rates -->

---

## Notas de sesión

<!-- ¿Los rates se sienten bien? ¿Demasiado rush? ¿Falta zeny? ¿MVP drops muy bajos? -->
