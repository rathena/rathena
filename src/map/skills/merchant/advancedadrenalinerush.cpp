// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "advancedadrenalinerush.hpp"

#include "map/clif.hpp"
#include "map/party.hpp"
#include "map/pc.hpp"

SkillAdvancedAdrenalineRush::SkillAdvancedAdrenalineRush() : SkillImpl(BS_ADRENALINE2) {
}

void SkillAdvancedAdrenalineRush::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);

	if (sd == nullptr || sd->status.party_id == 0 || (flag & 1)) {
		int32 weapontype = skill_get_weapontype(getSkillId());
		if (!weapontype || !dstsd || pc_check_weapontype(dstsd, weapontype)) {
			clif_skill_nodamage(target, *target, getSkillId(), skill_lv,
				sc_start2(src, target, skill_get_sc(getSkillId()), 100, skill_lv, (src == target) ? 1 : 0, skill_get_time(getSkillId(), skill_lv)));
		}
	} else if (sd) {
		party_foreachsamemap(skill_area_sub,
			sd,skill_get_splash(getSkillId(), skill_lv),
			src,getSkillId(),skill_lv,tick, flag|BCT_PARTY|1,
			skill_castend_nodamage_id);
	}
}
