// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "ainrhapsody.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillAinRhapsody::SkillAinRhapsody() : SkillImpl(TR_AIN_RHAPSODY) {
}

void SkillAinRhapsody::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (flag & 1)
		sc_start4(src, target, skill_get_sc(getSkillId()), 100, skill_lv, 0, flag, 0, skill_get_time(getSkillId(), skill_lv));
	else if (sd) {
		clif_skill_nodamage(target, *target, getSkillId(), skill_lv);

		sd->skill_id_song = getSkillId();
		sd->skill_lv_song = skill_lv;

		if (skill_check_pc_partner(sd, getSkillId(), &skill_lv, AREA_SIZE, 0) > 0)
			flag |= 2;

		map_foreachinallrange(skill_area_sub, src, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | 1, skill_castend_nodamage_id);
	}
}
