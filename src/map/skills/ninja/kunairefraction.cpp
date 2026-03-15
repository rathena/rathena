// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "kunairefraction.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillKunaiRefraction::SkillKunaiRefraction() : SkillImpl(SS_KUNAIKUSSETSU) {
}

void SkillKunaiRefraction::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 250 + 420 * skill_lv;
	skillratio += pc_checkskill( sd, SS_KUNAIKAITEN ) * 10 * skill_lv;
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillKunaiRefraction::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_foreachinallrange(skill_detonator, src, skill_get_splash(getSkillId(), skill_lv), BL_SKILL, src, skill_lv);
	clif_skill_nodamage(src, *src, getSkillId(), skill_lv);
}
