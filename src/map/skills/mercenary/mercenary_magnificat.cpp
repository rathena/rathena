// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mercenary_magnificat.hpp"

#include "map/clif.hpp"
#include "map/mercenary.hpp"
#include "map/party.hpp"
#include "map/status.hpp"

SkillMercenaryMagnificat::SkillMercenaryMagnificat() : SkillImpl(MER_MAGNIFICAT) {
}

void SkillMercenaryMagnificat::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	s_mercenary_data* mer = BL_CAST(BL_MER, src);
	sc_type type = skill_get_sc(getSkillId());

	if( mer != nullptr )
	{
		clif_skill_nodamage(target, *target, getSkillId(), skill_lv, sc_start(src,target,type,100,skill_lv,skill_get_time(getSkillId(),skill_lv)));
		if( mer->master && mer->master->status.party_id != 0 && !(flag&1) )
			party_foreachsamemap(skill_area_sub, mer->master, skill_get_splash(getSkillId(), skill_lv), src, getSkillId(), skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		else if( mer->master && !(flag&1) )
			clif_skill_nodamage(src, *mer->master, getSkillId(), skill_lv, sc_start(src,target,type,100,skill_lv,skill_get_time(getSkillId(),skill_lv)));
	}
}
