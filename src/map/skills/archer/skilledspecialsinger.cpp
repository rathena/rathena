// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skilledspecialsinger.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillSkilledSpecialSinger::SkillSkilledSpecialSinger() : SkillImpl(CG_SPECIALSINGER) {
}

void SkillSkilledSpecialSinger::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);

	if (tsc && tsc->getSCE(SC_ENSEMBLEFATIGUE)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		status_change_end(target, SC_ENSEMBLEFATIGUE);
	}
}
