// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "unchainedserenade.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"

SkillUnchainedSerenade::SkillUnchainedSerenade() : WeaponSkillImpl(BA_DISSONANCE) {
}

void SkillUnchainedSerenade::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	const map_session_data* sd = BL_CAST( BL_PC, src );

	base_skillratio += 10 + skill_lv * 50;
	if (sd != nullptr)
		base_skillratio = base_skillratio * sd->status.job_level / 10;
#endif
}

void SkillUnchainedSerenade::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
#ifdef RENEWAL
	skill_castend_song(src, getSkillId(), skill_lv, tick);
#endif
}

void SkillUnchainedSerenade::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
#ifndef RENEWAL
	flag|=1;//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	// Ammo should be deleted right away.
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
#endif
}
