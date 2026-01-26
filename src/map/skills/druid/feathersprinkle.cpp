// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "feathersprinkle.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

#include "skill_factory_druid.hpp"

SkillFeatherSprinkle::SkillFeatherSprinkle() : SkillImplRecursiveDamageSplash(KR_FEATHER_SPRINKLE) {
}

void SkillFeatherSprinkle::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillFeatherSprinkle::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);

	SkillFactoryDruid::try_gain_growth_stacks(src, tick, getSkillId());
}

void SkillFeatherSprinkle::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	int32 skillratio = 1100 + 90 * (skill_lv - 1);
	if (sc && sc->hasSCE(SC_ENRAGE_RAPTOR)) {
		skillratio += 390;
	}
	skillratio += sstatus->dex; // TODO - unknown scaling [munkrej]
	RE_LVL_DMOD(100);
	base_skillratio += -100 + skillratio;
}

int64 SkillFeatherSprinkle::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	return skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}
