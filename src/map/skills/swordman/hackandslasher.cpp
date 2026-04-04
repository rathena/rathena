// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hackandslasher.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillHackAndSlasher::SkillHackAndSlasher() : SkillImplRecursiveDamageSplash(DK_HACKANDSLASHER) {
}

void SkillHackAndSlasher::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 350 + 820 * skill_lv;
	skillratio += 7 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillHackAndSlasher::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}

SkillHackAndSlasherAttack::SkillHackAndSlasherAttack() : SkillImpl(DK_HACKANDSLASHER_ATK) {
}

void SkillHackAndSlasherAttack::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 350 + 820 * skill_lv;
	skillratio += 7 * sstatus->pow;
	RE_LVL_DMOD(100);
}
