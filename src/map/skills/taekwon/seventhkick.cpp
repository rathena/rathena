// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "seventhkick.hpp"

#include "map/battle.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

SkillSeventhKick::SkillSeventhKick() : WeaponSkillImpl(SKE_SEVENTH_KICK) {
}

void SkillSeventhKick::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 7000 * skill_lv;
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillSeventhKick::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change* sc = status_get_sc(src);

	if (sc != nullptr && sc->hasSCE(SC_SEVENTH_KICK_MAX)) {
		status_change_end(src, SC_SEVENTH_KICK_MAX);
		status_change_end(src, SC_SEVENTH_KICK_SKILLORB);

		skill_attack(BF_WEAPON, src, src, target, SKE_SEVENTH_KICK_S, skill_lv, tick, flag);
		return;
	}

	skill_attack(BF_WEAPON, src, src, target, SKE_SEVENTH_KICK, skill_lv, tick, flag);

	int32 lit = 1;
	if (sc != nullptr && sc->hasSCE(SC_SEVENTH_KICK_SKILLORB))
		lit += sc->getSCE(SC_SEVENTH_KICK_SKILLORB)->val1;
	lit = std::min(7, lit);
	sc_start(src, src, SC_SEVENTH_KICK_SKILLORB, 100, lit, 10000);
	if (lit >= 7)
		sc_start(src, src, SC_SEVENTH_KICK_MAX, 100, 1, 10000);
}
