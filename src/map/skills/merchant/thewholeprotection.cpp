// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "thewholeprotection.hpp"

#include "map/clif.hpp"
#include "map/party.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillTheWholeProtection::SkillTheWholeProtection() : SkillImpl(BO_THE_WHOLE_PROTECTION) {
}

void SkillTheWholeProtection::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);

	if (sd == nullptr || sd->status.party_id == 0 || (flag & 1)) {
		uint32 equip[] = { EQP_WEAPON, EQP_SHIELD, EQP_ARMOR, EQP_HEAD_TOP };

		for (uint8 i_eqp = 0; i_eqp < 4; i_eqp++) {
			if (target->type != BL_PC || (dstsd && pc_checkequip(dstsd, equip[i_eqp]) < 0))
				continue;
			sc_start(src, target, (sc_type)(SC_CP_WEAPON + i_eqp), 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
		}
	} else if (sd) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		party_foreachsamemap(skill_area_sub, sd, skill_get_splash(getSkillId(), skill_lv), src, getSkillId(), skill_lv, tick, flag | BCT_PARTY | 1, skill_castend_nodamage_id);
	}
}
