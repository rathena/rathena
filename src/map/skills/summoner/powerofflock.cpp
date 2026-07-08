// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "powerofflock.hpp"

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/status.hpp"

SkillPowerofFlock::SkillPowerofFlock() : SkillImpl(SU_POWEROFFLOCK) {
}

void SkillPowerofFlock::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (flag&1) {
		sc_start(src, target, SC_FEAR, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
		sc_start(src, target, SC_FREEZE, 100, skill_lv, skill_get_time2(getSkillId(), skill_lv)); //! TODO: What's the duration?
	} else {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		if (battle_config.skill_wall_check)
			map_foreachinshootrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
		else
			map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
	}
}
