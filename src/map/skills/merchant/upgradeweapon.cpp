// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "upgradeweapon.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillUpgradeWeapon::SkillUpgradeWeapon() : SkillImpl(WS_WEAPONREFINE) {
}

void SkillUpgradeWeapon::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	if( sd != nullptr ){
		clif_item_refine_list( *sd );
	}
}
