// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "tastyshrimpparty.hpp"

#include "map/clif.hpp"
#include "map/party.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillTastyShrimpParty::SkillTastyShrimpParty() : SkillImpl(SU_SHRIMPARTY) {
}

void SkillTastyShrimpParty::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST(BL_PC, src);
	int32 i = 0;

	if (sd == nullptr || sd->status.party_id == 0 || flag&1) {
		sc_start(src, target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
		if (sd && (i = pc_checkskill(sd, SU_FRESHSHRIMP)) > 0) {
			clif_skill_nodamage(target, *target, SU_FRESHSHRIMP, i, 1);
			sc_start(src, target, SC_FRESHSHRIMP, 100, i, skill_get_time(SU_FRESHSHRIMP, i));
		}
	} else if (sd)
		party_foreachsamemap(skill_area_sub, sd, skill_get_splash(getSkillId(), skill_lv), src, getSkillId(), skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
}
