// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "wildwalk.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillWildWalk::SkillWildWalk() : WeaponSkillImpl(WH_WILD_WALK) {
}

void SkillWildWalk::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST( BL_PC, src );

	skillratio += -100 + 1800 + 2800 * skill_lv;
	// !TODO: unknown con and WH_NATUREFRIENDLY/HT_STEELCROW skills ratio
	skillratio += 5 * sstatus->con;
	skillratio += skillratio * pc_checkskill(sd, WH_NATUREFRIENDLY) / 10;
	skillratio += skillratio * pc_checkskill(sd, HT_STEELCROW) / 10;
	RE_LVL_DMOD(100);
}

void SkillWildWalk::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
	sc_start(src, src, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(),skill_lv));
}
