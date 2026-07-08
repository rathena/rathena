# Web y API

## Estructura del sitio

Al crear contenido nuevo in-game hay que reflejarlo en la web:

| Sección | Contenido |
|---------|-----------|
| **Info** | Solo el *listado* de NPCs custom y quests custom |
| **Enciclopedia** | Guías y detalles específicos (si aplica) |

## Stack actual `implementado`

| Componente | Ubicación |
|------------|-----------|
| API | `ozro-backup/src/api/server.js` |
| Web | `ozro-site/` (React + Vite, Firebase hosting) |
| DB | MariaDB (misma del servidor RO) |

## Endpoints existentes

| Endpoint | Datos |
|----------|-------|
| `GET /health` | Estado API |
| `GET /uptime` | Uptime del host |
| `GET /status` | login/char/map online |
| `GET /players` | Jugadores online |
| `GET /stats` | Cuentas, chars, economía, guilds |
| `GET /rankings/accounts` | Zeny, cartas, MVP/boss cards, diamantes, logins |
| `GET /rankings/characters` | Nivel, EXP, fame |

## Rankings actuales en web `implementado`

Pestañas en `ozro-site/src/components/Rankings.tsx`:

- Zeny total (+ diamantes × 500M)
- Cartas distintas / total
- Cartas Boss / MVP
- Conteo de logins
- Fama por clase
- Experiencia / nivel

### Problema

Rankings miden **farm y tiempo** (zeny, MVP, EXP) → refuerzan desigualdad y no dan incentivo significativo. La colección de cartas es la excepción.

## Dirección: perfil por cuenta `pendiente`

### Nuevos datos a exponer

| Dato | Fuente | Prioridad |
|------|--------|-----------|
| % colección cartas | `acc_reg_num` → `#CARD_CT_*` / `#CARD_SEEN_*` | Alta |
| Prestigio | `#PRESTIGE` (cuando exista) | Alta |
| Challenges completados | `#CHALLENGE_DONE_*` | Alta |
| Arcos lore | `#LORE_ARC_*` | Media |
| Variedad de clases | Query `char` por account | Media |

### UI propuesta

- Página **Perfil** por jugador (cuenta principal)
- % colección prominente (como logro principal)
- Challenges y arcos lore como badges
- Rankings secundarios (zeny/MVP) como curiosidad, no meta principal

### Libro de cartas (álbum) `pendiente`

Ya tenemos el registro de cartas de cada PJ (`#CARD_*`). Mostrarlo en la web como un **libro de cromos**: vista amigable donde el jugador ve qué cartas tiene y **cuáles le faltan**.

## Ideas del doc viejo (web) `reformular`

| Idea | Estado | Notas |
|------|--------|-------|
| Daily Roulette en web → premio in-game | `pendiente` | Requiere endpoint seguro + NPC entrega |
| Daily Guess the Monster | `pendiente` | Pistas + recompensa escalada |
| Calendario de eventos | `pendiente` | Con eventos automáticos in-game |

## Sincronización juego ↔ web

El Card Collector ya guarda en variables de cuenta → la API puede leer `acc_reg_num`:

```sql
SELECT `key`, `value` FROM acc_reg_num WHERE account_id = ? AND `key` LIKE '#CARD_%'
```

Mismo patrón para `#PRESTIGE`, `#CHALLENGE_*`, `#LORE_ARC_*` cuando existan.

## Seguridad

- API detrás de VPN / CORS restringido (ya configurado)
- Endpoints de **entrega de premios** (ruleta daily, etc.) deben validar cuenta y anti-abuse
- No exponer datos de cuentas admin (`group_id = 99`)

## Comentarios

<!-- Borrador libre para ideas de web/API; luego organizar arriba -->
