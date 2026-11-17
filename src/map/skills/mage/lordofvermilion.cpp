// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "lordofvermilion.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillLordOfVermilion::SkillLordOfVermilion() : SkillImpl(WZ_VERMILION) {
}

void SkillLordOfVermilion::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	flag |= 1;

	skill_unitsetting(src, getSkillId(),skill_lv,x,y,0);
}

void SkillLordOfVermilion::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	const map_session_data* sd = BL_CAST(BL_PC, src);

	if(sd)
		base_skillratio += 300 + skill_lv * 100;
	else
		base_skillratio += 20 * skill_lv - 20; //Monsters use old formula
#else
	base_skillratio += 20 * skill_lv - 20;
#endif
}

void SkillLordOfVermilion::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
#ifdef RENEWAL
	sc_start(src,target,SC_BLIND,10 + 5 * skill_lv,skill_lv,skill_get_time2(getSkillId(),skill_lv));
#else
	sc_start(src,target,SC_BLIND,min(4*skill_lv,40),skill_lv,skill_get_time2(getSkillId(),skill_lv));
#endif
}
