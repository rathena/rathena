// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "chasingbreak.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/path.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillChasingBreak::SkillChasingBreak() : SkillImplRecursiveDamageSplash(ABC_CHASING_BREAK) {
}

void SkillChasingBreak::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 1550 + 450 * skill_lv;
	skillratio += 5 * sstatus->pow;
	if (sc != nullptr && sc->hasSCE(SC_CHASING))
		skillratio += 200 + 50 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillChasingBreak::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	uint8 dir = DIR_NORTHEAST;

	if (target->x != src->x || target->y != src->y)
		dir = map_calc_dir(target, src->x, src->y);

	if (skill_check_unit_movepos(0, src, target->x + dirx[dir], target->y + diry[dir], 1, 1))
		clif_blown(src);
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, 1);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
