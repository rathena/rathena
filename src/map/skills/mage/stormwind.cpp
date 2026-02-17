// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "stormwind.hpp"

#include "map/clif.hpp"
#include "map/elemental.hpp"
#include "map/status.hpp"

SkillStormWind::SkillStormWind() : SkillImplRecursiveDamageSplash(EM_EL_STORM_WIND) {
}

void SkillStormWind::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	const s_elemental_data* ed = BL_CAST(BL_ELEM, src);

	base_skillratio += -100 + 2600;
	if (ed)
		base_skillratio += base_skillratio * status_get_lv(ed->master) / 100;
}

void SkillStormWind::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
