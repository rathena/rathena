// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "lexdivina.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillLexDivina::SkillLexDivina() : SkillImpl(PR_LEXDIVINA) {
}

void SkillLexDivina::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	status_change* tsc = status_get_sc(target);
	status_change_entry* tsce = (tsc && type != SC_NONE) ? tsc->getSCE(type) : nullptr;

	if (tsce)
		status_change_end(target, type);
	else
		skill_addtimerskill(src, tick+1000, target->id, 0, 0, getSkillId(), skill_lv, 100, flag);
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
}
