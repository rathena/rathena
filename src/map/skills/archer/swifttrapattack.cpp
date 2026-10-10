// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "swifttrapattack.hpp"

#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

SkillSwiftTrapAttack::SkillSwiftTrapAttack() : SkillImplRecursiveDamageSplash(WH_SWIFTTRAP_ATK) {
}

void SkillSwiftTrapAttack::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 3950 * skill_lv + 50 * pc_checkskill(sd, WH_ADVANCED_TRAP);
	skillratio += 5 * sstatus->con;
	RE_LVL_DMOD(100);
}

void SkillSwiftTrapAttack::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	// Advanced Trap learned: +1 AP on top of the skill_db GiveAp.
	if (sd != nullptr && pc_checkskill(sd, WH_ADVANCED_TRAP) > 0)
		status_heal(src, 0, 0, 1, 0);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
