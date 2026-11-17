// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mirrorimage.hpp"

#include "map/status.hpp"

SkillMirrorImage::SkillMirrorImage() : StatusSkillImpl(NJ_BUNSINJYUTSU) {
}

void SkillMirrorImage::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	// TODO: refactor into status.yml
	status_change_end(target, SC_BUNSINJYUTSU); // on official recasting cancels existing mirror image [helvetica]
	StatusSkillImpl::castendNoDamageId(src, target, skill_lv, tick, flag);
	status_change_end(target, SC_NEN);
}
