// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "kaute.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillKaute::SkillKaute() : SkillImpl(SP_KAUTE) {
}

void SkillKaute::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* sstatus = status_get_status_data(*src);
	status_data* tstatus = status_get_status_data(*target);
	map_session_data* sd = BL_CAST( BL_PC, src );
	map_session_data* dstsd = BL_CAST( BL_PC, target );

	if (sd) {
		if (!dstsd || !(
			(sd->sc.getSCE(SC_SPIRIT) && sd->sc.getSCE(SC_SPIRIT)->val2 == SL_SOULLINKER) ||
			(dstsd->class_&MAPID_SECONDMASK) == MAPID_SOUL_LINKER ||
			dstsd->status.char_id == sd->status.char_id ||
			dstsd->status.char_id == sd->status.partner_id ||
			dstsd->status.char_id == sd->status.child ||
			(dstsd->sc.getSCE(SC_SOULUNITY))
		)) {
			status_change_start(src,src,SC_STUN,10000,skill_lv,0,0,0,500,SCSTART_NORATEDEF);
			clif_skill_fail( *sd, getSkillId() );
			return;
		}
	}
	if (!status_charge(src, sstatus->max_hp * (10 + 2 * skill_lv) / 100, 0)) {
		if (sd)
			clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL );
		return;
	}
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	status_heal(target, 0, tstatus->max_sp * (10 + 2 * skill_lv) / 100, 2);
}
