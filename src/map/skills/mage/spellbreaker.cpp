// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "spellbreaker.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillSpellBreaker::SkillSpellBreaker() : SkillImpl(SA_SPELLBREAKER) {
}

void SkillSpellBreaker::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* tstatus = status_get_status_data(*target);
	status_change *tsc = status_get_sc(target);
	map_session_data* sd = BL_CAST( BL_PC, src );
	map_session_data* dstsd = BL_CAST( BL_PC, target );

	int32 sp;
	if (dstsd && tsc && tsc->getSCE(SC_MAGICROD)) {
		// If target enemy player has Magic Rod, then 20% of your SP is transferred to that player
		sp = status_percent_damage(target, src, 0, -20, false);
		status_heal(target, 0, sp, 2);
	}
	else {
		struct unit_data* ud = unit_bl2ud(target);
		if (!ud || ud->skilltimer == INVALID_TIMER)
			return; //Nothing to cancel.
		int32 hp = 0;
		if (status_has_mode(tstatus, MD_STATUSIMMUNE)) { //Only 10% success chance against status immune. [Skotlex]
			if (rnd_chance(90, 100))
			{
				if (sd) clif_skill_fail( *sd, getSkillId() );
				return;
			}
		}
#ifdef RENEWAL
		else // HP damage does not work on bosses in renewal
#endif
			if (skill_lv >= 5 && (!dstsd || map_flag_vs(target->m))) //HP damage only on pvp-maps when against players.
				hp = tstatus->max_hp / 50; //Siphon 2% HP at level 5

		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		unit_skillcastcancel(target, 0);
		sp = skill_get_sp(ud->skill_id, ud->skill_lv);
		status_zap(target, 0, sp);
		// Recover some of the SP used
		status_heal(src, 0, sp * (25 * (skill_lv - 1)) / 100, 2);

		// If damage would be lethal, it does not deal damage
		if (hp && hp < tstatus->hp) {
			clif_damage(*src, *target, tick, 0, 0, hp, 0, DMG_NORMAL, 0, false);
			status_zap(target, hp, 0);
			// Recover 50% of damage dealt
			status_heal(src, hp / 2, 0, 2);
		}
	}
}
