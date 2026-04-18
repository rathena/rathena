// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "bssacramenti.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillBenedictioSanctissimiSacramenti::SkillBenedictioSanctissimiSacramenti() : SkillImpl(PR_BENEDICTIO) {
}

void SkillBenedictioSanctissimiSacramenti::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* tstatus = status_get_status_data(*target);

	if (!battle_check_undead(tstatus->race, tstatus->def_ele) && tstatus->race != RC_DEMON)
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start(src, target, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
}

void SkillBenedictioSanctissimiSacramenti::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* tstatus = status_get_status_data(*target);

	//Should attack undead and demons. [Skotlex]
	if (battle_check_undead(tstatus->race, tstatus->def_ele) || tstatus->race == RC_DEMON)
		skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillBenedictioSanctissimiSacramenti::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_area_temp[1] = src->id;
	int32 i = skill_get_splash(getSkillId(), skill_lv);
	map_foreachinallarea(skill_area_sub,
		src->m, x-i, y-i, x+i, y+i, BL_PC,
		src, getSkillId(), skill_lv, tick, flag|BCT_ALL|1,
		skill_castend_nodamage_id);
	map_foreachinallarea(skill_area_sub,
		src->m, x-i, y-i, x+i, y+i, BL_CHAR,
		src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|1,
		skill_castend_damage_id);
}
