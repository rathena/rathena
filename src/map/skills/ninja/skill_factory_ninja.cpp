// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_ninja.hpp"

#include "../skill_impl.hpp"

// Include .cpp files into the TU to optimize compile time
// For reference see unity builds or amalgamated builds
#include "coldbloodedcannon.cpp"
#include "crimsonfireformation.cpp"
#include "crimsonfirepetal.cpp"
#include "darkdragonnightmare.cpp"
#include "darkeningcannon.cpp"
#include "finalstrike.cpp"
#include "fourcolorscharm.cpp"
#include "goldendragoncannon.cpp"
#include "hiddenwater.cpp"
#include "huumashurikenconstruct.cpp"
#include "huumashurikengrasp.cpp"
#include "icemeteor.cpp"
#include "improviseddefense.cpp"
#include "infiltrate.cpp"
#include "kamaitachi.cpp"
#include "kunaidistortion.cpp"
#include "kunainightmare.cpp"
#include "kunairefraction.cpp"
#include "kunairotation.cpp"
#include "lightningstrikeofdestruction.cpp"
#include "meltaway.cpp"
#include "mirage.cpp"
#include "mirrorimage.cpp"
#include "nightmareerasion.cpp"
#include "ragingfiredragon.cpp"
#include "redflamecannon.cpp"
#include "shadowdance.cpp"
#include "shadowflash.cpp"
#include "shadowhunting.cpp"
#include "shadowleap.cpp"
#include "shadownightmare.cpp"
#include "shadowslash.cpp"
#include "spearofice.cpp"
#include "throwhuumashuriken.cpp"
#include "throwkunai.cpp"
#include "throwshuriken.cpp"
#include "throwzeny.cpp"
#include "thunderingcannon.cpp"
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
		case SS_AKUMUKESU:
			return std::make_unique<SkillNightmareErasion>();
		case SS_ANKOKURYUUAKUMU:
			return std::make_unique<SkillDarkDragonNightmare>();
		case SS_ANTENPOU:
			return std::make_unique<SkillDarkeningCannon>();
		case SS_FOUR_CHARM:
			return std::make_unique<SkillFourColorsCharm>();
		case SS_FUUMAKOUCHIKU:
			return std::make_unique<SkillHuumaShurikenConstruct>();
		case SS_FUUMASHOUAKU:
			return std::make_unique<SkillHuumaShurikenGrasp>();
		case SS_HITOUAKUMU:
			return std::make_unique<SkillKunaiNightmare>();
		case SS_KAGEAKUMU:
			return std::make_unique<SkillShadowNightmare>();
		case SS_KAGEGARI:
			return std::make_unique<SkillShadowHunting>();
		case SS_KAGEGISSEN:
			return std::make_unique<SkillShadowFlash>();
		case SS_KAGENOMAI:
			return std::make_unique<SkillShadowDance>();
		case SS_KINRYUUHOU:
			return std::make_unique<SkillGoldenDragonCannon>();
		case SS_KUNAIKAITEN:
			return std::make_unique<SkillKunaiRotation>();
		case SS_KUNAIKUSSETSU:
			return std::make_unique<SkillKunaiRefraction>();
		case SS_KUNAIWAIKYOKU:
			return std::make_unique<SkillKunaiDistortion>();
		case SS_RAIDENPOU:
			return std::make_unique<SkillThunderingCannon>();
		case SS_REIKETSUHOU:
			return std::make_unique<SkillColdBloodedCannon>();
		case SS_SEKIENHOU:
			return std::make_unique<SkillRedFlameCannon>();
		case SS_SHIMIRU:
			return std::make_unique<SkillInfiltrate>();
		case SS_SHINKIROU:
			return std::make_unique<SkillMirage>();
		case SS_TOKEDASU:
			return std::make_unique<SkillMeltAway>();

		default:
			return nullptr;
	}
}
