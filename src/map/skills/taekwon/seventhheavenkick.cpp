// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "seventhheavenkick.hpp"

#include "map/skill.hpp"
#include "map/status.hpp"

SkillSeventhHeavenKick::SkillSeventhHeavenKick() : WeaponSkillImpl(SKE_SEVENTH_KICK_S) {
}

void SkillSeventhHeavenKick::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 7770 * (skill_lv + 1);
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}
