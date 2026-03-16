// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mercenary_removetrap.hpp"

#include "map/clif.hpp"

SkillMercenaryRemoveTrap::SkillMercenaryRemoveTrap() : SkillImpl(MA_REMOVETRAP) {
}

void SkillMercenaryRemoveTrap::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_unit* su = BL_CAST(BL_SKILL, target);
	std::shared_ptr<s_skill_unit_group> sg;
	std::shared_ptr<s_skill_db> skill_group;

	// Mercenaries can remove any trap
	if( su && (sg = su->group) && ( skill_group = skill_db.find(sg->skill_id) ) && skill_group->inf2[INF2_ISTRAP] )
	{
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		skill_delunit(su);
	}
}
