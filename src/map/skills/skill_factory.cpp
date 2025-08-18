// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory.hpp"

#include <common/showmsg.hpp>

#include "./mercenary/skill_factory_mercenary.hpp"
#include "./swordsman/skill_factory_swordsman.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryImpl::create( const e_skill skill_id ) const {
	if( std::unique_ptr<const SkillImpl> impl = SkillFactoryMercenary::getInstance().create( skill_id ); impl != nullptr ){
		return impl;
	}

	if( std::unique_ptr<const SkillImpl> impl = SkillFactorySwordsman::getInstance().create( skill_id ); impl != nullptr ){
		return impl;
	}

	return nullptr;
}
