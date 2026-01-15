// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "dispell.hpp"

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillDispell::SkillDispell() : SkillImpl(SA_DISPELL) {
}

void SkillDispell::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	mob_data* dstmd = BL_CAST(BL_MOB, target);
	status_change *tsc = status_get_sc(target);
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST( BL_PC, src );
	map_session_data* dstsd = BL_CAST( BL_PC, target );
	int32 i = 0;

	if (flag&1 || (i = skill_get_splash(getSkillId(), skill_lv)) < 1) {
		if (sd && dstsd && !map_flag_vs(sd->m) && (!sd->duel_group || sd->duel_group != dstsd->duel_group) && (!sd->status.party_id || sd->status.party_id != dstsd->status.party_id))
			return; // Outside PvP it should only affect party members and no skill fail message
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
		if((dstsd && (dstsd->class_&MAPID_SECONDMASK) == MAPID_SOUL_LINKER)
			|| (tsc && tsc->getSCE(SC_SPIRIT) && tsc->getSCE(SC_SPIRIT)->val2 == SL_ROGUE) //Rogue's spirit defends againt32 dispel.
			|| rnd()%100 >= 50+10*skill_lv)
		{
			if (sd)
				clif_skill_fail( *sd, getSkillId() );
			return;
		}
		if(status_isimmune(target))
			return;

		//Remove bonus_script by Dispell
		if (dstsd)
			pc_bonus_script_clear(dstsd,BSF_REM_ON_DISPELL);
		// Monsters will unlock their target instead
		else if (dstmd)
			mob_unlocktarget(dstmd, tick);

		if(tsc == nullptr || tsc->empty())
			return;

		//Statuses that can't be Dispelled
		for (const auto &it : status_db) {
			sc_type status = static_cast<sc_type>(it.first);

			if (!tsc->getSCE(status))
				continue;

			if (it.second->flag[SCF_NODISPELL])
				continue;
			switch (status) {
				// bugreport:4888 these songs may only be dispelled if you're not in their song area anymore
				case SC_WHISTLE:		case SC_ASSNCROS:		case SC_POEMBRAGI:
				case SC_APPLEIDUN:		case SC_HUMMING:		case SC_DONTFORGETME:
				case SC_FORTUNE:		case SC_SERVICE4U:
					if (!battle_config.dispel_song || tsc->getSCE(status)->val4 == 0)
						continue; //If in song area don't end it, even if config enatargeted
					break;
				case SC_ASSUMPTIO:
					if( target->type == BL_MOB )
						continue;
					break;
			}
			if (status == SC_BERSERK || status == SC_SATURDAYNIGHTFEVER)
				tsc->getSCE(status)->val2 = 0; //Mark a dispelled berserk to avoid setting hp to 100 by setting hp penalty to 0.
			status_change_end(target, status);
		}
		return;
	}

	//Affect all targets on splash area.
	map_foreachinallrange(skill_area_sub, target, i, BL_CHAR,
		src, getSkillId(), skill_lv, tick, flag|1,
		skill_castend_damage_id);
}
