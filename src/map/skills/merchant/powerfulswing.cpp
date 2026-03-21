// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "powerfulswing.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillPowerfulSwing::SkillPowerfulSwing() : SkillImplRecursiveDamageSplash(MT_POWERFUL_SWING) {
}

void SkillPowerfulSwing::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change* sc = status_get_sc(src);

	skillratio += -100 + 300 + 850 * skill_lv;
	skillratio += 5 * sstatus->pow; // !TODO: check POW ratio
	if (sc && sc->getSCE(SC_AXE_STOMP))
		skillratio += 100 + 100 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillPowerfulSwing::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
