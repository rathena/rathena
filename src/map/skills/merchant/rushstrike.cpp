// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "rushstrike.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/path.hpp"
#include "map/status.hpp"

SkillRushStrike::SkillRushStrike() : SkillImplRecursiveDamageSplash(MT_RUSH_STRIKE) {
}

void SkillRushStrike::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 3500 * skill_lv;
	skillratio += 5 * sstatus->pow; // !TODO: check POW ratio
	RE_LVL_DMOD(100);
}

void SkillRushStrike::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	// Jump to the target before attacking.
	if( skill_check_unit_movepos( 5, src, target->x, target->y, 0, 1 ) ){
		skill_blown( src, src, 1, direction_opposite( static_cast<enum directions>( map_calc_dir( target, src->x, src->y ) ) ), BLOWN_NONE);
	}
	clif_skill_nodamage( src, *target, getSkillId(), skill_lv); // Trigger animation
	clif_blown( src );

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
