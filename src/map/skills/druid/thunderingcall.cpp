// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "thunderingcall.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

#include "skill_factory_druid.hpp"

SkillThunderingCall::SkillThunderingCall() : SkillImplRecursiveDamageSplash(KR_THUNDERING_CALL) {
}

void SkillThunderingCall::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change* sc = status_get_sc(src);

	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);

	try_gain_thundering_charge(src, sc, getSkillId(), 1);
}

void SkillThunderingCall::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	int32 skillratio = 5200 + 200 * (skill_lv - 1);
	if (sc && sc->hasSCE(SC_TRUTH_OF_WIND)) {
		skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
		RE_LVL_DMOD(100);
	}
	base_skillratio += -100 + skillratio;
}

int64 SkillThunderingCall::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	e_skill actual_skill = getSkillId();
	const status_change* sc = status_get_sc(src);
	actual_skill = resolve_thundering_charge_skill(sc, actual_skill);

	return skill_attack(skill_get_type(actual_skill), src, src, target, actual_skill, skill_lv, tick, flag);
}

SkillThunderingCallS::SkillThunderingCallS() : SkillImplRecursiveDamageSplash(KR_THUNDERING_CALL_S) {
}

void SkillThunderingCallS::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillThunderingCallS::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	int32 skillratio = 9500 + 500 * (skill_lv - 1);
	if (sc && sc->hasSCE(SC_TRUTH_OF_WIND)) {
		skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
		RE_LVL_DMOD(100);
	}
	base_skillratio += -100 + skillratio;
}

int64 SkillThunderingCallS::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	return skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}
