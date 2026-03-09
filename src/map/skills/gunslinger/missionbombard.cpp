// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "missionbombard.hpp"

#include <config/core.hpp>

#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillMissionBombard::SkillMissionBombard() : WeaponSkillImpl(NW_MISSION_BOMBARD) {
}

void SkillMissionBombard::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 i = skill_get_splash(getSkillId(), skill_lv);
	map_foreachinarea(skill_area_sub, src->m, x - i, y - i, x + i, y + i, BL_CHAR | BL_SKILL, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SKILL_ALTDMG_FLAG | 1, skill_castend_damage_id);
	skill_unitsetting(src, getSkillId(), skill_lv, x, y, flag);

	for (i = 1; i <= (skill_get_time(getSkillId(), skill_lv) / skill_get_unit_interval(getSkillId())); i++) {
		skill_addtimerskill(src, tick + (t_tick)i * skill_get_unit_interval(getSkillId()), 0, x, y, getSkillId(), skill_lv, 0, flag);
	}
}

void SkillMissionBombard::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);

	if (wd->miscflag & SKILL_ALTDMG_FLAG) {
		skillratio += -100 + 5000 + 1800 * skill_lv;
		skillratio += pc_checkskill(sd, NW_GRENADE_MASTERY) * 100;
	}
	else {
		skillratio += -100 + 800 + 200 * skill_lv;
		skillratio += pc_checkskill(sd, NW_GRENADE_MASTERY) * 30;
	}
	skillratio += 5 * sstatus->con;
	RE_LVL_DMOD(100);
}
