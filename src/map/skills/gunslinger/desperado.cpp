// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "desperado.hpp"

#include "map/status.hpp"

SkillDesperado::SkillDesperado() : SkillImpl(GS_DESPERADO) {
}

void SkillDesperado::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	const status_change *sc = status_get_sc(src);

	base_skillratio += 50 * (skill_lv - 1);
	if (sc && sc->getSCE(SC_FALLEN_ANGEL))
		base_skillratio *= 2;
}

void SkillDesperado::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	flag |= 1;
	skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);
}
