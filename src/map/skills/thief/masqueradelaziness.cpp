// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "masqueradelaziness.hpp"

#include <common/random.hpp>
#include <common/utils.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillMasqueradeLaziness::SkillMasqueradeLaziness() : SkillImpl(SC_LAZINESS) {
}

void SkillMasqueradeLaziness::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST(BL_PC, src);

	if( !(tsc && tsc->getSCE(type)) ) {
		status_data* sstatus = status_get_status_data(*src);
		status_data* tstatus = status_get_status_data(*target);
		map_session_data* dstsd = BL_CAST(BL_PC, target);

		int32 rate;

		if (status_get_class_(target) == CLASS_BOSS)
			return;
		rate = status_get_lv(src) / 10 + rnd_value(sstatus->dex / 12, sstatus->dex / 4) + ( sd ? sd->status.job_level : 50 ) + 10 * skill_lv
				   - (status_get_lv(target) / 10 + rnd_value(tstatus->agi / 6, tstatus->agi / 3) + tstatus->luk / 10 + ( dstsd ? (dstsd->max_weight / 10 - dstsd->weight / 10 ) / 100 : 0));
		rate = cap_value(rate, skill_lv + sstatus->dex / 20, 100);
		clif_skill_nodamage(src,*target,getSkillId(),0,sc_start(src,target,type,rate,skill_lv,skill_get_time(getSkillId(),skill_lv)));
	} else if( sd )
		 clif_skill_fail( *sd, getSkillId() );
}
