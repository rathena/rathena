// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "stormgust.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillStormGust::SkillStormGust() : SkillImpl(WZ_STORMGUST) {
}

void SkillStormGust::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	flag |= 1;

	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}

void SkillStormGust::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	base_skillratio -= 30; // Offset only once
	base_skillratio += 50 * skill_lv;
#else
	base_skillratio += 40 * skill_lv;
#endif
}

void SkillStormGust::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	// Storm Gust counter was dropped in renewal
#ifdef RENEWAL
	sc_start(src,target,SC_FREEZE,65-(5*skill_lv),skill_lv,skill_get_time2(getSkillId(),skill_lv));
#else
	status_change* tsc = status_get_sc( target );

	if (tsc != nullptr) {
		//On third hit, there is a 150% to freeze the target
		if(tsc->sg_counter >= 3 &&
			sc_start(src,target,SC_FREEZE,150,skill_lv,skill_get_time2(getSkillId(),skill_lv)))
			tsc->sg_counter = 0;
		// Being it only resets on success it'd keep stacking and eventually overflowing on mvps, so we reset at a high value
		else if( tsc->sg_counter > 250 )
			tsc->sg_counter = 0;
	}
#endif
}
