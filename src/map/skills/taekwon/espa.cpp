// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "espa.hpp"

#include <config/core.hpp>

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillEspa::SkillEspa() : SkillImpl(SP_SPA) {
}

void SkillEspa::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	skillratio += 400 + 250 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillEspa::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	if (sd && !battle_config.allow_es_magic_pc && target->type != BL_MOB) {
		status_change_start(src,src,SC_STUN,10000,skill_lv,0,0,0,500,SCSTART_NOTICKDEF|SCSTART_NORATEDEF);
		clif_skill_fail( *sd, getSkillId() );
		return;
	}
	skill_attack(BF_MAGIC,src,src,target,getSkillId(),skill_lv,tick,flag);
}
