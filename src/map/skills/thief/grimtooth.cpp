// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "grimtooth.hpp"

#include "map/mob.hpp"
#include "map/status.hpp"

SkillGrimtooth::SkillGrimtooth() : SkillImplRecursiveDamageSplash(AS_GRIMTOOTH) {
}

void SkillGrimtooth::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 20 * skill_lv;
}

void SkillGrimtooth::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	flag |= SD_PREAMBLE; // a fake packet will be sent for the first target to be hit

	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillGrimtooth::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	status_data* tstatus = status_get_status_data(*target);
	mob_data* dstmd = BL_CAST(BL_MOB, target);

	if (dstmd && !status_has_mode(tstatus,MD_STATUSIMMUNE))
		sc_start(src,target,SC_QUAGMIRE,100,0,skill_get_time2(getSkillId(),skill_lv));
}
