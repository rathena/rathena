// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "antidote.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillAntidote::SkillAntidote() : SkillImpl(GC_ANTIDOTE) {
}

void SkillAntidote::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);

	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	if( tsc )
	{
		status_change_end(target, SC_PARALYSE);
		status_change_end(target, SC_PYREXIA);
		status_change_end(target, SC_DEATHHURT);
		status_change_end(target, SC_LEECHESEND);
		status_change_end(target, SC_VENOMBLEED);
		status_change_end(target, SC_MAGICMUSHROOM);
		status_change_end(target, SC_TOXIN);
		status_change_end(target, SC_OBLIVIONCURSE);
	}
}
