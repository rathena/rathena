//===== rAthena Script =======================================
//= Mapflag: Disable item consumption of skills.
//===== By: ==================================================
//= Kenpachi
//===== Current Version: =====================================
//= 1.0
//===== Compatible With: =====================================
//= rAthena SVN
//===== Description: ========================================= 
//= Disables item consumption of skills.
//=
//= The additional parameter indicates which requirements are nullified. (bit mask)
//= 1 - No catalyst consumption. (Catalyst is still required in inventory.)
//= 2 - Catalyst is not required in inventory. (Does not imply 1-bit.)
//= 4 - No ammo consumption. (Ammo is still required in inventory and must be equipped.)
//= 8 - Ammo is not required in inventory. (Does not imply the 4-bit.)
//===== Additional Comments: ================================= 
//= 1.0 - Initial script.
//============================================================

verus01	mapflag	skillnorequirements	15