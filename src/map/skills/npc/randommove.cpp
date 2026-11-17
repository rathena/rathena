// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "randommove.hpp"

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/unit.hpp"

SkillRandomMove::SkillRandomMove() : SkillImpl(NPC_RANDOMMOVE) {
}

void SkillRandomMove::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	mob_data* md = BL_CAST(BL_MOB, src);

	if (md != nullptr) {
		// This skill creates fake casting state where a monster moves while showing a cast bar
		int32 tricktime = MOB_SKILL_INTERVAL * 3;
		md->trickcasting = tick + tricktime;
		clif_skillcasting(*src, src, 0, 0, getSkillId(), skill_lv, ELE_FIRE, tricktime + MOB_SKILL_INTERVAL / 2);
		// Monster cannot be stopped while moving
		md->state.can_escape = 1;
		// Move up to 8 cells
		unit_escape(md, target, 8, 3);
	}
}
