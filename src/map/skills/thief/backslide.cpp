// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "backslide.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillBackSlide::SkillBackSlide() : SkillImpl(TF_BACKSLIDING) {
}

void SkillBackSlide::castendNoDamageId(block_list *src, block_list *bl, uint16 skill_lv, t_tick tick, int32 &flag) const {
	//This is the correct implementation as per packet logging information. [Skotlex]

	// Backsliding makes you immune to being stopped for 200ms, but only if you don't have the endure effect yet
	if (unit_data *ud = unit_bl2ud(bl); ud != nullptr && !status_isendure(*bl, tick, true))
		ud->endure_tick = tick + 200;

#ifdef RENEWAL
	int16 blew_count = skill_blown(src, bl, skill_get_blewcount(getSkillId(), skill_lv), unit_getdir(bl),
	                               static_cast<enum e_skill_blown>(BLOWN_IGNORE_NO_KNOCKBACK | BLOWN_DONT_SEND_PACKET));
	clif_skill_nodamage(src, *bl, getSkillId(), skill_lv);

	if (blew_count > 0)
		clif_blown(src); // Always blow, otherwise it shows a casting animation. [Lemongrass]
#else
	int16 blew_count = skill_blown(src, bl, skill_get_blewcount(getSkillId(), skill_lv), unit_getdir(bl), BLOWN_IGNORE_NO_KNOCKBACK);
	clif_skill_nodamage(src, *bl, getSkillId(), skill_lv);
	clif_slide(*bl, bl->x, bl->y); //Show the casting animation on pre-re
#endif
}
