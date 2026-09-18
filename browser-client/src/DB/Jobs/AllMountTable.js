/**
 * DB/Jobs/AllMountTable.js
 *
 * Look up table <job> => <job mount>
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Antares
 */

import JobId from './JobConst.js';

const AllMountTable = {};

//REGULAR
AllMountTable[JobId.NOVICE] = JobId.PORING_NOVICE;

AllMountTable[JobId.ACOLYTE] = JobId.SHEEP_ACO;
AllMountTable[JobId.ARCHER] = JobId.OSTRICH_ARCHER;
AllMountTable[JobId.MAGICIAN] = JobId.FOX_MAGICIAN;
AllMountTable[JobId.MERCHANT] = JobId.PIG_MERCHANT;
AllMountTable[JobId.SWORDMAN] = JobId.PECO_SWORD;
AllMountTable[JobId.THIEF] = JobId.DOG_THIEF;

AllMountTable[JobId.PRIEST] = JobId.SHEEP_PRIEST;
AllMountTable[JobId.HUNTER] = JobId.OSTRICH_HUNTER;
AllMountTable[JobId.WIZARD] = JobId.FOX_WIZ;
AllMountTable[JobId.BLACKSMITH] = JobId.PIG_BLACKSMITH;
AllMountTable[JobId.KNIGHT] = JobId.LION_KNIGHT;
AllMountTable[JobId.ASSASSIN] = JobId.DOG_ASSASSIN;

AllMountTable[JobId.MONK] = JobId.SHEEP_MONK;
AllMountTable[JobId.BARD] = JobId.OSTRICH_BARD;
AllMountTable[JobId.DANCER] = JobId.OSTRICH_DANCER;
AllMountTable[JobId.SAGE] = JobId.FOX_SAGE;
AllMountTable[JobId.ALCHEMIST] = JobId.PIG_ALCHE;
AllMountTable[JobId.CRUSADER] = JobId.LION_CRUSADER;
AllMountTable[JobId.ROGUE] = JobId.DOG_ROGUE;

AllMountTable[JobId.ARCHBISHOP] = JobId.SHEEP_ARCB;
AllMountTable[JobId.RANGER] = JobId.OSTRICH_RANGER;
AllMountTable[JobId.WARLOCK] = JobId.FOX_WARLOCK;
AllMountTable[JobId.MECHANIC] = JobId.PIG_MECHANIC;
AllMountTable[JobId.RUNE_KNIGHT] = JobId.LION_RUNE_KNIGHT;
AllMountTable[JobId.GUILLOTINE_CROSS] = JobId.DOG_G_CROSS;

AllMountTable[JobId.SURA] = JobId.SHEEP_SURA;
AllMountTable[JobId.MINSTREL] = JobId.OSTRICH_MINSTREL;
AllMountTable[JobId.WANDERER] = JobId.OSTRICH_WANDER;
AllMountTable[JobId.SORCERER] = JobId.FOX_SORCERER;
AllMountTable[JobId.GENETIC] = JobId.PIG_GENETIC;
AllMountTable[JobId.ROYAL_GUARD] = JobId.LION_ROYAL_GUARD;
AllMountTable[JobId.SHADOW_CHASER] = JobId.DOG_CHASER;

AllMountTable[JobId.SUPERNOVICE] = JobId.PORING_SNOVICE;

AllMountTable[JobId.NINJA] = JobId.FROG_NINJA;
AllMountTable[JobId.GUNSLINGER] = JobId.PECO_GUNNER;
AllMountTable[JobId.TAEKWON] = JobId.PORING_TAEKWON;

AllMountTable[JobId.STAR] = JobId.PORING_STAR;
AllMountTable[JobId.LINKER] = JobId.FROG_LINKER;

AllMountTable[JobId.SUPERNOVICE2] = JobId.PORING_SNOVICE2;
AllMountTable[JobId.KAGEROU] = JobId.FROG_KAGEROU;
AllMountTable[JobId.OBORO] = JobId.FROG_OBORO;
AllMountTable[JobId.REBELLION] = JobId.PECO_REBELLION;
AllMountTable[JobId.SOUL_REAPER] = JobId.SOUL_REAPER2;
AllMountTable[JobId.STAR_EMPEROR] = JobId.STAR_EMPEROR2;

AllMountTable[JobId.DRAGON_KNIGHT] = JobId.DRAGON_KNIGHT_RIDING;
AllMountTable[JobId.MEISTER] = JobId.MEISTER_RIDING;
AllMountTable[JobId.SHADOW_CROSS] = JobId.SHADOW_CROSS_RIDING;
AllMountTable[JobId.ARCH_MAGE] = JobId.ARCH_MAGE_RIDING;
AllMountTable[JobId.CARDINAL] = JobId.CARDINAL_RIDING;
AllMountTable[JobId.WINDHAWK] = JobId.WINDHAWK_RIDING;
AllMountTable[JobId.IMPERIAL_GUARD] = JobId.IMPERIAL_GUARD_RIDING;
AllMountTable[JobId.BIOLO] = JobId.BIOLO_RIDING;
AllMountTable[JobId.ABYSS_CHASER] = JobId.ABYSS_CHASER_RIDING;
AllMountTable[JobId.ELEMENTAL_MASTER] = JobId.ELEMENTAL_MASTER_RIDING;
AllMountTable[JobId.INQUISITOR] = JobId.INQUISITOR_RIDING;
AllMountTable[JobId.TROUBADOUR] = JobId.TROUBADOUR_RIDING;
AllMountTable[JobId.TROUVERE] = JobId.TROUVERE_RIDING;

AllMountTable[JobId.DRUID] = JobId.DRUID_RIDING;
AllMountTable[JobId.ALITEA] = JobId.ALITEA_RIDING;
AllMountTable[JobId.KARNOS] = JobId.KARNOS_RIDING;

//REBIRTH
AllMountTable[JobId.NOVICE_H] = JobId.PORING_NOVICE_H;

AllMountTable[JobId.ACOLYTE_H] = JobId.SHEEP_ACO_H;
AllMountTable[JobId.ARCHER_H] = JobId.OSTRICH_ARCHER_H;
AllMountTable[JobId.MAGICIAN_H] = JobId.FOX_MAGICIAN_H;
AllMountTable[JobId.MERCHANT_H] = JobId.PIG_MERCHANT_H;
AllMountTable[JobId.SWORDMAN_H] = JobId.PECO_SWORD_H;
AllMountTable[JobId.THIEF_H] = JobId.DOG_THIEF_H;

AllMountTable[JobId.PRIEST_H] = JobId.SHEEP_HPRIEST;
AllMountTable[JobId.HUNTER_H] = JobId.OSTRICH_SNIPER;
AllMountTable[JobId.WIZARD_H] = JobId.FOX_HWIZ;
AllMountTable[JobId.BLACKSMITH_H] = JobId.PIG_WHITESMITH;
AllMountTable[JobId.KNIGHT_H] = JobId.LION_KNIGHT_H;
AllMountTable[JobId.ASSASSIN_H] = JobId.DOG_ASSA_X;

AllMountTable[JobId.MONK_H] = JobId.SHEEP_CHAMP;
AllMountTable[JobId.BARD_H] = JobId.OSTRICH_CROWN;
AllMountTable[JobId.DANCER_H] = JobId.OSTRICH_ZIPSI;
AllMountTable[JobId.SAGE_H] = JobId.FOX_PROF;
AllMountTable[JobId.ALCHEMIST_H] = JobId.PIG_CREATOR;
AllMountTable[JobId.CRUSADER_H] = JobId.LION_CRUSADER_H;
AllMountTable[JobId.ROGUE_H] = JobId.DOG_STALKER;

//BABY
AllMountTable[JobId.NOVICE_B] = JobId.PORING_NOVICE_B;

AllMountTable[JobId.ACOLYTE_B] = JobId.SHEEP_ACO_B;
AllMountTable[JobId.ARCHER_B] = JobId.OSTRICH_ARCHER_B;
AllMountTable[JobId.MAGICIAN_B] = JobId.FOX_MAGICIAN_B;
AllMountTable[JobId.MERCHANT_B] = JobId.PIG_MERCHANT_B;
AllMountTable[JobId.SWORDMAN_B] = JobId.PECO_SWORD_B;
AllMountTable[JobId.THIEF_B] = JobId.DOG_THIEF_B;

AllMountTable[JobId.PRIEST_B] = JobId.SHEEP_PRIEST_B;
AllMountTable[JobId.HUNTER_B] = JobId.OSTRICH_HUNTER_B;
AllMountTable[JobId.WIZARD_B] = JobId.FOX_WIZ_B;
AllMountTable[JobId.BLACKSMITH_B] = JobId.PIG_BLACKSMITH_B;
AllMountTable[JobId.KNIGHT_B] = JobId.LION_KNIGHT_B;
AllMountTable[JobId.ASSASSIN_B] = JobId.DOG_ASSASSIN_B;

AllMountTable[JobId.MONK_B] = JobId.SHEEP_MONK_B;
AllMountTable[JobId.BARD_B] = JobId.OSTRICH_BARD_B;
AllMountTable[JobId.DANCER_B] = JobId.OSTRICH_DANCER_B;
AllMountTable[JobId.SAGE_B] = JobId.FOX_SAGE_B;
AllMountTable[JobId.ALCHEMIST_B] = JobId.PIG_ALCHE_B;
AllMountTable[JobId.CRUSADER_B] = JobId.LION_CRUSADER_B;
AllMountTable[JobId.ROGUE_B] = JobId.DOG_ROGUE_B;

AllMountTable[JobId.ARCHBISHOP_B] = JobId.SHEEP_ARCB_B;
AllMountTable[JobId.RANGER_B] = JobId.OSTRICH_RANGER_B;
AllMountTable[JobId.WARLOCK_B] = JobId.FOX_WARLOCK_B;
AllMountTable[JobId.MECHANIC_B] = JobId.PIG_MECHANIC_B;
AllMountTable[JobId.RUNE_KNIGHT_B] = JobId.LION_RUNE_KNIGHT_B;
AllMountTable[JobId.GUILLOTINE_CROSS_B] = JobId.DOG_G_CROSS_B;

AllMountTable[JobId.SURA_B] = JobId.SHEEP_SURA_B;
AllMountTable[JobId.MINSTREL_B] = JobId.OSTRICH_MINSTREL_B;
AllMountTable[JobId.WANDERER_B] = JobId.OSTRICH_WANDER_B;
AllMountTable[JobId.SORCERER_B] = JobId.FOX_SORCERER_B;
AllMountTable[JobId.GENETIC_B] = JobId.PIG_GENETIC_B;
AllMountTable[JobId.ROYAL_GUARD_B] = JobId.LION_ROYAL_GUARD_B;
AllMountTable[JobId.SHADOW_CHASER_B] = JobId.DOG_CHASER_B;

AllMountTable[JobId.SUPERNOVICE_B] = JobId.PORING_SNOVICE_B;

AllMountTable[JobId.NINJA_B] = JobId.FROG_NINJA_B;
AllMountTable[JobId.GUNSLINGER_B] = JobId.PECO_GUNSLINGER_B;
AllMountTable[JobId.TAEKWON_B] = JobId.PORING_TAEKWON_B;

AllMountTable[JobId.STAR_B] = JobId.PORING_STAR_B;
AllMountTable[JobId.LINKER_B] = JobId.PORING_LINKER_B;

AllMountTable[JobId.SUPERNOVICE2_B] = JobId.PORING_SNOVICE2_B;
AllMountTable[JobId.KAGEROU_B] = JobId.FROG_KAGEROU_B;
AllMountTable[JobId.OBORO_B] = JobId.FROG_OBORO_B;
AllMountTable[JobId.REBELLION_B] = JobId.PECO_REBELLION_B;
AllMountTable[JobId.SOUL_REAPER_B] = JobId.SOUL_REAPER2_B;
AllMountTable[JobId.STAR_EMPEROR_B] = JobId.STAR_EMPEROR2_B;

function duplicateEntry(origin) {
	const value = AllMountTable[origin];
	for (let i = 1, count = arguments.length; i < count; ++i) {
		AllMountTable[arguments[i]] = value;
	}
}

// Inherit
duplicateEntry(JobId.ARCHBISHOP, JobId.ARCHBISHOP_H);
duplicateEntry(JobId.RANGER, JobId.RANGER_H);
duplicateEntry(JobId.WARLOCK, JobId.WARLOCK_H);
duplicateEntry(JobId.MECHANIC, JobId.MECHANIC_H);
duplicateEntry(JobId.RUNE_KNIGHT, JobId.RUNE_KNIGHT_H);
duplicateEntry(JobId.GUILLOTINE_CROSS, JobId.GUILLOTINE_CROSS_H);

duplicateEntry(JobId.SURA, JobId.SURA_H);
duplicateEntry(JobId.MINSTREL, JobId.MINSTREL_H);
duplicateEntry(JobId.WANDERER, JobId.WANDERER_H);
duplicateEntry(JobId.SORCERER, JobId.SORCERER_H);
duplicateEntry(JobId.GENETIC, JobId.GENETIC_H);
duplicateEntry(JobId.ROYAL_GUARD, JobId.ROYAL_GUARD_H);
duplicateEntry(JobId.SHADOW_CHASER, JobId.SHADOW_CHASER_H);

duplicateEntry(JobId.ARCHBISHOP, JobId.ARCH_BISHOP_2ND);
duplicateEntry(JobId.RANGER, JobId.RANGER_2ND);
duplicateEntry(JobId.WARLOCK, JobId.WARLOCK_2ND);
duplicateEntry(JobId.MECHANIC, JobId.MECHANIC_2ND);
duplicateEntry(JobId.RUNE_KNIGHT, JobId.RUNE_KNIGHT_2ND);
duplicateEntry(JobId.GUILLOTINE_CROSS, JobId.GUILLOTINE_CROSS_2ND);

duplicateEntry(JobId.SURA, JobId.SURA_2ND);
duplicateEntry(JobId.MINSTREL, JobId.MINSTREL_2ND);
duplicateEntry(JobId.WANDERER, JobId.WANDERER_2ND);
duplicateEntry(JobId.SORCERER, JobId.SORCERER_2ND);
duplicateEntry(JobId.GENETIC, JobId.GENETIC_2ND);
duplicateEntry(JobId.ROYAL_GUARD, JobId.ROYAL_GUARD2_2ND);
duplicateEntry(JobId.SHADOW_CHASER, JobId.SHADOW_CHASER_2ND);

export default AllMountTable;
