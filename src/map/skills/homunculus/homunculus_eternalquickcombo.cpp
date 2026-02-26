// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_eternalquickcombo.hpp"

#include "map/clif.hpp"
#include "map/homunculus.hpp"
#include "map/status.hpp"

SkillEternalQuickCombo::SkillEternalQuickCombo() : SkillImpl(MH_EQC) {
}

void SkillEternalQuickCombo::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 duration = max(skill_lv, (status_get_str(src) / 7 - status_get_str(target) / 10)) * 1000; //Yommy formula

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start4(src, target, SC_EQC, 100, skill_lv, src->id, 0, 0, duration));
	skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillEternalQuickCombo::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	homun_data* hd = BL_CAST(BL_HOM, src);

	if (hd) {
		sc_start2(src, target, SC_STUN, 100, skill_lv, target->id, 1000 * hd->homunculus.level / 50 + 500 * skill_lv);
		status_change_end(target, SC_TINDER_BREAKER2);
	}
}
