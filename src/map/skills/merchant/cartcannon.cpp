// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "cartcannon.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillCartCannon::SkillCartCannon() : SkillImplRecursiveDamageSplash(GN_CARTCANNON) {
}

void SkillCartCannon::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + (250 + 20 * pc_checkskill(sd, GN_REMODELING_CART)) * skill_lv + 2 * sstatus->int_ / (6 - pc_checkskill(sd, GN_REMODELING_CART));
	RE_LVL_DMOD(100);
}

void SkillCartCannon::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd && pc_checkskill(sd, GN_REMODELING_CART))
		hit_rate += pc_checkskill(sd, GN_REMODELING_CART) * 4;
}

void SkillCartCannon::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
