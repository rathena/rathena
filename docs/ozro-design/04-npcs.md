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
| Bounty Boards | `bounty/*.txt` | Misiones de caza por ciudad (14 mapas) |
| Quests repetibles | `repeteables/*.txt` | EXP repetible por monstruo (17 quests) |

## En repo, desactivados `en-repo`

| NPC | Archivo | Función | Notas |
|-----|---------|---------|-------|
| Dungeon Warper | `dungeon_warper.txt` | Teleport a dungeons | QoL solo/LAN |
| Endow Sage | `endow_sage.txt` | Encanta armas elementales | Doc viejo: "Endower" |
| Gravekeeper | `gravekeeper.txt` | ? | Revisar script |
| Soul Linker | `soul_link.txt` | Buffs SL por zeny | Doc: añadir Rebirth Soul |
| Thrasher | `trasher.txt` | Compra items aleatorios caro | Sink de zeny |
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
| Crimson Weapons (quest) | `reformular` | Ver [05-quests-custom.md](./05-quests-custom.md) — **rAthena ya permite obtenerlas** |
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

## Comentarios

<!-- NPCs a priorizar, feedback in-game -->

## Notas de sesión

<!-- ¿Qué NPCs usamos? ¿Cuáles ignoramos? -->
