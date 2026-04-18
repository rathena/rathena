// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "silvervineroottwist.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillSilvervineRootTwist::SkillSilvervineRootTwist() : SkillImpl(SU_SV_ROOTTWIST) {
}

void SkillSilvervineRootTwist::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST(BL_PC, src);

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	if (sd && status_get_class_(target) == CLASS_BOSS) {
		clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL_TOTARGET );
		return;
	}
	if (tsc != nullptr && tsc->hasSCE(type)) // Refresh the status only if it's already active.
		sc_start(src, target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
	else {
		sc_start(src, target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
		if (sd && pc_checkskill(sd, SU_SPIRITOFLAND))
			sc_start(src, src, SC_DORAM_MATK, 100, sd->status.base_level, skill_get_time(SU_SPIRITOFLAND, 1));
		skill_addtimerskill(src, tick + 1000, target->id, 0, 0, SU_SV_ROOTTWIST_ATK, skill_lv, skill_get_type(SU_SV_ROOTTWIST_ATK), flag);
	}
}
