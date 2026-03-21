// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mug.hpp"

#include <common/random.hpp>

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"

SkillMug::SkillMug() : SkillImpl(RG_STEALCOIN) {
}

void SkillMug::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );
	mob_data *dstmd = BL_CAST(BL_MOB, target);

	if (sd == nullptr || dstmd == nullptr)
		return;

	int32 target_lv = status_get_lv(target);
	int32 rate = 10 * pc_checkskill(sd, RG_STEALCOIN);
	rate += sd->battle_status.dex / 2;
	rate += sd->battle_status.luk / 2;
	rate += 2 * (sd->status.base_level - target_lv);

	if (!rnd_chance_official(rate, 1000))
	{
		clif_skill_fail(*sd, getSkillId());
		return;
	}

	dstmd->state.steal_coin_flag = 1;

	// Zeny Steal Amount
	int32 amount = rnd_value(8 * target_lv, 10 * target_lv);
	amount += (skill_lv * target_lv) / 10;

	pc_getzeny(sd, amount, LOG_TYPE_STEAL);

	// This triggers a 0 damage event and might make the monster switch target to caster
	battle_damage(src, target, 0, 1, skill_lv, 0, ATK_DEF, BF_WEAPON|BF_LONG|BF_NORMAL, true, tick, false);

	// Client uses skill_lv to show how many Zeny were stolen
	clif_skill_nodamage(src, *target, getSkillId(), amount);		
}
