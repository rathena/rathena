// Contexts for item hooks (onUse / onEquip / onUnequip / combo onActive).

import type { PlayerContext, WorldOps } from "./api";

/**
 * Context handed to `onUse` hooks (potions, scrolls, boxes).
 *
 * @example
 * registerItem({
 *     id: 501,  // Red Potion
 *     async onUse(ctx) {
 *         ctx.player.heal(45 + ctx.rand(20), 0);
 *     },
 * });
 */
export interface ItemUseContext {
    /** The acting player. Same surface as `NpcContext.player`. */
    player: PlayerContext;
    /** World ops (announce, spawn, etc.). Same surface as `NpcContext.world`. */
    world: WorldOps;
    /** The item triggering this hook (id, refine, slot, etc.). */
    item: ItemInfo;

    /** Uniform RNG `[0, n)`. Variadic to absorb both 1- and 2-arg forms. */
    rand(...args: number[]): number;
    /** Inclusive RNG `[min, max]`. */
    randRange(min: number, max: number): number;

    /**
     * Escape hatch for converter-emitted DSL calls (`sc_start`,
     * `getgroupitem`, `itemheal`, `laphine_synthesis`, …). Unknown
     * names return a no-op function that yields 0. Hand-written items
     * should prefer the explicit methods on `player` / `world`.
     */
    [builtin: string]: any;
}

/**
 * Context handed to `onEquip` / `onUnequip` / combo `onActive`. These
 * fire during equip recalc on the game loop and MUST NOT suspend.
 *
 * @example
 * registerItem({
 *     id: 4001,  // Poring Card
 *     onEquip(ctx) {
 *         ctx.bonus("bLuk", 2);
 *         ctx.bonus2("bAddRace", "RC_All", 1);
 *     },
 * });
 */
export interface ItemEquipContext {
    /** The acting player. Read-only during recalc. */
    player: PlayerContext;
    /** The item triggering this hook. */
    item: ItemInfo;

    // ========== Bonus mutation =============================================

    /**
     * Flat bonus. Mirrors `bonus <bKey>, <val>;`.
     *
     * @example
     * ctx.bonus("bStr", 3);
     * ctx.bonus("bDefEle", "Ele_Ghost");
     */
    bonus(key: string, value?: number | string): void;
    /** Indexed bonus (race / element / class slot). */
    bonus2(key: string, index: string | number, value: number | string): void;
    bonus3(key: string, a: string | number, b: string | number, value: number | string): void;
    bonus4(key: string, a: string | number, b: string | number, c: string | number, value: number | string): void;
    bonus5(key: string, a: string | number, b: string | number, c: string | number, d: string | number, value: number | string): void;

    /** Trigger a sub-script on attack with `rate` chance for `durationMs`. */
    autobonus(body: string, rate: number, durationMs: number, atkType?: string | number, onfailScript?: string): void;
    /** Same, but fires when the player is hit. */
    autobonus2(body: string, rate: number, durationMs: number, atkType?: string | number, onfailScript?: string): void;
    /** Same, but fires on a specific skill. */
    autobonus3(body: string, rate: number, durationMs: number, skillName: string, onfailScript?: string): void;

    // ========== SC / skill family ==========================================

    sc_start(...args: any[]): void;
    sc_start2(...args: any[]): void;
    sc_start4(...args: any[]): void;
    sc_end(...args: any[]): void;
    /** Grant a permanent skill while equipped. */
    skill(...args: any[]): void;

    // ========== HP / SP / effect family ====================================

    heal(...args: any[]): void;
    percentheal(...args: any[]): void;
    itemheal(...args: any[]): void;
    /** Visual-only effect (no-op in equip context). */
    specialeffect(...args: any[]): void;
    specialeffect2(...args: any[]): void;
    hateffect(...args: any[]): void;
    petloot(...args: any[]): void;
    setoption(...args: any[]): void;
    message(...args: any[]): void;
    dispbottom(...args: any[]): void;

    // ========== Equip queries ==============================================

    /** Right-hand refine. */
    getrefine(): number;
    getequiprefinerycnt(slot: string | number): number;
    getequipid(slot: string | number): number;
    getequipweaponlv(slot?: string | number): number;
    getenchantgrade(slot?: string | number): number;

    // ========== PC queries =================================================

    readparam(name: string | number): any;
    /** Whitelisted PC param read (Class, Luk, …). */
    getParam(name: string | number): any;
    getskilllv(skillName: string): number;
    getiteminfo(...args: any[]): number;
    getitemcount(...args: any[]): number;
    countitem(...args: any[]): number;
    checkoption(...args: any[]): number;
    checkmount(...args: any[]): number;
    isequipped(...args: any[]): number;
    isequippedcnt(...args: any[]): number;
    basicskillcheck(): number;
    checkfalcon(): number;
    checkriding(): number;
    checkcart(): number;
    checkidle(): number;
    /** Expanded-class bitmask. */
    eaclass(...args: any[]): number;

    // ========== Math helpers ===============================================

    max(...args: number[]): number;
    min(...args: number[]): number;
    pow(a: number, b: number): number;
    rand(...args: number[]): number;

    // ========== Pet queries ================================================

    /** Read pet info (egg id, name, level, hunger, intimacy, …). */
    getpetinfo(type: string | number): number | string;

    /** Escape hatch — unknown buildins return a no-op function. */
    [builtin: string]: any;
}

/**
 * Snapshot of the item triggering an item hook.
 */
export interface ItemInfo {
    /** rAthena `item_db.id`. */
    readonly id: number;
    /** Aegis name (`name_aegis` column). */
    readonly nameAegis: string;
    /** Refine level. */
    readonly refine: number;
    /** Card slot count. */
    readonly slot: number;
    /** Stack size in the inventory. */
    readonly amount: number;
}
