// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_taekwon.hpp"

#include "../status_skill_impl.hpp"
#include "../weapon_skill_impl.hpp"

#include "counter.hpp"
#include "downkick.hpp"
#include "eska.hpp"
#include "eske.hpp"
#include "esma.hpp"
#include "estin.hpp"
#include "estun.hpp"
#include "eswoo.hpp"
#include "highjump.hpp"
#include "jumpkick.hpp"
#include "kaahi.hpp"
#include "kaite.hpp"
#include "kaizel.hpp"
#include "kaupe.hpp"
#include "mission.hpp"
#include "run.hpp"
#include "sevenwind.hpp"
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
#include "turnkick.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryTaekwon::create(const e_skill skill_id) const {
	switch (skill_id) {
		case SG_FUSION:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case SG_MOON_COMFORT:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SG_STAR_COMFORT:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SG_SUN_COMFORT:
			return std::make_unique<StatusSkillImpl>(skill_id);
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
		case SJ_SUNSTANCE:
			return std::make_unique<StatusSkillImpl>(skill_id, true);
		case SJ_STARSTANCE:
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
