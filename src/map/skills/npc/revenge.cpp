// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "revenge.hpp"

#include "map/mob.hpp"
#include "map/status.hpp"

SkillRevenge::SkillRevenge() : SkillImpl(NPC_REVENGE) {
}

void SkillRevenge::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	mob_data* md = BL_CAST(BL_MOB, src);
	status_data* sstatus = status_get_status_data(*src);

	// not really needed... but adding here anyway ^^
	if (md && md->master_id > 0) {
		block_list *mbl, *tbl;
		if ((mbl = map_id2bl(md->master_id)) == nullptr ||
			(tbl = battle_gettargeted(mbl)) == nullptr)
			return;
		md->state.provoke_flag = tbl->id;
		mob_target(md, tbl, sstatus->rhw.range);
	}
}
