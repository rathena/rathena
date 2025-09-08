#include "skill_gs_cracker.hpp"

SkillGS_CRACKER::SkillGS_CRACKER() : WeaponSkillImpl(GS_CRACKER) {
}

void SkillGS_CRACKER::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	const struct map_session_data *sd = BL_CAST(BL_PC, src);
	const struct map_session_data *dstsd = BL_CAST(BL_PC, bl);
	const struct mob_data *dstmd = BL_CAST(BL_MOB, bl);
	
	/* per official standards, this skill works on players and mobs. */
	if (sd && (dstsd || dstmd)) {
		int32 i = 65 - 5 * distance_bl(src, bl); // Base rate
		if (i < 30)
			i = 30;
		clif_skill_nodamage(src, *bl, GS_CRACKER, skill_lv);
		sc_start(src, bl, SC_STUN, i, skill_lv, skill_get_time2(GS_CRACKER, skill_lv));
	}
}
