// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mirage.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillMirage::SkillMirage() : SkillImpl(SS_SHINKIROU) {
}

void SkillMirage::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	flag |= 1;
	clif_skill_nodamage(src, *src, getSkillId(), skill_lv);
	sc_start(src, src, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
	skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);
}
