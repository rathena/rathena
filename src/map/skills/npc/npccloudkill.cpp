// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "npccloudkill.hpp"

SkillNpcCloudKill::SkillNpcCloudKill() : SkillImpl(NPC_CLOUD_KILL) {
}

void SkillNpcCloudKill::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += -100 + 50 * skill_lv;
}

void SkillNpcCloudKill::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	flag |= 4;
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}
