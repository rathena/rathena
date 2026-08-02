// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "wildshot.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillWildShot::SkillWildShot() : SkillImpl(NW_WILD_SHOT) {
}

void SkillWildShot::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (flag & 1) {
		skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
	} else {
		int32 splash = skill_get_splash(getSkillId(), skill_lv);

		if (sd != nullptr && sd->weapontype1 == W_RIFLE)
			splash += 1;
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv, 1);
		map_foreachinrange(skill_area_sub, target, splash, BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);

	}
}

void SkillWildShot::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 870 + 180 * skill_lv;
	if (sd != nullptr && sc != nullptr && sc->hasSCE(SC_HIDDEN_CARD)) {
		if (sd->weapontype1 == W_REVOLVER)
			skillratio += 60 * skill_lv;
		else if (sd->weapontype1 == W_RIFLE)
			skillratio += 100 * skill_lv;
	}
	skillratio += 5 * sstatus->con; //!TODO: check con ratio
	RE_LVL_DMOD(100);
}
