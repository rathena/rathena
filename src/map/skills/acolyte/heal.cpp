// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "heal.hpp"

#include "../../status.hpp"
#include "../../clif.hpp"

SkillHeal::SkillHeal() : SkillImpl(AL_HEAL)
{
}

void SkillHeal::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const
{
	int32 heal = skill_calc_heal(src, bl, skill_id, skill_lv, true);

	if (status_isimmune(bl) || (dstmd && (status_get_class(bl) == MOBID_EMPERIUM || status_get_class_(bl) == CLASS_BATTLEFIELD)))
		heal = 0;

	if (tsc != nullptr && !tsc->empty())
	{
		if (tsc->getSCE(SC_KAITE) && !status_has_mode(sstatus, MD_STATUSIMMUNE))
		{ // Bounce back heal
			if (--tsc->getSCE(SC_KAITE)->val2 <= 0)
				status_change_end(bl, SC_KAITE);
			if (src == bl)
				heal = 0; // When you try to heal yourself under Kaite, the heal is voided.
			else
			{
				bl = src;
				dstsd = sd;
			}
		}
		else if (tsc->getSCE(SC_BERSERK) || tsc->getSCE(SC_SATURDAYNIGHTFEVER))
			heal = 0; // Needed so that it actually displays 0 when healing.
	}
	if (skill_id == AL_HEAL)
		status_change_end(bl, SC_BITESCAR);
	clif_skill_nodamage(src, *bl, skill_id, heal);
	if (tsc && tsc->getSCE(SC_AKAITSUKI) && heal && skill_id != HLIF_HEAL)
		heal = ~heal + 1;
	t_exp heal_get_jobexp = status_heal(bl, heal, 0, 0);

	if (sd && dstsd && heal > 0 && sd != dstsd && battle_config.heal_exp > 0)
	{
		heal_get_jobexp = heal_get_jobexp * battle_config.heal_exp / 100;
		if (heal_get_jobexp <= 0)
			heal_get_jobexp = 1;
		pc_gainexp(sd, bl, 0, heal_get_jobexp, 0);
	}
}

void SkillHeal::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 flag) const
{
	skill_attack(BF_MAGIC, src, src, bl, skill_id, skill_lv, tick, flag);
}
