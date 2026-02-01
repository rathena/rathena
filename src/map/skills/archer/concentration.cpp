// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "concentration.hpp"

#include "../../pc.hpp"
#include "../../map.hpp"

SkillConcentration::SkillConcentration() : SkillImpl(AC_CONCENTRATION)
{
}

void SkillConcentration::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const
{
	sc_type type = skill_get_sc(getSkillId());

	int32 splash = skill_get_splash(getSkillId(), skill_lv);
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start(src, target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
	skill_reveal_trap_inarea(src, splash, src->x, src->y);
	map_foreachinallrange(status_change_timer_sub, src, splash, BL_CHAR, src, nullptr, type, tick);
}
