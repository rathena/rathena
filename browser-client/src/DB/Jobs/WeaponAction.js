/**
 * DB/Jobs/WeaponAction.js
 *
 * Define attack action for each weapon
 * WeaponAction[<job>][<weapon type>]
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import JobId from './JobConst.js';
import WeaponType from 'DB/Items/WeaponType.js';

const WeaponAction = {};

WeaponAction[JobId.NOVICE] = [
	new (function () {
		// female
		this[WeaponType.NONE] = 0;
		this[WeaponType.ROD] = 1;
		this[WeaponType.TWOHANDROD] = 1;
		this[WeaponType.SWORD] = 1;
		this[WeaponType.TWOHANDSWORD] = 1;
		this[WeaponType.AXE] = 1;
		this[WeaponType.TWOHANDAXE] = 1;
		this[WeaponType.MACE] = 1;
		this[WeaponType.TWOHANDMACE] = 1;
		this[WeaponType.SHORTSWORD] = 2;
	})(),
	new (function () {
		// male
		this[WeaponType.NONE] = 0;
		this[WeaponType.SHORTSWORD] = 1;
		this[WeaponType.ROD] = 2;
		this[WeaponType.TWOHANDROD] = 2;
		this[WeaponType.SWORD] = 2;
		this[WeaponType.TWOHANDSWORD] = 2;
		this[WeaponType.AXE] = 2;
		this[WeaponType.TWOHANDAXE] = 2;
		this[WeaponType.MACE] = 2;
		this[WeaponType.TWOHANDMACE] = 2;
	})()
];

WeaponAction[JobId.SWORDMAN] = new (function () {
	this[WeaponType.NONE] = 0;
	this[WeaponType.SHORTSWORD] = 1;
	this[WeaponType.SWORD] = 1;
	this[WeaponType.TWOHANDSWORD] = 1;
	this[WeaponType.AXE] = 1;
	this[WeaponType.TWOHANDAXE] = 1;
	this[WeaponType.MACE] = 1;
	this[WeaponType.TWOHANDMACE] = 1;
	this[WeaponType.SPEAR] = 2;
	this[WeaponType.TWOHANDSPEAR] = 2;
})();

WeaponAction[JobId.MAGICIAN] = new (function () {
	this[WeaponType.NONE] = 0;
	this[WeaponType.ROD] = 1;
	this[WeaponType.TWOHANDROD] = 1;
	this[WeaponType.SHORTSWORD] = 2;
})();

WeaponAction[JobId.ARCHER] = new (function () {
	this[WeaponType.NONE] = 0;
	this[WeaponType.BOW] = 1;
	this[WeaponType.SHORTSWORD] = 2;
})();

WeaponAction[JobId.ACOLYTE] = new (function () {
	this[WeaponType.NONE] = 0;
	this[WeaponType.ROD] = 1;
	this[WeaponType.TWOHANDROD] = 1;
	this[WeaponType.MACE] = 1;
	this[WeaponType.TWOHANDMACE] = 1;
})();

WeaponAction[JobId.MERCHANT] = new (function () {
	this[WeaponType.NONE] = 0;
	this[WeaponType.MACE] = 1;
	this[WeaponType.TWOHANDMACE] = 1;
	this[WeaponType.AXE] = 1;
	this[WeaponType.TWOHANDAXE] = 1;
	this[WeaponType.SWORD] = 1;
	this[WeaponType.TWOHANDSWORD] = 1;
	this[WeaponType.SHORTSWORD] = 2;
})();

WeaponAction[JobId.THIEF] = new (function () {
	this[WeaponType.NONE] = 0;
	this[WeaponType.SWORD] = 1;
	this[WeaponType.TWOHANDSWORD] = 1;
	this[WeaponType.SHORTSWORD] = 1;
	this[WeaponType.BOW] = 2;
})();

WeaponAction[JobId.KNIGHT] = new (function () {
	this[WeaponType.NONE] = 0;
	this[WeaponType.SHORTSWORD] = 1;
	this[WeaponType.SWORD] = 1;
	this[WeaponType.TWOHANDSWORD] = 1;
	this[WeaponType.TWOHANDMACE] = 1;
	this[WeaponType.AXE] = 1;
	this[WeaponType.TWOHANDAXE] = 1;
	this[WeaponType.SWORD] = 1;
	this[WeaponType.MACE] = 1;
	this[WeaponType.TWOHANDMACE] = 1;
	this[WeaponType.SPEAR] = 2;
	this[WeaponType.TWOHANDSPEAR] = 2;
})();

WeaponAction[JobId.PRIEST] = new (function () {
	this[WeaponType.NONE] = 0;
	this[WeaponType.ROD] = 1;
	this[WeaponType.TWOHANDROD] = 1;
	this[WeaponType.MACE] = 1;
	this[WeaponType.TWOHANDMACE] = 1;
	this[WeaponType.BOOK] = 2;
})();

WeaponAction[JobId.WIZARD] = [
	new (function () {
		// female
		this[WeaponType.NONE] = 0;
		this[WeaponType.SHORTSWORD] = 1;
		this[WeaponType.ROD] = 2;
		this[WeaponType.TWOHANDROD] = 2;
	})(),
	new (function () {
		// male
		this[WeaponType.NONE] = 0;
		this[WeaponType.ROD] = 1;
		this[WeaponType.TWOHANDROD] = 1;
		this[WeaponType.SHORTSWORD] = 2;
	})()
];

WeaponAction[JobId.BLACKSMITH] = new (function () {
	this[WeaponType.NONE] = 0;
	this[WeaponType.SHORTSWORD] = 1;
	this[WeaponType.SWORD] = 2;
	this[WeaponType.TWOHANDSWORD] = 2;
	this[WeaponType.AXE] = 2;
	this[WeaponType.TWOHANDAXE] = 2;
	this[WeaponType.MACE] = 2;
	this[WeaponType.TWOHANDMACE] = 2;
})();

WeaponAction[JobId.HUNTER] = new (function () {
	this[WeaponType.NONE] = 0;
	this[WeaponType.SHORTSWORD] = 1;
	this[WeaponType.BOW] = 2;
})();

WeaponAction[JobId.ASSASSIN] = new (function () {
	this[WeaponType.NONE] = 0;
	this[WeaponType.AXE] = 1;
	this[WeaponType.SWORD] = 1;
	this[WeaponType.SHORTSWORD] = 1;
	this[WeaponType.SHORTSWORD_SHORTSWORD] = 2;
	this[WeaponType.SWORD_SWORD] = 2;
	this[WeaponType.AXE_AXE] = 2;
	this[WeaponType.SHORTSWORD_SWORD] = 2;
	this[WeaponType.SHORTSWORD_AXE] = 2;
	this[WeaponType.SWORD_AXE] = 2;
	this[WeaponType.KATAR] = 2;
})();

WeaponAction[JobId.KNIGHT2] = new (function () {
	this[WeaponType.NONE] = 0;
	this[WeaponType.SHORTSWORD] = 1;
	this[WeaponType.SWORD] = 1;
	this[WeaponType.TWOHANDSWORD] = 1;
	this[WeaponType.AXE] = 1;
	this[WeaponType.TWOHANDAXE] = 1;
	this[WeaponType.MACE] = 1;
	this[WeaponType.TWOHANDMACE] = 1;
	this[WeaponType.SPEAR] = 2;
	this[WeaponType.TWOHANDSPEAR] = 2;
})();

WeaponAction[JobId.CRUSADER] = new (function () {
	this[WeaponType.NONE] = 0;
	this[WeaponType.SHORTSWORD] = 1;
	this[WeaponType.SWORD] = 1;
	this[WeaponType.TWOHANDSWORD] = 1;
	this[WeaponType.AXE] = 1;
	this[WeaponType.TWOHANDAXE] = 1;
	this[WeaponType.MACE] = 1;
	this[WeaponType.TWOHANDMACE] = 1;
	this[WeaponType.SPEAR] = 2;
	this[WeaponType.TWOHANDSPEAR] = 2;
})();

WeaponAction[JobId.MONK] = new (function () {
	this[WeaponType.NONE] = 0;
	this[WeaponType.ROD] = 1;
	this[WeaponType.TWOHANDROD] = 1;
	this[WeaponType.MACE] = 1;
	this[WeaponType.TWOHANDMACE] = 1;
	this[WeaponType.KNUKLE] = 2;
})();

WeaponAction[JobId.SAGE] = new (function () {
	this[WeaponType.NONE] = 0;
	this[WeaponType.SHORTSWORD] = 1;
	this[WeaponType.ROD] = 2;
	this[WeaponType.TWOHANDROD] = 2;
	this[WeaponType.BOOK] = 2;
})();

WeaponAction[JobId.ROGUE] = new (function () {
	this[WeaponType.NONE] = 0;
	this[WeaponType.SWORD] = 1;
	this[WeaponType.TWOHANDSWORD] = 1;
	this[WeaponType.SHORTSWORD] = 1;
	this[WeaponType.BOW] = 2;
})();

WeaponAction[JobId.ALCHEMIST] = new (function () {
	this[WeaponType.NONE] = 0;
	this[WeaponType.SHORTSWORD] = 1;
	this[WeaponType.TWOHANDSWORD] = 2;
	this[WeaponType.SWORD] = 2;
	this[WeaponType.AXE] = 2;
	this[WeaponType.TWOHANDAXE] = 2;
	this[WeaponType.MACE] = 2;
	this[WeaponType.TWOHANDMACE] = 2;
})();

WeaponAction[JobId.BARD] = new (function () {
	this[WeaponType.NONE] = 0;
	this[WeaponType.SHORTSWORD] = 1;
	this[WeaponType.INSTRUMENT] = 1;
	this[WeaponType.BOW] = 2;
})();

WeaponAction[JobId.DANCER] = new (function () {
	this[WeaponType.NONE] = 0;
	this[WeaponType.SHORTSWORD] = 0;
	this[WeaponType.WHIP] = 1;
	this[WeaponType.BOW] = 2;
})();

WeaponAction[JobId.CRUSADER2] = new (function () {
	this[WeaponType.NONE] = 0;
	this[WeaponType.SHORTSWORD] = 1;
	this[WeaponType.SWORD] = 1;
	this[WeaponType.TWOHANDSWORD] = 1;
	this[WeaponType.AXE] = 1;
	this[WeaponType.TWOHANDAXE] = 1;
	this[WeaponType.MACE] = 1;
	this[WeaponType.SPEAR] = 2;
	this[WeaponType.TWOHANDSPEAR] = 2;
})();

WeaponAction[JobId.SUPERNOVICE] = [
	new (function () {
		//female
		this[WeaponType.NONE] = 0;
		this[WeaponType.ROD] = 1;
		this[WeaponType.TWOHANDROD] = 1;
		this[WeaponType.AXE] = 1;
		this[WeaponType.TWOHANDAXE] = 1;
		this[WeaponType.MACE] = 1;
		this[WeaponType.TWOHANDMACE] = 1;
		this[WeaponType.SWORD] = 1;
		this[WeaponType.SHORTSWORD] = 2;
	})(),
	new (function () {
		//male
		this[WeaponType.NONE] = 0;
		this[WeaponType.SHORTSWORD] = 1;
		this[WeaponType.ROD] = 2;
		this[WeaponType.TWOHANDROD] = 2;
		this[WeaponType.AXE] = 2;
		this[WeaponType.TWOHANDAXE] = 2;
		this[WeaponType.MACE] = 2;
		this[WeaponType.TWOHANDMACE] = 2;
		this[WeaponType.SWORD] = 2;
	})()
];

WeaponAction[JobId.NINJA] = new (function () {
	this[WeaponType.NONE] = 0;
	this[WeaponType.SHORTSWORD] = 1;
	this[WeaponType.SYURIKEN] = 2;
})();

WeaponAction[JobId.GUNSLINGER] = new (function () {
	// I don't get when 0 is used ? seems like a grenade launcher.
	this[WeaponType.NONE] = 1;
	this[WeaponType.GUN_HANDGUN] = 1;
	this[WeaponType.GUN_SHOTGUN] = 1;
	this[WeaponType.GUN_GATLING] = 2;
	this[WeaponType.GUN_RIFLE] = 2;
	this[WeaponType.GUN_GRANADE] = 2;
})();

// I don't get where the weapon sprites are located.
WeaponAction[JobId.LINKER] = [
	new (function () {
		// female
		this[WeaponType.NONE] = 0;
		this[WeaponType.SHORTSWORD] = 1;
		this[WeaponType.ROD] = 2;
		this[WeaponType.TWOHANDROD] = 2;
	})(),
	new (function () {
		// male
		this[WeaponType.NONE] = 0;
		this[WeaponType.ROD] = 1;
		this[WeaponType.TWOHANDROD] = 1;
		this[WeaponType.SHORTSWORD] = 2;
	})()
];

function duplicateEntry(origin) {
	const value = WeaponAction[origin];
	for (let i = 1, count = arguments.length; i < count; ++i) {
		WeaponAction[arguments[i]] = value;
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
duplicateEntry(
	JobId.KNIGHT,
	JobId.KNIGHT_B,
	JobId.KNIGHT_H,
	JobId.RUNE_KNIGHT,
	JobId.RUNE_KNIGHT_H,
	JobId.RUNE_KNIGHT_B,
	JobId.RUNE_KNIGHT_2ND,
	JobId.DRAGON_KNIGHT,
	JobId.DRAGON_KNIGHT_RIDING
);
duplicateEntry(
	JobId.KNIGHT2,
	JobId.KNIGHT2_B,
	JobId.KNIGHT2_H,
	JobId.RUNE_KNIGHT2,
	JobId.RUNE_KNIGHT2_H,
	JobId.RUNE_KNIGHT2_B,
	JobId.RUNE_KNIGHT2_2ND,
	JobId.DRAGON_KNIGHT2
);
duplicateEntry(
	JobId.PRIEST,
	JobId.PRIEST_B,
	JobId.PRIEST_H,
	JobId.ARCHBISHOP,
	JobId.ARCHBISHOP_H,
	JobId.ARCHBISHOP_B,
	JobId.ARCHBISHOP_2ND,
	JobId.CARDINAL,
	JobId.CARDINAL_RIDING
);
duplicateEntry(
	JobId.WIZARD,
	JobId.WIZARD_B,
	JobId.WIZARD_H,
	JobId.WARLOCK,
	JobId.WARLOCK_H,
	JobId.WARLOCK_B,
	JobId.WARLOCK_2ND,
	JobId.ARCH_MAGE,
	JobId.ARCH_MAGE_RIDING
);
duplicateEntry(
	JobId.BLACKSMITH,
	JobId.BLACKSMITH_B,
	JobId.BLACKSMITH_H,
	JobId.MECHANIC,
	JobId.MECHANIC_H,
	JobId.MECHANIC_B,
	JobId.MECHANIC2_2ND,
	JobId.MECHANIC_2ND,
	JobId.MEISTER,
	JobId.MEISTER2,
	JobId.MEISTER_RIDING
);
duplicateEntry(
	JobId.HUNTER,
	JobId.HUNTER_B,
	JobId.HUNTER_H,
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
	JobId.ASSASSIN_B,
	JobId.ASSASSIN_H,
	JobId.GUILLOTINE_CROSS,
	JobId.GUILLOTINE_CROSS_H,
	JobId.GUILLOTINE_CROSS_B,
	JobId.GUILLOTINE_CROSS_2ND,
	JobId.SHADOW_CROSS,
	JobId.SHADOW_CROSS_RIDING
);
duplicateEntry(
	JobId.CRUSADER,
	JobId.CRUSADER_B,
	JobId.CRUSADER_H,
	JobId.ROYAL_GUARD,
	JobId.ROYAL_GUARD_H,
	JobId.ROYAL_GUARD_B,
	JobId.ROYAL_GUARD_2ND,
	JobId.IMPERIAL_GUARD,
	JobId.IMPERIAL_GUARD_RIDING
);
duplicateEntry(
	JobId.CRUSADER2,
	JobId.CRUSADER2_B,
	JobId.CRUSADER2_H,
	JobId.ROYAL_GUARD2,
	JobId.ROYAL_GUARD2_H,
	JobId.ROYAL_GUARD2_B,
	JobId.ROYAL_GUARD2_2ND,
	JobId.IMPERIAL_GUARD2
);
duplicateEntry(
	JobId.MONK,
	JobId.MONK_B,
	JobId.MONK_H,
	JobId.SURA,
	JobId.SURA_H,
	JobId.SURA_B,
	JobId.SURA_2ND,
	JobId.INQUISITOR,
	JobId.INQUISITOR_RIDING
);
duplicateEntry(
	JobId.SAGE,
	JobId.SAGE_B,
	JobId.SAGE_H,
	JobId.SORCERER,
	JobId.SORCERER_H,
	JobId.SORCERER_B,
	JobId.SORCERER_2ND,
	JobId.ELEMENTAL_MASTER,
	JobId.ELEMENTAL_MASTER_RIDING
);
duplicateEntry(
	JobId.ROGUE,
	JobId.ROGUE_B,
	JobId.ROGUE_H,
	JobId.SHADOW_CHASER,
	JobId.SHADOW_CHASER_H,
	JobId.SHADOW_CHASER_B,
	JobId.SHADOW_CHASER_2ND,
	JobId.ABYSS_CHASER,
	JobId.ABYSS_CHASER_RIDING
);
duplicateEntry(
	JobId.ALCHEMIST,
	JobId.ALCHEMIST_B,
	JobId.ALCHEMIST_H,
	JobId.GENETIC,
	JobId.GENETIC_H,
	JobId.GENETIC_B,
	JobId.GENETIC_2ND,
	JobId.BIOLO,
	JobId.BIOLO_RIDING
);
duplicateEntry(
	JobId.BARD,
	JobId.BARD_B,
	JobId.BARD_H,
	JobId.MINSTREL,
	JobId.MINSTREL_H,
	JobId.MINSTREL_B,
	JobId.MINSTREL_2ND,
	JobId.TROUBADOUR,
	JobId.TROUBADOUR_RIDING
);
duplicateEntry(
	JobId.DANCER,
	JobId.DANCER_B,
	JobId.DANCER_H,
	JobId.WANDERER,
	JobId.WANDERER_H,
	JobId.WANDERER_B,
	JobId.WANDERER_2ND,
	JobId.TROUVERE,
	JobId.TROUVERE_RIDING
);
duplicateEntry(JobId.SUPERNOVICE, JobId.SUPERNOVICE_B, JobId.SUPERNOVICE2, JobId.SUPERNOVICE2_B, JobId.HYPER_NOVICE);
duplicateEntry(
	JobId.NINJA,
	JobId.NINJA_B,
	JobId.KAGEROU,
	JobId.KAGEROU_B,
	JobId.OBORO,
	JobId.OBORO_B,
	JobId.SHINKIRO,
	JobId.SHIRANUI
);
duplicateEntry(JobId.GUNSLINGER, JobId.GUNSLINGER_B, JobId.REBELLION, JobId.REBELLION_B, JobId.NIGHT_WATCH);
duplicateEntry(JobId.LINKER, JobId.LINKER_B, JobId.SOUL_REAPER, JobId.SOUL_REAPER_B, JobId.SOUL_ASCETIC);

/**
 * Exports
 */
export default WeaponAction;
