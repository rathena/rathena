// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "carttornado.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillCartTornado::SkillCartTornado() : SkillImplRecursiveDamageSplash(GN_CART_TORNADO) {
}

void SkillCartTornado::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	const status_change *sc = status_get_sc(src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	// ATK [( Skill Level x 200 ) + ( Cart Weight / ( 150 - Caster Base STR ))] + ( Cart Remodeling Skill Level x 50 )] %
	base_skillratio += -100 + 200 * skill_lv;
	if(sd && sd->cart_weight)
		base_skillratio += sd->cart_weight / 10 / (150 - min(sd->status.str,120)) + pc_checkskill(sd,GN_REMODELING_CART) * 50;
	if (sc && sc->getSCE(SC_BIONIC_WOODENWARRIOR))
		base_skillratio *= 2;
}

void SkillCartTornado::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	skill_castend_damage_id(src, src, getSkillId(), skill_lv, tick, flag);
}

void SkillCartTornado::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd && pc_checkskill(sd, GN_REMODELING_CART))
		hit_rate += pc_checkskill(sd, GN_REMODELING_CART) * 4;
}
