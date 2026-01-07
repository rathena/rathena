// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_thief.hpp"

#include "../status_skill_impl.hpp"
#include "../weapon_skill_impl.hpp"

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
		case ABC_ABYSS_SLAYER:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case ABC_CHAIN_REACTION_SHOT_ATK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case AS_POISONREACT:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case AS_SONICBLOW:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case AS_VENOMKNIFE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case ASC_BREAKER:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case GC_VENOMIMPRESS:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case GC_VENOMPRESSURE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case GC_WEAPONBLOCKING:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case GC_WEAPONCRUSH:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case RG_INTIMIDATE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case SC_DEADLYINFECT:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SC_FEINTBOMB:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case SC_TRIANGLESHOT:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case SHC_ENCHANTING_SHADOW:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SHC_POTENT_VENOM:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SHC_SHADOW_EXCEED:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case ST_PRESERVE:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case ST_REJECTSWORD:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case TF_BACKSLIDING:
			return std::make_unique<SkillBackSlide>();
		case TF_DETOXIFY:
			return std::make_unique<SkillDetoxify>();
		case TF_DOUBLE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
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
