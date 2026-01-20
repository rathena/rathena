// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "questioning.hpp"

#include "map/clif.hpp"

SkillQuestioning::SkillQuestioning() : SkillImpl(SA_QUESTION) {
}

void SkillQuestioning::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_emotion( *src, ET_QUESTION );
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
}
