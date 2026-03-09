// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "magicstrings.hpp"

#include <config/core.hpp>

SkillMagicStrings::SkillMagicStrings() : SkillImpl(BA_POEMBRAGI) {
}

void SkillMagicStrings::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
#ifdef RENEWAL
	skill_castend_song(src, getSkillId(), skill_lv, tick);
#endif
}

void SkillMagicStrings::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
#ifndef RENEWAL
	flag|=1;//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
#endif
}
