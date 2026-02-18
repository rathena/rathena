// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "cruelbite.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/unit.hpp"

SkillCruelBite::SkillCruelBite() : WeaponSkillImpl(DR_CRUEL_BITE) {
}

void SkillCruelBite::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 60 * skill_lv;

	if (sc != nullptr && sc->hasSCE(SC_ENRAGE_WOLF)) {
		skillratio += 20 * skill_lv;
	}

	skillratio += 4 * sstatus->str;

	RE_LVL_DMOD(100);
}

void SkillCruelBite::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	if (!unit_movepos(src, target->x, target->y, 2, true)) {
		if (map_session_data *sd = BL_CAST(BL_PC, src); sd != nullptr) {
			clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
		}
		return;
	}

	clif_blown(src);
	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}
