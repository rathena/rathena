// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_homunculus.hpp"

#include "../skill_impl.hpp"

// Include .cpp files into the TU to optimize compile time
// For reference see unity builds or amalgamated builds
#include "homunculus_absolutezephyr.cpp"
#include "homunculus_avoid.cpp"
#include "homunculus_benedictionofchaos.cpp"
#include "homunculus_bioexplosion.cpp"
#include "homunculus_blastforge.cpp"
#include "homunculus_blazingandfurious.cpp"
#include "homunculus_caprice.cpp"
#include "homunculus_castling.cpp"
#include "homunculus_change.cpp"
#include "homunculus_continualbreakcombo.cpp"
#include "homunculus_defense.cpp"
#include "homunculus_erasercutter.cpp"
#include "homunculus_eternalquickcombo.cpp"
#include "homunculus_glanzenspies.cpp"
#include "homunculus_goldenetone.cpp"
#include "homunculus_graniticarmor.cpp"
#include "homunculus_healingtouch.cpp"
#include "homunculus_heiligepferd.cpp"
#include "homunculus_holypole.cpp"
#include "homunculus_lavaslide.cpp"
#include "homunculus_lightofregene.cpp"
#include "homunculus_magmaflow.cpp"
#include "homunculus_midnightfrenzy.cpp"
#include "homunculus_needleofparalyze.cpp"
#include "homunculus_needlestinger.cpp"
#include "homunculus_overedboost.cpp"
#include "homunculus_painkiller.cpp"
#include "homunculus_poisonmist.cpp"
#include "homunculus_pyroclastic.cpp"
#include "homunculus_silentbreeze.cpp"
#include "homunculus_silverveinrush.cpp"
#include "homunculus_sonicclaw.cpp"
#include "homunculus_steelhorn.cpp"
#include "homunculus_stylechange.cpp"
#include "homunculus_summonlegion.cpp"
#include "homunculus_tempering.cpp"
#include "homunculus_theonefighterrises.cpp"
#include "homunculus_tinderbreaker.cpp"
#include "homunculus_toxinofmandara.cpp"
#include "homunculus_twistercutter.cpp"

std::unique_ptr<const SkillImpl> SkillFactoryHomunculus::create(const e_skill skill_id) const {
	switch (skill_id) {
		case HAMI_BLOODLUST:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case HAMI_CASTLE:
			return std::make_unique<SkillCastling>();
		case HAMI_DEFENCE:
			return std::make_unique<SkillDefense>();
		case HLIF_AVOID:
			return std::make_unique<SkillAvoid>();
		case HLIF_CHANGE:
			return std::make_unique<SkillChange>();
		case HLIF_HEAL:
			return std::make_unique<SkillHealingTouch>();
		case MH_BLAZING_AND_FURIOUS:
			return std::make_unique<SkillBlazingAndFurious>();
		case HVAN_CHAOTIC:
			return std::make_unique<SkillBenedictionOfChaos>();
		case HVAN_EXPLOSION:
			return std::make_unique<SkillBioExplosion>();
		case MH_BLAST_FORGE:
			return std::make_unique<SkillBlastForge>();
		case MH_CBC:
			return std::make_unique<SkillContinualBreakCombo>();
		case HVAN_CAPRICE:
			return std::make_unique<SkillCaprice>();
		case MH_EQC:
			return std::make_unique<SkillEternalQuickCombo>();
		case MH_ERASER_CUTTER:
			return std::make_unique<SkillEraserCutter>();
		case MH_GRANITIC_ARMOR:
			return std::make_unique<SkillGraniticArmor>();
		case MH_GLANZEN_SPIES:
			return std::make_unique<SkillGlanzenSpies>();
		case MH_GOLDENE_TONE:
			return std::make_unique<SkillGoldeneTone>();
		case MH_HEILIGE_PFERD:
			return std::make_unique<SkillHeiligePferd>();
		case MH_HEILIGE_STANGE:
			return std::make_unique<SkillHolyPole>();
		case MH_LAVA_SLIDE:
			return std::make_unique<SkillLavaSlide>();
		case MH_LIGHT_OF_REGENE:
			return std::make_unique<SkillLightOfRegene>();
		case MH_MAGMA_FLOW:
			return std::make_unique<SkillMagmaFlow>();
		case MH_MIDNIGHT_FRENZY:
			return std::make_unique<SkillMidnightFrenzy>();
		case MH_NEEDLE_STINGER:
			return std::make_unique<SkillNeedleStinger>();
		case MH_NEEDLE_OF_PARALYZE:
			return std::make_unique<SkillNeedleOfParalyze>();
		case MH_OVERED_BOOST:
			return std::make_unique<SkillOveredBoost>();
		case MH_PAIN_KILLER:
			return std::make_unique<SkillPainKiller>();
		case MH_POISON_MIST:
			return std::make_unique<SkillPoisonMist>();
		case MH_PYROCLASTIC:
			return std::make_unique<SkillPyroclastic>();
		case MH_SILENT_BREEZE:
			return std::make_unique<SkillSilentBreeze>();
		case MH_SILVERVEIN_RUSH:
			return std::make_unique<SkillSilverveinRush>();
		case MH_SONIC_CRAW:
			return std::make_unique<SkillSonicClaw>();
		case MH_STAHL_HORN:
			return std::make_unique<SkillSteelHorn>();
		case MH_STYLE_CHANGE:
			return std::make_unique<SkillStyleChange>();
		case MH_SUMMON_LEGION:
			return std::make_unique<SkillSummonLegion>();
		case MH_TEMPERING:
			return std::make_unique<SkillTempering>();
		case MH_TINDER_BREAKER:
			return std::make_unique<SkillTinderBreaker>();
		case MH_THE_ONE_FIGHTER_RISES:
			return std::make_unique<SkillTheOneFighterRises>();
		case MH_TWISTER_CUTTER:
			return std::make_unique<SkillTwisterCutter>();
		case MH_ABSOLUTE_ZEPHYR:
			return std::make_unique<SkillAbsoluteZephyr>();
		case MH_TOXIN_OF_MANDARA:
			return std::make_unique<SkillToxinOfMandara>();
		case HVAN_INSTRUCT:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case MH_ANGRIFFS_MODUS:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case MH_GOLDENE_FERSE:
			return std::make_unique<StatusSkillImpl>(skill_id);

		default:
			return nullptr;
	}
}
