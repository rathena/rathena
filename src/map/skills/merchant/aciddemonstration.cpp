// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "aciddemonstration.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillAcidDemonstration::SkillAcidDemonstration() : WeaponSkillImpl(CR_ACIDDEMONSTRATION) {
}

void SkillAcidDemonstration::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);

	base_skillratio += -100 + 200 * skill_lv + sstatus->int_ + tstatus->vit; // !TODO: Confirm status bonus
	if (target->type == BL_PC)
		base_skillratio /= 2;
#endif
}

void SkillAcidDemonstration::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
#ifdef RENEWAL
	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
#else
	skill_attack(skill_get_type(getSkillId()),src,src,target,getSkillId(),skill_lv,tick,flag);
#endif
}

void SkillAcidDemonstration::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	skill_break_equip(src,target, EQP_WEAPON|EQP_ARMOR, 100*skill_lv, BCT_ENEMY);
}
