// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "beastlyhypnosis.hpp"

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"
#include "map/pet.hpp"

SkillBeastlyHypnosis::SkillBeastlyHypnosis() : SkillImpl(SA_TAMINGMONSTER) {
}

void SkillBeastlyHypnosis::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );
	mob_data* dstmd = BL_CAST(BL_MOB, target);

	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	if (sd != nullptr && dstmd != nullptr) {
		pet_catch_process_start( *sd, 0, PET_CATCH_UNIVERSAL_ALL );
	}
}
