# Configuración del servidor

Fuente de verdad actual: `conf/import/battle_conf.txt`

Documento histórico de referencia (puede diferir de la config real). Actualizar cuando cambien rates.

## Información general

| Parámetro | Valor |
|-----------|-------|
| Episodio | 14.3 |
| Modo | Renewal |
| Nivel máximo | 175 / 60 |
| Acceso | VPN privada, solo invitación |
| Jugadores objetivo | ~3 (familiar) |

## Experiencia (rates actuales en `battle_conf.txt`)

| Parámetro | Rate | Notas |
|-----------|------|-------|
| Base EXP | 5x (`500`) | |
| Job EXP | 5x (`500`) | |
| MVP EXP | 10x (`1000`) | |
| Quest EXP | 3x (`300`) | |
| Multi level-up | Sí | |
| EXP bonus por atacante | +25% c/u | Diseño MMO; revisar en solo |
| Shop EXP (merchant) | Activado (`100`) | |
| Party even share bonus | 0 | Sin bonus extra por party size en config actual |

### Documento antiguo (referencia)

El doc viejo mencionaba **+80% EXP por miembro en party**. No está en `battle_conf.txt` actual — verificar si se perdió en la migración o se movió a otro archivo.

## Drops (rates actuales)

| Tipo | Rate config | Equivalente doc viejo |
|------|-------------|----------------------|
| Common (etc) | 5x (`500`) | Doc decía 10x |
| Heal / Use | 10x (`1000`) | Doc decía 20x consumibles |
| Equip | 15x (`1500`) | Doc decía 30x |
| Card | 100x (`10000`) | Doc decía 200x (~2%) |
| MVP items | 5x (`500`) | |
| Treasure (WoE) | 5x (`500`) | |
| Card boss | 100x (`10000`) | Doc decía 100x (~1% MVP/boss) |

## Otras reglas activas

| Regla | Estado | Archivo / nota |
|-------|--------|----------------|
| Zeny de mobs | Sí | `zeny_from_mobs: yes` |
| HP mobs x2 | Sí | `monster_hp_rate: 200` |
| HP MVP x2 | Sí | `mvp_hp_rate: 200` |
| Barra HP oculta (no MVP) | Sí | `monster_hp_bars_info: no` |
| Penalización zeny al morir | 1% | `zeny_penalty: 100` (doc viejo decía 10% — **cambió**) |
| Autoloot mercenario/homúnculo | Sí | |
| Anuncio drops raros (≤1%) | Sí | `rare_drop_announce: 100` |
| Vending tax | 0% | |
| Barter | On | `feature.barter: on` |
| Instancia reconnect | Sí | |
| Logros oficiales | Off | `feature.achievement: off` |
| Pet equip required | No | |
| Pet attack/support | Sí | |
| Pet level rate | 50% | |

## Personalizaciones pendientes de verificar

Del documento antiguo — confirmar si siguen activas o dónde viven:

- [ ] No requiere quest para primer job change
- [ ] Reborn con platinum skills desbloqueadas
- [ ] Penalización 5% por muerte (¿conflicto con `zeny_penalty: 1%`?)
- [ ] Mobs pueden hacer críticos
- [ ] Boss no se curan en combate
- [ ] Drop reducido 50% si diferencia de nivel ≥15
- [ ] ASPD max 193 / Third 195
- [ ] Bounty Missions → Cash Points + Special Gold
- [ ] Cash shop: Job Card Sets, Endow Scrolls, consumibles

## Comentarios / ajustes futuros

```
# Del doc viejo — pendientes de balance:
- Duplicar precio Crimson (cuando se reactive la quest)
- Bajar precios hunting a la mitad
- Cambiar ratio compra/venta cash points (no 1:1)
- Deshabilitar banco/correo del menú → solo vía NPC
```

## Notas de sesión

<!-- ¿Los rates se sienten bien? ¿Demasiado rush? ¿Falta zeny? -->
