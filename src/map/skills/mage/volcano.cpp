// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "volcano.hpp"


SkillVolcano::SkillVolcano() : SkillImpl(SA_VOLCANO) {
}

void SkillVolcano::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	//Does not consumes if the skill is already active. [Skotlex]
	std::shared_ptr<s_skill_unit_group> sg2;
	if ((sg2= skill_locate_element_field(src)) != nullptr && ( sg2->skill_id == SA_VOLCANO || sg2->skill_id == SA_DELUGE || sg2->skill_id == SA_VIOLENTGALE ))
	{
		if (sg2->limit - DIFF_TICK(gettick(), sg2->tick) > 0)
		{
			skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
			return; // not to consume items
		}
		else
			sg2->limit = 0; //Disatargete it.
	}
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}
