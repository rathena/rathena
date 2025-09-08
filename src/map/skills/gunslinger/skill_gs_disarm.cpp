#include "skill_gs_disarm.hpp"

SkillGS_DISARM::SkillGS_DISARM() : WeaponSkillImpl(GS_DISARM) {
}

void SkillGS_DISARM::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_WEAPON, src, src, target, GS_DISARM, skill_lv, tick, flag);
}

void SkillGS_DISARM::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	skill_strip_equip(src, target, GS_DISARM, skill_lv);
	clif_skill_nodamage(src, *target, GS_DISARM, skill_lv);
}
