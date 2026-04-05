// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "energycannonade.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillEnergyCannonade::SkillEnergyCannonade() : SkillImplRecursiveDamageSplash(MT_ENERGY_CANNONADE) {
}

void SkillEnergyCannonade::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 250 + 750 * skill_lv;
	skillratio += 5 * sstatus->pow; // !TODO: check POW ratio
	RE_LVL_DMOD(100);
}

void SkillEnergyCannonade::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
