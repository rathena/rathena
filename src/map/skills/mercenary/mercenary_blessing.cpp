// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mercenary_blessing.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillMercenaryBlessing::SkillMercenaryBlessing() : SkillImpl(MER_BLESSING) {
}

void SkillMercenaryBlessing::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* tstatus = status_get_status_data(*target);
	status_change *tsc = status_get_sc(target);
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* dstsd = BL_CAST(BL_PC, target);

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	if (dstsd != nullptr && tsc && tsc->getSCE(SC_CHANGEUNDEAD)) {
		if (tstatus->hp > 1)
			skill_attack(BF_MISC,src,src,target,getSkillId(),skill_lv,tick,flag);
		return;
	}
	sc_start(src, target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
}
