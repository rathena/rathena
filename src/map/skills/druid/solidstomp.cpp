// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "solidstomp.hpp"

#include "map/clif.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

#include "skill_factory_druid.hpp"

SkillSolidStomp::SkillSolidStomp() : SkillImplRecursiveDamageSplash(AT_SOLID_STOMP) {
}

void SkillSolidStomp::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillSolidStomp::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	if (!(flag & 1)) {
		sc_type type = skill_get_sc(getSkillId());
		if (type != SC_NONE) {
			sc_start(src, src, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
		}
	}

	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);

	if (!(flag & 1)) {
		int64 heal = static_cast<int64>(status_get_max_hp(src)) * skill_lv / 100;
		if (heal > 0) {
			status_heal(src, heal, 0, 0);
		}
	}
	if (!(flag & 1)) {
		SkillFactoryDruid::try_gain_growth_stacks(src, tick, getSkillId());
	}
}

void SkillSolidStomp::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	int32 skillratio = 10400 + 800 * (skill_lv - 1);
	base_skillratio += -100 + skillratio;
}

int64 SkillSolidStomp::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	return skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}
