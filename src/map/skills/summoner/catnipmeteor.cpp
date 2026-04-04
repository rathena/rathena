// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "catnipmeteor.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

// SU_CN_METEOR
SkillCatnipMeteor::SkillCatnipMeteor() : SkillImpl(SU_CN_METEOR) {
}

void SkillCatnipMeteor::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 200 + 100 * skill_lv;
	if (status_get_lv(src) > 99) {
		skillratio += sstatus->int_ * 5;
	}
	RE_LVL_DMOD(100);
}

void SkillCatnipMeteor::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	e_skill skill_id = getSkillId();

	if (sd) {
		// FIX ME: missing check of required item
		if (pc_search_inventory(sd, skill_db.find(SU_CN_METEOR)->require.itemid[0]) >= 0)
			skill_id = SU_CN_METEOR2;
		if (pc_checkskill(sd, SU_SPIRITOFLAND))
			sc_start(src, src, SC_DORAM_SVSP, 100, 100, skill_get_time(SU_SPIRITOFLAND, 1));
	}

	int32 area = skill_get_splash(skill_id, skill_lv);
	int16 tmpx = 0, tmpy = 0;

	for (int32 i = 1; i <= skill_get_time(skill_id, skill_lv) / skill_get_unit_interval(skill_id); i++) {
		// Creates a random Cell in the Splash Area
		tmpx = x - area + rnd() % (area * 2 + 1);
		tmpy = y - area + rnd() % (area * 2 + 1);
		skill_unitsetting(src, skill_id, skill_lv, tmpx, tmpy, flag + i * skill_get_unit_interval(skill_id));
	}
}


// SU_CN_METEOR2
SkillCatnipMeteor2::SkillCatnipMeteor2() : SkillImpl(SU_CN_METEOR2) {
}

void SkillCatnipMeteor2::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_CURSE, 20, skill_lv, skill_get_time2(getSkillId(), skill_lv));
}

void SkillCatnipMeteor2::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 200 + 100 * skill_lv;
	if (status_get_lv(src) > 99) {
		skillratio += sstatus->int_ * 5;
	}
	RE_LVL_DMOD(100);
}
