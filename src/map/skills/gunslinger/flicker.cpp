// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "flicker.hpp"

#include <common/nullpo.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"

static int32 skill_bind_trap(block_list* bl, va_list ap);

SkillFlicker::SkillFlicker() : SkillImpl(RL_FLICKER) {
}

void SkillFlicker::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd) {
		sd->flicker = true;
		skill_area_temp[1] = 0;
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		// Detonate RL_B_TRAP
		if (pc_checkskill(sd, RL_B_TRAP)) {
			map_foreachinallrange(skill_bind_trap, src, AREA_SIZE, BL_SKILL, src);
		}
		// Detonate RL_H_MINE
		if (int32 mine_lv = pc_checkskill(sd, RL_H_MINE)) {
			map_foreachinallrange(skill_area_sub, src, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, RL_H_MINE, mine_lv, tick, flag | BCT_ENEMY | SD_SPLASH, skill_castend_damage_id);
		}
		sd->flicker = false;
	}
}

/**
 * Rebellion's Bind Trap explosion
 * @author [Cydh]
 */
static int32 skill_bind_trap(block_list *bl, va_list ap) {
	skill_unit *su = nullptr;
	block_list *src = nullptr;

	nullpo_ret(bl);

	src = va_arg(ap,block_list *);

	if (bl->type != BL_SKILL || !(su = (skill_unit *)bl) || !(su->group))
		return 0;
	if (su->group->unit_id != UNT_B_TRAP || su->group->src_id != src->id)
		return 0;

	map_foreachinallrange(skill_trap_splash, bl, su->range, BL_CHAR, bl,su->group->tick);
	clif_changetraplook(bl, UNT_USED_TRAPS);
	su->group->unit_id = UNT_USED_TRAPS;
	su->group->limit = DIFF_TICK(gettick(), su->group->tick) + 500;
	return 1;
}
