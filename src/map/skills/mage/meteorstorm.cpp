// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "meteorstorm.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillMeteorStorm::SkillMeteorStorm() : SkillImpl(WZ_METEOR) {
}

void SkillMeteorStorm::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 area = skill_get_splash(getSkillId(), skill_lv);
	int16 tmpx = 0, tmpy = 0;

	for (int32 i = 1; i <= skill_get_time(getSkillId(), skill_lv) / skill_get_unit_interval(getSkillId()); i++) {
		// Creates a random Cell in the Splash Area
		tmpx = x - area + rnd() % (area * 2 + 1);
		tmpy = y - area + rnd() % (area * 2 + 1);
		skill_unitsetting(src, getSkillId(), skill_lv, tmpx, tmpy, flag + i * skill_get_unit_interval(getSkillId()));
	}
}

void SkillMeteorStorm::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	base_skillratio += 25;
#endif
}

void SkillMeteorStorm::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src,target,SC_STUN,3*skill_lv,skill_lv,skill_get_time2(getSkillId(),skill_lv));
}
