// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fatalshadowcrow.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillFatalShadowCrow::SkillFatalShadowCrow() : SkillImplRecursiveDamageSplash(SHC_FATAL_SHADOW_CROW) {
}

void SkillFatalShadowCrow::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	sc_start( src, target, SC_DARKCROW, 100, max( 1, pc_checkskill( sd, GC_DARKCROW ) ), skill_get_time( getSkillId(), skill_lv ) );
}

void SkillFatalShadowCrow::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);

	skillratio += -100 + 1300 * skill_lv + 10 * sstatus->pow;
	if (tstatus->race == RC_DEMIHUMAN || tstatus->race == RC_DRAGON)
		skillratio += 150 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillFatalShadowCrow::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	uint8 dir = DIR_NORTHEAST;

	if (target->x != src->x || target->y != src->y)
		dir = map_calc_dir(target, src->x, src->y);	// dir based on target as we move player based on target location

	// Move the player 1 cell near the target, between the target and the player
	if (skill_check_unit_movepos(5, src, target->x + dirx[dir], target->y + diry[dir], 0, 1))
		clif_blown(src);
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);// Trigger animation

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
