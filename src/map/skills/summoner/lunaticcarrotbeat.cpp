// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "lunaticcarrotbeat.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

// SU_LUNATICCARROTBEAT
SkillLunaticCarrotBeat::SkillLunaticCarrotBeat() : SkillImplRecursiveDamageSplash(SU_LUNATICCARROTBEAT) {
}

void SkillLunaticCarrotBeat::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += 100 + 100 * skill_lv;
	if (sd && pc_checkskill(sd, SU_SPIRITOFLIFE))
		skillratio += skillratio * status_get_hp(src) / status_get_max_hp(src);
	if (status_get_lv(src) > 99)
		skillratio += sstatus->str;
	RE_LVL_DMOD(100);
}

void SkillLunaticCarrotBeat::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (!(flag & 1)) {
		map_session_data* sd = BL_CAST(BL_PC, src);

		// FIX ME: missing check of required item
		if (sd && pc_search_inventory(sd, skill_db.find(getSkillId())->require.itemid[0]) >= 0) {
			SkillLunaticCarrotBeat2 lunatic2;
			lunatic2.castendDamageId(src, target, skill_lv, tick, flag);
		}
		else {
			SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
		}
	}
}


// SU_LUNATICCARROTBEAT2
SkillLunaticCarrotBeat2::SkillLunaticCarrotBeat2() : SkillImplRecursiveDamageSplash(SU_LUNATICCARROTBEAT2) {
}

void SkillLunaticCarrotBeat2::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += 100 + 100 * skill_lv;
	if (sd && pc_checkskill(sd, SU_SPIRITOFLIFE))
		skillratio += skillratio * status_get_hp(src) / status_get_max_hp(src);
	if (status_get_lv(src) > 99)
		skillratio += sstatus->str;
	RE_LVL_DMOD(100);
}

void SkillLunaticCarrotBeat2::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_STUN, 20, skill_lv, skill_get_time2(getSkillId(), skill_lv));
}

