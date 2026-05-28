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
     * Mirrors rAthena's `setquest`.
     *
     * @example ctx.player.quest.add(7001);
     */
    add(questId: number): void;
    /** Mark a quest as complete. */
    complete(questId: number): void;
    /** Remove a quest from the log. */
    erase(questId: number): void;
    /** Replace one quest entry with another. */
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
    /** True if the quest is currently in the BEGIN state. */
    isBegin(questId: number): boolean;
    /** Show a quest-marker on the NPC head. */
    showEvent(icon: number, markColor?: number): void;
    /** Force the client to re-fetch its quest list. */
    refreshInfo(): void;
    /** Show a navigation marker tied to a quest. */
    showInfo(icon: number, markColor?: number, condition?: string): void;
}

// ============================================================================
// Achievements
// ============================================================================

/**
 * Achievement system. Accessed as `ctx.player.achievement`.
 */
export interface AchievementOps {
    /** Grant the achievement. */
    add(achievementId: number): void;
    /** Revoke. */
    remove(achievementId: number): void;
    /** Mark as complete. */
    complete(achievementId: number): void;
    /** True if the player has the achievement (any state). */
    exists(achievementId: number): boolean;
    /** Read an achievement field by type code. */
    info(achievementId: number, type: number): number;
    /** Bump a tracker counter (kill N of mob X, etc). */
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
    /** Open the storage window. */
    open(mode?: number): void;
    /** Open an extended storage (premium/event). */
    openExtra(storageId: number, mode?: number): void;
    /** Count items in storage. */
    countItem(itemId: number, opts?: ItemOpts): number;
    /** Remove from storage. */
    delItem(itemId: number, amount: number, opts?: ItemOpts): void;
    /** Open the guild's shared storage. */
    openGuildStorage(): void;
    countGuildItem(itemId: number, opts?: ItemOpts): number;
    delGuildItem(itemId: number, amount: number, opts?: ItemOpts): void;
    /** Read the guild-storage audit log. */
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
    /** Open the mailbox UI. */
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
    /** Attempt to catch a wild pet. */
    catchPet(itemId: number, flag?: number): void;
    /** Spawn an egg item for the pet id. */
    makePet(petId: number): void;
    /** Hatch the player's first un-hatched egg. */
    birthPet(): void;
    openIncubator(): void;
    /** Read pet info by type code (mirrors `getpetinfo`). */
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
    /** Evolve from baby form. */
    evolve(): void;
    /** Apply a Mutate (homunculus S). */
    morph(): void;
    /** Mutate into a specific form by id. */
    mutate(id?: number): void;
    /** Re-roll Homunculus stats. */
    shuffle(): void;
    /** Adjust intimacy points. */
    addIntimacy(amount: number): void;
}

// ============================================================================
// Mercenary
// ============================================================================

/**
 * Mercenary ops. Accessed as `ctx.player.merc`.
 */
export interface MercOps {
    /** Hire a mercenary for `contractTimeSec` seconds. */
    create(classId: number, contractTimeSec: number): void;
    /** Dismiss the active mercenary. */
    delete(reply?: number): void;
    heal(hp: number, sp?: number): void;
    /** Apply a status effect to the mercenary. */
    scStart(type: number, durationMs: number, val1: number): void;
    /** Total kill count for a guild type (1 = neutral, 2 = morocc, 3 = orc). */
    getCalls(guildType: number): number;
    setCalls(guildType: number, value: number): void;
    /** Faith score for a guild type. */
    getFaith(guildType: number): number;
    setFaith(guildType: number, value: number): void;
    /** Read mercenary info by type code. */
    info(type: number): unknown;
    elementalInfo(type: number): unknown;
}
