// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "rokicapriccio.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillRokiCapriccio::SkillRokiCapriccio() : SkillImpl(TR_ROKI_CAPRICCIO) {
}

void SkillRokiCapriccio::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (flag & 1) { // Need official success chances.
		uint16 success_chance = 5 * skill_lv;

		if (flag & 2)
			success_chance *= 2;

		// Is it a chance to inflect so and so, or seprate chances for inflicting each status? [Rytech]
		sc_start(src, target, SC_CONFUSION, 4 * skill_lv, skill_lv, skill_get_time(getSkillId(), skill_lv));
		sc_start(src, target, SC_HANDICAPSTATE_MISFORTUNE, success_chance, skill_lv, skill_get_time2(getSkillId(), skill_lv));
	}
	else if (sd) {
		clif_skill_nodamage(target, *target, getSkillId(), skill_lv);

		sd->skill_id_song = getSkillId();
		sd->skill_lv_song = skill_lv;

		if (skill_check_pc_partner(sd, getSkillId(), &skill_lv, AREA_SIZE, 0) > 0)
			flag |= 2;

		map_foreachinallrange(skill_area_sub, src, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | 1, skill_castend_nodamage_id);
	}
}
