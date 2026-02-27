// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "resurrection.hpp"

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillResurrection::SkillResurrection() : SkillImpl(ALL_RESURRECTION) {
}

void SkillResurrection::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* tstatus = status_get_status_data(*target);

	if (!battle_check_undead(tstatus->race, tstatus->def_ele))
		return;
	skill_attack(BF_MAGIC,src,src,target,getSkillId(),skill_lv,tick,flag);
}

void SkillResurrection::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);

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
