// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "howlingmine.hpp"

#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

// TODO: Refactor to SkillImplRecursiveDamageSplash
SkillHowlingMine::SkillHowlingMine() : SkillImpl(RL_H_MINE) {
}

void SkillHowlingMine::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	status_change* tsc = status_get_sc(target);

	if (!(flag&1)) {
		// Triggered by RL_FLICKER
		if( sd != nullptr && sd->flicker ){
			// Don't consume requirement and mark recursion
			flag |= 1;

			// Only if the target has a sticky bomb on it
			if( tsc != nullptr && tsc->hasSCE( skill_get_sc( getSkillId() ) ) && tsc->getSCE( skill_get_sc( getSkillId() ) )->val2 == src->id ){
				// Splash damage around it!
				map_foreachinrange( skill_area_sub, target, skill_get_splash( getSkillId(), skill_lv ), BL_CHAR|BL_SKILL, src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_damage_id );

				// Remove the sticky bomb
				status_change_end( target, skill_get_sc( getSkillId() ) );

				// Apply burning
				sc_start4( src, target, SC_BURNING, 10 * skill_lv, skill_lv, 1000, src->id, 0, skill_get_time2( getSkillId(), skill_lv ) );
			}else{
				// No damage for (possible) targets without a sticky bomb on it
				return;
			}
		}else{
			// Direct attack
			if( skill_attack( skill_get_type( getSkillId() ), src, src, target, getSkillId(), skill_lv, tick, flag ) != 0 ){
				// Add a sticky bomb to the target
				status_change_start( src, target, skill_get_sc( getSkillId() ), 10000, getSkillId(), 0, 0, 0, skill_get_time( getSkillId(), skill_lv ), SCSTART_NOAVOID|SCSTART_NOTICKDEF|SCSTART_NORATEDEF );
			}
		}
	}else{
		// Deal splash damage
		skill_attack( skill_get_type( getSkillId() ), src, src, target, getSkillId(), skill_lv, tick, flag );
	}
}

void SkillHowlingMine::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd && sd->flicker) {
		// Flicker explosion damage: 500 + 300 * SkillLv
		skillratio += -100 + 500 + 300 * skill_lv;
	} else {
		// Direct trigger damage: 200 + 200 * SkillLv
		skillratio += -100 + 200 + 200 * skill_lv;
	}
}
