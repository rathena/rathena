// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_benedictionofchaos.hpp"

#include <array>

SkillBenedictionOfChaos::SkillBenedictionOfChaos() : SkillImpl(HVAN_CHAOTIC) {
}

void SkillBenedictionOfChaos::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	// Chance per skill level
	static const std::array<uint8, 5> chance_homunculus = {
		20,
		50,
		25,
		50,
		34
	};
	static const std::array<uint8, 5> chance_master = {
		static_cast<uint8>(chance_homunculus[0] + 30),
		static_cast<uint8>(chance_homunculus[1] + 10),
		static_cast<uint8>(chance_homunculus[2] + 50),
		static_cast<uint8>(chance_homunculus[3] + 4),
		static_cast<uint8>(chance_homunculus[4] + 33)
	};

	uint8 chance = rnd_value(1, 100);

	// Homunculus
	if (chance <= chance_homunculus[skill_lv - 1]) {
		target = src;
	// Master
	} else if (chance <= chance_master[skill_lv - 1]) {
		target = battle_get_master(src);
	// Enemy (A random enemy targeting the master)
	} else {
		target = battle_gettargeted(battle_get_master(src));
	}

	// If there's no enemy the chance reverts to the homunculus
	if (target == nullptr) {
		target = src;
	}

	int32 heal = skill_calc_heal(src, target, getSkillId(), rnd_value<uint16>(1, skill_lv), true);

	// Official servers send the Heal skill packet with the healed amount, and then the skill packet with 1 as healed amount
	clif_skill_nodamage(src, *target, AL_HEAL, heal);
	clif_skill_nodamage(src, *target, getSkillId(), 1);
	status_heal(target, heal, 0, 0);
}
