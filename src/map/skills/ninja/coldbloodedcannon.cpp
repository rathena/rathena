// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "coldbloodedcannon.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillColdBloodedCannon::SkillColdBloodedCannon() : SkillImpl(SS_REIKETSUHOU) {
}

void SkillColdBloodedCannon::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 450 + 950 * skill_lv;
	skillratio += 40 * pc_checkskill( sd, SS_ANTENPOU ) * skill_lv;
	skillratio += 5 * sstatus->spl;

	if( sc != nullptr && sc->hasSCE( SC_WATER_CHARM_POWER ) ){
		skillratio += 7000;
	}

	RE_LVL_DMOD(100);
}

void SkillColdBloodedCannon::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (flag & 1)
		skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillColdBloodedCannon::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	skill_mirage_cast(*src, nullptr, SS_ANTENPOU, skill_lv, 0, 0, tick, flag | BCT_WOS);
	if (map_getcell(src->m, x, y, CELL_CHKLANDPROTECTOR)) {
		if (sd != nullptr) {
			clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL );
		}
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}
	int32 i = skill_get_splash(getSkillId(), skill_lv);
	map_foreachinallarea(skill_area_sub, src->m, x - i, y - i, x + i, y + i, BL_CHAR,
		src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | 1, skill_castend_damage_id);
}
