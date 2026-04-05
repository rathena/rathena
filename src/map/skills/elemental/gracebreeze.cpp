// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "gracebreeze.hpp"

#include "map/clif.hpp"
#include "map/elemental.hpp"
#include "map/status.hpp"

SkillGraceBreeze::SkillGraceBreeze() : SkillImpl(EM_EL_GRACE_BREEZE) {
}

void SkillGraceBreeze::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);
	sc_type type = skill_get_sc(getSkillId());

	s_elemental_data *ele = BL_CAST(BL_ELEM, src);
	if( ele ) {
		sc_type type2 = (sc_type)(type-1);
		status_change *esc = status_get_sc(ele);

		if( (esc && esc->getSCE(type2)) || (tsc && tsc->getSCE(type)) ) {
			status_change_end(src,type);
			status_change_end(target,type2);
		} else {
			clif_skill_nodamage(src,*src,getSkillId(),skill_lv);
			sc_start(src,src,type2,100,skill_lv,skill_get_time(getSkillId(),skill_lv));
			sc_start(src,target,type,100,skill_lv,skill_get_time(getSkillId(),skill_lv));
		}
	}
}
