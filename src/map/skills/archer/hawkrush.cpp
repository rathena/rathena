// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hawkrush.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillHawkRush::SkillHawkRush() : WeaponSkillImpl(WH_HAWKRUSH) {
}

void SkillHawkRush::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST( BL_PC, src );

	skillratio += -100 + 500 * skill_lv + 5 * sstatus->con;
	if (sd)
		skillratio += skillratio * pc_checkskill(sd, WH_NATUREFRIENDLY) / 10;
	RE_LVL_DMOD(100);
}

void SkillHawkRush::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}
