// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "npcvenomimpress.hpp"

#include "map/status.hpp"

SkillNpcVenomImpress::SkillNpcVenomImpress() : SkillImpl(NPC_VENOMIMPRESS) {
}

void SkillNpcVenomImpress::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	// TODO: refactor to applyAdditionalEffects
	if (skill_attack(BF_WEAPON, src, src, target, getSkillId(), skill_lv, tick, flag))
		sc_start(src, target, SC_VENOMIMPRESS, 100, skill_lv, skill_get_time(getSkillId(),skill_lv));
}
