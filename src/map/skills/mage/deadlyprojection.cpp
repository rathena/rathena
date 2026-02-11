// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "deadlyprojection.hpp"

#include <config/const.hpp>

#include "map/status.hpp"

SkillDeadlyProjection::SkillDeadlyProjection() : SkillImpl(AG_DEADLY_PROJECTION) {
}

void SkillDeadlyProjection::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 2800 * skill_lv + 5 * sstatus->spl;
	RE_LVL_DMOD(100);
}

void SkillDeadlyProjection::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_start(src, target, SC_DEADLY_DEFEASANCE, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
	skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
}
