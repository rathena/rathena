// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "kisulwaterspraying.hpp"

#include "map/clif.hpp"
#include "map/party.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillKisulWaterSpraying::SkillKisulWaterSpraying() : SkillImpl(SH_KI_SUL_WATER_SPRAYING) {
}

void SkillKisulWaterSpraying::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd == nullptr || sd->status.party_id == 0 || (flag & 1)) {
		// TODO: verify on official server, if this should be moved into skill_calc_heal
		int32 heal = 500 * skill_lv + status_get_int(src) * 5;
		heal += pc_checkskill(sd, SH_MYSTICAL_CREATURE_MASTERY) * 100;

		if( pc_checkskill( sd, SH_COMMUNE_WITH_KI_SUL ) > 0 || ( sc != nullptr && sc->getSCE( SC_TEMPORARY_COMMUNION ) != nullptr ) ){
			heal += 250 * skill_lv;
			heal += pc_checkskill(sd, SH_MYSTICAL_CREATURE_MASTERY) * 50;
		}
		heal = heal * (100 + status_get_crt(src)) * status_get_lv(src) / 10000;
		status_heal(target, heal, 0, 0, 0);
		clif_skill_nodamage(nullptr, *target, AL_HEAL, heal);
	}
	else {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		int32 range = skill_get_splash(getSkillId(), skill_lv);
		if( pc_checkskill( sd, SH_COMMUNE_WITH_KI_SUL ) > 0 || ( sc != nullptr && sc->getSCE( SC_TEMPORARY_COMMUNION ) != nullptr ) )
			range += 2;
		party_foreachsamemap(skill_area_sub, sd, range, src, getSkillId(), skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
	}
}
