// Per-feature sub-objects hanging off ctx.player. Mirrors mmo-csharp's
// PlayerContext.{Quest,Achievement,Storage,Cart,Mail,Pet,Hom,Merc}.

import type { ItemOpts } from "./api";

// ============================================================================
// Quests
// ============================================================================

/**
 * Quest log ops. Accessed as `ctx.player.quest`.
 *
 * Most ops mirror rAthena buildins with the `quest` prefix.
 */
export interface QuestOps {
    /**
     * Add a quest to the player's log.
     *
     * Mirrors rAthena's `setquest`. NOT YET WIRED — stub.
     *
     * @example ctx.player.quest.add(7001);
     */
    add(questId: number): void;
    /** Mark a quest as complete. NOT YET WIRED — stub. */
    complete(questId: number): void;
    /** Remove a quest from the log. NOT YET WIRED — stub. */
    erase(questId: number): void;
    /** Replace one quest entry with another. NOT YET WIRED — stub. */
    change(fromId: number, toId: number): void;
    /**
     * Query quest progress.
     *
     * @param questId Quest id.
     * @param mode `"any"` (default) tests presence; `"playtime"` checks
     *   the playtime requirement; `"hunting"` checks hunt counters.
     * @returns 0 = not active, 1 = active, 2 = completed.
     */
    check(questId: number, mode?: "any" | "playtime" | "hunting"): number;
    /** True if the quest is currently in the BEGIN state. NOT YET WIRED — stub. */
    isBegin(questId: number): boolean;
    /** Show a quest-marker on the NPC head. NOT YET WIRED — stub. */
    showEvent(icon: number, markColor?: number): void;
    /** Force the client to re-fetch its quest list. NOT YET WIRED — stub. */
    refreshInfo(): void;
    /** Show a navigation marker tied to a quest. NOT YET WIRED — stub. */
    showInfo(icon: number, markColor?: number, condition?: string): void;
}

// ============================================================================
// Achievements
// ============================================================================

/**
 * Achievement system. Accessed as `ctx.player.achievement`.
 */
export interface AchievementOps {
    /** Grant the achievement. NOT YET WIRED — stub. */
    add(achievementId: number): void;
    /** Revoke. NOT YET WIRED — stub. */
    remove(achievementId: number): void;
    /** Mark as complete. NOT YET WIRED — stub. */
    complete(achievementId: number): void;
    /** True if the player has the achievement (any state). NOT YET WIRED — stub. */
    exists(achievementId: number): boolean;
    /** Read an achievement field by type code. NOT YET WIRED — stub. */
    info(achievementId: number, type: number): number;
    /** Bump a tracker counter (kill N of mob X, etc). NOT YET WIRED — stub. */
    update(achievementId: number, type: number, value: number): void;
}

// ============================================================================
// Storage
// ============================================================================

/**
 * Personal + guild storage. Accessed as `ctx.player.storage`. For UI
 * opening from a script, see also `ctx.player.openStorage()`.
 */
export interface StorageOps {
    /** Open the storage window. NOT YET WIRED — stub. */
    open(mode?: number): void;
    /** Open an extended storage (premium/event). NOT YET WIRED — stub. */
    openExtra(storageId: number, mode?: number): void;
    /** Count items in storage. NOT YET WIRED — stub. */
    countItem(itemId: number, opts?: ItemOpts): number;
    /** Remove from storage. NOT YET WIRED — stub. */
    delItem(itemId: number, amount: number, opts?: ItemOpts): void;
    /** Open the guild's shared storage. NOT YET WIRED — stub. */
    openGuildStorage(): void;
    countGuildItem(itemId: number, opts?: ItemOpts): number;
    delGuildItem(itemId: number, amount: number, opts?: ItemOpts): void;
    /** Read the guild-storage audit log. NOT YET WIRED — stub. */
    guildLog(): unknown[];
}

// ============================================================================
// Cart
// ============================================================================

/**
 * Cart ops (Merchant / Genetic / Mechanic). Accessed as `ctx.player.cart`.
 */
export interface CartOps {
    isEnabled(): boolean;
    countItem(itemId: number, opts?: ItemOpts): number;
    delItem(itemId: number, amount: number, opts?: ItemOpts): void;
}

// ============================================================================
// Mail
// ============================================================================

export interface MailOps {
    /** Open the mailbox UI. NOT YET WIRED — stub. */
    open(): void;
}

// ============================================================================
// Pet
// ============================================================================

/**
 * Pet ops. Accessed as `ctx.player.pet`.
 *
 * `skillBonus` / `skillSupport` / `skillAttack` mirror the pet AI
 * declarations in `db/(pre-)re/pet_db.yml`.
 */
export interface PetOps {
    /** Attempt to catch a wild pet. NOT YET WIRED — stub. */
    catchPet(itemId: number, flag?: number): void;
    /** Spawn an egg item for the pet id. NOT YET WIRED — stub. */
    makePet(petId: number): void;
    /** Hatch the player's first un-hatched egg. NOT YET WIRED — stub. */
    birthPet(): void;
    openIncubator(): void;
    /** Read pet info by type code (mirrors `getpetinfo`). NOT YET WIRED — stub. */
    info(type: number): unknown;
    /** Pet AI: timed stat bonus. */
    skillBonus(bonusType: number, value: number, durationMs: number, delayMs: number): void;
    /** Pet AI: support skill. */
    skillSupport(skillId: number, skillLv: number, delayMs: number, hpPct: number, spPct: number): void;
    /** Pet AI: offensive skill. */
    skillAttack(skillId: number, skillLv: number, rate: number, bonusRate: number): void;
    skillAttack2(skillId: number, damage: number, attacks: number, rate: number, bonusRate: number): void;
    /** Pet AI: scheduled status recovery. */
    recovery(statusType: number, delayMs: number): void;
    /** Pet AI: enable looting up to N items. */
    loot(maxItems: number): void;
}

// ============================================================================
// Homunculus
// ============================================================================

/**
 * Homunculus ops. Accessed as `ctx.player.hom`.
 */
export interface HomOps {
    exists(): boolean;
    isCalled(): boolean;
    info(type: number): unknown;
    /** Evolve from baby form. NOT YET WIRED — stub. */
    evolve(): void;
    /** Apply a Mutate (homunculus S). NOT YET WIRED — stub. */
    morph(): void;
    /** Mutate into a specific form by id. NOT YET WIRED — stub. */
    mutate(id?: number): void;
    /** Re-roll Homunculus stats. NOT YET WIRED — stub. */
    shuffle(): void;
    /** Adjust intimacy points. NOT YET WIRED — stub. */
    addIntimacy(amount: number): void;
}

// ============================================================================
// Mercenary
// ============================================================================

/**
 * Mercenary ops. Accessed as `ctx.player.merc`.
 */
export interface MercOps {
    /** Hire a mercenary for `contractTimeSec` seconds. NOT YET WIRED — stub. */
    create(classId: number, contractTimeSec: number): void;
    /** Dismiss the active mercenary. NOT YET WIRED — stub. */
    delete(reply?: number): void;
    heal(hp: number, sp?: number): void;
    /** Apply a status effect to the mercenary. NOT YET WIRED — stub. */
    scStart(type: number, durationMs: number, val1: number): void;
    /** Total kill count for a guild type (1 = neutral, 2 = morocc, 3 = orc). */
    getCalls(guildType: number): number;
    setCalls(guildType: number, value: number): void;
    /** Faith score for a guild type. */
    getFaith(guildType: number): number;
    setFaith(guildType: number, value: number): void;
    /** Read mercenary info by type code. NOT YET WIRED — stub. */
    info(type: number): unknown;
    elementalInfo(type: number): unknown;
}
