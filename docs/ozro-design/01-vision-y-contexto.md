# Visión y contexto

## Quiénes somos

Servidor familiar OzRo — Renewal, Episode 14.3, acceso por VPN privada (solo invitación). Pocos jugadores (~3), optimizado para LAN/offline, no para población masiva.

## Cómo jugamos hoy

- **Frecuencia:** ~2 veces al año, en vacaciones (~2 semanas intensas), luego abandono hasta la siguiente.
- **Conexión:** Intentamos jugar los 3 juntos, pero a veces no coinciden niveles → cada uno termina con ~10 personajes.
- **Multi-cuenta:** Cada uno tiene ~3 cuentas; a veces doble o triple cuenta en paralelo para suplir ausencias en party. Esto **entorpece** la experiencia.
- **Tiempo desigual:** Quien tiene más tiempo farmea mucho más → ventaja para los que no pueden dedicarle tanto.
- **Post-rush:** Tras subir a nivel alto, nos quedamos estancados sin saber qué hacer, los ultimos niveles son imposibles de subir.
- **Lore:** No hemos completado ni una misión del lore principal — no sabemos por dónde empezar.
- **Builds:** Miramos items/builds en internet y nos los damos como recompensa → rompe la progresión escalonada.
- **Competencia MVP:** Cacería de MVPs como competencia, pero penaliza healers y roles no-DPS.
- **Reset:** Reset infinito para probar builds/roles, pero si es difícil va en contra del incentivo de cambiar de PJ.

## Migración Hercules → rAthena

- rAthena solucionó varias cosas rotas en Hercules.
- Se perdieron algunos NPCs custom al migrar (ver [04-npcs.md](./04-npcs.md)).
- Algunas mecánicas que antes no funcionaban (ej. Crimson Weapons) ahora sí en rAthena.

## Hacia dónde queremos ir

### Objetivos

1. **Jugar entre vacaciones** — sesiones cortas (15–30 min) con progreso real.
2. **Meta por cuenta** — una cuenta principal; recompensas y colección que no se multipliquen con alts.
3. **Progreso escalonado** — rutas de equipo y desafíos deterministas, no RNG 0.001% ni regalos de wiki.
4. **Cosas para mostrar** — colección, casa/vitrina, prestigio visible (en juego y web), no solo poder. (inicialmente en la web)
5. **Roles variados** — incentivar distintos jobs/builds sin castigar soportes.
6. **Lore curado** — arcos oficiales seleccionados, con guía in-game según los vayamos haciendo.
7. **Server-side primero** — sin tocar el client por ahora (GRF es dolor de cabeza). Items custom experimentales más adelante.

### Principios de diseño

| Principio | Implicación |
|-----------|-------------|
| Disfrutar mientras armamos | Implementar en trozos pequeños; probar en la siguiente sesión |
| Cuenta > personaje | Variables `#` de cuenta, challenges, colección, prestigio |
| Coleccionista como modelo | El Card Collector ya funciona; replicar ese patrón |
| Sin atajos de poder | No regalar endgame de wikis; craft/cadenas por tier |
| Competencia justa | Métricas distintas para DPS, soporte, colección, lore |
| Reset fácil | Probar builds no debe costar; la dificultad va en challenges y equip |
| Web como vitrina | Rankings con incentivo real; perfil de cuenta |

## Problemas → soluciones (mapa rápido)

| Problema | Dirección |
|----------|-----------|
| 10 PJs, no juntamos niveles | Contenido por bracket + challenges con restricciones |
| Farm desigual por tiempo | Meta larga por cuenta (cartas, challenges, prestigio) |
| No sabemos empezar lore | Arcos curados + cronista como guía (ver [06-arcos-oficiales.md](./06-arcos-oficiales.md)) |
| Items de wiki rompen todo | Cadenas de craft/barter por nivel |
| MVP race injusta | Puntos por rol; rankings diversos (ver [07-web-api.md](./07-web-api.md)) |
| Rankings sin peso | Perfil de cuenta + recompensas cosméticas |
| Multi-cuenta para party | Diseñar para solo/duo; mercenarios, autoloot, rates actuales |
| Abandono post-2 semanas | Loop diario bajo esfuerzo entre vacaciones |

## Comentarios

<!-- Espacio libre para notas personales -->
