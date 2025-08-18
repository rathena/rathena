// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include <common/utilities.hpp>

#include "../skill.hpp"
#include "skill_impl.hpp"

/**
 * Factory class for creating and managing Skill instances
 */
class SkillFactory{
public:
	// Destructor
	virtual ~SkillFactory() = default;

	virtual std::unique_ptr<const SkillImpl> create( const e_skill skill_id ) const = 0;
};

class SkillFactoryImpl : public SkillFactory, public rathena::util::Singleton<SkillFactoryImpl, SkillFactory> {
public:
	virtual std::unique_ptr<const SkillImpl> create(const e_skill skill_id) const override;
};
