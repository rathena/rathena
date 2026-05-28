// =====================================================================
// rAthena TS scripting — top-level API surface
// =====================================================================
//
// This file is the BARREL — it pulls together the split type files
// (registrations / npc / player / player-ops / world / social / item)
// and declares the global registrar functions injected by the host.
//
// Authors usually never edit this file. To add or change a method's
// signature, edit the relevant themed file:
//
//   npc-ts/types/registrations.d.ts  → NpcRegistration, ShopRegistration, …
//   npc-ts/types/npc.d.ts            → NpcContext, NpcInfo
//   npc-ts/types/player.d.ts         → PlayerContext, ItemOpts, …
//   npc-ts/types/player-ops.d.ts     → QuestOps / StorageOps / PetOps / …
//   npc-ts/types/world.d.ts          → WorldOps, AnnounceOpts
//   npc-ts/types/social.d.ts         → Party / Guild / Instance / Bg / Channel
//   npc-ts/types/item.d.ts           → ItemUseContext, ItemEquipContext
//
// The C++ stubs are codegen'd from these declarations by
// tools/scripting/gen_bindings.py. To graduate a stub to a real impl,
// add `"Interface.method"` to `api_surface.hand_impl` in
// tools/scripting/bindings.yaml, regenerate (or just `make`), then
// write the C++ method in src/map/scripting/dialog_context.cpp.
//
// SYNC-LOOKING SCRIPTS: all the awaits are inserted at build time by
// npc-ts/build/auto-await-plugin.mjs. Authors write `ctx.mes("…")` /
// `const c = ctx.select([…])` as if synchronous; the build emits the
// equivalent `await ctx.mes(…)` so V8 actually suspends correctly.

declare global {
    // ========== Registrars ==============================================

    /**
     * Register one or more world-placed NPCs. Variadic.
     *
     * @example
     * registerNpc({ name: "kafra#01", map: "prontera", x: 155, y: 183,
     *               sprite: 115, async onClick(ctx) { ctx.mes("Hi!"); } });
     */
    function registerNpc(...npcs: NpcRegistration[]): void;

    /**
     * Register an event-only NPC with no world position. Use for clock
     * timers, global tickers, library functions, etc.
     */
    function registerFloatingNpc(...npcs: FloatingNpcRegistration[]): void;

    /**
     * Register a named user-function callable via `ctx.callfunc(name, ...args)`.
     * Mirrors rAthena's `function<Name>` script-side declarations — used by
     * scripts to share logic without going through an NPC's event-label tree.
     *
     * @example
     * registerFunction("F_GetWeekDay", (ctx, day) => ["Sun","Mon","Tue","Wed","Thu","Fri","Sat"][day]);
     * // Anywhere: const name = ctx.callfunc("F_GetWeekDay", 3);  // "Wed"
     */
    function registerFunction(name: string, fn: (ctx: NpcContext, ...args: unknown[]) => unknown): void;

    /** Register one or more declarative shops. */
    function registerShop(...shops: ShopRegistration[]): void;

    /** Register one or more declarative warp portals. */
    function registerWarp(...warps: WarpRegistration[]): void;

    /** Register one or more declarative mob spawn points. */
    function registerSpawn(...spawns: SpawnRegistration[]): void;

    /** Register one or more declarative map flags. */
    function registerMapFlag(...flags: MapFlagRegistration[]): void;

    /**
     * Register one or more item-script hooks. The item's catalog row
     * (name, type, weight, slots, …) is owned by SQL — only `id` and
     * the hooks live here.
     */
    function registerItem(...items: ItemRegistration[]): void;

    /** Register combo activations. Fires when every member is equipped. */
    function registerCombo(...combos: ComboRegistration[]): void;

    // ========== Ambient type aliases ====================================

    type NpcContext        = import("./api").NpcContext;
    type ItemUseContext    = import("./api").ItemUseContext;
    type ItemEquipContext  = import("./api").ItemEquipContext;
    type NpcHandler        = import("./api").NpcHandler;
    type ItemUseHandler    = import("./api").ItemUseHandler;
    type ItemEquipHandler  = import("./api").ItemEquipHandler;

    type NpcRegistration         = import("./api").NpcRegistration;
    type FloatingNpcRegistration = import("./api").FloatingNpcRegistration;
    type ShopRegistration        = import("./api").ShopRegistration;
    type WarpRegistration        = import("./api").WarpRegistration;
    type SpawnRegistration       = import("./api").SpawnRegistration;
    type MapFlagRegistration     = import("./api").MapFlagRegistration;
    type ItemRegistration        = import("./api").ItemRegistration;
    type ComboRegistration       = import("./api").ComboRegistration;
}

// =====================================================================
// Handler types
// =====================================================================

/** Async handler — MUST be declared `async`. The auto-await build step
 *  inserts awaits in the body, which only work inside async. */
export type NpcHandler = (ctx: NpcContext) => Promise<void>;

/** Async handler — MUST be declared `async`. */
export type ItemUseHandler = (ctx: ItemUseContext) => Promise<void>;

/** SYNC handler — fires during equip recalc on the game loop and must
 *  not return a Promise. Used for `onEquip` / `onUnequip` / combo
 *  `onActive`. */
export type ItemEquipHandler = (ctx: ItemEquipContext) => void;

// =====================================================================
// Re-exports from the split files (the actual interface bodies)
// =====================================================================

export type {
    NpcRegistration, NpcDuplicate, FloatingNpcRegistration,
    ShopRegistration, ShopItem, MarketShopItem,
    WarpRegistration,
    MapFlagRegistration,
    ItemRegistration, ComboRegistration,
    SpawnRegistration,
} from "./registrations";

export type { NpcContext, NpcInfo } from "./npc";

export type {
    PlayerContext,
    ItemOpts, InventoryEntry, SkillEntry,
} from "./player";

export type {
    QuestOps, AchievementOps, StorageOps, CartOps,
    MailOps, PetOps, HomOps, MercOps,
} from "./player-ops";

export type {
    WorldOps, AnnounceOpts, UnitSkillOpts,
} from "./world";

export type {
    PartyOps, GuildOps, InstanceOps,
    BattlegroundOps, ChannelOps,
} from "./social";

export type {
    ItemUseContext, ItemEquipContext, ItemInfo,
} from "./item";
