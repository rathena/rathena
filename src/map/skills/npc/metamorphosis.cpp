// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "metamorphosis.hpp"

#include "map/mob.hpp"

SkillMetamorphosis::SkillMetamorphosis() : SkillImpl(NPC_METAMORPHOSIS) {
}

void SkillMetamorphosis::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	mob_data* md = BL_CAST(BL_MOB, src);

	if(md && md->skill_idx >= 0) {
		int32 class_ = mob_random_class (md->db->skill[md->skill_idx]->val,0);
		if (skill_lv > 1) //Multiply the rest of mobs. [Skotlex]
			mob_summonslave(md,md->db->skill[md->skill_idx]->val,skill_lv-1,getSkillId());
		if (class_) mob_class_change(md, class_);
	}
}
