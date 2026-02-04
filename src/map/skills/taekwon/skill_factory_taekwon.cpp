// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_taekwon.hpp"

#include "../status_skill_impl.hpp"
#include "../weapon_skill_impl.hpp"

#include "circleofdirectionsandelementals.hpp"
#include "counter.hpp"
#include "downkick.hpp"
#include "eska.hpp"
#include "eske.hpp"
#include "esma.hpp"
#include "estin.hpp"
#include "estun.hpp"
#include "eswoo.hpp"
#include "exorcismofmalicioussoul.hpp"
#include "feelingthesunmoonandstars.hpp"
#include "hatredofthesunmoonandstars.hpp"
#include "highjump.hpp"
#include "jumpkick.hpp"
#include "kaahi.hpp"
#include "kaite.hpp"
#include "kaizel.hpp"
#include "kaupe.hpp"
#include "mission.hpp"
#include "run.hpp"
#include "sevenwind.hpp"
#include "soulgathering.hpp"
#include "soulofheavenandearth.hpp"
#include "spiritofrebirth.hpp"
#include "spiritofthealchemist.hpp"
#include "spiritoftheartist.hpp"
#include "spiritoftheassasin.hpp"
#include "spiritoftheblacksmith.hpp"
#include "spiritofthecrusader.hpp"
#include "spiritofthehunter.hpp"
#include "spiritoftheknight.hpp"
#include "spiritofthemonk.hpp"
#include "spiritofthepriest.hpp"
#include "spiritoftherogue.hpp"
#include "spiritofthesage.hpp"
#include "spiritofthesoullinker.hpp"
#include "spiritofthestargladiator.hpp"
#include "spiritofthesupernovice.hpp"
#include "spiritofthewizard.hpp"
#include "stormkick.hpp"
#include "talismanofblacktortoise.hpp"
#include "talismanofbluedragon.hpp"
#include "talismanoffiveelements.hpp"
#include "talismanoffourbearinggod.hpp"
#include "talismanofmagician.hpp"
#include "talismanofprotection.hpp"
#include "talismanofredphoenix.hpp"
#include "talismanofsoulstealing.hpp"
#include "talismanofwarrior.hpp"
#include "talismanofwhitetiger.hpp"
#include "totemoftutelary.hpp"
#include "turnkick.hpp"
#include "warmthofthemoon.hpp"
#include "warmthofthestars.hpp"
#include "warmthofthesun.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryTaekwon::create(const e_skill skill_id) const {
	switch (skill_id) {
		case SG_FEEL:
			return std::make_unique<SkillFeelingtheSunMoonandStars>();
		case SG_FUSION:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case SG_HATE:
			return std::make_unique<SkillHatredoftheSunMoonandStars>();
		case SG_MOON_COMFORT:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SG_MOON_WARM:
			return std::make_unique<SkillWarmthoftheMoon>();
		case SG_STAR_COMFORT:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SG_STAR_WARM:
			return std::make_unique<SkillWarmthoftheStars>();
		case SG_SUN_COMFORT:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SG_SUN_WARM:
			return std::make_unique<SkillWarmthoftheSun>();
		case SJ_BOOKOFDIMENSION:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SJ_FALLINGSTAR:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SJ_LIGHTOFMOON:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SJ_LIGHTOFSTAR:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SJ_LIGHTOFSUN:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SJ_LUNARSTANCE:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case SJ_STARSTANCE:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case SJ_SUNSTANCE:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case SJ_UNIVERSESTANCE:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case SKE_DAWN_BREAK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case SKE_ENCHANTING_SKY:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SKE_MIDNIGHT_KICK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case SKE_RISING_MOON:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case SL_ALCHEMIST:
			return std::make_unique<SkillSpiritoftheAlchemist>();
		case SL_ASSASIN:
			return std::make_unique<SkillSpiritoftheAssasin>();
		case SL_BARDDANCER:
			return std::make_unique<SkillSpiritoftheArtist>();
		case SL_BLACKSMITH:
			return std::make_unique<SkillSpiritoftheBlacksmith>();
		case SL_CRUSADER:
			return std::make_unique<SkillSpiritoftheCrusader>();
		case SL_HIGH:
			return std::make_unique<SkillSpiritofRebirth>();
		case SL_HUNTER:
			return std::make_unique<SkillSpiritoftheHunter>();
		case SL_KAAHI:
			return std::make_unique<SkillKaahi>();
		case SL_KAITE:
			return std::make_unique<SkillKaite>();
		case SL_KAIZEL:
			return std::make_unique<SkillKaizel>();
		case SL_KAUPE:
			return std::make_unique<SkillKaupe>();
		case SL_KNIGHT:
			return std::make_unique<SkillSpiritoftheKnight>();
		case SL_MONK:
			return std::make_unique<SkillSpiritoftheMonk>();
		case SL_PRIEST:
			return std::make_unique<SkillSpiritofthePriest>();
		case SL_ROGUE:
			return std::make_unique<SkillSpiritoftheRogue>();
		case SL_SAGE:
			return std::make_unique<SkillSpiritoftheSage>();
		case SL_SKA:
			return std::make_unique<SkillEska>();
		case SL_SKE:
			return std::make_unique<SkillEske>();
		case SL_SMA:
			return std::make_unique<SkillEsma>();
		case SL_SOULLINKER:
			return std::make_unique<SkillSpiritoftheSoulLinker>();
		case SL_STAR:
			return std::make_unique<SkillSpiritoftheStarGladiator>();
		case SL_STIN:
			return std::make_unique<SkillEstin>();
		case SL_STUN:
			return std::make_unique<SkillEstun>();
		case SL_SUPERNOVICE:
			return std::make_unique<SkillSpiritoftheSupernovice>();
		case SL_SWOO:
			return std::make_unique<SkillEswoo>();
		case SL_WIZARD:
			return std::make_unique<SkillSpiritoftheWizard>();
		case SOA_CIRCLE_OF_DIRECTIONS_AND_ELEMENTALS:
			return std::make_unique<SkillCircleOfDirectionsAndElementals>();
		case SOA_EXORCISM_OF_MALICIOUS_SOUL:
			return std::make_unique<SkillExorcismOfMaliciousSoul>();
		case SOA_SOUL_GATHERING:
			return std::make_unique<SkillSoulGathering>();
		case SOA_SOUL_OF_HEAVEN_AND_EARTH:
			return std::make_unique<SkillSoulOfHeavenAndEarth>();
		case SOA_TALISMAN_OF_BLACK_TORTOISE:
			return std::make_unique<SkillTalismanOfBlackTortoise>();
		case SOA_TALISMAN_OF_BLUE_DRAGON:
			return std::make_unique<SkillTalismanOfBlueDragon>();
		case SOA_TALISMAN_OF_FIVE_ELEMENTS:
			return std::make_unique<SkillTalismanOfFiveElements>();
		case SOA_TALISMAN_OF_FOUR_BEARING_GOD:
			return std::make_unique<SkillTalismanOfFourBearingGod>();
		case SOA_TALISMAN_OF_MAGICIAN:
			return std::make_unique<SkillTalismanOfMagician>();
		case SOA_TALISMAN_OF_PROTECTION:
			return std::make_unique<SkillTalismanOfProtection>();
		case SOA_TALISMAN_OF_RED_PHOENIX:
			return std::make_unique<SkillTalismanOfRedPhoenix>();
		case SOA_TALISMAN_OF_SOUL_STEALING:
			return std::make_unique<SkillTalismanOfSoulStealing>();
		case SOA_TALISMAN_OF_WARRIOR:
			return std::make_unique<SkillTalismanOfWarrior>();
		case SOA_TALISMAN_OF_WHITE_TIGER:
			return std::make_unique<SkillTalismanOfWhiteTiger>();
		case SOA_TOTEM_OF_TUTELARY:
			return std::make_unique<SkillTotemOfTutelary>();
		case SP_SOULREAPER:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case TK_COUNTER:
			return std::make_unique<SkillCounter>();
		case TK_DODGE:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case TK_DOWNKICK:
			return std::make_unique<SkillDownKick>();
		case TK_HIGHJUMP:
			return std::make_unique<SkillHighJump>();
		case TK_JUMPKICK:
			return std::make_unique<SkillJumpKick>();
		case TK_MISSION:
			return std::make_unique<SkillMission>();
		case TK_READYCOUNTER:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case TK_READYDOWN:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case TK_READYSTORM:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case TK_READYTURN:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case TK_RUN:
			return std::make_unique<SkillRun>();
		case TK_SEVENWIND:
			return std::make_unique<SkillSevenWind>();
		case TK_STORMKICK:
			return std::make_unique<SkillStormKick>();
		case TK_TURNKICK:
			return std::make_unique<SkillTurnKick>();

		default:
			return nullptr;
	}
}
