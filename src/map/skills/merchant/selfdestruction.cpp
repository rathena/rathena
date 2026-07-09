// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "selfdestruction.hpp"

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillSelfDestruction::SkillSelfDestruction() : SkillImplRecursiveDamageSplash(NC_SELFDESTRUCTION) {
}

void SkillSelfDestruction::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd == nullptr) {
		return;
	}

	if (pc_ismadogear(sd)) {
		pc_setmadogear(sd, false);
	}

	skill_area_temp[1] = 0;
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR | BL_SKILL, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
	status_set_sp(src, 0, 0);
	skill_clear_unitgroup(src);
}
