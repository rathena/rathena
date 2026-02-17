// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "abysssquare.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillAbyssSquare::SkillAbyssSquare() : SkillImpl(ABC_ABYSS_SQUARE) {
}

void SkillAbyssSquare::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	flag|=1;//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).

	skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);
}

void SkillAbyssSquare::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 750 * skill_lv;
	skillratio += 40 * pc_checkskill(sd, ABC_MAGIC_SWORD_M) * skill_lv;
	skillratio += 5 * sstatus->spl;
	RE_LVL_DMOD(100);
}
