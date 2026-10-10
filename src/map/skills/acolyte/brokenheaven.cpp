// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "brokenheaven.hpp"

#include "map/skill.hpp"
#include "map/status.hpp"

namespace {
// The map loop is single-threaded. Nested casts restore their caller's budget.
int64 broken_heaven_drain_total = 0;

int32 broken_heaven_chapter(const status_change* sc) {
	if (sc == nullptr)
		return 0;
	if (sc->getSCE(SC_THIRD_EXOR_FLAME))
		return 3;
	if (sc->getSCE(SC_SECOND_JUDGE))
		return 2;
	return sc->getSCE(SC_FIRST_FAITH_POWER) != nullptr ? 1 : 0;
}
}

SkillBrokenHeaven::SkillBrokenHeaven() : SkillImplRecursiveDamageSplash(IQ_BROKENHEAVEN) {
}

void SkillBrokenHeaven::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 1100 * skill_lv + 2450;
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillBrokenHeaven::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	const status_change* sc = status_get_sc(&src);

	switch (broken_heaven_chapter(sc)) {
		case 3: dmg.div_ = 2; break;
		case 2: dmg.div_ = 4; break;
		default: dmg.div_ = 3; break;
	}
}

void SkillBrokenHeaven::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_SECOND_BRAND, 100, skill_lv, 5000);
}

int64 SkillBrokenHeaven::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	int64 damage = SkillImplRecursiveDamageSplash::splashDamage(src, target, skill_lv, tick, flag);

	// Third Exorcism Flame: drain 3% per level of the damage, at most 50,000 HP per cast.
	if (damage > 0 && broken_heaven_chapter(status_get_sc(src)) == 3) {
		int64 drain = std::min<int64>(damage * 3 * skill_lv / 100, 50000 - broken_heaven_drain_total);

		if (drain > 0) {
			broken_heaven_drain_total += drain;
			status_heal(src, drain, 0, 2);
		}
	}

	return damage;
}

void SkillBrokenHeaven::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (flag & 1) {
		SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
		return;
	}

	const int64 previous = broken_heaven_drain_total;
	broken_heaven_drain_total = 0;
	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
	broken_heaven_drain_total = previous;
}
