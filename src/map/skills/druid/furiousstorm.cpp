// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "furiousstorm.hpp"

#include "map/clif.hpp"

SkillFuriousStorm::SkillFuriousStorm() : SkillImplRecursiveDamageSplash(AT_FURIOS_STORM) {
}

void SkillFuriousStorm::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillFuriousStorm::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillFuriousStorm::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	int32 skillratio = 7800 + 400 * (skill_lv - 1);
	base_skillratio += -100 + skillratio;
}

int64 SkillFuriousStorm::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	return skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}
