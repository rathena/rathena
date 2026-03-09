// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "crimsonarrow.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

// AG_CRIMSON_ARROW
SkillCrimsonArrow::SkillCrimsonArrow() : SkillImpl(AG_CRIMSON_ARROW) {
}

void SkillCrimsonArrow::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 400 * skill_lv + 3 * sstatus->spl;
	RE_LVL_DMOD(100);
}

void SkillCrimsonArrow::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_area_temp[1] = target->id;
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	if (battle_config.skill_eightpath_algorithm) {
		//Use official AoE algorithm
		if (!(map_foreachindir(skill_attack_area, src->m, src->x, src->y, target->x, target->y,
				skill_get_splash(getSkillId(), skill_lv), skill_get_maxcount(getSkillId(), skill_lv), 0, splash_target(src),
				skill_get_type(getSkillId()), src, src, getSkillId(), skill_lv, tick, flag, BCT_ENEMY))) {

			//These skills hit at least the target if the AoE doesn't hit
			skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
		}
	} else {
		map_foreachinpath(skill_attack_area, src->m, src->x, src->y, target->x, target->y,
			skill_get_splash(getSkillId(), skill_lv), skill_get_maxcount(getSkillId(), skill_lv), splash_target(src),
			skill_get_type(getSkillId()), src, src, getSkillId(), skill_lv, tick, flag, BCT_ENEMY);
	}
	skill_castend_damage_id(src, target, AG_CRIMSON_ARROW_ATK, skill_lv, tick, flag|SD_LEVEL|SD_ANIMATION);
}


// AG_CRIMSON_ARROW_ATK
SkillCrimsonArrowAttack::SkillCrimsonArrowAttack() : SkillImplRecursiveDamageSplash(AG_CRIMSON_ARROW_ATK) {
}

void SkillCrimsonArrowAttack::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 750 * skill_lv + 5 * sstatus->spl;
	RE_LVL_DMOD(100);
}
