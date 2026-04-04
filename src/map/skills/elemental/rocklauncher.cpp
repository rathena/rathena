// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "rocklauncher.hpp"

#include <common/random.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

// EL_ROCK_CRUSHER
SkillRockLauncher::SkillRockLauncher() : SkillImpl(EL_ROCK_CRUSHER) {
}

void SkillRockLauncher::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src,target, SC_ROCK_CRUSHER,50,skill_lv,skill_get_time(EL_ROCK_CRUSHER,skill_lv));
}

void SkillRockLauncher::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 700;
}

void SkillRockLauncher::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src,*battle_get_master(src),getSkillId(),skill_lv);
	clif_skill_damage( *src, *src, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE );
	if( rnd()%100 < 50 )
		skill_attack(BF_MAGIC,src,src,target,getSkillId(),skill_lv,tick,flag);
	else
		skill_attack(BF_WEAPON,src,src,target,EL_ROCK_CRUSHER_ATK,skill_lv,tick,flag);
}


// EL_ROCK_CRUSHER_ATK
SkillRockLauncherAttack::SkillRockLauncherAttack() : SkillImpl(EL_ROCK_CRUSHER_ATK) {
}

void SkillRockLauncherAttack::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src,target,SC_ROCK_CRUSHER_ATK,50,skill_lv,skill_get_time(EL_ROCK_CRUSHER,skill_lv));
}

void SkillRockLauncherAttack::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 200;
}
