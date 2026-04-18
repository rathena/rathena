// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_needlestinger.hpp"

#include "map/status.hpp"

SkillNeedleStinger::SkillNeedleStinger() : SkillImpl(MH_NEEDLE_STINGER) {
}

void SkillNeedleStinger::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillNeedleStinger::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	base_skillratio += -100 + 200 + 500 * skill_lv * status_get_lv(src) / 100 + sstatus->dex; // !TODO: Confirm Base Level and DEX bonus
}
