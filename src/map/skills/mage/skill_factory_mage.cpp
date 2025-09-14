// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_mage.hpp"

#include "sight.hpp"
#include "soulstrike.hpp"
#include "stonecurse.hpp"
#include "lightningbolt.hpp"
#include "napalmbeat.hpp"
#include "firebolt.hpp"
#include "frostdiver.hpp"
#include "energycoat.hpp"
#include "fireball.hpp"
#include "coldbolt.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryMage::create(const e_skill skill_id) const {
	switch (skill_id) {
		case MG_SIGHT:
			return std::make_unique<SkillSight>();
		case MG_SOULSTRIKE:
			return std::make_unique<SkillSoulStrike>();
		case MG_STONECURSE:
			return std::make_unique<SkillStoneCurse>();
		case MG_LIGHTNINGBOLT:
			return std::make_unique<SkillLightningBolt>();
		case MG_NAPALMBEAT:
			return std::make_unique<SkillNapalmBeat>();
		case MG_FIREBOLT:
			return std::make_unique<SkillFireBolt>();
		case MG_FROSTDIVER:
			return std::make_unique<SkillFrostDiver>();
		case MG_ENERGYCOAT:
			return std::make_unique<SkillEnergyCoat>();
		case MG_FIREBALL:
			return std::make_unique<SkillFireBall>();
		case MG_COLDBOLT:
			return std::make_unique<SkillColdBolt>();
		default:
			return nullptr;
	}
}
