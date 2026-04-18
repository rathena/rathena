// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "traumaticblow.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillTraumaticBlow::SkillTraumaticBlow() : WeaponSkillImpl(LK_HEADCRUSH) {
}

void SkillTraumaticBlow::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 40 * skill_lv;
}

void SkillTraumaticBlow::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	if (status_get_class_(target) == CLASS_BOSS) {
		if (sd)
			clif_skill_fail( *sd, getSkillId() );
		return;
	}

	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillTraumaticBlow::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	status_data* tstatus = status_get_status_data(*target);

	 // Headcrush has chance of causing Bleeding status, except on demon and undead element
	if (!(battle_check_undead(tstatus->race, tstatus->def_ele) || tstatus->race == RC_DEMON))
		sc_start2(src,target, SC_BLEEDING,50, skill_lv, src->id, skill_get_time2(getSkillId(),skill_lv));
}
