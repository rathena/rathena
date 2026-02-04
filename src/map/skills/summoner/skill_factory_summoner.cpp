// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_summoner.hpp"

#include "../status_skill_impl.hpp"

#include "bite.hpp"
#include "bunchofshrimp.hpp"
#include "catnipmeteor.hpp"
#include "catnippowdering.hpp"
#include "hiss.hpp"
#include "lope.hpp"
#include "lunaticcarrotbeat.hpp"
#include "meowmeow.hpp"
#include "nyanggrass.hpp"
#include "pickypeck.hpp"
#include "powerofflock.hpp"
#include "purring.hpp"
#include "scaroftarou.hpp"
#include "scratch.hpp"
#include "silvervineroottwist.hpp"
#include "silvervinestemspear.hpp"
#include "spiritofsavage.hpp"
#include "tastyshrimpparty.hpp"
#include "tunabelly.hpp"
#include "tunaparty.hpp"

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
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SU_CN_METEOR2:
			return std::make_unique<SkillCatnipMeteor2>();
		case SU_CN_METEOR:
			return std::make_unique<SkillCatnipMeteor>();
		case SU_CN_POWDERING:
			return std::make_unique<SkillCatnipPowdering>();
		case SU_FRESHSHRIMP:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SU_GROOMING:
			return std::make_unique<StatusSkillImpl>(skill_id);
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
