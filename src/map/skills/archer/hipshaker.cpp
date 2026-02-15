// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hipshaker.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"

SkillHipShaker::SkillHipShaker() : SkillImpl(DC_UGLYDANCE) {
}

void SkillHipShaker::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
#ifdef RENEWAL
	// !TODO: How does caster's DEX/AGI play a role?
	status_zap( target, 0, 2 * skill_lv + 10 );
#else
	map_session_data* sd = BL_CAST( BL_PC, src );

	int32 rate = 5 + 5 * skill_lv;
	rate += skill_lv * pc_checkskill(sd, DC_DANCINGLESSON);
	status_zap( target, 0, rate );
#endif
}

void SkillHipShaker::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
#ifdef RENEWAL
	skill_castend_song(src, getSkillId(), skill_lv, tick);
#endif
}

void SkillHipShaker::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
#ifndef RENEWAL
	flag|=1;//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	// Ammo should be deleted right away.
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
#endif
}
