// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_gunslinger.hpp"

#include "adjustment.hpp"
#include "bullseye.hpp"
#include "chainaction.hpp"
#include "cracker.hpp"
#include "desperado.hpp"
#include "disarm.hpp"
#include "dust.hpp"
#include "fling.hpp"
#include "fullbuster.hpp"
#include "gatlingfever.hpp"
#include "glittering.hpp"
#include "grounddrift.hpp"
#include "increasing.hpp"
#include "madnesscancel.hpp"
#include "magicalbullet.hpp"
#include "piercingshot.hpp"
#include "rapidshower.hpp"
#include "singleaction.hpp"
#include "snakeeye.hpp"
#include "spreadattack.hpp"
#include "tracking.hpp"
#include "tripleaction.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryGunslinger::create(const e_skill skill_id) const {
	switch (skill_id) {
		case GS_ADJUSTMENT:
			return std::make_unique<SkillAdjustment>();
		case GS_BULLSEYE:
			return std::make_unique<SkillBullseye>();
		case GS_CHAINACTION:
			return std::make_unique<SkillChainaction>();
		case GS_CRACKER:
			return std::make_unique<SkillCracker>();
		case GS_DESPERADO:
			return std::make_unique<SkillDesperado>();
		case GS_DISARM:
			return std::make_unique<SkillDisarm>();
		case GS_DUST:
			return std::make_unique<SkillDust>();
		case GS_FLING:
			return std::make_unique<SkillFling>();
		case GS_FULLBUSTER:
			return std::make_unique<SkillFullbuster>();
		case GS_GATLINGFEVER:
			return std::make_unique<SkillGatlingfever>();
		case GS_GLITTERING:
			return std::make_unique<SkillGlittering>();
		case GS_GROUNDDRIFT:
			return std::make_unique<SkillGrounddrift>();
		case GS_INCREASING:
			return std::make_unique<SkillIncreasing>();
		case GS_MADNESSCANCEL:
			return std::make_unique<SkillMadnesscancel>();
		case GS_MAGICALBULLET:
			return std::make_unique<SkillMagicalbullet>();
		case GS_PIERCINGSHOT:
			return std::make_unique<SkillPiercingshot>();
		case GS_RAPIDSHOWER:
			return std::make_unique<SkillRapidshower>();
		case GS_SINGLEACTION:
			return std::make_unique<SkillSingleaction>();
		case GS_SNAKEEYE:
			return std::make_unique<SkillSnakeeye>();
		case GS_SPREADATTACK:
			return std::make_unique<SkillSpreadattack>();
		case GS_TRACKING:
			return std::make_unique<SkillTracking>();
		case GS_TRIPLEACTION:
			return std::make_unique<SkillTripleaction>();
		default:
			return nullptr;
	}
}