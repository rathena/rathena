// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "twinklinggalaxy.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillTwinklingGalaxy::SkillTwinklingGalaxy() : SkillImpl(SKE_TWINKLING_GALAXY) {
}

void SkillTwinklingGalaxy::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (flag & 1)
		skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillTwinklingGalaxy::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	for (int32 i = 0; i < skill_get_time(getSkillId(), skill_lv) / skill_get_unit_interval(getSkillId()); i++)
		skill_addtimerskill(src, tick + (t_tick)i*skill_get_unit_interval(getSkillId()), 0, x, y, getSkillId(), skill_lv, 0, flag);
	flag |= 1;
	skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);
}

void SkillTwinklingGalaxy::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 300 + 500 * skill_lv;
	skillratio += pc_checkskill(sd, SKE_SKY_MASTERY) * 3 * skill_lv;
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}
