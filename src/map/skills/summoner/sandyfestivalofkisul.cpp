// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "sandyfestivalofkisul.hpp"

#include "map/clif.hpp"
#include "map/party.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillSandyFestivalofKisul::SkillSandyFestivalofKisul() : SkillImpl(SH_SANDY_FESTIVAL_OF_KI_SUL) {
}

void SkillSandyFestivalofKisul::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd == nullptr || sd->status.party_id == 0 || (flag & 1)) {
		int32 time = skill_get_time(getSkillId(), skill_lv);
		if( pc_checkskill( sd, SH_COMMUNE_WITH_KI_SUL ) > 0 || ( sc != nullptr && sc->getSCE( SC_TEMPORARY_COMMUNION ) != nullptr ) )
			time *= 2;
		sc_start(src, target, type, 100, skill_lv, time);
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}
	else {
		int32 range = skill_get_splash(getSkillId(), skill_lv);
		if( pc_checkskill( sd, SH_COMMUNE_WITH_KI_SUL ) > 0 || ( sc != nullptr && sc->getSCE( SC_TEMPORARY_COMMUNION ) != nullptr ) )
			range += 2;
		party_foreachsamemap(skill_area_sub, sd, range, src, getSkillId(), skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
	}
}
