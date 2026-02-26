// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_mercenary.hpp"

#include "../skill_impl.hpp"

// Include .cpp files into the TU to optimize compile time
// For reference see unity builds or amalgamated builds
#include "mercenary_arrowrepel.cpp"
#include "mercenary_arrowshower.cpp"
#include "mercenary_bash.cpp"
#include "mercenary_benediction.cpp"
#include "mercenary_blessing.cpp"
#include "mercenary_bowlingbash.cpp"
#include "mercenary_brandishspear.cpp"
#include "mercenary_compress.cpp"
#include "mercenary_crash.cpp"
#include "mercenary_decreaseagi.cpp"
#include "mercenary_doublestrafe.cpp"
#include "mercenary_focusedarrowstrike.cpp"
#include "mercenary_freezingtrap.cpp"
#include "mercenary_increaseagility.cpp"
#include "mercenary_kyrieeleison.cpp"
#include "mercenary_landmine.cpp"
#include "mercenary_lexdivina.cpp"
#include "mercenary_magnificat.cpp"
#include "mercenary_magnumbreak.cpp"
#include "mercenary_mentalcure.cpp"
#include "mercenary_mindblaster.cpp"
#include "mercenary_pierce.cpp"
#include "mercenary_provoke.cpp"
#include "mercenary_recuperate.cpp"
#include "mercenary_regain.cpp"
#include "mercenary_removetrap.cpp"
#include "mercenary_sacrifice.cpp"
#include "mercenary_sandman.cpp"
#include "mercenary_scapegoat.cpp"
#include "mercenary_sense.cpp"
#include "mercenary_shieldreflect.cpp"
#include "mercenary_sight.cpp"
#include "mercenary_skidtrap.cpp"
#include "mercenary_spiralpierce.cpp"
#include "mercenary_tender.cpp"

std::unique_ptr<const SkillImpl> SkillFactoryMercenary::create(const e_skill skill_id) const {
	switch( skill_id ){
		case ABR_BATTLE_BUSTER:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case ABR_DUAL_CANNON_FIRE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case ABR_INFINITY_BUSTER:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case MA_CHARGEARROW:
			return std::make_unique<SkillMercenaryArrowRepel>();
		case MA_DOUBLE:
			return std::make_unique<SkillMercenaryDoubleStrafe>();
		case MA_FREEZINGTRAP:
			return std::make_unique<SkillMercenaryFreezingTrap>();
		case MA_LANDMINE:
			return std::make_unique<SkillMercenaryLandMine>();
		case MA_REMOVETRAP:
			return std::make_unique<SkillMercenaryRemoveTrap>();
		case MA_SANDMAN:
			return std::make_unique<SkillMercenarySandman>();
		case MA_SHARPSHOOTING:
			return std::make_unique<SkillMercenaryFocusedArrowStrike>();
		case MA_SHOWER:
			return std::make_unique<SkillMercenaryArrowShower>();
		case MA_SKIDTRAP:
			return std::make_unique<SkillMercenarySkidTrap>();
		case MER_AUTOBERSERK:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case MER_BENEDICTION:
			return std::make_unique<SkillMercenaryBenediction>();
		case MER_BLESSING:
			return std::make_unique<SkillMercenaryBlessing>();
		case MER_COMPRESS:
			return std::make_unique<SkillMercenaryCompress>();
		case MER_CRASH:
			return std::make_unique<SkillMercenaryCrash>();
		case MER_DECAGI:
			return std::make_unique<SkillMercenaryDecreaseAgi>();
		case MER_ESTIMATION:
			return std::make_unique<SkillMercenarySense>();
		case MER_INCAGI:
			return std::make_unique<SkillMercenaryIncreaseAgility>();
		case MER_INVINCIBLEOFF2:
			return std::make_unique<SkillMercenaryMindBlaster>();
		case MER_KYRIE:
			return std::make_unique<SkillMercenaryKyrieEleison>();
		case MER_LEXDIVINA:
			return std::make_unique<SkillMercenaryLexDivina>();
		case MER_MAGNIFICAT:
			return std::make_unique<SkillMercenaryMagnificat>();
		case MER_MENTALCURE:
			return std::make_unique<SkillMercenaryMentalCure>();
		case MER_PROVOKE:
			return std::make_unique<SkillMercenaryProvoke>();
		case MER_QUICKEN:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case MER_RECUPERATE:
			return std::make_unique<SkillMercenaryRecuperate>();
		case MER_REGAIN:
			return std::make_unique<SkillMercenaryRegain>();
		case MER_SCAPEGOAT:
			return std::make_unique<SkillMercenaryScapegoat>();
		case MER_SIGHT:
			return std::make_unique<SkillMercenarySight>();
		case MER_TENDER:
			return std::make_unique<SkillMercenaryTender>();
		case ML_AUTOGUARD:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case ML_BRANDISH:
			return std::make_unique<SkillMercenaryBrandishSpear>();
		case ML_DEFENDER:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case ML_DEVOTION:
			return std::make_unique<SkillMercenarySacrifice>();
		case ML_PIERCE:
			return std::make_unique<SkillMercenaryPierce>();
		case ML_SPIRALPIERCE:
			return std::make_unique<SkillMercenarySpiralPierce>();
		case MS_BASH:
			return std::make_unique<SkillMercenaryBash>();
		case MS_BERSERK:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case MS_BOWLINGBASH:
			return std::make_unique<SkillMercenaryBowlingBash>();
		case MS_MAGNUM:
			return std::make_unique<SkillMercenaryMagnumBreak>();
		case MS_PARRYING:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case MS_REFLECTSHIELD:
			return std::make_unique<SkillMercenaryShieldReflect>();

		default:
			return nullptr;
	}
}
