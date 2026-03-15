// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "improvisedsong.hpp"

#include <common/random.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillImprovisedSong::SkillImprovisedSong() : SkillImpl(WM_RANDOMIZESPELL) {
}

void SkillImprovisedSong::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (rnd() % 100 < 30 + (10 * skill_lv)) {
		status_change_end(target, SC_SONGOFMANA);
		status_change_end(target, SC_DANCEWITHWUG);
		status_change_end(target, SC_LERADSDEW);
		status_change_end(target, SC_SATURDAYNIGHTFEVER);
		status_change_end(target, SC_BEYONDOFWARCRY);
		status_change_end(target, SC_MELODYOFSINK);
		status_change_end(target, SC_BEYONDOFWARCRY);
		status_change_end(target, SC_UNLIMITEDHUMMINGVOICE);

		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}
}
