// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "wargdash.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillWargDash::SkillWargDash() : SkillImplRecursiveDamageSplash(RA_WUGDASH) {
}

void SkillWargDash::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	// ATK 300%
	base_skillratio += 200;
}

void SkillWargDash::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );
	sc_type type = skill_get_sc(getSkillId());
	status_change *tsc = status_get_sc(target);
	status_change_entry *tsce = (tsc && type != SC_NONE)?tsc->getSCE(type):nullptr;

	if( tsce ) {
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv,status_change_end(target, type));
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}
	if( sd && pc_isridingwug(sd) ) {
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv,sc_start4(src,target,type,100,skill_lv,unit_getdir(target),0,0,0));
		clif_walkok(*sd);
	}
}
