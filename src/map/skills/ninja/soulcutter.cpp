// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "soulcutter.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillSoulCutter::SkillSoulCutter() : WeaponSkillImpl(KO_SETSUDAN) {
}

void SkillSoulCutter::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	// Remove soul link when hit.
	status_change_end(target, SC_SPIRIT);
	status_change_end(target, SC_SOULGOLEM);
	status_change_end(target, SC_SOULSHADOW);
	status_change_end(target, SC_SOULFALCON);
	status_change_end(target, SC_SOULFAIRY);
}

void SkillSoulCutter::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_change *tsc = status_get_sc(target);

	skillratio += 100 * (skill_lv - 1);
	RE_LVL_DMOD(100);
	if (tsc) {
		const status_change_entry *sce;

		if ((sce = tsc->getSCE(SC_SPIRIT)) || (sce = tsc->getSCE(SC_SOULGOLEM)) || (sce = tsc->getSCE(SC_SOULSHADOW)) || (sce = tsc->getSCE(SC_SOULFALCON)) || (sce = tsc->getSCE(SC_SOULFAIRY))) // Bonus damage added when target is soul linked.
			skillratio += 200 * sce->val1;
	}
}
