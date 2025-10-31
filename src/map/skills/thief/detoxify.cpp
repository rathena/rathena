// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "detoxify.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillDetoxify::SkillDetoxify() : SkillImpl(TF_DETOXIFY) {
}

void SkillDetoxify::castendNoDamageId(block_list *src, block_list *bl, uint16 skill_lv, t_tick tick, int32 &flag) const {
	clif_skill_nodamage(src, *bl, getSkillId(), skill_lv);
	status_change_end(bl, SC_POISON);
	status_change_end(bl, SC_DPOISON);
}
