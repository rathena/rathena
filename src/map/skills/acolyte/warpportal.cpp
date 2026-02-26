// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "warpportal.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillWarpPortal::SkillWarpPortal() : SkillImpl(AL_WARP) {
}

void SkillWarpPortal::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	status_change* sc = status_get_sc(src);

	if(sd != nullptr) {
		std::vector<std::string> maps( MAX_MEMOPOINTS + 1 );

		maps.push_back( sd->status.save_point.map );

		if( skill_lv >= 2 ){
			maps.push_back( sd->status.memo_point[0].map );

			if( skill_lv >= 3 ){
				maps.push_back( sd->status.memo_point[1].map );

				if( skill_lv >= 4 ){
					maps.push_back( sd->status.memo_point[2].map );
				}
			}
		}

		clif_skill_warppoint( *sd, getSkillId(), skill_lv, maps );
	}
	if( sc && sc->getSCE(SC_CURSEDCIRCLE_ATKER) ) //Should only remove after the skill has been casted.
		status_change_end(src,SC_CURSEDCIRCLE_ATKER);
	// not to consume item.
	flag |= SKILL_NOCONSUME_REQ;
}
