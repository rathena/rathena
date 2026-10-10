// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "punitio.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/path.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillPunitio::SkillPunitio() : WeaponSkillImpl(CD_PUNITIO) {
}

void SkillPunitio::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 550 * skill_lv + 1000 + 35 * pc_checkskill(sd, CD_MACE_BOOK_M);
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillPunitio::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (src->x != target->x || src->y != target->y) {
		uint8 dir = map_calc_dir(target, src->x, src->y);

		if (unit_movepos(src, target->x + dirx[dir], target->y + diry[dir], 2, true))
			clif_blown(src);
	}

	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}
