// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "autoshadowspell.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillAutoShadowSpell::SkillAutoShadowSpell() : SkillImpl(SC_AUTOSHADOWSPELL) {
}

void SkillAutoShadowSpell::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if( sd ) {
		if( (sd->reproduceskill_idx > 0 && sd->status.skill[sd->reproduceskill_idx].id) ||
			(sd->cloneskill_idx > 0 && sd->status.skill[sd->cloneskill_idx].id) )
		{
			sc_start(src,src,SC_STOP,100,skill_lv,INFINITE_TICK);// The skill_lv is stored in val1 used in skill_select_menu to determine the used skill lvl [Xazax]
			clif_autoshadowspell_list( *sd );
			clif_skill_nodamage(src,*target,getSkillId(),1);
		}
		else
			clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL_IMITATION_SKILL_NONE );
	}
}
