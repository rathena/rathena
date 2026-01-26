// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "glacialshard.hpp"

#include "map/clif.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

#include "skill_factory_druid.hpp"

SkillGlacialShard::SkillGlacialShard() : SkillImplRecursiveDamageSplash(AT_GLACIER_SHARD) {
}

void SkillGlacialShard::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change* sc = status_get_sc(src);

	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
	if (!(flag & 1)) {
		int32 gx = 0;
		int32 gy = 0;
		if (get_glacier_center_on_map(src, sc, gx, gy)) {
			skill_castend_pos2(src, gx, gy, AT_GLACIER_NOVA, 1, tick, 0);
		}
	}
}

void SkillGlacialShard::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	int32 skillratio = 5500 + 300 * (skill_lv - 1);
	if (sc && sc->hasSCE(SC_TRUTH_OF_ICE)) {
		skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
		RE_LVL_DMOD(100);
	}
	base_skillratio += -100 + skillratio;
}

int64 SkillGlacialShard::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	return skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}
