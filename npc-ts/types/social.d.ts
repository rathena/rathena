// Party / guild / instance / battleground / channel ops.

import type { AnnounceOpts } from "./api";

// ============================================================================
// Party
// ============================================================================

/**
 * Party ops. Accessed as `ctx.party`. Most calls need an explicit
 * `partyId` since `ctx.party` is the world-level surface (not the
 * attached player's party — for that, read `ctx.player.partyId`).
 */
export interface PartyOps {
    /** Party display name. NOT YET WIRED — stub. */
    getName(partyId: number): string;
    /** Member list as objects (charId, name, level, …). NOT YET WIRED — stub. */
    getMembers(partyId: number, type?: number): unknown[];
    /** Char id of the leader. NOT YET WIRED — stub. */
    getLeader(partyId: number, type?: number): number;
    /** True if the attached player is leader. NOT YET WIRED — stub. */
    isLeader(partyId?: number): boolean;
    /**
     * Create a new party.
     *
     * Mirrors rAthena's `party_create`. NOT YET WIRED — stub.
     *
     * @returns The new party id.
     */
    create(name: string, leaderCharId?: number, itemShare?: boolean, itemShareType?: number): number;
    /** Disband. NOT YET WIRED — stub. */
    destroy(partyId: number): void;
    /** Invite a character. NOT YET WIRED — stub. */
    addMember(partyId: number, charId: number): void;
    /** Kick a character. NOT YET WIRED — stub. */
    delMember(charId: number, partyId?: number): void;
    /** Promote a member to leader. NOT YET WIRED — stub. */
    changeLeader(partyId: number, charId: number): void;
    /** Toggle EXP / item share. NOT YET WIRED — stub. */
    changeOption(partyId: number, option: number, flag: boolean): void;
}

// ============================================================================
// Guild
// ============================================================================

/**
 * Guild ops. Accessed as `ctx.guild`.
 */
export interface GuildOps {
    getName(guildId: number): string;
    getMaster(guildId: number): string;
    getMasterId(guildId: number): number;
    /** Read a guild field by type code (level, ave_lv, exp, max members, …). */
    info(guildId: number, type: number): number;
    getMembers(guildId: number, type?: number): unknown[];
    /** Level of a specific guild skill. NOT YET WIRED — stub. */
    getSkillLv(guildId: number, skillId: number): number;
    /** Alliance / antagonist state between two guilds. NOT YET WIRED — stub. */
    getAlliance(g1: number, g2: number): number;
    /** Player count on a specific map belonging to the guild. NOT YET WIRED — stub. */
    getMapUsers(map: string, guildId: number): number;
    /** Transfer guild master. NOT YET WIRED — stub. */
    changeMaster(guildId: number, newMasterName: string): void;
    /** Force a guild-info refresh (and optionally fire a callback event). NOT YET WIRED — stub. */
    requestInfo(guildId: number, eventLabel?: string): void;
}

// ============================================================================
// Instance
// ============================================================================

/**
 * Instance ops. Accessed as `ctx.instance`. Mirrors rAthena's
 * `instance_*` buildins.
 */
export interface InstanceOps {
    /**
     * Create a new instance.
     *
     * @returns The new instance id (≥ 1) or a negative error code.
     */
    create(name: string, mode?: number, ownerId?: number): number;
    /** Tear down an instance. NOT YET WIRED — stub. */
    destroy(instanceId?: number): void;
    /**
     * Enter the instance with the attached player.
     *
     * @returns 0 on success, negative on error.
     */
    enter(name: string, x?: number, y?: number, charId?: number, instanceId?: number): number;
    /** Instance-mangled NPC name (`#<instanceId>` suffix). NOT YET WIRED — stub. */
    npcName(npcName: string, instanceId?: number): string;
    /** Instance-mangled map name (`<map>@<instanceId>`). NOT YET WIRED — stub. */
    mapName(map: string, instanceId?: number): string;
    /** Active instance id for the attached player. NOT YET WIRED — stub. */
    id(mode?: number): number;
    /** Warp every instance member to a tile. NOT YET WIRED — stub. */
    warpAll(map: string, x: number, y: number, instanceId?: number, flag?: number): void;
    /** Broadcast text to the instance. NOT YET WIRED — stub. */
    announce(instanceId: number, text: string, flag?: number, opts?: AnnounceOpts): void;
    /** Predicate: does the party meet entry requirements? NOT YET WIRED — stub. */
    checkParty(partyId: number, amount?: number, minLv?: number, maxLv?: number): boolean;
    checkGuild(guildId: number, amount?: number, minLv?: number, maxLv?: number): boolean;
    checkClan(clanId: number, amount?: number, minLv?: number, maxLv?: number): boolean;
    /** Static info about an instance template (`name`). NOT YET WIRED — stub. */
    info(name: string, infoType: number, mapIndex?: number): unknown;
    /** Live info about a running instance. NOT YET WIRED — stub. */
    liveInfo(infoType: number, instanceId?: number): unknown;
    /** Running instances visible from a map. NOT YET WIRED — stub. */
    list(map: string, mode?: number): unknown[];
    /** Read an instance variable. NOT YET WIRED — stub. */
    getVar(name: string, instanceId: number): unknown;
    /** Write an instance variable. NOT YET WIRED — stub. */
    setVar(name: string, value: unknown, instanceId: number): void;
}

// ============================================================================
// Battleground
// ============================================================================

/**
 * Battleground ops. Accessed as `ctx.bg`.
 */
export interface BattlegroundOps {
    /**
     * Create a new battleground team.
     *
     * @returns The new battleground id.
     */
    create(map: string, x: number, y: number, onQuitEvent?: string, onDeathEvent?: string): number;
    join(battleGroup: number, map?: string, x?: number, y?: number, charId?: number): number;
    setTeamXY(battleGroup: number, x: number, y: number): void;
    /** Reserve a queue slot for the map. NOT YET WIRED — stub. */
    reserve(map: string, ended?: boolean): number;
    unbook(map: string): void;
    /** Mark the attached player as deserter. NOT YET WIRED — stub. */
    desert(charId?: number): void;
    /** Warp every team member to a tile. NOT YET WIRED — stub. */
    warp(battleGroup: number, map: string, x: number, y: number): void;
    spawnMonster(battleGroup: number, map: string, x: number, y: number, displayName: string, mobId: number, eventLabel: string): number;
    setMonsterTeam(gid: number, battleGroup: number): void;
    leave(charId?: number): void;
    destroy(battleGroup: number): void;
    waitingRoomToBgSingle(battleGroup: number, map: string, x: number, y: number, npcName?: string): void;
    waitingRoomToBg(map: string, x: number, y: number, onQuitEvent?: string, onDeathEvent?: string, npcName?: string): void;
    /** Read team data (score, member count, …). NOT YET WIRED — stub. */
    getData(battleGroup: number, type: number): number;
    areaUsers(battleGroup: number, map: string, x0: number, y0: number, x1: number, y1: number): number;
    updateScore(map: string, guillaumeScore: number, croixScore: number): void;
    info(bgName: string, type: number): unknown;
}

// ============================================================================
// Channel
// ============================================================================

/**
 * Custom chat channel ops. Accessed as `ctx.channel`.
 */
export interface ChannelOps {
    /**
     * Create a new channel.
     *
     * Mirrors rAthena's `channel_create`. NOT YET WIRED — stub.
     */
    create(name: string, alias: string, password?: string, option?: number, delay?: number, color?: number, charId?: number): void;
    /** Make the attached player join. NOT YET WIRED — stub. */
    join(name: string, charId?: number): void;
    setOption(name: string, option: number, value: number): void;
    getOption(name: string, option: number): number;
    setColor(name: string, color: number): void;
    setPassword(name: string, password: string): void;
    /** Restrict access to specific group ids. NOT YET WIRED — stub. */
    setGroups(name: string, groupIds: number[]): void;
    /** Send a message in the channel. NOT YET WIRED — stub. */
    chat(name: string, message: string, color?: number): void;
    ban(name: string, charId: number): void;
    unban(name: string, charId: number): void;
    kick(name: string, charId: number): void;
}
