#include "skill_tk_turnkick.hpp"

SkillTK_TURNKICK::SkillTK_TURNKICK() : WeaponSkillImpl(TK_TURNKICK) {
}

void SkillTK_TURNKICK::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	base_skillratio += 90 + 30 * skill_lv;
}

void SkillTK_TURNKICK::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	// Note: attack_type is passed as BF_WEAPON for the actual target, BF_MISC for the splash-affected mobs.
	if (attack_type & BF_MISC) {
		sc_start(src, target, SC_STUN, 200, skill_lv, skill_get_time(this->skill_id, skill_lv));
		clif_specialeffect(target, EF_SPINEDBODY, AREA);
		sc_start(src, target, SC_NOACTION, 100, 1, skill_get_time2(this->skill_id, skill_lv));
	}
}

void SkillTK_TURNKICK::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_area_temp[1] = target->id; // NOTE: This is used in skill_castend_nodamage_id to avoid affecting the target.
	if (skill_attack(BF_WEAPON, src, src, target, this->skill_id, skill_lv, tick, flag))
		map_foreachinallrange(skill_area_sub, target,
							  skill_get_splash(this->skill_id, skill_lv), (this->skill_id == TK_TURNKICK) ? BL_MOB : BL_CHAR,
							  src, this->skill_id, skill_lv, tick, flag | BCT_ENEMY | 1,
							  skill_castend_nodamage_id);
}

void SkillTK_TURNKICK::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const {
	if (skill_area_temp[1] != bl->id) {
		skill_blown(src, bl, skill_get_blewcount(this->skill_id, skill_lv), -1, BLOWN_NONE);
		skill_additional_effect(src, bl, this->skill_id, skill_lv, BF_MISC, ATK_DEF, tick); // Use Misc rather than weapon to signal passive pushback
	}
}
