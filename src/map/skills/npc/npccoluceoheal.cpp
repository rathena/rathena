// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "npccoluceoheal.hpp"

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillNpcColuceoHeal::SkillNpcColuceoHeal() : SkillImpl(NPC_CHEAL) {
}

void SkillNpcColuceoHeal::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if( flag&1 ) {
		status_data* tstatus = status_get_status_data(*target);
		status_change *tsc = status_get_sc(target);

		if( tstatus && !battle_check_undead(tstatus->race, tstatus->def_ele) && tsc != nullptr && !tsc->hasSCE(SC_BERSERK) ) {
			int32 i = skill_calc_heal(src, target, AL_HEAL, 10, true);
			if (status_isimmune(target))
				i = 0;
			clif_skill_nodamage(src, *target, getSkillId(), i);
			if( tsc && tsc->getSCE(SC_AKAITSUKI) && i )
				i = ~i + 1;
			status_heal(target, i, 0, 0);
		}
	}
	else {
		map_foreachinallrange(skill_area_sub, src, skill_get_splash(getSkillId(), skill_lv), BL_MOB,
			src, getSkillId(), skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}
}
