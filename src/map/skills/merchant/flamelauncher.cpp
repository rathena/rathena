// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "flamelauncher.hpp"

#include <config/core.hpp>

#include "map/battle.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillFlameLauncher::SkillFlameLauncher() : SkillImpl(NC_FLAMELAUNCHER) {
}

void SkillFlameLauncher::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_BURNING, 20 + 10 * skill_lv, skill_lv, skill_get_time(getSkillId(), skill_lv));
}

void SkillFlameLauncher::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += 200 + 300 * skill_lv;
	RE_LVL_DMOD(150);
}

void SkillFlameLauncher::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_area_temp[1] = target->id;
	if (battle_config.skill_eightpath_algorithm) {
		//Use official AoE algorithm
		map_foreachindir(skill_attack_area, src->m, src->x, src->y, target->x, target->y,
			skill_get_splash(getSkillId(), skill_lv), skill_get_maxcount(getSkillId(), skill_lv), 0, splash_target(src),
			skill_get_type(getSkillId()), src, src, getSkillId(), skill_lv, tick, flag, BCT_ENEMY);
	} else {
		map_foreachinpath(skill_attack_area, src->m, src->x, src->y, target->x, target->y,
			skill_get_splash(getSkillId(), skill_lv), skill_get_maxcount(getSkillId(), skill_lv), splash_target(src),
			skill_get_type(getSkillId()), src, src, getSkillId(), skill_lv, tick, flag, BCT_ENEMY);
	}
}
