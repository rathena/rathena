// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "soulcurse.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillSoulCurse::SkillSoulCurse() : SkillImpl(SP_SOULCURSE) {
}

void SkillSoulCurse::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (flag&1)
		sc_start(src, target, skill_get_sc(getSkillId()), 30 + 10 * skill_lv, skill_lv, skill_get_time(getSkillId(), skill_lv));
	else {
		map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}
}
