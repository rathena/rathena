// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "soulstrike.hpp"

#include "map/status.hpp"

SkillSoulStrike::SkillSoulStrike() : SkillImpl(MG_SOULSTRIKE) {
}

void SkillSoulStrike::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	const status_data *tstatus = status_get_status_data(*target);

	if (battle_check_undead(tstatus->race, tstatus->def_ele))
		base_skillratio += 5 * skill_lv;
}

void SkillSoulStrike::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
}
