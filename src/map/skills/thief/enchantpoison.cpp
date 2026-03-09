// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "enchantpoison.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillEnchantPoison::SkillEnchantPoison() : SkillImpl(AS_ENCHANTPOISON) {
}

void SkillEnchantPoison::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST( BL_PC, src );

	if( sc_start( src, target, type, 100, skill_lv, skill_get_time( getSkillId(), skill_lv ) ) ){
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}else{
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv, false );

		if( sd != nullptr ){
			clif_skill_fail( *sd, getSkillId() );
		}
	}
}
