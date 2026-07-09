// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "duplelight.hpp"

// AB_DUPLELIGHT_MAGIC
SkillDupleLightMagic::SkillDupleLightMagic() : SkillImpl(AB_DUPLELIGHT_MAGIC) {
}

void SkillDupleLightMagic::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 300 + 40 * skill_lv;
}

void SkillDupleLightMagic::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_attack(BF_MAGIC,src,src,target,getSkillId(),skill_lv,tick,flag);
}


// AB_DUPLELIGHT_MELEE
SkillDupleLightMelee::SkillDupleLightMelee() : WeaponSkillImpl(AB_DUPLELIGHT_MELEE) {
}

void SkillDupleLightMelee::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 50 + 15 * skill_lv;
}
