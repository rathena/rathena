// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "eswoo.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillEswoo::SkillEswoo() : StatusSkillImpl(SL_SWOO) {
}

void SkillEswoo::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	status_change *tsc = status_get_sc(target);
	status_change_entry *tsce = (tsc && type != SC_NONE)?tsc->getSCE(type):nullptr;
	map_session_data* sd = BL_CAST( BL_PC, src );

	if (tsce) {
		if(sd)
			clif_skill_fail( *sd, getSkillId() );
		status_change_start(src,src,SC_STUN,10000,skill_lv,0,0,0,10000,SCSTART_NORATEDEF);
		status_change_end(target, SC_SWOO);
		return;
	}
	if (sd && !battle_config.allow_es_magic_pc && target->type != BL_MOB) {
		clif_skill_fail( *sd, getSkillId() );
		status_change_start(src,src,SC_STUN,10000,skill_lv,0,0,0,500,SCSTART_NOTICKDEF|SCSTART_NORATEDEF);
		return;
	}

	StatusSkillImpl::castendNoDamageId(src, target, skill_lv, tick, flag);
}
