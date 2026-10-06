// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "lockonlaser.hpp"

SkillLockonLaser::SkillLockonLaser() : SkillImpl(NPC_LOCKON_LASER) {
}

void SkillLockonLaser::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change_start(src, target, skill_get_sc(getSkillId()), 10000, skill_lv, src->id, 0, 0, skill_get_time(getSkillId(), skill_lv), SCSTART_NOAVOID | SCSTART_NOTICKDEF | SCSTART_NORATEDEF);
}

SkillLockonLaserAttack::SkillLockonLaserAttack() : SkillImplRecursiveDamageSplash(NPC_LOCKON_LASER_ATK) {
}

void SkillLockonLaserAttack::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_poseffect(*src, getSkillId(), skill_lv, target->x, target->y, tick);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
