// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "secondfaith.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillSecondFaith::SkillSecondFaith() : SkillImplRecursiveDamageSplash(IQ_SECOND_FAITH) {
}

void SkillSecondFaith::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 100 + 2300 * skill_lv + 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillSecondFaith::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_SECOND_BRAND, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
}

void SkillSecondFaith::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
