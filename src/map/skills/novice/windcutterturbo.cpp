// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "windcutterturbo.hpp"

#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

SkillWindCutterTurbo::SkillWindCutterTurbo() : SkillImplRecursiveDamageSplash(HN_WIND_CUTTER_TURBO) {
}

void SkillWindCutterTurbo::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 550 * skill_lv + 15 * pc_checkskill(sd, HN_SELFSTUDY_TATICS);
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}
