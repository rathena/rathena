// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "overdriveprotocol.hpp"

#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

SkillOverdriveProtocol::SkillOverdriveProtocol() : SkillImplRecursiveDamageSplash(MT_OVERDRIVE_PROTOCAL) {
}

void SkillOverdriveProtocol::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 1050 * skill_lv + 2600 + 50 * pc_checkskill(sd, MT_ABR_M);
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillOverdriveProtocol::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	const status_change* sc = status_get_sc(&src);

	dmg.div_ = (sc != nullptr && sc->hasSCE(SC_ABR_DUAL_CANNON)) ? 4 : 2;
}
