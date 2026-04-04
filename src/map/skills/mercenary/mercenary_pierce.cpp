// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mercenary_pierce.hpp"

#include "map/status.hpp"

SkillMercenaryPierce::SkillMercenaryPierce() : WeaponSkillImpl(ML_PIERCE) {
}

void SkillMercenaryPierce::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	const status_data* tstatus = status_get_status_data(target);

	dmg.div_= (dmg.div_> 0 ? tstatus->size+1 : -(tstatus->size+1));
}

void SkillMercenaryPierce::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 10 * skill_lv;
}

void SkillMercenaryPierce::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	hit_rate += hit_rate * 5 * skill_lv / 100;
}
