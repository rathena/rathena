// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "frenzyshot.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillFrenzyShot::SkillFrenzyShot() : WeaponSkillImpl(ABC_FRENZY_SHOT) {
}

void SkillFrenzyShot::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillFrenzyShot::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 250 + 800 * skill_lv;
	skillratio += 15 * sstatus->con;
	RE_LVL_DMOD(100);
}
