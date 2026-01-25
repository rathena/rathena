// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "cruelbite.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/unit.hpp"

SkillCruelBite::SkillCruelBite() : SkillImplRecursiveDamageSplash(DR_CRUEL_BITE) {
}

void SkillCruelBite::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	if (!(flag & 1)) {
		if (!unit_movepos(src, target->x, target->y, 2, true)) {
			map_session_data *sd = BL_CAST(BL_PC, src);
			if (sd) {
				clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
			}
			return;
		}
		clif_blown(src);
	}
	skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillCruelBite::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	int32 skillratio = 60 * skill_lv;
	if (sc && sc->getSCE(SC_ENRAGE_WOLF)) {
		skillratio += 20 * skill_lv;
	}
	skillratio += sstatus->str; // TODO - unknown scaling [munkrej]
	RE_LVL_DMOD(100);
	base_skillratio += -100 + skillratio;
}
