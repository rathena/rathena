// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "beaststrafing.hpp"

#include "map/status.hpp"

SkillBeastStrafing::SkillBeastStrafing() : SkillImpl(HT_POWER) {
}

void SkillBeastStrafing::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* tstatus = status_get_status_data(*target);

	if( tstatus->race == RC_BRUTE || tstatus->race == RC_PLAYER_DORAM || tstatus->race == RC_INSECT )
		skill_attack(BF_WEAPON,src,src,target,getSkillId(),skill_lv,tick,flag);
}

void SkillBeastStrafing::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	base_skillratio += -50 + 8 * sstatus->str;
}
