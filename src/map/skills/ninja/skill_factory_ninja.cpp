// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_ninja.hpp"

#include "improviseddefense.hpp"
#include "shurikentraining.hpp"
#include "cicadaskinsheeding.hpp"
#include "throwzeny.hpp"
#include "vanishingslash.hpp"
#include "shadowslash.hpp"
#include "crimsonfirepetal.hpp"
#include "throwkunai.hpp"
#include "soul.hpp"
#include "spiritoftheblade.hpp"
#include "lightningstrikeofdestruction.hpp"
#include "shadowleap.hpp"
#include "hiddenwater.hpp"
#include "throwshuriken.hpp"
#include "ragingfiredragon.hpp"
#include "mirrorimage.hpp"
#include "windblade.hpp"
#include "throwhuumashuriken.hpp"
#include "spearofice.hpp"
#include "icemeteor.hpp"
#include "finalstrike.hpp"
#include "crimsonfireformation.hpp"
#include "kamaitachi.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryNinja::create(const e_skill skill_id) const {
	switch (skill_id) {
		case NJ_TOBIDOUGU:
			return std::make_unique<SkillShurikenTraining>();
		case NJ_SYURIKEN:
			return std::make_unique<SkillThrowShuriken>();
		case NJ_KUNAI:
			return std::make_unique<SkillThrowKunai>();
		case NJ_HUUMA:
			return std::make_unique<SkillThrowHuumaShuriken>();
		case NJ_ZENYNAGE:
			return std::make_unique<SkillThrowZeny>();
		case NJ_TATAMIGAESHI:
			return std::make_unique<SkillImprovisedDefense>();
		case NJ_KASUMIKIRI:
			return std::make_unique<SkillVanishingSlash>();
		case NJ_SHADOWJUMP:
			return std::make_unique<SkillShadowLeap>();
		case NJ_KIRIKAGE:
			return std::make_unique<SkillShadowSlash>();
		case NJ_UTSUSEMI:
			return std::make_unique<SkillCicadaSkinSheeding>();
		case NJ_BUNSINJYUTSU:
			return std::make_unique<SkillMirrorImage>();
		case NJ_NINPOU:
			return std::make_unique<SkillSpiritOfTheBlade>();
		case NJ_KOUENKA:
			return std::make_unique<SkillCrimsonFirePetal>();
		case NJ_KAENSIN:
			return std::make_unique<SkillCrimsonFireFormation>();
		case NJ_BAKUENRYU:
			return std::make_unique<SkillRagingFireDragon>();
		case NJ_HYOUSENSOU:
			return std::make_unique<SkillSpearOfIce>();
		case NJ_SUITON:
			return std::make_unique<SkillHiddenWater>();
		case NJ_HYOUSYOURAKU:
			return std::make_unique<SkillIceMeteor>();
		case NJ_HUUJIN:
			return std::make_unique<SkillWindBlade>();
		case NJ_RAIGEKISAI:
			return std::make_unique<SkillLightningStrikeOfDestruction>();
		case NJ_KAMAITACHI:
			return std::make_unique<SkillKamaitachi>();
		case NJ_NEN:
			return std::make_unique<SkillSoul>();
		case NJ_ISSEN:
			return std::make_unique<SkillFinalStrike>();
		default:
			return nullptr;
	}
}
