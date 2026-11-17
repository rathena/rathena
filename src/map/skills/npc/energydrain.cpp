// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "energydrain.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillEnergyDrain::SkillEnergyDrain() : SkillImpl(NPC_ENERGYDRAIN) {
}

void SkillEnergyDrain::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 100 * skill_lv;
}

void SkillEnergyDrain::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 heal = (int32)skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
	if (heal > 0){
		clif_skill_nodamage(nullptr, *src, AL_HEAL, heal);
		status_heal(src, heal, 0, 0);
	}
}
