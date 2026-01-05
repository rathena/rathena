// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "magicalbullet.hpp"

#ifdef RENEWAL
SkillMagicalBullet::SkillMagicalBullet() : StatusSkillImpl(GS_MAGICALBULLET) {
}
#else
SkillMagicalBullet::SkillMagicalBullet() : WeaponSkillImpl(GS_MAGICALBULLET) {
}
#endif
