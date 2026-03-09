// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "savageimpact.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillSavageImpact::SkillSavageImpact() : SkillImplRecursiveDamageSplash(SHC_SAVAGE_IMPACT) {
}

void SkillSavageImpact::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);

	skillratio += -100 + 105 * skill_lv + 5 * sstatus->pow;

	if( sc != nullptr && sc->getSCE( SC_SHADOW_EXCEED ) ){
		skillratio += 20 * skill_lv + 3 * sstatus->pow;	// !TODO: check POW ratio
	}

	RE_LVL_DMOD(100);
}

void SkillSavageImpact::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	status_change *sc = status_get_sc(src);

	if( sc && sc->getSCE( SC_CLOAKINGEXCEED ) ){
		skill_area_temp[0] = 2;
		status_change_end( src, SC_CLOAKINGEXCEED );
	}

	uint8 dir = DIR_NORTHEAST;	// up-right when src is on the same cell of target

	if (target->x != src->x || target->y != src->y)
		dir = map_calc_dir(target, src->x, src->y);	// dir based on target as we move player based on target location

	// Move the player 1 cell near the target, between the target and the player
	if (skill_check_unit_movepos(5, src, target->x + dirx[dir], target->y + diry[dir], 0, 1))
		clif_blown(src);
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
