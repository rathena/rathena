// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "roundtrip.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

// RL_R_TRIP
SkillRoundTrip::SkillRoundTrip() : SkillImplRecursiveDamageSplash(RL_R_TRIP) {
}

void SkillRoundTrip::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	skillratio += -100 + 350 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillRoundTrip::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	skill_castend_damage_id(src, target, getSkillId(), skill_lv, tick, flag);
}


// RL_R_TRIP_PLUSATK
SkillRoundTripPlusAttack::SkillRoundTripPlusAttack() : SkillImpl(RL_R_TRIP_PLUSATK) {
}

void SkillRoundTripPlusAttack::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += -100 + 300 + 300 * skill_lv;
}
