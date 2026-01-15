// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_thief.hpp"

#include "../status_skill_impl.hpp"
#include "../weapon_skill_impl.hpp"

#include "backslide.hpp"
#include "backstab.hpp"
#include "cloaking.hpp"
#include "closeconfine.hpp"
#include "detoxify.hpp"
#include "divestarmor.hpp"
#include "divesthelm.hpp"
#include "divestshield.hpp"
#include "divestweapon.hpp"
#include "enchantpoison.hpp"
#include "envenom.hpp"
#include "findstone.hpp"
#include "grimtooth.hpp"
#include "hiding.hpp"
#include "mug.hpp"
#include "remover.hpp"
#include "sandattack.hpp"
#include "scribble.hpp"
#include "sightlessmind.hpp"
#include "snatch.hpp"
#include "sonicblow.hpp"
#include "steal.hpp"
#include "stonefling.hpp"
#include "throwvenomknife.hpp"
#include "venomdust.hpp"
#include "venomsplasher.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryThief::create(const e_skill skill_id) const {
	switch (skill_id) {
		case ABC_ABYSS_SLAYER:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case ABC_CHAIN_REACTION_SHOT_ATK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case AS_CLOAKING:
			return std::make_unique<SkillCloaking>();
		case AS_ENCHANTPOISON:
			return std::make_unique<SkillEnchantPoison>();
		case AS_GRIMTOOTH:
			return std::make_unique<SkillGrimtooth>();
		case AS_POISONREACT:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case AS_SONICBLOW:
			return std::make_unique<SkillSonicBlow>();
		case AS_SPLASHER:
			return std::make_unique<SkillVenomSplasher>();
		case AS_VENOMDUST:
			return std::make_unique<SkillVenomDust>();
		case AS_VENOMKNIFE:
			return std::make_unique<SkillThrowVenomKnife>();
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
