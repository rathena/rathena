// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "blessingofmysticalcreatures.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillBlessingofMysticalCreatures::SkillBlessingofMysticalCreatures() : SkillImpl(SH_BLESSING_OF_MYSTICAL_CREATURES) {
}

void SkillBlessingofMysticalCreatures::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_heal(target, 0, 0, 200-status_get_ap(target), 0);
	sc_start(src, target, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
	clif_skill_nodamage(src, *src, getSkillId(), skill_lv);
}
