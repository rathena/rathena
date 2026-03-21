// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "psychicstream.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/path.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillPsychicStream::SkillPsychicStream() : SkillImplRecursiveDamageSplash(EM_PSYCHIC_STREAM) {
}

void SkillPsychicStream::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 1000 + 3500 * skill_lv;
	skillratio += 5 * sstatus->spl;
	RE_LVL_DMOD(100);
}

void SkillPsychicStream::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	uint8 dir = DIR_NORTHEAST;

	if (target->x != src->x || target->y != src->y)
		dir = map_calc_dir(target, src->x, src->y);	// dir based on target as we move player based on target location

	if (skill_check_unit_movepos(0, src, target->x + dirx[dir], target->y + diry[dir], 1, 1)) {
		clif_blown(src);
		skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}
	else {
		if (sd != nullptr)
			clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);

		// TODO: Should we return here?
	}

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
