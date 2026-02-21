// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fullheal.hpp"

#include "map/clif.hpp"
#include "map/mob.hpp"

SkillFullHeal::SkillFullHeal() : SkillImpl(NPC_ALLHEAL) {
}

void SkillFullHeal::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	mob_data* dstmd = BL_CAST(BL_MOB, target);

	int32 heal;
	if( status_isimmune(target) )
		return;
	heal = status_percent_heal(target, 100, 0);
	clif_skill_nodamage(nullptr, *target, AL_HEAL, heal);
	if( dstmd )
	{ // Reset Damage Logs
		dstmd->dmglog.clear();
	}
}
