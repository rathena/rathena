// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "earthspike.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillEarthSpike::SkillEarthSpike() : SkillImpl(WZ_EARTHSPIKE) {
}

void SkillEarthSpike::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_attack(BF_MAGIC,src,src,target,getSkillId(),skill_lv,tick,flag);
}

void SkillEarthSpike::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	const status_change* sc = status_get_sc(src);

	base_skillratio += 100;
	if (sc && sc->getSCE(SC_EARTH_CARE_OPTION))
		base_skillratio += base_skillratio * 800 / 100;
#endif
}
