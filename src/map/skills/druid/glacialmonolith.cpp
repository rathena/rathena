// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "glacialmonolith.hpp"

#include "map/clif.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

void clear_glacier_monolith(block_list* src);

SkillGlacialMonolith::SkillGlacialMonolith() : SkillImplRecursiveDamageSplash(AT_GLACIER_MONOLITH) {
}

void SkillGlacialMonolith::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillGlacialMonolith::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	SkillImplRecursiveDamageSplash::castendPos2(src, x, y, skill_lv, tick, flag);
	clear_glacier_monolith(src);
	const t_tick buff_duration = skill_get_unit_interval(getSkillId());
	sc_start4(src, src, SC_GLACIER_SHEILD, 100, skill_lv, x, y, src->m, buff_duration);
	skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);
}

void SkillGlacialMonolith::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	int32 skillratio = 7100 + 300 * (skill_lv - 1);
	if (sc && sc->hasSCE(SC_TRUTH_OF_ICE)) {
		skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
		RE_LVL_DMOD(100);
	}
	base_skillratio += -100 + skillratio;
}

int64 SkillGlacialMonolith::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	return skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

void clear_glacier_monolith(block_list* src) {
	unit_data* ud = unit_bl2ud(src);

	if (!ud) {
		return;
	}

	for (size_t i = 0; i < ud->skillunits.size();) {
		std::shared_ptr<s_skill_unit_group> sg = ud->skillunits[i];

		if (sg && (sg->skill_id == AT_GLACIER_MONOLITH || sg->unit_id == UNT_GLACIAL_MONOLITH)) {
			skill_delunitgroup(sg);
			continue;
		}

		++i;
	}
}
