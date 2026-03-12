// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "catnipmeteor.hpp"

#include <common/random.hpp>

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

// SU_CN_METEOR
SkillCatnipMeteor::SkillCatnipMeteor() : SkillImpl(SU_CN_METEOR) {
}

void SkillCatnipMeteor::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	skillratio += -100 + 200 + 100 * skill_lv;

	if (status_get_lv(src) > 99) {
		const status_data* sstatus = status_get_status_data(*src);

		skillratio += sstatus->int_ * 5;

		RE_LVL_DMOD(100);
	}
}

void SkillCatnipMeteor::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (map_session_data* sd = BL_CAST(BL_PC, src); pc_checkskill(sd, SU_SPIRITOFLAND) > 0) {
		sc_start(src, src, SC_DORAM_SVSP, 100, 100, skill_get_time(SU_SPIRITOFLAND, 1));
	}

	if (int32 interval = skill_get_unit_interval(getSkillId()); interval > 0) {
		int32 area = skill_get_splash(getSkillId(), skill_lv);

		for (int32 i = 1; i <= skill_get_time(getSkillId(), skill_lv) / interval; i++) {
			// Creates a random Cell in the Splash Area
			int16 tmpx = x - area + rnd() % (area * 2 + 1);
			int16 tmpy = y - area + rnd() % (area * 2 + 1);
			skill_unitsetting(src, getSkillId(), skill_lv, tmpx, tmpy, flag + i * interval);
		}
	}
}


// SU_CN_METEOR2
SkillCatnipMeteor2::SkillCatnipMeteor2() : SkillImpl(SU_CN_METEOR2) {
}

void SkillCatnipMeteor2::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_CURSE, 20, skill_lv, skill_get_time2(getSkillId(), skill_lv));
}

void SkillCatnipMeteor2::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 200 + 100 * skill_lv;

	if (status_get_lv(src) > 99) {
		const status_data* sstatus = status_get_status_data(*src);

		skillratio += sstatus->int_ * 5;

		RE_LVL_DMOD(100);
	}
}

void SkillCatnipMeteor2::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (map_session_data* sd = BL_CAST(BL_PC, src); pc_checkskill(sd, SU_SPIRITOFLAND) > 0) {
		sc_start(src, src, SC_DORAM_SVSP, 100, 100, skill_get_time(SU_SPIRITOFLAND, 1));
	}

	if (int32 interval = skill_get_unit_interval(getSkillId()); interval > 0) {
		int32 area = skill_get_splash(getSkillId(), skill_lv);

		for (int32 i = 1; i <= skill_get_time(getSkillId(), skill_lv) / interval; i++) {
			// Creates a random Cell in the Splash Area
			int16 tmpx = x - area + rnd() % (area * 2 + 1);
			int16 tmpy = y - area + rnd() % (area * 2 + 1);
			skill_unitsetting(src, getSkillId(), skill_lv, tmpx, tmpy, flag + i * interval);
		}
	}
}
