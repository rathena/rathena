// NpcContext (the `ctx` object handed to every hook) + NpcInfo
// (`ctx.npc` — info about the bound NPC).

import type {
    PlayerContext, WorldOps, PartyOps, GuildOps,
    InstanceOps, BattlegroundOps, ChannelOps,
} from "./api";
import type { ShopItem } from "./registrations";

/**
 * The argument every NPC hook receives. Composes the four hand-written
 * dialog primitives, a set of flow utilities, and seven nested
 * sub-objects (npc / player / world / party / guild / instance / bg /
 * channel).
 *
 * Scripts are sync-looking. Behind the scenes the build step (see
 * `npc-ts/build/auto-await-plugin.mjs`) inserts `await` in front of
 * every `ctx.X(...)` call, so the handler MUST be declared `async` —
 * V8 needs an async function frame to host the awaits.
 */
export interface NpcContext {
    /** The NPC the hook is bound to. Read-only facts + display ops. */
    npc: NpcInfo;
    /**
     * Attached player. `null` for `onInit`, `onTimer`, and `onClock`
     * hooks where no player is in context.
     */
    player: PlayerContext | null;
    /** World-level queries, broadcasts, mob spawning, map flags, etc. */
    world: WorldOps;
    /** Party ops (create / leader / members). */
    party: PartyOps;
    /** Guild ops (info / master / members). */
    guild: GuildOps;
    /** Instance ops (create / enter / vars). */
    instance: InstanceOps;
    /** Battleground ops. */
    bg: BattlegroundOps;
    /** Channel ops. */
    channel: ChannelOps;

    // ========== Dialog primitives ==========================================
    //
    // These are the four hand-written ops in DialogContext. Every other
    // method on ctx is currently a no-op stub that logs the call.

    /**
     * Append a line to the open dialog window. Non-suspending — the
     * client just appends the text and the script continues immediately.
     *
     * Mirrors rAthena's `mes`.
     *
     * @param text Line of dialog text. Can include rAthena color codes.
     *
     * @example
     * ctx.mes("[Kafra Employee]");
     * ctx.mes("Welcome to the Kafra Corporation!");
     * ctx.mes("How may I help you today?");
     */
    mes(text: string): void;

    /**
     * Render a "Next" button and suspend until the player clicks it.
     * Used between pages of dialog text.
     *
     * Mirrors rAthena's `next`.
     *
     * @example
     * ctx.mes("First page of the story…");
     * ctx.next();
     * ctx.mes("…and the second page.");
     */
    next(): void;

    /**
     * Render a menu of choices and suspend until the player picks one.
     *
     * Mirrors rAthena's `select`/`menu`. The TS contract differs from
     * the legacy script engine: indices are **0-based** here (legacy
     * is 1-based), and a player Escape returns **-1** (legacy returns 0).
     *
     * @param options Labels shown in the menu, in display order.
     * @returns 0-based index of the chosen option, or -1 on cancel.
     *
     * @example
     * const pick = ctx.select(["Buy", "Sell", "Storage", "Cancel"]);
     * if (pick === 0) ctx.mes("Let's see what you'd like to buy.");
     * else if (pick === -1) ctx.mes("Come back anytime!");
     */
    select(options: string[]): number;

    /**
     * Alias of {@link select}. Both render as the same `ZC_MENU_LIST` packet.
     *
     * @param options Labels shown in the menu, in display order.
     * @returns 0-based index of the chosen option, or -1 on cancel.
     */
    menu(options: string[]): number;

    /**
     * Render a "Close" button and suspend until the player clicks it.
     * The dialog ends after the resume; further `ctx.mes(...)` will not
     * show.
     *
     * Mirrors rAthena's `close`.
     *
     * @example
     * ctx.mes("See you next time!");
     * ctx.close();
     */
    close(): void;

    // ========== Flow utilities (STUBS for now) =============================

    /**
     * Numeric input dialog. The client shows a number-only field.
     *
     * Mirrors rAthena's `input`. Suspends until the client returns the
     * entered integer; the auto-await transform makes this read as sync.
     *
     * @param min Lower bound (inclusive). Default 0.
     * @param max Upper bound (inclusive). Default `INT32_MAX`.
     * @param defaultValue Pre-filled value.
     * @returns The entered number.
     *
     * @example
     * ctx.mes("How many would you like?");
     * const amount = ctx.input(1, 100);
     * ctx.mes(`You picked ${amount}.`);
     */
    input(min?: number, max?: number, defaultValue?: number): number;

    /**
     * Text input dialog (free-form string). Suspends until the client
     * sends the typed text back.
     *
     * @param defaultValue Pre-filled string.
     * @returns The entered text.
     *
     * @example
     * ctx.mes("Type the password:");
     * const pwd = ctx.inputString();
     */
    inputString(defaultValue?: string): string;

    /**
     * Suspend execution for the given duration.
     *
     * @param milliseconds Sleep duration.
     *
     * @example
     * ctx.mes("Wait a moment…");
     * ctx.sleep(2000);
     * ctx.mes("There you go.");
     */
    sleep(milliseconds: number): void;

    /**
     * Fire an event label on another (floating or placed) NPC.
     *
     * Mirrors rAthena's `doevent`.
     *
     * @param eventTarget Target in the form `"NpcName::OnLabel"`.
     *
     * @example
     * ctx.doevent("MidnightTicker::OnTrigger");
     */
    doevent(eventTarget: string): void;

    /**
     * Schedule a one-shot timer that fires an event label after the
     * given delay.
     *
     * Mirrors rAthena's `addtimer`.
     */
    addTimer(milliseconds: number, eventTarget: string): void;

    /** Cancel a previously-scheduled timer. */
    delTimer(eventTarget: string): void;

    /** Schedule a per-player timer. */
    addPlayerTimer(charId: number, milliseconds: number, eventTarget: string): void;

    /**
     * Call another NPC's user-function. For the TS
     * engine, prefer plain JS imports over cross-NPC calls.
     */
    callfunc(functionName: string, ...args: unknown[]): unknown;

    /** End the script early. */
    end(): void;

    /**
     * Clear all dialog text but keep the dialog open. Useful between
     * "pages" without a "Next" prompt.
     *
     * Mirrors rAthena's `cleararray`-style dialog reset.
     */
    clear(): void;

    /**
     * Uniform RNG in `[0, max)`.
     *
     * Mirrors rAthena's single-arg `rand(max)`.
     *
     * @param max Exclusive upper bound.
     *
     * @example
     * const roll = ctx.rand(100);  // 0..99
     */
    rand(max: number): number;

    /**
     * Uniform RNG in `[min, max]` inclusive on both ends.
     *
     * Mirrors rAthena's two-arg `rand(min, max)`.
     *
     * @example
     * const dmg = ctx.randRange(50, 100);  // any of 50, 51, …, 100
     */
    randRange(min: number, max: number): number;
}

// ============================================================================
// NpcInfo
// ============================================================================

/**
 * Facts + ops scoped to the NPC the current hook is bound to. Exposed
 * as `ctx.npc`. All operations target THIS NPC; for arbitrary NPCs,
 * use {@link WorldOps.unitWalk} / {@link WorldOps.unitTalk} with a GID.
 */
export interface NpcInfo {
    /** Map name (rAthena map_index). */
    readonly map: string;
    /** X tile coordinate. */
    readonly x: number;
    /** Y tile coordinate. */
    readonly y: number;
    /** Facing direction 0..7. */
    readonly dir: number;
    /** Display name (and unique identifier across the bundle). */
    readonly name: string;
    /** Numeric sprite class id. */
    readonly sprite: number;
    /**
     * NPC-local memory-only variable bag. Resets on script reload.
     * Useful for counters, cached state, etc.
     */
    vars: Record<string, unknown>;

    // ========== Display / movement (STUBS) =================================

    /**
     * Change the NPC's display name + class + (optionally) size.
     *
     * Mirrors rAthena's `setunitdata` + `setunitname`.
     *
     * @example ctx.npc.setDisplay("Disguised Stranger", 4_W_M_01, 1);
     */
    setDisplay(displayName: string, classId: number, size?: number): void;

    /** Set the NPC's walk speed (lower = faster). Default 200. */
    speed(value: number): void;

    /** Make the NPC walk to a target tile. */
    walkTo(x: number, y: number): void;

    /** Stop the NPC's current movement. */
    stop(clearTarget?: boolean): void;

    /** Teleport the NPC to a new tile (no walk animation). */
    moveTo(x: number, y: number, dir?: number): void;

    /** Hide the NPC from clients. */
    hide(): void;

    /** Show the NPC again after `hide()`. */
    show(): void;

    /** Disable the NPC entirely — no click, no touch, no display. */
    disable(): void;

    /** Re-enable a `disable()`d NPC. */
    enable(): void;

    /** Spawn a per-player dynamic duplicate. */
    duplicateDynamic(charId?: number): void;

    // ========== Shop ops ===================================================

    /** Replace the shop's item list. */
    shopSet(items: ShopItem[]): void;
    /** Append items to the shop. */
    shopAdd(items: ShopItem[]): void;
    /** Remove items by id. */
    shopDel(itemIds: number[]): void;
    /** Attach / detach the shop UI to the calling player. */
    shopAttach(flag?: boolean): void;
    /** Update an item's price (and stock for market shops). */
    shopUpdate(itemId: number, price: number, stock?: number): void;

    // ========== Waiting room ===============================================

    /**
     * Create a waiting room (chat room) attached to the NPC.
     *
     * @param roomName Title shown to clients.
     * @param limit Max participants.
     * @param opts Optional event trigger, entry cost, level gates.
     */
    createWaitingRoom(
        roomName: string,
        limit: number,
        opts?: { eventLabel?: string; trigger?: number; zeny?: number; minLv?: number; maxLv?: number },
    ): void;
    removeWaitingRoom(): void;
    enableWaitingRoom(): void;
    disableWaitingRoom(): void;
    /** Query waiting-room state (count, limit, …) by infoType code. */
    getWaitingRoomState(infoType: number): number;
    /** Warp all participants to `(map, x, y)`. `count` caps the warp. */
    warpWaitingPc(map: string, x: number, y: number, count?: number): void;
    /** Kick a single participant by name. */
    kickWaitingRoomUser(charName: string): void;
    /** Kick everyone. */
    kickAllWaitingRoom(): void;
    /** Current participant count. */
    getWaitingRoomUsers(): number;

    /** Read NPC timer info. */
    npcTimer(infoType: number): number;
    /** Get the runtime block-list id of an NPC by type. */
    getNpcId(type: number): number;
    /** Read a string fact about the NPC (name, ai class, etc.). */
    npcInfo(type: number): string;
}
