// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "documentofsunmoonandstar.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillDocumentofSunMoonAndStar::SkillDocumentofSunMoonAndStar() : SkillImpl(SJ_DOCUMENT) {
}

void SkillDocumentofSunMoonAndStar::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd) {
		switch (skill_lv) {
			case 1:
				pc_resetfeel(sd);
				break;
			case 2:
				pc_resethate(sd);
				break;
			case 3:
				pc_resetfeel(sd);
				pc_resethate(sd);
				break;
		}
	}
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
}
