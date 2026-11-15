// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hunger.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillHunger::SkillHunger() : SkillImpl(DR_HUNGER) {
}

void SkillHunger::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 80 * skill_lv;

	if (sc != nullptr && sc->hasSCE(SC_ENRAGE_WOLF)) {
		skillratio += 40 * skill_lv;
	}

	skillratio += 3 * sstatus->str;

	RE_LVL_DMOD(100);
}

void SkillHunger::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	int64 damage = skill_attack(BF_WEAPON, src, src, target, getSkillId(), skill_lv, tick, flag);
	if (damage > 0) {
		int32 rate = (skill_lv + 1) / 2;
		int64 heal = damage * rate / 100;

		if (heal > 100000) {
			heal = 100000;
		}

		status_heal(src, heal, 0, 0, 2);
	}
}
