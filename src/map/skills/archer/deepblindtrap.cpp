// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "deepblindtrap.hpp"

#include <config/const.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillDeepBlindTrap::SkillDeepBlindTrap() : SkillImpl(WH_DEEPBLINDTRAP) {
}

void SkillDeepBlindTrap::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST( BL_PC, src );

	skillratio += -100 + 850 * skill_lv + 5 * sstatus->con;
	RE_LVL_DMOD(100);
	skillratio += skillratio * (20 * (sd ? pc_checkskill(sd, WH_ADVANCED_TRAP) : 5)) / 100;
}

void SkillDeepBlindTrap::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	flag|=1;//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}
