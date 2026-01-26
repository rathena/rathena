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
#include "../status_skill_impl.hpp"

#include "aerosync.hpp"
#include "alphaclaw.hpp"
#include "alphaphase.hpp"
#include "apexphase.hpp"
#include "aroundflower.hpp"
#include "chillingblast.hpp"
#include "chopchop.hpp"
#include "clawwave.hpp"
#include "cruelbite.hpp"
#include "cuttingwind.hpp"
#include "doubleslash.hpp"
#include "earthdrill.hpp"
#include "earthflower.hpp"
#include "earthstamp.hpp"
#include "feathersprinkle.hpp"
#include "feralclaw.hpp"
#include "flickingtornado.hpp"
#include "flipflap.hpp"
#include "frenzyfang.hpp"
#include "furiousstorm.hpp"
#include "glacialmonolith.hpp"
#include "glacialnova.hpp"
#include "glacialshard.hpp"
#include "glacialstomp.hpp"
#include "gravityhole.hpp"
#include "groundbloom.hpp"
#include "hunger.hpp"
#include "icecloud.hpp"
#include "icepillar.hpp"
#include "icesplash.hpp"
#include "icetotem.hpp"
#include "lowflight.hpp"
#include "nastyslash.hpp"
#include "natureharmony.hpp"
#include "natureprotection.hpp"
#include "nomercyclaw.hpp"
#include "pinionshot.hpp"
#include "primalclaw.hpp"
#include "pulseofmadness.hpp"
#include "quillspear.hpp"
#include "roaringcharge.hpp"
#include "roaringpiercer.hpp"
#include "savagelunge.hpp"
#include "sharpengust.hpp"
#include "sharpenhail.hpp"
#include "shootingfeather.hpp"
#include "solidstomp.hpp"
#include "tempestflap.hpp"
#include "terraharvest.hpp"
#include "terrawave.hpp"
#include "thunderingcall.hpp"
#include "thunderingfocus.hpp"
#include "thunderingorb.hpp"
#include "transformationbeast.hpp"
#include "transformationraptor.hpp"
#include "truthofearth.hpp"
#include "truthofice.hpp"
#include "truthofwind.hpp"
#include "typhoonwing.hpp"
#include "windbomb.hpp"
#include "windveil.hpp"
#include "zephyrlink.hpp"

	constexpr int32 kClawChainDuration = 5000;
	constexpr int32 kThunderingChargeDuration = 10000;
	constexpr int32 kGrowthDuration = 10000;

	int32 SkillFactoryDruid::get_madness_stage(const status_change *sc) {
		if (sc == nullptr) {
			return 0;
		}

		if (sc->hasSCE(SC_ALPHA_PHASE) || sc->hasSCE(SC_INSANE3)) {
			return 3;
		}
		if (sc->hasSCE(SC_INSANE2)) {
			return 2;
		}
		if (sc->hasSCE(SC_INSANE)) {
			return 1;
		}

		return 0;
	}

	bool is_thundering_charge_skill(e_skill skill_id) {
		switch (skill_id) {
			case KR_THUNDERING_FOCUS:
			case KR_THUNDERING_ORB:
			case KR_THUNDERING_CALL:
			case AT_ROARING_PIERCER:
			case AT_ROARING_CHARGE:
				return true;
			default:
				return false;
		}
	}

	e_skill SkillFactoryDruid::resolve_thundering_charge_skill(const status_change *sc, e_skill skill_id) {
		if (sc == nullptr || !sc->hasSCE(SC_THUNDERING_ROD_MAX)) {
			return skill_id;
		}

		switch (skill_id) {
			case KR_THUNDERING_FOCUS:
				return KR_THUNDERING_FOCUS_S;
			case KR_THUNDERING_ORB:
				return KR_THUNDERING_ORB_S;
			case KR_THUNDERING_CALL:
				return KR_THUNDERING_CALL_S;
			case AT_ROARING_PIERCER:
				return AT_ROARING_PIERCER_S;
			case AT_ROARING_CHARGE:
				return AT_ROARING_CHARGE_S;
			default:
				return skill_id;
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

	void SkillFactoryDruid::try_gain_madness(block_list *src) {
		status_change *sc = status_get_sc(src);
		status_change_entry *pulse = sc != nullptr ? sc->getSCE(SC_PULSE_OF_MADNESS) : nullptr;

		if (pulse == nullptr) {
			return;
		}

		int32 chance = 20 + 10 * (pulse->val1 - 1);
		if (!rnd_chance(chance, 100)) {
			return;
		}

		t_tick duration = skill_get_time2(AT_PULSE_OF_MADNESS, pulse->val1);

		if (sc->hasSCE(SC_INSANE3)) {
			sc_start(src, src, SC_INSANE3, 100, 1, duration);
			return;
		}

		if (sc->hasSCE(SC_INSANE2)) {
			status_change_end(src, SC_INSANE2);
			sc_start(src, src, SC_INSANE3, 100, 1, duration);
			return;
		}

		if (sc->hasSCE(SC_INSANE)) {
			status_change_end(src, SC_INSANE);
			sc_start(src, src, SC_INSANE2, 100, 1, duration);
			return;
		}

		sc_start(src, src, SC_INSANE, 100, 1, duration);
	}

	void add_thundering_charge(block_list *src, int32 count) {
		status_change *sc = status_get_sc(src);
		if (sc == nullptr || sc->hasSCE(SC_THUNDERING_ROD_MAX) || count <= 0) {
			return;
		}

		status_change_entry *charge = sc->getSCE(SC_THUNDERING_ROD);
		int32 current = charge ? charge->val3 : 0;
		int32 next = current + count;

		if (next > 5) {
			status_change_end(src, SC_THUNDERING_ROD);
			status_change_start(src, src, SC_THUNDERING_ROD_MAX, 10000, 1, 0, 0, 0, kThunderingChargeDuration, SCSTART_NOAVOID);
			return;
		}
		status_change_start(src, src, SC_THUNDERING_ROD, 10000, 0, 0, next, 0, kThunderingChargeDuration, SCSTART_NOAVOID);
	}

	void SkillFactoryDruid::try_gain_thundering_charge(block_list *src, const status_change *sc, e_skill skill_id, int32 gain) {
		if (!is_thundering_charge_skill(skill_id)) {
			return;
		}

		if (sc != nullptr && sc->hasSCE(SC_THUNDERING_ROD_MAX)) {
			status_change_end(src, SC_THUNDERING_ROD_MAX);
		} else {
			add_thundering_charge(src, gain);
		}
	}

	void add_growth_stacks(block_list *src, t_tick tick, int32 amount) {
		map_session_data *sd = BL_CAST(BL_PC, src);
		status_change *sc = status_get_sc(src);

		if (sd == nullptr || sc == nullptr || amount <= 0) {
			return;
		}

		int32 bud_lv = pc_checkskill(sd, KR_EARTH_BUD);
		if (bud_lv <= 0) {
			return;
		}

		status_change_entry *grow = sc->getSCE(SC_GROUND_GROW);
		int32 current = grow ? grow->val3 : 0;

		if (current >= 12) {
			status_change_end(src, SC_GROUND_GROW);
			skill_castend_nodamage_id(src, src, KR_GROUND_BLOOM, bud_lv, tick, 0);

			int64 heal = static_cast<int64>(status_get_max_hp(src)) * (bud_lv * 3) / 100;
			if (heal > 0) {
				status_heal(src, heal, 0, 0);
			}
			return;
		}

		int32 next = current + amount;
		if (next > 12) {
			next = 12;
		}

		status_change_start(src, src, SC_GROUND_GROW, 10000, 0, 0, next, 0, kGrowthDuration, SCSTART_NOAVOID);
	}

	void SkillFactoryDruid::try_gain_growth_stacks(block_list *src, t_tick tick, e_skill skill_id) {
		int32 amount = 0;
		switch (skill_id) {
			case KR_EARTH_DRILL:
			case KR_EARTH_STAMP:
				amount = 1;
				break;
			case AT_TERRA_HARVEST:
			case AT_TERRA_WAVE:
				amount = 2;
				break;
			case AT_SOLID_STOMP:
				amount = 4;
				break;
			default:
				return;
		}

		add_growth_stacks(src, tick, amount);
	}

	bool get_glacier_center(const status_change *sc, int32 &x, int32 &y, int32 &map_id) {
		if (sc == nullptr) {
			return false;
		}

		const status_change_entry *sce = sc->getSCE(SC_GLACIER_SHEILD);
		if (sce == nullptr) {
			return false;
		}

		x = sce->val2;
		y = sce->val3;
		map_id = sce->val4;
		return true;
	}

	bool SkillFactoryDruid::get_glacier_center_on_map(const block_list *src, const status_change *sc, int32 &gx, int32 &gy) {
		int32 map_id = 0;

		if (!get_glacier_center(sc, gx, gy, map_id)) {
			return false;
		}

		return src->m == map_id;
	}

	int32 apply_splash_outer_sub(block_list *bl, va_list ap) {
		block_list *src = va_arg(ap, block_list *);
		uint16 skill_id = static_cast<uint16>(va_arg(ap, int));
		uint16 skill_lv = static_cast<uint16>(va_arg(ap, int));
		t_tick tick = va_arg(ap, t_tick);
		int32 flag = va_arg(ap, int32);
		int32 x = va_arg(ap, int32);
		int32 y = va_arg(ap, int32);
		int32 min_distance = va_arg(ap, int32);
		int32 exclude_id = va_arg(ap, int32);

		if (bl->id == exclude_id) {
			return 0;
		}

		if (distance_xy(x, y, bl->x, bl->y) <= min_distance) {
			return 0;
		}

		if (battle_check_target(src, bl, BCT_ENEMY) <= 0) {
			return 0;
		}

		return static_cast<int32>(skill_attack(skill_get_type(static_cast<e_skill>(skill_id)), src, src, bl, skill_id, skill_lv, tick, flag | SD_ANIMATION));
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
			return std::make_unique<SkillTruthOfEarth>();
		case DR_TRUTH_OF_ICE:
			return std::make_unique<SkillTruthOfIce>();
		case DR_TRUTH_OF_WIND:
			return std::make_unique<SkillTruthOfWind>();
		case DR_WIND_BOMB:
			return std::make_unique<SkillWindBomb>();
		case DR_WEREWOLF:
			return std::make_unique<SkillTransformationBeast>();
		case DR_WERERAPTOR:
			return std::make_unique<SkillTransformationRaptor>();

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
			return std::make_unique<SkillWindVeil>();

		case AT_AERO_SYNC:
			return std::make_unique<SkillAeroSync>();
		case AT_ALPHA_CLAW:
			return std::make_unique<SkillAlphaClaw>();
		case AT_ALPHA_PHASE:
			return std::make_unique<SkillAlphaPhase>();
		case AT_APEX_PHASE:
			return std::make_unique<SkillApexPhase>();
		case AT_CHILLING_BLAST:
			return std::make_unique<SkillChillingBlast>();
		case AT_FERAL_CLAW:
			return std::make_unique<SkillFeralClaw>();
		case AT_FLIP_FLAP:
			return std::make_unique<SkillFlipFlap>();
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
			return std::make_unique<SkillNatureHarmony>();
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
			return std::make_unique<SkillZephyrLink>();

		default:
			return nullptr;
	}
}
