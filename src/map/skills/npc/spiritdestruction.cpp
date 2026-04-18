// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "spiritdestruction.hpp"

#include <common/random.hpp>

#include "map/status.hpp"

SkillSpiritDestruction::SkillSpiritDestruction() : WeaponSkillImpl(NPC_MENTALBREAKER) {
}

void SkillSpiritDestruction::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	//SP Damage 12%/16%/25%/50%/100% of MaxSP
	int32 rate;
	switch (skill_lv) {
		case 1:
			rate = 12;
			break;
		case 2:
			rate = 16;
			break;
		case 3:
			rate = 25;
			break;
		case 4:
			rate = 50;
			break;
		case 5:
			rate = 100;
			break;
		default:
			// For easy customization
			rate = skill_lv;
			break;
	}
	status_percent_damage(src, target, 0, -rate, false);
}

void SkillSpiritDestruction::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	hit_rate += hit_rate * 20 / 100;
}
