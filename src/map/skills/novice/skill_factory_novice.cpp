// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_novice.hpp"

#include "basic.hpp"
#include "firstaid.hpp"
#include "trickdead.hpp"
#include "callbaby.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryNovice::create(const e_skill skill_id) const {
	switch( skill_id ){
		case NV_BASIC:
			return std::make_unique<SkillBasic>(skill_id);
		case NV_FIRSTAID:
			return std::make_unique<SkillFirstAid>(skill_id);
		case NV_TRICKDEAD:
			return std::make_unique<SkillTrickDead>(skill_id);
		case WE_CALLBABY:
			return std::make_unique<SkillCallBaby>(skill_id);

		default:
			return nullptr;
	}
}
