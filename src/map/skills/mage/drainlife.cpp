// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "drainlife.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillDrainLife::SkillDrainLife() : SkillImpl(WL_DRAINLIFE) {
}

void SkillDrainLife::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 200 * skill_lv + sstatus->int_;
	RE_LVL_DMOD(100);
}

void SkillDrainLife::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 heal = (int32)skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
	int32 rate = 70 + 5 * skill_lv;

	heal = heal * (5 + 5 * skill_lv) / 100;

	if( target->type == BL_SKILL )
		heal = 0; // Don't absorb heal from Ice Walls or other skill units.

	if( heal && rnd()%100 < rate )
	{
		status_heal(src, heal, 0, 0);
		clif_skill_nodamage(nullptr, *src, AL_HEAL, heal);
	}
}
