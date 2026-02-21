// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "npcelectricwalk.hpp"

#include "map/status.hpp"

SkillNpcElectricWalk::SkillNpcElectricWalk() : SkillImpl(NPC_ELECTRICWALK) {
}

void SkillNpcElectricWalk::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += -100 + 100 * skill_lv;
}

void SkillNpcElectricWalk::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);
	sc_type type = skill_get_sc(getSkillId());

	if( sc && sc->getSCE(type) )
		status_change_end(src,type);
	sc_start2(src, src, type, 100, getSkillId(), skill_lv, skill_get_time(getSkillId(), skill_lv));
}
