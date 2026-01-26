// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "pinionshot.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

constexpr int32 kZephyrChargeDuration = 10000;

int32 apply_zephyr_link_sub(block_list* bl, va_list ap);

SkillPinionShot::SkillPinionShot() : SkillImplRecursiveDamageSplash(AT_PINION_SHOT) {
}

void SkillPinionShot::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change* sc = status_get_sc(src);

	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	int32 attack_flag = flag | SD_ANIMATION;
	skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, attack_flag);

	if (!(flag & 1)) {
		status_change_entry *charge = sc ? sc->getSCE(SC_ZEPHYR_CHARGE) : nullptr;
		int32 current = charge ? charge->val1 : 0;
		if (current >= 5) {
			map_session_data *sd = BL_CAST(BL_PC, src);
			if (sd) {
				int32 party_id = sd->status.party_id;
				const int16 link_range = skill_get_splash(AT_ZEPHYR_LINK, 1);
				if (party_id == 0) {
					skill_castend_nodamage_id(src, src, AT_ZEPHYR_LINK, 1, tick, 0);
				} else {
					map_foreachinrange(apply_zephyr_link_sub, src, link_range, BL_PC, src, party_id, tick);
				}
			}
			status_change_end(src, SC_ZEPHYR_CHARGE);
		} else {
			sc_start4(src, src, SC_ZEPHYR_CHARGE, 100, current + 1, current + 1, current + 1, 0, kZephyrChargeDuration);
		}
	}
}

void SkillPinionShot::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	int32 skillratio = 2450 * skill_lv;
	skillratio += sstatus->con * 5; // TODO - unknown scaling [munkrej]
	RE_LVL_DMOD(100);
	base_skillratio += -100 + skillratio;
}

int32 apply_zephyr_link_sub(block_list *bl, va_list ap) {
	block_list *src = va_arg(ap, block_list *);
	int32 party_id = va_arg(ap, int32);
	t_tick tick = va_arg(ap, t_tick);
	map_session_data *sd = BL_CAST(BL_PC, bl);

	if (!sd || sd->status.party_id != party_id) {
		return 0;
	}

	return skill_castend_nodamage_id(src, bl, AT_ZEPHYR_LINK, 1, tick, 0);
}
