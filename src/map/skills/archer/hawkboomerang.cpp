// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hawkboomerang.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillHawkBoomerang::SkillHawkBoomerang() : WeaponSkillImpl(WH_HAWKBOOMERANG) {
}

void SkillHawkBoomerang::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);
	const map_session_data* sd = BL_CAST( BL_PC, src );

	skillratio += -100 + 600 * skill_lv + 10 * sstatus->con;
	if (sd)
		skillratio += skillratio * pc_checkskill(sd, WH_NATUREFRIENDLY) / 10;
	if (tstatus->race == RC_BRUTE || tstatus->race == RC_FISH)
		skillratio += skillratio * 50 / 100;
	RE_LVL_DMOD(100);
}

void SkillHawkBoomerang::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}
