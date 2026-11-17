// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hellsjudgement2.hpp"

#include <common/random.hpp>

#include "map/status.hpp"

SkillHellsJudgement2::SkillHellsJudgement2() : SkillImplRecursiveDamageSplash(NPC_HELLJUDGEMENT2) {
}

void SkillHellsJudgement2::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	switch(rnd()%6) {
	case 0:
		sc_start(src,target,SC_SLEEP,100,skill_lv,skill_get_time2(getSkillId(),skill_lv));
		break;
	case 1:
		sc_start(src,target,SC_CONFUSION,100,skill_lv,skill_get_time2(getSkillId(),skill_lv));
		break;
	case 2:
		sc_start(src,target,SC_HALLUCINATION,100,skill_lv,skill_get_time2(getSkillId(),skill_lv));
		break;
	case 3:
		sc_start(src,target,SC_STUN,100,skill_lv,skill_get_time2(getSkillId(),skill_lv));
		break;
	case 4:
		sc_start(src,target,SC_FEAR,100,skill_lv,skill_get_time2(getSkillId(),skill_lv));
		break;
	default:
		sc_start(src,target,SC_CURSE,100,skill_lv,skill_get_time2(getSkillId(),skill_lv));
		break;
	}
}

void SkillHellsJudgement2::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 100 * (skill_lv - 1);
}

void SkillHellsJudgement2::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_castend_damage_id(src, src, getSkillId(), skill_lv, tick, flag);
}
