// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mercenary_scapegoat.hpp"

#include "map/mercenary.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillMercenaryScapegoat::SkillMercenaryScapegoat() : SkillImpl(MER_SCAPEGOAT) {
}

void SkillMercenaryScapegoat::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	s_mercenary_data* mer = BL_CAST(BL_MER, src);

	if( mer && mer->master )
	{
		status_heal(mer->master, mer->battle_status.hp, 0, 2);
		status_damage(src, src, mer->battle_status.max_hp, 0, 0, 1, getSkillId());
	}
}
