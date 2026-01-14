// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_taekwon.hpp"

#include "../status_skill_impl.hpp"
#include "../weapon_skill_impl.hpp"

#include "counter.hpp"
#include "downkick.hpp"
#include "highjump.hpp"
#include "jumpkick.hpp"
#include "mission.hpp"
#include "run.hpp"
#include "sevenwind.hpp"
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
