# Ideas backlog

Lluvia de ideas del documento antiguo, **reformuladas** y etiquetadas. No es lista de tareas — es inventario para priorizar cuando toque.

**Leyenda:** `implementado` | `en-repo` | `pendiente` | `reformular` | `descartado` | `probar`

---

## Alta prioridad (alineadas con visión actual)

| Idea | Estado | Notas reformuladas |
|------|--------|-------------------|
| Challenges por cuenta con build/dificultad | `pendiente` | Ver [03-sistemas-core.md](./03-sistemas-core.md) |
| Meta colección cartas + web | `implementado` / `pendiente` | Collector hecho; falta % en web |
| Cronista como guía de lore | `reformular` | Expandir `chronicler.txt` |
| Daily Hunting Quest con racha | `en-repo` | `hunting_missions.txt` desactivado |
| Casas / refugios instanciados | `pendiente` | Sin client; dolls como decoración |
| Recompensas por cuenta (prestigio) | `pendiente` | Reducir incentivo multi-cuenta |
| Crimson Weapons + Marcus | `reformular` | rAthena OK; ver [05-quests-custom.md](./05-quests-custom.md) |
| Quests equipo pro con lore | `reformular` | = challenges + cadenas craft, no regalos |
| Restructurar NPC cartas (tienda + misiones) | `reformular` | Extender card_collector |
| Mercado partes equip común (barter) | `pendiente` | Barter NPC por tier; precios referencia PlayRO |

---

## Economía y QoL

| Idea | Estado | Notas |
|------|--------|-------|
| Deshabilitar banco/correo del menú → solo NPC | `pendiente` | Más inmersivo; scripts bank en repo |
| Cash points compra/venta no 1:1 | `probar` | `points2zeny.txt` |
| Reducir votos Kafra Voting Dungeon a 5 | `pendiente` | Si usan el dungeon |
| Monke: frutas aleatorias (cualquiera) | `probar` | Doc: estaba roto |
| Thrasher (compra junk caro) | `en-repo` | Sink zeny |
| Stock Market NPC | `en-repo` | `npc/custom/etc/stock_market.txt` |
| Blessed Ores en vez de Safe tickets + quest lore | `pendiente` | |
| Incluir más cajas cashshop + World Tour Ticket crafteable | `pendiente` | |
| Deadly Potion, Holy Water en cashshop | `pendiente` | |

---

## Actividades diarias / entre sesiones

| Idea | Estado | Notas |
|------|--------|-------|
| Daily Roulette (web → juego) | `pendiente` | [07-web-api.md](./07-web-api.md) |
| Daily Guess the Monster | `pendiente` | Menos pistas = más premio |
| Fishing (pesca) | `reformular` | Diseñar balance antes |
| Chef (cocina con fallo / puntos ingredientes) | `reformular` | Relacionar con cooking quest oficial |
| Treasure Hunter semanal | `pendiente` | NPC examen monstruo + llave diaria + cofre en mapa random |
| Quest cadena → desbloquea cacería tesoro | `pendiente` | Pistas mundo → explorador |
| Eventos automáticos + calendario | `pendiente` | `floating_rates.txt` como base |
| NPC errante semanal (misiones/acertijos) | `pendiente` | "¿Dónde está ahora?" |

---

## Combate, equipamiento y progresión

| Idea | Estado | Notas |
|------|--------|-------|
| Hungry Weapons (arma pide cacerías, sube nivel) | `reformular` | Similar a armas evolutivas |
| Armas que evolucionan con uso/sacrificios | `reformular` | Integrar en challenges |
| Champion mobs (Furious/Swift/etc.) → gemas elementales | `pendiente` | Encantamiento permanente |
| MVP instancia con mutaciones | `pendiente` | Late game grupo pequeño |
| Mazmorras con llaves (daily/trade/kills) | `pendiente` | |
| Instancia oleadas temáticas (zombies, orcos) | `pendiente` | Party pequeña |
| Bloody Branch Manufacturer | `pendiente` | |
| MVP Hunting gacha (items comunes → monedas → gacha) | `reformular` | Riesgo power creep; mejor barter determinista |
| Sets custom con skills de otros jobs + Almas | `reformular` | Solo ultra-late o cosmético |
| Dark Ritual: reset a Novice 1 → SoulStone valiosa | `reformular` | ¿Conflicto con reset fácil? |
| NPC buffs por temporadas (esferas monk, adrenaline) | `pendiente` | Eventos estacionales |
| Old Blacksmith: Special Gold → piedras BS | `pendiente` | Depende de bounty rewards |

---

## Mascotas y social

| Idea | Estado | Notas |
|------|--------|-------|
| Craft comida mascotas raras | `pendiente` | |
| Intercambio huevos / mercado mascotas | `pendiente` | Incentivar catching |
| Sistema afinidad/reputación NPC | `pendiente` | Diálogos según reputación |
| Matrimonio / eventos sociales | `en-repo` | `marriage.txt` en custom genérico |

---

## Casa / vitrina (detalle)

| Idea | Estado | Notas |
|------|--------|-------|
| Vender casas en ciudad o refugios campo | `pendiente` | Instancia individual |
| Decorar con dolls | `pendiente` | Sin client: limitado |
| Vitrina cartas para enseñar | `pendiente` | NPC o instancia |
| Storage más grande / cofres múltiples | `pendiente` | Viable con pocos jugadores |
| Contratar NPCs de servicio (pago mantenimiento) | `pendiente` | |
| Carta inobtenible aleatoria por precio absurdo | `reformular` | Muy jackpot; evaluar sink extremo |

---

## Minijuegos y flavor

| Idea | Estado | Notas |
|------|--------|-------|
| NPC minijuegos (RPS, dados, lotería) | `en-repo` | blackjack, lottery, rpsroulette en custom |
| Story Teller (historias junto al fuego) | `pendiente` | Complemento cronista, no reemplazo arcos |
| Lore al Traveler Tool Dealer | `pendiente` | |
| Rebirth Soul en Soul Linker | `pendiente` | `soul_link.txt` en repo |

---

## Ideas ChatGPT (doc viejo) — reformuladas

| Idea original | Estado | Integración |
|---------------|--------|-------------|
| Minijuegos NPC | `en-repo` | Scripts ya en `npc/custom/etc/` |
| Story Teller | `pendiente` | Zonas del mundo, historias cortas |
| Armas evolutivas | `reformular` | → Hungry Weapons / challenges |
| NPC errante semanal | `pendiente` | Evento rotativo |
| MVP mutados en instancia | `pendiente` | Late game |
| Llaves para mazmorras | `pendiente` | Daily + trade |
| Reputación NPC | `pendiente` | Largo plazo |

---

## Descartadas o en pausa

| Idea | Motivo |
|------|--------|
| Servidor "balanceado y listo" antes de jugar | Reemplazado por enfoque incremental |
| Libro con todas las quests extraídas | Preferir guía in-game + arcos curados |
| Competencia principal por MVP/zeny | Injusta para tiempo y roles |
| Regalar equip de wikis | Rompe progresión |

---

## Cómo priorizar

1. ¿Funciona con 3 jugadores y 2 semanas/año?
2. ¿Progreso por cuenta?
3. ¿Sin tocar client?
4. ¿Se puede probar en una tarde?

Si 3+ son sí → subir prioridad.

## Comentarios

algunas ideas extra que no he organizado:
añadir gachas y sistemas basados en tiempo, tomando inspiracion de otros juegos como los idle games o juegos de gestion en celular, yo hago cosas pero debo entrar a revisar para obtener beneficios y cosas asi, tipo npc granjero, etc...

quizas tener retos que se hagan fuera del juego pero que tengan impacto en el juego, que se hagan en la pagina web o retos y asi.

## Notas de sesión

<!-- Qué idea probamos next -->
