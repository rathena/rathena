// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "raisingdragon.hpp"

#include "map/pc.hpp"
#include "map/status.hpp"

SkillRaisingDragon::SkillRaisingDragon() : SkillImpl(SR_RAISINGDRAGON) {
}

void SkillRaisingDragon::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	sc_type type = skill_get_sc(getSkillId());

	if (sd == nullptr)
		return;

	int16 max = 5 + skill_lv;
	sc_start(src, target, SC_EXPLOSIONSPIRITS, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
	for (int32 i = 0; i < max; i++) // Don't call more than max available spheres.
		pc_addspiritball(sd, skill_get_time(getSkillId(), skill_lv), max);
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start(src, target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
}
