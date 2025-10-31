// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_thief.hpp"
#include "backslide.hpp"
#include "detoxify.hpp"
#include "envenom.hpp"
#include "findstone.hpp"
#include "hiding.hpp"
#include "sandattack.hpp"
#include "steal.hpp"
#include "stonefling.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryThief::create(const e_skill skill_id) const {
	switch (skill_id) {
		case TF_BACKSLIDING:
			return std::make_unique<SkillBackSlide>();
		case TF_DETOXIFY:
			return std::make_unique<SkillDetoxify>();
		case TF_DOUBLE:
			return std::make_unique<WeaponSkillImpl>(TF_DOUBLE);
		case TF_POISON:
			return std::make_unique<SkillEnvenom>();
		case TF_PICKSTONE:
			return std::make_unique<SkillFindStone>();
		case TF_HIDING:
			return std::make_unique<SkillHiding>();
		case TF_SPRINKLESAND:
			return std::make_unique<SkillSandAttack>();
		case TF_STEAL:
			return std::make_unique<SkillSteal>();
		case TF_THROWSTONE:
			return std::make_unique<SkillStoneFling>();
		default:
			return nullptr;
	}
}
