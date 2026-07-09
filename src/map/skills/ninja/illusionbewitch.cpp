// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "illusionbewitch.hpp"

#include <common/random.hpp>

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillIllusionBewitch::SkillIllusionBewitch() : SkillImpl(KO_GENWAKU) {
}

void SkillIllusionBewitch::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	mob_data* dstmd = BL_CAST(BL_MOB, target);
	status_data* tstatus = status_get_status_data(*target);
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);

	if ((dstsd || dstmd) && !status_has_mode(tstatus,MD_IGNOREMELEE|MD_IGNOREMAGIC|MD_IGNORERANGED|MD_IGNOREMISC) && battle_check_target(src,target,BCT_ENEMY) > 0) {
		int32 x = src->x, y = src->y;

		if (sd && rnd()%100 > ((45+5*skill_lv) - status_get_int(target)/10)) { //[(Base chance of success) - (Intelligence Objectives / 10)]%.
			clif_skill_fail( *sd, getSkillId() );
			return;
		}

		// Confusion is still inflicted (but rate isn't reduced), no matter map type.
		status_change_start(src, src, SC_CONFUSION, 2500, skill_lv, 0, 0, 0, skill_get_time(getSkillId(), skill_lv), SCSTART_NORATEDEF);
		status_change_start(src, target, SC_CONFUSION, 7500, skill_lv, 0, 0, 0, skill_get_time(getSkillId(), skill_lv), SCSTART_NORATEDEF);

		if (skill_check_unit_movepos(5,src,target->x,target->y,0,0)) {
			clif_skill_nodamage(src, *src, getSkillId(), skill_lv);
			clif_blown(src);
			if (!unit_blown_immune(target, 0x1)) {
				unit_movepos(target,x,y,0,0);
				if (target->type == BL_PC && pc_issit((TBL_PC*)target))
					clif_sitting(*target); //Avoid sitting sync problem
				clif_blown(target);
				map_foreachinallrange(unit_changetarget, src, AREA_SIZE, BL_CHAR, src, target);
			}
		}
	}
}
