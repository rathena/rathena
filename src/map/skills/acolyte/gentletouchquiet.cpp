// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "gentletouchquiet.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillGentleTouchQuiet::SkillGentleTouchQuiet() : WeaponSkillImpl(SR_GENTLETOUCH_QUIET) {
}

void SkillGentleTouchQuiet::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	// [(Skill Level x 5) + (Caster?s DEX + Caster?s Base Level) / 10]
	sc_start(src,target, SC_SILENCE, 5 * skill_lv + (status_get_dex(src) + status_get_lv(src)) / 10, skill_lv, skill_get_time(getSkillId(), skill_lv));
}

void SkillGentleTouchQuiet::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 100 * skill_lv + sstatus->dex;
	RE_LVL_DMOD(100);
}
