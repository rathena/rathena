// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "acidterror.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillAcidTerror::SkillAcidTerror() : WeaponSkillImpl(AM_ACIDTERROR) {
}

void SkillAcidTerror::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	const map_session_data* sd = BL_CAST(BL_PC, src);

	base_skillratio += -100 + 200 * skill_lv;
	if (sd && pc_checkskill(sd, AM_LEARNINGPOTION))
		base_skillratio += 100; // !TODO: What's this bonus increase?
#else
	base_skillratio += -50 + 50 * skill_lv;
#endif
}

void SkillAcidTerror::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start2(src,target,SC_BLEEDING,(skill_lv*3),skill_lv,src->id,skill_get_time2(getSkillId(),skill_lv));
#ifdef RENEWAL
	if (skill_break_equip(src,target, EQP_ARMOR, (1000 * skill_lv + 500) - 1000, BCT_ENEMY))
#else
	if (skill_break_equip(src,target, EQP_ARMOR, 100*skill_get_time(getSkillId(),skill_lv), BCT_ENEMY))
#endif
		clif_emotion( *target, ET_HUK );
}
