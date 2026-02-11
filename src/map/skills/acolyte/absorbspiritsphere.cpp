// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "absorbspiritsphere.hpp"

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillAbsorbSpiritSphere::SkillAbsorbSpiritSphere() : SkillImpl(MO_ABSORBSPIRITS) {
}

void SkillAbsorbSpiritSphere::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);
	mob_data* dstmd = BL_CAST(BL_MOB, target);
	status_data* tstatus = status_get_status_data(*target);

	int32 i = 0;
	if (dstsd && (battle_check_target(src, target, BCT_SELF) > 0 || battle_check_target(src, target, BCT_ENEMY) > 0) && // Only works on self and enemies
		(dstsd->class_&MAPID_FIRSTMASK) != MAPID_GUNSLINGER ) { // split the if for readability, and included gunslingers in the check so that their coins cannot be removed [Reddozen]
		if (dstsd->spiritball > 0) {
			i = dstsd->spiritball * 7;
			pc_delspiritball(dstsd,dstsd->spiritball,0);
		}
		if (dstsd->spiritcharm_type != CHARM_TYPE_NONE && dstsd->spiritcharm > 0) {
			i += dstsd->spiritcharm * 7;
			pc_delspiritcharm(dstsd,dstsd->spiritcharm,dstsd->spiritcharm_type);
		}
	} else if (dstmd && !status_has_mode(tstatus,MD_STATUSIMMUNE) && rnd() % 100 < 20) { // check if target is a monster and not status immune, for the 20% chance to absorb 2 SP per monster's level [Reddozen]
		i = 2 * dstmd->level;
		mob_target(dstmd,src,0);
	} else {
		if (sd)
			clif_skill_fail( *sd, getSkillId() );
		return;
	}
	if (i) status_heal(src, 0, i, 3);
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv,i != 0);
}
