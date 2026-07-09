// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "eske.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillEske::SkillEske() : StatusSkillImpl(SL_SKE) {
}

void SkillEske::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	if (sd && !battle_config.allow_es_magic_pc && target->type != BL_MOB) {
		clif_skill_fail( *sd, getSkillId() );
		status_change_start(src,src,SC_STUN,10000,skill_lv,0,0,0,500,SCSTART_NOTICKDEF|SCSTART_NORATEDEF);
		return;
	}

	StatusSkillImpl::castendNoDamageId(src, target, skill_lv, tick, flag);

	sc_start(src,src,SC_SMA,100,skill_lv,skill_get_time(SL_SMA,skill_lv));
}
