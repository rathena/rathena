// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "weaponrepair.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillWeaponRepair::SkillWeaponRepair() : SkillImpl(BS_REPAIRWEAPON) {
}

void SkillWeaponRepair::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);

	if(sd && dstsd)
		clif_item_repair_list( *sd, *dstsd, skill_lv );
}
