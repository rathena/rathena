// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_summonlegion.hpp"

#include "map/map.hpp"
#include "map/mob.hpp"
#include "map/status.hpp"

static int32 summon_legion_count_sub(block_list* bl, va_list ap);

SkillSummonLegion::SkillSummonLegion() : SkillImpl(MH_SUMMON_LEGION) {
}

void SkillSummonLegion::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	int32 summons[5] = { MOBID_S_HORNET, MOBID_S_GIANT_HORNET, MOBID_S_GIANT_HORNET, MOBID_S_LUCIOLA_VESPA, MOBID_S_LUCIOLA_VESPA };
	int32 qty[5] = { 3, 3, 4, 4, 5 };
	int32 count = 0;
	int32 maxcount = qty[skill_lv - 1];

	map_foreachinmap(summon_legion_count_sub, src->m, BL_MOB, src->id, summons[skill_lv - 1], &count);
	if (count >= maxcount) {
		flag |= SKILL_NOCONSUME_REQ;
		return; //max qty already spawned
	}

	for (int32 i_slave = 0; i_slave < qty[skill_lv - 1]; i_slave++) { //easy way
		mob_data *sum_md = mob_once_spawn_sub(src, src->m, src->x, src->y, status_get_name(*src), summons[skill_lv - 1], "", SZ_SMALL, AI_ATTACK);
		if (sum_md) {
			sum_md->master_id = src->id;
			sum_md->special_state.ai = AI_LEGION;
			if (sum_md->deletetimer != INVALID_TIMER) {
				delete_timer(sum_md->deletetimer, mob_timer_delete);
			}
			sum_md->deletetimer = add_timer(gettick() + skill_get_time(getSkillId(), skill_lv), mob_timer_delete, sum_md->id, 0);
			mob_spawn(sum_md); //Now it is ready for spawning.
			sc_start4(sum_md, sum_md, SC_MODECHANGE, 100, 1, 0, MD_CANATTACK | MD_AGGRESSIVE, 0, 60000);
		}
	}
}

static int32 summon_legion_count_sub(block_list *bl, va_list ap) {
	mob_data *md = reinterpret_cast<mob_data *>(bl);
	int32 src_id = va_arg(ap, int32);
	int32 mob_class = va_arg(ap, int32);
	int32 *count = va_arg(ap, int32 *);

	if (md->master_id != src_id || md->special_state.ai != AI_LEGION) {
		return 0;
	}

	if (md->mob_id == mob_class) {
		(*count)++;
	}

	return 1;
}
