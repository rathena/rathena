// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "kunairotation.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillKunaiRotation::SkillKunaiRotation() : SkillImpl(SS_KUNAIKAITEN) {
}

void SkillKunaiRotation::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 950 + 1050 * skill_lv;
	skillratio += pc_checkskill( sd, SS_KUNAIWAIKYOKU ) * 100 * skill_lv;
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillKunaiRotation::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_start(src, src, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time2(getSkillId(), skill_lv));
	skill_unitsetting(src, getSkillId(), skill_lv, x, y, UNIT_NOCONSUME_AMMO);
	skill_unitsetting(src, SS_KUNAIWAIKYOKU, skill_lv, x, y, UNIT_NOCONSUME_AMMO);
}
