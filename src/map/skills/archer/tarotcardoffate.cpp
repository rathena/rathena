// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "tarotcardoffate.hpp"

#include <config/core.hpp>
#include <common/random.hpp>

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

int32 skill_tarotcard(block_list* src, block_list* target, uint16 skill_id, uint16 skill_lv, t_tick tick);


SkillTarotCardOfFate::SkillTarotCardOfFate() : SkillImpl(CG_TAROTCARD) {
}

void SkillTarotCardOfFate::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data *sd = BL_CAST(BL_PC, src);
	mob_data *dstmd = BL_CAST(BL_MOB, target);
	status_change *tsc = status_get_sc(target);

	if (tsc && tsc->getSCE(SC_TAROTCARD)) {
		// Target currently has the SUN tarot card effect and is immune to any other effect.
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}

	if (rnd() % 100 > skill_lv * 8 ||
#ifndef RENEWAL
		(tsc && tsc->getSCE(SC_BASILICA)) ||
#endif
		(dstmd && ((dstmd->guardian_data && dstmd->mob_id == MOBID_EMPERIUM) || status_get_class_(target) == CLASS_BATTLEFIELD))) {
		if (sd != nullptr)
			clif_skill_fail(*sd, getSkillId());
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}

	status_zap(src, 0, skill_get_sp(getSkillId(), skill_lv)); // Consume SP only on success.
	int32 card = skill_tarotcard(src, target, getSkillId(), skill_lv, tick); // Actual effect is executed here.
	clif_specialeffect((card == 6) ? src : target, EF_TAROTCARD1 + card - 1, AREA);
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
}


/*========================================== [Playtester]
* Process tarot card's effects
* @param src: Source of the tarot card effect
* @param target: Target of the tartor card effect
* @param skill_id: ID of the skill used
* @param skill_lv: Level of the skill used
* @param tick: Processing tick time
* @return Card number
*------------------------------------------*/
int32 skill_tarotcard(block_list* src, block_list *target, uint16 skill_id, uint16 skill_lv, t_tick tick)
{
	int32 card = 0;

	if (battle_config.tarotcard_equal_chance) {
		//eAthena equal chances
		card = rnd() % 14 + 1;
	}
	else {
		//Official chances
		int32 rate = rnd() % 100;
		if (rate < 10) card = 1; // THE FOOL
		else if (rate < 20) card = 2; // THE MAGICIAN
		else if (rate < 30) card = 3; // THE HIGH PRIESTESS
		else if (rate < 37) card = 4; // THE CHARIOT
		else if (rate < 47) card = 5; // STRENGTH
		else if (rate < 62) card = 6; // THE LOVERS
		else if (rate < 63) card = 7; // WHEEL OF FORTUNE
		else if (rate < 69) card = 8; // THE HANGED MAN
		else if (rate < 74) card = 9; // DEATH
		else if (rate < 82) card = 10; // TEMPERANCE
		else if (rate < 83) card = 11; // THE DEVIL
		else if (rate < 85) card = 12; // THE TOWER
		else if (rate < 90) card = 13; // THE STAR
		else card = 14; // THE SUN
	}

	switch (card) {
	case 1: // THE FOOL - heals SP to 0
	{
		status_percent_damage(src, target, 0, 100, false);
		break;
	}
	case 2: // THE MAGICIAN - matk halved
	{
		sc_start(src, target, SC_INCMATKRATE, 100, -50, skill_get_time2(skill_id, skill_lv));
		break;
	}
	case 3: // THE HIGH PRIESTESS - all buffs removed
	{
		status_change_clear_buffs(target, SCCB_BUFFS | SCCB_CHEM_PROTECT);
		break;
	}
	case 4: // THE CHARIOT - 1000 damage, random armor destroyed
	{
		battle_fix_damage(src, target, 1000, 0, skill_id);
		clif_damage(*src, *target, tick, 0, 0, 1000, 0, DMG_NORMAL, 0, false);
		if (!status_isdead(*target))
		{
			uint16 where[] = { EQP_ARMOR, EQP_SHIELD, EQP_HELM };
			skill_break_equip(src, target, where[rnd() % 3], 10000, BCT_ENEMY);
		}
		break;
	}
	case 5: // STRENGTH - atk halved
	{
		sc_start(src, target, SC_INCATKRATE, 100, -50, skill_get_time2(skill_id, skill_lv));
		break;
	}
	case 6: // THE LOVERS - 2000HP heal, random teleported
	{
		status_heal(target, 2000, 0, 0);
		if (!map_flag_vs(target->m))
			unit_warp(target, -1, -1, -1, CLR_TELEPORT);
		break;
	}
	case 7: // WHEEL OF FORTUNE - random 2 other effects
	{
		// Recursive call
		skill_tarotcard(src, target, skill_id, skill_lv, tick);
		skill_tarotcard(src, target, skill_id, skill_lv, tick);
		break;
	}
	case 8: // THE HANGED MAN - ankle, freeze or stoned
	{
		enum sc_type sc[] = { SC_ANKLE, SC_FREEZE, SC_STONEWAIT };
		uint8 rand_eff = rnd() % 3;
		int32 time = ((rand_eff == 0) ? skill_get_time2(skill_id, skill_lv) : skill_get_time2(status_db.getSkill(sc[rand_eff]), 1));

		if (sc[rand_eff] == SC_STONEWAIT)
			sc_start2(src, target, SC_STONEWAIT, 100, skill_lv, src->id, time, skill_get_time(status_db.getSkill(SC_STONEWAIT), 1));
		else
			sc_start(src, target, sc[rand_eff], 100, skill_lv, time);
		break;
	}
	case 9: // DEATH - curse, coma and poison
	{
		status_change_start(src, target, SC_COMA, 10000, skill_lv, 0, src->id, 0, 0, SCSTART_NONE);
		sc_start(src, target, SC_CURSE, 100, skill_lv, skill_get_time2(status_db.getSkill(SC_CURSE), 1));
		sc_start2(src, target, SC_POISON, 100, skill_lv, src->id, skill_get_time2(status_db.getSkill(SC_POISON), 1));
		break;
	}
	case 10: // TEMPERANCE - confusion
	{
		sc_start(src, target, SC_CONFUSION, 100, skill_lv, skill_get_time2(skill_id, skill_lv));
		break;
	}
	case 11: // THE DEVIL - 6666 damage, atk and matk halved, cursed
	{
		battle_fix_damage(src, target, 6666, 0, skill_id);
		clif_damage(*src, *target, tick, 0, 0, 6666, 0, DMG_NORMAL, 0, false);
		sc_start(src, target, SC_INCATKRATE, 100, -50, skill_get_time2(skill_id, skill_lv));
		sc_start(src, target, SC_INCMATKRATE, 100, -50, skill_get_time2(skill_id, skill_lv));
		sc_start(src, target, SC_CURSE, skill_lv, 100, skill_get_time2(status_db.getSkill(SC_CURSE), 1));
		break;
	}
	case 12: // THE TOWER - 4444 damage
	{
		battle_fix_damage(src, target, 4444, 0, skill_id);
		clif_damage(*src, *target, tick, 0, 0, 4444, 0, DMG_NORMAL, 0, false);
		break;
	}
	case 13: // THE STAR - stun
	{
		sc_start(src, target, SC_STUN, 100, skill_lv, skill_get_time2(status_db.getSkill(SC_STUN), 1));
		break;
	}
	default: // THE SUN - atk, matk, hit, flee and def reduced, immune to more tarot card effects
	{
#ifdef RENEWAL
		//In renewal, this card gives the SC_TAROTCARD status change which makes you immune to other cards
		sc_start(src, target, SC_TAROTCARD, 100, skill_lv, skill_get_time2(skill_id, skill_lv));
#endif
		sc_start(src, target, SC_INCATKRATE, 100, -20, skill_get_time2(skill_id, skill_lv));
		sc_start(src, target, SC_INCMATKRATE, 100, -20, skill_get_time2(skill_id, skill_lv));
		sc_start(src, target, SC_INCHITRATE, 100, -20, skill_get_time2(skill_id, skill_lv));
		sc_start(src, target, SC_INCFLEERATE, 100, -20, skill_get_time2(skill_id, skill_lv));
		sc_start(src, target, SC_INCDEFRATE, 100, -20, skill_get_time2(skill_id, skill_lv));
		return 14; //To make sure a valid number is returned
	}
	}

	return card;
}
