/**
 * DB/Jobs/MountTable.js
 *
 * Look up table <job> => <job mount>
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import JobId from './JobConst.js';

const MountTable = {};

// MountTable[<Base job>]       = <Mount job>

MountTable[JobId.KNIGHT] = JobId.KNIGHT2;
MountTable[JobId.KNIGHT_H] = JobId.KNIGHT2_H;
MountTable[JobId.KNIGHT_B] = JobId.KNIGHT2_B;

MountTable[JobId.CRUSADER] = JobId.CRUSADER2;
MountTable[JobId.CRUSADER_H] = JobId.CRUSADER2_H;
MountTable[JobId.CRUSADER_B] = JobId.CRUSADER2_B;
MountTable[JobId.CRUSADER_2ND] = JobId.CRUSADER2_2ND;

MountTable[JobId.RUNE_KNIGHT] = JobId.RUNE_KNIGHT2;
MountTable[JobId.RUNE_KNIGHT_H] = JobId.RUNE_KNIGHT2_H;
MountTable[JobId.RUNE_KNIGHT_B] = JobId.RUNE_KNIGHT2_B;
MountTable[JobId.RUNE_KNIGHT_2ND] = JobId.RUNE_KNIGHT2_2ND;

MountTable[JobId.ROYAL_GUARD] = JobId.ROYAL_GUARD2;
MountTable[JobId.ROYAL_GUARD_H] = JobId.ROYAL_GUARD2_H;
MountTable[JobId.ROYAL_GUARD_B] = JobId.ROYAL_GUARD2_B;
MountTable[JobId.ROYAL_GUARD_2ND] = JobId.ROYAL_GUARD2_2ND;

MountTable[JobId.RANGER] = JobId.RANGER2;
MountTable[JobId.RANGER_H] = JobId.RANGER2_H;
MountTable[JobId.RANGER_B] = JobId.RANGER2_B;
MountTable[JobId.RANGER_2ND] = JobId.RANGER2_2ND;

MountTable[JobId.MECHANIC] = JobId.MECHANIC2;
MountTable[JobId.MECHANIC_H] = JobId.MECHANIC2_H;
MountTable[JobId.MECHANIC_B] = JobId.MECHANIC2_B;
MountTable[JobId.MECHANIC_2ND] = JobId.MECHANIC2_2ND;

// 4th job
MountTable[JobId.WINDHAWK] = JobId.WINDHAWK2;
MountTable[JobId.MEISTER] = JobId.MEISTER2;
MountTable[JobId.DRAGON_KNIGHT] = JobId.DRAGON_KNIGHT2;
MountTable[JobId.IMPERIAL_GUARD] = JobId.IMPERIAL_GUARD2;

export default MountTable;
