// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "gentletouchcure.hpp"

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/status.hpp"

SkillGentleTouchCure::SkillGentleTouchCure() : SkillImpl(SR_GENTLETOUCH_CURE) {
}

void SkillGentleTouchCure::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	mob_data* dstmd = BL_CAST(BL_MOB, target);
	status_change *tsc = status_get_sc(target);

	uint32 heal;

	if (dstmd && (dstmd->mob_id == MOBID_EMPERIUM || status_get_class_(target) == CLASS_BATTLEFIELD))
		heal = 0;
	else {
		heal = (120 * skill_lv) + (status_get_max_hp(target) * skill_lv / 100);
		status_heal(target, heal, 0, 0);
	}

	if( tsc != nullptr && !tsc->empty() && rnd_chance( ( skill_lv * 5 + ( status_get_dex( src ) + status_get_lv( src ) ) / 4 ) - rnd_value( 1, 10 ), 100 ) ){
		status_change_end(target, SC_STONE);
		status_change_end(target, SC_FREEZE);
		status_change_end(target, SC_STUN);
		status_change_end(target, SC_POISON);
		status_change_end(target, SC_SILENCE);
		status_change_end(target, SC_BLIND);
		status_change_end(target, SC_HALLUCINATION);
	}

	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
}
