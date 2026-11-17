// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "spiritoftheartist.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillSpiritoftheArtist::SkillSpiritoftheArtist() : SkillImpl(SL_BARDDANCER) {
}

void SkillSpiritoftheArtist::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST( BL_PC, src );

	if( sc_start2( src, target, type, 100, skill_lv, getSkillId(), skill_get_time( getSkillId(), skill_lv ) ) ){
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

		sc_start( src, src, SC_SMA, 100, skill_lv, skill_get_time( SL_SMA, skill_lv ) );
	}else{
		if( sd ){
			clif_skill_fail( *sd, getSkillId() );
		}
	}
}
