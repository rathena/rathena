// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include <config/core.hpp>

#ifdef RENEWAL
#include "../status_skill_impl.hpp"

class SkillMagicalBullet : public StatusSkillImpl {
public:
	SkillMagicalBullet();
};
#else // PRE-RENEWAL
#include "../weapon_skill_impl.hpp"

class SkillMagicalBullet : public WeaponSkillImpl {
public:
	SkillMagicalBullet();
};
#endif
