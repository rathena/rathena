// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "desperado.hpp"

#include "map/status.hpp"

SkillDesperado::SkillDesperado() : WeaponSkillImpl(GS_DESPERADO) {
}

void SkillDesperado::calculateSkillRatio(Damage *wd, block_list *src, block_list *target, uint16 skill_lv, int32 &base_skillratio) const {
	status_change *sc = status_get_sc(src);

	base_skillratio += 50 * (skill_lv - 1);
	if (sc && sc->getSCE(SC_FALLEN_ANGEL))
		base_skillratio *= 2;
}
