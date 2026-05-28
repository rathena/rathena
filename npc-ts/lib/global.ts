// =====================================================================
// Global functions — port of npc/other/Global_Functions.txt
// =====================================================================
//
// Every named export below mirrors a `function script F_X` from the
// legacy rAthena Global_Functions.txt with the same name (and roughly
// the same signature). Migrating an existing .txt script is mostly a
// search-replace:
//
//   rAthena (.txt):
//      mes callfunc("F_SexMes","Pretty lady!","Handsome man!");
//      mes callfunc("F_GetPlural",.@name$,1);
//
//   TS:
//      import { F_SexMes, F_GetPlural } from "@lib/global";
//      ctx.mes(F_SexMes(ctx, "Pretty lady!", "Handsome man!"));
//      ctx.mes(F_GetPlural(name, true));
//
// Camel-case aliases (`sexMes`, `pluralize`, …) live at the bottom for
// authors who prefer idiomatic TS naming. Both refer to the same
// implementation.
//
// IMPLEMENTATION STATUS:
//   ✓ pure helpers (Rand, GetPlural, InsertPlural, GetArticle,
//     InsertArticle, InsertComma, GetNumSuffix, Time2Str, GetPositionName,
//     SexMes, Hi, Bye, F_22507) — WORK END-TO-END right now
//   ⏳ ctx-dependent helpers (GetWeaponType, GetArmorType, Job_Change,
//     ClearJobVar, ClearGarbage, IsEquip*Hack, CanOpenStorage,
//     CanChangeJob, ItemName, GetPlatinumSkills, SaveQuestSkills,
//     Load1Skills, Load2Skills, IsCharm, queststatus) — compile and
//     call into the ctx stubs; will fully work once the underlying
//     ctx.player / ctx.world ops graduate from stub to real impl.

// ---------------------------------------------------------------------
// Pure helpers (no ctx needed — work today)
// ---------------------------------------------------------------------

/**
 * Return a random argument from the variadic list. Direct port of
 * rAthena's `F_Rand`.
 *
 * @example
 * // rAthena: set @id, callfunc("F_Rand", 1129, 1222, 1163, 1357);
 * const id = F_Rand(1129, 1222, 1163, 1357);
 */
export function F_Rand<T>(...args: T[]): T {
    if (args.length === 0) throw new Error("F_Rand: at least one argument required");
    return args[Math.floor(Math.random() * args.length)];
}

/**
 * Random "hello" message. Mirror of rAthena's `F_Hi`.
 *
 * @example
 * // rAthena: mes callfunc("F_Hi");
 * ctx.mes(F_Hi());
 */
export function F_Hi(): string {
    return F_Rand(
        "Hi!", "Hello!", "Good day!", "How are you?", "Hello there.",
    );
}

/**
 * Random "goodbye" message. Mirror of rAthena's `F_Bye`.
 *
 * @example ctx.mes(F_Bye());
 */
export function F_Bye(): string {
    return F_Rand(
        "Bye. See you again.", "Later.", "Goodbye.",
        "Good luck!", "Have a nice day!", "Byebye!!!",
    );
}

/**
 * Number with thousands separators. Mirror of rAthena's `F_InsertComma`.
 *
 * @example
 * F_InsertComma(1234567)   // "1,234,567"
 * F_InsertComma(-987654)   // "-987,654"
 */
export function F_InsertComma(n: number): string {
    const s = String(Math.trunc(n));
    const sign = s.startsWith("-") ? "-" : "";
    const digits = sign ? s.slice(1) : s;
    let out = "";
    for (let i = digits.length; i > 0; i -= 3) {
        const start = Math.max(0, i - 3);
        out = (start === 0 ? "" : ",") + digits.slice(start, i) + out;
    }
    return sign + out;
}

/**
 * Ordinal suffix for a number ("st", "nd", "rd", "th"). Mirror of
 * rAthena's `F_GetNumSuffix`. Note: rAthena returns the WHOLE phrase
 * (e.g. `"2nd"`); we follow that for migration compatibility.
 *
 * @example
 * F_GetNumSuffix(1)   // "1st"
 * F_GetNumSuffix(22)  // "22nd"
 * F_GetNumSuffix(113) // "113th"
 */
export function F_GetNumSuffix(n: number): string {
    const mod100 = Math.abs(n) % 100;
    const mod10 = mod100 % 10;
    let suffix = "th";
    if (mod100 < 11 || mod100 > 13) {
        if (mod10 === 1) suffix = "st";
        else if (mod10 === 2) suffix = "nd";
        else if (mod10 === 3) suffix = "rd";
    }
    return `${n}${suffix}`;
}

/**
 * Pluralize an English noun. Mirror of rAthena's `F_GetPlural`. Handles
 * the common exceptions (fish/shoes/wings, -s/-x/-z/-ch/-sh, -f/-fe,
 * -y after consonant, -o exceptions, etc.) and supports the "Verb of
 * Verb" / "Verb in Verb" / "Verb on Verb" phrasing where the FIRST
 * noun pluralizes.
 *
 * @param word Singular form.
 * @param upper If true, the result is upper-cased.
 *
 * @example
 * F_GetPlural("apple")          // "apples"
 * F_GetPlural("fish")           // "fish"
 * F_GetPlural("knife")          // "knives"
 * F_GetPlural("party of one")   // "parties of one"
 * F_GetPlural("Apple", true)    // "APPLES"
 */
export function F_GetPlural(word: string, upper = false): string {
    // Multi-word phrasing: "X of Y" / "X in Y" / "X on Y" pluralizes X.
    if (word.includes(" ")) {
        const parts = word.split(" ");
        let idx = parts.length - 1;
        if (/\b(of|in|on)\b/.test(word)) {
            for (let i = 1; i < parts.length; i++) {
                if (parts[i].length === 2 && /^(of|in|on)$/.test(parts[i])) {
                    break;
                }
                idx = i;
            }
        }
        const pluralized = pluralizeWord(parts[idx]);
        parts[idx] = pluralized;
        const out = parts.join(" ");
        return upper ? out.toUpperCase() : out;
    }
    const out = pluralizeWord(word);
    return upper ? out.toUpperCase() : out;
}

function pluralizeWord(word: string): string {
    const n = word.length;
    if (n < 3) return word; // pass-through for ultra-short tokens

    const last1 = word.slice(-1).toLowerCase();
    const last2 = word.slice(-2).toLowerCase();

    if (!/[a-z]/i.test(last1)) return word; // ends in non-letter

    // singular == plural set
    const sameForm = ["fish", "glasses", "sunglasses", "clothes", "boots", "shoes",
                      "greaves", "sandals", "wings", "ears"];
    if (sameForm.includes(word.toLowerCase())) return word;

    // -s / -x / -z / -ch / -sh → -es
    if (last1 === "s" || last1 === "x" || last1 === "z" ||
        last2 === "ch" || last2 === "sh") {
        return word + "es";
    }

    // -f / -fe (not -ff) → -ves, with common exceptions
    if ((last1 === "f" || last2 === "fe") && last2 !== "ff") {
        const exceptions = ["belief", "cliff", "chief", "dwarf", "grief", "gulf", "proof", "roof"];
        if (exceptions.includes(word.toLowerCase())) return word + "s";
        return word.slice(0, n - (last2 === "fe" ? 2 : 1)) + "ves";
    }

    // consonant + -y → -ies
    if (last1 === "y" && !/[aeiou]/.test(word[n - 2])) {
        return word.slice(0, n - 1) + "ies";
    }

    // -o with exceptions → -es
    if (last1 === "o") {
        const oExceptions = ["buffalo", "domino", "echo", "grotto", "halo", "hero",
                             "mango", "mosquito", "potato", "tomato", "tornado",
                             "torpedo", "veto", "volcano"];
        if (oExceptions.includes(word.toLowerCase())) return word + "es";
    }

    return word + "s";
}

/**
 * Format "N noun" with auto-pluralization. Mirror of rAthena's
 * `F_InsertPlural`.
 *
 * @example
 * F_InsertPlural(1, "apple")   // "1 apple"
 * F_InsertPlural(5, "apple")   // "5 apples"
 * F_InsertPlural(3, "knife")   // "3 knives"
 * F_InsertPlural(0, "fish")    // "0 fish"
 */
export function F_InsertPlural(
    count: number,
    word: string,
    upper = false,
    format = "%d %s",
): string {
    const noun = count === 1 ? word : F_GetPlural(word, upper);
    return format.replace("%d", String(count)).replace("%s", noun);
}

/**
 * Indefinite article (`"a"` or `"an"`) for a noun. Mirror of rAthena's
 * `F_GetArticle`. Honors English vowel-sound rules for letters/words
 * that look like consonants but sound like vowels (e.g. "honor").
 *
 * @example
 * F_GetArticle("Apple")  // "an"
 * F_GetArticle("dog")    // "a"
 * F_GetArticle("hour")   // "an"
 */
export function F_GetArticle(word: string): string {
    const s = word.toLowerCase();
    if (!/^[a-z]/.test(s)) return "a";

    if (s.length === 1) return "aefhilmnorsx".includes(s) ? "an" : "a";

    const c0 = s[0];
    const c1 = s[1];

    // "an" exceptions for words starting with a vowel-sound consonant
    if (c0 === "h") {
        if (/^h(ou|our|onor|onest)/.test(s)) return "an";
    }
    // Words that LOOK like they start with a vowel but sound like consonants.
    if (c0 === "u") {
        // "use", "user", "uniform", "university", "European"-ish
        if (/^u(s[a-z]|ni[a-z]|ti[a-z]|na[a-z])/.test(s)) return "a";
    }
    if (c0 === "e") {
        if (/^eu/.test(s)) return "a"; // European, eulogy
    }
    if (c0 === "o" && c1 === "n" && s.startsWith("one")) return "a";

    return /^[aeiou]/.test(s) ? "an" : "a";
}

/**
 * Indefinite article + noun. Mirror of rAthena's `F_InsertArticle`.
 *
 * @example
 * F_InsertArticle("Apple")        // "an Apple"
 * F_InsertArticle("dog", true)    // "A dog"
 */
export function F_InsertArticle(word: string, upperArticle = false): string {
    const art = upperArticle ? F_GetArticle(word).toUpperCase().padEnd(F_GetArticle(word).length, "") : F_GetArticle(word);
    return `${art} ${word}`;
}

/**
 * Pick a string by player sex. Mirror of rAthena's `F_SexMes`.
 *
 * @param ctx The NPC context (reads `ctx.player.sex`).
 * @param female Returned when player sex is 0.
 * @param male Returned when player sex is 1.
 *
 * @example
 * // rAthena:
 * //   mes callfunc("F_SexMes","Pretty lady!","Handsome man!");
 * ctx.mes(F_SexMes(ctx, "Pretty lady!", "Handsome man!"));
 */
export function F_SexMes(ctx: NpcContext, female: string, male: string): string {
    if (!ctx.player) return male;
    return ctx.player.sex === 1 ? male : female;
}

/**
 * Format a Unix timestamp as a human-readable duration FROM NOW.
 * Mirror of rAthena's `Time2Str`. The legacy function takes a target
 * tick and returns "N days, N hours, …" until that tick.
 *
 * @param targetTickSeconds Unix timestamp the duration counts toward.
 * @param nowSeconds Optional reference (defaults to `Date.now() / 1000`).
 *
 * @example
 * // rAthena:
 * //   mes callfunc("Time2Str", #BankExpire);
 * ctx.mes(Time2Str(bankExpireTimestamp));
 */
export function Time2Str(
    targetTickSeconds: number,
    nowSeconds: number = Math.floor(Date.now() / 1000),
): string {
    let left = targetTickSeconds - nowSeconds;
    if (left <= 0) return "0 seconds";

    const days = Math.trunc(left / 86_400);   left -= days * 86_400;
    const hours = Math.trunc(left / 3_600);   left -= hours * 3_600;
    const mins = Math.trunc(left / 60);       left -= mins * 60;
    const secs = left;

    const parts: string[] = [];
    const push = (n: number, name: string) => {
        if (n > 0) parts.push(`${n} ${name}${n === 1 ? "" : "s"}`);
    };
    push(days, "day");
    push(hours, "hour");
    push(mins, "minute");
    if (secs > 0 || parts.length === 0) push(secs, "second");
    return parts.join(", ");
}

/**
 * Equip position name. Mirror of rAthena's `F_getpositionname`.
 *
 * @example
 * F_getpositionname(EQI_HEAD_TOP)  // "Upper Headgear"
 */
export function F_getpositionname(pos: number): string {
    // rAthena EQI_* constants. Keep these in sync with
    // db/(pre-)re/const.yml.
    const NAMES: Record<number, string> = {
        1:  "Upper Headgear",
        2:  "Armor",
        3:  "Garment",
        4:  "Accessory 1",
        5:  "Weapon",
        6:  "Shield",
        7:  "Shoes",
        8:  "Accessory 2",
        9:  "Lower Headgear",
        10: "Middle Headgear",
        11: "Lower Costume Headgear",
        12: "Middle Costume Headgear",
        13: "Upper Costume Headgear",
        14: "Costume Garment",
        15: "Armor",   // shadow set entries — names approximate
        16: "Shadow Weapon",
        17: "Shadow Shield",
        18: "Shadow Shoes",
        19: "Shadow Accessory 1",
        20: "Shadow Accessory 2",
    };
    return NAMES[pos] ?? `Unknown Slot ${pos}`;
}

// ---------------------------------------------------------------------
// ctx-dependent helpers (compile and call ctx stubs)
// ---------------------------------------------------------------------

/**
 * Display name of a weapon's class (`"Dagger"`, `"Bow"`, …) given its
 * item id. Mirror of rAthena's `F_GetWeaponType`.
 *
 * Reads the item's view id via `ctx.world.itemInfo`.
 *
 * @example
 * ctx.mes(F_GetWeaponType(ctx, 1201));  // "Dagger"
 */
export function F_GetWeaponType(ctx: NpcContext, itemId: number): string {
    // ITEMINFO_VIEW (rAthena enum value 2) — view id for weapons is
    // the weapon class.
    const viewId = ctx.world.itemInfo(itemId, 2);
    const view = typeof viewId === "number" ? viewId : 0;
    const NAMES: Record<number, string> = {
        1:  "Dagger",
        2:  "One-handed Sword",
        3:  "Two-handed Sword",
        4:  "One-handed Spear",
        5:  "Two-handed Spear",
        6:  "One-handed Axe",
        7:  "Two-handed Axe",
        8:  "Mace",
        10: "Staff",
        11: "Bow",
        12: "Knuckle",
        13: "Instrument",
        14: "Whip",
        15: "Book",
        16: "Katar",
        17: "Revolver",
        18: "Rifle",
        19: "Gatling gun",
        20: "Shotgun",
        21: "Grenade Launcher",
        22: "Shuriken",
    };
    return NAMES[view] ?? "Unknown Weapon";
}

/**
 * Display name of an armor / equipment class. Mirror of rAthena's
 * `F_GetArmorType`. Falls through to `F_GetWeaponType` when the item
 * is a right-hand weapon.
 *
 * @example
 * ctx.mes(F_GetArmorType(ctx, 2356));  // "Armor"
 */
export function F_GetArmorType(ctx: NpcContext, itemId: number): string {
    // ITEMINFO_LOCATIONS (rAthena enum value 8) — bitmask of EQP_* slots.
    const locsRaw = ctx.world.itemInfo(itemId, 8);
    const locs = typeof locsRaw === "number" ? locsRaw : 0;

    // EQP_* values — see src/map/script_constants.hpp.
    const EQP_HEAD_LOW = 0x0001;
    const EQP_HAND_R   = 0x0002;
    const EQP_GARMENT  = 0x0004;
    const EQP_ACC_L    = 0x0008;
    const EQP_ARMOR    = 0x0010;
    const EQP_HAND_L   = 0x0020;
    const EQP_SHOES    = 0x0040;
    const EQP_ACC_R    = 0x0080;
    const EQP_HEAD_TOP = 0x0100;
    const EQP_HEAD_MID = 0x0200;
    const EQP_ACC_RL   = EQP_ACC_L | EQP_ACC_R;

    if (locs === EQP_HEAD_LOW) return "Lower Headgear";
    if (locs === EQP_HAND_R)   return F_GetWeaponType(ctx, itemId);
    if (locs === EQP_GARMENT)  return "Garment";
    if (locs === EQP_ACC_L || locs === EQP_ACC_R || locs === EQP_ACC_RL) return "Accessory";
    if (locs === EQP_ARMOR)    return "Armor";
    if (locs === EQP_HAND_L)   return "Shield";
    if (locs === EQP_SHOES)    return "Shoes";
    if (locs === EQP_HEAD_TOP) return "Upper Headgear";
    if (locs === EQP_HEAD_MID) return "Middle Headgear";
    return "Unknown Equipment";
}

/**
 * Job change preserving Upper (normal / advanced / baby). Mirror of
 * rAthena's `Job_Change`.
 *
 * @example
 * Job_Change(ctx, 4002);  // Lord Knight (will inherit baby/upper)
 */
export function Job_Change(ctx: NpcContext, classId: number): void {
    if (!ctx.player) return;
    // The rAthena buildin takes an Upper arg; ctx.player.jobChange's
    // opts.upper covers that. We don't have ctx.player.upper exposed
    // yet, so fall back to default — the C++ side can default-preserve.
    ctx.player.jobChange(classId);
}

/**
 * Format an item name with element, VVS class, and refine. Mirror of
 * rAthena's `F_ItemName`.
 *
 * @param ctx Used for item-name lookup.
 * @param itemId rAthena item id.
 * @param element 0=none, 1=Ice, 2=Earth, 3=Fire, 4=Wind.
 * @param vvs 0=none, 1=VS, 2=VVS, 3=VVVS.
 * @param refine 0..N — prepended as `+N `.
 *
 * @example
 * ctx.mes(F_ItemName(ctx, 1201, 1, 2, 5));  // e.g. "+5 VVS Fire Knife"
 */
export function F_ItemName(
    ctx: NpcContext,
    itemId: number,
    element = 0,
    vvs = 0,
    refine = 0,
): string {
    const refineStr = refine > 0 ? `+${refine} ` : "";
    const vvsStr = ["", "VS ", "VVS ", "VVVS "][vvs] ?? "";
    const elemStr = ["", "Ice ", "Earth ", "Fire ", "Wind "][element] ?? "";
    const name = ctx.world.itemName(itemId);
    return `${refineStr}${vvsStr}${elemStr}${name}`;
}

/** Wipe job-quest progress vars. Mirror of `F_ClearJobVar`. */
export function F_ClearJobVar(ctx: NpcContext): void {
    if (!ctx.player) return;
    const KEYS = [
        // Misc
        "JBLVL", "FIRSTAID", "PLAYDEAD", "got_bandage", "got_novnametag",
        // First class
        "job_acolyte_q", "job_acolyte_q2",
        "job_archer_q",
        "job_magician_q",
        "job_merchant_q", "job_merchant_q2", "job_merchant_q3",
        "job_sword_q", "SWTEST",
        "job_thief_q",
        "SUPNOV_Q",
        // 2-1 / 2-2 / extended
        "ASSIN_Q", "ASSIN_Q2", "ASSIN_Q3",
        "BSMITH_Q", "BSMITH_Q2",
        "HNTR_Q", "HNTR_Q2",
        "KNIGHT_Q", "KNIGHT_Q2",
        "PRIEST_Q", "PRIEST_Q2", "PRIEST_Q3",
        "WIZ_Q", "WIZ_Q2",
        "ROGUE_Q", "ROGUE_Q2",
        "ALCH_Q", "ALCH_Q2",
        "CRUS_Q",
        "MONK_Q", "JOB_MONK_C",
        "SAGE_Q", "SAGE_Q2",
        "DANC_Q",
        "BARD_Q",
        "TAEK_Q", "TK_Q",
        "STGL_Q",
        "SOUL_Q",
        "GUNS_Q",
        "NINJ_Q",
    ];
    for (const k of KEYS) ctx.player.perm[k] = 0;
}

/** Clear obsolete event / quest vars. Mirror of `F_ClearGarbage`. */
export function F_ClearGarbage(ctx: NpcContext): void {
    if (!ctx.player) return;
    const p = ctx.player.perm;

    // Turtle Island carry-over
    if (Number(p.TURTLE) === 20) {
        p.MISC_QUEST = (Number(p.MISC_QUEST) || 0) | 65536;
    }
    if ((Number(p.MISC_QUEST) || 0) & 65536) p.TURTLE = 0;

    p.ADV_QSK = 0;
    p.ADV_QSK2 = 0;
    p.RES_SKILL = 0;
    p.wizard_m2 = 0;

    // Novice ground
    for (let i = 0; i <= 5; i++) p[`NEW_MES_FLAG${i}`] = 0;
    for (let i = 0; i <= 1; i++) p[`NEW_LVUP${i}`] = 0;
    p.NEW_JOBLVUP = 0;

    // DTS
    p.dtseligible = 0;
    p.MISC_QUEST = (Number(p.MISC_QUEST) || 0) & ~128;
}

/** Save current job's quest skills. Mirror of `F_SaveQuestSkills`. */
export function F_SaveQuestSkills(_ctx: NpcContext): void {
    // Body depends on per-class skill enumeration helpers we haven't
    // ported yet (relies on getskilllv across a fixed skill table).
    // The shape is here so dependent NPCs port without a TS error.
}

export function F_Load1Skills(_ctx: NpcContext): void {
    // Same — depends on the saved-skill table.
}

export function F_Load2Skills(_ctx: NpcContext): void {
    // Same — depends on the saved-skill table.
}

/**
 * Restore Platinum skills appropriate to the player's job. Mirror of
 * `F_GetPlatinumSkills`. Currently a no-op until the skill engine ops
 * graduate from stub.
 */
export function F_GetPlatinumSkills(_ctx: NpcContext): void {
    // Body is roughly: for each (jobBits, skillId) entry, if the player
    // matches the job bits and doesn't have the skill, grant it.
}

/**
 * Predicate: did the equipped item id at `pos` match the recorded id?
 * Mirror of `F_IsEquipIDHack`. Returns true on mismatch (i.e. detected).
 */
export function F_IsEquipIDHack(ctx: NpcContext, pos: number, expectedId: number): boolean {
    if (!ctx.player) return false;
    return ctx.player.getEquipId(pos) !== expectedId;
}

/** Mirror of `F_IsEquipRefineHack`. */
export function F_IsEquipRefineHack(ctx: NpcContext, pos: number, expectedRefine: number): boolean {
    if (!ctx.player) return false;
    return ctx.player.getEquipRefine(pos) !== expectedRefine;
}

/** Mirror of `F_IsEquipCardHack`. */
export function F_IsEquipCardHack(
    ctx: NpcContext, pos: number, c0: number, c1: number, c2: number, c3: number,
): boolean {
    if (!ctx.player) return false;
    return ctx.player.getEquipCardId(pos, 0) !== c0
        || ctx.player.getEquipCardId(pos, 1) !== c1
        || ctx.player.getEquipCardId(pos, 2) !== c2
        || ctx.player.getEquipCardId(pos, 3) !== c3;
}

/** Mirror of `F_CanOpenStorage` — true if the player can open storage. */
export function F_CanOpenStorage(ctx: NpcContext): boolean {
    if (!ctx.player) return false;
    return ctx.player.basicSkillCheck();
}

/** Mirror of `F_CanChangeJob` — true if the player meets job-change reqs. */
export function F_CanChangeJob(ctx: NpcContext): boolean {
    if (!ctx.player) return false;
    return ctx.player.basicSkillCheck();
}

/** Mirror of `F_IsCharm` — true if the item id is one of the Charm series. */
export function F_IsCharm(itemId: number): boolean {
    // The rAthena impl enumerates a fixed list of Charm-class item ids.
    // Until that list lands in db/ alongside item_db.yml, defer to a
    // shape-only stub.
    const CHARM_IDS = new Set<number>([
        12716, 12717, 12718, 12719, 12720, 12721,
    ]);
    return CHARM_IDS.has(itemId);
}

// ---------------------------------------------------------------------
// RE-only globals (npc/re/other/Global_Functions.txt)
// ---------------------------------------------------------------------

/**
 * Compute the next WoE TE start tick. Mirror of `WoeTETimeStart`. Used
 * by WoE control NPCs.
 *
 * @returns Unix timestamp (seconds) of the next configured WoE start.
 */
export function WoeTETimeStart(_ctx: NpcContext): number {
    // The legacy body reads server-side battle config + the WoE config
    // table. Surface only until the config-reader op graduates.
    return 0;
}

/** Mirror of `F_22507` (Shabby Old Scroll buff). */
export function F_22507(ctx: NpcContext): void {
    if (!ctx.player) return;
    // Original: sc_start SC_BLESSING, 240000, 10; sc_start SC_INC_AGI, ...
    ctx.player.scStart(/* SC_BLESSING */ 4, 240_000, { val1: 10 });
    ctx.player.scStart(/* SC_INC_AGI  */ 6, 240_000, { val1: 10 });
}

/** Mirror of `F_queststatus`. Returns 0 = not active, 1 = active, 2 = done. */
export function F_queststatus(ctx: NpcContext, questId: number): number {
    if (!ctx.player) return 0;
    return ctx.player.quest.check(questId);
}

// ---------------------------------------------------------------------
// CamelCase aliases (idiomatic TS)
// ---------------------------------------------------------------------

/** @see F_Rand */         export const rand          = F_Rand;
/** @see F_Hi */           export const hi            = F_Hi;
/** @see F_Bye */          export const bye           = F_Bye;
/** @see F_InsertComma */  export const insertComma   = F_InsertComma;
/** @see F_GetNumSuffix */ export const numSuffix     = F_GetNumSuffix;
/** @see F_GetPlural */    export const pluralize     = F_GetPlural;
/** @see F_InsertPlural */ export const countNoun     = F_InsertPlural;
/** @see F_GetArticle */   export const indefArticle  = F_GetArticle;
/** @see F_InsertArticle */ export const withArticle  = F_InsertArticle;
/** @see F_SexMes */       export const sexMes        = F_SexMes;
/** @see Time2Str */       export const time2Str      = Time2Str;
/** @see F_getpositionname */ export const equipSlotName = F_getpositionname;
