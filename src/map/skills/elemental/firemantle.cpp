// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "firemantle.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillFireMantle::SkillFireMantle() : SkillImpl(EL_FIRE_MANTLE) {
}

void SkillFireMantle::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 900;
}

void SkillFireMantle::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_damage( *src, *target, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE );
	skill_unitsetting(src,getSkillId(),skill_lv,target->x,target->y,0);
}
