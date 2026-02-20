// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "flicker.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"

SkillFlicker::SkillFlicker() : SkillImpl(RL_FLICKER) {
}

void SkillFlicker::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd) {
		sd->flicker = true;
		skill_area_temp[1] = 0;
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		if (pc_checkskill(sd, RL_B_TRAP)) {
			map_foreachinallrange(skill_bind_trap, src, AREA_SIZE, BL_SKILL, src);
		}
		if (int32 mine_lv = pc_checkskill(sd, RL_H_MINE)) {
			map_foreachinallrange(skill_area_sub, src, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, RL_H_MINE, mine_lv, tick, flag | BCT_ENEMY | SD_SPLASH, skill_castend_damage_id);
		}
		sd->flicker = false;
	}
}
