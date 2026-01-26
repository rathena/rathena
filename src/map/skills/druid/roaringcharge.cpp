// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "roaringcharge.hpp"

#include "map/clif.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

#include "skill_factory_druid.hpp"

SkillRoaringCharge::SkillRoaringCharge() : SkillImplRecursiveDamageSplash(AT_ROARING_CHARGE) {
}

void SkillRoaringCharge::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillRoaringCharge::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change* sc = status_get_sc(src);

	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
	if (!(flag & 1)) {
		SkillFactoryDruid::try_gain_thundering_charge(src, sc, getSkillId(), 1 + skill_lv);
	}
}

void SkillRoaringCharge::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	int32 skillratio = 8000 + 400 * (skill_lv - 1);
	if (sc && sc->hasSCE(SC_TRUTH_OF_WIND)) {
		skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
		RE_LVL_DMOD(100);
	}
	base_skillratio += -100 + skillratio;
}

int64 SkillRoaringCharge::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	e_skill actual_skill = getSkillId();
	const status_change* sc = status_get_sc(src);
	actual_skill = SkillFactoryDruid::resolve_thundering_charge_skill(sc, actual_skill);

	return skill_attack(skill_get_type(actual_skill), src, src, target, actual_skill, skill_lv, tick, flag);
}

SkillRoaringChargeS::SkillRoaringChargeS() : SkillImplRecursiveDamageSplash(AT_ROARING_CHARGE_S) {
}

void SkillRoaringChargeS::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillRoaringChargeS::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillRoaringChargeS::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	int32 skillratio = 11500 + 500 * (skill_lv - 1);
	if (sc && sc->hasSCE(SC_TRUTH_OF_WIND)) {
		skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
		RE_LVL_DMOD(100);
	}
	base_skillratio += -100 + skillratio;
}

int64 SkillRoaringChargeS::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	return skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}
