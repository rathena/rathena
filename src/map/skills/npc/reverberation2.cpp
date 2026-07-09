// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "reverberation2.hpp"

// NPC_REVERBERATION
SkillReverberation2::SkillReverberation2() : SkillImpl(NPC_REVERBERATION) {
}

void SkillReverberation2::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	flag|=1; // Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}


// NPC_REVERBERATION_ATK
SkillReverberationAttack::SkillReverberationAttack() : SkillImplRecursiveDamageSplash(NPC_REVERBERATION_ATK) {
}

void SkillReverberationAttack::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 400 + 200 * skill_lv;
}

int32 SkillReverberationAttack::getSplashTarget(block_list* src) const {
	return splash_target(src);
}

void SkillReverberationAttack::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_area_temp[1] = 0;

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
