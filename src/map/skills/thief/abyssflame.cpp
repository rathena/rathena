// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "abyssflame.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillAbyssFlame::SkillAbyssFlame() : SkillImpl(ABC_ABYSS_FLAME) {
}

void SkillAbyssFlame::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_foreachinrange(skill_area_sub, src, skill_get_splash(getSkillId(), skill_lv), BL_CHAR | BL_SKILL, src, getSkillId(), skill_lv, tick, (flag | BCT_ENEMY | SD_SPLASH) & ~BCT_SELF, skill_castend_damage_id);
	skill_castend_damage_id(src, target, ABC_ABYSS_FLAME_ATK, skill_lv, tick, flag);
}

void SkillAbyssFlame::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	clif_skill_damage(*src, *target, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE);
	skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillAbyssFlame::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 500 * skill_lv;
	skillratio += 10 * sstatus->spl;
	skillratio += 15 * skill_lv * pc_checkskill(sd, ABC_MAGIC_SWORD_M);
	RE_LVL_DMOD(100);
}

SkillAbyssFlameAttack::SkillAbyssFlameAttack() : SkillImplRecursiveDamageSplash(ABC_ABYSS_FLAME_ATK) {
}

void SkillAbyssFlameAttack::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 820 * skill_lv;
	skillratio += 10 * sstatus->spl;
	skillratio += 30 * skill_lv * pc_checkskill(sd, ABC_MAGIC_SWORD_M);
	RE_LVL_DMOD(100);
}

void SkillAbyssFlameAttack::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_damage(*src, *target, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE);
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
