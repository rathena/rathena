// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "redemptio.hpp"

#include <config/core.hpp>

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/party.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillRedemptio::SkillRedemptio() : SkillImpl(PR_REDEMPTIO) {
}

void SkillRedemptio::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);
	status_change* tsc = status_get_sc(target);

	if (sd && !(flag&1)) {
		if (sd->status.party_id == 0) {
			clif_skill_fail( *sd, getSkillId() );
			return;
		}
		skill_area_temp[0] = 0;
		party_foreachsamemap(skill_area_sub,
			sd,skill_get_splash(getSkillId(), skill_lv),
			src,getSkillId(),skill_lv,tick, flag|BCT_PARTY|1,
			skill_castend_nodamage_id);
		if (skill_area_temp[0] == 0) {
			clif_skill_fail( *sd, getSkillId() );
			return;
		}
#ifndef RENEWAL
		skill_area_temp[0] = battle_config.exp_cost_redemptio_limit - skill_area_temp[0]; // The actual penalty...
		if (skill_area_temp[0] > 0 && !map_getmapflag(src->m, MF_NOEXPPENALTY) && battle_config.exp_cost_redemptio) { //Apply penalty
			//If total penalty is 1% => reduced 0.2% penalty per each revived player
			pc_lostexp(sd, u64min(sd->status.base_exp, (pc_nextbaseexp(sd) * skill_area_temp[0] * battle_config.exp_cost_redemptio / battle_config.exp_cost_redemptio_limit) / 100), 0);
		}
		status_set_sp(src, 0, 0);
#endif
		status_set_hp(src, 1, 0);
		return;
	} else if (!(status_isdead(*target) && flag&1)) { 
		//Invalid target, skip resurrection.
		return;
	}
	//Revive
	skill_area_temp[0]++; //Count it in, then fall-through to the Resurrection code.
	skill_lv = 3; //Resurrection level 3 is used

	if(sd && (map_flag_gvg2(target->m) || map_getmapflag(target->m, MF_BATTLEGROUND)))
	{	//No reviving in WoE grounds!
		clif_skill_fail( *sd, getSkillId() );
		return;
	}
	if (!status_isdead(*target))
		return;

	int32 per = 0, sper = 0;
	if (tsc && tsc->getSCE(SC_HELLPOWER)) {
		clif_skill_nodamage(src, *target, ALL_RESURRECTION, skill_lv);
		return;
	}

	if (map_getmapflag(target->m, MF_PVP) && dstsd && dstsd->pvp_point < 0)
		return;

	switch(skill_lv){
	case 1: per=10; break;
	case 2: per=30; break;
	case 3: per=50; break;
	case 4: per=80; break;
	}
	if(dstsd && dstsd->special_state.restart_full_recover)
		per = sper = 100;
	if (status_revive(target, per, sper))
	{
		clif_skill_nodamage(src,*target,ALL_RESURRECTION,skill_lv); //Both Redemptio and Res show this skill-animation.
		if(sd && dstsd && battle_config.resurrection_exp > 0)
		{
			t_exp exp = 0,jexp = 0;
			int32 lv = dstsd->status.base_level - sd->status.base_level, jlv = dstsd->status.job_level - sd->status.job_level;
			if(lv > 0 && pc_nextbaseexp(dstsd)) {
				exp = (t_exp)(dstsd->status.base_exp * lv * battle_config.resurrection_exp / 1000000.);
				if (exp < 1) exp = 1;
			}
			if(jlv > 0 && pc_nextjobexp(dstsd)) {
				jexp = (t_exp)(dstsd->status.job_exp * lv * battle_config.resurrection_exp / 1000000.);
				if (jexp < 1) jexp = 1;
			}
			if(exp > 0 || jexp > 0)
				pc_gainexp (sd, target, exp, jexp, 0);
		}
	}
}
