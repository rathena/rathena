// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "firewalk.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillFireWalk::SkillFireWalk() : SkillImpl(SO_FIREWALK) {
}

void SkillFireWalk::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change* sc = status_get_sc(src);
	sc_type type = skill_get_sc(getSkillId());

	if (sc && sc->getSCE(type))
		status_change_end(src, type);

	sc_start2(src, src, type, 100, getSkillId(), skill_lv, skill_get_time(getSkillId(), skill_lv));
}

void SkillFireWalk::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* sc = status_get_sc(src);

	skillratio += -100 + 60 * skill_lv;
	RE_LVL_DMOD(100);
	if( sc && sc->getSCE(SC_HEATER_OPTION) )
		skillratio += (sd ? sd->status.job_level / 2 : 0);
}
