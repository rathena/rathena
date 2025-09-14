// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_taekwon.hpp"

#include "skill_tk_counter.hpp"
#include "skill_tk_dodge.hpp"
#include "skill_tk_downkick.hpp"
#include "skill_tk_highjump.hpp"
#include "skill_tk_hptime.hpp"
#include "skill_tk_jumpkick.hpp"
#include "skill_tk_mission.hpp"
#include "skill_tk_power.hpp"
#include "skill_tk_readycounter.hpp"
#include "skill_tk_readydown.hpp"
#include "skill_tk_readystorm.hpp"
#include "skill_tk_readyturn.hpp"
#include "skill_tk_run.hpp"
#include "skill_tk_sevenwind.hpp"
#include "skill_tk_sptime.hpp"
#include "skill_tk_stormkick.hpp"
#include "skill_tk_turnkick.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryTaekwon::create(const e_skill skill_id) const {
	switch (skill_id) {
		case TK_COUNTER:
			return std::make_unique<SkillCounter>();
		case TK_DODGE:
			return std::make_unique<SkillDodge>();
		case TK_DOWNKICK:
			return std::make_unique<SkillDownkick>();
		case TK_HIGHJUMP:
			return std::make_unique<SkillHighjump>();
		case TK_HPTIME:
			return std::make_unique<SkillHptime>();
		case TK_JUMPKICK:
			return std::make_unique<SkillJumpkick>();
		case TK_MISSION:
			return std::make_unique<SkillMission>();
		case TK_POWER:
			return std::make_unique<SkillPower>();
		case TK_READYCOUNTER:
			return std::make_unique<SkillReadycounter>();
		case TK_READYDOWN:
			return std::make_unique<SkillReadydown>();
		case TK_READYSTORM:
			return std::make_unique<SkillReadystorm>();
		case TK_READYTURN:
			return std::make_unique<SkillReadyturn>();
		case TK_RUN:
			return std::make_unique<SkillRun>();
		case TK_SEVENWIND:
			return std::make_unique<SkillSevenwind>();
		case TK_SPTIME:
			return std::make_unique<SkillSptime>();
		case TK_STORMKICK:
			return std::make_unique<SkillStormkick>();
		case TK_TURNKICK:
			return std::make_unique<SkillTurnkick>();

		default:
			return nullptr;
	}
}
