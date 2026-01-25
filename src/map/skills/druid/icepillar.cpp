// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "icepillar.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillIcePillar::SkillIcePillar() : SkillImplRecursiveDamageSplash(KR_ICE_PILLAR) {
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

void SkillIcePillar::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	const bool alt_damage = (mflag & SKILL_ALTDMG_FLAG) != 0;
	int32 skillratio = alt_damage ? (450 + 50 * (skill_lv - 1)) : (720 + 120 * (skill_lv - 1));
	if (sc && sc->hasSCE(SC_TRUTH_OF_ICE)) {
		skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
		RE_LVL_DMOD(100);
	}
	base_skillratio += -100 + skillratio;
}

int64 SkillIcePillar::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	return skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}
