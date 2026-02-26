// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "spiritdestruction.hpp"

#include <common/random.hpp>

#include "map/status.hpp"

SkillSpiritDestruction::SkillSpiritDestruction() : WeaponSkillImpl(NPC_MENTALBREAKER) {
}

void SkillSpiritDestruction::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	status_data* sstatus = status_get_status_data(*src);

	//Based on observations by Tharis, Mental Breaker should do SP damage
	//equal to Matk*skLevel.
	int32 rate = sstatus->matk_min;
	if (rate < sstatus->matk_max)
		rate += rnd()%(sstatus->matk_max - sstatus->matk_min);
	rate*=skill_lv;
	status_zap(target, 0, rate);
}
