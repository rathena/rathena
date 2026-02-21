// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "glacialstomp.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

#include "skill_factory_druid.hpp"

SkillGlacialStomp::SkillGlacialStomp() : SkillImplRecursiveDamageSplash(AT_GLACIER_STOMP) {
}

void SkillGlacialStomp::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillGlacialStomp::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change* sc = status_get_sc(src);

	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	if (!(flag & 1)) {
		int32 gx = 0;
		int32 gy = 0;
		if (!SkillFactoryDruid::get_glacier_center_on_map(src, sc, gx, gy)) {
			map_session_data* sd = BL_CAST(BL_PC, src);
			if (sd) {
				clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
			}
			return;
		}
		if (!unit_movepos(src, gx, gy, 2, true)) {
			map_session_data* sd = BL_CAST(BL_PC, src);
			if (sd) {
				clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
			}
			return;
		}
		clif_fixpos(*src);
	}
	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);

	if (!(flag & 1)) {
		int32 gx = 0;
		int32 gy = 0;
		if (SkillFactoryDruid::get_glacier_center_on_map(src, sc, gx, gy)) {
			skill_castend_pos2(src, gx, gy, AT_GLACIER_NOVA, 1, tick, 0);
		}
	}
}

void SkillGlacialStomp::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	int32 skillratio = 6400 + 500 * (skill_lv - 1);
	if (sc != nullptr && sc->hasSCE(SC_TRUTH_OF_ICE)) {
		skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
		RE_LVL_DMOD(100);
	}
	base_skillratio += -100 + skillratio;
}

int64 SkillGlacialStomp::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	return skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}
