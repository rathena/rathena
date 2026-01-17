// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "gravitationfield.hpp"

#include <config/const.hpp>
#include <config/core.hpp>

#include "map/status.hpp"

SkillGravitationField::SkillGravitationField() : SkillImpl(HW_GRAVITATION) {
}

void SkillGravitationField::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
#ifdef RENEWAL
	skillratio += -100 + 100 * skill_lv;
	RE_LVL_DMOD(100);
#endif
}

void SkillGravitationField::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
#ifdef RENEWAL
	flag|=1;//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
#else
	std::shared_ptr<s_skill_unit_group> sg;
	sc_type type = skill_get_sc(getSkillId());

	if ((sg = skill_unitsetting(src,getSkillId(),skill_lv,x,y,0)))
		sc_start4(src,src,type,100,skill_lv,0,BCT_SELF,sg->group_id,skill_get_time(getSkillId(),skill_lv));
	flag|=1;
#endif
}

void SkillGravitationField::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
#ifndef RENEWAL
	// Gravitation can trigger physical autospells
	attack_type |= BF_NORMAL;
	attack_type |= BF_WEAPON;
#endif
}
