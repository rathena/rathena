/**
 * DB/Jobs/JobNameTable.js
 *
 * Look up: job id -> ressource name
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault, Antares
 */

import JobId from './JobConst.js';

const JobNameTable = {};

JobNameTable[JobId.NOVICE] = '\xC3\xCA\xBA\xB8\xC0\xDA';

JobNameTable[JobId.SWORDMAN] = '\xB0\xCB\xBB\xE7';
JobNameTable[JobId.MAGICIAN] = '\xB8\xB6\xB9\xFD\xBB\xE7';
JobNameTable[JobId.ARCHER] = '\xB1\xC3\xBC\xF6';
JobNameTable[JobId.ACOLYTE] = '\xBC\xBA\xC1\xF7\xC0\xDA';
JobNameTable[JobId.MERCHANT] = '\xBB\xF3\xC0\xCE';
JobNameTable[JobId.THIEF] = '\xB5\xB5\xB5\xCF';

JobNameTable[JobId.KNIGHT] = '\xB1\xE2\xBB\xE7';
JobNameTable[JobId.PRIEST] = '\xC7\xC1\xB8\xAE\xBD\xBA\xC6\xAE';
JobNameTable[JobId.WIZARD] = '\xC0\xA7\xC0\xFA\xB5\xE5';
JobNameTable[JobId.BLACKSMITH] = '\xC1\xA6\xC3\xB6\xB0\xF8';
JobNameTable[JobId.HUNTER] = '\xC7\xE5\xC5\xCD';
JobNameTable[JobId.ASSASSIN] = '\xBE\xEE\xBC\xBC\xBD\xC5';
JobNameTable[JobId.KNIGHT2] = '\xC6\xE4\xC4\xDA\xC6\xE4\xC4\xDA_\xB1\xE2\xBB\xE7';

JobNameTable[JobId.CRUSADER] = '\xC5\xA9\xB7\xE7\xBC\xBC\xC0\xCC\xB4\xF5';
JobNameTable[JobId.MONK] = '\xB8\xF9\xC5\xA9';
JobNameTable[JobId.SAGE] = '\xBC\xBC\xC0\xCC\xC1\xF6';
JobNameTable[JobId.ROGUE] = '\xB7\xCE\xB1\xD7';
JobNameTable[JobId.ALCHEMIST] = '\xBF\xAC\xB1\xDD\xBC\xFA\xBB\xE7';
JobNameTable[JobId.BARD] = '\xB9\xD9\xB5\xE5';
JobNameTable[JobId.DANCER] = '\xB9\xAB\xC8\xF1';
JobNameTable[JobId.CRUSADER2] = '\xBD\xC5\xC6\xE4\xC4\xDA\xC5\xA9\xB7\xE7\xBC\xBC\xC0\xCC\xB4\xF5';

JobNameTable[JobId.SUPERNOVICE] = '\xBD\xB4\xC6\xDB\xB3\xEB\xBA\xF1\xBD\xBA';
JobNameTable[JobId.GUNSLINGER] = '\xB0\xC7\xB3\xCA';
JobNameTable[JobId.NINJA] = '\xB4\xD1\xC0\xDA';
JobNameTable[JobId.TAEKWON] = '\xc5\xc2\xb1\xc7\xbc\xd2\xb3\xe2';
JobNameTable[JobId.STAR] = '\xb1\xc7\xbc\xba';
JobNameTable[JobId.STAR2] = '\xb1\xc7\xbc\xba\xc0\xb6\xc7\xd5';
JobNameTable[JobId.LINKER] = '\xbc\xd2\xbf\xef\xb8\xb5\xc4\xbf';

JobNameTable[JobId.MARRIED] = '\xB0\xE1\xC8\xA5';
JobNameTable[JobId.XMAS] = '\xBB\xEA\xC5\xB8';
JobNameTable[JobId.SUMMER] = '\xBF\xA9\xB8\xA7';

JobNameTable[JobId.KNIGHT_H] = '\xB7\xCE\xB5\xE5\xB3\xAA\xC0\xCC\xC6\xAE';
JobNameTable[JobId.PRIEST_H] = '\xC7\xCF\xC0\xCC\xC7\xC1\xB8\xAE';
JobNameTable[JobId.WIZARD_H] = '\xC7\xCF\xC0\xCC\xC0\xA7\xC0\xFA\xB5\xE5';
JobNameTable[JobId.BLACKSMITH_H] = '\xC8\xAD\xC0\xCC\xC6\xAE\xBD\xBA\xB9\xCC\xBD\xBA';
JobNameTable[JobId.HUNTER_H] = '\xBD\xBA\xB3\xAA\xC0\xCC\xC6\xDB';
JobNameTable[JobId.ASSASSIN_H] = '\xBE\xEE\xBD\xD8\xBD\xC5\xC5\xA9\xB7\xCE\xBD\xBA';
JobNameTable[JobId.KNIGHT2_H] = '\xB7\xCE\xB5\xE5\xC6\xE4\xC4\xDA';
JobNameTable[JobId.CRUSADER_H] = '\xC6\xC8\xB6\xF3\xB5\xF2';
JobNameTable[JobId.MONK_H] = '\xC3\xA8\xC7\xC7\xBF\xC2';
JobNameTable[JobId.SAGE_H] = '\xC7\xC1\xB7\xCE\xC6\xE4\xBC\xAD';
JobNameTable[JobId.ROGUE_H] = '\xBD\xBA\xC5\xE4\xC4\xBF';
JobNameTable[JobId.ALCHEMIST_H] = '\xC5\xA9\xB8\xAE\xBF\xA1\xC0\xCC\xC5\xCD';
JobNameTable[JobId.BARD_H] = '\xC5\xAC\xB6\xF3\xBF\xEE';
JobNameTable[JobId.DANCER_H] = '\xC1\xFD\xBD\xC3';
JobNameTable[JobId.CRUSADER2_H] = '\xC6\xE4\xC4\xDA\xC6\xC8\xB6\xF3\xB5\xF2';

JobNameTable[JobId.RUNE_KNIGHT] = '\xB7\xE9\xB3\xAA\xC0\xCC\xC6\xAE';
JobNameTable[JobId.WARLOCK] = '\xBF\xF6\xB7\xCF';
JobNameTable[JobId.RANGER] = '\xB7\xB9\xC0\xCE\xC1\xAE';
JobNameTable[JobId.ARCHBISHOP] = '\xBE\xC6\xC5\xA9\xBA\xF1\xBC\xF3';
JobNameTable[JobId.MECHANIC] = '\xB9\xCC\xC4\xC9\xB4\xD0';
JobNameTable[JobId.GUILLOTINE_CROSS] = '\xB1\xE6\xB7\xCE\xC6\xBE\xC5\xA9\xB7\xCE\xBD\xBA';

JobNameTable[JobId.ROYAL_GUARD] = '\xB0\xA1\xB5\xE5';
JobNameTable[JobId.SORCERER] = '\xBC\xD2\xBC\xAD\xB7\xAF';
JobNameTable[JobId.MINSTREL] = '\xB9\xCE\xBD\xBA\xC6\xAE\xB7\xB2';
JobNameTable[JobId.WANDERER] = '\xBF\xF8\xB4\xF5\xB7\xAF';
JobNameTable[JobId.SURA] = '\xBD\xB4\xB6\xF3';
JobNameTable[JobId.GENETIC] = '\xC1\xA6\xB3\xD7\xB8\xAF';
JobNameTable[JobId.SHADOW_CHASER] = '\xBD\xA6\xB5\xB5\xBF\xEC\xC3\xBC\xC0\xCC\xBC\xAD';

JobNameTable[JobId.RUNE_KNIGHT2] = '\xB7\xE9\xB3\xAA\xC0\xCC\xC6\xAE\xBB\xDA\xB6\xEC';
JobNameTable[JobId.ROYAL_GUARD2] = '\xB1\xD7\xB8\xAE\xC6\xF9\xB0\xA1\xB5\xE5';
JobNameTable[JobId.RANGER2] = '\xB7\xB9\xC0\xCE\xC1\xAE\xB4\xC1\xB4\xEB';
JobNameTable[JobId.MECHANIC2] = '\xB8\xB6\xB5\xB5\xB1\xE2\xBE\xEE';

JobNameTable[JobId.SUPERNOVICE2] = '\xBD\xB4\xC6\xDB\xB3\xEB\xBA\xF1\xBD\xBA';
JobNameTable[JobId.KAGEROU] = 'kagerou';
JobNameTable[JobId.OBORO] = 'oboro';
JobNameTable[JobId.REBELLION] = 'rebellion';
JobNameTable[JobId.STAR_EMPEROR] = '\xbc\xba\xc1\xa6';
JobNameTable[JobId.SOUL_REAPER] = '\xbc\xd2\xbf\xef\xb8\xae\xc6\xdb';

// 4th
JobNameTable[JobId.DRAGON_KNIGHT] = 'dragon_knight';
JobNameTable[JobId.MEISTER] = 'meister';
JobNameTable[JobId.SHADOW_CROSS] = 'shadow_cross';
JobNameTable[JobId.ARCH_MAGE] = 'arch_mage';
JobNameTable[JobId.CARDINAL] = 'cardinal';
JobNameTable[JobId.WINDHAWK] = 'windhawk';
JobNameTable[JobId.IMPERIAL_GUARD] = 'imperial_guard';
JobNameTable[JobId.BIOLO] = 'biolo';
JobNameTable[JobId.ABYSS_CHASER] = 'abyss_chaser';
JobNameTable[JobId.ELEMENTAL_MASTER] = 'elemetal_master';
JobNameTable[JobId.INQUISITOR] = 'inquisitor';
JobNameTable[JobId.TROUBADOUR] = 'troubadour';
JobNameTable[JobId.TROUVERE] = 'trouvere';

JobNameTable[JobId.WINDHAWK2] = 'wolf_windhawk';
JobNameTable[JobId.MEISTER2] = 'meister_madogear2';
JobNameTable[JobId.DRAGON_KNIGHT2] = 'dragon_knight_chicken';
JobNameTable[JobId.IMPERIAL_GUARD2] = 'imperial_guard_chicken';

JobNameTable[JobId.SKY_EMPEROR] = 'sky_emperor';
JobNameTable[JobId.SOUL_ASCETIC] = 'soul_ascetic';
JobNameTable[JobId.SHINKIRO] = 'shinkiro';
JobNameTable[JobId.SHIRANUI] = 'shiranui';
JobNameTable[JobId.NIGHT_WATCH] = 'night_watch';
JobNameTable[JobId.HYPER_NOVICE] = 'hyper_novice';
JobNameTable[JobId.SPIRIT_HANDLER] = 'spirit_handler';

JobNameTable[JobId.SKY_EMPEROR2] = 'sky_emperor2';

//MOUNTS
JobNameTable[JobId.PORING_NOVICE] = '\xb3\xeb\xba\xf1\xbd\xba\xc6\xf7\xb8\xb5';

JobNameTable[JobId.SHEEP_ACO] = '\xba\xb9\xbb\xe7\xbe\xcb\xc6\xc4\xc4\xab';
JobNameTable[JobId.OSTRICH_ARCHER] = '\xc5\xb8\xc1\xb6\xb1\xc3\xbc\xf6';
JobNameTable[JobId.FOX_MAGICIAN] = '\xbf\xa9\xbf\xec\xb8\xb6\xb9\xfd\xbb\xe7';
JobNameTable[JobId.PIG_MERCHANT] = '\xbb\xf3\xc0\xce\xb8\xe4\xb5\xc5\xc1\xf6';
JobNameTable[JobId.PECO_SWORD] = '\xc6\xe4\xc4\xda\xb0\xcb\xbb\xe7';
JobNameTable[JobId.DOG_THIEF] = '\xc4\xcc\xba\xa3\xb7\xce\xbd\xba\xb5\xb5\xb5\xcf';

JobNameTable[JobId.SHEEP_PRIEST] = '\xc7\xc1\xb8\xae\xbd\xba\xc6\xae\xbe\xcb\xc6\xc4\xc4\xab';
JobNameTable[JobId.OSTRICH_HUNTER] = '\xc5\xb8\xc1\xb6\xc7\xe5\xc5\xcd';
JobNameTable[JobId.FOX_WIZ] = '\xbf\xa9\xbf\xec\xc0\xa7\xc0\xfa\xb5\xe5';
JobNameTable[JobId.PIG_BLACKSMITH] = '\xc1\xa6\xc3\xb6\xb0\xf8\xb8\xe4\xb5\xc5\xc1\xf6';
JobNameTable[JobId.LION_KNIGHT] = '\xbb\xe7\xc0\xda\xb1\xe2\xbb\xe7';
JobNameTable[JobId.DOG_ASSASSIN] = '\xc4\xcc\xba\xa3\xb7\xce\xbd\xba\xbe\xee\xbd\xea\xbd\xc5';

JobNameTable[JobId.SHEEP_MONK] = '\xb8\xf9\xc5\xa9\xbe\xcb\xc6\xc4\xc4\xab';
JobNameTable[JobId.OSTRICH_BARD] = '\xc5\xb8\xc1\xb6\xb9\xd9\xb5\xe5';
JobNameTable[JobId.OSTRICH_DANCER] = '\xc5\xb8\xc1\xb6\xb9\xab\xc8\xf1';
JobNameTable[JobId.FOX_SAGE] = '\xbf\xa9\xbf\xec\xbc\xbc\xc0\xcc\xc1\xf6';
JobNameTable[JobId.PIG_ALCHE] = '\xbf\xac\xb1\xdd\xbc\xfa\xbb\xe7\xb8\xe4\xb5\xc5\xc1\xf6';
JobNameTable[JobId.LION_CRUSADER] = '\xbb\xe7\xc0\xda\xc5\xa9\xb7\xe7\xbc\xbc\xc0\xcc\xb4\xf5';
JobNameTable[JobId.DOG_ROGUE] = '\xc4\xcc\xba\xa3\xb7\xce\xbd\xba\xb7\xce\xb1\xd7';

JobNameTable[JobId.SHEEP_ARCB] = '\xbe\xc6\xc5\xa9\xba\xf1\xbc\xf3\xbe\xcb\xc6\xc4\xc4\xab';
JobNameTable[JobId.OSTRICH_RANGER] = '\xc5\xb8\xc1\xb6\xb7\xb9\xc0\xce\xc1\xae';
JobNameTable[JobId.FOX_WARLOCK] = '\xbf\xa9\xbf\xec\xbf\xf6\xb7\xcf';
JobNameTable[JobId.PIG_MECHANIC] = '\xb9\xcc\xc4\xc9\xb4\xd0\xb8\xe4\xb5\xc5\xc1\xf6';
JobNameTable[JobId.LION_RUNE_KNIGHT] = '\xbb\xe7\xc0\xda\xb7\xe9\xb3\xaa\xc0\xcc\xc6\xae';
JobNameTable[JobId.DOG_G_CROSS] = '\xc4\xcc\xba\xa3\xb7\xce\xbd\xba\xb1\xe6\xb7\xce\xc6\xbe\xc5\xa9\xb7\xce\xbd\xba';

JobNameTable[JobId.SHEEP_SURA] = '\xbd\xb4\xb6\xf3\xbe\xcb\xc6\xc4\xc4\xab';
JobNameTable[JobId.OSTRICH_MINSTREL] = '\xc5\xb8\xc1\xb6\xb9\xce\xbd\xba\xc6\xae\xb7\xb2';
JobNameTable[JobId.OSTRICH_WANDER] = '\xc5\xb8\xc1\xb6\xbf\xf8\xb4\xf5\xb7\xaf';
JobNameTable[JobId.FOX_SORCERER] = '\xbf\xa9\xbf\xec\xbc\xd2\xbc\xad\xb7\xaf';
JobNameTable[JobId.PIG_GENETIC] = '\xc1\xa6\xb3\xd7\xb8\xaf\xb8\xe4\xb5\xc5\xc1\xf6';
JobNameTable[JobId.LION_ROYAL_GUARD] = '\xbb\xe7\xc0\xda\xb7\xce\xbe\xe2\xb0\xa1\xb5\xe5';
JobNameTable[JobId.DOG_CHASER] = '\xc4\xcc\xba\xa3\xb7\xce\xbd\xba\xbd\xa6\xb5\xb5\xbf\xec\xc3\xbc\xc0\xcc\xbc\xad';

JobNameTable[JobId.PORING_SNOVICE] = '\xbd\xb4\xc6\xdb\xb3\xeb\xba\xf1\xbd\xba\xc6\xf7\xb8\xb5';

JobNameTable[JobId.FROG_NINJA] = '\xb3\xeb\xba\xf1\xbd\xba\xc6\xf7\xb8\xb5';
JobNameTable[JobId.PECO_GUNNER] = '\xc6\xe4\xc4\xda\xb0\xc7\xb3\xca';
JobNameTable[JobId.PORING_TAEKWON] = '\xc5\xc2\xb1\xc7\xbc\xd2\xb3\xe2\xc6\xf7\xb8\xb5';

JobNameTable[JobId.PORING_STAR] = '\xb1\xc7\xbc\xba\xc6\xf7\xb8\xb5';
JobNameTable[JobId.FROG_LINKER] = '\xb5\xce\xb2\xa8\xba\xf1\xbc\xd2\xbf\xef\xb8\xb5\xc4\xbf';

JobNameTable[JobId.FROG_KAGEROU] = 'frog_kagerou';
JobNameTable[JobId.FROG_OBORO] = 'frog_oboro';
JobNameTable[JobId.PECO_REBELLION] = 'peco_rebellion';

JobNameTable[JobId.SOUL_REAPER2] = '\xc7\xd8\xc5\xc2\xbc\xd2\xbf\xef\xb8\xae\xc6\xdb'; // 해태소울리퍼
JobNameTable[JobId.STAR_EMPEROR2] = '\xc7\xd8\xc5\xc2\xbc\xba\xc1\xa6'; // 해태소울리

JobNameTable[JobId.DO_SUMMONER] = 'summoner';

JobNameTable[JobId.SHEEP_HPRIEST] = '\xc7\xcf\xc0\xcc\xc7\xc1\xb8\xae\xbd\xba\xc6\xae\xbe\xcb\xc6\xc4\xc4\xab';
JobNameTable[JobId.OSTRICH_SNIPER] = '\xc5\xb8\xc1\xb6\xbd\xba\xb3\xaa\xc0\xcc\xc6\xdb';
JobNameTable[JobId.FOX_HWIZ] = '\xbf\xa9\xbf\xec\xc7\xcf\xc0\xcc\xc0\xa7\xc0\xfa\xb5\xe5';
JobNameTable[JobId.PIG_WHITESMITH] = '\xc8\xad\xc0\xcc\xc6\xae\xbd\xba\xb9\xcc\xbd\xba\xb8\xe4\xb5\xc5\xc1\xf6';
JobNameTable[JobId.LION_KNIGHT_H] = '\xbb\xe7\xc0\xda\xb7\xce\xb5\xe5\xb3\xaa\xc0\xcc\xc6\xae';
JobNameTable[JobId.DOG_ASSA_X] = '\xc4\xcc\xba\xa3\xb7\xce\xbd\xba\xbe\xee\xbd\xea\xbd\xc5\xc5\xa9\xb7\xce\xbd\xba';

JobNameTable[JobId.SHEEP_CHAMP] = '\xc3\xa8\xc7\xc7\xbf\xc2\xbe\xcb\xc6\xc4\xc4\xab';
JobNameTable[JobId.OSTRICH_CROWN] = '\xc5\xb8\xc1\xb6\xc5\xa9\xb6\xf3\xbf\xee';
JobNameTable[JobId.OSTRICH_ZIPSI] = '\xc5\xb8\xc1\xb6\xc2\xa4\xbd\xc3';
JobNameTable[JobId.FOX_PROF] = '\xbf\xa9\xbf\xec\xc7\xc1\xb7\xce\xc6\xe4\xbc\xad';
JobNameTable[JobId.PIG_CREATOR] = '\xc5\xa9\xb8\xae\xbf\xa1\xc0\xcc\xc5\xcd\xb8\xe4\xb5\xc5\xc1\xf6';
JobNameTable[JobId.LION_CRUSADER_H] = '\xbb\xe7\xc0\xda\xc6\xc8\xb6\xf3\xb5\xf2';
JobNameTable[JobId.DOG_STALKER] = '\xc4\xcc\xba\xa3\xb7\xce\xbd\xba\xbd\xba\xc5\xe4\xc4\xbf';

JobNameTable[JobId.DRAGON_KNIGHT_RIDING] = 'dragon_knight_riding';
JobNameTable[JobId.MEISTER_RIDING] = 'meister_riding';
JobNameTable[JobId.SHADOW_CROSS_RIDING] = 'shadow_cross_riding';
JobNameTable[JobId.ARCH_MAGE_RIDING] = 'arch_mage_riding';
JobNameTable[JobId.CARDINAL_RIDING] = 'cardinal_riding';
JobNameTable[JobId.WINDHAWK_RIDING] = 'windhawk_riding';
JobNameTable[JobId.IMPERIAL_GUARD_RIDING] = 'imperial_guard_riding';
JobNameTable[JobId.BIOLO_RIDING] = 'biolo_riding';
JobNameTable[JobId.ABYSS_CHASER_RIDING] = 'abyss_chaser_riding';
JobNameTable[JobId.ELEMENTAL_MASTER_RIDING] = 'elemental_master_riding';
JobNameTable[JobId.INQUISITOR_RIDING] = 'inquisitor_riding';
JobNameTable[JobId.TROUBADOUR_RIDING] = 'troubadour_riding';
JobNameTable[JobId.TROUVERE_RIDING] = 'trouvere_riding';

JobNameTable[JobId.DRUID] = 'druid';
JobNameTable[JobId.ALITEA] = 'alitea';
JobNameTable[JobId.KARNOS] = 'karnos';
JobNameTable[JobId.DRUID_RIDING] = 'druid_riding';
JobNameTable[JobId.ALITEA_RIDING] = 'alitea_riding';
JobNameTable[JobId.KARNOS_RIDING] = 'karnos_riding';
JobNameTable[JobId.WEREWOLF] = 'werewolf';
JobNameTable[JobId.WERERAPTOR] = 'wereraptor';

function duplicateEntry(origin) {
	const value = JobNameTable[origin];
	for (let i = 1, count = arguments.length; i < count; ++i) {
		JobNameTable[arguments[i]] = value;
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

duplicateEntry(JobId.RUNE_KNIGHT, JobId.RUNE_KNIGHT_H, JobId.RUNE_KNIGHT_2ND, JobId.RUNE_KNIGHT_B);
duplicateEntry(JobId.RUNE_KNIGHT2, JobId.RUNE_KNIGHT2_H, JobId.RUNE_KNIGHT2_2ND, JobId.RUNE_KNIGHT2_B);
duplicateEntry(JobId.WARLOCK, JobId.WARLOCK_H, JobId.WARLOCK_2ND, JobId.WARLOCK_B);
duplicateEntry(JobId.RANGER, JobId.RANGER_H, JobId.RANGER_2ND, JobId.RANGER_B);
duplicateEntry(JobId.RANGER2, JobId.RANGER2_H, JobId.RANGER2_2ND, JobId.RANGER2_B);
duplicateEntry(JobId.ARCHBISHOP, JobId.ARCHBISHOP_H, JobId.ARCHBISHOP_2ND, JobId.ARCHBISHOP_B);
duplicateEntry(JobId.MECHANIC, JobId.MECHANIC_H, JobId.MECHANIC_2ND, JobId.MECHANIC_B);
duplicateEntry(JobId.MECHANIC2, JobId.MECHANIC2_H, JobId.MECHANIC2_2ND, JobId.MECHANIC2_B);
duplicateEntry(JobId.GUILLOTINE_CROSS, JobId.GUILLOTINE_CROSS_H, JobId.GUILLOTINE_CROSS_2ND, JobId.GUILLOTINE_CROSS_B);
duplicateEntry(JobId.ROYAL_GUARD, JobId.ROYAL_GUARD_H, JobId.ROYAL_GUARD_2ND, JobId.ROYAL_GUARD_B);
duplicateEntry(JobId.ROYAL_GUARD2, JobId.ROYAL_GUARD2_H, JobId.ROYAL_GUARD2_2ND, JobId.ROYAL_GUARD2_B);
duplicateEntry(JobId.SORCERER, JobId.SORCERER_H, JobId.SORCERER_2ND, JobId.SORCERER_B);
duplicateEntry(JobId.MINSTREL, JobId.MINSTREL_H, JobId.MINSTREL_2ND, JobId.MINSTREL_B);
duplicateEntry(JobId.WANDERER, JobId.WANDERER_H, JobId.WANDERER_2ND, JobId.WANDERER_B);
duplicateEntry(JobId.SURA, JobId.SURA_H, JobId.SURA_2ND, JobId.SURA_B);
duplicateEntry(JobId.GENETIC, JobId.GENETIC_H, JobId.GENETIC_2ND, JobId.GENETIC_B);
duplicateEntry(JobId.SHADOW_CHASER, JobId.SHADOW_CHASER_H, JobId.SHADOW_CHASER_2ND, JobId.SHADOW_CHASER_B);
duplicateEntry(JobId.SOUL_REAPER, JobId.SOUL_REAPER_B);
duplicateEntry(JobId.STAR_EMPEROR, JobId.STAR_EMPEROR_B);
duplicateEntry(JobId.DO_SUMMONER, JobId.DO_SUMMONER_B);

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
duplicateEntry(JobId.SOUL_REAPER2, JobId.SOUL_REAPER2_B);
duplicateEntry(JobId.STAR_EMPEROR2, JobId.STAR_EMPEROR2_B);

duplicateEntry(JobId.PORING_STAR, JobId.PORING_STAR_B);
duplicateEntry(JobId.FROG_LINKER, JobId.FROG_LINKER_B);

duplicateEntry(JobId.FROG_KAGEROU, JobId.FROG_KAGEROU_B);
duplicateEntry(JobId.FROG_OBORO, JobId.FROG_OBORO_B);
duplicateEntry(JobId.PECO_REBELLION, JobId.PECO_REBELLION_B);

export default JobNameTable;
