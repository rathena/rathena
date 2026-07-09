// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "chargeattack.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/path.hpp"
#include "map/unit.hpp"

SkillChargeAttack::SkillChargeAttack() : SkillImpl(KN_CHARGEATK) {
}

void SkillChargeAttack::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	bool path = path_search_long(nullptr, src->m, src->x, src->y, target->x, target->y,CELL_CHKWALL);
#ifdef RENEWAL
	int32 dist = skill_get_blewcount(getSkillId(), skill_lv);
#else
	// Charge attack in pre-renewal calculates the distance mathetically
	int32 dist = static_cast<int32>(distance_math_bl(src, target));
#endif
	uint8 dir = map_calc_dir(target, src->x, src->y);

	// teleport to target (if not on WoE grounds)
	if (skill_check_unit_movepos(5, src, target->x + dirx[dir], target->y + diry[dir], 0, true))
		clif_blown(src);

	// cause damage and knockback if the path to target was a straight one
	if (path) {
		if(skill_attack(BF_WEAPON, src, src, target, getSkillId(), skill_lv, tick, dist)) {
#ifdef RENEWAL
			if (map_getmapdata(src->m)->getMapFlag(MF_PVP))
				dist += 2; // Knockback is 4 on PvP maps
#endif
			skill_blown(src, target, dist, dir, BLOWN_NONE);
		}
	}
}

void SkillChargeAttack::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	base_skillratio += 600;
#else
	// +100% every 3 cells of distance but hard-limited to 500%
	int32 k = (wd->miscflag - 1) / 3;
	if (k < 0)
		k = 0;
	else if (k > 4)
		k = 4;
	base_skillratio += 100 * k;
#endif
}
