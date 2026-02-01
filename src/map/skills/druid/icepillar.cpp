// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "icepillar.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillIcePillar::SkillIcePillar() : SkillImplRecursiveDamageSplash(KR_ICE_PILLAR) {
}

void SkillIcePillar::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	if (mflag & SKILL_ALTDMG_FLAG)
		skillratio += -100 + 450 + 50 * (skill_lv - 1);	// Fog damage
	else {
		// Initial damage
		skillratio += -100 + 720 + 120 * (skill_lv - 1);

		// SC_TRUTH_OF_ICE only increases the initial damage
		if (sc != nullptr && sc->hasSCE(SC_TRUTH_OF_ICE)) {
			skillratio += 600;
		}
	}

	// INT ratio and BaseLevel modifier are not part of SC_TRUTH_OF_ICE
	skillratio += 4 * sstatus->int_;
	RE_LVL_DMOD(100);
}

void SkillIcePillar::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
	if (!(flag & 1)) {
		skill_unitsetting(src, getSkillId(), skill_lv, target->x, target->y, 0);
	}
}

void SkillIcePillar::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	SkillImplRecursiveDamageSplash::castendPos2(src, x, y, skill_lv, tick, flag);
	skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);
}
