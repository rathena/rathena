// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_sbr44.hpp"

#include "map/clif.hpp"
#include "map/homunculus.hpp"

SkillSBR44::SkillSBR44() : WeaponSkillImpl(HFLI_SBR44) {
}

void SkillSBR44::applyCounterAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& attack_type) const {
	if (src->type == BL_HOM) {
		homun_data& hd = reinterpret_cast<homun_data&>(*src);

		hd.homunculus.intimacy = hom_intimacy_grade2intimacy(HOMGRADE_HATE_WITH_PASSION);

		clif_send_homdata(hd, SP_INTIMATE);
	}
}

void SkillSBR44::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	base_skillratio += 100 * (skill_lv - 1);
}
