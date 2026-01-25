// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "napalmvulcan.hpp"

#include <config/const.hpp>
#include <config/core.hpp>

#include "map/status.hpp"

SkillNapalmVulcan::SkillNapalmVulcan() : SkillImplRecursiveDamageSplash(HW_NAPALMVULCAN) {
}

void SkillNapalmVulcan::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
#ifdef RENEWAL
	skillratio += -100 + 70 * skill_lv;
	RE_LVL_DMOD(100);
#else
	skillratio += 25;
#endif
}

void SkillNapalmVulcan::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src,target,SC_CURSE,5*skill_lv,skill_lv,skill_get_time2(getSkillId(),skill_lv));
}
