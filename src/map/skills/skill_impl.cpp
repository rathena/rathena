// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_impl.hpp"

#include "../clif.hpp"
#include "../status.hpp"

SkillImpl::SkillImpl(e_skill skill_id) : skill_id_(skill_id) {
}

e_skill SkillImpl::getSkillId() const {
	return skill_id_;
}

void SkillImpl::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	throw SkillImplException("castendNoDamageId is not implemented for skill " + std::to_string(getSkillId()));
}

void SkillImpl::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_damage(*src, *target, tick, status_get_amotion(src), status_get_dmotion(target), 0,
					  abs(skill_get_num(getSkillId(), skill_lv)), getSkillId(), skill_lv,
					  skill_get_hit(getSkillId()));
	throw SkillImplException("castendDamageId is not implemented for skill " + std::to_string(getSkillId()));
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
