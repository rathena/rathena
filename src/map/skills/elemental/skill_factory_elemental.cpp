// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_elemental.hpp"

#include <config/core.hpp>

#include "../skill_impl.hpp"

// Include .cpp files into the TU to optimize compile time
// For reference see unity builds or amalgamated builds
#include "ageofice.cpp"
#include "avalanche.cpp"
#include "coldforce.cpp"
#include "crystalarmor.cpp"
#include "deadlypoison.cpp"
#include "deeppoisoning.cpp"
#include "earthcare.cpp"
#include "eyesofstorm.cpp"
#include "flamearmor.cpp"
#include "flamerock.cpp"
#include "flametechnic.cpp"
#include "gracebreeze.cpp"
#include "poisonshield.cpp"
#include "stormwind.cpp"
#include "strongprotection.cpp"

std::unique_ptr<const SkillImpl> SkillFactoryElemental::create(const e_skill skill_id) const {
	switch (skill_id) {
		case EM_EL_AGE_OF_ICE:
			return std::make_unique<SkillAgeOfIce>();
		case EM_EL_AVALANCHE:
			return std::make_unique<SkillAvalanche>();
		case EM_EL_COLD_FORCE:
			return std::make_unique<SkillColdForce>();
		case EM_EL_CRYSTAL_ARMOR:
			return std::make_unique<SkillCrystalArmor>();
		case EM_EL_DEADLY_POISON:
			return std::make_unique<SkillDeadlyPoison>();
		case EM_EL_DEEP_POISONING:
			return std::make_unique<SkillDeepPoisoning>();
		case EM_EL_EARTH_CARE:
			return std::make_unique<SkillEarthCare>();
		case EM_EL_EYES_OF_STORM:
			return std::make_unique<SkillEyesOfStorm>();
		case EM_EL_FLAMEARMOR:
			return std::make_unique<SkillFlameArmor>();
		case EM_EL_FLAMEROCK:
			return std::make_unique<SkillFlameRock>();
		case EM_EL_FLAMETECHNIC:
			return std::make_unique<SkillFlameTechnic>();
		case EM_EL_GRACE_BREEZE:
			return std::make_unique<SkillGraceBreeze>();
		case EM_EL_POISON_SHIELD:
			return std::make_unique<SkillPoisonShield>();
		case EM_EL_STORM_WIND:
			return std::make_unique<SkillStormWind>();
		case EM_EL_STRONG_PROTECTION:
			return std::make_unique<SkillStrongProtection>();

		default:
			return nullptr;
	}
}
