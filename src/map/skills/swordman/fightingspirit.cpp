// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fightingspirit.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillFightingSpirit::SkillFightingSpirit() : SkillImpl(RK_FIGHTINGSPIRIT) {
}

void SkillFightingSpirit::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	uint8 runemastery_skill_lv = (sd ? pc_checkskill(sd, RK_RUNEMASTERY) : skill_get_max(RK_RUNEMASTERY));

	// val1: ATKBonus: ? // !TODO: Confirm new ATK formula
	// val2: ASPD boost: [RK_RUNEMASTERYlevel * 4 / 10] * 10 ==> RK_RUNEMASTERYlevel * 4
	sc_start2(src,target,skill_get_sc(getSkillId()),100,70 + 7 * runemastery_skill_lv,4 * runemastery_skill_lv,skill_get_time(getSkillId(),skill_lv));
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
}
