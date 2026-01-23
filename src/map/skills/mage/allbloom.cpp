// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "allbloom.hpp"

#include "map/status.hpp"

SkillAllBloom::SkillAllBloom() : SkillImpl(AG_ALL_BLOOM) {
}

void SkillAllBloom::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());

	sc_start(src, target, type, 100, skill_lv, skill_get_time2(getSkillId(), skill_lv));
}

void SkillAllBloom::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);

	int32 area = skill_get_splash(getSkillId(), skill_lv);
	int32 unit_time = skill_get_time(getSkillId(), skill_lv);
	int32 unit_interval = skill_get_unit_interval(getSkillId());
	uint16 tmpx = 0, tmpy = 0, climax_lv = 0;
	int32 i = 0;

	// Grab Climax's effect level if active.
	// This affects the behavior of certain skills in certain ways.
	if (sc && sc->getSCE(SC_CLIMAX))
		climax_lv = sc->getSCE(SC_CLIMAX)->val1;

	if (climax_lv == 1) { // Rose buds spawn at doutargete the speed.
		unit_time /= 2;
		unit_interval /= 2;
	}

	// Displays the flower garden.
	skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);

	if (climax_lv == 4) { // Deals no damage and instead inflicts a status on the enemys in range.
		i = skill_get_splash(getSkillId(), skill_lv);
		map_foreachinallarea(skill_area_sub, src->m, x - i, y - i, x + i, y + i, BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | 1, skill_castend_nodamage_id);
	} else for (i = 1; i <= unit_time / unit_interval; i++) { // Spawn the rose buds on random spots at seperate intervals
		tmpx = x - area + rnd() % (area * 2 + 1);
		tmpy = y - area + rnd() % (area * 2 + 1);
		skill_unitsetting(src, AG_ALL_BLOOM_ATK, skill_lv, tmpx, tmpy, flag + i * unit_interval);

		if (getSkillId() == AG_ALL_BLOOM && climax_lv == 2) { // Spwan a 2nd rose bud along with the 1st one.
			tmpx = x - area + rnd() % (area * 2 + 1);
			tmpy = y - area + rnd() % (area * 2 + 1);
			skill_unitsetting(src, AG_ALL_BLOOM_ATK, skill_lv, tmpx, tmpy, flag + i * unit_interval);
		}
	}

	// One final attack the size of the flower garden is dealt after
	// all rose buds explode if Climax level 5 is active.
	if (climax_lv == 5)
		skill_unitsetting(src, AG_ALL_BLOOM_ATK2, skill_lv, x, y, flag + i * unit_interval);
}
