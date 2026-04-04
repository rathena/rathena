// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mercenary_mindblaster.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillMercenaryMindBlaster::SkillMercenaryMindBlaster() : SkillImpl(MER_INVINCIBLEOFF2) {
}

void SkillMercenaryMindBlaster::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	status_change_end(target, SC_INVINCIBLE);
}
