// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "oratio.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillOratio::SkillOratio() : SkillImpl(AB_ORATIO) {
}

void SkillOratio::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());

	if( flag&1 )
		sc_start(src,target, type, 40 + 5 * skill_lv, skill_lv, skill_get_time(getSkillId(), skill_lv));
	else {
		map_foreachinallrange(skill_area_sub, src, skill_get_splash(getSkillId(), skill_lv), BL_CHAR,
			src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}
}
