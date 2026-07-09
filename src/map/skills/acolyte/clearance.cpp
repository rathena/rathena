// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "clearance.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillClearance::SkillClearance() : SkillImpl(AB_CLEARANCE) {
}

void SkillClearance::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);
	map_session_data* sd = BL_CAST( BL_PC, src );
	map_session_data* dstsd = BL_CAST( BL_PC, target );
	int32 i = 0;

	if( flag&1 || (i = skill_get_splash(getSkillId(), skill_lv)) < 1 ) { // As of the behavior in official server Clearance is just a super version of Dispell skill. [Jobbie]

		if( target->type != BL_MOB && battle_check_target(src,target,BCT_PARTY) <= 0 ) // Only affect mob or party.
			return;

		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);

		if(rnd()%100 >= 60 + 8 * skill_lv) {
			if (sd)
				clif_skill_fail( *sd, getSkillId() );
			return;
		}

		if(status_isimmune(target))
			return;

		//Remove bonus_script by Clearance
		if (dstsd)
			pc_bonus_script_clear(dstsd,BSF_REM_ON_CLEARANCE);

		if(tsc == nullptr || tsc->empty())
			return;

		//Statuses change that can't be removed by Cleareance
		for (const auto &it : status_db) {
			sc_type status = static_cast<sc_type>(it.first);

			if (!tsc->getSCE(status))
				continue;

			if (it.second->flag[SCF_NOCLEARANCE])
				continue;

			switch (status) {
				case SC_WHISTLE:		case SC_ASSNCROS:		case SC_POEMBRAGI:
				case SC_APPLEIDUN:		case SC_HUMMING:		case SC_DONTFORGETME:
				case SC_FORTUNE:		case SC_SERVICE4U:
					if (!battle_config.dispel_song || tsc->getSCE(status)->val4 == 0)
						continue; //If in song area don't end it, even if config enatargeted
					break;
				case SC_ASSUMPTIO:
					if (target->type == BL_MOB)
						continue;
					break;
			}
			if (status == SC_BERSERK || status == SC_SATURDAYNIGHTFEVER)
				tsc->getSCE(status)->val2 = 0; //Mark a dispelled berserk to avoid setting hp to 100 by setting hp penalty to 0.
			status_change_end(target,status);
		}
		return;
	}

	map_foreachinallrange(skill_area_sub, target, i, BL_CHAR, src, getSkillId(), skill_lv, tick, flag|1, skill_castend_damage_id);
}
