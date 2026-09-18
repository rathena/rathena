/**
 * DB/Jobs/WeaponJobTable.js
 *
 * Look up: job id -> ressource name
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import JobId from './JobConst.js';
import JobNameTable from './JobNameTable.js';

const WeaponJobTable = {};

// Extend
const keys = Object.keys(JobNameTable);
for (let i = 0, count = keys.length; i < count; ++i) {
	WeaponJobTable[keys[i]] = JobNameTable[keys[i]];
}

function duplicateEntry(origin) {
	const value = JobNameTable[origin];
	for (let i = 1, count = arguments.length; i < count; ++i) {
		WeaponJobTable[arguments[i]] = value;
	}
}

// Rewrite inheritance
duplicateEntry(JobId.NOVICE, JobId.NOVICE_H, JobId.NOVICE_B);
duplicateEntry(JobId.SWORDMAN, JobId.SWORDMAN_H, JobId.SWORDMAN_B);
duplicateEntry(JobId.MAGICIAN, JobId.MAGICIAN_H, JobId.MAGICIAN_B);
duplicateEntry(JobId.ARCHER, JobId.ARCHER_H, JobId.ARCHER_B);
duplicateEntry(JobId.ACOLYTE, JobId.ACOLYTE_H, JobId.ACOLYTE_B);
duplicateEntry(JobId.MERCHANT, JobId.MERCHANT_H, JobId.MERCHANT_B);
duplicateEntry(JobId.THIEF, JobId.THIEF_H, JobId.THIEF_B);

// 2nd/3rd/4th job inheritance with riding mounts
duplicateEntry(
	JobId.KNIGHT,
	JobId.KNIGHT_H,
	JobId.KNIGHT_B,
	JobId.RUNE_KNIGHT,
	JobId.RUNE_KNIGHT_H,
	JobId.RUNE_KNIGHT_B,
	JobId.RUNE_KNIGHT_2ND,
	JobId.DRAGON_KNIGHT,
	JobId.DRAGON_KNIGHT_RIDING
);
duplicateEntry(
	JobId.KNIGHT2,
	JobId.KNIGHT2_H,
	JobId.KNIGHT2_B,
	JobId.RUNE_KNIGHT2,
	JobId.RUNE_KNIGHT2_H,
	JobId.RUNE_KNIGHT2_B,
	JobId.RUNE_KNIGHT2_2ND,
	JobId.DRAGON_KNIGHT2
);
duplicateEntry(
	JobId.PRIEST,
	JobId.PRIEST_H,
	JobId.PRIEST_B,
	JobId.ARCHBISHOP,
	JobId.ARCHBISHOP_H,
	JobId.ARCHBISHOP_B,
	JobId.ARCH_BISHOP_2ND,
	JobId.CARDINAL,
	JobId.CARDINAL_RIDING
);
duplicateEntry(
	JobId.WIZARD,
	JobId.WIZARD_H,
	JobId.WIZARD_B,
	JobId.WARLOCK,
	JobId.WARLOCK_H,
	JobId.WARLOCK_B,
	JobId.WARLOCK_2ND,
	JobId.ARCH_MAGE,
	JobId.ARCH_MAGE_RIDING
);
duplicateEntry(
	JobId.BLACKSMITH,
	JobId.BLACKSMITH_H,
	JobId.BLACKSMITH_B,
	JobId.MECHANIC,
	JobId.MECHANIC_H,
	JobId.MECHANIC_B,
	JobId.MECHANIC_2ND,
	JobId.MECHANIC2_2ND,
	JobId.MEISTER,
	JobId.MEISTER2,
	JobId.MEISTER_RIDING
);
duplicateEntry(
	JobId.HUNTER,
	JobId.HUNTER_H,
	JobId.HUNTER_B,
	JobId.RANGER,
	JobId.RANGER_H,
	JobId.RANGER_B,
	JobId.RANGER_2ND,
	JobId.RANGER2_2ND,
	JobId.WINDHAWK,
	JobId.WINDHAWK2,
	JobId.WINDHAWK_RIDING
);
duplicateEntry(
	JobId.ASSASSIN,
	JobId.ASSASSIN_H,
	JobId.ASSASSIN_B,
	JobId.GUILLOTINE_CROSS,
	JobId.GUILLOTINE_CROSS_H,
	JobId.GUILLOTINE_CROSS_B,
	JobId.GUILLOTINE_CROSS_2ND,
	JobId.SHADOW_CROSS,
	JobId.SHADOW_CROSS_RIDING
);
duplicateEntry(
	JobId.CRUSADER,
	JobId.CRUSADER_H,
	JobId.CRUSADER_B,
	JobId.ROYAL_GUARD,
	JobId.ROYAL_GUARD_H,
	JobId.ROYAL_GUARD_B,
	JobId.ROYAL_GUARD_2ND,
	JobId.IMPERIAL_GUARD,
	JobId.IMPERIAL_GUARD_RIDING
);
duplicateEntry(
	JobId.CRUSADER2,
	JobId.CRUSADER2_H,
	JobId.CRUSADER2_B,
	JobId.ROYAL_GUARD2,
	JobId.ROYAL_GUARD2_H,
	JobId.ROYAL_GUARD2_B,
	JobId.ROYAL_GUARD2_2ND,
	JobId.IMPERIAL_GUARD2
);
duplicateEntry(
	JobId.MONK,
	JobId.MONK_H,
	JobId.MONK_B,
	JobId.SURA,
	JobId.SURA_H,
	JobId.SURA_B,
	JobId.SURA_2ND,
	JobId.INQUISITOR,
	JobId.INQUISITOR_RIDING
);
duplicateEntry(
	JobId.SAGE,
	JobId.SAGE_H,
	JobId.SAGE_B,
	JobId.SORCERER,
	JobId.SORCERER_H,
	JobId.SORCERER_B,
	JobId.SORCERER_2ND,
	JobId.ELEMENTAL_MASTER,
	JobId.ELEMENTAL_MASTER_RIDING
);
duplicateEntry(
	JobId.ROGUE,
	JobId.ROGUE_H,
	JobId.ROGUE_B,
	JobId.SHADOW_CHASER,
	JobId.SHADOW_CHASER_H,
	JobId.SHADOW_CHASER_B,
	JobId.SHADOW_CHASER_2ND,
	JobId.ABYSS_CHASER,
	JobId.ABYSS_CHASER_RIDING
);
duplicateEntry(
	JobId.ALCHEMIST,
	JobId.ALCHEMIST_H,
	JobId.ALCHEMIST_B,
	JobId.GENETIC,
	JobId.GENETIC_H,
	JobId.GENETIC_B,
	JobId.GENETIC_2ND,
	JobId.BIOLO,
	JobId.BIOLO_RIDING
);
duplicateEntry(
	JobId.BARD,
	JobId.BARD_H,
	JobId.BARD_B,
	JobId.MINSTREL,
	JobId.MINSTREL_H,
	JobId.MINSTREL_B,
	JobId.MINSTREL_2ND,
	JobId.TROUBADOUR,
	JobId.TROUBADOUR_RIDING
);
duplicateEntry(
	JobId.DANCER,
	JobId.DANCER_H,
	JobId.DANCER_B,
	JobId.WANDERER,
	JobId.WANDERER_H,
	JobId.WANDERER_B,
	JobId.WANDERER_2ND,
	JobId.TROUVERE,
	JobId.TROUVERE_RIDING
);

// Exports
export default WeaponJobTable;
