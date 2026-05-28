// PlayerContext — the player-scoped sub-object exposed as `ctx.player`.
// The data fields (name / hp / zeny / str / …) are snapshotted at click
// time by the libclang-driven `player_binding.generated.cpp`; the
// methods listed here are stubs unless they're in
// tools/scripting/bindings.yaml: api_surface.hand_impl.

import type {
    QuestOps, AchievementOps, StorageOps, CartOps,
    MailOps, PetOps, HomOps, MercOps,
} from "./api";

/**
 * The clicking player. The data fields are a snapshot read at click
 * time — they don't track mutations made later in the same dialog.
 * The methods are the action surface (heal, warp, giveItem, …).
 */
export interface PlayerContext {
    // ========== Identity ===================================================
    /** Character id (alias of `charId`). */
    readonly id: number;
    /** Character id (rAthena `mmo_charstatus.char_id`). */
    readonly charId: number;
    /** Account id (rAthena `mmo_charstatus.account_id`). */
    readonly accountId: number;
    /** Character name. */
    readonly name: string;
    /** 0 = female, 1 = male (rAthena convention). */
    readonly sex: number;
    /** Numeric class / job id. See `db/(pre-)re/job_db.yml`. */
    readonly classId: number;
    /** Permission group id. */
    readonly groupId: number;
    /** Alias of `groupId`. */
    readonly gmLevel: number;
    /** Active party id, or 0 if not in a party. */
    readonly partyId: number;
    /** Active guild id, or 0 if guildless. */
    readonly guildId: number;

    // ========== Level ======================================================
    /** Character base level (1..max). */
    readonly baseLevel: number;
    /** Character job level (1..max). */
    readonly jobLevel: number;

    // ========== Position / weight ==========================================
    /** Total carried weight (current). */
    readonly weight: number;
    /** Maximum carry weight (base + bonus). */
    readonly maxWeight: number;
    /** Current map name. */
    readonly mapName: string;
    /** X tile coordinate. */
    readonly x: number;
    /** Y tile coordinate. */
    readonly y: number;
    /** Facing direction 0..7. */
    readonly dir: number;

    // ========== Base stats =================================================
    readonly str: number;
    readonly agi: number;
    readonly vit: number;
    readonly int: number;
    readonly dex: number;
    readonly luk: number;
    /** Unspent status points. */
    readonly statusPoint: number;
    /** Unspent skill points. */
    readonly skillPoint: number;

    // ========== Vitals =====================================================
    /** Current HP. Setter clamps to `[0, maxHp]` and broadcasts SP_HP. */
    hp: number;
    readonly maxHp: number;
    /** Current SP. Setter clamps to `[0, maxSp]` and broadcasts SP_SP. */
    sp: number;
    readonly maxSp: number;
    /** Activity points (4th-job mechanic). */
    readonly ap: number;
    readonly maxAp: number;
    /** Currency. Setter clamps to ≥ 0 and broadcasts SP_ZENY. */
    zeny: number;

    // ========== Variable bags ==============================================
    /** Memory-only variable bag (rAthena `@var`). Cleared on logout. */
    session: Record<string, unknown>;
    /** Per-character permanent (rAthena bare `var`). Persists across logins. */
    perm: Record<string, number | string>;
    /** Per-account local (rAthena `#var`). */
    account: Record<string, number | string>;
    /** Per-account global (rAthena `##var`). Shared across login servers. */
    accountGlobal: Record<string, number | string>;

    // ========== Sub-surfaces ===============================================
    quest: QuestOps;
    achievement: AchievementOps;
    storage: StorageOps;
    cart: CartOps;
    mail: MailOps;
    pet: PetOps;
    hom: HomOps;
    merc: MercOps;

    // ========== Heal / SP / AP =============================================

    /**
     * Restore HP and (optionally) SP. Bypasses the no-heal mapflag.
     *
     * Mirrors rAthena's `heal`. NOT YET WIRED — stub.
     *
     * @param hp Amount of HP to restore. Negative damages.
     * @param sp Amount of SP to restore.
     *
     * @example
     * ctx.player.heal(500, 200);
     */
    heal(hp: number, sp?: number): void;

    /** Restore activity points (4th-job). NOT YET WIRED — stub. */
    healAp(ap: number): void;

    /**
     * Heal as if from an item — respects no-heal mapflags and uses
     * itemheal_rate / vit_def_rate. NOT YET WIRED — stub.
     *
     * Mirrors rAthena's `itemheal`.
     */
    itemHeal(hp: number, sp: number): void;

    /**
     * Heal by percentage of max. NOT YET WIRED — stub.
     *
     * Mirrors rAthena's `percentheal`.
     *
     * @example
     * ctx.player.percentHeal(100, 100);  // full HP + SP
     */
    percentHeal(hpPercent: number, spPercent: number): void;

    /**
     * Composite "revive / restore" op — same parameter shape as
     * rAthena's `recovery`. NOT YET WIRED — stub.
     *
     * @param type Recovery target type (0 = self, 1 = party, 2 = guild, 3 = map, …).
     */
    recovery(type: number, opts?: { option?: number; reviveFlag?: number; mapName?: string }): void;

    // ========== Experience / level =========================================

    /**
     * Award base / job experience.
     *
     * Mirrors rAthena's `getexp` (+ `getexp2` when both are negative).
     * NOT YET WIRED — stub.
     *
     * @param baseExp Base XP to award.
     * @param jobExp Job XP to award.
     * @param opts If `quest: true`, treats as quest XP (different multipliers).
     *
     * @example
     * ctx.player.giveExp(50_000, 5_000, { quest: true });
     */
    giveExp(baseExp: number, jobExp: number, opts?: { quest?: boolean }): void;

    /** % of the current level's base XP per unit. NOT YET WIRED — stub. */
    baseExpRatio(percent: number, level?: number): number;
    /** Same as `baseExpRatio` but for job XP. NOT YET WIRED — stub. */
    jobExpRatio(percent: number, level?: number): number;

    // ========== Job / class ================================================

    /**
     * Change the player's job/class.
     *
     * Mirrors rAthena's `jobchange`. NOT YET WIRED — stub.
     */
    jobChange(jobId: number, opts?: { upper?: number }): void;
    changeBase(classId: number): void;
    changeSex(): void;
    /** Human-readable name of a job id. NOT YET WIRED — stub. */
    jobName(jobId: number): string;

    // ========== Movement ===================================================

    /**
     * Teleport the player to a specific map+tile.
     *
     * Mirrors rAthena's `warp`. NOT YET WIRED — stub.
     *
     * @example
     * ctx.player.warp("prontera", 156, 191);
     * ctx.player.warp("Random", 0, 0);   // random on current map
     * ctx.player.warp("SavePoint", 0, 0); // back to save point
     */
    warp(map: string, x: number, y: number): void;

    /**
     * Set the player's respawn point.
     *
     * Mirrors rAthena's `savepoint`. NOT YET WIRED — stub.
     */
    savePoint(map: string, x: number, y: number, rangeX?: number, rangeY?: number): void;
    /** Alias of {@link savePoint}. NOT YET WIRED — stub. */
    save(map: string, x: number, y: number): void;
    /** Read the player's save point. NOT YET WIRED — stub. */
    getSavePoint(): { map: string; x: number; y: number } | null;
    /** Push the player N cells in a direction. NOT YET WIRED — stub. */
    pushPc(direction: number, cells: number): void;
    /** Warp the player's spouse to the same destination. NOT YET WIRED — stub. */
    warpPartner(map: string, x: number, y: number): void;

    // ========== Items: give / take / count =================================

    /**
     * Give an item to the player.
     *
     * Mirrors rAthena's `getitem`. NOT YET WIRED — stub.
     *
     * @param itemId rAthena `item_db.id`.
     * @param amount Quantity (default 1).
     * @param opts Refine / cards / bound / random options.
     *
     * @example
     * ctx.player.giveItem(501, 5);                       // 5 Red Potions
     * ctx.player.giveItem(1201, 1, { refine: 7 });       // +7 Knife
     * ctx.player.giveItem(1124, 1, { cards: [4001, 4001, 4001, 4001] });
     */
    giveItem(itemId: number, amount?: number, opts?: ItemOpts): void;
    /** Give a time-limited rental item. NOT YET WIRED — stub. */
    giveRentItem(itemId: number, seconds: number, opts?: ItemOpts): void;
    /** Give an item with a custom inscription. NOT YET WIRED — stub. */
    giveNamedItem(itemId: number, inscribeName: string): void;
    /** Draw a random item from a group. NOT YET WIRED — stub. */
    giveRandomGroupItem(groupId: number, qty?: number, opts?: { subGroup?: number; identify?: boolean }): void;
    /** Give every item in a group. NOT YET WIRED — stub. */
    giveGroupItem(groupId: number, opts?: { identify?: boolean }): void;

    /**
     * Remove items from the player's inventory.
     *
     * Mirrors rAthena's `delitem`. NOT YET WIRED — stub.
     *
     * @example
     * ctx.player.delItem(501, 1);  // consume one Red Potion
     */
    delItem(itemId: number, amount?: number, opts?: ItemOpts): void;
    /** Remove items by inventory index. NOT YET WIRED — stub. */
    delItemAtIndex(index: number, amount?: number): void;

    /**
     * Count items in the player's inventory.
     *
     * Mirrors rAthena's `countitem`. NOT YET WIRED — stub.
     *
     * @example
     * if (ctx.player.countItem(501) >= 5) {
     *     ctx.mes("You have at least 5 Red Potions.");
     * }
     */
    countItem(itemId: number, opts?: ItemOpts): number;
    /** Count account-/character-/guild-bound items. NOT YET WIRED — stub. */
    countBound(boundType?: number): number;
    /** True if the player has at least `amount` of the item. NOT YET WIRED — stub. */
    hasItem(itemId: number, amount?: number, opts?: ItemOpts): boolean;
    /** Wipe the player's entire inventory. NOT YET WIRED — stub. */
    clearItems(): void;
    /** Consume a single item (decrement by 1). NOT YET WIRED — stub. */
    consumeItem(itemId: number): void;
    /** Item ids matching the name substring. NOT YET WIRED — stub. */
    searchItem(namePart: string): number[];
    /** Snapshot of the entire inventory. NOT YET WIRED — stub. */
    getInventory(): InventoryEntry[];
    /** Re-stack mergeable items. NOT YET WIRED — stub. */
    mergeItems(itemId?: number): void;
    /** Identify all unidentified items (or those of a specific type). NOT YET WIRED — stub. */
    identifyAll(type?: number): void;
    /** Predicate: can the player carry N more of this item? NOT YET WIRED — stub. */
    checkWeight(itemId: number, amount: number, more?: Array<{ itemId: number; amount: number }>): boolean;

    // ========== Equipment ==================================================

    /** Item id in equip slot. NOT YET WIRED — stub. */
    getEquipId(slot: number): number;
    /** Item name in equip slot. NOT YET WIRED — stub. */
    getEquipName(slot: number): string;
    /** Inventory unique id of the equipped item. NOT YET WIRED — stub. */
    getEquipUniqueId(slot: number): number;
    /** Refine level of the equipped item. NOT YET WIRED — stub. */
    getEquipRefine(slot: number): number;
    getEquipWeaponLv(slot?: number): number;
    getEquipArmorLv(slot?: number): number;
    /** How many cards are slotted. NOT YET WIRED — stub. */
    getEquipCardCount(slot: number): number;
    /** Card id in a specific card slot. NOT YET WIRED — stub. */
    getEquipCardId(slot: number, cardSlot: number): number;
    /** Enchant grade for the slot. NOT YET WIRED — stub. */
    getEnchantGrade(slot?: number): number;
    /** True if anything is equipped in the slot. NOT YET WIRED — stub. */
    isEquipped(slot: number): boolean;
    /** True if the slot's item is refinable. NOT YET WIRED — stub. */
    isEquipEnableRef(slot: number): boolean;
    /** Inventory index of the slot's item. NOT YET WIRED — stub. */
    getItemPos(slot: number): number;

    /**
     * Equip a specific item. NOT YET WIRED — stub.
     * Mirrors rAthena's `equip`.
     */
    equip(itemId: number): void;
    /** Auto-equip an item on level-up. NOT YET WIRED — stub. */
    autoEquip(itemId: number, enable: boolean): void;
    /** Unequip the item in the slot. NOT YET WIRED — stub. */
    unequip(slot: number): void;
    /** Delete the item in the slot. NOT YET WIRED — stub. */
    delEquip(slot: number): void;
    /** Break the item in the slot. NOT YET WIRED — stub. */
    breakEquip(slot: number): void;
    /** Refine up the slot's item. NOT YET WIRED — stub. */
    successRefine(slot: number, count?: number): void;
    /** Refine fail (item breaks at the refine level). NOT YET WIRED — stub. */
    failRefine(slot: number): void;
    /** Refine down. NOT YET WIRED — stub. */
    downRefine(slot: number, count?: number): void;
    /** Repair a specific broken item. NOT YET WIRED — stub. */
    repair(brokenIndex: number): void;
    /** Repair every broken item. NOT YET WIRED — stub. */
    repairAll(): void;
    /** Pop cards out of a slot. NOT YET WIRED — stub. */
    removeCards(slot: number, success: boolean, type?: number): void;
    /** Inventory id of a broken item. NOT YET WIRED — stub. */
    getBrokenId(number: number): number;

    // ========== Skills =====================================================

    /**
     * Skill level the player has (0 if not learned).
     *
     * Mirrors rAthena's `getskilllv`. NOT YET WIRED — stub.
     */
    skillLv(skillId: number): number;
    /** Grant / set a skill's level. NOT YET WIRED — stub. */
    addSkill(skillId: number, level: number, opts?: { permanent?: boolean }): void;
    /** Use a skill as if cast from an item (no SP cost). NOT YET WIRED — stub. */
    itemSkill(skillId: number, level: number, keepRequirement?: boolean): void;
    getSkillList(): SkillEntry[];
    skillPointCount(): number;
    /** Has Basic Skill (chat, party, trade, …). NOT YET WIRED — stub. */
    basicSkillCheck(): boolean;

    // ========== Looks / mounts =============================================

    /**
     * Change a "look" param (hair, clothes, weapon view, …).
     *
     * Mirrors rAthena's `setlook` / `changelook`. NOT YET WIRED — stub.
     */
    setLook(type: number, value: number): void;
    changeLook(type: number, value: number): void;
    getLook(type: number): number;
    setFont(font: number): void;
    /** Attach / detach a cart (and pick its sprite). NOT YET WIRED — stub. */
    setCart(type?: number): void;
    setFalcon(flag?: boolean): void;
    setRiding(flag?: boolean): void;
    setDragon(color?: number): void;
    setMadogear(flag?: boolean, type?: number): void;
    setMounting(): void;
    checkCart(): boolean;
    checkFalcon(): boolean;
    checkRiding(): boolean;
    checkDragon(): boolean;
    checkMadogear(): boolean;
    checkWug(): boolean;
    isMounting(): boolean;

    // ========== Options / status ===========================================

    setOption(option: number, flag?: boolean): void;
    checkOption(option: number): boolean;
    checkOption1(option: number): boolean;
    checkOption2(option: number): boolean;
    /**
     * Apply a status effect. NOT YET WIRED — stub.
     *
     * Mirrors rAthena's `sc_start` family.
     *
     * @param type SC_* constant from script_constants.
     * @param durationMs Effect duration.
     */
    scStart(type: number, durationMs: number, opts?: { val1?: number; val2?: number; val3?: number; val4?: number }): void;
    /** End a status effect (or all if `type` omitted). NOT YET WIRED — stub. */
    scEnd(type?: number): void;
    /** Query a SC's runtime info. NOT YET WIRED — stub. */
    getStatus(effectType: number, infoType?: number): number;
    /** True if the player is dead. NOT YET WIRED — stub. */
    isDead(): boolean;
    /** Force a stat recalc. NOT YET WIRED — stub. */
    recalculateStat(): void;
    /** Status points required to raise a stat to `value`. NOT YET WIRED — stub. */
    needStatusPoint(statType: number, value: number): number;

    // ========== Reset ======================================================

    /** Reset all stat allocations and refund points. NOT YET WIRED — stub. */
    resetStatus(): void;
    /** Reset all skills and refund points. NOT YET WIRED — stub. */
    resetSkill(): void;
    /** Reset the Star Gladiator feel-the-stars. NOT YET WIRED — stub. */
    resetFeel(): void;
    /** Reset the Star Gladiator hate-the-mob. NOT YET WIRED — stub. */
    resetHate(): void;

    // ========== Display effects ============================================

    /**
     * Show a server-side text message in the player's chat box.
     *
     * Mirrors rAthena's `message`. NOT YET WIRED — stub.
     */
    message(text: string): void;
    /** Bottom-of-screen message with optional color. NOT YET WIRED — stub. */
    dispBottom(text: string, color?: number): void;
    /** Show a quest-style "script" tooltip. NOT YET WIRED — stub. */
    showScript(text: string, flag?: number): void;
    /** Display a cutin BMP. NOT YET WIRED — stub. */
    cutin(filename: string, position: number): void;
    /** Play an emotion (sweat drop, exclamation, …). NOT YET WIRED — stub. */
    emotion(emoNum: number, target?: number): void;
    /** Play a misc effect by id. NOT YET WIRED — stub. */
    miscEffect(effectNum: number): void;
    /** Play a sound effect file. NOT YET WIRED — stub. */
    soundEffect(filename: string, type?: number): void;
    /** Start playing a BGM file. NOT YET WIRED — stub. */
    playBgm(filename: string): void;
    /** Display a map viewpoint marker. NOT YET WIRED — stub. */
    viewpoint(action: number, x: number, y: number, point: number, color: number): void;
    /** Show a digit-roll overlay (lottery / quest). NOT YET WIRED — stub. */
    showDigit(value: number, type?: number): void;
    /** Toggle a hat effect. NOT YET WIRED — stub. */
    hatEffect(hatEffectId: number, state: boolean): void;

    // ========== UI windows =================================================

    /** Open the storage window (mode = 0..N for kafra type). NOT YET WIRED — stub. */
    openStorage(mode?: number): void;
    openBank(): void;
    openMail(): void;
    openAuction(): void;
    openRefineUi(): void;
    openStylist(): void;
    openDressRoom(): void;
    openRoulette(): void;
    openQuestUi(questId?: number): void;
    openEnchantGrade(): void;
    openLaphineSynthesis(itemId?: number): void;
    openLaphineUpgrade(): void;
    openItemEnchant(luaIndex: number): void;
    openItemReform(itemId?: number): void;
    /** Special-popup macro hint. NOT YET WIRED — stub. */
    specialPopup(popupId: number): void;
    /** Show a tip pop-up. NOT YET WIRED — stub. */
    openTips(tipId: number): void;
    /** Open a book item. NOT YET WIRED — stub. */
    readBook(bookId: number, page?: number): void;

    // ========== Spirit balls ===============================================

    addSpiritBall(count: number, durationMs: number): void;
    delSpiritBall(count: number): void;
    countSpiritBall(): number;

    // ========== Reputation / fame ==========================================

    getReputation(type: number): number;
    setReputation(type: number, points: number): void;
    addReputation(type: number, points: number): void;
    /** Total fame score (sum across categories). NOT YET WIRED — stub. */
    getFame(): number;
    addFame(amount: number): void;
    /** Rank position in the global fame list. NOT YET WIRED — stub. */
    getFameRank(): number;

    // ========== Marriage / family ==========================================

    /** Marry the named partner. NOT YET WIRED — stub. */
    marry(spouseName: string): void;
    divorce(): void;
    /** Adopt — `parentName` must be married. NOT YET WIRED — stub. */
    adopt(parentName: string, babyName: string): void;
    getPartnerId(): number;
    getMotherId(): number;
    getFatherId(): number;
    getChildId(): number;
    /** True if the partner is currently online. NOT YET WIRED — stub. */
    isPartnerOn(): boolean;

    // ========== Permissions ================================================

    /** True if the player holds the named permission. NOT YET WIRED — stub. */
    permissionCheck(permission: string): boolean;
    permissionAdd(permission: string): void;
    permissionRemove(permission: string): void;
    guildHasPermission(permission: string): boolean;

    // ========== VIP / macro ================================================

    vipStatus(type: number): number;
    /** Award VIP time (in seconds). NOT YET WIRED — stub. */
    vipTime(seconds: number): void;
    /** Force-trigger the anti-bot macro detector. NOT YET WIRED — stub. */
    macroDetector(): void;

    // ========== Misc =======================================================

    /** String info about the character (name, party name, guild, map, …). NOT YET WIRED — stub. */
    charInfo(type: number): string;
    /** Read an SP_* param. NOT YET WIRED — stub. */
    readParam(paramNumber: number): number;
    charId4Type(type: number): number;
    charIp(): string;
    /** Kick the player from the server. NOT YET WIRED — stub. */
    kick(): void;
    ignoreTimeout(flag: boolean): void;
    autoLoot(rate?: number): number;
    hasAutoLoot(): boolean;
    /** True if the player's job can enter the map. NOT YET WIRED — stub. */
    jobCanEnterMap(map: string, jobId?: number): boolean;
    checkVending(): boolean;
    checkChatting(): boolean;
    checkIdle(): boolean;
    /** Show a navigation arrow to a map+tile. NOT YET WIRED — stub. */
    navigateTo(map: string, x?: number, y?: number, flag?: number, hideWindow?: boolean, monsterId?: number): void;
    clanJoin(clanId: number): void;
    clanLeave(): void;
    /** Read camera info (range / rotation / latitude). NOT YET WIRED — stub. */
    cameraInfo(range: number, rotation: number, latitude: number): unknown;
}

// ============================================================================
// Companion types
// ============================================================================

/**
 * Common options for item ops (give / take / count / makeitem). Every
 * field is optional; omitted fields match any value (for count/has) or
 * use rAthena defaults (for give).
 */
export interface ItemOpts {
    identify?: boolean;
    refine?: number;
    /** Special attribute (broken flag etc.). */
    attribute?: number;
    /** 4-slot card tuple. */
    cards?: [number, number, number, number];
    /** rAthena bound type (1 = account, 2 = guild, 3 = party, 4 = char). */
    bound?: number;
    grade?: number;
    randomOptions?: Array<{ id: number; value: number; param: number }>;
}

export interface InventoryEntry {
    /** Inventory slot index. */
    index: number;
    itemId: number;
    amount: number;
    identified: boolean;
    refine: number;
    cards: [number, number, number, number];
    bound: number;
    grade: number;
    /** Rental expiry tick. Unset for non-rental items. */
    expireTime?: number;
}

export interface SkillEntry {
    id: number;
    level: number;
    flag: number;
}
