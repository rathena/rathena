// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_novice.hpp"

#include "../status_skill_impl.hpp"

#include "doublebowlingbash.hpp"
#include "groundgravitation.hpp"
#include "hellsdrive.hpp"
#include "helpangel.hpp"
#include "jackfrostnova.hpp"
#include "jupitelthunderstorm.hpp"
#include "megasonicblow.hpp"
#include "meteorstormbuster.hpp"
#include "napalmvulcanstrike.hpp"
#include "overcomingcrisis.hpp"
#include "shieldchainrush.hpp"
#include "spiralpiercemax.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryNovice::create(const e_skill skill_id) const {
	switch( skill_id ){
		case HN_BREAKINGLIMIT:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case HN_DOUBLEBOWLINGBASH:
			return std::make_unique<SkillDoubleBowlingBash>();
		case HN_GROUND_GRAVITATION:
			return std::make_unique<SkillGroundGravitation>();
		case HN_HELLS_DRIVE:
			return std::make_unique<SkillHellsDrive>();
		case HN_JACK_FROST_NOVA:
			return std::make_unique<SkillJackFrostNova>();
		case HN_JUPITEL_THUNDER_STORM:
			return std::make_unique<SkillJupitelThunderstorm>();
		case HN_MEGA_SONIC_BLOW:
			return std::make_unique<SkillMegaSonicBlow>();
		case HN_METEOR_STORM_BUSTER:
			return std::make_unique<SkillMeteorStormBuster>();
		case HN_NAPALM_VULCAN_STRIKE:
			return std::make_unique<SkillNapalmVulcanStrike>();
		case HN_OVERCOMING_CRISIS:
			return std::make_unique<SkillOvercomingCrisis>();
		case HN_RULEBREAK:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case HN_SHIELD_CHAIN_RUSH:
			return std::make_unique<SkillShieldChainRush>();
		case HN_SPIRAL_PIERCE_MAX:
			return std::make_unique<SkillSpiralPierceMax>();
		case NV_HELPANGEL:
			return std::make_unique<SkillHelpAngel>();
		case NV_TRICKDEAD:
			return std::make_unique<StatusSkillImpl>(skill_id, true);

		default:
			return nullptr;
	}
	return nullptr;
}
