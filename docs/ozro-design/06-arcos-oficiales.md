# Arcos oficiales (lore del juego)

## Enfoque acordado

- **No** extraer todo el lore a un libro de una vez.
- **No** tener el servidor listo antes de jugar.
- Buscar **guías en internet** (iRO wiki, RateMyServer, etc.) por arco.
- Crear **pistas y ayudas in-game** (cronista, NPCs) solo para el arco que estemos jugando.
- **Ajustar recompensas** de NPCs si la quest exige items muy difíciles de conseguir en nuestro servidor (pocos jugadores, sin mercado).

## Por qué no hemos empezado

- Decenas de archivos en `npc/quests/` y `npc/re/quests/`
- Sin guía clara de por dónde empezar
- Foco histórico en rush a max level, no en narrativa

## Propuesta: temporadas curadas

En cada período de vacaciones (~2 semanas), **un arco** con 8–15 quests encadenadas.

### Plantilla de arco

```markdown
## Arco: [Nombre]
- Episodio / nivel sugerido:
- Ciudad base:
- Guía de referencia (URL):
- Quests (orden):
  1. ...
  2. ...
- NPC ayuda OzRo: Kaz el Cronista / otro
- Ajustes de recompensas necesarios:
- Variables cuenta: #LORE_ARC_[ID]
- Estado: pendiente | en-progreso | completado | descartado
```

---

## Arcos candidatos

<!-- Ir llenando según se elijan. No comprometerse a todos. -->

### Arco 1 — Por definir `pendiente`

| Campo | Valor |
|-------|-------|
| Nivel | 1–40 |
| Zona | Prontera → Geffen → Payon |
| Guía | <!-- URL --> |
| Quests | <!-- lista --> |
| Notas | Buen primer arco para temporada |

### Arco 2 — Por definir `pendiente`

| Campo | Valor |
|-------|-------|
| Nivel | 40–70 |
| Zona | Morocc / Sphinx |
| Guía | |
| Quests | |

### Arco 3 — Por definir `pendiente`

| Campo | Valor |
|-------|-------|
| Nivel | 70+ |
| Zona | Rachel / Thanatos / Juperos |
| Guía | |
| Quests | |

---

## Arcos largos (referencia, no prioritarios)

Quests épicas del repo — para más adelante:

| Arco | Archivo(s) aprox. |
|------|-------------------|
| Kiel Hyre | `npc/quests/kiel_hyre_quest.txt` |
| Thanatos | `npc/quests/thana_quest.txt` |
| Juperos | `npc/quests/quests_juperos.txt` |
| Seals (god items) | `npc/quests/seals/` |
| 13.1 / 13.2 | `npc/quests/quests_13_1.txt`, `quests_13_2.txt` |
| Niflheim | `npc/quests/quests_niflheim.txt` |
| Eyes of Hellion | `npc/quests/eye_of_hellion.txt` |

---

## Integración con challenges

Un challenge puede **requerir** completar un arco lore como hito (ej. "Acolyte a 60 + arco Payon").

## Integración con cronista

Cuando un arco esté activo, Kaz el Cronista debe poder decir:
- Dónde ir
- Qué nivel recomendado
- Si ya completaste capítulos (`#LORE_ARC_*`)

## Ruta de historias / crónicas de episodios `pendiente`

Revisar el **historial de episodios** del juego e implementar NPCs que cuenten las viejas historias (cómo era Rune-Midgard antes de aliarse con Arunafeltz, antes de la grieta interdimensional, etc.).

- Se arma una **ruta encadenada**: un NPC lleva al siguiente y cada uno cuenta un trozo de la historia.
- Complementa al cronista, no reemplaza los arcos jugables (relacionado con *Story Teller* del [backlog](./08-ideas-backlog.md)).

## Mapeo de quests en la web `pendiente`

A medida que se **traducen/documentan** las quests del juego, se publican en la sección **Enciclopedia** del sitio (ver [07-web-api.md](./07-web-api.md)) y se mapea:

- Qué NPCs están **"linkeados"** (accesibles desde un *starting point* ya documentado en la web) y cuáles no.
- El **% de quests activas mapeadas**.
- Identificar **NPCs huérfanos** (sin ruta documentada de acceso).

## Checklist al preparar un arco

- [ ] Leer guía externa completa
- [ ] Probar primeras 3 quests en servidor de prueba
- [ ] Listar items requeridos y verificar obtainability (drops, NPCs, craft)
- [ ] Ajustar recompensas / barter si algo es imposible
- [ ] Escribir diálogos de pista (mínimos)
- [ ] Definir variable de progreso por cuenta
- [ ] Jugar en vacaciones y anotar ajustes/feedback en *Comentarios*

## Comentarios

<!-- Borrador libre para ideas de arcos/lore; luego organizar arriba -->
