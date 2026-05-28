// Registrar payload shapes — the objects you pass to registerNpc /
// registerShop / registerWarp / etc.

import type { NpcHandler, ItemUseHandler, ItemEquipHandler } from "./api";

// ============================================================================
// NPCs
// ============================================================================

/**
 * A scripted NPC placed on a map. Drives `registerNpc(...)`.
 *
 * @example
 * registerNpc({
 *     name: "kafra_prt#01",
 *     map: "prontera",
 *     x: 155, y: 183,
 *     dir: 4,
 *     sprite: 115,
 *     async onClick(ctx) {
 *         ctx.mes("[Kafra]");
 *         ctx.mes("Welcome!");
 *         const choice = ctx.select(["Storage", "Cancel"]);
 *         if (choice === 0) ctx.player.openStorage();
 *         ctx.close();
 *     },
 * });
 */
export interface NpcRegistration {
    /** rAthena map_index name (no `.gat` suffix). */
    map: string;
    /** X coordinate on the map (1-based, like the legacy `npc/*.txt`). */
    x: number;
    /** Y coordinate on the map. */
    y: number;
    /** Facing direction: 0 = north, clockwise to 7. Default 0. */
    dir?: number;
    /** Numeric sprite class id (matches rAthena's NPC sprite ids in
     *  `db/(pre-)re/job_db.yml`). */
    sprite: number;
    /**
     * Unique identifier and display name. Must be globally unique
     * across the bundle — the host uses it as the click-dispatch key.
     * Convention: `LocalizedName#stableId`.
     */
    name: string;
    /**
     * Optional touch trigger area. When set, the NPC's `onTouch` hook
     * fires whenever a player walks into the area centered on `(x, y)`
     * with half-extents `xs` / `ys`.
     */
    triggerArea?: { xs: number; ys: number };

    /** Fires when a player clicks the NPC. */
    onClick?: NpcHandler;
    /** Fires when a player walks into `triggerArea`. */
    onTouch?: NpcHandler;
    /** Fires once at map-server boot, after all NPCs have spawned. */
    onInit?: NpcHandler;
    /** Recurring timer hooks keyed by interval in milliseconds. */
    onTimer?: Record<number, NpcHandler>;
    /** Fires when any player logs in (received by every NPC on the server). */
    onPCLogin?: NpcHandler;
    /** Fires when an attached player dies. */
    onPCDeath?: NpcHandler;
    /** Fires when an attached player kills another player. */
    onPCKill?: NpcHandler;
    /** Fires when an attached player kills a monster. */
    onNPCKill?: NpcHandler;

    /**
     * Placed copies of this NPC. Each duplicate spawns at its own
     * `(map, x, y)` and uses its own `name`, but SHARES the parent's
     * hook closures (`onClick`, `onTouch`, `onTimer`, …) — clicking
     * any duplicate runs the same handler that clicking the primary
     * would. Mirrors rAthena's `duplicate(Name)` syntax.
     *
     * `sprite`, `dir`, and `triggerArea` fall back to the parent's
     * values when not overridden on the duplicate.
     *
     * @example
     * // 30 mailboxes scattered around the world, one logic body.
     * registerNpc({
     *     name: "Mailbox#prt1",
     *     map: "prontera", x: 146, y: 86, sprite: 888,
     *     async onClick(ctx) {
     *         ctx.mes("[Mailbox]");
     *         ctx.mes("Pay 130z to use the mailbox?");
     *         if (ctx.select(["Yes", "No"]) !== 0) { ctx.close(); return; }
     *         if (ctx.player!.zeny < 130) {
     *             ctx.mes("Not enough zeny."); ctx.close(); return;
     *         }
     *         ctx.player!.zeny -= 130;
     *         ctx.player!.openMail();
     *         ctx.close();
     *     },
     *     duplicates: [
     *         { name: "Mailbox#prt2", map: "prontera", x: 275, y: 213 },
     *         { name: "Mailbox#prt3", map: "prontera", x:  34, y: 212 },
     *         { name: "Mailbox#iz",   map: "izlude",   x: 136, y:  94 },
     *         // …
     *     ],
     * });
     */
    duplicates?: NpcDuplicate[];
}

/**
 * One placed copy of a templated NPC. `name`, `map`, `x`, `y` are
 * required; everything else inherits from the parent registration.
 */
export interface NpcDuplicate {
    /** Unique exname for this duplicate. */
    name: string;
    /** Map name. */
    map: string;
    /** X tile coordinate. */
    x: number;
    /** Y tile coordinate. */
    y: number;
    /** Facing 0..7. Inherits from the parent when omitted. */
    dir?: number;
    /** Sprite class id. Inherits from the parent when omitted. */
    sprite?: number;
}

/**
 * Event-only NPC with no world position. Replaces rAthena's `-` map
 * sentinel for floating script blocks. Fired by name via
 * `ctx.doevent("Name::OnFoo")`.
 *
 * @example
 * registerFloatingNpc({
 *     name: "MidnightTicker",
 *     onClock: {
 *         "0000": async (ctx) => ctx.world.announce("It is now midnight."),
 *     },
 * });
 */
export interface FloatingNpcRegistration {
    /** Unique identifier used by `ctx.doevent("Name::OnLabel")`. */
    name: string;
    onInit?: NpcHandler;
    onTimer?: Record<number, NpcHandler>;
    /** Clock-driven hooks. Key is a 24-hour `"HHMM"` string. */
    onClock?: Record<string, NpcHandler>;
    onPCLogin?: NpcHandler;
    onPCDeath?: NpcHandler;
}

// ============================================================================
// Shops
// ============================================================================

/**
 * Declarative shop. The `kind` discriminator selects payment mode:
 * - `shop` — zeny
 * - `cash` — cash points
 * - `item` — pay with a specific item (`costItem`)
 * - `point` — pay with a permanent variable balance (`costVariable`)
 * - `market` — stocked items (per-item `stock`)
 *
 * @example
 * registerShop({
 *     kind: "shop",
 *     name: "Bakery",
 *     map: "prontera", x: 156, y: 219, sprite: 60,
 *     items: [
 *         { itemId: 538, price: 50 },
 *         { itemId: 539, price: 70 },
 *     ],
 * });
 */
export type ShopRegistration =
    | ({ kind: "shop"   } & ShopBase & { items: ShopItem[] })
    | ({ kind: "cash"   } & ShopBase & { items: ShopItem[] })
    | ({ kind: "item"   } & ShopBase & { costItem: number;     items: ShopItem[] })
    | ({ kind: "point"  } & ShopBase & { costVariable: string; items: ShopItem[] })
    | ({ kind: "market" } & ShopBase & { items: MarketShopItem[] });

interface ShopBase {
    map: string;
    x: number;
    y: number;
    dir?: number;
    sprite: number;
    name: string;
}

export interface ShopItem {
    /** rAthena `item_db.id` of the item. */
    itemId: number;
    /** Price in the shop's currency (zeny / cash points / costItem / costVariable). */
    price: number;
    /** Per-item discount percentage. Valid only for `kind: "item"` / `"point"`. */
    discount?: number;
}

export interface MarketShopItem {
    itemId: number;
    price: number;
    /** Per-item stock count. Decrements on purchase. */
    stock: number;
}

// ============================================================================
// Warps
// ============================================================================

/**
 * Declarative warp portal. Players who walk into the `area` half-extent
 * around `(from.x, from.y)` are teleported to `(to.map, to.x, to.y)`.
 *
 * @example
 * registerWarp({
 *     from: { map: "prontera", x: 273, y: 354 },
 *     area: { xs: 2, ys: 2 },
 *     to:   { map: "prt_fild05", x: 175, y: 39 },
 * });
 */
export interface WarpRegistration {
    from: { map: string; x: number; y: number };
    /** Trigger half-extent. The active area is
     *  `(x-xs..x+xs, y-ys..y+ys)`. */
    area: { xs: number; ys: number };
    to: { map: string; x: number; y: number };
    /** `"warp2"` triggers for hidden players too. Default `"warp"`. */
    type?: "warp" | "warp2";
}

// ============================================================================
// Map flags
// ============================================================================

/**
 * Declarative map flag. Mirrors rAthena's `npc/re/mapflag/*.txt`.
 *
 * @example
 * registerMapFlag({ map: "prontera", flag: "nomemo" });
 * registerMapFlag({ map: "guild_vs5", flag: "pvp", value: "on" });
 */
export interface MapFlagRegistration {
    map: string;
    /** Flag name verbatim from rAthena: `"pvp"`, `"gvg"`, `"nobranch"`, … */
    flag: string;
    /** Optional value when the flag carries one (e.g. `"100"` for `restricted`). */
    value?: string;
}

// ============================================================================
// Items
// ============================================================================

/**
 * Per-item script hooks. The id is the rAthena `item_db.id`; every
 * other catalog column (name, type, weight, slots, …) is sourced from
 * SQL and is NOT redeclared here.
 *
 * @example
 * // Old Blue Box gives random zeny on use.
 * registerItem({
 *     id: 668,
 *     async onUse(ctx) {
 *         ctx.player.zeny += ctx.randRange(1000, 10000);
 *     },
 * });
 */
export interface ItemRegistration {
    /** Numeric item id (rAthena `item_db.id`). Globally unique across the bundle. */
    id: number;
    /** Fires when a player uses the item (potions, scrolls, boxes). Async. */
    onUse?: ItemUseHandler;
    /** Fires on equip success. Sync — must not suspend. Typically pushes
     *  bonuses via `ctx.bonus(...)`. */
    onEquip?: ItemEquipHandler;
    /** Fires on unequip. Sync, same contract as `onEquip`. */
    onUnequip?: ItemEquipHandler;
}

/**
 * Combo registration — fires `onActive` when every listed aegis-named
 * member item is equipped simultaneously.
 */
export interface ComboRegistration {
    /** Combo id from rAthena's `item_combo_db`. Preserved for traceability. */
    comboId: number;
    /** Aegis names of every item that must be simultaneously equipped to fire. */
    members: string[];
    /** Fires during equip recalc when every member is equipped. Sync. */
    onActive?: ItemEquipHandler;
}

// ============================================================================
// Spawns
// ============================================================================

/**
 * Declarative mob spawn point.
 *
 * @example
 * registerSpawn({
 *     map: "prt_fild08",
 *     area: { x: 0, y: 0, xs: 100, ys: 100 },
 *     mobId: 1002,           // Poring
 *     amount: 30,
 *     respawn: { baseMs: 5000, jitterMs: 2000 },
 * });
 */
export interface SpawnRegistration {
    map: string;
    /** Spawn area. Omit for "anywhere walkable on the map". */
    area?: { x: number; y: number; xs: number; ys: number };
    mobId: number;
    amount: number;
    respawn?: { baseMs: number; jitterMs?: number };
    boss?: boolean;
    /** Display name override. Empty / undefined uses `mob_db` name. */
    name?: string;
    /** Event label fired on death (e.g. `"DungeonBoss::OnKilled"`). */
    onDeath?: string;
    /** Size override. 0 = mob_db default, 1 = small, 2 = large. */
    size?: 0 | 1 | 2;
    /** AI mode override. */
    ai?: number;
}
