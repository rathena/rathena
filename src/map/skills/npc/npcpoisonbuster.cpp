// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "npcpoisonbuster.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillNpcPoisonBuster::SkillNpcPoisonBuster() : SkillImpl(NPC_POISON_BUSTER) {
}

void SkillNpcPoisonBuster::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += -100 + 1500 * skill_lv;
}

void SkillNpcPoisonBuster::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);
	map_session_data* sd = BL_CAST(BL_PC, src);

	if( tsc && tsc->getSCE(SC_POISON) ) {
		skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
		status_change_end(target, SC_POISON);
	}
	else if( sd )
		clif_skill_fail( *sd, getSkillId() );
}
