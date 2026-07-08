// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "nightmareerasion.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillNightmareErasion::SkillNightmareErasion() : SkillImpl(SS_AKUMUKESU) {
}

void SkillNightmareErasion::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (flag & 1) {
		status_change_end(target, SC_NIGHTMARE);
	} else {
		int32 range = skill_get_splash( getSkillId(), skill_lv );

		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

		map_foreachinrange( skill_area_sub, target, range, BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_nodamage_id );
	}
}
