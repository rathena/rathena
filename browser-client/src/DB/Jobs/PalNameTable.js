/**
 * DB/Jobs/PalNameTable.js
 *
 * Look up: job id -> pal ressource name
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 */

import JobId from './JobConst.js';
import JobNameTable from './JobNameTable.js';

const PalNameTable = {};
PalNameTable[JobId.NOVICE] = JobNameTable[JobId.NOVICE];

PalNameTable[JobId.SWORDMAN] = JobNameTable[JobId.SWORDMAN];
PalNameTable[JobId.MAGICIAN] = JobNameTable[JobId.MAGICIAN];
PalNameTable[JobId.ARCHER] = JobNameTable[JobId.ARCHER];
PalNameTable[JobId.ACOLYTE] = JobNameTable[JobId.ACOLYTE];
PalNameTable[JobId.MERCHANT] = JobNameTable[JobId.MERCHANT];
PalNameTable[JobId.THIEF] = JobNameTable[JobId.THIEF];

PalNameTable[JobId.KNIGHT] = JobNameTable[JobId.KNIGHT];
PalNameTable[JobId.PRIEST] = JobNameTable[JobId.PRIEST];
PalNameTable[JobId.WIZARD] = JobNameTable[JobId.WIZARD];
PalNameTable[JobId.BLACKSMITH] = JobNameTable[JobId.BLACKSMITH];
PalNameTable[JobId.HUNTER] = JobNameTable[JobId.HUNTER];
PalNameTable[JobId.ASSASSIN] = JobNameTable[JobId.ASSASSIN];
PalNameTable[JobId.KNIGHT2] = JobNameTable[JobId.KNIGHT2];
PalNameTable[JobId.CRUSADER] = '\xc5\xa9\xb7\xe7'; // "crew"
PalNameTable[JobId.MONK] = JobNameTable[JobId.MONK];
PalNameTable[JobId.SAGE] = JobNameTable[JobId.SAGE];
PalNameTable[JobId.ROGUE] = JobNameTable[JobId.ROGUE];
PalNameTable[JobId.ALCHEMIST] = JobNameTable[JobId.ALCHEMIST];
PalNameTable[JobId.BARD] = JobNameTable[JobId.BARD];
PalNameTable[JobId.DANCER] = '\xb4\xed\xbc\xad'; // "dancer" daenseo instead of muhui
PalNameTable[JobId.CRUSADER2] = '\xc6\xe4\xc4\xda\xc6\xe4\xc4\xda\x5f\xc5\xa9\xb7\xe7'; // "pecopeco crew"

PalNameTable[JobId.SUPERNOVICE] = JobNameTable[JobId.SUPERNOVICE];
PalNameTable[JobId.GUNSLINGER] = JobNameTable[JobId.GUNSLINGER];
PalNameTable[JobId.NINJA] = JobNameTable[JobId.NINJA];
PalNameTable[JobId.TAEKWON] = JobNameTable[JobId.TAEKWON];
PalNameTable[JobId.STAR] = JobNameTable[JobId.STAR];
PalNameTable[JobId.STAR2] = JobNameTable[JobId.STAR2];

PalNameTable[JobId.LINKER] = JobNameTable[JobId.LINKER];
PalNameTable[JobId.MARRIED] = JobNameTable[JobId.MARRIED];
PalNameTable[JobId.XMAS] = JobNameTable[JobId.XMAS];
PalNameTable[JobId.SUMMER] = JobNameTable[JobId.SUMMER];

PalNameTable[JobId.KNIGHT_H] = JobNameTable[JobId.KNIGHT_H];
PalNameTable[JobId.PRIEST_H] = '\xc7\xcf\xc0\xcc\xc7\xc1\xb8\xae\xbd\xba\xc6\xae'; // "high priest"
PalNameTable[JobId.WIZARD_H] = JobNameTable[JobId.WIZARD_H];
PalNameTable[JobId.BLACKSMITH_H] = JobNameTable[JobId.BLACKSMITH_H];
PalNameTable[JobId.HUNTER_H] = JobNameTable[JobId.HUNTER_H];
PalNameTable[JobId.ASSASSIN_H] = '\xbe\xee\xbc\xbc\xbd\xc5\xc5\xa9\xb7\xce\xbd\xba'; // "assassin cross"
PalNameTable[JobId.KNIGHT2_H] = '\xc6\xe4\xc4\xda\xb7\xce\xb3\xaa'; // "pecorona"
PalNameTable[JobId.CRUSADER_H] = JobNameTable[JobId.CRUSADER_H];
PalNameTable[JobId.MONK_H] = JobNameTable[JobId.MONK_H];
PalNameTable[JobId.SAGE_H] = JobNameTable[JobId.SAGE_H];
PalNameTable[JobId.ROGUE_H] = JobNameTable[JobId.ROGUE_H];
PalNameTable[JobId.ALCHEMIST_H] = JobNameTable[JobId.ALCHEMIST_H];
PalNameTable[JobId.BARD_H] = '\xc5\xa9\xb6\xf3\xbf\xee'; // "crown"
PalNameTable[JobId.DANCER_H] = JobNameTable[JobId.DANCER_H];
PalNameTable[JobId.CRUSADER2_H] = '\xc6\xe4\xc4\xda\xc6\xc8\xb6\xf3'; // "pekopala"

PalNameTable[JobId.RUNE_KNIGHT] = JobNameTable[JobId.RUNE_KNIGHT];
PalNameTable[JobId.WARLOCK] = JobNameTable[JobId.WARLOCK];
PalNameTable[JobId.RANGER] = '\xb7\xb9\xc0\xce\xc0\xfa'; // "Ranger"... has a typo compared to path name.
PalNameTable[JobId.ARCHBISHOP] = JobNameTable[JobId.ARCHBISHOP];
PalNameTable[JobId.MECHANIC] = JobNameTable[JobId.MECHANIC];
PalNameTable[JobId.GUILLOTINE_CROSS] = JobNameTable[JobId.GUILLOTINE_CROSS];
PalNameTable[JobId.ROYAL_GUARD] = '\xb7\xce\xbe\xe2\xb0\xa1\xb5\xe5'; // "royal guard"
PalNameTable[JobId.SORCERER] = JobNameTable[JobId.SORCERER];
PalNameTable[JobId.MINSTREL] = JobNameTable[JobId.MINSTREL];
PalNameTable[JobId.WANDERER] = JobNameTable[JobId.WANDERER];
PalNameTable[JobId.SURA] = JobNameTable[JobId.SURA];
PalNameTable[JobId.GENETIC] = JobNameTable[JobId.GENETIC];
PalNameTable[JobId.SHADOW_CHASER] = JobNameTable[JobId.SHADOW_CHASER];
PalNameTable[JobId.RUNE_KNIGHT2] = '\xb7\xe9\xb5\xe5\xb7\xa1\xb0\xef'; // "rune dragon"
PalNameTable[JobId.ROYAL_GUARD2] = JobNameTable[JobId.ROYAL_GUARD2];
PalNameTable[JobId.RANGER2] = '\xbf\xef\xc7\xc1\xb7\xb9\xc0\xce\xc0\xfa'; // "wolfranger"
PalNameTable[JobId.MECHANIC2] = '\xb9\xcc\xc4\xc9\xb4\xd0\x5f\xb8\xb6\xb5\xb5\xb1\xe2\xbe\xee'; // "mechanic_mado gear"

PalNameTable[JobId.RUNE_KNIGHT_2ND] = 'costume_1/' + PalNameTable[JobId.RUNE_KNIGHT];
PalNameTable[JobId.WARLOCK_2ND] = 'costume_1/' + PalNameTable[JobId.WARLOCK];
PalNameTable[JobId.RANGER_2ND] = 'costume_1/' + PalNameTable[JobId.RANGER];
PalNameTable[JobId.ARCHBISHOP_2ND] = 'costume_1/' + PalNameTable[JobId.ARCHBISHOP];
PalNameTable[JobId.MECHANIC_2ND] = 'costume_1/' + PalNameTable[JobId.MECHANIC];
PalNameTable[JobId.GUILLOTINE_CROSS_2ND] = 'costume_1/' + PalNameTable[JobId.GUILLOTINE_CROSS];
PalNameTable[JobId.ROYAL_GUARD_2ND] = 'costume_1/' + PalNameTable[JobId.ROYAL_GUARD];
PalNameTable[JobId.SORCERER_2ND] = 'costume_1/' + PalNameTable[JobId.SORCERER];
PalNameTable[JobId.MINSTREL_2ND] = 'costume_1/' + PalNameTable[JobId.MINSTREL];
PalNameTable[JobId.WANDERER_2ND] = 'costume_1/' + PalNameTable[JobId.WANDERER];
PalNameTable[JobId.SURA_2ND] = 'costume_1/' + PalNameTable[JobId.SURA];
PalNameTable[JobId.GENETIC_2ND] = 'costume_1/' + PalNameTable[JobId.GENETIC];
PalNameTable[JobId.SHADOW_CHASER_2ND] = 'costume_1/' + PalNameTable[JobId.SHADOW_CHASER];
PalNameTable[JobId.RUNE_KNIGHT2_2ND] = 'costume_1/' + PalNameTable[JobId.RUNE_KNIGHT2];
PalNameTable[JobId.ROYAL_GUARD2_2ND] = 'costume_1/' + PalNameTable[JobId.ROYAL_GUARD2];
PalNameTable[JobId.RANGER2_2ND] = 'costume_1/' + PalNameTable[JobId.RANGER2];
PalNameTable[JobId.MECHANIC2_2ND] = 'costume_1/' + PalNameTable[JobId.MECHANIC2];

PalNameTable[JobId.SUPERNOVICE2] = JobNameTable[JobId.SUPERNOVICE2];
PalNameTable[JobId.KAGEROU] = 'KAGEROU'; // Upper case
PalNameTable[JobId.OBORO] = 'OBORO'; // Upper case
PalNameTable[JobId.REBELLION] = '\xb8\xae\xba\xa7\xb8\xae\xbf\xc2'; // "rebellion"
PalNameTable[JobId.STAR_EMPEROR] = JobNameTable[JobId.STAR_EMPEROR];
PalNameTable[JobId.STAR_EMPEROR2] = JobNameTable[JobId.STAR_EMPEROR2];
PalNameTable[JobId.SOUL_REAPER] = JobNameTable[JobId.SOUL_REAPER];
PalNameTable[JobId.SOUL_REAPER2] = JobNameTable[JobId.SOUL_REAPER2];
PalNameTable[JobId.DO_SUMMONER] = '\xb9\xa6\xc1\xb7'; // "miao"

//MOUNTS                              	//MOUNTS
PalNameTable[JobId.PORING_NOVICE] = JobNameTable[JobId.PORING_NOVICE];

PalNameTable[JobId.SHEEP_ACO] = JobNameTable[JobId.SHEEP_ACO];
PalNameTable[JobId.OSTRICH_ARCHER] = JobNameTable[JobId.OSTRICH_ARCHER];
PalNameTable[JobId.FOX_MAGICIAN] = JobNameTable[JobId.FOX_MAGICIAN];
PalNameTable[JobId.PIG_MERCHANT] = JobNameTable[JobId.PIG_MERCHANT];
PalNameTable[JobId.PECO_SWORD] = JobNameTable[JobId.PECO_SWORD];
PalNameTable[JobId.DOG_THIEF] = JobNameTable[JobId.DOG_THIEF];

PalNameTable[JobId.SHEEP_PRIEST] = JobNameTable[JobId.SHEEP_PRIEST];
PalNameTable[JobId.OSTRICH_HUNTER] = JobNameTable[JobId.OSTRICH_HUNTER];
PalNameTable[JobId.FOX_WIZ] = JobNameTable[JobId.FOX_WIZ];
PalNameTable[JobId.PIG_BLACKSMITH] = JobNameTable[JobId.PIG_BLACKSMITH];
PalNameTable[JobId.LION_KNIGHT] = JobNameTable[JobId.LION_KNIGHT];
PalNameTable[JobId.DOG_ASSASSIN] = JobNameTable[JobId.DOG_ASSASSIN];
PalNameTable[JobId.SHEEP_MONK] = JobNameTable[JobId.MONK]; // Monk
PalNameTable[JobId.OSTRICH_BARD] = JobNameTable[JobId.BARD]; // Bard
PalNameTable[JobId.OSTRICH_DANCER] = JobNameTable[JobId.DANCER]; // Dancer
PalNameTable[JobId.FOX_SAGE] = JobNameTable[JobId.SAGE]; // Sage
PalNameTable[JobId.PIG_ALCHE] = JobNameTable[JobId.ALCHEMIST]; // Alchemist
PalNameTable[JobId.LION_CRUSADER] = '\xc5\xa9\xb7\xe7'; // "crew"
PalNameTable[JobId.DOG_ROGUE] = JobNameTable[JobId.ROGUE]; // Rogue

PalNameTable[JobId.SHEEP_ARCB] = JobNameTable[JobId.SHEEP_ARCB];
PalNameTable[JobId.OSTRICH_RANGER] = JobNameTable[JobId.OSTRICH_RANGER];
PalNameTable[JobId.FOX_WARLOCK] = JobNameTable[JobId.FOX_WARLOCK];
PalNameTable[JobId.PIG_MECHANIC] = JobNameTable[JobId.PIG_MECHANIC];
PalNameTable[JobId.LION_RUNE_KNIGHT] = JobNameTable[JobId.LION_RUNE_KNIGHT];
PalNameTable[JobId.DOG_G_CROSS] = JobNameTable[JobId.DOG_G_CROSS];
PalNameTable[JobId.SHEEP_SURA] = JobNameTable[JobId.SHEEP_SURA];
PalNameTable[JobId.OSTRICH_MINSTREL] = JobNameTable[JobId.OSTRICH_MINSTREL];
PalNameTable[JobId.OSTRICH_WANDER] = JobNameTable[JobId.OSTRICH_WANDER];
PalNameTable[JobId.FOX_SORCERER] = JobNameTable[JobId.FOX_SORCERER];
PalNameTable[JobId.PIG_GENETIC] = JobNameTable[JobId.PIG_GENETIC];
PalNameTable[JobId.LION_ROYAL_GUARD] = JobNameTable[JobId.LION_ROYAL_GUARD];
PalNameTable[JobId.DOG_CHASER] = JobNameTable[JobId.DOG_CHASER];

PalNameTable[JobId.PORING_SNOVICE] = JobNameTable[JobId.PORING_SNOVICE];
PalNameTable[JobId.FROG_NINJA] = JobNameTable[JobId.FROG_NINJA];
PalNameTable[JobId.PECO_GUNNER] = JobNameTable[JobId.PECO_GUNNER];
PalNameTable[JobId.PORING_TAEKWON] = JobNameTable[JobId.PORING_TAEKWON];
PalNameTable[JobId.PORING_STAR] = JobNameTable[JobId.STAR]; // Star Gladiator
PalNameTable[JobId.FROG_LINKER] = JobNameTable[JobId.FROG_LINKER];

PalNameTable[JobId.FROG_KAGEROU] = JobNameTable[JobId.FROG_KAGEROU];
PalNameTable[JobId.FROG_OBORO] = JobNameTable[JobId.FROG_OBORO];
PalNameTable[JobId.PECO_REBELLION] = JobNameTable[JobId.PECO_REBELLION];

PalNameTable[JobId.SHEEP_HPRIEST] = '\xc7\xcf\xc0\xcc\xc7\xc1\xb8\xae\xbd\xba\xc6\xae'; // "high priest"
PalNameTable[JobId.OSTRICH_SNIPER] = JobNameTable[JobId.HUNTER_H]; // Sniper
PalNameTable[JobId.FOX_HWIZ] = JobNameTable[JobId.WIZARD_H]; // High Wizard
PalNameTable[JobId.PIG_WHITESMITH] = JobNameTable[JobId.BLACKSMITH_H]; // Whitesmith
PalNameTable[JobId.LION_KNIGHT_H] = JobNameTable[JobId.LION_KNIGHT_H];
PalNameTable[JobId.DOG_ASSA_X] = '\xbe\xee\xbc\xbc\xbd\xc5\xc5\xa9\xb7\xce\xbd\xba'; // "assassin cross"
PalNameTable[JobId.SHEEP_CHAMP] = JobNameTable[JobId.MONK_H]; // Champion
PalNameTable[JobId.OSTRICH_CROWN] = '\xc5\xa9\xb6\xf3\xbf\xee'; // "crown"
PalNameTable[JobId.OSTRICH_ZIPSI] = '\xc1\xfd\xbd\xc3'; // "gypsy"
PalNameTable[JobId.FOX_PROF] = JobNameTable[JobId.SAGE_H]; // Professor
PalNameTable[JobId.PIG_CREATOR] = JobNameTable[JobId.ALCHEMIST_H]; // Creator
PalNameTable[JobId.LION_CRUSADER_H] = JobNameTable[JobId.CRUSADER_H]; // Paladin
PalNameTable[JobId.DOG_STALKER] = JobNameTable[JobId.ROGUE_H]; // Stalker

//PalNameTable[JobId.CART_DO_SUMMONER] 	 = "\xb0\xed\xbe\xe7\xc0\xcc\xc4\xab\xc6\xae";	// "cat cart"

// 4th
PalNameTable[JobId.DRAGON_KNIGHT] = JobNameTable[JobId.DRAGON_KNIGHT];
PalNameTable[JobId.MEISTER] = JobNameTable[JobId.MEISTER];
PalNameTable[JobId.SHADOW_CROSS] = JobNameTable[JobId.SHADOW_CROSS];
PalNameTable[JobId.ARCH_MAGE] = JobNameTable[JobId.ARCH_MAGE];
PalNameTable[JobId.CARDINAL] = JobNameTable[JobId.CARDINAL];
PalNameTable[JobId.WINDHAWK] = JobNameTable[JobId.WINDHAWK];
PalNameTable[JobId.IMPERIAL_GUARD] = JobNameTable[JobId.IMPERIAL_GUARD];
PalNameTable[JobId.BIOLO] = JobNameTable[JobId.BIOLO];
PalNameTable[JobId.ABYSS_CHASER] = JobNameTable[JobId.ABYSS_CHASER];
PalNameTable[JobId.ELEMENTAL_MASTER] = JobNameTable[JobId.ELEMENTAL_MASTER];
PalNameTable[JobId.INQUISITOR] = JobNameTable[JobId.INQUISITOR];
PalNameTable[JobId.TROUBADOUR] = JobNameTable[JobId.TROUBADOUR];
PalNameTable[JobId.TROUVERE] = JobNameTable[JobId.TROUVERE];

PalNameTable[JobId.WINDHAWK2] = JobNameTable[JobId.WINDHAWK2];
PalNameTable[JobId.MEISTER2] = JobNameTable[JobId.MEISTER2];
PalNameTable[JobId.DRAGON_KNIGHT2] = JobNameTable[JobId.DRAGON_KNIGHT2];
PalNameTable[JobId.IMPERIAL_GUARD2] = JobNameTable[JobId.IMPERIAL_GUARD2];

PalNameTable[JobId.SKY_EMPEROR] = JobNameTable[JobId.SKY_EMPEROR];
PalNameTable[JobId.SOUL_ASCETIC] = JobNameTable[JobId.SOUL_ASCETIC];
PalNameTable[JobId.SHINKIRO] = JobNameTable[JobId.SHINKIRO];
PalNameTable[JobId.SHIRANUI] = JobNameTable[JobId.SHIRANUI];
PalNameTable[JobId.NIGHT_WATCH] = JobNameTable[JobId.NIGHT_WATCH];
PalNameTable[JobId.HYPER_NOVICE] = JobNameTable[JobId.HYPER_NOVICE];
PalNameTable[JobId.SPIRIT_HANDLER] = JobNameTable[JobId.SPIRIT_HANDLER];

PalNameTable[JobId.SKY_EMPEROR2] = JobNameTable[JobId.SKY_EMPEROR2];

function duplicateEntry(origin) {
	const value = JobNameTable[origin];
	for (let i = 1, count = arguments.length; i < count; ++i) {
		PalNameTable[arguments[i]] = value;
	}
}
// Inherit
duplicateEntry(JobId.NOVICE, JobId.NOVICE_H, JobId.NOVICE_B);
duplicateEntry(JobId.SWORDMAN, JobId.SWORDMAN_H, JobId.SWORDMAN_B);
duplicateEntry(JobId.MAGICIAN, JobId.MAGICIAN_H, JobId.MAGICIAN_B);
duplicateEntry(JobId.ARCHER, JobId.ARCHER_H, JobId.ARCHER_B);
duplicateEntry(JobId.ACOLYTE, JobId.ACOLYTE_H, JobId.ACOLYTE_B);
duplicateEntry(JobId.MERCHANT, JobId.MERCHANT_H, JobId.MERCHANT_B);
duplicateEntry(JobId.THIEF, JobId.THIEF_H, JobId.THIEF_B);
duplicateEntry(JobId.KNIGHT, JobId.KNIGHT_B);
duplicateEntry(JobId.KNIGHT2, JobId.KNIGHT2_B);
duplicateEntry(JobId.PRIEST, JobId.PRIEST_B);
duplicateEntry(JobId.WIZARD, JobId.WIZARD_B);
duplicateEntry(JobId.BLACKSMITH, JobId.BLACKSMITH_B);
duplicateEntry(JobId.HUNTER, JobId.HUNTER_B);
duplicateEntry(JobId.ASSASSIN, JobId.ASSASSIN_B);
duplicateEntry(JobId.CRUSADER, JobId.CRUSADER_B);
duplicateEntry(JobId.CRUSADER2, JobId.CRUSADER2_B);
duplicateEntry(JobId.MONK, JobId.MONK_B);
duplicateEntry(JobId.SAGE, JobId.SAGE_B);
duplicateEntry(JobId.ROGUE, JobId.ROGUE_B);
duplicateEntry(JobId.ALCHEMIST, JobId.ALCHEMIST_B);
duplicateEntry(JobId.BARD, JobId.BARD_B);
duplicateEntry(JobId.DANCER, JobId.DANCER_B);
duplicateEntry(JobId.RUNE_KNIGHT, JobId.RUNE_KNIGHT_H, JobId.RUNE_KNIGHT_B);
duplicateEntry(JobId.RUNE_KNIGHT2, JobId.RUNE_KNIGHT2_H, JobId.RUNE_KNIGHT2_B);
duplicateEntry(JobId.WARLOCK, JobId.WARLOCK_H, JobId.WARLOCK_B);
duplicateEntry(JobId.RANGER, JobId.RANGER_H, JobId.RANGER_B);
duplicateEntry(JobId.RANGER2, JobId.RANGER2_H, JobId.RANGER2_B);
duplicateEntry(JobId.ARCHBISHOP, JobId.ARCHBISHOP_H, JobId.ARCHBISHOP_B);
duplicateEntry(JobId.MECHANIC, JobId.MECHANIC_H, JobId.MECHANIC_B);
duplicateEntry(JobId.MECHANIC2, JobId.MECHANIC2_H, JobId.MECHANIC2_B);
duplicateEntry(JobId.GUILLOTINE_CROSS, JobId.GUILLOTINE_CROSS_H, JobId.GUILLOTINE_CROSS_B);
duplicateEntry(JobId.ROYAL_GUARD, JobId.ROYAL_GUARD_H, JobId.ROYAL_GUARD_B);
duplicateEntry(JobId.ROYAL_GUARD2, JobId.ROYAL_GUARD2_H, JobId.ROYAL_GUARD2_B);
duplicateEntry(JobId.SORCERER, JobId.SORCERER_H, JobId.SORCERER_B);
duplicateEntry(JobId.MINSTREL, JobId.MINSTREL_H, JobId.MINSTREL_B);
duplicateEntry(JobId.WANDERER, JobId.WANDERER_H, JobId.WANDERER_B);
duplicateEntry(JobId.SURA, JobId.SURA_H, JobId.SURA_B);
duplicateEntry(JobId.GENETIC, JobId.GENETIC_H, JobId.GENETIC_B);
duplicateEntry(JobId.SHADOW_CHASER, JobId.SHADOW_CHASER_H, JobId.SHADOW_CHASER_B);
duplicateEntry(JobId.DO_SUMMONER, JobId.DO_SUMMONER_B);
duplicateEntry(JobId.SOUL_REAPER, JobId.SOUL_REAPER_B);
duplicateEntry(JobId.STAR_EMPEROR, JobId.STAR_EMPEROR_B);
//MOUNTS
duplicateEntry(JobId.PORING_NOVICE, JobId.PORING_NOVICE_H, JobId.PORING_NOVICE_B);
duplicateEntry(JobId.SHEEP_ACO, JobId.SHEEP_ACO_H, JobId.SHEEP_ACO_B);
duplicateEntry(JobId.OSTRICH_ARCHER, JobId.OSTRICH_ARCHER_H, JobId.OSTRICH_ARCHER_B);
duplicateEntry(JobId.FOX_MAGICIAN, JobId.FOX_MAGICIAN_H, JobId.FOX_MAGICIAN_B);
duplicateEntry(JobId.PIG_MERCHANT, JobId.PIG_MERCHANT_H, JobId.PIG_MERCHANT_B);
duplicateEntry(JobId.PECO_SWORD, JobId.PECO_SWORD_H, JobId.PECO_SWORD_B);
duplicateEntry(JobId.DOG_THIEF, JobId.DOG_THIEF_H, JobId.DOG_THIEF_B);
duplicateEntry(JobId.SHEEP_PRIEST, JobId.SHEEP_PRIEST_B);
duplicateEntry(JobId.OSTRICH_HUNTER, JobId.OSTRICH_HUNTER_B);
duplicateEntry(JobId.FOX_WIZ, JobId.FOX_WIZ_B);
duplicateEntry(JobId.PIG_BLACKSMITH, JobId.PIG_BLACKSMITH_B);
duplicateEntry(JobId.LION_KNIGHT, JobId.LION_KNIGHT_B);
duplicateEntry(JobId.DOG_ASSASSIN, JobId.DOG_ASSASSIN_B);
duplicateEntry(JobId.SHEEP_MONK, JobId.SHEEP_MONK_B);
duplicateEntry(JobId.OSTRICH_BARD, JobId.OSTRICH_BARD_B);
duplicateEntry(JobId.OSTRICH_DANCER, JobId.OSTRICH_DANCER_B);
duplicateEntry(JobId.FOX_SAGE, JobId.FOX_SAGE_B);
duplicateEntry(JobId.PIG_ALCHE, JobId.PIG_ALCHE_B);
duplicateEntry(JobId.LION_CRUSADER, JobId.LION_CRUSADER_B);
duplicateEntry(JobId.DOG_ROGUE, JobId.DOG_ROGUE_B);
duplicateEntry(JobId.SHEEP_ARCB, JobId.SHEEP_ARCB_B);
duplicateEntry(JobId.OSTRICH_RANGER, JobId.OSTRICH_RANGER_B);
duplicateEntry(JobId.FOX_WARLOCK, JobId.FOX_WARLOCK_B);
duplicateEntry(JobId.PIG_MECHANIC, JobId.PIG_MECHANIC_B);
duplicateEntry(JobId.LION_RUNE_KNIGHT, JobId.LION_RUNE_KNIGHT_B);
duplicateEntry(JobId.DOG_G_CROSS, JobId.DOG_G_CROSS_B);
duplicateEntry(JobId.SHEEP_SURA, JobId.SHEEP_SURA_B);
duplicateEntry(JobId.OSTRICH_MINSTREL, JobId.OSTRICH_MINSTREL_B);
duplicateEntry(JobId.OSTRICH_WANDER, JobId.OSTRICH_WANDER_B);
duplicateEntry(JobId.FOX_SORCERER, JobId.FOX_SORCERER_B);
duplicateEntry(JobId.PIG_GENETIC, JobId.PIG_GENETIC_B);
duplicateEntry(JobId.LION_ROYAL_GUARD, JobId.LION_ROYAL_GUARD_B);
duplicateEntry(JobId.DOG_CHASER, JobId.DOG_CHASER_B);
duplicateEntry(JobId.PORING_SNOVICE, JobId.PORING_SNOVICE_B, JobId.PORING_SNOVICE2, JobId.PORING_SNOVICE2_B);
duplicateEntry(JobId.FROG_NINJA, JobId.FROG_NINJA_B);
duplicateEntry(JobId.PECO_GUNNER, JobId.PECO_GUNNER_B);
duplicateEntry(JobId.PORING_TAEKWON, JobId.PORING_TAEKWON_B);
duplicateEntry(JobId.PORING_STAR, JobId.PORING_STAR_B);
duplicateEntry(JobId.FROG_LINKER, JobId.FROG_LINKER_B);
duplicateEntry(JobId.FROG_KAGEROU, JobId.FROG_KAGEROU_B);
duplicateEntry(JobId.FROG_OBORO, JobId.FROG_OBORO_B);
duplicateEntry(JobId.PECO_REBELLION, JobId.PECO_REBELLION_B);
duplicateEntry(JobId.SOUL_REAPER2, JobId.SOUL_REAPER2_B);
duplicateEntry(JobId.STAR_EMPEROR2, JobId.STAR_EMPEROR2_B);

export default PalNameTable;
