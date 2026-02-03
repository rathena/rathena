// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_swordman.hpp"

#include "../status_skill_impl.hpp"
#include "../weapon_skill_impl.hpp"

#include "autoberserk.hpp"
#include "bash.hpp"
#include "bowlingbash.hpp"
#include "brandishspear.hpp"
#include "counterattack.hpp"
#include "dragonicaura.hpp"
#include "dragonicbreath.hpp"
#include "dragonicpierce.hpp"
#include "grandcross.hpp"
#include "hackandslasher.hpp"
#include "holycross.hpp"
#include "madnesscrusher.hpp"
#include "magnum.hpp"
#include "pierce.hpp"
#include "provoke.hpp"
#include "relax.hpp"
#include "resistantsouls.hpp"
#include "sacrifice.hpp"
#include "selfprovoke.hpp"
#include "servantweapon.hpp"
#include "servantweapondemolition.hpp"
#include "servantweaponphantom.hpp"
#include "servantweaponsign.hpp"
#include "shieldboomerang.hpp"
#include "shieldreflect.hpp"
#include "smite.hpp"
#include "spearboomerang.hpp"
#include "spearstab.hpp"
#include "spiralpierce.hpp"
#include "stormslash.hpp"
#include "traumaticblow.hpp"
#include "vitalstrike.hpp"

std::unique_ptr<const SkillImpl> SkillFactorySwordman::create(const e_skill skill_id) const {
	switch( skill_id ){
		case CR_AUTOGUARD:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case CR_DEFENDER:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case CR_DEVOTION:
			return std::make_unique<SkillSacrifice>();
		case CR_GRANDCROSS:
			return std::make_unique<SkillGrandCross>();
		case CR_HOLYCROSS:
			return std::make_unique<SkillHolyCross>();
		case CR_PROVIDENCE:
			return std::make_unique<SkillResistantSouls>();
		case CR_REFLECTSHIELD:
			return std::make_unique<SkillShieldReflect>();
		case CR_SHIELDBOOMERANG:
			return std::make_unique<SkillShieldBoomerang>();
		case CR_SHIELDCHARGE:
			return std::make_unique<SkillSmite>();
		case CR_SHRINK:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case CR_SPEARQUICKEN:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case DK_CHARGINGPIERCE:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case DK_DRAGONIC_AURA:
			return std::make_unique<SkillDragonicAura>();
		case DK_DRAGONIC_BREATH:
			return std::make_unique<SkillDragonicBreath>();
		case DK_DRAGONIC_PIERCE:
			return std::make_unique<SkillDragonicPierce>();
		case DK_HACKANDSLASHER:
			return std::make_unique<SkillHackAndSlasher>();
		case DK_HACKANDSLASHER_ATK:
			return std::make_unique<SkillHackAndSlasherAttack>();
		case DK_MADNESS_CRUSHER:
			return std::make_unique<SkillMadnessCrusher>();
		case DK_SERVANTWEAPON:
			return std::make_unique<SkillServantWeapon>();
		case DK_SERVANTWEAPON_ATK:
			return std::make_unique<SkillServantWeaponAttack>();
		case DK_SERVANT_W_DEMOL:
			return std::make_unique<SkillServantWeaponDemolition>();
		case DK_SERVANT_W_PHANTOM:
			return std::make_unique<SkillServantWeaponPhantom>();
		case DK_SERVANT_W_SIGN:
			return std::make_unique<SkillServantWeaponSign>();
		case DK_STORMSLASH:
			return std::make_unique<SkillStormSlash>();
		case DK_VIGOR:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case IG_ATTACK_STANCE:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case IG_GUARD_STANCE:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case IG_HOLY_SHIELD:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case IG_REBOUND_SHIELD:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case KN_BOWLINGBASH:
			return std::make_unique<SkillBowlingBash>();
		case KN_BRANDISHSPEAR:
			return std::make_unique<SkillBrandishSpear>();
		case KN_AUTOCOUNTER:
			return std::make_unique<SkillCounterAttack>();
		case KN_ONEHAND:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case KN_PIERCE:
			return std::make_unique<SkillPierce>();
		case KN_SPEARBOOMERANG:
			return std::make_unique<SkillSpearBoomerang>();
		case KN_SPEARSTAB:
			return std::make_unique<SkillSpearStab>();
		case KN_TWOHANDQUICKEN:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case LG_BANISHINGPOINT:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case LG_EXEEDBREAK:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case LG_HESPERUSLIT:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case LG_INSPIRATION:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case LG_PRESTIGE:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case LG_RAGEBURST:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case LG_REFLECTDAMAGE:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case LG_SHIELDPRESS:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case LK_AURABLADE:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case LK_BERSERK:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case LK_CONCENTRATION:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case LK_HEADCRUSH:
			return std::make_unique<SkillTraumaticBlow>();
		case LK_JOINTBEAT:
			return std::make_unique<SkillVitalStrike>();
		case LK_PARRYING:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case LK_SPIRALPIERCE:
			return std::make_unique<SkillSpiralPierce>();
		case LK_TENSIONRELAX:
			return std::make_unique<SkillRelax>();
		case PA_SACRIFICE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case PA_SHIELDCHAIN:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case RK_DEATHBOUND:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case RK_SONICWAVE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case SM_AUTOBERSERK:
			return std::make_unique<SkillAutoBerserk>();
		case SM_BASH:
			return std::make_unique<SkillBash>();
		case SM_ENDURE:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SM_MAGNUM:
			return std::make_unique<SkillMagnumBreak>();
		case SM_PROVOKE:
			return std::make_unique<SkillProvoke>();
		case SM_SELFPROVOKE:
			return std::make_unique<SkillProvokeSelf>();

		default:
			return nullptr;
	}
}
