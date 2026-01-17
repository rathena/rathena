// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "classchange.hpp"

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillClassChange::SkillClassChange() : SkillImpl(SA_CLASSCHANGE) {
}

void SkillClassChange::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	mob_data* dstmd = BL_CAST(BL_MOB, target);
	status_change *tsc = status_get_sc(target);
	map_session_data* sd = BL_CAST( BL_PC, src );
	int32 i = 0;

	if (dstmd)
	{
		int32 class_;

		if ( sd && status_has_mode(&dstmd->status,MD_STATUSIMMUNE) ) {
			clif_skill_fail( *sd, getSkillId() );
			return;
		}
		class_ = mob_get_random_id(MOBG_CLASSCHANGE, RMF_DB_RATE, 0);
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
		mob_class_change(dstmd,class_);
		if( tsc && status_has_mode(&dstmd->status,MD_STATUSIMMUNE) ) {
			const enum sc_type scs[] = { SC_QUAGMIRE, SC_PROVOKE, SC_ROKISWEIL, SC_GRAVITATION, SC_SUITON, SC_STRIPWEAPON, SC_STRIPSHIELD, SC_STRIPARMOR, SC_STRIPHELM, SC_BLADESTOP };
			for (i = SC_COMMON_MIN; i <= SC_COMMON_MAX; i++)
				if (tsc->getSCE(i)) status_change_end(target, (sc_type)i);
			for (i = 0; i < ARRAYLENGTH(scs); i++)
				if (tsc->getSCE(scs[i])) status_change_end(target, scs[i]);
		}
	}
}
