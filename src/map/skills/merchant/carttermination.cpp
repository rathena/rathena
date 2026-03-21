// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "carttermination.hpp"

#include "map/pc.hpp"
#include "map/status.hpp"

SkillCartTermination::SkillCartTermination() : WeaponSkillImpl(WS_CARTTERMINATION) {
}

void SkillCartTermination::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST( BL_PC, src );

	int32 i = 10 * (16 - skill_lv);
	if (i < 1) i = 1;
	//Preserve damage ratio when max cart weight is changed.
	if (sd && sd->cart_weight)
		base_skillratio += sd->cart_weight / i * 80000 / battle_config.max_cart_weight - 100;
	else if (!sd)
		base_skillratio += 80000 / i - 100;
}

void SkillCartTermination::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src,target,SC_STUN,5*skill_lv,skill_lv,skill_get_time2(getSkillId(),skill_lv));
}
