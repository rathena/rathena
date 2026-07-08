// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "holycross.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillHolyCross::SkillHolyCross() : WeaponSkillImpl(CR_HOLYCROSS) {
}

void SkillHolyCross::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	const map_session_data* sd = BL_CAST(BL_PC, src);

	if(sd && sd->status.weapon == W_2HSPEAR)
		base_skillratio += 70 * skill_lv;
	else
#endif
		base_skillratio += 35 * skill_lv;
}

void SkillHolyCross::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src,target,SC_BLIND,3*skill_lv,skill_lv,skill_get_time2(getSkillId(),skill_lv));
}
