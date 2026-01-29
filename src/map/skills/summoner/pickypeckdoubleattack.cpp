// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "pickypeckdoubleattack.hpp"

#include "map/pc.hpp"
#include "map/status.hpp"

SkillPickyPeckDoubleAttack::SkillPickyPeckDoubleAttack() : SkillImpl(SU_PICKYPECK_DOUBLE_ATK) {
}

void SkillPickyPeckDoubleAttack::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST( BL_PC, src );

	base_skillratio += 100 + 100 * skill_lv;
	if (status_get_hp(target) < (status_get_max_hp(target) / 2))
		base_skillratio *= 2;
	if (sd && pc_checkskill(sd, SU_SPIRITOFLIFE))
		base_skillratio += base_skillratio * status_get_hp(src) / status_get_max_hp(src);
}
