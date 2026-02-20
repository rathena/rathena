// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "trample.hpp"

#include <common/nullpo.hpp>

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillTrample::SkillTrample() : SkillImpl(LG_TRAMPLE) {
}

/**
 * For Royal Guard's LG_TRAMPLE
 */
static int32 skill_destroy_trap(block_list *bl, va_list ap)
{
	skill_unit *su = (skill_unit *)bl;

	nullpo_ret(su);

	std::shared_ptr<s_skill_unit_group> sg;
	t_tick tick = va_arg(ap, t_tick);

	if (su->alive && (sg = su->group) && skill_get_inf2(sg->skill_id, INF2_ISTRAP)) {
		switch( sg->unit_id ) {
			case UNT_CLAYMORETRAP:
			case UNT_FIRINGTRAP:
			case UNT_ICEBOUNDTRAP:
				map_foreachinrange(skill_trap_splash,su, skill_get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag|BL_SKILL|~BCT_SELF, su,tick);
				break;
			case UNT_LANDMINE:
			case UNT_BLASTMINE:
			case UNT_SHOCKWAVE:
			case UNT_SANDMAN:
			case UNT_FLASHER:
			case UNT_FREEZINGTRAP:
			case UNT_CLUSTERBOMB:
				if (battle_config.skill_wall_check && !skill_get_nk(sg->skill_id, NK_NODAMAGE))
					map_foreachinshootrange(skill_trap_splash,su, skill_get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag, su,tick);
				else
					map_foreachinallrange(skill_trap_splash,su, skill_get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag, su,tick);
				break;
		}
		// Traps aren't recovered.
		skill_delunit(su);
	}

	return 0;
}

void SkillTrample::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_damage(*src, *target, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE);

	if (rnd() % 100 < (25 + 25 * skill_lv)) {
		map_foreachinallrange(skill_destroy_trap, target, skill_get_splash(getSkillId(), skill_lv), BL_SKILL, tick);
	}

	status_change_end(target, SC_SV_ROOTTWIST);
}
