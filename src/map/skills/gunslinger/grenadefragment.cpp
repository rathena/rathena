// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "grenadefragment.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillGrenadeFragment::SkillGrenadeFragment() : SkillImpl(NW_GRENADE_FRAGMENT) {
}

void SkillGrenadeFragment::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change_end(src, skill_get_sc(getSkillId()));
	if (skill_lv < 7)
		sc_start(src, target, (sc_type)(SC_GRENADE_FRAGMENT_1 -1 + skill_lv), 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
	else if (skill_lv == 7) {
		status_change_end(src, SC_GRENADE_FRAGMENT_1);
		status_change_end(src, SC_GRENADE_FRAGMENT_2);
		status_change_end(src, SC_GRENADE_FRAGMENT_3);
		status_change_end(src, SC_GRENADE_FRAGMENT_4);
		status_change_end(src, SC_GRENADE_FRAGMENT_5);
		status_change_end(src, SC_GRENADE_FRAGMENT_6);
	}
	clif_skill_nodamage(src, *src, getSkillId(), skill_lv);
}
