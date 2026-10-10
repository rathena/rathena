// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fuguedesfleches.hpp"

#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

SkillFugueDesFleches::SkillFugueDesFleches() : SkillImplRecursiveDamageSplash(TR_FUGUE_DES_FLECHES) {
}

void SkillFugueDesFleches::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change* sc = status_get_sc(src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	if (sc != nullptr && sc->hasSCE(SC_MYSTIC_SYMPHONY))
		skillratio += -100 + 350 * skill_lv + 200;
	else
		skillratio += -100 + 250 * skill_lv + 300;
	if (pc_checkskill(sd, TR_STAGE_MANNER) > 0)
		skillratio += 5 * sstatus->con;
	RE_LVL_DMOD(100);
}
