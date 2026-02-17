// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_ninja.hpp"

#include "../skill_impl.hpp"

// Include .cpp files into the TU to optimize compile time
// For reference see unity builds or amalgamated builds
#include "crimsonfireformation.cpp"
#include "crimsonfirepetal.cpp"
#include "finalstrike.cpp"
#include "hiddenwater.cpp"
#include "icemeteor.cpp"
#include "improviseddefense.cpp"
#include "kamaitachi.cpp"
#include "lightningstrikeofdestruction.cpp"
#include "mirrorimage.cpp"
#include "ragingfiredragon.cpp"
#include "shadowleap.cpp"
#include "shadowslash.cpp"
#include "spearofice.cpp"
#include "throwhuumashuriken.cpp"
#include "throwkunai.cpp"
#include "throwshuriken.cpp"
#include "throwzeny.cpp"
#include "vanishingslash.cpp"
#include "windblade.cpp"

std::unique_ptr<const SkillImpl> SkillFactoryNinja::create(const e_skill skill_id) const {
	switch( skill_id ){
		case KO_MEIKYOUSISUI:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case KO_SETSUDAN:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NJ_BAKUENRYU:
			return std::make_unique<SkillRagingFireDragon>();
		case NJ_BUNSINJYUTSU:
			return std::make_unique<SkillMirrorImage>();
		case NJ_HUUJIN:
			return std::make_unique<SkillWindBlade>();
		case NJ_HUUMA:
			return std::make_unique<SkillThrowHuumaShuriken>();
		case NJ_HYOUSENSOU:
			return std::make_unique<SkillSpearOfIce>();
		case NJ_HYOUSYOURAKU:
			return std::make_unique<SkillIceMeteor>();
		case NJ_ISSEN:
			return std::make_unique<SkillFinalStrike>();
		case NJ_KAENSIN:
			return std::make_unique<SkillCrimsonFireFormation>();
		case NJ_KAMAITACHI:
			return std::make_unique<SkillKamaitachi>();
		case NJ_KASUMIKIRI:
			return std::make_unique<SkillVanishingSlash>();
		case NJ_KIRIKAGE:
			return std::make_unique<SkillShadowSlash>();
		case NJ_KOUENKA:
			return std::make_unique<SkillCrimsonFirePetal>();
		case NJ_KUNAI:
			return std::make_unique<SkillThrowKunai>();
		case NJ_NEN:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case NJ_RAIGEKISAI:
			return std::make_unique<SkillLightningStrikeOfDestruction>();
		case NJ_SHADOWJUMP:
			return std::make_unique<SkillShadowLeap>();
		case NJ_SUITON:
			return std::make_unique<SkillHiddenWater>();
		case NJ_SYURIKEN:
			return std::make_unique<SkillThrowShuriken>();
		case NJ_TATAMIGAESHI:
			return std::make_unique<SkillImprovisedDefense>();
		case NJ_UTSUSEMI:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case NJ_ZENYNAGE:
			return std::make_unique<SkillThrowZeny>();
		case SS_FUUMAKOUCHIKU:
			return std::make_unique<WeaponSkillImpl>(skill_id);

		default:
			return nullptr;
	}
}
