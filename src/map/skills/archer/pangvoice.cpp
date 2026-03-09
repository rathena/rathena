// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "pangvoice.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillPangVoice::SkillPangVoice() : SkillImpl(BA_PANGVOICE) {
}

void SkillPangVoice::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
#ifdef RENEWAL
	// In Renewal it causes Confusion and Bleeding to 100% base chance
	sc_start(src, target, SC_CONFUSION, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
	sc_start(src, target, SC_BLEEDING, 100, skill_lv, skill_get_time2(getSkillId(), skill_lv));
#else
	// In Pre-renewal it causes Confusion to 70% base chance
	sc_start(src, target, SC_CONFUSION, 70, skill_lv, skill_get_time(getSkillId(), skill_lv));
#endif
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
}
