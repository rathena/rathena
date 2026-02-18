// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "tarotcardoffate.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillTarotCardOfFate::SkillTarotCardOfFate() : SkillImpl(CG_TAROTCARD) {
}

void SkillTarotCardOfFate::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data *sd = BL_CAST(BL_PC, src);
	mob_data *dstmd = BL_CAST(BL_MOB, target);
	status_change *tsc = status_get_sc(target);

	if (tsc && tsc->getSCE(SC_TAROTCARD)) {
		// Target currently has the SUN tarot card effect and is immune to any other effect.
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}

	if (rnd() % 100 > skill_lv * 8 ||
#ifndef RENEWAL
		(tsc && tsc->getSCE(SC_BASILICA)) ||
#endif
		(dstmd && ((dstmd->guardian_data && dstmd->mob_id == MOBID_EMPERIUM) || status_get_class_(target) == CLASS_BATTLEFIELD))) {
		if (sd != nullptr)
			clif_skill_fail(*sd, getSkillId());
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}

	status_zap(src, 0, skill_get_sp(getSkillId(), skill_lv)); // Consume SP only on success.
	int32 card = skill_tarotcard(src, target, getSkillId(), skill_lv, tick); // Actual effect is executed here.
	clif_specialeffect((card == 6) ? src : target, EF_TAROTCARD1 + card - 1, AREA);
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
}
