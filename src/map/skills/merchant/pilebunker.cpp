// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "pilebunker.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillPileBunker::SkillPileBunker() : WeaponSkillImpl(NC_PILEBUNKER) {
}

void SkillPileBunker::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	if( rnd()%100 < 25 + 15*skill_lv ) {
		status_change_end(target, SC_KYRIE);
		status_change_end(target, SC_ASSUMPTIO);
		status_change_end(target, SC_STEELBODY);
		status_change_end(target, SC_GT_CHANGE);
		status_change_end(target, SC_GT_REVITALIZE);
		status_change_end(target, SC_AUTOGUARD);
		status_change_end(target, SC_REFLECTDAMAGE);
		status_change_end(target, SC_DEFENDER);
		status_change_end(target, SC_PRESTIGE);
		status_change_end(target, SC_BANDING);
		status_change_end(target, SC_MILLENNIUMSHIELD);
	}
}

void SkillPileBunker::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	skillratio += 200 + 100 * skill_lv + status_get_str(src);
	RE_LVL_DMOD(100);
}
