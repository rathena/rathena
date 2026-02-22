// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "dominionimpulse.hpp"

#include <common/nullpo.hpp>

#include "map/map.hpp"

static int32 skill_active_reverberation(block_list *bl, va_list ap);


SkillDominionImpulse::SkillDominionImpulse() : SkillImpl(WM_DOMINION_IMPULSE) {
}

void SkillDominionImpulse::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 i = skill_get_splash(getSkillId(), skill_lv);
	map_foreachinallarea(skill_active_reverberation, src->m, x-i, y-i, x+i,y+i,BL_SKILL);
}

static int32 skill_active_reverberation(block_list *bl, va_list ap) {
	skill_unit *su = (skill_unit*)bl;

	nullpo_ret(su);

	if (bl->type != BL_SKILL)
		return 0;

	std::shared_ptr<s_skill_unit_group> sg = su->group;

	if (su->alive && sg && sg->skill_id == NPC_REVERBERATION) {
		map_foreachinallrange(skill_trap_splash, bl, skill_get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag, bl, gettick());
		su->limit = DIFF_TICK(gettick(), sg->tick);
		sg->unit_id = UNT_USED_TRAPS;
	}
	return 1;
}
