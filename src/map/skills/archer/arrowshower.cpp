// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "arrowshower.hpp"

#include "../../status.hpp"

SkillArrowShower::SkillArrowShower() : SkillImpl(AC_SHOWER) {
}

void SkillArrowShower::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
#ifdef RENEWAL
	base_skillratio += 50 + 10 * skill_lv;
#else
	base_skillratio += -25 + 5 * skill_lv;
#endif
}

void SkillArrowShower::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	status_change *tsc = status_get_sc(target);

	if (flag & 1)
    { // Recursive invocation
        int32 sflag = skill_area_temp[0] & 0xFFF;
        std::bitset<INF2_MAX> inf2 = skill_db.find(getSkillId())->inf2;

        if (tsc && tsc->getSCE(SC_HOVERING) && inf2[INF2_IGNOREHOVERING])
            return; // Under Hovering characters are immune to select trap and ground target skills.

        if (flag & SD_LEVEL)
            sflag |= SD_LEVEL; // -1 will be used in packets instead of the skill level
        if (skill_area_temp[1] != target->id && !inf2[INF2_ISNPC])
            sflag |= SD_ANIMATION; // original target gets no animation (as well as all NPC skills)

        skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, sflag);
    }
    else
    {
        int32 starget = BL_CHAR | BL_SKILL, splash_size = skill_get_splash(getSkillId(), skill_lv);

        skill_area_temp[0] = 0;
        skill_area_temp[1] = target->id;
        skill_area_temp[2] = 0;

        // if skill damage should be split among targets, count them
        // SD_LEVEL -> Forced splash damage for Auto Blitz-Beat -> count targets
        // special case: Venom Splasher uses a different range for searching than for splashing
        if (flag & SD_LEVEL || skill_get_nk(getSkillId(), NK_SPLASHSPLIT))
        {
            skill_area_temp[0] = map_foreachinallrange(skill_area_sub, target, (getSkillId() == AS_SPLASHER) ? 1 : splash_size, BL_CHAR, src, getSkillId(), skill_lv, tick, BCT_ENEMY, 1);
            // If there are no characters in the area, then it always counts as if there was one target
            // This happens when targetting skill units such as icewall
            skill_area_temp[0] = std::max(1, skill_area_temp[0]);
        }

        // recursive invocation of skill_castend_damage_id() with flag|1
        map_foreachinrange(skill_area_sub, target, splash_size, starget, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
    }
}
