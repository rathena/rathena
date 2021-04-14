//===== rAthena Script =======================================
//= Mapflag: Damage Adjustment
//===== By: ==================================================
//= [Cydh]
//===== Description: =========================================
// <mapname>	mapflag	atk_rate	<AttackerType>,<ShortDamageRate>,<LongDamageRate>,<WeaponDamageRate>,<MagicDamageRate>,<MiscDamageRate>
//
// 'AttackerType' is bitmask value of
//     1: Players
//     2: Monsters
//     4: Pets
//     8: Homunculus
//    16: Mercenaries
//    32: Elementals
// If only 'ShortDamageRate' that has the given value, the rest of rates will be same with its value.
//============================================================
// Example:
//niflheim	mapflag	atk_rate	1,200,200,200,200,200	// All players damage will be 200%
//nif_fild01	mapflag	atk_rate	1,70,80,60,75,120	// Short: 70%, Long: 80%, Physical Skill: 60%, Magic Skill: 75%, Misc: 120%
