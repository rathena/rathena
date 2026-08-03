// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_druid.hpp"

#include <cstdarg>

#include <common/random.hpp>

#include "../../battle.hpp"
#include "../../clif.hpp"
#include "../../map.hpp"
#include "../../party.hpp"
#include "../../path.hpp"
#include "../../pc.hpp"
#include "../../status.hpp"
#include "../../unit.hpp"

// Include .cpp files into the TU to optimize compile time
// For reference see unity builds or amalgamated builds
#include "aerosync.cpp"
#include "alphaclaw.cpp"
#include "aroundflower.cpp"
#include "chillingblast.cpp"
#include "chopchop.cpp"
#include "clawwave.cpp"
#include "cruelbite.cpp"
#include "cuttingwind.cpp"
#include "doubleslash.cpp"
#include "earthdrill.cpp"
#include "earthflower.cpp"
#include "earthstamp.cpp"
#include "feathersprinkle.cpp"
#include "feralclaw.cpp"
#include "flickingtornado.cpp"
#include "frenzyfang.cpp"
#include "furiousstorm.cpp"
#include "glacialmonolith.cpp"
#include "glacialnova.cpp"
#include "glacialshard.cpp"
#include "glacialstomp.cpp"
#include "gravityhole.cpp"
#include "groundbloom.cpp"
#include "hunger.cpp"
#include "icecloud.cpp"
#include "icepillar.cpp"
#include "icesplash.cpp"
#include "icetotem.cpp"
#include "lowflight.cpp"
#include "nastyslash.cpp"
#include "natureprotection.cpp"
#include "nomercyclaw.cpp"
#include "pinionshot.cpp"
#include "primalclaw.cpp"
#include "pulseofmadness.cpp"
#include "quillspear.cpp"
#include "roaringcharge.cpp"
#include "roaringpiercer.cpp"
#include "savagelunge.cpp"
#include "sharpengust.cpp"
#include "sharpenhail.cpp"
#include "shootingfeather.cpp"
#include "solidstomp.cpp"
#include "tempestflap.cpp"
#include "terraharvest.cpp"
#include "terrawave.cpp"
#include "thunderingcall.cpp"
#include "thunderingfocus.cpp"
#include "thunderingorb.cpp"
#include "typhoonwing.cpp"
#include "windbomb.cpp"

	constexpr int32 kGrowthDuration = 10000;

	void SkillFactoryDruid::addThunderingCharge(block_list *src, uint16 skill_id, uint16 skill_lv, int32 charge) {
		if (charge <= 0)
			return;

		status_change* sc = status_get_sc(src);

		if (sc == nullptr)
			return;

		sc_type type = skill_get_sc(skill_id);

		if (type == SC_NONE)
			return;

		int32 duration = skill_get_time(skill_id, skill_lv);

		if (sc->hasSCE(type)) {
			charge += sc->getSCE(type)->val3;
		}
		
		charge = std::min(6, charge);

		sc_start4(src, src, type, 100, skill_id, skill_lv, charge, 0, duration);

		if (charge == 6) {
			sc_start(src, src, SC_THUNDERING_ROD_MAX, 100, 1, duration);
		}
	}

	e_skill SkillFactoryDruid::resolve_quill_spear_skill(const status_change *sc, e_skill skill_id) {
		if (skill_id != AT_QUILL_SPEAR) {
			return skill_id;
		}

		if (sc != nullptr && sc->hasSCE(SC_APEX_PHASE)) {
			return AT_QUILL_SPEAR_S;
		}

		return skill_id;
	}

std::unique_ptr<const SkillImpl> SkillFactoryDruid::create(const e_skill skill_id) const {
	switch (skill_id) {
		case DR_AROUND_FLOWER:
			return std::make_unique<SkillAroundFlower>();
		case DR_BLOOD_HOWLING:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case DR_CRUEL_BITE:
			return std::make_unique<SkillCruelBite>();
		case DR_CUTTING_WIND:
			return std::make_unique<SkillCuttingWind>();
		case DR_EARTH_FLOWER:
			return std::make_unique<SkillEarthFlower>();
		case DR_ENRAGE_WOLF:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case DR_ENRAGE_RAPTOR:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case DR_FLICKING_TONADO:
			return std::make_unique<SkillFlickingTornado>();
		case DR_HUNGER:
			return std::make_unique<SkillHunger>();
		case DR_ICE_CLOUD:
			return std::make_unique<SkillIceCloud>();
		case DR_ICE_TOTEM:
			return std::make_unique<SkillIceTotem>();
		case DR_LOW_FLIGHT:
			return std::make_unique<SkillLowFlight>();
		case DR_NATURE_SHIELD:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case DR_NOMERCY_CLAW:
			return std::make_unique<SkillNoMercyClaw>();
		case DR_PREENING:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case DR_SHOOTING_FEATHER:
			return std::make_unique<SkillShootingFeather>();
		case DR_TRUTH_OF_EARTH:
		case DR_TRUTH_OF_ICE:
		case DR_TRUTH_OF_WIND:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case DR_WIND_BOMB:
			return std::make_unique<SkillWindBomb>();
		case DR_WEREWOLF:
		case DR_WERERAPTOR:
			return std::make_unique<StatusSkillImpl>(skill_id, true);

		case KR_CHOP_CHOP:
			return std::make_unique<SkillChopChop>();
		case KR_CLAW_WAVE:
			return std::make_unique<SkillClawWave>();
		case KR_DOUBLE_SLASH:
			return std::make_unique<SkillDoubleSlash>();
		case KR_EARTH_DRILL:
			return std::make_unique<SkillEarthDrill>();
		case KR_EARTH_STAMP:
			return std::make_unique<SkillEarthStamp>();
		case KR_FEATHER_SPRINKLE:
			return std::make_unique<SkillFeatherSprinkle>();
		case KR_GROUND_BLOOM:
			return std::make_unique<SkillGroundBloom>();
		case KR_ICE_PILLAR:
			return std::make_unique<SkillIcePillar>();
		case KR_ICE_SPLASH:
			return std::make_unique<SkillIceSplash>();
		case KR_IRON_HOWLING:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case KR_NASTY_SLASH:
			return std::make_unique<SkillNastySlash>();
		case KR_NATURE_PROTECTION:
			return std::make_unique<SkillNatureProtection>();
		case KR_SHARPEN_GUST:
			return std::make_unique<SkillSharpenGust>();
		case KR_SHARPEN_HAIL:
			return std::make_unique<SkillSharpenHail>();
		case KR_THUNDERING_CALL:
			return std::make_unique<SkillThunderingCall>();
		case KR_THUNDERING_CALL_S:
			return std::make_unique<SkillThunderingCallS>();
		case KR_THUNDERING_FOCUS:
			return std::make_unique<SkillThunderingFocus>();
		case KR_THUNDERING_FOCUS_S:
			return std::make_unique<SkillThunderingFocusS>();
		case KR_THUNDERING_ORB:
			return std::make_unique<SkillThunderingOrb>();
		case KR_THUNDERING_ORB_S:
			return std::make_unique<SkillThunderingOrbS>();
		case KR_TYPHOON_WING:
			return std::make_unique<SkillTyphoonWing>();
		case KR_WIND_VEIL:
			return std::make_unique<StatusSkillImpl>(skill_id);

		case AT_AERO_SYNC:
			return std::make_unique<SkillAeroSync>();
		case AT_ALPHA_CLAW:
			return std::make_unique<SkillAlphaClaw>();
		case AT_ALPHA_PHASE:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case AT_APEX_PHASE:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case AT_CHILLING_BLAST:
			return std::make_unique<SkillChillingBlast>();
		case AT_FERAL_CLAW:
			return std::make_unique<SkillFeralClaw>();
		case AT_FLIP_FLAP:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case AT_FRENZY_FANG:
			return std::make_unique<SkillFrenzyFang>();
		case AT_FURIOS_STORM:
			return std::make_unique<SkillFuriousStorm>();
		case AT_GLACIER_MONOLITH:
			return std::make_unique<SkillGlacialMonolith>();
		case AT_GLACIER_NOVA:
			return std::make_unique<SkillGlacialNova>();
		case AT_GLACIER_SHARD:
			return std::make_unique<SkillGlacialShard>();
		case AT_GLACIER_STOMP:
			return std::make_unique<SkillGlacialStomp>();
		case AT_GRAVITY_HOLE:
			return std::make_unique<SkillGravityHole>();
		case AT_NATURE_HARMONY:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case AT_PINION_SHOT:
			return std::make_unique<SkillPinionShot>();
		case AT_PRIMAL_CLAW:
			return std::make_unique<SkillPrimalClaw>();
		case AT_PULSE_OF_MADNESS:
			return std::make_unique<SkillPulseOfMadness>();
		case AT_QUILL_SPEAR:
			return std::make_unique<SkillQuillSpear>();
		case AT_QUILL_SPEAR_S:
			return std::make_unique<SkillQuillSpearS>();
		case AT_ROARING_CHARGE:
			return std::make_unique<SkillRoaringCharge>();
		case AT_ROARING_CHARGE_S:
			return std::make_unique<SkillRoaringChargeS>();
		case AT_ROARING_PIERCER:
			return std::make_unique<SkillRoaringPiercer>();
		case AT_ROARING_PIERCER_S:
			return std::make_unique<SkillRoaringPiercer>();
		case AT_SAVAGE_LUNGE:
			return std::make_unique<SkillSavageLunge>();
		case AT_SOLID_STOMP:
			return std::make_unique<SkillSolidStomp>();
		case AT_TEMPEST_FLAP:
			return std::make_unique<SkillTempestFlap>();
		case AT_TERRA_HARVEST:
			return std::make_unique<SkillTerraHarvest>();
		case AT_TERRA_WAVE:
			return std::make_unique<SkillTerraWave>();
		case AT_ZEPHYR_LINK:
			return std::make_unique<StatusSkillImpl>(skill_id);

		default:
			return nullptr;
	}
}
