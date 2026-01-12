// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_archer.hpp"

#include "../status_skill_impl.hpp"
#include "../weapon_skill_impl.hpp"

#include "anklesnare.hpp"
#include "arrowshower.hpp"
#include "blastmine.hpp"
#include "blitzbeat.hpp"
#include "chargearrow.hpp"
#include "claymoretrap.hpp"
#include "concentration.hpp"
#include "detect.hpp"
#include "doublestrafe.hpp"
#include "flasher.hpp"
#include "freezingtrap.hpp"
#include "landmine.hpp"
#include "makingarrow.hpp"
#include "removetrap.hpp"
#include "sandman.hpp"
#include "shockwavetrap.hpp"
#include "skidtrap.hpp"
#include "springtrap.hpp"
#include "talkiebox.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryArcher::create(const e_skill skill_id) const {
	switch( skill_id ){
		case AC_CHARGEARROW:
			return std::make_unique<SkillChargeArrow>();
		case AC_CONCENTRATION:
			return std::make_unique<SkillConcentration>();
		case AC_DOUBLE:
			return std::make_unique<SkillDoubleStrafe>();
		case AC_MAKINGARROW:
			return std::make_unique<SkillMakingArrow>();
		case AC_SHOWER:
			return std::make_unique<SkillArrowShower>();
		case BA_DISSONANCE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case BA_MUSICALSTRIKE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case CG_ARROWVULCAN:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case DC_THROWARROW:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case HT_ANKLESNARE:
			return std::make_unique<SkillAnkleSnare>();
		case HT_BLASTMINE:
			return std::make_unique<SkillBlastMine>();
		case HT_BLITZBEAT:
			return std::make_unique<SkillBlitzBeat>();
		case HT_CLAYMORETRAP:
			return std::make_unique<SkillClaymoreTrap>();
		case HT_DETECTING:
			return std::make_unique<SkillDetect>();
		case HT_FLASHER:
			return std::make_unique<SkillFlasher>();
		case HT_FREEZINGTRAP:
			return std::make_unique<SkillFreezingTrap>();
		case HT_LANDMINE:
			return std::make_unique<SkillLandMine>();
		case HT_PHANTASMIC:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case HT_REMOVETRAP:
			return std::make_unique<SkillRemoveTrap>();
		case HT_SANDMAN:
			return std::make_unique<SkillSandman>();
		case HT_SHOCKWAVE:
			return std::make_unique<SkillShockwaveTrap>();
		case HT_SKIDTRAP:
			return std::make_unique<SkillSkidTrap>();
		case HT_SPRINGTRAP:
			return std::make_unique<SkillSpringTrap>();
		case HT_TALKIEBOX:
			return std::make_unique<SkillTalkieBox>();
		case RA_AIMEDBOLT:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case RA_UNLIMIT:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SN_SIGHT:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case TR_KVASIR_SONATA:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case TR_MYSTIC_SYMPHONY:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case WH_CALAMITYGALE:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case WH_WIND_SIGN:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case WM_GREAT_ECHO:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case WM_SEVERE_RAINSTORM_MELEE:
			return std::make_unique<WeaponSkillImpl>(skill_id);

		default:
			return nullptr;
	}

	return nullptr;
}
