// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_ninja.hpp"

#include "../status_skill_impl.hpp"
#include "../weapon_skill_impl.hpp"

#include "crimsonfireformation.hpp"
#include "crimsonfirepetal.hpp"
#include "finalstrike.hpp"
#include "hiddenwater.hpp"
#include "icemeteor.hpp"
#include "improviseddefense.hpp"
#include "kamaitachi.hpp"
#include "lightningstrikeofdestruction.hpp"
#include "mirrorimage.hpp"
#include "ragingfiredragon.hpp"
#include "shadowleap.hpp"
#include "shadowslash.hpp"
#include "shurikentraining.hpp"
#include "spearofice.hpp"
#include "throwhuumashuriken.hpp"
#include "throwkunai.hpp"
#include "throwshuriken.hpp"
#include "throwzeny.hpp"
#include "vanishingslash.hpp"
#include "windblade.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryNinja::create(const e_skill skill_id) const {
	switch (skill_id) {
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
		case NJ_TOBIDOUGU:
			return std::make_unique<SkillShurikenTraining>();
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
