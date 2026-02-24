// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_homunculus.hpp"

#include "../skill_impl.hpp"

// Include .cpp files into the TU to optimize compile time
// For reference see unity builds or amalgamated builds
#include "homunculus_needleofparalyze.cpp"
#include "homunculus_lightofregene.cpp"
#include "homunculus_erasercutter.cpp"
#include "homunculus_overedboost.cpp"
#include "homunculus_painkiller.cpp"
#include "homunculus_poisonmist.cpp"
#include "homunculus_silentbreeze.cpp"
#include "homunculus_stylechange.cpp"
#include "homunculus_summonlegion.cpp"

std::unique_ptr<const SkillImpl> SkillFactoryHomunculus::create(const e_skill skill_id) const {
	switch (skill_id) {
		case MH_ERASER_CUTTER:
			return std::make_unique<SkillEraserCutter>();
		case MH_LIGHT_OF_REGENE:
			return std::make_unique<SkillLightOfRegene>();
		case MH_NEEDLE_OF_PARALYZE:
			return std::make_unique<SkillNeedleOfParalyze>();
		case MH_OVERED_BOOST:
			return std::make_unique<SkillOveredBoost>();
		case MH_PAIN_KILLER:
			return std::make_unique<SkillPainKiller>();
		case MH_POISON_MIST:
			return std::make_unique<SkillPoisonMist>();
		case MH_SILENT_BREEZE:
			return std::make_unique<SkillSilentBreeze>();
		case MH_STYLE_CHANGE:
			return std::make_unique<SkillStyleChange>();
		case MH_SUMMON_LEGION:
			return std::make_unique<SkillSummonLegion>();
		case MH_ANGRIFFS_MODUS:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case MH_GOLDENE_FERSE:
			return std::make_unique<StatusSkillImpl>(skill_id);

		default:
			return nullptr;
	}
}
