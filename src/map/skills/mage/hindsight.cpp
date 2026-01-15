// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hindsight.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillHindsight::SkillHindsight() : SkillImpl(SA_AUTOSPELL) {
}

void SkillHindsight::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);
	map_session_data* sd = BL_CAST( BL_PC, src );

	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	if (sd) {
		sd->state.workinprogress = WIP_DISABLE_ALL;
		clif_autospell( *sd, skill_lv );
	} else {
		int32 maxlv=1,spellid=0;
		static const int32 spellarray[3] = { MG_COLDBOLT,MG_FIREBOLT,MG_LIGHTNINGBOLT };

		if(skill_lv >= 10) {
			spellid = MG_FROSTDIVER;
//			if (tsc && tsc->getSCE(SC_SPIRIT) && tsc->getSCE(SC_SPIRIT)->val2 == SA_SAGE)
//				maxlv = 10;
//			else
				maxlv = skill_lv - 9;
		}
		else if(skill_lv >=8) {
			spellid = MG_FIREBALL;
			maxlv = skill_lv - 7;
		}
		else if(skill_lv >=5) {
			spellid = MG_SOULSTRIKE;
			maxlv = skill_lv - 4;
		}
		else if(skill_lv >=2) {
			int32 i_rnd = rnd()%3;
			spellid = spellarray[i_rnd];
			maxlv = skill_lv - 1;
		}
		else if(skill_lv > 0) {
			spellid = MG_NAPALMBEAT;
			maxlv = 3;
		}

		if(spellid > 0)
			sc_start4(src,src,SC_AUTOSPELL,100,skill_lv,spellid,maxlv,0,
				skill_get_time(SA_AUTOSPELL,skill_lv));
	}
}
