// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_taekwon.hpp"

#include "tk_counter.hpp"
#include "tk_dodge.hpp"
#include "tk_downkick.hpp"
#include "tk_highjump.hpp"
#include "tk_jumpkick.hpp"
#include "tk_mission.hpp"
#include "tk_readycounter.hpp"
#include "tk_readydown.hpp"
#include "tk_readystorm.hpp"
#include "tk_readyturn.hpp"
#include "tk_run.hpp"
#include "tk_sevenwind.hpp"
#include "tk_sptime.hpp"
#include "tk_stormkick.hpp"
#include "tk_turnkick.hpp"

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
		case TK_JUMPKICK:
			return std::make_unique<SkillJumpkick>();
		case TK_MISSION:
			return std::make_unique<SkillMission>();
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
