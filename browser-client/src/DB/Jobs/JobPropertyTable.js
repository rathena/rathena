/**
 * DB/Jobs/JobPropertyTable.js
 *
 * Job property/flag list for DB Manager
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault, Antares
 */

import JOB from './JobConst.js';

// Job Property Table
const JPT = {};

// INIT with empty just in case something is missed later...
Object.values(JOB).forEach(ID => {
	JPT[ID] = DJP();
});

// Define Job Property
function DJP(
	isHuman,
	isDoram,
	isNoviceClass,
	isFirstClass,
	isSecondClass,
	isThirdClass,
	isFourthClass,
	isRebirth,
	isBaby,
	isExpanded,
	base1stClass,
	base2ndClass,
	base3rdClass,
	base4thClass
) {
	return {
		isHuman: !!isHuman,
		isDoram: !!isDoram,

		isNoviceClass: !!isNoviceClass,

		isFirstClass: !!isFirstClass,
		isSecondClass: !!isSecondClass,
		isThirdClass: !!isThirdClass,
		isFourthClass: !!isFourthClass,

		isRebirth: !!isRebirth,
		isBaby: !!isBaby,
		isExpanded: !!isExpanded,

		base1stClass: base1stClass || null,
		base2ndClass: base2ndClass || null,
		base3rdClass: base3rdClass || null,
		base4thClass: base4thClass || null
	};
}

JPT[JOB.NOVICE] = DJP(1, 0, 1, 0, 0, 0, 0, 0, 0, 0);
JPT[JOB.SWORDMAN] = DJP(1, 0, 0, 1, 0, 0, 0, 0, 0, 0, JOB.SWORDMAN);
JPT[JOB.MAGICIAN] = DJP(1, 0, 0, 1, 0, 0, 0, 0, 0, 0, JOB.MAGICIAN);
JPT[JOB.ARCHER] = DJP(1, 0, 0, 1, 0, 0, 0, 0, 0, 0, JOB.ARCHER);
JPT[JOB.ACOLYTE] = DJP(1, 0, 0, 1, 0, 0, 0, 0, 0, 0, JOB.ACOLYTE);
JPT[JOB.MERCHANT] = DJP(1, 0, 0, 1, 0, 0, 0, 0, 0, 0, JOB.MERCHANT);
JPT[JOB.THIEF] = DJP(1, 0, 0, 1, 0, 0, 0, 0, 0, 0, JOB.THIEF);

JPT[JOB.KNIGHT] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 0, 0, JOB.SWORDMAN, JOB.KNIGHT);
JPT[JOB.PRIEST] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 0, 0, JOB.ACOLYTE, JOB.PRIEST);
JPT[JOB.WIZARD] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 0, 0, JOB.MAGICIAN, JOB.WIZARD);
JPT[JOB.BLACKSMITH] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 0, 0, JOB.MERCHANT, JOB.BLACKSMITH);
JPT[JOB.HUNTER] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 0, 0, JOB.ARCHER, JOB.HUNTER);
JPT[JOB.ASSASSIN] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 0, 0, JOB.THIEF, JOB.ASSASSIN);
JPT[JOB.KNIGHT2] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 0, 0, JOB.SWORDMAN, JOB.KNIGHT);

JPT[JOB.CRUSADER] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 0, 0, JOB.SWORDMAN, JOB.CRUSADER);
JPT[JOB.MONK] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 0, 0, JOB.ACOLYTE, JOB.MONK);
JPT[JOB.SAGE] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 0, 0, JOB.MAGICIAN, JOB.SAGE);
JPT[JOB.ROGUE] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 0, 0, JOB.THIEF, JOB.ROGUE);
JPT[JOB.ALCHEMIST] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 0, 0, JOB.MERCHANT, JOB.ALCHEMIST);
JPT[JOB.BARD] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 0, 0, JOB.ARCHER, JOB.BARD);
JPT[JOB.DANCER] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 0, 0, JOB.ARCHER, JOB.DANCER);
JPT[JOB.CRUSADER2] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 0, 0, JOB.SWORDMAN, JOB.CRUSADER);

JPT[JOB.MARRIED] = DJP(1, 0, 0, 0, 0, 0, 0, 0, 0, 0);

JPT[JOB.SUPERNOVICE] = DJP(1, 0, 1, 1, 0, 0, 0, 0, 0, 1, JOB.SUPERNOVICE);
JPT[JOB.GUNSLINGER] = DJP(1, 0, 0, 1, 0, 0, 0, 0, 0, 1, JOB.GUNSLINGER);
JPT[JOB.NINJA] = DJP(1, 0, 0, 1, 0, 0, 0, 0, 0, 1, JOB.NINJA);

JPT[JOB.XMAS] = DJP(1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
JPT[JOB.SUMMER] = DJP(1, 0, 0, 0, 0, 0, 0, 0, 0, 0);

JPT[JOB.NOVICE_H] = DJP(1, 0, 1, 0, 0, 0, 0, 1, 0, 0);
JPT[JOB.SWORDMAN_H] = DJP(1, 0, 0, 1, 0, 0, 0, 1, 0, 0, JOB.SWORDMAN);
JPT[JOB.MAGICIAN_H] = DJP(1, 0, 0, 1, 0, 0, 0, 1, 0, 0, JOB.MAGICIAN);
JPT[JOB.ARCHER_H] = DJP(1, 0, 0, 1, 0, 0, 0, 1, 0, 0, JOB.ARCHER);
JPT[JOB.ACOLYTE_H] = DJP(1, 0, 0, 1, 0, 0, 0, 1, 0, 0, JOB.ACOLYTE);
JPT[JOB.MERCHANT_H] = DJP(1, 0, 0, 1, 0, 0, 0, 1, 0, 0, JOB.MERCHANT);
JPT[JOB.THIEF_H] = DJP(1, 0, 0, 1, 0, 0, 0, 1, 0, 0, JOB.THIEF);

JPT[JOB.KNIGHT_H] = DJP(1, 0, 0, 0, 1, 0, 0, 1, 0, 0, JOB.SWORDMAN, JOB.KNIGHT);
JPT[JOB.PRIEST_H] = DJP(1, 0, 0, 0, 1, 0, 0, 1, 0, 0, JOB.ACOLYTE, JOB.PRIEST);
JPT[JOB.WIZARD_H] = DJP(1, 0, 0, 0, 1, 0, 0, 1, 0, 0, JOB.MAGICIAN, JOB.WIZARD);
JPT[JOB.BLACKSMITH_H] = DJP(1, 0, 0, 0, 1, 0, 0, 1, 0, 0, JOB.MERCHANT, JOB.BLACKSMITH);
JPT[JOB.HUNTER_H] = DJP(1, 0, 0, 0, 1, 0, 0, 1, 0, 0, JOB.ARCHER, JOB.HUNTER);
JPT[JOB.ASSASSIN_H] = DJP(1, 0, 0, 0, 1, 0, 0, 1, 0, 0, JOB.THIEF, JOB.ASSASSIN);
JPT[JOB.KNIGHT2_H] = DJP(1, 0, 0, 0, 1, 0, 0, 1, 0, 0, JOB.SWORDMAN, JOB.KNIGHT);

JPT[JOB.CRUSADER_H] = DJP(1, 0, 0, 0, 1, 0, 0, 1, 0, 0, JOB.SWORDMAN, JOB.CRUSADER);
JPT[JOB.MONK_H] = DJP(1, 0, 0, 0, 1, 0, 0, 1, 0, 0, JOB.ACOLYTE, JOB.MONK);
JPT[JOB.SAGE_H] = DJP(1, 0, 0, 0, 1, 0, 0, 1, 0, 0, JOB.MAGICIAN, JOB.SAGE);
JPT[JOB.ROGUE_H] = DJP(1, 0, 0, 0, 1, 0, 0, 1, 0, 0, JOB.THIEF, JOB.ROGUE);
JPT[JOB.ALCHEMIST_H] = DJP(1, 0, 0, 0, 1, 0, 0, 1, 0, 0, JOB.MERCHANT, JOB.ALCHEMIST);
JPT[JOB.BARD_H] = DJP(1, 0, 0, 0, 1, 0, 0, 1, 0, 0, JOB.ARCHER, JOB.BARD);
JPT[JOB.DANCER_H] = DJP(1, 0, 0, 0, 1, 0, 0, 1, 0, 0, JOB.ARCHER, JOB.DANCER);
JPT[JOB.CRUSADER2_H] = DJP(1, 0, 0, 0, 1, 0, 0, 1, 0, 0, JOB.SWORDMAN, JOB.CRUSADER);

JPT[JOB.NOVICE_B] = DJP(1, 0, 1, 0, 0, 0, 0, 0, 1, 0);
JPT[JOB.SWORDMAN_B] = DJP(1, 0, 0, 1, 0, 0, 0, 0, 1, 0, JOB.SWORDMAN);
JPT[JOB.MAGICIAN_B] = DJP(1, 0, 0, 1, 0, 0, 0, 0, 1, 0, JOB.MAGICIAN);
JPT[JOB.ARCHER_B] = DJP(1, 0, 0, 1, 0, 0, 0, 0, 1, 0, JOB.ARCHER);
JPT[JOB.ACOLYTE_B] = DJP(1, 0, 0, 1, 0, 0, 0, 0, 1, 0, JOB.ACOLYTE);
JPT[JOB.MERCHANT_B] = DJP(1, 0, 0, 1, 0, 0, 0, 0, 1, 0, JOB.MERCHANT);
JPT[JOB.THIEF_B] = DJP(1, 0, 0, 1, 0, 0, 0, 0, 1, 0, JOB.THIEF);

JPT[JOB.KNIGHT_B] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, JOB.SWORDMAN, JOB.KNIGHT);
JPT[JOB.PRIEST_B] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, JOB.ACOLYTE, JOB.PRIEST);
JPT[JOB.WIZARD_B] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, JOB.MAGICIAN, JOB.WIZARD);
JPT[JOB.BLACKSMITH_B] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, JOB.MERCHANT, JOB.BLACKSMITH);
JPT[JOB.HUNTER_B] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, JOB.ARCHER, JOB.HUNTER);
JPT[JOB.ASSASSIN_B] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, JOB.THIEF, JOB.ASSASSIN);
JPT[JOB.KNIGHT2_B] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, JOB.SWORDMAN, JOB.KNIGHT);

JPT[JOB.CRUSADER_B] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, JOB.SWORDMAN, JOB.CRUSADER);
JPT[JOB.MONK_B] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, JOB.ACOLYTE, JOB.MONK);
JPT[JOB.SAGE_B] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, JOB.MAGICIAN, JOB.SAGE);
JPT[JOB.ROGUE_B] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, JOB.THIEF, JOB.ROGUE);
JPT[JOB.ALCHEMIST_B] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, JOB.MERCHANT, JOB.ALCHEMIST);
JPT[JOB.BARD_B] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, JOB.ARCHER, JOB.BARD);
JPT[JOB.DANCER_B] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, JOB.ARCHER, JOB.DANCER);
JPT[JOB.CRUSADER2_B] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, JOB.SWORDMAN, JOB.CRUSADER);

JPT[JOB.SUPERNOVICE_B] = DJP(1, 0, 1, 1, 0, 0, 0, 0, 1, 1, JOB.SUPERNOVICE);

JPT[JOB.TAEKWON] = DJP(1, 0, 0, 1, 0, 0, 0, 0, 0, 1, JOB.TAEKWON);
JPT[JOB.STAR] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 0, 1, JOB.TAEKWON, JOB.STAR);
JPT[JOB.STAR2] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 0, 1, JOB.TAEKWON, JOB.STAR);
JPT[JOB.LINKER] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 0, 1, JOB.TAEKWON, JOB.LINKER);
JPT[JOB.GANGSI] = DJP(1, 0, 0, 1, 0, 0, 0, 0, 0, 1, JOB.GANGSI);
JPT[JOB.DEATHKNIGHT] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 0, 1, JOB.GANGSI, JOB.DEATHKNIGHT);
JPT[JOB.COLLECTOR] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 0, 1, JOB.GANGSI, JOB.COLLECTOR);

JPT[JOB.RUNE_KNIGHT] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, JOB.SWORDMAN, JOB.KNIGHT, JOB.RUNE_KNIGHT);
JPT[JOB.WARLOCK] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, JOB.MAGICIAN, JOB.WIZARD, JOB.WARLOCK);
JPT[JOB.RANGER] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, JOB.ARCHER, JOB.HUNTER, JOB.RANGER);
JPT[JOB.ARCHBISHOP] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, JOB.ACOLYTE, JOB.PRIEST, JOB.ARCHBISHOP);
JPT[JOB.MECHANIC] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, JOB.MERCHANT, JOB.BLACKSMITH, JOB.MECHANIC);
JPT[JOB.GUILLOTINE_CROSS] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, JOB.THIEF, JOB.ASSASSIN, JOB.GUILLOTINE_CROSS);

JPT[JOB.RUNE_KNIGHT_H] = DJP(1, 0, 0, 0, 0, 1, 0, 1, 0, 0, JOB.SWORDMAN, JOB.KNIGHT, JOB.RUNE_KNIGHT);
JPT[JOB.WARLOCK_H] = DJP(1, 0, 0, 0, 0, 1, 0, 1, 0, 0, JOB.MAGICIAN, JOB.WIZARD, JOB.WARLOCK);
JPT[JOB.RANGER_H] = DJP(1, 0, 0, 0, 0, 1, 0, 1, 0, 0, JOB.ARCHER, JOB.HUNTER, JOB.RANGER);
JPT[JOB.ARCHBISHOP_H] = DJP(1, 0, 0, 0, 0, 1, 0, 1, 0, 0, JOB.ACOLYTE, JOB.PRIEST, JOB.ARCHBISHOP);
JPT[JOB.MECHANIC_H] = DJP(1, 0, 0, 0, 0, 1, 0, 1, 0, 0, JOB.MERCHANT, JOB.BLACKSMITH, JOB.MECHANIC);
JPT[JOB.GUILLOTINE_CROSS_H] = DJP(1, 0, 0, 0, 0, 1, 0, 1, 0, 0, JOB.THIEF, JOB.ASSASSIN, JOB.GUILLOTINE_CROSS);

JPT[JOB.ROYAL_GUARD] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, JOB.SWORDMAN, JOB.CRUSADER, JOB.ROYAL_GUARD);
JPT[JOB.SORCERER] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, JOB.MAGICIAN, JOB.SAGE, JOB.SORCERER);
JPT[JOB.MINSTREL] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, JOB.ARCHER, JOB.BARD, JOB.MINSTREL);
JPT[JOB.WANDERER] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, JOB.ARCHER, JOB.DANCER, JOB.WANDERER);
JPT[JOB.SURA] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, JOB.ACOLYTE, JOB.MONK, JOB.SURA);
JPT[JOB.GENETIC] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, JOB.MERCHANT, JOB.ALCHEMIST, JOB.GENETIC);
JPT[JOB.SHADOW_CHASER] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, JOB.THIEF, JOB.ROGUE, JOB.SHADOW_CHASER);

JPT[JOB.ROYAL_GUARD_H] = DJP(1, 0, 0, 0, 0, 1, 0, 1, 0, 0, JOB.SWORDMAN, JOB.CRUSADER, JOB.ROYAL_GUARD);
JPT[JOB.SORCERER_H] = DJP(1, 0, 0, 0, 0, 1, 0, 1, 0, 0, JOB.MAGICIAN, JOB.SAGE, JOB.SORCERER);
JPT[JOB.MINSTREL_H] = DJP(1, 0, 0, 0, 0, 1, 0, 1, 0, 0, JOB.ARCHER, JOB.BARD, JOB.MINSTREL);
JPT[JOB.WANDERER_H] = DJP(1, 0, 0, 0, 0, 1, 0, 1, 0, 0, JOB.ARCHER, JOB.DANCER, JOB.WANDERER);
JPT[JOB.SURA_H] = DJP(1, 0, 0, 0, 0, 1, 0, 1, 0, 0, JOB.ACOLYTE, JOB.MONK, JOB.SURA);
JPT[JOB.GENETIC_H] = DJP(1, 0, 0, 0, 0, 1, 0, 1, 0, 0, JOB.MERCHANT, JOB.ALCHEMIST, JOB.GENETIC);
JPT[JOB.SHADOW_CHASER_H] = DJP(1, 0, 0, 0, 0, 1, 0, 1, 0, 0, JOB.THIEF, JOB.ROGUE, JOB.SHADOW_CHASER);

JPT[JOB.RUNE_KNIGHT2] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, JOB.SWORDMAN, JOB.KNIGHT, JOB.RUNE_KNIGHT);
JPT[JOB.RUNE_KNIGHT2_H] = DJP(1, 0, 0, 0, 0, 1, 0, 1, 0, 0, JOB.SWORDMAN, JOB.KNIGHT, JOB.RUNE_KNIGHT);
JPT[JOB.ROYAL_GUARD2] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, JOB.SWORDMAN, JOB.CRUSADER, JOB.ROYAL_GUARD);
JPT[JOB.ROYAL_GUARD2_H] = DJP(1, 0, 0, 0, 0, 1, 0, 1, 0, 0, JOB.SWORDMAN, JOB.CRUSADER, JOB.ROYAL_GUARD);
JPT[JOB.RANGER2] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, JOB.ARCHER, JOB.HUNTER, JOB.RANGER);
JPT[JOB.RANGER2_H] = DJP(1, 0, 0, 0, 0, 1, 0, 1, 0, 0, JOB.ARCHER, JOB.HUNTER, JOB.RANGER);
JPT[JOB.MECHANIC2] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, JOB.MERCHANT, JOB.BLACKSMITH, JOB.MECHANIC);
JPT[JOB.MECHANIC2_H] = DJP(1, 0, 0, 0, 0, 1, 0, 1, 0, 0, JOB.MERCHANT, JOB.BLACKSMITH, JOB.MECHANIC);

JPT[JOB.RUNE_KNIGHT_B] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 1, 0, JOB.SWORDMAN, JOB.KNIGHT, JOB.RUNE_KNIGHT);
JPT[JOB.WARLOCK_B] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 1, 0, JOB.MAGICIAN, JOB.WIZARD, JOB.WARLOCK);
JPT[JOB.RANGER_B] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 1, 0, JOB.ARCHER, JOB.HUNTER, JOB.RANGER);
JPT[JOB.ARCHBISHOP_B] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 1, 0, JOB.ACOLYTE, JOB.PRIEST, JOB.ARCHBISHOP);
JPT[JOB.MECHANIC_B] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 1, 0, JOB.MERCHANT, JOB.BLACKSMITH, JOB.MECHANIC);
JPT[JOB.GUILLOTINE_CROSS_B] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 1, 0, JOB.THIEF, JOB.ASSASSIN, JOB.GUILLOTINE_CROSS);

JPT[JOB.ROYAL_GUARD_B] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 1, 0, JOB.SWORDMAN, JOB.CRUSADER, JOB.ROYAL_GUARD);
JPT[JOB.SORCERER_B] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 1, 0, JOB.MAGICIAN, JOB.SAGE, JOB.SORCERER);
JPT[JOB.MINSTREL_B] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 1, 0, JOB.ARCHER, JOB.BARD, JOB.MINSTREL);
JPT[JOB.WANDERER_B] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 1, 0, JOB.ARCHER, JOB.DANCER, JOB.WANDERER);
JPT[JOB.SURA_B] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 1, 0, JOB.ACOLYTE, JOB.MONK, JOB.SURA);
JPT[JOB.GENETIC_B] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 1, 0, JOB.MERCHANT, JOB.ALCHEMIST, JOB.GENETIC);
JPT[JOB.SHADOW_CHASER_B] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 1, 0, JOB.THIEF, JOB.ROGUE, JOB.SHADOW_CHASER);

JPT[JOB.RUNE_KNIGHT2_B] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 1, 0, JOB.SWORDMAN, JOB.KNIGHT, JOB.RUNE_KNIGHT);
JPT[JOB.ROYAL_GUARD2_B] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 1, 0, JOB.SWORDMAN, JOB.CRUSADER, JOB.ROYAL_GUARD);
JPT[JOB.RANGER2_B] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 1, 0, JOB.ARCHER, JOB.HUNTER, JOB.RANGER);
JPT[JOB.MECHANIC2_B] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 1, 0, JOB.MERCHANT, JOB.BLACKSMITH, JOB.MECHANIC);

// Mounts... No way I'm making these
/*
	JPT[ JOB.FROG_NINJA ] =         DJP(1,0,	0,	1,0,0,0, 0,0,1, JOB.NINJA, null, null, null);
	JPT[ JOB.PECO_GUNNER ] =        DJP(1,0,	0,	1,0,0,0, 0,0,1, JOB.GUNSLINGER, null, null, null);
	JPT[ JOB.PECO_SWORD ] =         DJP(1,0,	0,	1,0,0,0, 0,0,0, JOB.SWORDMAN, null, null, null);
	JPT[ JOB.FROG_LINKER ] =        DJP(1,0,	0,	0,1,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.PIG_WHITESMITH ] =     DJP(1,0,	0,	0,1,0,0, 1,0,0, null, null, null, null);
	JPT[ JOB.PIG_MERCHANT ] =       DJP(1,0,	0,	1,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.PIG_GENETIC ] =        DJP(1,0,	0,	0,0,1,0, 0,0,0, null, null, null, null);
	JPT[ JOB.PIG_CREATOR ] =        DJP(1,0,	0,	0,1,0,0, 1,0,0, null, null, null, null);
	JPT[ JOB.OSTRICH_ARCHER ] =     DJP(1,0,	0,	1,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.PORING_STAR ] =        DJP(1,0,	0,	1,0,0,0, 0,0,1, null, null, null, null);
	JPT[ JOB.PORING_NOVICE ] =      DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.SHEEP_MONK ] =         DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.SHEEP_ACO ] =          DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.SHEEP_SURA ] =         DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.PORING_SNOVICE ] =     DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.SHEEP_ARCB ] =         DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.FOX_MAGICIAN ] =       DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.FOX_SAGE ] =           DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.FOX_SORCERER ] =       DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.FOX_WARLOCK ] =        DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.FOX_WIZ ] =            DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.FOX_PROF ] =           DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.FOX_HWIZ ] =           DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.PIG_ALCHE ] =          DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.PIG_BLACKSMITH ] =     DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.SHEEP_CHAMP ] =        DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.DOG_G_CROSS ] =        DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.DOG_THIEF ] =          DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.DOG_ROGUE ] =          DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.DOG_CHASER ] =         DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.DOG_STALKER ] =        DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.DOG_ASSASSIN ] =       DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.DOG_ASSA_X ] =         DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.OSTRICH_DANCER ] =     DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.OSTRICH_MINSTREL ] =   DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.OSTRICH_BARD ] =       DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.OSTRICH_SNIPER ] =     DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.OSTRICH_WANDER ] =     DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.OSTRICH_ZIPSI ] =      DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.OSTRICH_CROWN ] =      DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.OSTRICH_HUNTER ] =     DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.PORING_TAEKWON ] =     DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.SHEEP_PRIEST ] =       DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.SHEEP_HPRIEST ] =      DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.PORING_NOVICE_B ] =    DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.PECO_SWORD_B ] =       DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.FOX_MAGICIAN_B ] =     DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.OSTRICH_ARCHER_B ] =   DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.SHEEP_ACO_B ] =        DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.PIG_MERCHANT_B ] =     DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.OSTRICH_HUNTER_B ] =   DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.DOG_ASSASSIN_B ] =     DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.SHEEP_MONK_B ] =       DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.FOX_SAGE_B ] =         DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.DOG_ROGUE_B ] =        DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.PIG_ALCHE_B ] =        DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.OSTRICH_BARD_B ] =     DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.OSTRICH_DANCER_B ] =   DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.PORING_SNOVICE_B ] =   DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.FOX_WARLOCK_B ] =      DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.SHEEP_ARCB_B ] =       DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.DOG_G_CROSS_B ] =      DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.FOX_SORCERER_B ] =     DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.OSTRICH_MINSTREL_B ] = DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.OSTRICH_WANDER_B ] =   DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.SHEEP_SURA_B ] =       DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.PIG_GENETIC_B ] =      DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.DOG_THIEF_B ] =        DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.DOG_CHASER_B ] =       DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.PORING_NOVICE_H ] =    DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.PECO_SWORD_H ] =       DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.FOX_MAGICIAN_H ] =     DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.OSTRICH_ARCHER_H ] =   DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.SHEEP_ACO_H ] =        DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.PIG_MERCHANT_H ] =     DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.DOG_THIEF_H ] =        DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	*/

JPT[JOB.SUPERNOVICE2] = DJP(1, 0, 1, 0, 0, 0, 0, 0, 0, 0, JOB.SUPERNOVICE, JOB.SUPERNOVICE2);
JPT[JOB.SUPERNOVICE2_B] = DJP(1, 0, 1, 0, 0, 0, 0, 0, 1, 0, JOB.SUPERNOVICE, JOB.SUPERNOVICE2);

/*
	JPT[ JOB.PORING_SNOVICE2 ] =    DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.PORING_SNOVICE2_B ] =  DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.SHEEP_PRIEST_B ] =     DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.FOX_WIZ_B ] =          DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.PIG_BLACKSMITH_B ] =   DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.PIG_MECHANIC ] =       DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.OSTRICH_RANGER ] =     DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.LION_KNIGHT ] =        DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.LION_KNIGHT_H ] =      DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.LION_ROYAL_GUARD ] =   DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.LION_RUNE_KNIGHT ] =   DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.LION_CRUSADER ] =      DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.LION_CRUSADER_H ] =    DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.PIG_MECHANIC_B ] =     DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.OSTRICH_RANGER_B ] =   DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.LION_KNIGHT_B ] =      DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.LION_ROYAL_GUARD_B ] = DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.LION_RUNE_KNIGHT_B ] = DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.LION_CRUSADER_B ] =    DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	*/

JPT[JOB.KAGEROU] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 0, 1, JOB.NINJA);
JPT[JOB.OBORO] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 0, 1, JOB.NINJA);

/*
	JPT[ JOB.FROG_KAGEROU ] =       DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	JPT[ JOB.FROG_OBORO ] =         DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	*/

JPT[JOB.REBELLION] = DJP(1, 0, 0, 0, 0, 0, 0, 0, 0, 1, JOB.GUNSLINGER);

/*
	JPT[ JOB.PECO_REBELLION ] =     DJP(1,0,	0,	0,0,0,0, 0,0,0, null, null, null, null);
	*/

JPT[JOB.DO_SUMMONER] = DJP(0, 1, 0, 1, 0, 0, 0, 0, 0, 0);
JPT[JOB.DO_SUMMONER_B] = DJP(0, 1, 0, 1, 0, 0, 0, 0, 1, 0);

JPT[JOB.NINJA_B] = DJP(1, 0, 0, 1, 0, 0, 0, 0, 1, 1, JOB.NINJA);
JPT[JOB.KAGEROU_B] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 1, 1, JOB.NINJA, JOB.KAGEROU);
JPT[JOB.OBORO_B] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 1, 1, JOB.NINJA, JOB.OBORO);
JPT[JOB.TAEKWON_B] = DJP(1, 0, 0, 1, 0, 0, 0, 0, 1, 1, JOB.TAEKWON);
JPT[JOB.STAR_B] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 1, 1, JOB.TAEKWON, JOB.STAR);
JPT[JOB.LINKER_B] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 1, 1, JOB.TAEKWON, JOB.LINKER);
JPT[JOB.GUNSLINGER_B] = DJP(1, 0, 0, 1, 0, 0, 0, 0, 1, 1, JOB.GUNSLINGER);
JPT[JOB.REBELLION_B] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 1, 1, JOB.GUNSLINGER, JOB.REBELLION);
JPT[JOB.STAR2_B] = DJP(1, 0, 0, 0, 1, 0, 0, 0, 1, 1, JOB.TAEKWON, JOB.STAR);

JPT[JOB.STAR_EMPEROR] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 0, 1, JOB.TAEKWON, JOB.STAR, JOB.STAR_EMPEROR);
JPT[JOB.SOUL_REAPER] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 0, 1, JOB.TAEKWON, JOB.LINKER, JOB.SOUL_REAPER);
JPT[JOB.STAR_EMPEROR_B] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 1, 1, JOB.TAEKWON, JOB.STAR, JOB.STAR_EMPEROR);
JPT[JOB.SOUL_REAPER_B] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 1, 1, JOB.TAEKWON, JOB.LINKER, JOB.SOUL_REAPER);

JPT[JOB.STAR_EMPEROR2] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 0, 1, JOB.TAEKWON, JOB.STAR, JOB.STAR_EMPEROR);
JPT[JOB.SOUL_REAPER2] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 1, 1, JOB.TAEKWON, JOB.LINKER, JOB.SOUL_REAPER);
JPT[JOB.STAR_EMPEROR2_B] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 1, 1, JOB.TAEKWON, JOB.STAR, JOB.STAR_EMPEROR);
JPT[JOB.SOUL_REAPER2_B] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 1, 1, JOB.TAEKWON, JOB.LINKER, JOB.SOUL_REAPER);

JPT[JOB.DRAGON_KNIGHT] = DJP(
	1,
	0,
	0,
	0,
	0,
	0,
	1,
	0,
	0,
	0,
	JOB.SWORDMAN,
	JOB.KNIGHT,
	JOB.RUNE_KNIGHT,
	JOB.DRAGON_KNIGHT
);
JPT[JOB.MEISTER] = DJP(1, 0, 0, 0, 0, 0, 1, 0, 0, 0, JOB.MERCHANT, JOB.BLACKSMITH, JOB.MECHANIC, JOB.MEISTER);
JPT[JOB.SHADOW_CROSS] = DJP(
	1,
	0,
	0,
	0,
	0,
	0,
	1,
	0,
	0,
	0,
	JOB.THIEF,
	JOB.ASSASSIN,
	JOB.GUILLOTINE_CROSS,
	JOB.SHADOW_CROSS
);
JPT[JOB.ARCH_MAGE] = DJP(1, 0, 0, 0, 0, 0, 1, 0, 0, 0, JOB.MAGICIAN, JOB.WIZARD, JOB.WARLOCK, JOB.ARCH_MAGE);
JPT[JOB.CARDINAL] = DJP(1, 0, 0, 0, 0, 0, 1, 0, 0, 0, JOB.ACOLYTE, JOB.PRIEST, JOB.ARCHBISHOP, JOB.CARDINAL);
JPT[JOB.WINDHAWK] = DJP(1, 0, 0, 0, 0, 0, 1, 0, 0, 0, JOB.ARCHER, JOB.HUNTER, JOB.RANGER, JOB.WINDHAWK);

JPT[JOB.IMPERIAL_GUARD] = DJP(
	1,
	0,
	0,
	0,
	0,
	0,
	1,
	0,
	0,
	0,
	JOB.SWORDMAN,
	JOB.CRUSADER,
	JOB.ROYAL_GUARD,
	JOB.IMPERIAL_GUARD
);
JPT[JOB.BIOLO] = DJP(1, 0, 0, 0, 0, 0, 1, 0, 0, 0, JOB.MERCHANT, JOB.ALCHEMIST, JOB.GENETIC, JOB.BIOLO);
JPT[JOB.ABYSS_CHASER] = DJP(1, 0, 0, 0, 0, 0, 1, 0, 0, 0, JOB.THIEF, JOB.ROGUE, JOB.SHADOW_CHASER, JOB.ABYSS_CHASER);
JPT[JOB.ELEMENTAL_MASTER] = DJP(
	1,
	0,
	0,
	0,
	0,
	0,
	1,
	0,
	0,
	0,
	JOB.MAGICIAN,
	JOB.SAGE,
	JOB.SORCERER,
	JOB.ELEMENTAL_MASTER
);
JPT[JOB.INQUISITOR] = DJP(1, 0, 0, 0, 0, 0, 1, 0, 0, 0, JOB.ACOLYTE, JOB.MONK, JOB.SURA, JOB.INQUISITOR);
JPT[JOB.TROUBADOUR] = DJP(1, 0, 0, 0, 0, 0, 1, 0, 0, 0, JOB.ARCHER, JOB.BARD, JOB.MINSTREL, JOB.TROUBADOUR);
JPT[JOB.TROUVERE] = DJP(1, 0, 0, 0, 0, 0, 1, 0, 0, 0, JOB.ARCHER, JOB.DANCER, JOB.WANDERER, JOB.TROUVERE);

JPT[JOB.WINDHAWK2] = DJP(1, 0, 0, 0, 0, 0, 1, 0, 0, 0, JOB.ARCHER, JOB.HUNTER, JOB.RANGER, JOB.WINDHAWK);
JPT[JOB.MEISTER2] = DJP(1, 0, 0, 0, 0, 0, 1, 0, 0, 0, JOB.MERCHANT, JOB.BLACKSMITH, JOB.MECHANIC, JOB.MEISTER);
JPT[JOB.DRAGON_KNIGHT2] = DJP(
	1,
	0,
	0,
	0,
	0,
	0,
	1,
	0,
	0,
	0,
	JOB.SWORDMAN,
	JOB.KNIGHT,
	JOB.RUNE_KNIGHT,
	JOB.DRAGON_KNIGHT
);
JPT[JOB.IMPERIAL_GUARD2] = DJP(
	1,
	0,
	0,
	0,
	0,
	0,
	1,
	0,
	0,
	0,
	JOB.SWORDMAN,
	JOB.CRUSADER,
	JOB.ROYAL_GUARD,
	JOB.IMPERIAL_GUARD
);

JPT[JOB.SKY_EMPEROR] = DJP(1, 0, 0, 0, 0, 0, 1, 0, 0, 1, JOB.TAEKWON, JOB.STAR, JOB.STAR_EMPEROR, JOB.SKY_EMPEROR);
JPT[JOB.SOUL_ASCETIC] = DJP(1, 0, 0, 0, 0, 0, 1, 0, 0, 1, JOB.TAEKWON, JOB.LINKER, JOB.SOUL_REAPER, JOB.SOUL_ASCETIC);
JPT[JOB.SHINKIRO] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 0, 1, JOB.NINJA, JOB.KAGEROU, JOB.SHINKIRO);
JPT[JOB.SHIRANUI] = DJP(1, 0, 0, 0, 0, 1, 0, 0, 0, 1, JOB.NINJA, JOB.OBORO, JOB.SHIRANUI);
JPT[JOB.NIGHT_WATCH] = DJP(1, 0, 0, 0, 0, 0, 1, 0, 0, 1, JOB.GUNSLINGER, JOB.REBELLION, JOB.NIGHT_WATCH);
JPT[JOB.HYPER_NOVICE] = DJP(1, 0, 1, 0, 0, 1, 0, 0, 0, 1, JOB.SUPERNOVICE, JOB.SUPERNOVICE2, JOB.HYPER_NOVICE);
JPT[JOB.SPIRIT_HANDLER] = DJP(0, 1, 0, 0, 1, 0, 0, 0, 0, 0, JOB.DO_SUMMONER, JOB.SPIRIT_HANDLER);
JPT[JOB.SKY_EMPEROR2] = DJP(1, 0, 0, 0, 0, 0, 1, 0, 0, 1, JOB.TAEKWON, JOB.STAR, JOB.STAR_EMPEROR, JOB.SKY_EMPEROR);

export default JPT;
