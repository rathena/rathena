// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_rogue.hpp"

#include "backstab.hpp"
#include "closeconfine.hpp"
#include "divestarmor.hpp"
#include "divesthelm.hpp"
#include "divestshield.hpp"
#include "divestweapon.hpp"
#include "mug.hpp"
#include "remover.hpp"
#include "scribble.hpp"
#include "sightlessmind.hpp"
#include "snatch.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryRogue::create(const e_skill skill_id) const {
	switch (skill_id) {
		case RG_BACKSTAP:
			return std::make_unique<SkillBackStab>();
		case RG_CLEANER:
			return std::make_unique<SkillRemover>();
		case RG_CLOSECONFINE:
			return std::make_unique<SkillCloseConfine>();
		case RG_GRAFFITI:
			return std::make_unique<SkillScribble>();
		case RG_INTIMIDATE:
			return std::make_unique<SkillSnatch>();
		case RG_RAID:
			return std::make_unique<SkillSightlessMind>();
		case RG_STEALCOIN:
			return std::make_unique<SkillMug>();
		case RG_STRIPWEAPON:
			return std::make_unique<SkillDivestWeapon>();
		case RG_STRIPSHIELD:
			return std::make_unique<SkillDivestShield>();
		case RG_STRIPARMOR:
			return std::make_unique<SkillDivestArmor>();
		case RG_STRIPHELM:
			return std::make_unique<SkillDivestHelm>();
		default:
			return nullptr;
	}
}
