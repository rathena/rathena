// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "flashcombo.hpp"

#include "map/pc.hpp"
#include "map/status.hpp"

SkillFlashCombo::SkillFlashCombo() : SkillImpl(SR_FLASHCOMBO) {
}

void SkillFlashCombo::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	const int32 combo[] = { SR_DRAGONCOMBO, SR_FALLENEMPIRE, SR_TIGERCANNON };
	const int32 delay[] = { 0, 750, 1250 };

	if (sd) // Disable attacking/acting/moving for skill's duration.
		sd->ud.attackabletime = sd->canuseitem_tick = sd->ud.canact_tick = tick + delay[2];

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv,
		sc_start(src, src, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));

	for (int32 i = 0; i < ARRAYLENGTH(combo); i++)
		skill_addtimerskill(src,tick + delay[i],target->id,0,0,combo[i],skill_lv,BF_WEAPON,flag|SD_LEVEL);
}
