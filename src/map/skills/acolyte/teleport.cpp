// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "teleport.hpp"

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/unit.hpp"

SkillTeleport::SkillTeleport() : SkillImpl(AL_TELEPORT) {
}

void SkillTeleport::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if(sd != nullptr)
	{
		if (map_getmapflag(target->m, MF_NOTELEPORT) && skill_lv <= 2) {
			clif_skill_teleportmessage( *sd, NOTIFY_MAPINFO_CANT_TP );
			return;
		}
		if(!battle_config.duel_allow_teleport && sd->duel_group && skill_lv <= 2) { // duel restriction [LuzZza]
			char output[128]; sprintf(output, msg_txt(sd,365), skill_get_name(getSkillId()));
			clif_displaymessage(sd->fd, output); //"Duel: Can't use %s in duel."
			return;
		}

		if( sd->state.autocast || ( (sd->skillitem == getSkillId() || battle_config.skip_teleport_lv1_menu) && skill_lv == 1 ) || skill_lv == 3 )
		{
			if( skill_lv == 1 )
				pc_randomwarp(sd,CLR_TELEPORT);
			else
				pc_setpos( sd, mapindex_name2id( sd->status.save_point.map ), sd->status.save_point.x, sd->status.save_point.y, CLR_TELEPORT );
			return;
		}

		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);

		std::vector<std::string> maps = {
			"Random"
		};

		if( skill_lv == 1 ){
			clif_skill_warppoint( *sd, getSkillId(), skill_lv, maps );
		}else{
			maps.push_back( sd->status.save_point.map );

			clif_skill_warppoint( *sd, getSkillId(), skill_lv, maps );
		}
	} else
		unit_warp(target,-1,-1,-1,CLR_TELEPORT);
}
