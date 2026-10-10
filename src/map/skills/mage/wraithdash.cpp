// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "wraithdash.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/path.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillWraithDash::SkillWraithDash() : SkillImplRecursiveDamageSplash(AG_WRAITH_DASH) {
}

void SkillWraithDash::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 4000 * skill_lv + 3000;
}

void SkillWraithDash::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *src, getSkillId(), skill_lv);
	skill_area_temp[0] = 0;
	skill_area_temp[1] = src->id;
	skill_area_temp[2] = 0;
	this->splashSearch(src, src, skill_lv, tick, flag);

	// Dash up to 7 cells forward; stop before the first obstacle.
	uint8 dir = unit_getdir(src);

	for (int32 dist = 7; dist > 0; dist--) {
		int16 x = src->x + dirx[dir] * dist;
		int16 y = src->y + diry[dir] * dist;

		if (map_getcell(src->m, x, y, CELL_CHKNOPASS))
			continue;
		if (!path_search_long(nullptr, src->m, src->x, src->y, x, y, CELL_CHKNOPASS))
			continue;
		if (unit_movepos(src, x, y, 1, true)) {
			clif_blown(src);
			break;
		}
	}
}
