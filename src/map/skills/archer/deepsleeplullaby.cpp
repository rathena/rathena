// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "deepsleeplullaby.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillDeepSleepLullaby::SkillDeepSleepLullaby() : SkillImpl(WM_LULLABY_DEEPSLEEP) {
}

void SkillDeepSleepLullaby::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (flag&1) {
		int32 rate = 4 * skill_lv + (sd ? pc_checkskill(sd, WM_LESSON) * 2 : 0) + status_get_lv(src) / 15 + (sd ? sd->status.job_level / 5 : 0);
		int32 duration = skill_get_time(getSkillId(), skill_lv) - (status_get_base_status(target)->int_ * 50 + status_get_lv(target) * 50); // Duration reduction for Deep Sleep Lullaby is doubled

		sc_start(src, target, type, rate, skill_lv, duration);
	} else {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		map_foreachinallrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
	}
}
