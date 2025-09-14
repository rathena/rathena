// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "backslide.hpp"

#include "map/status.hpp"
#include "map/clif.hpp"
#include "map/unit.hpp"

SkillBackSlide::SkillBackSlide() : WeaponSkillImpl(TF_BACKSLIDING) {
}

int32 SkillBackSlide::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	// Backsliding makes you immune to being stopped for 200ms, but only if you don't have the endure effect yet
	if (struct unit_data *ud = unit_bl2ud(bl); ud != nullptr && !status_isendure(*bl, tick, true))
		ud->endure_tick = tick + 200;

	int16 blew_count = skill_blown(src, bl, skill_get_blewcount(this->skill_id, skill_lv), unit_getdir(bl), (enum e_skill_blown) (BLOWN_IGNORE_NO_KNOCKBACK
#ifdef RENEWAL
			                               | BLOWN_DONT_SEND_PACKET
#endif
	                               ));
	clif_skill_nodamage(src, *bl, this->skill_id, skill_lv);
#ifdef RENEWAL
	if (blew_count > 0)
		clif_blown(src); // Always blow, otherwise it shows a casting animation. [Lemongrass]
#else
	clif_slide(*bl, bl->x, bl->y); // Show the casting animation on pre-re
#endif
	return 1;
}
