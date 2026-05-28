// Ambient declarations for the host-injected scripting API.
//
// The rAthena map server (src/map/scripting/) installs the registrar
// globals into a V8 context before evaluating dist/main.js. Side-effect
// imports starting at main.ts trigger every register*() call at
// module-evaluation time; the accumulated registrations spawn NPCs
// and attach hooks at boot.
//
// IMPLEMENTATION STATUS: this is the minimum-viable surface for the
// engine-and-registrars milestone. The four dialog primitives
// (mes / next / select / close) are hand-implemented in C++. Player
// snapshot fields, world ops, party/guild/instance, item hooks, etc.
// land in follow-up milestones.
//
// SYNC-LOOKING SCRIPTS: all the awaits are inserted at build time by
// npc-ts/build/auto-await-plugin.mjs. Authors write `ctx.mes("…")` /
// `const c = ctx.select([…])` as if synchronous; the build emits the
// equivalent `await ctx.mes(…)` so V8 actually suspends correctly.

declare global {
    /**
     * Register one or more world-placed NPCs. Variadic.
     *
     * @example
     * registerNpc({ name: "kafra#01", map: "prontera", x: 155, y: 183,
     *               sprite: 115, async onClick(ctx) { ctx.mes("Hi!"); } });
     */
    function registerNpc(...npcs: NpcRegistration[]): void;

    type NpcContext      = import("./api").NpcContext;
    type NpcHandler      = import("./api").NpcHandler;
    type NpcRegistration = import("./api").NpcRegistration;
    type NpcDuplicate    = import("./api").NpcDuplicate;
}

/** Async handler — MUST be declared `async`. The auto-await build step
 *  inserts awaits in the body, which only work inside async. */
export type NpcHandler = (ctx: NpcContext) => Promise<void>;

export interface NpcRegistration {
    /** rAthena map_index name (no `.gat` suffix). */
    map: string;
    x: number;
    y: number;
    dir?: number;
    /** Numeric sprite class id. */
    sprite: number;
    /** Unique exname. Used as the click-dispatch key. */
    name: string;
    /** Optional touch trigger area centered on `(x, y)`. */
    triggerArea?: { xs: number; ys: number };

    /** Fires when a player clicks the NPC. */
    onClick?: NpcHandler;

    /**
     * Placed copies of this NPC. Each duplicate spawns at its own
     * `(map, x, y)` with its own `name` but SHARES the parent's
     * hook closures — mirrors rAthena's `duplicate(Name)` syntax.
     * `sprite` / `dir` / `triggerArea` fall back to the parent.
     *
     * @example
     * registerNpc({
     *     name: "Mailbox#prt1", map: "prontera", x: 146, y: 86, sprite: 888,
     *     async onClick(ctx) { ctx.mes("[Mailbox]"); ctx.close(); },
     *     duplicates: [
     *         { name: "Mailbox#prt2", map: "prontera", x: 275, y: 213 },
     *         { name: "Mailbox#iz",   map: "izlude",   x: 136, y:  94 },
     *     ],
     * });
     */
    duplicates?: NpcDuplicate[];
}

export interface NpcDuplicate {
    name: string;
    map: string;
    x: number;
    y: number;
    dir?: number;
    sprite?: number;
}

/**
 * The `ctx` argument every NPC hook receives. Only the four dialog
 * primitives are wired today; the broader surface lands in follow-up
 * milestones (player snapshot, world ops, party/guild/instance, etc.).
 */
export interface NpcContext {
    /**
     * Append a line to the open dialog window. Non-suspending.
     * Mirrors rAthena's `mes`.
     */
    mes(text: string): void;

    /**
     * Render a "Next" button and suspend until the player clicks it.
     * Mirrors rAthena's `next`.
     */
    next(): void;

    /**
     * Render a menu of choices and suspend until the player picks one.
     * 0-based index, -1 on cancel.
     */
    select(options: string[]): number;

    /** Alias of {@link select}. */
    menu(options: string[]): number;

    /**
     * Render a "Close" button and suspend until clicked. Ends the
     * dialog on resume.
     */
    close(): void;
}
