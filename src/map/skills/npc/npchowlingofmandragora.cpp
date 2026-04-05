// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "npchowlingofmandragora.hpp"

#include <common/random.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillNpcHowlingOfMandragora::SkillNpcHowlingOfMandragora() : SkillImpl(NPC_MANDRAGORA) {
}

void SkillNpcHowlingOfMandragora::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* tstatus = status_get_status_data(*target);
	status_change *tsc = status_get_sc(target);
	sc_type type = skill_get_sc(getSkillId());

	if( flag&1 ) {
		int32 rate;
		rate = (20 * skill_lv) - (tstatus->vit + tstatus->luk) / 5;

		if (rate < 10)
			rate = 10;
		if (target->type == BL_MOB || (tsc && tsc->getSCE(type)))
			return; // Don't activate if target is a monster or zap SP if target already has Mandragora active.
		if (rnd()%100 < rate) {
			sc_start(src,target,type,100,skill_lv,skill_get_time(getSkillId(),skill_lv));
			status_zap(target,0,status_get_max_sp(target) * (25 + 5 * skill_lv) / 100);
		}
	} else {
		map_foreachinallrange(skill_area_sub,target,skill_get_splash(getSkillId(),skill_lv),BL_CHAR,src,getSkillId(),skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_nodamage_id);
		clif_skill_nodamage(src,*src,getSkillId(),skill_lv);
	}
}
