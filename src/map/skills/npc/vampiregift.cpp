// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "vampiregift.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillVampireGift::SkillVampireGift() : SkillImplRecursiveDamageSplash(NPC_VAMPIRE_GIFT) {
}

void SkillVampireGift::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += ((skill_lv - 1) % 5 + 1) * 100;
}

void SkillVampireGift::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_castend_damage_id(src, src, getSkillId(), skill_lv, tick, flag);
}

int64 SkillVampireGift::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	int32 heal = static_cast<int32>(SkillImplRecursiveDamageSplash::splashDamage(src, target, skill_lv, tick, flag));

	if (heal > 0) {
		clif_skill_nodamage(nullptr, *src, AL_HEAL, heal);
		status_heal(src, heal, 0, 0);
	}

	return heal;
}
