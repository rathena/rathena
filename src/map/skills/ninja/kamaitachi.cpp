// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "kamaitachi.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/skill.hpp"
#include "map/unit.hpp"

SkillKamaitachi::SkillKamaitachi() : SkillImpl(NJ_KAMAITACHI) {
}

void SkillKamaitachi::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	skill_area_temp[1] = target->id;
	if (battle_config.skill_eightpath_algorithm) {
		// Use official AoE algorithm
		map_foreachindir(skill_attack_area, src->m, src->x, src->y, target->x, target->y,
		                 skill_get_splash(this->skill_id_, skill_lv), skill_get_maxcount(this->skill_id_, skill_lv), 0,
		                 splash_target(src),
		                 skill_get_type(this->skill_id_), src, src, this->skill_id_, skill_lv, tick, flag, BCT_ENEMY);
	} else {
		map_foreachinpath(skill_attack_area, src->m, src->x, src->y, target->x, target->y,
		                  skill_get_splash(this->skill_id_, skill_lv), skill_get_maxcount(this->skill_id_, skill_lv),
		                  splash_target(src),
		                  skill_get_type(this->skill_id_), src, src, this->skill_id_, skill_lv, tick, flag, BCT_ENEMY);
	}
}
