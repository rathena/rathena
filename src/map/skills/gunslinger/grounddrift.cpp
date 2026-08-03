// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "grounddrift.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillGroundDrift::SkillGroundDrift() : SkillImpl(GS_GROUNDDRIFT) {
}

void SkillGroundDrift::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	const status_data* sstatus = status_get_status_data(src);

	dmg.amotion = sstatus->amotion;
	dmg.blewcount = 0;
}

void SkillGroundDrift::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	base_skillratio += 100 + 20 * skill_lv;
#endif
}

void SkillGroundDrift::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	// Ammo should be deleted right away.
	skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);
}

void SkillGroundDrift::modifyElement(const Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv, int32& element, int32 flag) const {
	element = dmg.miscflag; // element comes in flag.
}
