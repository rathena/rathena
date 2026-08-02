// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "wargbite.hpp"

#include "map/clif.hpp"
#include "map/path.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillWargBite::SkillWargBite() : WeaponSkillImpl(RA_WUGBITE) {
}

void SkillWargBite::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 300 + 200 * skill_lv;
	if (skill_lv == 5)
		base_skillratio += 100;
}

void SkillWargBite::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	if( path_search(nullptr,src->m,src->x,src->y,target->x,target->y,1,CELL_CHKNOREACH) ) {
		WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
	}else if( sd )
		clif_skill_fail( *sd, getSkillId() );
}

void SkillWargBite::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	int32 wug_rate = (50 + 10 * skill_lv) + 2 * ((sd) ? pc_checkskill(sd,RA_TOOTHOFWUG)*2 : skill_get_max(RA_TOOTHOFWUG)) - (status_get_agi(target) / 4);
	if (wug_rate < 50)
		wug_rate = 50;
	sc_start(src,target, SC_BITE, wug_rate, skill_lv, (skill_get_time(getSkillId(),skill_lv) + ((sd) ? pc_checkskill(sd,RA_TOOTHOFWUG)*500 : skill_get_max(RA_TOOTHOFWUG))) );
}
