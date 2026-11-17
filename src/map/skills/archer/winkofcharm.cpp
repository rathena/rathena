// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "winkofcharm.hpp"

#include <config/core.hpp>

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillWinkofCharm::SkillWinkofCharm() : SkillImpl(DC_WINKCHARM) {
}

void SkillWinkofCharm::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* dstsd = BL_CAST(BL_PC, target);
	mob_data* dstmd = BL_CAST(BL_MOB, target);

	if( dstsd ) {
#ifdef RENEWAL
		// In Renewal it causes Confusion and Hallucination to 100% base chance
		sc_start(src, target, SC_CONFUSION, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
		sc_start(src, target, SC_HALLUCINATION, 100, skill_lv, skill_get_time2(getSkillId(), skill_lv));
#else
		// In Pre-Renewal it only causes Wink Charm, if Confusion was successfully started
		if (sc_start(src, target, SC_CONFUSION, 10, skill_lv, skill_get_time(getSkillId(), skill_lv)))
			sc_start(src, target, type, 100, skill_lv, skill_get_time2(getSkillId(), skill_lv));
#endif
	} else
	if( dstmd )
	{
		// For monsters it causes Wink Charm with a chance depending on the level difference
		if (sc_start2(src, target, type, (status_get_lv(src) - status_get_lv(target)) + 40, skill_lv, src->id, skill_get_time2(getSkillId(), skill_lv))) {
			// This triggers a 0 damage event and might make the monster switch target to caster
			battle_damage(src, target, 0, 1, skill_lv, 0, ATK_DEF, BF_WEAPON|BF_LONG|BF_NORMAL, true, tick, false);
		}
	}
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
}
