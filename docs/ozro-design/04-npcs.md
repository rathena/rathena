# NPCs personalizados

Inventario de NPCs OzRo. Config: `npc/scripts_custom.conf`

## Activos `implementado`

| NPC | Archivo | Función |
|-----|---------|---------|
| Healer | `healer.txt` | Cura + buffs temporales |
| Stylist | `stylist.txt` | Cambio de apariencia |
| Monke | `monke.txt` | Intercambia bananas por frutas aleatorias |
| Canalizadora Mística | `resetnpc.txt` | Reset stats/skills (birthstone + zeny) |
| Sophie | `points2zeny.txt` | Cambio puntos cash ↔ zeny |
| Card Collector | `card_collector.txt` | Archivo de cartas por cuenta, % progreso |
| Card Remover | `card_remover.txt` | Quitar cartas de equip |
| Kaz el Cronista | `chronicler.txt` | Lore básico; conteo jugadores — **expandir como guía** |
| Sabio de los Elementos | `endow_sage.txt` | Endow elemental (zeny + piedra) — Izlude, Prontera, Morocc, Geffen |
| Ludovico el Sepulturero | `gravekeeper.txt` | Muertes del PJ + contador global `$OZRO_DEATHS_TOTAL` |
| Kessinger | `soul_link.txt` | Soul Link de pago (20k zeny); incluye Ninja, Gunslinger, Renacimiento |
| Garrett el Recolector | `trasher.txt` | Compra chatarra diaria; karma/moneda solo en 1er trato por item del dia (max 2/dia) |
| Bounty Boards | `bounty/*.txt` | Misiones de caza por ciudad (14 mapas) |
| Quests repetibles | `repeteables/*.txt` | EXP repetible por monstruo (17 quests) |

## En repo, desactivados `en-repo`

| NPC | Archivo | Función | Notas |
|-----|---------|---------|-------|
| Dungeon Warper | `dungeon_warper.txt` | Teleport a dungeons | QoL solo/LAN |
| Hunting Missions | `hunting_missions.txt` | Misiones caza aleatorias + shop | Candidato daily loop |

## Perdidos / sin portar desde Hercules `hercules`

| NPC (doc viejo) | Estado | Notas |
|-----------------|--------|-------|
| Card Trader | `pendiente` | Intercambiar cartas por objetos especiales. Refactor ene. 2026; script no activo en repo |
| Ox Hunter (Ushi) | `pendiente` | Minotauro viajero; Ox Coins por historias de MVPs cazados. Anunciado en web ene. 2026 |
| Ox Merchant (Kuma) | `pendiente` | Mercader minotauro; intercambia Ox Coins por items exclusivos. Pareja de Ushi |
| MVP Tracker | `pendiente` | Rankings de MVPs, anuncios globales y asistencias. Mencionado en changelog ene. 2026 |
| Crimson Weapons Enchanter | `pendiente` | Encantamientos aleatorios en armas Crimson. Distinto de quest Marcus |
| Cash Point Merchant | `parcial` | `points2zeny.txt` (Sophie) cubre parte; revisar ratio 1:1 |
| Old Blacksmith | `pendiente` | Special Gold (bounties) → piedras/bendiciones BS |
| Crimson Weapons (quest) | `reformular` | Traerla de vuelta pero que **imbuya elemento** en vez de crear las armas (rAthena ya permite obtenerlas). Ver [05-quests-custom.md](./05-quests-custom.md) |
| Marcus (encantador Crimson) | `pendiente` | Encantar elemento en Crimson Weapons |
| Traveler Tool Dealer | `pendiente` | Lore al dealer ambulante de tools |
| Bloody Branch Manufacturer | `pendiente` | NPC custom |
| MVP Hunting / gacha | `reformular` | Monedas por items comunes → gacha equipo MVP |
| Blessed Ores crafter | `pendiente` | Reemplazar Safe tickets; quest con lore |

## Scripts genéricos rAthena (comentados en `scripts_custom.conf`)

Útiles para OzRo — evaluar activación:

| Script | Ubicación | Relevancia |
|--------|-----------|------------|
| Stock Market | `npc/custom/etc/stock_market.txt` | Economía dinámica sin players |
| Bank | `npc/custom/etc/bank.txt` | Si se deshabilita banco del menú |
| Quest board | `npc/custom/quests/questboard.txt` | Tablón dinámico |
| Floating rates | `npc/custom/etc/floating_rates.txt` | Evento automático rates |
| Marriage | `npc/custom/etc/marriage.txt` | ? |
| Monster arena | `npc/custom/etc/monster_arena.txt` | Competencia PvE |

## Variables de script (NPCs)

| Variable | Alcance | Uso |
|----------|---------|-----|
| `OZRO_KARMA` | Personaje | Reputacion/karma firmado (`>0` bueno, `<0` malo). Garrett suma +1 por trato unico del dia |
| `TRSH_TRADES` | Personaje | Ventas totales con Garrett (todas, incluye repetir el mismo item) |
| `TRSH_UniqueTrades` | Personaje | Primer trato de cada item del dia; max +2/dia; cada 10 → moneda bronce + karma |
| `TRSH_DoneItem0` / `TRSH_DoneItem1` | Personaje | Flag diario: ya vendiste ese slot hoy (solo karma/moneda en el primero) |
| `TRSH_DailySpend` / `TRSH_TotalSpent` / `TRSH_LastDay` | Personaje | Economia diaria Garrett |
| `TRSH_Item0$` / `TRSH_Item1$` | Personaje | Items del dia (AegisName) |
| `$OZRO_DEATHS_TOTAL` | Global servidor | Muertes registradas (`ozro_death_tracker`) |

## Evolución planificada por NPC

### Kaz el Cronista `reformular`

Hoy: flavor + `getusers(1)`.

**Futuro:**
- Detectar nivel del PJ activo
- Indicar siguiente paso del arco lore activo
- Marcar progreso en `#LORE_ARC_*`
- Sincronizar con web

### Card Collector `implementado` → expandir

Del doc viejo:
- Reformar tienda de cartas (precios, misiones de obtención)
- Más lore de coleccionista (parcialmente hecho en v4.x)
- NPC vieja que cambia cartas por cifra absurda → carta inobtenible aleatoria (idea extrema; `reformular`)

### Monke `probar`

Doc viejo: "muy roto" — cambiar a entregar cualquier fruta al azar. Revisar si ya se ajustó.

### Reset `probar`

Doc viejo: una vez por personaje. Hoy: infinito con birthstone. Alinear con filosofía reset fácil.

### Sabio de los Elementos `implementado` → expandir

Hoy: Fuego, Viento, Agua, Tierra (30 min; `.duration_ms` en `OnInit`).

**Futuro (quest):**
- Desbloquear imbuiciones **Holy** (`SC_ASPERSIO`), **Ghost** (`SC_GHOSTWEAPON`), **Shadow** (`SC_SHADOWWEAPON`) — casos comentados en `endow_sage.txt`.
- Quest que alargue duracion del endow (subir `.duration_ms` / `.duration_min`).

### Kessinger (Soul Linker) `implementado` → expandir

Hoy: 20 min de vinculo (`.duration` = 1.200.000 ms en `OnInit`).

**Futuro (quest):**
- Alargar duracion del soul link para el PJ (mismo patron que el Sabio).

### Kafra Staff / Cool Event Corp (votación de dungeon) `pendiente`

- Que **no** requiera 20 votos: ganar por **mayoría simple**.
- Elecciones de **lunes a miércoles**; dungeon activo de **jueves a domingo**.

## NPCs / mercados custom planificados

### Mercado OzRo Coin (item custom) `pendiente`

- Moneda custom con **varios tiers**, cada uno con distintas recompensas.
- **Requisito** para ciertas quests o items especiales.
- Se **compra/vende por zeny** con **precio dinámico** según cuánto se hayan movido (ligar con Stock Market NPC).
- Generaliza el concepto de **Ox Coins** (Ushi/Kuma) — unificar en un solo sistema de moneda.

### Mercader de birthstones `pendiente`

- Vende birthstones (insumo del reset); su stock/oferta **cambia cada día**.

## Comentarios

<!-- Borrador libre para ideas de NPCs; luego organizar arriba -->
