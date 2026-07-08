# OzRo — Diseño del servidor

Documentación viva de ideas, sistemas y decisiones para el servidor privado OzRo (rAthena).

> **Filosofía:** No esperar a tener el servidor balanceado y listo para disfrutarlo. Ir armando, probando y ajustando mientras se juega. La retroalimentación de cada sesión alimenta el diseño.

## Cómo usar estos documentos

Cada archivo es independiente y editable. Convenciones:

| Etiqueta | Significado |
|----------|-------------|
| `implementado` | Activo en el servidor |
| `en-repo` | Script existe pero está desactivado o incompleto |
| `pendiente` | Por hacer |
| `reformular` | Idea válida pero hay que replantearla |
| `probar` | Implementado a medias; falta jugar y ajustar |
| `descartado` | Ya no aplica o se rechazó |
| `hercules` | Existía en Hercules; revisar al portar |

La sección **Comentarios** al final de cada archivo es un borrador libre: se apuntan ideas sueltas al jugar y luego se **organizan** dentro de las secciones estructuradas del propio documento.

## Flujo de trabajo

- **Ideas nuevas:** apuntarlas en *Comentarios* del archivo que corresponda; después moverlas a la tabla o sección adecuada.
- **Al implementar algo:** borrar el detalle de planificación y dejar solo una línea marcada como `implementado`. No acumular documentación de cosas ya hechas — la doc se adelgaza a medida que el servidor crece.
- **Al crear contenido nuevo, actualizar la web:**
  - Sección **Info** del sitio → solo el *listado* de NPCs custom y quests custom.
  - Sección **Enciclopedia** → guías y detalles específicos (si aplica).

## Índice

| Archivo | Contenido |
|---------|-----------|
| [01-vision-y-contexto.md](./01-vision-y-contexto.md) | Quiénes somos, cómo jugamos, problemas y principios de diseño |
| [02-configuracion-servidor.md](./02-configuracion-servidor.md) | Rates, reglas y personalizaciones técnicas |
| [03-sistemas-core.md](./03-sistemas-core.md) | Sistemas prioritarios: challenges, meta por cuenta, loops diarios |
| [04-npcs.md](./04-npcs.md) | Inventario de NPCs custom: activos, desactivados, perdidos |
| [05-quests-custom.md](./05-quests-custom.md) | Quests propias (Crimson Weapons, etc.) |
| [06-arcos-oficiales.md](./06-arcos-oficiales.md) | Lore oficial curado por temporadas; guías y ajustes |
| [07-web-api.md](./07-web-api.md) | Integración web, rankings, perfil de cuenta |
| [08-ideas-backlog.md](./08-ideas-backlog.md) | Lluvia de ideas antigua reformulada y priorizada |

## Repos relacionados

- **Servidor:** `rathena/` — scripts en `npc/custom/ozro/`, config en `conf/import/`
- **API:** `ozro-backup/src/api/server.js`
- **Web:** `ozro-site/`

## Decisiones recientes

<!-- Actualizar aquí cuando se tomen decisiones importantes -->

- **2026-07:** Meta de progreso por **cuenta principal**, no por personaje. Incentivar una cuenta por jugador en lugar de multi-cuenta para suplir party.
- **2026-07:** Arcos oficiales se curan poco a poco; buscar guías en internet y crear pistas/ayudas según se vayan jugando.
- **2026-07:** Sin cambios al client por ahora. Todo server-side + web.
- **2026-07:** Crimson Weapons — en Hercules estaban rotas; en rAthena ya se pueden conseguir. Dirección: reactivar el flujo pero que la quest **imbuya elemento** (Marcus) en vez de crear las armas.
