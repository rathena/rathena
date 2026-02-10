// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "rushquake.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/path.hpp"
#include "map/status.hpp"

SkillRushQuake::SkillRushQuake() : SkillImplRecursiveDamageSplash(MT_RUSH_QUAKE) {
}

void SkillRushQuake::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);

	skillratio += -100 + 3600 * skill_lv + 10 * sstatus->pow;
	if (tstatus->race == RC_FORMLESS || tstatus->race == RC_INSECT)
		skillratio += 150 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillRushQuake::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start( src, target, SC_RUSH_QUAKE1, 100, skill_lv, skill_get_time( getSkillId(), skill_lv ) );
}

void SkillRushQuake::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	// Jump to the target before attacking.
	if( skill_check_unit_movepos( 5, src, target->x, target->y, 0, 1 ) ){
		skill_blown( src, src, 1, direction_opposite( static_cast<enum directions>( map_calc_dir( target, src->x, src->y ) ) ), BLOWN_NONE);
	}
	clif_skill_nodamage( src, *target, getSkillId(), skill_lv); // Trigger animation
	clif_blown( src );

	// TODO: does this buff start before or after dealing damage? [Muh]
	sc_start( src, src, SC_RUSH_QUAKE2, 100, skill_lv, skill_get_time2( getSkillId(), skill_lv ) );

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
