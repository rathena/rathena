// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_summoner.hpp"

#include "../skill_impl.hpp"

// Include .cpp files into the TU to optimize compile time
// For reference see unity builds or amalgamated builds
#include "bite.cpp"
#include "bunchofshrimp.cpp"
#include "catnipmeteor.cpp"
#include "catnippowdering.cpp"
#include "chattering.cpp"
#include "grooming.cpp"
#include "hiss.cpp"
#include "lope.cpp"
#include "lunaticcarrotbeat.cpp"
#include "meowmeow.cpp"
#include "nyanggrass.cpp"
#include "pickypeck.cpp"
#include "powerofflock.cpp"
#include "purring.cpp"
#include "scaroftarou.cpp"
#include "scratch.cpp"
#include "silvervineroottwist.cpp"
#include "silvervinestemspear.cpp"
#include "spiritofsavage.cpp"
#include "tastyshrimpparty.cpp"
#include "tunabelly.cpp"
#include "tunaparty.cpp"

std::unique_ptr<const SkillImpl> SkillFactorySummoner::create(const e_skill skill_id) const {
	switch( skill_id ){
		case SH_TEMPORARY_COMMUNION:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SU_ARCLOUSEDASH:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SU_BITE:
			return std::make_unique<SkillBite>();
		case SU_BUNCHOFSHRIMP:
			return std::make_unique<SkillBunchofShrimp>();
		case SU_CHATTERING:
			return std::make_unique<SkillChattering>();	// FIX ME: this skill seems to be StatusSkillImpl
		case SU_CN_METEOR:
			return std::make_unique<SkillCatnipMeteor>();
		case SU_CN_METEOR2:
			return std::make_unique<SkillCatnipMeteor2>();
		case SU_CN_POWDERING:
			return std::make_unique<SkillCatnipPowdering>();
		case SU_FRESHSHRIMP:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SU_GROOMING:
			return std::make_unique<SkillGrooming>();	// FIX ME: this skill seems to be StatusSkillImpl
		case SU_HIDE:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case SU_HISS:
			return std::make_unique<SkillHiss>();
		case SU_LOPE:
			return std::make_unique<SkillLope>();
		case SU_LUNATICCARROTBEAT:
			return std::make_unique<SkillLunaticCarrotBeat>();
		case SU_LUNATICCARROTBEAT2:
			return std::make_unique<SkillLunaticCarrotBeat2>();
		case SU_MEOWMEOW:
			return std::make_unique<SkillMeowMeow>();
		case SU_NYANGGRASS:
			return std::make_unique<SkillNyangGrass>();
		case SU_PICKYPECK:
			return std::make_unique<SkillPickyPeck>();
		case SU_PICKYPECK_DOUBLE_ATK:
			return std::make_unique<SkillPickyPeckDoubleAttack>();
		case SU_POWEROFFLOCK:
			return std::make_unique<SkillPowerofFlock>();
		case SU_PURRING:
			return std::make_unique<SkillPurring>();
		case SU_SCAROFTAROU:
			return std::make_unique<SkillScarofTarou>();
		case SU_SCRATCH:
			return std::make_unique<SkillScratch>();
		case SU_SHRIMPARTY:
			return std::make_unique<SkillTastyShrimpParty>();
		case SU_STOOP:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SU_SVG_SPIRIT:
			return std::make_unique<SkillSpiritofSavage>();
		case SU_SV_ROOTTWIST:
			return std::make_unique<SkillSilvervineRootTwist>();
		case SU_SV_STEMSPEAR:
			return std::make_unique<SkillSilvervineStemSpear>();
		case SU_TUNABELLY:
			return std::make_unique<SkillTunaBelly>();
		case SU_TUNAPARTY:
			return std::make_unique<SkillTunaParty>();

		default:
			return nullptr;
	}
}
