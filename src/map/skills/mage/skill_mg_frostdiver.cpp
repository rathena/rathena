#include "skill_mg_frostdiver.hpp"

SkillMG_FROSTDIVER::SkillMG_FROSTDIVER() : SkillImpl(MG_FROSTDIVER) {
}

void SkillMG_FROSTDIVER::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillMG_FROSTDIVER::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	if (!sc_start(src, target, SC_FREEZE, min(skill_lv * 3 + 35, skill_lv + 60), skill_lv, skill_get_time2(getSkillId(), skill_lv)) && sd)
		clif_skill_fail(*sd, getSkillId());
}