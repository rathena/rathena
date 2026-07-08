# Sistemas core (prioritarios)

Sistemas que unifican el diseño. El **Card Collector** es el modelo a seguir.

## 1. Meta por cuenta `implementado` (parcial) → expandir

### Qué ya funciona

**Card Collector** (`npc/custom/ozro/card_collector.txt`):
- Variables de cuenta: `#CARD_SEEN_*`, `#CARD_CT_*`
- Progreso % por categoría
- Meta largo plazo, no depende de nivel ni de estar online con otros

### Qué falta

| Variable / concepto | Uso propuesto | Estado |
|-------------------|---------------|--------|
| `#PRESTIGE` | Puntos de prestigio (challenges, lore, eventos) | `pendiente` |
| `#CHALLENGE_ACTIVE` | Challenge en curso | `pendiente` |
| `#CHALLENGE_DONE_*` | Challenges completados | `pendiente` |
| `#LORE_ARC_*` | Arcos oficiales completados | `pendiente` |
| `#MAIN_ACCOUNT` | Flag cuenta principal (opcional) | `pendiente` |

### Política de cuentas

- **Objetivo:** Una cuenta principal por jugador real.
- **Problema actual:** ~3 cuentas c/u + multi-login para party → entorpece todo.
- **Enfoque:** Recompensas significativas solo en cuenta principal; alts sin multiplicar prestigio/colección/challenges.
- **No prohibir** multi-cuenta de golpe; hacer que no compense para metas.

## 2. Challenges por cuenta `pendiente` — **prioridad alta**

> Llevar un PJ con restricciones de build/dificultad a nivel alto, con cosas que hacer en cada rango. Equipo único por cadena determinista, no RNG ni pay2win.

### Estructura propuesta

```
Challenge (por cuenta)
├── ID y nombre (ej. "El Camino del Acolyte")
├── Restricciones
│   ├── Clase(s) permitida(s)
│   ├── Equip (solo white, sin MVP cards, etc.)
│   ├── Party (solo / máx 2 / libre)
│   └── Reset (permitido / prohibido durante challenge)
├── Hitos por nivel
│   ├── Nv 30 → quest / craft / bounty específica
│   ├── Nv 50 → ...
│   └── Nv 70+ → instancia / boss / arco lore
├── Recompensas (cosméticas / prestigio, no power creep)
│   ├── Título / fame
│   ├── Headgear trofeo (stats modestos o ninguno)
│   ├── Acceso a "casa" / vitrina
│   └── Puntos `#PRESTIGE`
└── Visible en web (perfil de cuenta)
```

### Equipamiento sin RNG

Cadena por tier (por hito de challenge):

1. Farmear materiales (bounties, repetibles, mobs del bracket)
2. NPC artesano → pieza base (barter)
3. Quest corta → mejora (+refine acotado, slot, bonus modesto)
4. Pieza final = trofeo del challenge (item existente al inicio; custom más adelante)

### Piloto sugerido

Primer challenge a definir según feedback de juego:
- Clase, nivel objetivo, restricciones
- 4–5 hitos
- 1 recompensa cosmética clara

**Comentarios:**

<!-- Definir aquí el primer challenge piloto -->

## 3. Loop entre vacaciones `pendiente`

Actividades de **15–30 min** con progreso real:

| Actividad | Esfuerzo | Progreso |
|-----------|----------|----------|
| Registrar cartas | 2 min | % colección |
| 1 bounty o repetible | 10–15 min | Materiales / zeny |
| Consultar cronista | 3 min | Siguiente paso lore |
| Instancia solo (tuneada) | 20 min | Hito challenge |
| Ver perfil en web | 0 min in-game | Social / motivación |

## 4. Competencia justa entre jugadores `reformular`

**Dejar de usar como meta principal:** zeny total, MVP kills, EXP máxima.

**Métricas por cuenta:**

| Categoría | Mide | Justicia |
|-----------|------|----------|
| Colección cartas | % archivo | Ya implementado |
| Challenges | Completados | No depende de tiempo de farm |
| Arcos lore | Capítulos | Exploración |
| Variedad | N clases > nivel X | Incentiva resets/roles |
| Soporte | Puntos heal/buff en MVP | No solo DPS |
| Prestigio total | Suma ponderada | Meta global |

## 5. Casa / vitrina (sin client) `pendiente`

RO no tiene housing real sin client. Alternativas server-side:

1. **Instancia privada por cuenta** — mapa con warp exclusivo; NPCs muestran progreso
2. **Títulos visibles** — fame o variables al clickear personaje
3. **Headgears trofeo** — recompensa challenge, sin stats absurdos
4. **Museo NPC** en Prontera — vitrina de los 3 jugadores
5. **Web** — perfil principal (ver [07-web-api.md](./07-web-api.md))

Ideas del doc viejo a integrar:
- Decorar con dolls del juego
- Storage ampliado / cofres múltiples (viable con pocos jugadores)
- Contratar NPCs de servicio (pagar mantenimiento)
- Vitrina de cartas para enseñar a otros

## 6. Economía cerrada `pendiente`

Sin jugadores vendiendo, el mercado = NPCs:

- Barter shops por tier de nivel (`feature.barter: on`)
- `points2zeny.txt` — puente cash ↔ zeny (revisar ratio)
- `card_collector` + trader de cartas — sinks/faucets
- Materiales de bounties/repetibles → craft de equip challenge
- **No regalar** items de builds de internet

## 7. Reset de personaje `implementado` → simplificar

**Actual:** `resetnpc.txt` — Canalizadora Mística, birthstone + 100k zeny.

**Tensión:** Reset difícil vs. incentivo multi-PJ.

**Dirección:** Reset barato/fácil; dificultad en challenges y progresión de equip. El doc viejo tenía reset **una vez por personaje** — hoy es repetible con birthstone.

**Comentarios:**

item custom: pergaminos de habilidad, para evitar la doblecuenta, se fabrican, asi como el potero fabrica pociones, se recolectan items y se hacen los pergaminos sin penalizaciones tan fuertes, solo dificultad por obtener los items.

## 8. Qué dejar de hacer

- [ ] Regalar items endgame de wikis
- [ ] Competir solo por MVP kills / zeny
- [ ] Rush a max level como única meta
- [ ] Esperar balance perfecto antes de jugar

## Notas de sesión

<!-- Feedback sobre sistemas al probar -->
