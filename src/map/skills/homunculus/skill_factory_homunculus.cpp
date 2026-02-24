// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_homunculus.hpp"

#include "../skill_impl.hpp"

// Include .cpp files into the TU to optimize compile time
// For reference see unity builds or amalgamated builds
#include "homunculus_needleofparalyze.cpp"
#include "homunculus_summonlegion.cpp"

std::unique_ptr<const SkillImpl> SkillFactoryHomunculus::create(const e_skill skill_id) const {
	switch (skill_id) {
		case MH_NEEDLE_OF_PARALYZE:
			return std::make_unique<SkillNeedleOfParalyze>();
		case MH_SUMMON_LEGION:
			return std::make_unique<SkillSummonLegion>();

		default:
			return nullptr;
	}
}
