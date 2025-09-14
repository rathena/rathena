// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_tk_jumpkick.hpp"

SkillJumpkick::SkillJumpkick() : WeaponSkillImpl(TK_JUMPKICK) {
}

void SkillJumpkick::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio) const {
	// Different damage formulas depending on damage trigger
	struct status_change *sc = status_get_sc(src);
	if (sc && sc->getSCE(SC_COMBO) && sc->getSCE(SC_COMBO)->val1 == this->skill_id)
		base_skillratio += -100 + 4 * status_get_lv(src); // Tumble formula [4%*baselevel]
	else if (wd->miscflag) {
		base_skillratio += -100 + 4 * status_get_lv(src); // Running formula [4%*baselevel]
		if (sc && sc->getSCE(SC_SPURT)) // Spurt formula [8%*baselevel]
			base_skillratio *= 2;
	} else
		base_skillratio += -70 + 10 * skill_lv;
}

void SkillJumpkick::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	struct status_change *tsc = status_get_sc(target);
	struct map_session_data *dstsd = BL_CAST(BL_PC, target);

	// debuff the following statuses
	if (dstsd && dstsd->class_ != MAPID_SOUL_LINKER && !tsc->getSCE(SC_PRESERVE)) {
		status_change_end(target, SC_SPIRIT);
		status_change_end(target, SC_ADRENALINE2);
		status_change_end(target, SC_KAITE);
		status_change_end(target, SC_KAAHI);
		status_change_end(target, SC_ONEHAND);
		status_change_end(target, SC_ASPDPOTION2);
		// New soul links confirmed to not dispell with this skill
		// but thats likely a bug since soul links can't stack and
		// soul cutter skill works on them. So ill add this here for now. [Rytech]
		status_change_end(target, SC_SOULGOLEM);
		status_change_end(target, SC_SOULSHADOW);
		status_change_end(target, SC_SOULFALCON);
		status_change_end(target, SC_SOULFAIRY);
	}
}

void SkillJumpkick::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_WEAPON, src, src, target, this->skill_id, skill_lv, tick, flag);
}

void SkillJumpkick::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const {
	struct map_session_data *sd = BL_CAST(BL_PC, src);

	/* Check if the target is an enemy; if not, skill should fail so the character doesn't unit_movepos (exploitable) */
	if (battle_check_target(src, bl, BCT_ENEMY) > 0) {
		if (unit_movepos(src, bl->x, bl->y, 2, 1)) {
			skill_attack(BF_WEAPON, src, src, bl, this->skill_id, skill_lv, tick, flag);
			clif_blown(src);
		}
	} else if (sd) {
		clif_skill_fail(*sd, this->skill_id, USESKILL_FAIL);
	}
}
