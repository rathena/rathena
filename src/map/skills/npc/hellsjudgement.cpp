// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hellsjudgement.hpp"

#include "map/status.hpp"

SkillHellsJudgement::SkillHellsJudgement() : SkillImplRecursiveDamageSplash(NPC_HELLJUDGEMENT) {
}

void SkillHellsJudgement::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src,target,SC_CURSE,100,skill_lv,skill_get_time2(getSkillId(),skill_lv));
}

void SkillHellsJudgement::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 100 * (skill_lv - 1);
}

void SkillHellsJudgement::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_castend_damage_id(src, src, getSkillId(), skill_lv, tick, flag);
}
