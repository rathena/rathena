// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "glacialnova.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillGlacialNova::SkillGlacialNova() : SkillImplRecursiveDamageSplash(AT_GLACIER_NOVA) {
}

void SkillGlacialNova::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 15000;

	// SPL and BaseLevel ratio do not depend on SC_TRUTH_OF_ICE
	skillratio += 15 * sstatus->spl;

	RE_LVL_DMOD(100);
}

void SkillGlacialNova::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change* sc = status_get_sc(src);

	if (sc == nullptr)
		return;

	const status_change_entry *sce = sc->getSCE(SC_GLACIER_SHEILD);

	if (sce == nullptr)
		return;

	if (src->m != sce->val4)
		return;

	// TODO : Should the distance to the player be checked?
	// On official server SC_GLACIER_SHEILD does not save the position of glacial monolith

	SkillImplRecursiveDamageSplash::castendPos2(src, sce->val2, sce->val3, skill_lv, tick, flag);
}
