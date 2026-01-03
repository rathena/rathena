// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fireball.hpp"

#include "map/pc.hpp"

SkillFireBall::SkillFireBall() : SkillImplRecursiveDamageSplash(MG_FIREBALL) {
}

void SkillFireBall::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	base_skillratio += 40 + 20 * skill_lv;
#else
	base_skillratio += -30 + 10 * skill_lv;
#endif
	if (wd->miscflag == 2) //Enemies at the edge of the area will take 75% of the damage
		base_skillratio = base_skillratio * 3 / 4;
}

void SkillFireBall::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
}

int64 SkillFireBall::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	// For players, the distance between original target and splash target determines the damage
	if( map_session_data* sd = BL_CAST( BL_PC, src ); sd != nullptr ){
		if (block_list* orig_bl = map_id2bl(skill_area_temp[1]); orig_bl != nullptr)
			flag |= distance_bl(orig_bl, target);
	}

	// Call default implementation
	return SkillImplRecursiveDamageSplash::splashDamage(src, target, skill_lv, tick, flag);
}
