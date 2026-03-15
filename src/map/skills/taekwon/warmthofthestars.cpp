// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "warmthofthestars.hpp"

#include <common/random.hpp>

#include "map/status.hpp"

SkillWarmthoftheStars::SkillWarmthoftheStars() : SkillImpl(SG_STAR_WARM) {
}

void SkillWarmthoftheStars::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	// A random 0~3 knockback bonus is added to the base knockback
	dmg.blewcount += rnd_value(0, 3);
}

void SkillWarmthoftheStars::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	std::shared_ptr<s_skill_unit_group> sg;

	skill_clear_unitgroup(src);
	if ((sg = skill_unitsetting(src,getSkillId(),skill_lv,src->x,src->y,0)))
		sc_start4(src,src,type,100,skill_lv,0,0,sg->group_id,skill_get_time(getSkillId(),skill_lv));
	flag|=1;
}
