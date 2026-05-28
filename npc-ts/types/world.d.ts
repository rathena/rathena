// WorldOps — world-level ops accessed as `ctx.world`. Announcements,
// mob spawning, unit ops, map flags, time, battle config, …

import type { ItemOpts } from "./api";

/**
 * Options for the announce family. Color is RGB packed as `0xRRGGBB`.
 *
 * @example
 * { flag: bc_all, color: 0x00FF00 }       // global green
 * { flag: bc_map, color: 0xFF0000 }       // current-map red
 */
export interface AnnounceOpts {
    /** Target / source flags packed: bc_all | bc_map | bc_self | bc_npc | bc_pc. */
    flag?: number;
    /** RGB color (0xRRGGBB). */
    color?: number;
    /** Font weight (FW_NORMAL = 400, FW_BOLD = 700). */
    fontSize?: number;
    fontType?: number;
    /** Horizontal alignment. */
    fontAlign?: number;
    /** Y offset from top of screen. */
    fontY?: number;
}

/**
 * Options for `unitSkillUse*` — pre-cast / target / range tweaks.
 */
export interface UnitSkillOpts {
    targetId?: number;
    castTime?: number;
    cancel?: number;
    lineId?: number;
    ignoreRange?: boolean;
}

/**
 * World-level ops accessed as `ctx.world`. Operations here don't need
 * a player attached — they operate on the map server, a specific map,
 * or an arbitrary unit by GID.
 */
export interface WorldOps {
    /** Server tick in milliseconds since boot. */
    now(): number;

    // ========== Announce family ============================================

    /**
     * Broadcast a message.
     *
     * Mirrors rAthena's `announce`.
     *
     * @param message Text to broadcast.
     * @param opts Target flags + color. Default is `bc_all | bc_pc` in yellow.
     *
     * @example
     * ctx.world.announce("Server restart in 5 minutes!");
     * ctx.world.announce("WoE starts now!", { flag: bc_all, color: 0x00FF00 });
     */
    announce(message: string, opts?: AnnounceOpts): void;

    /**
     * Broadcast to every player on a specific map.
     *
     * Mirrors rAthena's `mapannounce`.
     *
     * @example
     * ctx.world.mapAnnounce("prontera", "Free buffs at the fountain!");
     */
    mapAnnounce(map: string, message: string, opts?: AnnounceOpts): void;

    /**
     * Broadcast to every player inside a map rectangle.
     *
     * Mirrors rAthena's `areaannounce`.
     */
    areaAnnounce(map: string, x1: number, y1: number, x2: number, y2: number, message: string, opts?: AnnounceOpts): void;

    /** Server-wide message in everyone's chat box. */
    globalMessage(message: string, fromNpcName?: string): void;
    debugMessage(message: string): void;
    errorMessage(message: string): void;
    logMessage(message: string): void;

    // ========== Sound / BGM ================================================

    /**
     * Play a sound to every player in a map / area.
     *
     * Mirrors rAthena's `soundeffectall`.
     */
    soundEffectAll(filename: string, type?: number, map?: string, x0?: number, y0?: number, x1?: number, y1?: number): void;
    playBgmAll(filename: string, map?: string, x0?: number, y0?: number, x1?: number, y1?: number): void;

    // ========== Monsters / units ===========================================

    /**
     * Spawn N copies of a monster at a specific tile.
     *
     * Mirrors rAthena's `monster`.
     *
     * @param displayName Empty string uses `mob_db` default name.
     * @param onDeathEvent Optional event target fired when the mob dies.
     * @returns The first spawned mob's runtime block-list id.
     *
     * @example
     * ctx.world.spawnMob("prt_fild08", 100, 100, "Apple", 1002, 30);
     * ctx.world.spawnMob("dungeon_01", 50, 50, "Boss", 1115, 1, "DungeonNpc::OnBossDead");
     */
    spawnMob(map: string, x: number, y: number, displayName: string, mobId: number, amount?: number, onDeathEvent?: string): number;
    /**
     * Spawn N copies within a map rectangle.
     *
     * Mirrors rAthena's `areamonster`.
     */
    spawnAreaMob(map: string, x1: number, y1: number, x2: number, y2: number, displayName: string, mobId: number, amount?: number, onDeathEvent?: string): number;
    /**
     * Spawn a castle guardian.
     *
     * Mirrors rAthena's `guardian`. NOT YET WIRED — stub.
     */
    spawnGuardian(map: string, x: number, y: number, displayName: string, mobId: number, onDeathEvent?: string, guardianIndex?: number): number;
    guardianInfo(map: string, guardianIndex: number, type: number): unknown;
    /**
     * Kill every monster on `map` whose death event matches.
     *
     * Mirrors rAthena's `killmonster`.
     */
    killMonster(map: string, eventLabel: string): number;
    /** Kill every monster on a map. Mirrors `killmonsterall`. */
    killMonsterAll(map: string): number;
    /** Count monsters on a map whose death event matches. */
    mobCount(map: string, eventLabel: string): number;
    /** Re-spawn castle defenders for a guild. */
    respawnGuildOwned(map: string, guildId: number, flag?: number): void;
    /** Random mob id by tier / level. */
    getRandomMobId(type: number, flag?: number, level?: number): number;
    getMonsterInfo(mobId: number, type: number): unknown;
    getMobDrops(mobId: number): unknown[];
    mobInfo(type: number, mobId: number): string;

    // ========== Unit-level ops (any GID) ===================================

    /** Walk a unit to a tile. */
    unitWalk(gid: number, x: number, y: number, onArriveEvent?: string): void;
    unitWalkToTarget(gid: number, targetGid: number, onArriveEvent?: string): void;
    unitAttack(gid: number, targetGid: number, actionType?: number): void;
    unitKill(gid: number): void;
    unitWarp(gid: number, map: string, x: number, y: number): void;
    /**
     * Make a unit "speak" — shows a chat bubble.
     *
     * Mirrors rAthena's `unittalk`.
     */
    unitTalk(gid: number, text: string, flag?: number): void;
    /** Cast a skill at a target unit. */
    unitSkillUseId(gid: number, skillId: number, skillLv: number, opts?: UnitSkillOpts): void;
    /** Cast a skill at a ground tile. */
    unitSkillUsePos(gid: number, skillId: number, skillLv: number, x: number, y: number, opts?: UnitSkillOpts): void;
    unitStopAttack(gid: number): void;
    unitStopWalk(gid: number, flag?: number): void;
    unitExists(gid: number): boolean;
    getUnitType(gid: number): number;
    getUnitName(gid: number): string;
    setUnitName(gid: number, name: string): void;
    getUnitTitle(gid: number): string;
    setUnitTitle(gid: number, title: string): void;
    getUnitData(gid: number): unknown;
    /** Read/write a unit-data parameter (HP, max HP, walk speed, …). */
    setUnitData(gid: number, parameter: number, value: unknown): void;
    /** All units of a type, globally. NOT YET WIRED — stub. */
    getUnits(type: number): number[];
    /** All units of a type on a specific map. NOT YET WIRED — stub. */
    getMapUnits(type: number, map: string): number[];
    /** All units of a type inside a map rectangle. NOT YET WIRED — stub. */
    getAreaUnits(type: number, map: string, x1: number, y1: number, x2: number, y2: number): number[];

    // ========== Player / area queries ======================================

    /**
     * Player count on a map.
     *
     * Mirrors rAthena's `getmapusers`.
     */
    getMapUsers(map: string): number;
    /** Player count in a map rectangle. */
    getAreaUsers(map: string, x1: number, y1: number, x2: number, y2: number): number;
    /** Total online players. */
    getServerUsers(type?: number): number;
    /** True if the account / character is currently online. */
    isLoggedIn(accountId: number, charId?: number): boolean;
    /** Character name for a runtime block-list id. */
    ridToName(rid: number): string;
    /** Items on the floor in a rectangle. NOT YET WIRED — stub. */
    getAreaDropItem(map: string, x1: number, y1: number, x2: number, y2: number, itemId?: number): unknown[];

    // ========== Map / location =============================================

    /** Map name for a map id. */
    mapIdToName(mapId: number): string;
    /**
     * Position of a unit (any unit type).
     *
     * Mirrors rAthena's `getmapxy`.
     *
     * @returns `{ map, x, y }` or `null` if the unit can't be found.
     */
    getMapXY(gid: number, type?: number): { map: string; x: number; y: number } | null;
    /** Euclidean tile distance. */
    distance(x0: number, y0: number, x1: number, y1: number): number;
    /** Mark a region of cells with a flag. */
    setCell(map: string, x1: number, y1: number, x2: number, y2: number, type: number, flag: boolean): void;
    /** Check a single cell's flag. */
    checkCell(map: string, x: number, y: number, type: number): number;
    /** Find a walkable cell near `(x, y)`. NOT YET WIRED — stub. */
    getFreeCell(map: string, x?: number, y?: number, rangeX?: number, rangeY?: number, flag?: number): { x: number; y: number } | null;
    /** Create a named wall barrier. */
    setWall(map: string, x: number, y: number, size: number, dir: number, shootable: boolean, name: string): void;
    delWall(name: string): void;
    checkWall(name: string): boolean;
    /** Drop an item on the ground. */
    makeItem(itemId: number, amount: number, map: string, x: number, y: number, effect?: boolean, opts?: ItemOpts): void;
    /** Remove dropped items inside a rectangle. */
    cleanArea(map: string, x1: number, y1: number, x2: number, y2: number): void;
    /** Remove every dropped item on a map. */
    cleanMap(map: string): void;
    /** Create a one-shot warp portal NPC. */
    warpPortal(srcX: number, srcY: number, toMap: string, toX: number, toY: number): void;
    mapWarp(fromMap: string, toMap: string, x: number, y: number, type?: number, id?: number): void;
    areaWarp(fromMap: string, x1: number, y1: number, x2: number, y2: number, toMap: string, toX: number, toY: number, toX2?: number, toY2?: number): void;
    warpParty(toMap: string, x: number, y: number, partyId: number, fromOpts?: { map?: string; rangeX?: number; rangeY?: number }): void;
    warpGuild(toMap: string, x: number, y: number, guildId: number): void;
    /** Percentage heal everyone in a map rectangle. */
    areaPercentHeal(map: string, x1: number, y1: number, x2: number, y2: number, hp: number, sp: number): void;
    /** Attach a specific account-id as `rid` for subsequent ops. NOT YET WIRED — stub. */
    attachRid(accountId: number, force?: boolean): void;
    addRid(type: number, flag?: number, parameters?: unknown): void;
    playerAttached(): number;
    getAttachedRid(): number;

    // ========== Map flags ==================================================

    /** Set a map flag (pvp, gvg, nobranch, …). */
    setMapFlag(map: string, flag: number, zone?: string, type?: number): void;
    removeMapFlag(map: string, flag: number, zone?: string): void;
    getMapFlag(map: string, flag: number, type?: number): number;
    setMapFlagNoSave(map: string, altMap: string, x: number, y: number): void;

    // ========== Day / night / pvp / gvg / agit =============================

    /** Force daytime. */
    day(): void;
    /** Force nighttime. */
    night(): void;
    isDay(): boolean;
    isNight(): boolean;
    pvpOn(map: string): void;
    pvpOff(map: string): void;
    gvgOn(map: string): void;
    gvgOff(map: string): void;
    /** WoE 3.0 variants. */
    gvgOn3(map: string): void;
    gvgOff3(map: string): void;
    /** Start WoE. */
    agitStart(era?: number): void;
    agitEnd(era?: number): void;
    agitCheck(era?: number): boolean;
    /** Refresh a castle's emblem display. */
    flagEmblem(guildId: number): void;
    /** Castle name for a WoE map. */
    castleName(map: string): string;
    castleData(map: string, type: number): number;

    // ========== Time / weather =============================================

    getTime(type: number): number;
    getTimeTick(tickType: number): number;
    getTimeStr(format: string, maxLength: number, tick?: number): string;

    // ========== Battle / config ============================================

    /** Set a battle config flag at runtime. */
    setBattleFlag(flagName: string, value: number, reload?: boolean): void;
    getBattleFlag(flagName: string): number;

    // ========== At-commands ================================================

    /** Run an @command as if typed by a GM. */
    atCommand(command: string): void;
    charCommand(command: string): void;
    useAtCommand(command: string): void;
    /** Map an @command to an event label. */
    bindAtCommand(command: string, eventTarget: string, atLevel?: number, charLevel?: number): void;
    unbindAtCommand(command: string): void;

    // ========== Game-info queries ==========================================

    itemName(itemId: number): string;
    itemSlots(itemId: number): number;
    itemInfo(itemId: number, type: number): unknown;
    setItemInfo(itemId: number, type: number, value: number): void;
    /** Overwrite an item's `Script` block. */
    setItemScript(itemId: number, script: string, type?: number): void;
    gmLevel(charId?: number): number;
    groupId(charId?: number): number;
    /** Item-link tooltip text (rich-chat format). NOT YET WIRED — stub. */
    itemLink(itemId: number, opts?: ItemOpts): string;
}
