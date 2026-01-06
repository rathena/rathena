// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_gunslinger.hpp"

#include "adjustment.hpp"
#include "bullseye.hpp"
#include "cracker.hpp"
#include "desperado.hpp"
#include "disarm.hpp"
#include "dust.hpp"
#include "fling.hpp"
#include "fullbuster.hpp"
#include "gatlingfever.hpp"
#include "glittering.hpp"
#include "grounddrift.hpp"
#include "magicalbullet.hpp"
#include "piercingshot.hpp"
#include "rapidshower.hpp"
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
			return std::make_unique<WeaponSkillImpl>(GS_CHAINACTION);
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
			return std::make_unique<SkillFullBuster>();
		case GS_GATLINGFEVER:
			return std::make_unique<SkillGatlingfever>();
		case GS_GLITTERING:
			return std::make_unique<SkillGlittering>();
		case GS_GROUNDDRIFT:
			return std::make_unique<SkillGroundDrift>();
		case GS_INCREASING:
			return std::make_unique<StatusSkillImpl>(GS_INCREASING);
		case GS_MADNESSCANCEL:
			return std::make_unique<StatusSkillImpl>(GS_MADNESSCANCEL);
		case GS_MAGICALBULLET:
			return std::make_unique<SkillMagicalBullet>();
		case GS_PIERCINGSHOT:
			return std::make_unique<SkillPiercingShot>();
		case GS_RAPIDSHOWER:
			return std::make_unique<SkillRapidShower>();
		case GS_SPREADATTACK:
			return std::make_unique<SkillSpreadAttack>();
		case GS_TRACKING:
			return std::make_unique<SkillTracking>();
		case GS_TRIPLEACTION:
			return std::make_unique<SkillTripleAction>();
		default:
			return nullptr;
	}
}
