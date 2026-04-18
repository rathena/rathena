// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "greatecho.hpp"

#include <config/core.hpp>

#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillGreatEcho::SkillGreatEcho() : WeaponSkillImpl(WM_GREAT_ECHO) {
}

void SkillGreatEcho::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 250 + 500 * skill_lv;
	if (sd) {
		skillratio += pc_checkskill(sd, WM_LESSON) * 50; // !TODO: Confirm bonus
		if (skill_check_pc_partner(const_cast<map_session_data*>(sd), getSkillId(), &skill_lv, AREA_SIZE, 0) > 0)
			skillratio *= 2;
	}
	RE_LVL_DMOD(100);
}

void SkillGreatEcho::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 i = skill_get_splash(getSkillId(),skill_lv);
	map_foreachinarea(skill_area_sub,src->m,x-i,y-i,x+i,y+i,BL_CHAR,src,getSkillId(),skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
}
