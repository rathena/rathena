// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "grenadesdropping.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillGrenadesDropping::SkillGrenadesDropping() : SkillImpl(NW_GRENADES_DROPPING) {
}

void SkillGrenadesDropping::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	uint16 splash = skill_get_splash(getSkillId(), skill_lv);
	uint16 tmpx = rnd_value(x - splash, x + splash);
	uint16 tmpy = rnd_value(y - splash, y + splash);
	skill_unitsetting(src, getSkillId(), skill_lv, tmpx, tmpy, flag);
	for (int32 i = 0; i <= (skill_get_time(getSkillId(), skill_lv) / skill_get_unit_interval(getSkillId())); i++) {
		skill_addtimerskill(src, tick + (t_tick)i * skill_get_unit_interval(getSkillId()), 0, x, y, getSkillId(), skill_lv, 0, flag);
	}
}

void SkillGrenadesDropping::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 550 + 850 * skill_lv;
	skillratio += pc_checkskill(sd, NW_GRENADE_MASTERY) * 30;
	skillratio += 5 * sstatus->con;
	RE_LVL_DMOD(100);
}
