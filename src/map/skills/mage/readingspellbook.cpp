// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "readingspellbook.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillReadingSpellbook::SkillReadingSpellbook() : SkillImpl(WL_READING_SB_READING) {
}

void SkillReadingSpellbook::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd) {
		if (pc_checkskill(sd, WL_READING_SB) == 0 || skill_lv < 1 || skill_lv > 10) {
			clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL_SPELLBOOK_READING );
			return;
		}

		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		skill_spellbook(*sd, ITEMID_WL_MB_SG + skill_lv - 1);
	}
}
