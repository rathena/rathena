// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "thunderingorb.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

#include "skill_factory_druid.hpp"

SkillThunderingOrb::SkillThunderingOrb() : SkillImplRecursiveDamageSplash(KR_THUNDERING_ORB) {
}

void SkillThunderingOrb::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change* sc = status_get_sc(src);

	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);

	SkillFactoryDruid::try_gain_thundering_charge(src, sc, getSkillId(), 1);
}

void SkillThunderingOrb::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	int32 skillratio = 1400 + 70 * (skill_lv - 1);
	if (sc != nullptr && sc->hasSCE(SC_TRUTH_OF_WIND)) {
		skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
		RE_LVL_DMOD(100);
	}
	base_skillratio += -100 + skillratio;
}

int64 SkillThunderingOrb::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	e_skill actual_skill = getSkillId();
	const status_change* sc = status_get_sc(src);
	actual_skill = SkillFactoryDruid::resolve_thundering_charge_skill(sc, actual_skill);

	return skill_attack(skill_get_type(actual_skill), src, src, target, actual_skill, skill_lv, tick, flag);
}

SkillThunderingOrbS::SkillThunderingOrbS() : SkillImplRecursiveDamageSplash(KR_THUNDERING_ORB_S) {
}

void SkillThunderingOrbS::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillThunderingOrbS::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	int32 skillratio = 1750 + 100 * (skill_lv - 1);
	if (sc != nullptr && sc->hasSCE(SC_TRUTH_OF_WIND)) {
		skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
		RE_LVL_DMOD(100);
	}
	base_skillratio += -100 + skillratio;
}

int64 SkillThunderingOrbS::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	return skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}
