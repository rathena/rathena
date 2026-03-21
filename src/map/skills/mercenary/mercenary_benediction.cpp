// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mercenary_benediction.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillMercenaryBenediction::SkillMercenaryBenediction() : SkillImpl(MER_BENEDICTION) {
}

void SkillMercenaryBenediction::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change_end(target, SC_CURSE);
	status_change_end(target, SC_BLIND);
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
}
