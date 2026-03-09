// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "angelus.hpp"

#include "../../map.hpp"
#include "../../party.hpp"
#include "../../pc.hpp"

SkillAngelus::SkillAngelus() : SkillImpl(AL_ANGELUS)
{
}

void SkillAngelus::castendNoDamageId(block_list *src, block_list *bl, uint16 skill_lv, t_tick tick, int32& flag) const
{
	map_session_data *sd = BL_CAST(BL_PC, src);

	if (sd == nullptr || sd->status.party_id == 0 || (flag & 1))
	{
		sc_type type = skill_get_sc(getSkillId());

		// Animations don't play when outside visible range
		if (check_distance_bl(src, bl, AREA_SIZE))
			clif_skill_nodamage(bl, *bl, getSkillId(), skill_lv);


		sc_start(src, bl, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
	}
	else if (sd != nullptr)
	{
		party_foreachsamemap(skill_area_sub, sd, skill_get_splash(getSkillId(), skill_lv), src, getSkillId(), skill_lv, tick, flag | BCT_PARTY | 1, skill_castend_nodamage_id);
	}
}
