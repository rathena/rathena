// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "chillingblast.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

#include "glacialnova.hpp"

SkillChillingBlast::SkillChillingBlast() : SkillImplRecursiveDamageSplash(AT_CHILLING_BLAST) {
}

void SkillChillingBlast::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 8400 + 1500 * (skill_lv - 1);

	// SPL and BaseLevel ratio do not depend on SC_TRUTH_OF_ICE
	skillratio += 20 * sstatus->spl;

	RE_LVL_DMOD(100);
}

void SkillChillingBlast::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	this->castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillChillingBlast::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);

	SkillGlacialNova skillnova;
	skillnova.castendPos2(src, 0, 0, 1, tick, flag);
}
