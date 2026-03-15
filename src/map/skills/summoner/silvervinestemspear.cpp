// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "silvervinestemspear.hpp"

#include "map/pc.hpp"
#include "map/status.hpp"

SkillSilvervineStemSpear::SkillSilvervineStemSpear() : SkillImpl(SU_SV_STEMSPEAR) {
}

void SkillSilvervineStemSpear::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start2(src, target, SC_BLEEDING, 10, skill_lv, src->id, skill_get_time2(getSkillId(), skill_lv));
}

void SkillSilvervineStemSpear::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 600;
}

void SkillSilvervineStemSpear::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd && pc_checkskill(sd, SU_SPIRITOFLAND))
		sc_start(src, src, SC_DORAM_WALKSPEED, 100, 50, skill_get_time(SU_SPIRITOFLAND, 1));
	skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}
