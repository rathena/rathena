// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_impl.hpp"

#include "../status.hpp"

SkillImpl::SkillImpl(e_skill skill_id){
	this->skill_id_ = skill_id;
}

e_skill SkillImpl::getSkillId() const {
	return skill_id_;
}

void SkillImpl::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	// no-op
}

void SkillImpl::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	// no-op
}

void SkillImpl::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	// no-op
}

void SkillImpl::calculateSkillRatio(const Damage*, const block_list*, const block_list*, uint16, int32&) const {
	// no-op
}

void SkillImpl::modifyHitRate(int16&, const block_list*, const block_list*, uint16) const {
	// no-op
}

void SkillImpl::applyAdditionalEffects(block_list*, block_list*, uint16, t_tick, int32, enum damage_lv) const {
	// no-op
}

SkillImplRecursiveDamageSplash::SkillImplRecursiveDamageSplash(e_skill skill_id) : SkillImpl(skill_id){
}

void SkillImplRecursiveDamageSplash::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change* tsc = status_get_sc(target);

	if (flag & 1){
		// Recursive invocation
		int32 sflag = skill_area_temp[0] & 0xFFF;
		std::bitset<INF2_MAX> inf2 = skill_db.find(getSkillId())->inf2;

		if (tsc && tsc->getSCE(SC_HOVERING) && inf2[INF2_IGNOREHOVERING])
			return; // Under Hovering characters are immune to select trap and ground target skills.

		if (flag & SD_LEVEL)
			sflag |= SD_LEVEL; // -1 will be used in packets instead of the skill level
		if (skill_area_temp[1] != target->id && !inf2[INF2_ISNPC])
			sflag |= SD_ANIMATION; // original target gets no animation (as well as all NPC skills)

		this->splashDamage(src, target, skill_lv, tick, sflag);
	}else{
		skill_area_temp[0] = 0;
		skill_area_temp[1] = target->id;
		skill_area_temp[2] = 0;

		this->splashSearch(src, target, skill_lv, tick, flag);
	}
}

void SkillImplRecursiveDamageSplash::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	// Cast center might be relevant later (e.g. for knockback direction)
	skill_area_temp[4] = x;
	skill_area_temp[5] = y;

	int16 size = this->getSplashSearchSize(skill_lv);

	map_foreachinarea(skill_area_sub, src->m, x - size, y - size, x + size, y + size, this->getSplashTarget(src), src, this->getSkillId(), skill_lv, tick, flag | BCT_ENEMY | 1, skill_castend_damage_id);
}

int16 SkillImplRecursiveDamageSplash::getSearchSize(uint16 skill_lv) const {
	return skill_get_splash( this->getSkillId(), skill_lv );
}

int16 SkillImplRecursiveDamageSplash::getSplashSearchSize(uint16 skill_lv) const {
	return skill_get_splash( this->getSkillId(), skill_lv );
}

int32 SkillImplRecursiveDamageSplash::getSplashTarget(block_list* src) const {
	return BL_CHAR|BL_SKILL;
}

void SkillImplRecursiveDamageSplash::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	// if skill damage should be split among targets, count them
	// SD_LEVEL -> Forced splash damage -> count targets
	if (flag & SD_LEVEL || skill_get_nk(getSkillId(), NK_SPLASHSPLIT)){
		skill_area_temp[0] = map_foreachinallrange(skill_area_sub, target, this->getSearchSize(skill_lv), BL_CHAR, src, getSkillId(), skill_lv, tick, BCT_ENEMY, 1);
		// If there are no characters in the area, then it always counts as if there was one target
		// This happens when targetting skill units such as icewall
		skill_area_temp[0] = std::max(1, skill_area_temp[0]);
	}

	// recursive invocation of skill_castend_damage_id() with flag|1
	map_foreachinrange(skill_area_sub, target, this->getSplashSearchSize(skill_lv), this->getSplashTarget(src), src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
}

int64 SkillImplRecursiveDamageSplash::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	return skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}
