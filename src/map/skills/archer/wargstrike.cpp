// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "wargstrike.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/path.hpp"
#include "map/pc.hpp"
#include "map/unit.hpp"

SkillWargStrike::SkillWargStrike() : WeaponSkillImpl(RA_WUGSTRIKE) {
}

void SkillWargStrike::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += -100 + 200 * skill_lv;
}

void SkillWargStrike::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	if( sd && pc_isridingwug(sd) ){
		uint8 dir = map_calc_dir(target, src->x, src->y);

		if( unit_movepos(src, target->x+dirx[dir], target->y+diry[dir], 1, 1) ) {
			clif_blown(src);
			WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
		}
		return;
	}
	if( path_search(nullptr,src->m,src->x,src->y,target->x,target->y,1,CELL_CHKNOREACH) ) {
		WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
	}
}
