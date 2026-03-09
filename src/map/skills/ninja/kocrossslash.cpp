// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "kocrossslash.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillKoCrossSlash::SkillKoCrossSlash() : WeaponSkillImpl(KO_JYUMONJIKIRI) {
}

void SkillKoCrossSlash::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src,target,SC_JYUMONJIKIRI,100,skill_lv,skill_get_time(getSkillId(),skill_lv));
}

void SkillKoCrossSlash::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_change *sc = status_get_sc(src);
	const status_change *tsc = status_get_sc(target);

	skillratio += -100 + 200 * skill_lv;
	RE_LVL_DMOD(120);
	if(tsc && tsc->getSCE(SC_JYUMONJIKIRI))
		skillratio += skill_lv * status_get_lv(src);
	if (sc && sc->getSCE(SC_KAGEMUSYA))
		skillratio += skillratio * sc->getSCE(SC_KAGEMUSYA)->val2 / 100;
}

void SkillKoCrossSlash::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int16 x, y;
	int16 dir = map_calc_dir(src,target->x,target->y);

	if (dir > 0 && dir < 4)
		x = 2;
	else if (dir > 4)
		x = -2;
	else
		x = 0;
	if (dir > 2 && dir < 6)
		y = 2;
	else if (dir == 7 || dir < 2)
		y = -2;
	else
		y = 0;
	if (unit_movepos(src,target->x + x,target->y + y,1,1)) {
		clif_blown(src);
		WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
	}
}
