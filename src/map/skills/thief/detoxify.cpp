// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "detoxify.hpp"

#include "map/status.hpp"
#include "map/clif.hpp"

SkillDetoxify::SkillDetoxify() : WeaponSkillImpl(TF_DETOXIFY) {
}

int32 SkillDetoxify::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *bl, this->skill_id, skill_lv);
	status_change_end(bl, SC_POISON);
	status_change_end(bl, SC_DPOISON);
	return 1;
}