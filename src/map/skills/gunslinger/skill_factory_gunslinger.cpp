// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_gunslinger.hpp"

#include <config/core.hpp>

#include "../status_skill_impl.hpp"
#include "../weapon_skill_impl.hpp"

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
#include "piercingshot.hpp"
#include "rapidshower.hpp"
#include "spreadattack.hpp"
#include "tracking.hpp"
#include "tripleaction.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryGunslinger::create(const e_skill skill_id) const {
	switch (skill_id) {
		case GS_ADJUSTMENT:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case GS_BULLSEYE:
			return std::make_unique<SkillBullseye>();
		case GS_CHAINACTION:
			return std::make_unique<WeaponSkillImpl>(skill_id);
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
			return std::make_unique<StatusSkillImpl>(skill_id);
		case GS_MADNESSCANCEL:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case GS_MAGICALBULLET:
#ifdef RENEWAL
			return std::make_unique<StatusSkillImpl>(skill_id);
#else
			return std::make_unique<WeaponSkillImpl>(skill_id);
#endif
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
		case NW_AUTO_FIRING_LAUNCHER:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case NW_BASIC_GRENADE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NW_HASTY_FIRE_IN_THE_HOLE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NW_HIDDEN_CARD:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case NW_MIDNIGHT_FALLEN:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NW_MISSION_BOMBARD:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NW_WILD_FIRE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case RL_AM_BLAST:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case RL_BANISHING_BUSTER:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case RL_E_CHAIN:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case RL_HEAT_BARREL:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case RL_MASS_SPIRAL:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case RL_P_ALTER:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case RL_SLUGSHOT:
			return std::make_unique<WeaponSkillImpl>(skill_id);

		default:
			return nullptr;
	}
}
