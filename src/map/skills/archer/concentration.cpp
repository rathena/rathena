// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "concentration.hpp"

#include "../../pc.hpp"
#include "../../map.hpp"

SkillConcentration::SkillConcentration() : WeaponSkillImpl(AC_CONCENTRATION)
{
}

void SkillConcentration::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 flag) const
{
	e_skill skill_id = getSkillId();
	sc_type type = skill_get_sc(skill_id);

	int32 splash = skill_get_splash(skill_id, skill_lv);
	clif_skill_nodamage(src, *target, skill_id, skill_lv, sc_start(src, target, type, 100, skill_lv, skill_get_time(skill_id, skill_lv)));
	skill_reveal_trap_inarea(src, splash, src->x, src->y);
	map_foreachinallrange(status_change_timer_sub, src, splash, BL_CHAR, src, nullptr, type, tick);
}
