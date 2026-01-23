// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "astralstrikeattack.hpp"

#include <config/const.hpp>

#include "map/status.hpp"

SkillAstralStrikeAttack::SkillAstralStrikeAttack() : SkillImpl(AG_ASTRAL_STRIKE_ATK) {
}

void SkillAstralStrikeAttack::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 650 * skill_lv + 10 * sstatus->spl;
	RE_LVL_DMOD(100);
}

void SkillAstralStrikeAttack::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_attack(BF_MAGIC,src,src,target,getSkillId(),skill_lv,tick,flag);
}
