// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "bullseye.hpp"

#include "map/status.hpp"

SkillBullseye::SkillBullseye() : WeaponSkillImpl(GS_BULLSEYE) {
}

void SkillBullseye::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	const status_data *tstatus = status_get_status_data(*target);

	// Only works well against brute/demihumans non bosses.
	if ((tstatus->race == RC_BRUTE || tstatus->race == RC_DEMIHUMAN || tstatus->race == RC_PLAYER_HUMAN || tstatus->race == RC_PLAYER_DORAM) && !status_has_mode(
		    tstatus, MD_STATUSIMMUNE))
		base_skillratio += 400;
}

void SkillBullseye::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	status_data *tstatus = status_get_status_data(*target);

	// 0.1% coma rate.
	if (tstatus->race == RC_BRUTE || tstatus->race == RC_DEMIHUMAN || tstatus->race == RC_PLAYER_HUMAN || tstatus->race == RC_PLAYER_DORAM)
		status_change_start(src, target, SC_COMA, 10, skill_lv, 0, src->id, 0, 0, SCSTART_NONE);
}
