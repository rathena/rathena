#include "skill_nj_kamaitachi.hpp"

#include "../../clif.hpp"
#include "../../map.hpp"
#include "../../skill.hpp"
#include "../../unit.hpp"

SkillNJ_KAMAITACHI::SkillNJ_KAMAITACHI() : SkillImpl(NJ_KAMAITACHI) {
}

void SkillNJ_KAMAITACHI::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_area_temp[1] = target->id;
	if (battle_config.skill_eightpath_algorithm)
	{
		// Use official AoE algorithm
		map_foreachindir(skill_attack_area, src->m, src->x, src->y, target->x, target->y,
						 skill_get_splash(this->skill_id_, skill_lv), skill_get_maxcount(this->skill_id_, skill_lv), 0, splash_target(src),
						 skill_get_type(this->skill_id_), src, src, this->skill_id_, skill_lv, tick, flag, BCT_ENEMY);
	}
	else
	{
		map_foreachinpath(skill_attack_area, src->m, src->x, src->y, target->x, target->y,
						  skill_get_splash(this->skill_id_, skill_lv), skill_get_maxcount(this->skill_id_, skill_lv), splash_target(src),
						  skill_get_type(this->skill_id_), src, src, this->skill_id_, skill_lv, tick, flag, BCT_ENEMY);
	}
}
