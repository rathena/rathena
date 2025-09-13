// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "crazyuproar.hpp"

#include "map/clif.hpp"
#include "map/party.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillCrazyUproar::SkillCrazyUproar() : SkillImpl(MC_LOUD) {
}

void SkillCrazyUproar::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 flag) const {
#ifdef RENEWAL
	map_session_data *sd = BL_CAST(BL_PC, src);
	sc_type type = skill_get_sc(getSkillId());

	if (sd == nullptr || sd->status.party_id == 0 || (flag & 1)) {
		clif_skill_nodamage(src, *src, getSkillId(), skill_lv, sc_start(src, src, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
	} else if (sd) {
		party_foreachsamemap(skill_area_sub, sd, skill_get_splash(getSkillId(), skill_lv), src, getSkillId(), skill_lv, tick, flag | BCT_PARTY | 1, skill_castend_nodamage_id);
	}
#endif
}
