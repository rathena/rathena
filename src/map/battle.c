// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/ers.h"
#include "../common/strlib.h"
#include "../common/utils.h"

#include "map.h"
#include "path.h"
#include "pc.h"
#include "status.h"
#include "skill.h"
#include "homunculus.h"
#include "mercenary.h"
#include "mob.h"
#include "itemdb.h"
#include "clif.h"
#include "pet.h"
#include "guild.h"
#include "party.h"
#include "battle.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int attr_fix_table[4][ELE_MAX][ELE_MAX];

struct Battle_Config battle_config;
static struct eri *delay_damage_ers; //For battle delay damage structures.

int battle_getcurrentskill(struct block_list *bl)
{	//Returns the current/last skill in use by this bl.
	struct unit_data *ud;

	if( bl->type == BL_SKILL )
	{
		struct skill_unit * su = (struct skill_unit*)bl;
		return su->group?su->group->skill_id:0;
	}
	ud = unit_bl2ud(bl);
	return ud?ud->skillid:0;
}

/*==========================================
 * Get random targetting enemy
 *------------------------------------------*/
static int battle_gettargeted_sub(struct block_list *bl, va_list ap)
{
	struct block_list **bl_list;
	struct unit_data *ud;
	int target_id;
	int *c;

	bl_list = va_arg(ap, struct block_list **);
	c = va_arg(ap, int *);
	target_id = va_arg(ap, int);

	if (bl->id == target_id)
		return 0;
	if (*c >= 24)
		return 0;

	ud = unit_bl2ud(bl);
	if (!ud) return 0;

	if (ud->target == target_id || ud->skilltarget == target_id) {
		bl_list[(*c)++] = bl;
		return 1;
	}
	return 0;	
}

struct block_list* battle_gettargeted(struct block_list *target)
{
	struct block_list *bl_list[24];
	int c = 0;
	nullpo_retr(NULL, target);

	memset(bl_list, 0, sizeof(bl_list));
	map_foreachinrange(battle_gettargeted_sub, target, AREA_SIZE, BL_CHAR, bl_list, &c, target->id);
	if (c == 0 || c > 24)
		return NULL;
	return bl_list[rand()%c];
}


//Returns the id of the current targetted character of the passed bl. [Skotlex]
int battle_gettarget(struct block_list* bl)
{
	switch (bl->type)
	{
		case BL_PC:  return ((struct map_session_data*)bl)->ud.target;
		case BL_MOB: return ((struct mob_data*)bl)->target_id;
		case BL_PET: return ((struct pet_data*)bl)->target_id;
		case BL_HOM: return ((struct homun_data*)bl)->ud.target;
		case BL_MER: return ((struct mercenary_data*)bl)->ud.target;
	}
	return 0;
}

static int battle_getenemy_sub(struct block_list *bl, va_list ap)
{
	struct block_list **bl_list;
	struct block_list *target;
	int *c;

	bl_list = va_arg(ap, struct block_list **);
	c = va_arg(ap, int *);
	target = va_arg(ap, struct block_list *);

	if (bl->id == target->id)
		return 0;
	if (*c >= 24)
		return 0;
	if (status_isdead(bl))
		return 0;
	if (battle_check_target(target, bl, BCT_ENEMY) > 0) {
		bl_list[(*c)++] = bl;
		return 1;
	}
	return 0;	
}

// Picks a random enemy of the given type (BL_PC, BL_CHAR, etc) within the range given. [Skotlex]
struct block_list* battle_getenemy(struct block_list *target, int type, int range)
{
	struct block_list *bl_list[24];
	int c = 0;
	memset(bl_list, 0, sizeof(bl_list));
	map_foreachinrange(battle_getenemy_sub, target, range, type, bl_list, &c, target);
	if (c == 0 || c > 24)
		return NULL;
	return bl_list[rand()%c];
}

// ƒ_ƒ??[ƒW‚Ì’x‰„
struct delay_damage {
	struct block_list *src;
	int target;
	int damage;
	int delay;
	unsigned short distance;
	unsigned short skill_lv;
	unsigned short skill_id;
	enum damage_lv dmg_lv;
	unsigned short attack_type;
};

int battle_delay_damage_sub(int tid, unsigned int tick, int id, intptr data)
{
	struct delay_damage *dat = (struct delay_damage *)data;
	struct block_list *target = map_id2bl(dat->target);
	if (target && dat && map_id2bl(id) == dat->src && target->prev != NULL && !status_isdead(target) &&
		target->m == dat->src->m &&
		(target->type != BL_PC || ((TBL_PC*)target)->invincible_timer == -1) &&
		check_distance_bl(dat->src, target, dat->distance)) //Check to see if you haven't teleported. [Skotlex]
	{
		map_freeblock_lock();
		status_fix_damage(dat->src, target, dat->damage, dat->delay);
		if (dat->damage > 0 && dat->attack_type)
		{
			if (!status_isdead(target))
				skill_additional_effect(dat->src,target,dat->skill_id,dat->skill_lv,dat->attack_type,tick);
			skill_counter_additional_effect(dat->src,target,dat->skill_id,dat->skill_lv,dat->attack_type,tick);
		}
		map_freeblock_unlock();
	}
	ers_free(delay_damage_ers, dat);
	return 0;
}

int battle_delay_damage (unsigned int tick, int amotion, struct block_list *src, struct block_list *target, int attack_type, int skill_id, int skill_lv, int damage, enum damage_lv dmg_lv, int ddelay)
{
	struct delay_damage *dat;
	nullpo_retr(0, src);
	nullpo_retr(0, target);

	if (!battle_config.delay_battle_damage) {
		map_freeblock_lock();
		status_fix_damage(src, target, damage, ddelay);
		if (damage > 0 && attack_type)
		{
			if (!status_isdead(target))
				skill_additional_effect(src, target, skill_id, skill_lv, attack_type, gettick());
			skill_counter_additional_effect(src, target, skill_id, skill_lv, attack_type, gettick());
		}
		map_freeblock_unlock();
		return 0;
	}
	dat = ers_alloc(delay_damage_ers, struct delay_damage);
	dat->src = src;
	dat->target = target->id;
	dat->skill_id = skill_id;
	dat->skill_lv = skill_lv;
	dat->attack_type = attack_type;
	dat->damage = damage;
	dat->dmg_lv = dmg_lv;
	dat->delay = ddelay;
	dat->distance = distance_bl(src, target)+10; //Attack should connect regardless unless you teleported.
	if (src->type != BL_PC && amotion > 1000)
		amotion = 1000; //Aegis places a damage-delay cap of 1 sec to non player attacks. [Skotlex]
	add_timer(tick+amotion, battle_delay_damage_sub, src->id, (intptr)dat);
	
	return 0;
}

int battle_attr_ratio(int atk_elem,int def_type, int def_lv)
{
	
	if (atk_elem < 0 || atk_elem >= ELE_MAX)
		return 100;

	if (def_type < 0 || def_type > ELE_MAX || def_lv < 1 || def_lv > 4)
		return 100;

	return attr_fix_table[def_lv-1][atk_elem][def_type];
}

/*==========================================
 * Does attribute fix modifiers. 
 * Added passing of the chars so that the status changes can affect it. [Skotlex]
 * Note: Passing src/target == NULL is perfectly valid, it skips SC_ checks.
 *------------------------------------------*/
int battle_attr_fix(struct block_list *src, struct block_list *target, int damage,int atk_elem,int def_type, int def_lv)
{
	struct status_change *sc=NULL, *tsc=NULL;
	int ratio;
	
	if (src) sc = status_get_sc(src);
	if (target) tsc = status_get_sc(target);
	
	if (atk_elem < 0 || atk_elem >= ELE_MAX)
		atk_elem = rand()%ELE_MAX;

	if (def_type < 0 || def_type > ELE_MAX ||
		def_lv < 1 || def_lv > 4) {
		ShowError("battle_attr_fix: unknown attr type: atk=%d def_type=%d def_lv=%d\n",atk_elem,def_type,def_lv);
		return damage;
	}

	ratio = attr_fix_table[def_lv-1][atk_elem][def_type];
	if (sc && sc->count)
	{
		if(sc->data[SC_VOLCANO] && atk_elem == ELE_FIRE)
			ratio += enchant_eff[sc->data[SC_VOLCANO]->val1-1];
		if(sc->data[SC_VIOLENTGALE] && atk_elem == ELE_WIND)
			ratio += enchant_eff[sc->data[SC_VIOLENTGALE]->val1-1];
		if(sc->data[SC_DELUGE] && atk_elem == ELE_WATER)
			ratio += enchant_eff[sc->data[SC_DELUGE]->val1-1];
	}
	if (tsc && tsc->count)
	{
		if(tsc->data[SC_SPIDERWEB] && atk_elem == ELE_FIRE)
		{	// [Celest]
			damage <<= 1;
			status_change_end(target, SC_SPIDERWEB, -1);
		}
	}
	return damage*ratio/100;
}

/*==========================================
 * ƒ_ƒ??[ƒW?Å?IŒvŽZ
 *------------------------------------------*/
int battle_calc_damage(struct block_list *src,struct block_list *bl,int damage,int div_,int skill_num,int skill_lv,int flag)
{
	struct map_session_data *sd = NULL;
	struct status_change *sc;
	struct status_change_entry *sce;

	nullpo_retr(0, bl);

	if (!damage)
		return 0;
	
	if( mob_ksprotected(src, bl) )
		return 0;

	if (bl->type == BL_PC) {
		sd=(struct map_session_data *)bl;
		//Special no damage states
		if(flag&BF_WEAPON && sd->special_state.no_weapon_damage)
			damage -= damage*sd->special_state.no_weapon_damage/100;

		if(flag&BF_MAGIC && sd->special_state.no_magic_damage)
			damage -= damage*sd->special_state.no_magic_damage/100;

		if(flag&BF_MISC && sd->special_state.no_misc_damage)
			damage -= damage*sd->special_state.no_misc_damage/100;

		if(!damage) return 0;
	}
	
	if (skill_num == PA_PRESSURE)
		return damage; //This skill bypass everything else.

	sc = status_get_sc(bl);

	if( sc && sc->count )
	{
		//First, sc_*'s that reduce damage to 0.
		if( sc->data[SC_BASILICA] && !(status_get_mode(src)&MD_BOSS) && skill_num != PA_PRESSURE )
			return 0; // Basilica reduces damage to 0 except Pressure

		if( sc->data[SC_SAFETYWALL] && (flag&(BF_SHORT|BF_MAGIC))==BF_SHORT )
		{
			struct skill_unit_group* group = skill_id2group(sc->data[SC_SAFETYWALL]->val3);
			if (group) {
				if (--group->val2<=0)
					skill_delunitgroup(NULL,group);
				return 0;
			}
			status_change_end(bl,SC_SAFETYWALL,-1);
		}

		if( sc->data[SC_PNEUMA] && (flag&(BF_MAGIC|BF_LONG)) == BF_LONG )
			return 0;

		if( (sce=sc->data[SC_AUTOGUARD]) && flag&BF_WEAPON && !(skill_get_nk(skill_num)&NK_NO_CARDFIX_ATK) && rand()%100 < sce->val2 )
		{
			int delay;
			clif_skill_nodamage(bl,bl,CR_AUTOGUARD,sce->val1,1);
			// different delay depending on skill level [celest]
			if (sce->val1 <= 5)
				delay = 300;
			else if (sce->val1 > 5 && sce->val1 <= 9)
				delay = 200;
			else
				delay = 100;
			unit_set_walkdelay(bl, gettick(), delay, 1);

			if(sc->data[SC_SHRINK] && rand()%100<5*sce->val1)
				skill_blown(bl,src,skill_get_blewcount(CR_SHRINK,1),-1,0);
			return 0;
		}

		if( (sce=sc->data[SC_PARRYING]) && flag&BF_WEAPON && skill_num != WS_CARTTERMINATION && rand()%100 < sce->val2 )
		{ // attack blocked by Parrying
			clif_skill_nodamage(bl, bl, LK_PARRYING, sce->val1,1);
			return 0;
		}
		
		if(sc->data[SC_DODGE] && !sc->opt1 &&
			(flag&BF_LONG || sc->data[SC_SPURT])
			&& rand()%100 < 20) {
			if (sd && pc_issit(sd)) pc_setstand(sd); //Stand it to dodge.
			clif_skill_nodamage(bl,bl,TK_DODGE,1,1);
			if (!sc->data[SC_COMBO])
				sc_start4(bl, SC_COMBO, 100, TK_JUMPKICK, src->id, 1, 0, 2000);
			return 0;
		}

		if(sc->data[SC_HERMODE] && flag&BF_MAGIC)
			return 0;

		if(sc->data[SC_TATAMIGAESHI] && (flag&(BF_MAGIC|BF_LONG)) == BF_LONG)
			return 0;

		if((sce=sc->data[SC_KAUPE]) && rand()%100 < sce->val2)
		{	//Kaupe blocks damage (skill or otherwise) from players, mobs, homuns, mercenaries.
			clif_specialeffect(bl, 462, AREA);
			//Shouldn't end until Breaker's non-weapon part connects.
			if (skill_num != ASC_BREAKER || !(flag&BF_WEAPON))
				if (--(sce->val3) <= 0) //We make it work like Safety Wall, even though it only blocks 1 time.
					status_change_end(bl, SC_KAUPE, -1);
			return 0;
		}

		if (((sce=sc->data[SC_UTSUSEMI]) || sc->data[SC_BUNSINJYUTSU])
		&& 
			flag&BF_WEAPON && !(skill_get_nk(skill_num)&NK_NO_CARDFIX_ATK))
		{
			if (sce) {
				clif_specialeffect(bl, 462, AREA);
				skill_blown(src,bl,sce->val3,-1,0);
			}
			//Both need to be consumed if they are active.
			if (sce && --(sce->val2) <= 0)
				status_change_end(bl, SC_UTSUSEMI, -1);
			if ((sce=sc->data[SC_BUNSINJYUTSU]) && --(sce->val2) <= 0)
				status_change_end(bl, SC_BUNSINJYUTSU, -1);
			return 0;
		}

		//Now damage increasing effects
		if( sc->data[SC_AETERNA] && skill_num != PF_SOULBURN )
		{
			if( src->type != BL_MER || skill_num == 0 )
				damage <<= 1; // Lex Aeterna only doubles damage of regular attacks from mercenaries

			if( skill_num != ASC_BREAKER || !(flag&BF_WEAPON) )
				status_change_end( bl,SC_AETERNA,-1 ); //Shouldn't end until Breaker's non-weapon part connects.
		}

		//Finally damage reductions....
		if( sc->data[SC_ASSUMPTIO] )
		{
			if( map_flag_vs(bl->m) )
				damage = damage*2/3; //Receive 66% damage
			else
				damage >>= 1; //Receive 50% damage
		}

		if(sc->data[SC_DEFENDER] &&
			(flag&(BF_LONG|BF_WEAPON)) == (BF_LONG|BF_WEAPON))
			damage=damage*(100-sc->data[SC_DEFENDER]->val2)/100;

		if(sc->data[SC_ADJUSTMENT] &&
			(flag&(BF_LONG|BF_WEAPON)) == (BF_LONG|BF_WEAPON))
			damage -= 20*damage/100;

		if(sc->data[SC_FOGWALL]) {
			if(flag&BF_SKILL) //25% reduction
				damage -= 25*damage/100;
			else if ((flag&(BF_LONG|BF_WEAPON)) == (BF_LONG|BF_WEAPON))
				damage >>= 2; //75% reduction
		}

		if((sce=sc->data[SC_ARMOR]) && //NPC_DEFENDER
			sce->val3&flag && sce->val4&flag)
			damage -= damage*sc->data[SC_ARMOR]->val2/100;

		if(sc->data[SC_ENERGYCOAT] && flag&BF_WEAPON
			&& skill_num != WS_CARTTERMINATION)
		{
			struct status_data *status = status_get_status_data(bl);
			int per = 100*status->sp / status->max_sp -1; //100% should be counted as the 80~99% interval
			per /=20; //Uses 20% SP intervals.
			//SP Cost: 1% + 0.5% per every 20% SP
			if (!status_charge(bl, 0, (10+5*per)*status->max_sp/1000))
				status_change_end( bl,SC_ENERGYCOAT,-1 );
			//Reduction: 6% + 6% every 20%
			damage -= damage * 6 * (1+per) / 100;
		}

		if((sce=sc->data[SC_REJECTSWORD]) && flag&BF_WEAPON &&
			// Fixed the condition check [Aalye]
			(src->type!=BL_PC || (
				((TBL_PC *)src)->status.weapon == W_DAGGER ||
				((TBL_PC *)src)->status.weapon == W_1HSWORD ||
				((TBL_PC *)src)->status.weapon == W_2HSWORD
			)) &&
			rand()%100 < sce->val2
		){
			damage = damage*50/100;
			status_fix_damage(bl,src,damage,clif_damage(bl,src,gettick(),0,0,damage,0,0,0));
			clif_skill_nodamage(bl,bl,ST_REJECTSWORD,sce->val1,1);
			if(--(sce->val3)<=0)
				status_change_end(bl, SC_REJECTSWORD, -1);
		}

		//Finally Kyrie because it may, or not, reduce damage to 0.
		if((sce = sc->data[SC_KYRIE]) && damage > 0){
			sce->val2-=damage;
			if(flag&BF_WEAPON || skill_num == TF_THROWSTONE){
				if(sce->val2>=0)
					damage=0;
				else
				  	damage=-sce->val2;
			}
			if((--sce->val3)<=0 || (sce->val2<=0) || skill_num == AL_HOLYLIGHT)
				status_change_end(bl, SC_KYRIE, -1);
		}

		if (!damage) return 0;

		//Probably not the most correct place, but it'll do here
		//(since battle_drain is strictly for players currently)
		if ((sce=sc->data[SC_BLOODLUST]) && flag&BF_WEAPON && damage > 0 &&
			rand()%100 < sce->val3)
			status_heal(src, damage*sce->val4/100, 0, 3);

	}
	//SC effects from caster side. Currently none.
/*	
	sc = status_get_sc(src);
	if (sc && sc->count) {
	}
*/	
	if (battle_config.pk_mode && sd && bl->type == BL_PC && damage)
  	{
		if (flag & BF_SKILL) { //Skills get a different reduction than non-skills. [Skotlex]
			if (flag&BF_WEAPON)
				damage = damage * battle_config.pk_weapon_damage_rate/100;
			if (flag&BF_MAGIC)
				damage = damage * battle_config.pk_magic_damage_rate/100;
			if (flag&BF_MISC)
				damage = damage * battle_config.pk_misc_damage_rate/100;
		} else { //Normal attacks get reductions based on range.
			if (flag & BF_SHORT)
				damage = damage * battle_config.pk_short_damage_rate/100;
			if (flag & BF_LONG)
				damage = damage * battle_config.pk_long_damage_rate/100;
		}
		if(!damage) damage  = 1;
	}

	if(battle_config.skill_min_damage && damage > 0 && damage < div_)
	{
		if ((flag&BF_WEAPON && battle_config.skill_min_damage&1)
			|| (flag&BF_MAGIC && battle_config.skill_min_damage&2)
			|| (flag&BF_MISC && battle_config.skill_min_damage&4)
		)
			damage = div_;
	}

	if( bl->type == BL_MOB && !status_isdead(bl) && src != bl) {
	  if (damage > 0 )
			mobskill_event((TBL_MOB*)bl,src,gettick(),flag);
	  if (skill_num)
			mobskill_event((TBL_MOB*)bl,src,gettick(),MSC_SKILLUSED|(skill_num<<16));
	}

	return damage;
}

/*==========================================
 * Calculates GVG related damage adjustments.
 *------------------------------------------*/
int battle_calc_gvg_damage(struct block_list *src,struct block_list *bl,int damage,int div_,int skill_num,int skill_lv,int flag)
{
	struct mob_data *md = NULL;
	int class_;

	if (!damage) //No reductions to make.
		return 0;
	
	class_ = status_get_class(bl);

	if (bl->type == BL_MOB)
		md=(struct mob_data *)bl;
	
	if(md && md->guardian_data) {
		if(class_ == MOBID_EMPERIUM && flag&BF_SKILL)
		//Skill immunity.
			switch (skill_num) {
			case MO_TRIPLEATTACK:
			case HW_GRAVITATION:
				break;
			default:
				return 0;
		}
		if(src->type != BL_MOB) {
			struct guild *g=guild_search(status_get_guild_id(src));
			if (!g) return 0;
			if (class_ == MOBID_EMPERIUM && guild_checkskill(g,GD_APPROVAL) <= 0)
				return 0;
			if (battle_config.guild_max_castles &&
				guild_checkcastles(g)>=battle_config.guild_max_castles)
				return 0; // [MouseJstr]
		}
	}

	switch (skill_num) {
	//Skills with no damage reduction.
	case PA_PRESSURE:
	case HW_GRAVITATION:
	case NJ_ZENYNAGE:
		break;
	default:
		if (md && md->guardian_data) {
			damage -= damage * (md->guardian_data->castle->defense/100) * battle_config.castle_defense_rate/100;
		}
		if (flag & BF_SKILL) { //Skills get a different reduction than non-skills. [Skotlex]
			if (flag&BF_WEAPON)
				damage = damage * battle_config.gvg_weapon_damage_rate/100;
			if (flag&BF_MAGIC)
				damage = damage * battle_config.gvg_magic_damage_rate/100;
			if (flag&BF_MISC)
				damage = damage * battle_config.gvg_misc_damage_rate/100;
		} else { //Normal attacks get reductions based on range.
			if (flag & BF_SHORT)
				damage = damage * battle_config.gvg_short_damage_rate/100;
			if (flag & BF_LONG)
				damage = damage * battle_config.gvg_long_damage_rate/100;
		}
		if(!damage) damage  = 1;
	}
	return damage;
}

/*==========================================
 * HP/SP‹zŽû‚ÌŒvŽZ
 *------------------------------------------*/
static int battle_calc_drain(int damage, int rate, int per)
{
	int diff = 0;

	if (per && rand()%1000 < rate) {
		diff = (damage * per) / 100;
		if (diff == 0) {
			if (per > 0)
				diff = 1;
			else
				diff = -1;
		}
	}
	return diff;
}

/*==========================================
 * ?C—ûƒ_ƒ??[ƒW
 *------------------------------------------*/
int battle_addmastery(struct map_session_data *sd,struct block_list *target,int dmg,int type)
{
	int damage,skill;
	struct status_data *status = status_get_status_data(target);
	int weapon;
	damage = dmg;

	nullpo_retr(0, sd);

	if((skill = pc_checkskill(sd,AL_DEMONBANE)) > 0 &&
		target->type == BL_MOB && //This bonus doesnt work against players.
		(battle_check_undead(status->race,status->def_ele) || status->race==RC_DEMON) )
		damage += (skill*(int)(3+(sd->status.base_level+1)*0.05));	// submitted by orn
		//damage += (skill * 3);

	if((skill = pc_checkskill(sd,HT_BEASTBANE)) > 0 && (status->race==RC_BRUTE || status->race==RC_INSECT) ) {
		damage += (skill * 4);
		if (sd->sc.data[SC_SPIRIT] && sd->sc.data[SC_SPIRIT]->val2 == SL_HUNTER)
			damage += sd->status.str;
	}

	if(type == 0)
		weapon = sd->weapontype1;
	else
		weapon = sd->weapontype2;
	switch(weapon)
	{
		case W_DAGGER:
		case W_1HSWORD:
			if((skill = pc_checkskill(sd,SM_SWORD)) > 0)
				damage += (skill * 4);
			break;
		case W_2HSWORD:
			if((skill = pc_checkskill(sd,SM_TWOHAND)) > 0)
				damage += (skill * 4);
			break;
		case W_1HSPEAR:
		case W_2HSPEAR:
			if((skill = pc_checkskill(sd,KN_SPEARMASTERY)) > 0) {
				if(!pc_isriding(sd))
					damage += (skill * 4);
				else
					damage += (skill * 5);
			}
			break;
		case W_1HAXE:
		case W_2HAXE:
			if((skill = pc_checkskill(sd,AM_AXEMASTERY)) > 0)
				damage += (skill * 3);
			break;
		case W_MACE:
		case W_2HMACE:
			if((skill = pc_checkskill(sd,PR_MACEMASTERY)) > 0)
				damage += (skill * 3);
			break;
		case W_FIST:
			if((skill = pc_checkskill(sd,TK_RUN)) > 0)
				damage += (skill * 10);
			// No break, fallthrough to Knuckles
		case W_KNUCKLE:
			if((skill = pc_checkskill(sd,MO_IRONHAND)) > 0)
				damage += (skill * 3);
			break;
		case W_MUSICAL:
			if((skill = pc_checkskill(sd,BA_MUSICALLESSON)) > 0)
				damage += (skill * 3);
			break;
		case W_WHIP:
			if((skill = pc_checkskill(sd,DC_DANCINGLESSON)) > 0)
				damage += (skill * 3);
			break;
		case W_BOOK:
			if((skill = pc_checkskill(sd,SA_ADVANCEDBOOK)) > 0)
				damage += (skill * 3);
			break;
		case W_KATAR:
			if((skill = pc_checkskill(sd,AS_KATAR)) > 0)
				damage += (skill * 3);
			break;
	}

	return damage;
}
/*==========================================
 * Calculates the standard damage of a normal attack assuming it hits,
 * it calculates nothing extra fancy, is needed for magnum break's WATK_ELEMENT bonus. [Skotlex]
 *------------------------------------------
 * Pass damage2 as NULL to not calc it.
 * Flag values:
 * &1: Critical hit
 * &2: Arrow attack
 * &4: Skill is Magic Crasher
 * &8: Skip target size adjustment (Extremity Fist?)
 *&16: Arrow attack but BOW, REVOLVER, RIFLE, SHOTGUN, GATLING or GRENADE type weapon not equipped (i.e. shuriken, kunai and venom knives not affected by DEX)
 */
static int battle_calc_base_damage(struct status_data *status, struct weapon_atk *wa, struct status_change *sc, unsigned short t_size, struct map_session_data *sd, int flag)
{
	unsigned short atkmin=0, atkmax=0;
	short type = 0;
	int damage = 0;

	if (!sd)
	{	//Mobs/Pets
		if(flag&4)
		{		  
			atkmin = status->matk_min;
			atkmax = status->matk_max;
		} else {
			atkmin = wa->atk;
			atkmax = wa->atk2;
		}
		if (atkmin > atkmax)
			atkmin = atkmax;
	} else {	//PCs
		atkmax = wa->atk;
		type = (wa == &status->lhw)?EQI_HAND_L:EQI_HAND_R;

		if (!(flag&1) || (flag&2))
		{	//Normal attacks
			atkmin = status->dex;
			
			if (sd->equip_index[type] >= 0 && sd->inventory_data[sd->equip_index[type]])
				atkmin = atkmin*(80 + sd->inventory_data[sd->equip_index[type]]->wlv*20)/100;

			if (atkmin > atkmax)
				atkmin = atkmax;
			
			if(flag&2 && !(flag&16))
			{	//Bows
				atkmin = atkmin*atkmax/100;
				if (atkmin > atkmax)
					atkmax = atkmin;
			}
		}
	}
	
	if (sc && sc->data[SC_MAXIMIZEPOWER])
		atkmin = atkmax;
	
	//Weapon Damage calculation
	if (!(flag&1))
		damage = (atkmax>atkmin? rand()%(atkmax-atkmin):0)+atkmin;
	else 
		damage = atkmax;
	
	if (sd)
	{
		//rodatazone says the range is 0~arrow_atk-1 for non crit
		if (flag&2 && sd->arrow_atk)
			damage += ((flag&1)?sd->arrow_atk:rand()%sd->arrow_atk);

		//SizeFix only for players
		if (!(sd->special_state.no_sizefix || (flag&8)))
			damage = damage*(type==EQI_HAND_L?
				sd->left_weapon.atkmods[t_size]:
				sd->right_weapon.atkmods[t_size])/100;
	}
	
	//Finally, add baseatk
	if(flag&4)
		damage += status->matk_min;
	else
		damage += status->batk;
	
	//rodatazone says that Overrefine bonuses are part of baseatk
	//Here we also apply the weapon_atk_rate bonus so it is correctly applied on left/right hands.
	if(sd) {
		if (type == EQI_HAND_L) {
			if(sd->left_weapon.overrefine)
				damage += rand()%sd->left_weapon.overrefine+1;
			if (sd->weapon_atk_rate[sd->weapontype2])
				damage += damage*sd->weapon_atk_rate[sd->weapontype2]/100;;
		} else { //Right hand
			if(sd->right_weapon.overrefine)
				damage += rand()%sd->right_weapon.overrefine+1;
			if (sd->weapon_atk_rate[sd->weapontype1])
				damage += damage*sd->weapon_atk_rate[sd->weapontype1]/100;;
		}
	}
	return damage;
}

/*==========================================
 * Consumes ammo for the given skill.
 *------------------------------------------*/
void battle_consume_ammo(TBL_PC*sd, int skill, int lv)
{
	int qty=1;
	if (!battle_config.arrow_decrement)
		return;
	
	if (skill)
	{
		qty = skill_get_ammo_qty(skill, lv);
		if (!qty) qty = 1;
	}

	if(sd->equip_index[EQI_AMMO]>=0) //Qty check should have been done in skill_check_condition
		pc_delitem(sd,sd->equip_index[EQI_AMMO],qty,0);
}

static int battle_range_type(
	struct block_list *src, struct block_list *target,
	int skill_num, int skill_lv)
{	//Skill Range Criteria
	if (battle_config.skillrange_by_distance &&
		(src->type&battle_config.skillrange_by_distance)
	) { //based on distance between src/target [Skotlex]
		if (check_distance_bl(src, target, 5))
			return BF_SHORT;
		return BF_LONG;
	}
	//based on used skill's range
	if (skill_get_range2(src, skill_num, skill_lv) < 5)
		return BF_SHORT;
	return BF_LONG;
}

static int battle_blewcount_bonus(struct map_session_data *sd, int skill_num)
{
	int i;
	if (!sd->skillblown[0].id)
		return 0;
	//Apply the bonus blewcount. [Skotlex]
	for (i = 0; i < ARRAYLENGTH(sd->skillblown) && sd->skillblown[i].id; i++) {
		if (sd->skillblown[i].id == skill_num)
			return sd->skillblown[i].val;
	}
	return 0;
}

struct Damage battle_calc_magic_attack(struct block_list *src,struct block_list *target,int skill_num,int skill_lv,int mflag);
struct Damage battle_calc_misc_attack(struct block_list *src,struct block_list *target,int skill_num,int skill_lv,int mflag);

//For quick div adjustment.
#define damage_div_fix(dmg, div) { if (div > 1) (dmg)*=div; else if (div < 0) (div)*=-1; }
/*==========================================
 * battle_calc_weapon_attack (by Skotlex)
 *------------------------------------------*/
static struct Damage battle_calc_weapon_attack(struct block_list *src,struct block_list *target,int skill_num,int skill_lv,int wflag)
{
	unsigned int skillratio = 100;	//Skill dmg modifiers.
	short skill=0;
	short s_ele, s_ele_, t_class;
	int i, nk;

	struct map_session_data *sd, *tsd;
	struct Damage wd;
	struct status_change *sc = status_get_sc(src);
	struct status_change *tsc = status_get_sc(target);
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);
	struct {
		unsigned hit : 1; //the attack Hit? (not a miss)
		unsigned cri : 1;		//Critical hit
		unsigned idef : 1;	//Ignore defense
		unsigned idef2 : 1;	//Ignore defense (left weapon)
		unsigned pdef : 2;	//Pierces defense (Investigate/Ice Pick)
		unsigned pdef2 : 2;	//1: Use def+def2/100, 2: Use def+def2/50	
		unsigned infdef : 1;	//Infinite defense (plants)
		unsigned arrow : 1;	//Attack is arrow-based
		unsigned rh : 1;		//Attack considers right hand (wd.damage)
		unsigned lh : 1;		//Attack considers left hand (wd.damage2)
		unsigned weapon : 1; //It's a weapon attack (consider VVS, and all that)
	}	flag;	

	memset(&wd,0,sizeof(wd));
	memset(&flag,0,sizeof(flag));

	if(src==NULL || target==NULL)
	{
		nullpo_info(NLP_MARK);
		return wd;
	}
	//Initial flag
	flag.rh=1;
	flag.weapon=1;
	flag.infdef=(tstatus->mode&MD_PLANT?1:0);

	//Initial Values
	wd.type=0; //Normal attack
	wd.div_=skill_num?skill_get_num(skill_num,skill_lv):1;
	wd.amotion=(skill_num && skill_get_inf(skill_num)&INF_GROUND_SKILL)?0:sstatus->amotion; //Amotion should be 0 for ground skills.
	if(skill_num == KN_AUTOCOUNTER)
		wd.amotion >>= 1;
	wd.dmotion=tstatus->dmotion;
	wd.blewcount=skill_get_blewcount(skill_num,skill_lv);
	wd.flag = BF_WEAPON; //Initial Flag
	wd.flag|= skill_num?BF_SKILL:BF_NORMAL;
	wd.dmg_lv=ATK_DEF;	//This assumption simplifies the assignation later
	nk = skill_get_nk(skill_num);
	flag.hit	= nk&NK_IGNORE_FLEE?1:0;
	flag.idef = flag.idef2 = nk&NK_IGNORE_DEF?1:0;

	if (sc && !sc->count)
		sc = NULL; //Skip checking as there are no status changes active.
	if (tsc && !tsc->count)
		tsc = NULL; //Skip checking as there are no status changes active.

	sd = BL_CAST(BL_PC, src);
	tsd = BL_CAST(BL_PC, target);

	if(sd)
		wd.blewcount += battle_blewcount_bonus(sd, skill_num);

	//Set miscellaneous data that needs be filled regardless of hit/miss
	if(
		(sd && sd->state.arrow_atk) ||
		(!sd && ((skill_num && skill_get_ammotype(skill_num)) || sstatus->rhw.range>3))
	)
		flag.arrow = 1;
	
	if(skill_num){
		wd.flag |= battle_range_type(src, target, skill_num, skill_lv);
		switch(skill_num)
		{
			case MO_FINGEROFFENSIVE:
				if(sd) {
					if (battle_config.finger_offensive_type)
						wd.div_ = 1;
					else
						wd.div_ = sd->spiritball_old;
				}
				break;
			case HT_PHANTASMIC:
				//Since these do not consume ammo, they need to be explicitly set as arrow attacks.
				flag.arrow = 1;
				break;

			case CR_SHIELDBOOMERANG:
			case PA_SHIELDCHAIN:
				flag.weapon = 0;
				break;

			case KN_PIERCE:
			case ML_PIERCE:
				wd.div_= (wd.div_>0?tstatus->size+1:-(tstatus->size+1));
				break;

			case TF_DOUBLE: //For NPC used skill.
			case GS_CHAINACTION:
				wd.type = 0x08;
				break;
				
			case GS_GROUNDDRIFT:
			case KN_SPEARSTAB:
			case KN_BOWLINGBASH:
			case MS_BOWLINGBASH:
			case MO_BALKYOUNG:
			case TK_TURNKICK:
				wd.blewcount=0;
				break;

			case KN_AUTOCOUNTER:
				wd.flag=(wd.flag&~BF_SKILLMASK)|BF_NORMAL;
				break;

			case NPC_CRITICALSLASH:
				flag.cri = 1; //Always critical skill.
				break;
		}
	} else //Range for normal attacks.
		wd.flag |= flag.arrow?BF_LONG:BF_SHORT;
	
	if (!skill_num && tstatus->flee2 && rand()%1000 < tstatus->flee2)
	{	//Check for Lucky Dodge
		wd.type=0x0b;
		wd.dmg_lv=ATK_LUCKY;
		if (wd.div_ < 0) wd.div_*=-1;
		return wd;
	}

	t_class = status_get_class(target);
	s_ele = s_ele_ = skill_get_ele(skill_num, skill_lv);
	if (!skill_num || s_ele == -1) { //Take weapon's element
		s_ele = sstatus->rhw.ele;
		s_ele_ = sstatus->lhw.ele;
		if (flag.arrow && sd && sd->arrow_ele)
			s_ele = sd->arrow_ele;
	} else if (s_ele == -2) { //Use enchantment's element
		s_ele = s_ele_ = status_get_attack_sc_element(src,sc);
	}
	if (skill_num == GS_GROUNDDRIFT)
		s_ele = s_ele_ = wflag; //element comes in flag.

	if(!skill_num)
  	{	//Skills ALWAYS use ONLY your right-hand weapon (tested on Aegis 10.2)
		if (sd && sd->weapontype1 == 0 && sd->weapontype2 > 0)
		{
			flag.rh=0;
			flag.lh=1;
		}
		if (sstatus->lhw.atk)
			flag.lh=1;
	}

	//Check for critical
	if(!flag.cri && sstatus->cri &&
		(!skill_num ||
		skill_num == KN_AUTOCOUNTER ||
		skill_num == SN_SHARPSHOOTING || skill_num == MA_SHARPSHOOTING ||
		skill_num == NJ_KIRIKAGE))
	{
		short cri = sstatus->cri;
		if (sd)
		{
			cri+= sd->critaddrace[tstatus->race];
			if(flag.arrow)
				cri += sd->arrow_cri;
			if(sd->status.weapon == W_KATAR)
				cri <<=1;
		}
		//The official equation is *2, but that only applies when sd's do critical.
		//Therefore, we use the old value 3 on cases when an sd gets attacked by a mob
		cri -= tstatus->luk*(!sd&&tsd?3:2);
		
		if(tsc)
		{
			if (tsc->data[SC_SLEEP])
				cri <<=1;
		}
		switch (skill_num)
		{
			case KN_AUTOCOUNTER:
				if(battle_config.auto_counter_type &&
					(battle_config.auto_counter_type&src->type))
					flag.cri = 1;
				else
					cri <<= 1;
				break;
			case SN_SHARPSHOOTING:
			case MA_SHARPSHOOTING:
				cri += 200;
				break;
			case NJ_KIRIKAGE:
				cri += 250 + 50*skill_lv;
				break;
		}
		if(tsd && tsd->critical_def)
			cri = cri*(100-tsd->critical_def)/100;
		if (rand()%1000 < cri)
			flag.cri= 1;
	}
	if (flag.cri)
	{
		wd.type = 0x0a;
		flag.idef = flag.idef2 = flag.hit = 1;
	} else {	//Check for Perfect Hit
		if(sd && sd->perfect_hit > 0 && rand()%100 < sd->perfect_hit)
			flag.hit = 1;
		if (sc && sc->data[SC_FUSION]) {
			flag.hit = 1; //SG_FUSION always hit [Komurka]
			flag.idef = flag.idef2 = 1; //def ignore [Komurka]
		}
		if (skill_num && !flag.hit)
			switch(skill_num)
			{
				case AS_SPLASHER:
					if (wflag) // Always hits the one exploding.
						break;
					flag.hit = 1;
					break;
				case CR_SHIELDBOOMERANG:
					if (sc && sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_CRUSADER)
						flag.hit = 1;
					break;
				case 0:
					//If flag, this is splash damage from Baphomet Card and it always hits.
					if (wflag)
						flag.hit = 1;
					break;
			}
		if (tsc && !flag.hit && tsc->opt1 && tsc->opt1 != OPT1_STONEWAIT)
			flag.hit = 1;
	}

	if (!flag.hit)
	{	//Hit/Flee calculation
		short
			flee = tstatus->flee,
			hitrate=80; //Default hitrate

		if(battle_config.agi_penalty_type &&
			battle_config.agi_penalty_target&target->type)
		{	
			unsigned char attacker_count; //256 max targets should be a sane max
			attacker_count = unit_counttargeted(target,battle_config.agi_penalty_count_lv);
			if(attacker_count >= battle_config.agi_penalty_count)
			{
				if (battle_config.agi_penalty_type == 1)
					flee = (flee * (100 - (attacker_count - (battle_config.agi_penalty_count - 1))*battle_config.agi_penalty_num))/100;
				else //asume type 2: absolute reduction
					flee -= (attacker_count - (battle_config.agi_penalty_count - 1))*battle_config.agi_penalty_num;
				if(flee < 1) flee = 1;
			}
		}

		hitrate+= sstatus->hit - flee;

		if(wd.flag&BF_LONG && !skill_num && //Fogwall's hit penalty is only for normal ranged attacks.
			tsc && tsc->data[SC_FOGWALL])
			hitrate -= 50;

		if(sd && flag.arrow)
			hitrate += sd->arrow_hit;
		if(skill_num)
			switch(skill_num)
		{	//Hit skill modifiers
			//It is proven that bonus is applied on final hitrate, not hit.
			case SM_BASH:
			case MS_BASH:
				hitrate += hitrate * 5 * skill_lv / 100;
				break;
			case MS_MAGNUM:
			case SM_MAGNUM:
				hitrate += hitrate * 10 * skill_lv / 100;
				break;
			case KN_AUTOCOUNTER:
			case PA_SHIELDCHAIN:
			case NPC_WATERATTACK:
			case NPC_GROUNDATTACK:
			case NPC_FIREATTACK:
			case NPC_WINDATTACK:
			case NPC_POISONATTACK:
			case NPC_HOLYATTACK:
			case NPC_DARKNESSATTACK:
			case NPC_UNDEADATTACK:
			case NPC_TELEKINESISATTACK:
			case NPC_BLEEDING:
				hitrate += hitrate * 20 / 100;
				break;
			case KN_PIERCE:
			case ML_PIERCE:
				hitrate += hitrate * 5 * skill_lv / 100;
				break;
			case AS_SONICBLOW:
				if(sd && pc_checkskill(sd,AS_SONICACCEL)>0)
					hitrate += hitrate * 50 / 100;
				break;
		}

		// Weaponry Research hidden bonus
		if (sd && (skill = pc_checkskill(sd,BS_WEAPONRESEARCH)) > 0)
			hitrate += hitrate * ( 2 * skill ) / 100;

		hitrate = cap_value(hitrate, battle_config.min_hitrate, battle_config.max_hitrate); 

		if(rand()%100 >= hitrate)
			wd.dmg_lv = ATK_FLEE;
		else
			flag.hit = 1;
	}	//End hit/miss calculation

	if (flag.hit && !flag.infdef) //No need to do the math for plants
	{	//Hitting attack

//Assuming that 99% of the cases we will not need to check for the flag.rh... we don't.
//ATK_RATE scales the damage. 100 = no change. 50 is halved, 200 is doubled, etc
#define ATK_RATE( a ) { wd.damage= wd.damage*(a)/100 ; if(flag.lh) wd.damage2= wd.damage2*(a)/100; }
#define ATK_RATE2( a , b ) { wd.damage= wd.damage*(a)/100 ; if(flag.lh) wd.damage2= wd.damage2*(b)/100; }
//Adds dmg%. 100 = +100% (double) damage. 10 = +10% damage
#define ATK_ADDRATE( a ) { wd.damage+= wd.damage*(a)/100 ; if(flag.lh) wd.damage2+= wd.damage2*(a)/100; }
#define ATK_ADDRATE2( a , b ) { wd.damage+= wd.damage*(a)/100 ; if(flag.lh) wd.damage2+= wd.damage2*(b)/100; }
//Adds an absolute value to damage. 100 = +100 damage
#define ATK_ADD( a ) { wd.damage+= a; if (flag.lh) wd.damage2+= a; }
#define ATK_ADD2( a , b ) { wd.damage+= a; if (flag.lh) wd.damage2+= b; }

		switch (skill_num)
		{	//Calc base damage according to skill
			case NJ_ISSEN:
				wd.damage = 40*sstatus->str +skill_lv*(sstatus->hp/10 + 35);
				wd.damage2 = 0;
				status_set_hp(src, 1, 0);
				break;
			case PA_SACRIFICE:
				wd.damage = sstatus->max_hp* 9/100;
				status_zap(src, wd.damage, 0);//Damage to self is always 9%
				wd.damage2 = 0;

				if (sc && sc->data[SC_SACRIFICE])
				{
					if (--sc->data[SC_SACRIFICE]->val2 <= 0)
						status_change_end(src, SC_SACRIFICE,-1);
				}
				break;
			case LK_SPIRALPIERCE:
			case ML_SPIRALPIERCE:
				if (sd) {
					short index = sd->equip_index[EQI_HAND_R];

					if (index >= 0 &&
						sd->inventory_data[index] &&
						sd->inventory_data[index]->type == IT_WEAPON)
						wd.damage = sd->inventory_data[index]->weight*8/100; //80% of weight
				} else
					wd.damage = sstatus->rhw.atk2*8/10; //Else use Atk2

				ATK_ADDRATE(50*skill_lv); //Skill modifier applies to weight only.
				i = sstatus->str/10;
				i*=i;
				ATK_ADD(i); //Add str bonus.
				switch (tstatus->size) { //Size-fix. Is this modified by weapon perfection?
					case 0: //Small: 125%
						ATK_RATE(125);
						break;
					//case 1: //Medium: 100%
					case 2: //Large: 75%
						ATK_RATE(75);
						break;
				}
				break;
			case CR_SHIELDBOOMERANG:
			case PA_SHIELDCHAIN:
				wd.damage = sstatus->batk;
				if (sd) {
					short index = sd->equip_index[EQI_HAND_L];

					if (index >= 0 &&
						sd->inventory_data[index] &&
						sd->inventory_data[index]->type == IT_ARMOR)
						ATK_ADD(sd->inventory_data[index]->weight/10);
					break;
				} else
					ATK_ADD(sstatus->rhw.atk2); //Else use Atk2
				break;
			case HFLI_SBR44:	//[orn]
				if(src->type == BL_HOM) {
					wd.damage = ((TBL_HOM*)src)->homunculus.intimacy ;
					break;
				}
			default:
			{
				i = (flag.cri?1:0)|
					(flag.arrow?2:0)|
					(skill_num == HW_MAGICCRASHER?4:0)|
					(!skill_num && sc && sc->data[SC_CHANGE]?4:0)|
					(skill_num == MO_EXTREMITYFIST?8:0)|
					(sc && sc->data[SC_WEAPONPERFECTION]?8:0);
				if (flag.arrow && sd)
				switch(sd->status.weapon) {
					case W_BOW:
					case W_REVOLVER:
					case W_GATLING:
					case W_SHOTGUN:
					case W_GRENADE:
						break;
					default:
						i |= 16; // for ex. shuriken must not be influenced by DEX
				}
				wd.damage = battle_calc_base_damage(sstatus, &sstatus->rhw, sc, tstatus->size, sd, i);
				if (flag.lh)
					wd.damage2 = battle_calc_base_damage(sstatus, &sstatus->lhw, sc, tstatus->size, sd, i);

				if (nk&NK_SPLASHSPLIT){ // Divide ATK among targets
					if(wflag>0)
						wd.damage/= wflag;
					else
						ShowError("0 enemies targeted by %d:%s, divide per 0 avoided!\n", skill_num, skill_get_name(skill_num));
				}

				//Add any bonuses that modify the base baseatk+watk (pre-skills)
				if(sd)
				{
					if (sd->atk_rate != 100)
						ATK_RATE(sd->atk_rate);

					if(flag.cri && sd->crit_atk_rate)
						ATK_ADDRATE(sd->crit_atk_rate);

					if(sd->status.party_id && (skill=pc_checkskill(sd,TK_POWER)) > 0){
						i = party_foreachsamemap(party_sub_count, sd, 0);
						ATK_ADDRATE(2*skill*i);
					}
				}
				break;
			}	//End default case
		} //End switch(skill_num)

		//Skill damage modifiers that stack linearly
		if(sc && skill_num != PA_SACRIFICE)
		{
			if(sc->data[SC_OVERTHRUST])
				skillratio += sc->data[SC_OVERTHRUST]->val3;
			if(sc->data[SC_MAXOVERTHRUST])
				skillratio += sc->data[SC_MAXOVERTHRUST]->val2;
			if(sc->data[SC_BERSERK])
				skillratio += 100;
		}
		if( !skill_num )
		{ // Random chance to deal multiplied damage - Consider it as part of skill-based-damage
			if( sd && sd->random_attack_increase_add > 0 && sd->random_attack_increase_per && rand()%100 < sd->random_attack_increase_per )
				skillratio += sd->random_attack_increase_add;

			ATK_RATE(skillratio);
		}
		else
		{
			switch( skill_num )
			{
				case SM_BASH:
				case MS_BASH:
					skillratio += 30*skill_lv;
					break;
				case SM_MAGNUM:
				case MS_MAGNUM:
					skillratio += 20*skill_lv; 
					break;
				case MC_MAMMONITE:
					skillratio += 50*skill_lv;
					break;
				case HT_POWER: //FIXME: How exactly is the STR based damage supposed to be done? [Skotlex]
					skillratio += 5*sstatus->str;
					break;
				case AC_DOUBLE:
				case MA_DOUBLE:
					skillratio += 10*(skill_lv-1);
					break;
				case AC_SHOWER:
				case MA_SHOWER:
					skillratio += 5*skill_lv-25;
					break;
				case AC_CHARGEARROW:
				case MA_CHARGEARROW:
					skillratio += 50;
					break;
				case HT_FREEZINGTRAP:
				case MA_FREEZINGTRAP:
					skillratio += -50+10*skill_lv;
					break;
				case KN_PIERCE:
				case ML_PIERCE:
					skillratio += 10*skill_lv;
					break;
				case MER_CRASH:
					skillratio += 10*skill_lv;
					break;
				case KN_SPEARSTAB:
					skillratio += 15*skill_lv;
					break;
				case KN_SPEARBOOMERANG:
					skillratio += 50*skill_lv;
					break;
				case KN_BRANDISHSPEAR:
				case ML_BRANDISH:
				{
					int ratio = 100+20*skill_lv;
					skillratio += ratio-100;
					if(skill_lv>3 && wflag==1) skillratio += ratio/2;
					if(skill_lv>6 && wflag==1) skillratio += ratio/4;
					if(skill_lv>9 && wflag==1) skillratio += ratio/8;
					if(skill_lv>6 && wflag==2) skillratio += ratio/2;
					if(skill_lv>9 && wflag==2) skillratio += ratio/4;
					if(skill_lv>9 && wflag==3) skillratio += ratio/2;
					break;
				}
				case KN_BOWLINGBASH:
				case MS_BOWLINGBASH:
					skillratio+= 40*skill_lv;
					break;
				case AS_GRIMTOOTH:
					skillratio += 20*skill_lv;
					break;
				case AS_POISONREACT:
					skillratio += 30*skill_lv;
					break;
				case AS_SONICBLOW:
					skillratio += -50+5*skill_lv;
					break;
				case TF_SPRINKLESAND:
					skillratio += 30;
					break;
				case MC_CARTREVOLUTION:
					skillratio += 50;
					if(sd && sd->cart_weight)
						skillratio += 100*sd->cart_weight/battle_config.max_cart_weight; // +1% every 1% weight
					else if (!sd)
						skillratio += 100; //Max damage for non players.
					break;
				case NPC_RANDOMATTACK:
					skillratio += rand()%150-50;
					break;
				case NPC_WATERATTACK:
				case NPC_GROUNDATTACK:
				case NPC_FIREATTACK:
				case NPC_WINDATTACK:
				case NPC_POISONATTACK:
				case NPC_HOLYATTACK:
				case NPC_DARKNESSATTACK:
				case NPC_UNDEADATTACK:
				case NPC_TELEKINESISATTACK:
				case NPC_BLOODDRAIN:
				case NPC_ACIDBREATH:
				case NPC_DARKNESSBREATH:
				case NPC_FIREBREATH:
				case NPC_ICEBREATH:
				case NPC_THUNDERBREATH:
				case NPC_HELLJUDGEMENT:
				case NPC_PULSESTRIKE:
					skillratio += 100*(skill_lv-1);
					break;
				case RG_BACKSTAP:
					if(sd && sd->status.weapon == W_BOW && battle_config.backstab_bow_penalty)
						skillratio += (200+40*skill_lv)/2;
					else
						skillratio += 200+40*skill_lv;
					break;
				case RG_RAID:
					skillratio += 40*skill_lv;
					break;
				case RG_INTIMIDATE:
					skillratio += 30*skill_lv;
					break;
				case CR_SHIELDCHARGE:
					skillratio += 20*skill_lv;
					break;
				case CR_SHIELDBOOMERANG:
					skillratio += 30*skill_lv;
					break;
				case NPC_DARKCROSS:
				case CR_HOLYCROSS:
					skillratio += 35*skill_lv;
					break;
				case AM_DEMONSTRATION:
					skillratio += 20*skill_lv;
					break;
				case AM_ACIDTERROR:
					skillratio += 40*skill_lv;
					break;
				case MO_FINGEROFFENSIVE:
					skillratio+= 50 * skill_lv;
					break;
				case MO_INVESTIGATE:
					skillratio += 75*skill_lv;
					flag.pdef = flag.pdef2 = 2;
					break;
				case MO_EXTREMITYFIST:
					{	//Overflow check. [Skotlex]
						unsigned int ratio = skillratio + 100*(8 + sstatus->sp/10);
						//You'd need something like 6K SP to reach this max, so should be fine for most purposes.
						if (ratio > 60000) ratio = 60000; //We leave some room here in case skillratio gets further increased.
						skillratio = (unsigned short)ratio;
						status_set_sp(src, 0, 0);
					}
					break;
				case MO_TRIPLEATTACK:
					skillratio += 20*skill_lv;
					break;
				case MO_CHAINCOMBO:
					skillratio += 50+50*skill_lv;
					break;
				case MO_COMBOFINISH:
					skillratio += 140+60*skill_lv;
					break;
				case BA_MUSICALSTRIKE:
				case DC_THROWARROW:
					skillratio += 25+25*skill_lv;
					break;
				case CH_TIGERFIST:
					skillratio += 100*skill_lv-60;
					break;
				case CH_CHAINCRUSH:
					skillratio += 300+100*skill_lv;
					break;
				case CH_PALMSTRIKE:
					skillratio += 100+100*skill_lv;
					break;
				case LK_HEADCRUSH:
					skillratio += 40*skill_lv;
					break;
				case LK_JOINTBEAT:
					i = 10*skill_lv-50;
					// Although not clear, it's being assumed that the 2x damage is only for the break neck ailment.
					if (wflag&BREAK_NECK) i*=2;
					skillratio += i;
					break;
				case ASC_METEORASSAULT:
					skillratio += 40*skill_lv-60;
					break;
				case SN_SHARPSHOOTING:
				case MA_SHARPSHOOTING:
					skillratio += 100+50*skill_lv;
					break;
				case CG_ARROWVULCAN:
					skillratio += 100+100*skill_lv;
					break;
				case AS_SPLASHER:
					skillratio += 400+50*skill_lv;
					if(sd)
						skillratio += 30 * pc_checkskill(sd,AS_POISONREACT);
					break;
				case ASC_BREAKER:
					skillratio += 100*skill_lv-100;
					break;
				case PA_SACRIFICE:
					skillratio += 10*skill_lv-10;
					break;
				case PA_SHIELDCHAIN:
					skillratio += 30*skill_lv;
					break;
				case WS_CARTTERMINATION:
					i = 10 * (16 - skill_lv);
					if (i < 1) i = 1;
					//Preserve damage ratio when max cart weight is changed.
					if(sd && sd->cart_weight)
						skillratio += sd->cart_weight/i * 80000/battle_config.max_cart_weight - 100;
					else if (!sd)
						skillratio += 80000 / i - 100;
					break;
				case TK_DOWNKICK:
					skillratio += 60 + 20*skill_lv;
					break;
				case TK_STORMKICK:
					skillratio += 60 + 20*skill_lv;
					break;
				case TK_TURNKICK:
					skillratio += 90 + 30*skill_lv;
					break;
				case TK_COUNTER:
					skillratio += 90 + 30*skill_lv;
					break;
				case TK_JUMPKICK:
					skillratio += -70 + 10*skill_lv;
					if (sc && sc->data[SC_COMBO] && sc->data[SC_COMBO]->val1 == skill_num)
						skillratio += 10*status_get_lv(src)/3; //Tumble bonus
					if (wflag)
						skillratio += 10*status_get_lv(src)/3; //Running bonus (TODO: What is the real bonus?)
					break;
				case GS_TRIPLEACTION:
					skillratio += 50*skill_lv;
					break;
				case GS_BULLSEYE:
					//Only works well against brute/demihumans non bosses.
					if((tstatus->race == RC_BRUTE || tstatus->race == RC_DEMIHUMAN)
						&& !(tstatus->mode&MD_BOSS))
						skillratio += 400;
					break;
				case GS_TRACKING:
					skillratio += 100 *(skill_lv+1);
					break;
				case GS_PIERCINGSHOT:
					skillratio += 20*skill_lv;
					break;
				case GS_RAPIDSHOWER:
					skillratio += 10*skill_lv;
					break;
				case GS_DESPERADO:
					skillratio += 50*(skill_lv-1);
					break;
				case GS_DUST:
					skillratio += 50*skill_lv;
					break;
				case GS_FULLBUSTER:
					skillratio += 100*(skill_lv+2);
					break;
				case GS_SPREADATTACK:
					skillratio += 20*(skill_lv-1);
					break;
				case NJ_HUUMA:
					skillratio += 50 + 150*skill_lv;
					break;
				case NJ_TATAMIGAESHI:
					skillratio += 10*skill_lv;
					break;
				case NJ_KASUMIKIRI:
					skillratio += 10*skill_lv;
					break;
				case NJ_KIRIKAGE:
					skillratio += 100*(skill_lv-1);
					break;
				case KN_CHARGEATK:
					{
					int k = (wflag-1)/3; //+100% every 3 cells of distance
					if( k > 2 ) k = 2; // ...but hard-limited to 300%.
					skillratio += 100 * k; 
					}
					break;
				case HT_PHANTASMIC:
					skillratio += 50;
					break;
				case MO_BALKYOUNG:
					skillratio += 200;
					break;
				case HFLI_MOON:	//[orn]
					skillratio += 10+110*skill_lv;
					break;
				case HFLI_SBR44:	//[orn]
					skillratio += 100 *(skill_lv-1);
					break;
			}

			ATK_RATE(skillratio);

			//Constant/misc additions from skills
			switch (skill_num) {
				case MO_EXTREMITYFIST:
					ATK_ADD(250 + 150*skill_lv);
					break;
				case TK_DOWNKICK:
				case TK_STORMKICK:
				case TK_TURNKICK:
				case TK_COUNTER:
				case TK_JUMPKICK:
					//TK_RUN kick damage bonus.
					if(sd && sd->weapontype1 == W_FIST && sd->weapontype2 == W_FIST)
						ATK_ADD(10*pc_checkskill(sd, TK_RUN));
					break;
				case GS_MAGICALBULLET:
					if(sstatus->matk_max>sstatus->matk_min) {
						ATK_ADD(sstatus->matk_min+rand()%(sstatus->matk_max-sstatus->matk_min));
					} else {
						ATK_ADD(sstatus->matk_min);
					}
					break;
				case NJ_SYURIKEN:
					ATK_ADD(4*skill_lv);
					break;
			}
		}
		//Div fix.
		damage_div_fix(wd.damage, wd.div_);

		//The following are applied on top of current damage and are stackable.
		if (sc) {
			if(sc->data[SC_TRUESIGHT])
				ATK_ADDRATE(2*sc->data[SC_TRUESIGHT]->val1);

			if(sc->data[SC_EDP] &&
			  	skill_num != ASC_BREAKER &&
				skill_num != ASC_METEORASSAULT &&
				skill_num != AS_SPLASHER &&
				skill_num != AS_VENOMKNIFE)
				ATK_ADDRATE(sc->data[SC_EDP]->val3);
		}

		switch (skill_num) {
			case AS_SONICBLOW:
				if (sc && sc->data[SC_SPIRIT] &&
					sc->data[SC_SPIRIT]->val2 == SL_ASSASIN)
					ATK_ADDRATE(map_flag_gvg(src->m)?25:100); //+25% dmg on woe/+100% dmg on nonwoe

				if(sd && pc_checkskill(sd,AS_SONICACCEL)>0)
					ATK_ADDRATE(10);
			break;
			case CR_SHIELDBOOMERANG:
				if(sc && sc->data[SC_SPIRIT] &&
					sc->data[SC_SPIRIT]->val2 == SL_CRUSADER)
					ATK_ADDRATE(100);
				break;
		}
		
		if( sd )
		{
			if (skill_num && (i = pc_skillatk_bonus(sd, skill_num)))
				ATK_ADDRATE(i);

			if( skill_num != PA_SACRIFICE && skill_num != MO_INVESTIGATE && skill_num != CR_GRANDCROSS && skill_num != NPC_GRANDDARKNESS && skill_num != PA_SHIELDCHAIN && !flag.cri )
			{ //Elemental/Racial adjustments
				if( sd->right_weapon.def_ratio_atk_ele & (1<<tstatus->def_ele) ||
					sd->right_weapon.def_ratio_atk_race & (1<<tstatus->race) ||
					sd->right_weapon.def_ratio_atk_race & (1<<(is_boss(target)?RC_BOSS:RC_NONBOSS))
				)
					flag.pdef = 1;

				if( sd->left_weapon.def_ratio_atk_ele & (1<<tstatus->def_ele) ||
					sd->left_weapon.def_ratio_atk_race & (1<<tstatus->race) ||
					sd->left_weapon.def_ratio_atk_race & (1<<(is_boss(target)?RC_BOSS:RC_NONBOSS))
				)
				{ //Pass effect onto right hand if configured so. [Skotlex]
					if (battle_config.left_cardfix_to_right && flag.rh)
						flag.pdef = 1;
					else
						flag.pdef2 = 1;
				}
			}

			if (skill_num != CR_GRANDCROSS && skill_num != NPC_GRANDDARKNESS)
		  	{	//Ignore Defense?
				if (!flag.idef && (
					sd->right_weapon.ignore_def_ele & (1<<tstatus->def_ele) ||
					sd->right_weapon.ignore_def_race & (1<<tstatus->race) ||
					sd->right_weapon.ignore_def_race & (is_boss(target)?1<<RC_BOSS:1<<RC_NONBOSS)
				))
					flag.idef = 1;

				if (!flag.idef2 && (
					sd->left_weapon.ignore_def_ele & (1<<tstatus->def_ele) ||
					sd->left_weapon.ignore_def_race & (1<<tstatus->race) ||
					sd->left_weapon.ignore_def_race & (is_boss(target)?1<<RC_BOSS:1<<RC_NONBOSS)
				)) {
						if(battle_config.left_cardfix_to_right && flag.rh) //Move effect to right hand. [Skotlex]
							flag.idef = 1;
						else
							flag.idef2 = 1;
				}
			}
		}

		if( sc && sc->count && sc->data[SC_DEFRATIOATK] && skill_num != PA_SACRIFICE && skill_num != CR_GRANDCROSS && skill_num != NPC_GRANDDARKNESS && skill_num != PA_SHIELDCHAIN && !flag.cri )
			flag.pdef = flag.pdef2 = sc->data[SC_DEFRATIOATK]->val1; // Occult Impact Effect

		if (!flag.idef || !flag.idef2)
		{	//Defense reduction
			short vit_def;
			signed char def1 = status_get_def(target); //Don't use tstatus->def1 due to skill timer reductions.
			short def2 = (short)tstatus->def2;

			if( sd )
			{
				i = sd->ignore_def[is_boss(target)?RC_BOSS:RC_NONBOSS];
				i += sd->ignore_def[tstatus->race];
				if( i )
				{
					if( i > 100 ) i = 100;
					def1 -= def1 * i / 100;
					// def2 -= def2 * i / 100;
				}
			}

			if( battle_config.vit_penalty_type && battle_config.vit_penalty_target&target->type )
			{
				unsigned char target_count; //256 max targets should be a sane max
				target_count = unit_counttargeted(target,battle_config.vit_penalty_count_lv);
				if(target_count >= battle_config.vit_penalty_count) {
					if(battle_config.vit_penalty_type == 1) {
						def1 = (def1 * (100 - (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num))/100;
						def2 = (def2 * (100 - (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num))/100;
					} else { //Assume type 2
						def1 -= (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num;
						def2 -= (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num;
					}
				}
				if(skill_num == AM_ACIDTERROR) def1 = 0; //Acid Terror ignores only armor defense. [Skotlex]
				if(def2 < 1) def2 = 1;
			}
			//Vitality reduction from rodatazone: http://rodatazone.simgaming.net/mechanics/substats.php#def	
			if (tsd)	//Sd vit-eq
			{	//[VIT*0.5] + rnd([VIT*0.3], max([VIT*0.3],[VIT^2/150]-1))
				vit_def = def2*(def2-15)/150;
				vit_def = def2/2 + (vit_def>0?rand()%vit_def:0);
				
				if((battle_check_undead(sstatus->race,sstatus->def_ele) || sstatus->race==RC_DEMON) && //This bonus already doesnt work vs players
					src->type == BL_MOB && (skill=pc_checkskill(tsd,AL_DP)) > 0)
					vit_def += skill*(int)(3 +(tsd->status.base_level+1)*0.04);   // submitted by orn
			} else { //Mob-Pet vit-eq
				//VIT + rnd(0,[VIT/20]^2-1)
				vit_def = (def2/20)*(def2/20);
				vit_def = def2 + (vit_def>0?rand()%vit_def:0);
			}
			
			if (battle_config.weapon_defense_type) {
				vit_def += def1*battle_config.weapon_defense_type;
				def1 = 0;
			}
			if (def1 > 100) def1 = 100;
			ATK_RATE2(
				flag.idef ?100:
				(flag.pdef ?flag.pdef *(def1 + vit_def):
				100-def1),
			  	flag.idef2?100:
				(flag.pdef2?flag.pdef2*(def1 + vit_def):
				100-def1)
			);
			ATK_ADD2(
				flag.idef ||flag.pdef ?0:-vit_def,
				flag.idef2||flag.pdef2?0:-vit_def
			);
		}

		//Post skill/vit reduction damage increases
		if( sc && skill_num != LK_SPIRALPIERCE && skill_num != ML_SPIRALPIERCE )
		{	//SC skill damages
			if(sc->data[SC_AURABLADE]) 
				ATK_ADD(20*sc->data[SC_AURABLADE]->val1);
		}

		//Refine bonus
		if( sd && flag.weapon && skill_num != MO_INVESTIGATE && skill_num != MO_EXTREMITYFIST )
		{ // Counts refine bonus multiple times
			if( skill_num == MO_FINGEROFFENSIVE )
			{
				ATK_ADD2(wd.div_*sstatus->rhw.atk2, wd.div_*sstatus->lhw.atk2);
			} else {
				ATK_ADD2(sstatus->rhw.atk2, sstatus->lhw.atk2);
			}
		}

		//Set to min of 1
		if (flag.rh && wd.damage < 1) wd.damage = 1;
		if (flag.lh && wd.damage2 < 1) wd.damage2 = 1;

		if (sd && flag.weapon &&
			skill_num != MO_INVESTIGATE &&
		  	skill_num != MO_EXTREMITYFIST &&
		  	skill_num != CR_GRANDCROSS)
		{	//Add mastery damage
			if(skill_num != ASC_BREAKER && sd->status.weapon == W_KATAR &&
				(skill=pc_checkskill(sd,ASC_KATAR)) > 0)
		  	{	//Adv Katar Mastery is does not applies to ASC_BREAKER,
				// but other masteries DO apply >_>
				ATK_ADDRATE(10+ 2*skill);
			}

			wd.damage = battle_addmastery(sd,target,wd.damage,0);
			if (flag.lh)
				wd.damage2 = battle_addmastery(sd,target,wd.damage2,1);

			if (sc && sc->data[SC_MIRACLE]) i = 2; //Star anger
			else
			ARR_FIND(0, 3, i, t_class == sd->hate_mob[i]);
			if (i < 3 && (skill=pc_checkskill(sd,sg_info[i].anger_id))) 
			{
				skillratio = sd->status.base_level + sstatus->dex + sstatus->luk;
				if (i == 2) skillratio += sstatus->str; //Star Anger
				if (skill<4)
					skillratio /= 12-3*skill;
				ATK_ADDRATE(skillratio);
			}
			if (skill_num == NJ_SYURIKEN && (skill = pc_checkskill(sd,NJ_TOBIDOUGU)) > 0)
				ATK_ADD(3*skill);
			if (skill_num == NJ_KUNAI)
				ATK_ADD(60);
		}
	} //Here ends flag.hit section, the rest of the function applies to both hitting and missing attacks
  	else if(wd.div_ < 0) //Since the attack missed...
		wd.div_ *= -1; 

	if(skill_num == CR_GRANDCROSS || skill_num == NPC_GRANDDARKNESS)
		return wd; //Enough, rest is not needed.

	if(sd && (skill=pc_checkskill(sd,BS_WEAPONRESEARCH)) > 0) 
		ATK_ADD(skill*2);

	if(skill_num==TF_POISON)
		ATK_ADD(15*skill_lv);

	if(!(nk&NK_NO_ELEFIX || (s_ele == ELE_NEUTRAL &&
		battle_config.attack_attr_none&src->type)))
	{	//Elemental attribute fix
		if (wd.damage > 0)
		{
			wd.damage=battle_attr_fix(src,target,wd.damage,s_ele,tstatus->def_ele, tstatus->ele_lv);
			if(skill_num==MC_CARTREVOLUTION) //Cart Revolution applies the element fix once more with neutral element
				wd.damage = battle_attr_fix(src,target,wd.damage,ELE_NEUTRAL,tstatus->def_ele, tstatus->ele_lv);
			if(skill_num== GS_GROUNDDRIFT) //Additional 50*lv Neutral damage.
				wd.damage+= battle_attr_fix(src,target,50*skill_lv,ELE_NEUTRAL,tstatus->def_ele, tstatus->ele_lv);
		}
		if (flag.lh && wd.damage2 > 0)
			wd.damage2 = battle_attr_fix(src,target,wd.damage2,s_ele_,tstatus->def_ele, tstatus->ele_lv);
		if(sc && sc->data[SC_WATK_ELEMENT])
		{	//Descriptions indicate this means adding a percent of a normal attack in another element. [Skotlex]
			int damage= battle_calc_base_damage(sstatus, &sstatus->rhw, sc, tstatus->size, sd, (flag.arrow?2:0));
			damage = damage*sc->data[SC_WATK_ELEMENT]->val2/100;
			damage = battle_attr_fix(src,target,damage,sc->data[SC_WATK_ELEMENT]->val1,tstatus->def_ele, tstatus->ele_lv);
			ATK_ADD(damage);
		}
	}

	if (sd)
	{
		if (skill_num != CR_SHIELDBOOMERANG) //Only Shield boomerang doesn't takes the Star Crumbs bonus.
			ATK_ADD2(wd.div_*sd->right_weapon.star, wd.div_*sd->left_weapon.star);
		if (skill_num==MO_FINGEROFFENSIVE) { //The finger offensive spheres on moment of attack do count. [Skotlex]
			ATK_ADD(wd.div_*sd->spiritball_old*3);
		} else {
			ATK_ADD(wd.div_*sd->spiritball*3);
		}

		//Card Fix, sd side
		if( (wd.damage || wd.damage2) && !(nk&NK_NO_CARDFIX_ATK) )
		{
			int cardfix = 1000, cardfix_ = 1000;
			int t_race2 = status_get_race2(target);	
			if(sd->state.arrow_atk)
			{
				cardfix=cardfix*(100+sd->right_weapon.addrace[tstatus->race]+sd->arrow_addrace[tstatus->race])/100;
				if (!(nk&NK_NO_ELEFIX))
					cardfix=cardfix*(100+sd->right_weapon.addele[tstatus->def_ele]+sd->arrow_addele[tstatus->def_ele])/100;
				cardfix=cardfix*(100+sd->right_weapon.addsize[tstatus->size]+sd->arrow_addsize[tstatus->size])/100;
				cardfix=cardfix*(100+sd->right_weapon.addrace2[t_race2])/100;
				cardfix=cardfix*(100+sd->right_weapon.addrace[is_boss(target)?RC_BOSS:RC_NONBOSS]+sd->arrow_addrace[is_boss(target)?RC_BOSS:RC_NONBOSS])/100;
				if( tstatus->race != RC_DEMIHUMAN )
					cardfix=cardfix*(100+sd->right_weapon.addrace[RC_NONDEMIHUMAN]+sd->arrow_addrace[RC_NONDEMIHUMAN])/100;
			}
			else
			{ // Melee attack
				if( !battle_config.left_cardfix_to_right )
				{
					cardfix=cardfix*(100+sd->right_weapon.addrace[tstatus->race])/100;
					if (!(nk&NK_NO_ELEFIX))
						cardfix=cardfix*(100+sd->right_weapon.addele[tstatus->def_ele])/100;
					cardfix=cardfix*(100+sd->right_weapon.addsize[tstatus->size])/100;
					cardfix=cardfix*(100+sd->right_weapon.addrace2[t_race2])/100;
					cardfix=cardfix*(100+sd->right_weapon.addrace[is_boss(target)?RC_BOSS:RC_NONBOSS])/100;
					if( tstatus->race != RC_DEMIHUMAN )
						cardfix=cardfix*(100+sd->right_weapon.addrace[RC_NONDEMIHUMAN])/100;

					if( flag.lh )
					{
						cardfix_=cardfix_*(100+sd->left_weapon.addrace[tstatus->race])/100;
						if (!(nk&NK_NO_ELEFIX))
							cardfix_=cardfix_*(100+sd->left_weapon.addele[tstatus->def_ele])/100;
						cardfix_=cardfix_*(100+sd->left_weapon.addsize[tstatus->size])/100;
						cardfix_=cardfix_*(100+sd->left_weapon.addrace2[t_race2])/100;
						cardfix_=cardfix_*(100+sd->left_weapon.addrace[is_boss(target)?RC_BOSS:RC_NONBOSS])/100;
						if( tstatus->race != RC_DEMIHUMAN )
							cardfix=cardfix*(100+sd->left_weapon.addrace[RC_NONDEMIHUMAN])/100;
					}
				}
				else
				{
					cardfix=cardfix*(100+sd->right_weapon.addrace[tstatus->race]+sd->left_weapon.addrace[tstatus->race])/100;
					cardfix=cardfix*(100+sd->right_weapon.addele[tstatus->def_ele]+sd->left_weapon.addele[tstatus->def_ele])/100;
					cardfix=cardfix*(100+sd->right_weapon.addsize[tstatus->size]+sd->left_weapon.addsize[tstatus->size])/100;
					cardfix=cardfix*(100+sd->right_weapon.addrace2[t_race2]+sd->left_weapon.addrace2[t_race2])/100;
					cardfix=cardfix*(100+sd->right_weapon.addrace[is_boss(target)?RC_BOSS:RC_NONBOSS]+sd->left_weapon.addrace[is_boss(target)?RC_BOSS:RC_NONBOSS])/100;
					if( tstatus->race != RC_DEMIHUMAN )
						cardfix=cardfix*(100+sd->right_weapon.addrace[RC_NONDEMIHUMAN]+sd->left_weapon.addrace[RC_NONDEMIHUMAN])/100;
				}
			}

			for( i = 0; i < ARRAYLENGTH(sd->right_weapon.add_dmg) && sd->right_weapon.add_dmg[i].rate; i++ )
			{
				if( sd->right_weapon.add_dmg[i].class_ == t_class )
				{
					cardfix=cardfix*(100+sd->right_weapon.add_dmg[i].rate)/100;
					break;
				}
			}

			if( flag.lh )
			{
				for( i = 0; i < ARRAYLENGTH(sd->left_weapon.add_dmg) && sd->left_weapon.add_dmg[i].rate; i++ )
				{
					if( sd->left_weapon.add_dmg[i].class_ == t_class )
					{
						cardfix_=cardfix_*(100+sd->left_weapon.add_dmg[i].rate)/100;
						break;
					}
				}
			}

			if( wd.flag&BF_LONG )
				cardfix=cardfix*(100+sd->long_attack_atk_rate)/100;

			if( cardfix != 1000 || cardfix_ != 1000 )
				ATK_RATE2(cardfix/10, cardfix_/10);	//What happens if you use right-to-left and there's no right weapon, only left?
		}
		
		if( skill_num == CR_SHIELDBOOMERANG || skill_num == PA_SHIELDCHAIN )
		{ //Refine bonus applies after cards and elements.
			short index= sd->equip_index[EQI_HAND_L];
			if( index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_ARMOR )
				ATK_ADD(10*sd->status.inventory[index].refine);
		}
	} //if (sd)

	//Card Fix, tsd sid
	if( tsd && !(nk&NK_NO_CARDFIX_DEF) )
	{
		short s_race2,s_class;
		short cardfix=1000;

		s_race2 = status_get_race2(src);
		s_class = status_get_class(src);

		if( !(nk&NK_NO_ELEFIX) )
		{
			cardfix=cardfix*(100-tsd->subele[s_ele])/100;
			if( flag.lh && s_ele_ != s_ele )
				cardfix=cardfix*(100-tsd->subele[s_ele_])/100;
		}
		cardfix=cardfix*(100-tsd->subsize[sstatus->size])/100;
 		cardfix=cardfix*(100-tsd->subrace2[s_race2])/100;
		cardfix=cardfix*(100-tsd->subrace[sstatus->race])/100;
		cardfix=cardfix*(100-tsd->subrace[is_boss(src)?RC_BOSS:RC_NONBOSS])/100;
		if( sstatus->race != RC_DEMIHUMAN )
			cardfix=cardfix*(100-tsd->subrace[RC_NONDEMIHUMAN])/100;

		for( i = 0; i < ARRAYLENGTH(tsd->add_def) && tsd->add_def[i].rate;i++ )
		{
			if( tsd->add_def[i].class_ == s_class )
			{
				cardfix=cardfix*(100-tsd->add_def[i].rate)/100;
				break;
			}
		}

		if( wd.flag&BF_SHORT )
			cardfix=cardfix*(100-tsd->near_attack_def_rate)/100;
		else	// BF_LONG (there's no other choice)
			cardfix=cardfix*(100-tsd->long_attack_def_rate)/100;

		if( tsd->sc.data[SC_DEF_RATE] )
			cardfix=cardfix*(100-tsd->sc.data[SC_DEF_RATE]->val1)/100;

		if( cardfix != 1000 )
			ATK_RATE(cardfix/10);
	}

	if( flag.infdef )
	{ //Plants receive 1 damage when hit
		if( flag.rh && (flag.hit || wd.damage>0) )
			wd.damage = 1;
		if( flag.lh && (flag.hit || wd.damage2>0) )
			wd.damage2 = 1;
		if( !(battle_config.skill_min_damage&1) )
			//Do not return if you are supposed to deal greater damage to plants than 1. [Skotlex]
			return wd;
	}

	if(sd && !skill_num && !flag.cri)
	{	//Check for double attack.
		if(((skill_lv = pc_checkskill(sd,TF_DOUBLE)) > 0 && sd->weapontype1 == W_DAGGER)
			||(sd->double_rate > 0 && sd->weapontype1 != W_FIST)) //Will fail bare-handed
		{	//Success chance is not added, the higher one is used [Skotlex]
			if (rand()%100 < (5*skill_lv>sd->double_rate?5*skill_lv:sd->double_rate))
			{
				wd.div_=skill_get_num(TF_DOUBLE,skill_lv?skill_lv:1);
				damage_div_fix(wd.damage, wd.div_);
				wd.type = 0x08;
			}
		} else
	  	if (sd->weapontype1 == W_REVOLVER &&
			(skill_lv = pc_checkskill(sd,GS_CHAINACTION)) > 0 &&
			(rand()%100 < 5*skill_lv)
			)
		{
			wd.div_=skill_get_num(GS_CHAINACTION,skill_lv);
			damage_div_fix(wd.damage, wd.div_);
			wd.type = 0x08;
		}
	}

	if (sd)
	{
		if (!flag.rh && flag.lh) 
		{	//Move lh damage to the rh
			wd.damage = wd.damage2;
			wd.damage2 = 0;
			flag.rh=1;
			flag.lh=0;
		} else if(flag.rh && flag.lh)
		{	//Dual-wield
			if (wd.damage)
			{
				skill = pc_checkskill(sd,AS_RIGHT);
				wd.damage = wd.damage * (50 + (skill * 10))/100;
				if(wd.damage < 1) wd.damage = 1;
			}
			if (wd.damage2)
			{
				skill = pc_checkskill(sd,AS_LEFT);
				wd.damage2 = wd.damage2 * (30 + (skill * 10))/100;
				if(wd.damage2 < 1) wd.damage2 = 1;
			}
		} else if(sd->status.weapon == W_KATAR && !skill_num)
		{ //Katars (offhand damage only applies to normal attacks, tested on Aegis 10.2)
			skill = pc_checkskill(sd,TF_DOUBLE);
			wd.damage2 = wd.damage * (1 + (skill * 2))/100;

			if(wd.damage && !wd.damage2) wd.damage2 = 1;
			flag.lh = 1;
		}
	}

	if(!flag.rh && wd.damage)
		wd.damage=0;

	if(!flag.lh && wd.damage2)
		wd.damage2=0;

	if(wd.damage + wd.damage2)
	{	//There is a total damage value
		if(!wd.damage2) {
			wd.damage=battle_calc_damage(src,target,wd.damage,wd.div_,skill_num,skill_lv,wd.flag);
			if (map_flag_gvg2(target->m))
				wd.damage=battle_calc_gvg_damage(src,target,wd.damage,wd.div_,skill_num,skill_lv,wd.flag);
		} else
		if(!wd.damage) {
			wd.damage2=battle_calc_damage(src,target,wd.damage2,wd.div_,skill_num,skill_lv,wd.flag);
			if (map_flag_gvg2(target->m))
				wd.damage2=battle_calc_gvg_damage(src,target,wd.damage2,wd.div_,skill_num,skill_lv,wd.flag);
		} else
		{
			int d1=wd.damage+wd.damage2,d2=wd.damage2;
			wd.damage=battle_calc_damage(src,target,d1,wd.div_,skill_num,skill_lv,wd.flag);
			if (map_flag_gvg2(target->m))
				wd.damage=battle_calc_gvg_damage(src,target,wd.damage,wd.div_,skill_num,skill_lv,wd.flag);
			wd.damage2=(d2*100/d1)*wd.damage/100;
			if(wd.damage > 1 && wd.damage2 < 1) wd.damage2=1;
			wd.damage-=wd.damage2;
		}
	}

	if(skill_num==ASC_BREAKER)
	{	//Breaker's int-based damage (a misc attack?)
		struct Damage md = battle_calc_misc_attack(src, target, skill_num, skill_lv, wflag);
		wd.damage += md.damage;
	}

	if (wd.damage || wd.damage2) {
		if (sd && battle_config.equip_self_break_rate)
		{	// Self weapon breaking
			int breakrate = battle_config.equip_natural_break_rate;
			if (sc) {
				if(sc->data[SC_OVERTHRUST])
					breakrate += 10;
				if(sc->data[SC_MAXOVERTHRUST])
					breakrate += 10;
			}
			if (breakrate)
				skill_break_equip(src, EQP_WEAPON, breakrate, BCT_SELF);
		}
		//Cart Termination/Tomahawk won't trigger breaking data. Why? No idea, go ask Gravity.
		if (battle_config.equip_skill_break_rate && skill_num != WS_CARTTERMINATION && skill_num != ITM_TOMAHAWK)
		{	// Target equipment breaking
			int breakrate[2] = {0,0}; // weapon = 0, armor = 1
			if (sd) {	// Break rate from equipment
				breakrate[0] += sd->break_weapon_rate;
				breakrate[1] += sd->break_armor_rate;
			}
			if (sc) {
				if (sc->data[SC_MELTDOWN]) {
					breakrate[0] += sc->data[SC_MELTDOWN]->val2;
					breakrate[1] += sc->data[SC_MELTDOWN]->val3;
				}
			}	
			if (breakrate[0])
				skill_break_equip(target, EQP_WEAPON, breakrate[0], BCT_ENEMY);
			if (breakrate[1])
				skill_break_equip(target, EQP_ARMOR, breakrate[1], BCT_ENEMY);
		}
	}

	//SG_FUSION hp penalty [Komurka]
	if (sc && sc->data[SC_FUSION])
	{
		int hp= sstatus->max_hp;
		if (sd && tsd) {
			hp = 8*hp/100;
			if (100*sstatus->hp <= 20*sstatus->max_hp)
				hp = sstatus->hp;
		} else
			hp = 2*hp/100; //2% hp loss per hit
		status_zap(src, hp, 0);
	}

	return wd;
}

/*==========================================
 * battle_calc_magic_attack [DracoRPG]
 *------------------------------------------*/
struct Damage battle_calc_magic_attack(struct block_list *src,struct block_list *target,int skill_num,int skill_lv,int mflag)
{
	int i, nk;
	short s_ele;
	unsigned int skillratio = 100;	//Skill dmg modifiers.

	struct map_session_data *sd, *tsd;
	struct Damage ad;
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);
	struct {
		unsigned imdef : 1;
		unsigned infdef : 1;
	}	flag;

	memset(&ad,0,sizeof(ad));
	memset(&flag,0,sizeof(flag));

	if(src==NULL || target==NULL)
	{
		nullpo_info(NLP_MARK);
		return ad;
	}
	//Initial Values
	ad.damage = 1;
	ad.div_=skill_get_num(skill_num,skill_lv);
	ad.amotion=skill_get_inf(skill_num)&INF_GROUND_SKILL?0:sstatus->amotion; //Amotion should be 0 for ground skills.
	ad.dmotion=tstatus->dmotion;
	ad.blewcount = skill_get_blewcount(skill_num,skill_lv);
	ad.flag=BF_MAGIC|BF_SKILL;
	ad.dmg_lv=ATK_DEF;
	nk = skill_get_nk(skill_num);
	flag.imdef = nk&NK_IGNORE_DEF?1:0;

	sd = BL_CAST(BL_PC, src);
	tsd = BL_CAST(BL_PC, target);

	//Initialize variables that will be used afterwards
	s_ele = skill_get_ele(skill_num, skill_lv);

	if (s_ele == -1) // pl=-1 : the skill takes the weapon's element
		s_ele = sstatus->rhw.ele;
	else if (s_ele == -2) //Use status element
		s_ele = status_get_attack_sc_element(src,status_get_sc(src));
	
	//Set miscellaneous data that needs be filled
	if(sd) {
		sd->state.arrow_atk = 0;
		ad.blewcount += battle_blewcount_bonus(sd, skill_num);
	}

	//Skill Range Criteria
	ad.flag |= battle_range_type(src, target, skill_num, skill_lv);
	flag.infdef=(tstatus->mode&MD_PLANT?1:0);
		
	switch(skill_num)
	{
		case MG_FIREWALL:
		case NJ_KAENSIN:
			ad.dmotion = 0; //No flinch animation.
			if ( tstatus->def_ele == ELE_FIRE || battle_check_undead(tstatus->race, tstatus->def_ele) )
				ad.blewcount = 0; //No knockback
			break;
		case PR_SANCTUARY:
			ad.dmotion = 0; //No flinch animation.
			break;
	}

	if (!flag.infdef) //No need to do the math for plants
	{

//MATK_RATE scales the damage. 100 = no change. 50 is halved, 200 is doubled, etc
#define MATK_RATE( a ) { ad.damage= ad.damage*(a)/100; }
//Adds dmg%. 100 = +100% (double) damage. 10 = +10% damage
#define MATK_ADDRATE( a ) { ad.damage+= ad.damage*(a)/100; }
//Adds an absolute value to damage. 100 = +100 damage
#define MATK_ADD( a ) { ad.damage+= a; }

		switch (skill_num)
		{	//Calc base damage according to skill
			case AL_HEAL:
			case PR_BENEDICTIO:
				ad.damage = skill_calc_heal(src, target, skill_lv)/2;
				break;
			case PR_ASPERSIO:
				ad.damage = 40;
				break;
			case PR_SANCTUARY:
				ad.damage = (skill_lv>6)?388:skill_lv*50;
				break;
			case ALL_RESURRECTION:
			case PR_TURNUNDEAD:
				//Undead check is on skill_castend_damageid code.
				i = 20*skill_lv + sstatus->luk + sstatus->int_ + status_get_lv(src)
				  	+ 200 - 200*tstatus->hp/tstatus->max_hp;
				if(i > 700) i = 700;
				if(rand()%1000 < i && !(tstatus->mode&MD_BOSS))
					ad.damage = tstatus->hp;
				else
					ad.damage = status_get_lv(src) + sstatus->int_ + skill_lv * 10;
				break;
			case PF_SOULBURN:
				ad.damage = tstatus->sp * 2;
				break;
			default:
			{
				if (sstatus->matk_max > sstatus->matk_min) {
					MATK_ADD(sstatus->matk_min+rand()%(1+sstatus->matk_max-sstatus->matk_min));
				} else {
					MATK_ADD(sstatus->matk_min);
				}

				if(nk&NK_SPLASHSPLIT){ // Divide MATK in case of multiple targets skill
					if(mflag>0)
						ad.damage/= mflag;
					else
						ShowError("0 enemies targeted by %d:%s, divide per 0 avoided!\n", skill_num, skill_get_name(skill_num));
				}

				switch(skill_num){
					case MG_NAPALMBEAT:
					case MG_FIREBALL:
						skillratio += skill_lv*10-30;
						break;
					case MG_SOULSTRIKE:
						if (battle_check_undead(tstatus->race,tstatus->def_ele))
							skillratio += 5*skill_lv;
						break;
					case MG_FIREWALL:
						skillratio -= 50;
						break;
					case MG_THUNDERSTORM:
						skillratio -= 20;
						break;
					case MG_FROSTDIVER:
						skillratio += 10*skill_lv;
						break;
					case AL_HOLYLIGHT:
						skillratio += 25;
						if (sd && sd->sc.data[SC_SPIRIT] && sd->sc.data[SC_SPIRIT]->val2 == SL_PRIEST)
							skillratio *= 5; //Does 5x damage include bonuses from other skills?
						break;
					case AL_RUWACH:
						skillratio += 45;
						break;
					case WZ_FROSTNOVA:
						skillratio += (100+skill_lv*10)*2/3-100;
						break;
					case WZ_FIREPILLAR:
						if (skill_lv > 10)
							skillratio += 100;
						else
							skillratio -= 80;
						break;
					case WZ_SIGHTRASHER:
						skillratio += 20*skill_lv;
						break;
					case WZ_VERMILION:
						skillratio += 20*skill_lv-20;
						break;
					case WZ_WATERBALL:
						skillratio += 30*skill_lv;
						break;
					case WZ_STORMGUST:
						skillratio += 40*skill_lv;
						break;
					case HW_NAPALMVULCAN:
						skillratio += 10*skill_lv-30;
						break;
					case SL_STIN:
						skillratio += (tstatus->size?-99:10*skill_lv); //target size must be small (0) for full damage.
						break;
					case SL_STUN:
						skillratio += (tstatus->size!=2?5*skill_lv:-99); //Full damage is dealt on small/medium targets
						break;
					case SL_SMA:
						skillratio += -60 + status_get_lv(src); //Base damage is 40% + lv%
						break;
					case NJ_KOUENKA:
						skillratio -= 10;
						break;
					case NJ_KAENSIN:
						skillratio -= 50;
						break;
					case NJ_BAKUENRYU:
						skillratio += 50*(skill_lv-1);
						break;
					case NJ_HYOUSYOURAKU:
						skillratio += 50*skill_lv;
						break;
					case NJ_RAIGEKISAI:
						skillratio += 60 + 40*skill_lv;
						break;
					case NJ_KAMAITACHI:
					case NPC_ENERGYDRAIN:
						skillratio += 100*skill_lv;
						break;
					case NPC_EARTHQUAKE:
						skillratio += 100 +100*skill_lv +100*(skill_lv/2);
						break;
				}

				MATK_RATE(skillratio);
			
				//Constant/misc additions from skills
				if (skill_num == WZ_FIREPILLAR)
					MATK_ADD(50);
			}
		}

		if(sd) {
			//Damage bonuses
			if ((i = pc_skillatk_bonus(sd, skill_num)))
				ad.damage += ad.damage*i/100;

			//Ignore Defense?
			if (!flag.imdef && (
				sd->ignore_mdef_ele & (1<<tstatus->def_ele) ||
				sd->ignore_mdef_race & (1<<tstatus->race) ||
				sd->ignore_mdef_race & (is_boss(target)?1<<RC_BOSS:1<<RC_NONBOSS)
			))
				flag.imdef = 1;
		}

		if(!flag.imdef){
			char mdef = tstatus->mdef;
			int mdef2= tstatus->mdef2;
			if(sd) {
				i = sd->ignore_mdef[is_boss(target)?RC_BOSS:RC_NONBOSS];
				i+= sd->ignore_mdef[tstatus->race];
				if (i)
				{
					if (i > 100) i = 100;
					mdef -= mdef * i/100;
					//mdef2-= mdef2* i/100;
				}
			}
			if(battle_config.magic_defense_type)
				ad.damage = ad.damage - mdef*battle_config.magic_defense_type - mdef2;
			else
				ad.damage = ad.damage * (100-mdef)/100 - mdef2;
		}

		if(skill_num == CR_GRANDCROSS || skill_num == NPC_GRANDDARKNESS)
		{	//Apply the physical part of the skill's damage. [Skotlex]
			struct Damage wd = battle_calc_weapon_attack(src,target,skill_num,skill_lv,mflag);
			ad.damage = (wd.damage + ad.damage) * (100 + 40*skill_lv)/100;
			if(src==target)
			{
				if (src->type == BL_PC)
					ad.damage = ad.damage/2;
				else
					ad.damage = 0;
			}
		}
		
		if (skill_num == NPC_EARTHQUAKE)
		{	//Adds atk2 to the damage, should be influenced by number of hits and skill-ratio, but not mdef reductions. [Skotlex]
			//Also divide the extra bonuses from atk2 based on the number in range [Kevin]
			if(mflag>0)
				ad.damage+= (sstatus->rhw.atk2*skillratio/100)/mflag;
			else
				ShowError("Zero range by %d:%s, divide per 0 avoided!\n", skill_num, skill_get_name(skill_num));
		}

		if(ad.damage<1)
			ad.damage=1;

		if (!(nk&NK_NO_ELEFIX))
			ad.damage=battle_attr_fix(src, target, ad.damage, s_ele, tstatus->def_ele, tstatus->ele_lv);

		if (sd && !(nk&NK_NO_CARDFIX_ATK)) {
			short t_class = status_get_class(target);
			short cardfix=1000;

			cardfix=cardfix*(100+sd->magic_addrace[tstatus->race])/100;
			if (!(nk&NK_NO_ELEFIX))
				cardfix=cardfix*(100+sd->magic_addele[tstatus->def_ele])/100;
			cardfix=cardfix*(100+sd->magic_addsize[tstatus->size])/100;
			cardfix=cardfix*(100+sd->magic_addrace[is_boss(target)?RC_BOSS:RC_NONBOSS])/100;
			for(i=0; i< ARRAYLENGTH(sd->add_mdmg) && sd->add_mdmg[i].rate;i++) {
				if(sd->add_mdmg[i].class_ == t_class) {
					cardfix=cardfix*(100+sd->add_mdmg[i].rate)/100;
					continue;
				}
			}
			if (cardfix != 1000)
				MATK_RATE(cardfix/10);
		}

		if( tsd && !(nk&NK_NO_CARDFIX_DEF) )
	  	{ // Target cards.
			short s_race2 = status_get_race2(src);
			short s_class= status_get_class(src);
			int cardfix=1000;

			if (!(nk&NK_NO_ELEFIX))
				cardfix=cardfix*(100-tsd->subele[s_ele])/100;
			cardfix=cardfix*(100-tsd->subsize[sstatus->size])/100;
			cardfix=cardfix*(100-tsd->subrace2[s_race2])/100;
			cardfix=cardfix*(100-tsd->subrace[sstatus->race])/100;
			cardfix=cardfix*(100-tsd->subrace[is_boss(src)?RC_BOSS:RC_NONBOSS])/100;
			if( sstatus->race != RC_DEMIHUMAN )
				cardfix=cardfix*(100-tsd->subrace[RC_NONDEMIHUMAN])/100;

			for(i=0; i < ARRAYLENGTH(tsd->add_mdef) && tsd->add_mdef[i].rate;i++) {
				if(tsd->add_mdef[i].class_ == s_class) {
					cardfix=cardfix*(100-tsd->add_mdef[i].rate)/100;
					break;
				}
			}
			//It was discovered that ranged defense also counts vs magic! [Skotlex]
			if (ad.flag&BF_SHORT)
				cardfix=cardfix*(100-tsd->near_attack_def_rate)/100;
			else
				cardfix=cardfix*(100-tsd->long_attack_def_rate)/100;

			cardfix=cardfix*(100-tsd->magic_def_rate)/100;

			if( tsd->sc.data[SC_MDEF_RATE] )
				cardfix=cardfix*(100-tsd->sc.data[SC_MDEF_RATE]->val1)/100;

			if (cardfix != 1000)
				MATK_RATE(cardfix/10);
		}
	}

	damage_div_fix(ad.damage, ad.div_);
	
	if (flag.infdef && ad.damage)
		ad.damage = ad.damage>0?1:-1;
		
	ad.damage=battle_calc_damage(src,target,ad.damage,ad.div_,skill_num,skill_lv,ad.flag);
	if (map_flag_gvg2(target->m))
		ad.damage=battle_calc_gvg_damage(src,target,ad.damage,ad.div_,skill_num,skill_lv,ad.flag);
	return ad;
}

/*==========================================
 * ‚»‚Ì‘¼ƒ_ƒ??[ƒWŒvŽZ
 *------------------------------------------*/
struct Damage battle_calc_misc_attack(struct block_list *src,struct block_list *target,int skill_num,int skill_lv,int mflag)
{
	int skill;
	short i, nk;
	short s_ele;

	struct map_session_data *sd, *tsd;
	struct Damage md; //DO NOT CONFUSE with md of mob_data!
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);

	memset(&md,0,sizeof(md));

	if( src == NULL || target == NULL ){
		nullpo_info(NLP_MARK);
		return md;
	}

	//Some initial values
	md.amotion=skill_get_inf(skill_num)&INF_GROUND_SKILL?0:sstatus->amotion;
	md.dmotion=tstatus->dmotion;
	md.div_=skill_get_num( skill_num,skill_lv );
	md.blewcount=skill_get_blewcount(skill_num,skill_lv);
	md.dmg_lv=ATK_DEF;
	md.flag=BF_MISC|BF_SKILL;

	nk = skill_get_nk(skill_num);
	
	sd = BL_CAST(BL_PC, src);
	tsd = BL_CAST(BL_PC, target);
	
	if(sd) {
		sd->state.arrow_atk = 0;
		md.blewcount += battle_blewcount_bonus(sd, skill_num);
	}

	s_ele = skill_get_ele(skill_num, skill_lv);
	if (s_ele < 0) //Attack that takes weapon's element for misc attacks? Make it neutral [Skotlex]
		s_ele = ELE_NEUTRAL;

	//Skill Range Criteria
	md.flag |= battle_range_type(src, target, skill_num, skill_lv);

	switch( skill_num )
	{
	case HT_LANDMINE:
	case MA_LANDMINE:
		md.damage=skill_lv*(sstatus->dex+75)*(100+sstatus->int_)/100;
		break;
	case HT_BLASTMINE:
		md.damage=skill_lv*(sstatus->dex/2+50)*(100+sstatus->int_)/100;
		break;
	case HT_CLAYMORETRAP:
		md.damage=skill_lv*(sstatus->dex/2+75)*(100+sstatus->int_)/100;
		break;
	case HT_BLITZBEAT:
	case SN_FALCONASSAULT:
		//Blitz-beat Damage.
		if(!sd || (skill = pc_checkskill(sd,HT_STEELCROW)) <= 0)
			skill=0;
		md.damage=(sstatus->dex/10+sstatus->int_/2+skill*3+40)*2;
		if(mflag > 1) //Autocasted Blitz.
			nk|=NK_SPLASHSPLIT;
		
		if (skill_num == SN_FALCONASSAULT)
		{
			//Div fix of Blitzbeat
			skill = skill_get_num(HT_BLITZBEAT, 5);
			damage_div_fix(md.damage, skill); 

			//Falcon Assault Modifier
			md.damage=md.damage*(150+70*skill_lv)/100;
		}
		break;
	case TF_THROWSTONE:
		md.damage=50;
		break;
	case BA_DISSONANCE:
		md.damage=30+skill_lv*10;
		if (sd)
			md.damage+= 3*pc_checkskill(sd,BA_MUSICALLESSON);
		break;
	case NPC_SELFDESTRUCTION:
		md.damage = sstatus->hp;
		break;
	case NPC_SMOKING:
		md.damage=3;
		break;
	case NPC_DARKBREATH:
		md.damage = 500 + (skill_lv-1)*1000 + rand()%1000;
		if(md.damage > 9999) md.damage = 9999;
		break;
	case PA_PRESSURE:
		md.damage=500+300*skill_lv;
		break;
	case PA_GOSPEL:
		md.damage = 1+rand()%9999;
		break;
	case CR_ACIDDEMONSTRATION: // updated the formula based on a Japanese formula found to be exact [Reddozen]
		if(tstatus->vit+sstatus->int_) //crash fix
			md.damage = 7*tstatus->vit*sstatus->int_*sstatus->int_ / (10*(tstatus->vit+sstatus->int_));
		else
			md.damage = 0;
		if (tsd) md.damage>>=1;
		if (md.damage < 0 || md.damage > INT_MAX>>1)
	  	//Overflow prevention, will anyone whine if I cap it to a few billion?
		//Not capped to INT_MAX to give some room for further damage increase.
			md.damage = INT_MAX>>1;
		break;
	case NJ_ZENYNAGE:
		md.damage = skill_get_zeny(skill_num ,skill_lv);
		if (!md.damage) md.damage = 2;
		md.damage = md.damage + rand()%md.damage;
		if (is_boss(target))
			md.damage=md.damage/3;
		else if (tsd)
			md.damage=md.damage/2;
		break;
	case GS_FLING:
		md.damage = sd?sd->status.job_level:status_get_lv(src);
		break;
	case HVAN_EXPLOSION:	//[orn]
		md.damage = sstatus->max_hp * (50 + 50 * skill_lv) / 100 ;
		break ;
	case ASC_BREAKER:
		md.damage = 500+rand()%500 + 5*skill_lv * sstatus->int_;
		nk|=NK_IGNORE_FLEE|NK_NO_ELEFIX; //These two are not properties of the weapon based part.
		break;
	case HW_GRAVITATION:
		md.damage = 200+200*skill_lv;
		md.dmotion = 0; //No flinch animation.
		break;
	case NPC_EVILLAND:
		md.damage = (skill_lv>6)?666:skill_lv*100;
		break;
	}

	if (nk&NK_SPLASHSPLIT){ // Divide ATK among targets
		if(mflag>0)
			md.damage/= mflag;
		else
			ShowError("0 enemies targeted by %d:%s, divide per 0 avoided!\n", skill_num, skill_get_name(skill_num));
	}

	damage_div_fix(md.damage, md.div_);
	
	if (!(nk&NK_IGNORE_FLEE))
	{
		struct status_change *sc = status_get_sc(target);
		i = 0; //Temp for "hit or no hit"
		if(sc && sc->opt1 && sc->opt1 != OPT1_STONEWAIT)
			i = 1;
		else {
			short
				flee = tstatus->flee,
				hitrate=80; //Default hitrate

			if(battle_config.agi_penalty_type && 
				battle_config.agi_penalty_target&target->type)
			{	
				unsigned char attacker_count; //256 max targets should be a sane max
				attacker_count = unit_counttargeted(target,battle_config.agi_penalty_count_lv);
				if(attacker_count >= battle_config.agi_penalty_count)
				{
					if (battle_config.agi_penalty_type == 1)
						flee = (flee * (100 - (attacker_count - (battle_config.agi_penalty_count - 1))*battle_config.agi_penalty_num))/100;
					else //asume type 2: absolute reduction
						flee -= (attacker_count - (battle_config.agi_penalty_count - 1))*battle_config.agi_penalty_num;
					if(flee < 1) flee = 1;
				}
			}

			hitrate+= sstatus->hit - flee;
			hitrate = cap_value(hitrate, battle_config.min_hitrate, battle_config.max_hitrate);

			if(rand()%100 < hitrate)
				i = 1;
		}
		if (!i) {
			md.damage = 0;
			md.dmg_lv=ATK_FLEE;
		}
	}

	if( md.damage && tsd && !(nk&NK_NO_CARDFIX_DEF) )
	{
		int cardfix = 10000;
		int race2 = status_get_race2(src);
		if (!(nk&NK_NO_ELEFIX))
			cardfix=cardfix*(100-tsd->subele[s_ele])/100;
		cardfix=cardfix*(100-tsd->subsize[sstatus->size])/100;
		cardfix=cardfix*(100-tsd->subrace2[race2])/100;
		cardfix=cardfix*(100-tsd->subrace[sstatus->race])/100;
		cardfix=cardfix*(100-tsd->subrace[is_boss(src)?RC_BOSS:RC_NONBOSS])/100;
		if( sstatus->race != RC_DEMIHUMAN )
			cardfix=cardfix*(100-tsd->subrace[RC_NONDEMIHUMAN])/100;

		cardfix=cardfix*(100-tsd->misc_def_rate)/100;
		if(md.flag&BF_SHORT)
			cardfix=cardfix*(100-tsd->near_attack_def_rate)/100;
		else	// BF_LONG (there's no other choice)
			cardfix=cardfix*(100-tsd->long_attack_def_rate)/100;

		if (cardfix != 10000)
			md.damage=md.damage*cardfix/10000;
	}

	if (sd && (i = pc_skillatk_bonus(sd, skill_num)))
		md.damage += md.damage*i/100;

	if(md.damage < 0)
		md.damage = 0;
	else if(md.damage && tstatus->mode&MD_PLANT)
		md.damage = 1;

	if(!(nk&NK_NO_ELEFIX))
		md.damage=battle_attr_fix(src, target, md.damage, s_ele, tstatus->def_ele, tstatus->ele_lv);

	md.damage=battle_calc_damage(src,target,md.damage,md.div_,skill_num,skill_lv,md.flag);
	if (map_flag_gvg2(target->m))
		md.damage=battle_calc_gvg_damage(src,target,md.damage,md.div_,skill_num,skill_lv,md.flag);

	if (skill_num == NJ_ZENYNAGE && sd)
	{	//Time to Pay Up.
		if ( md.damage > sd->status.zeny )
			md.damage=sd->status.zeny;
		pc_payzeny(sd, md.damage);
	}

	return md;
}
/*==========================================
 * ƒ_ƒ??[ƒWŒvŽZˆêŠ‡?ˆ—?—p
 *------------------------------------------*/
struct Damage battle_calc_attack(int attack_type,struct block_list *bl,struct block_list *target,int skill_num,int skill_lv,int count)
{
	struct Damage d;
	switch(attack_type) {
	case BF_WEAPON: d = battle_calc_weapon_attack(bl,target,skill_num,skill_lv,count); break;
	case BF_MAGIC:  d = battle_calc_magic_attack(bl,target,skill_num,skill_lv,count);  break;
	case BF_MISC:   d = battle_calc_misc_attack(bl,target,skill_num,skill_lv,count);   break;
	default:
		ShowError("battle_calc_attack: unknown attack type! %d\n",attack_type);
		memset(&d,0,sizeof(d));
		break;
	}
	if (d.damage + d.damage2 < 1)
	{	//Miss/Absorbed
		//Weapon attacks should go through to cause additional effects.
		if (d.dmg_lv != ATK_LUCKY && attack_type&(BF_MAGIC|BF_MISC))
			d.dmg_lv = ATK_FLEE;
		d.dmotion = 0;
	}
	return d;
}

//Calculates BF_WEAPON returned damage.
int battle_calc_return_damage(struct block_list* bl, int damage, int flag)
{
	struct map_session_data* sd = NULL;
	int rdamage = 0;

	sd = BL_CAST(BL_PC, bl);

	//Bounces back part of the damage.
	if (flag & BF_SHORT) {
		struct status_change* sc;
		if (sd && sd->short_weapon_damage_return)
		{
			rdamage += damage * sd->short_weapon_damage_return / 100;
			if(rdamage < 1) rdamage = 1;
		}
		sc = status_get_sc(bl);
		if (sc && sc->data[SC_REFLECTSHIELD])
		{
			rdamage += damage * sc->data[SC_REFLECTSHIELD]->val2 / 100;
			if (rdamage < 1) rdamage = 1;
		}
	} else {
		if (sd && sd->long_weapon_damage_return)
		{
			rdamage += damage * sd->long_weapon_damage_return / 100;
			if (rdamage < 1) rdamage = 1;
		}
	}
	return rdamage;
}

void battle_drain(TBL_PC *sd, struct block_list *tbl, int rdamage, int ldamage, int race, int boss)
{
	struct weapon_data *wd;
	int type, thp = 0, tsp = 0, rhp = 0, rsp = 0, hp, sp, i, *damage;
	for (i = 0; i < 4; i++) {
		//First two iterations: Right hand
		if (i < 2) { wd = &sd->right_weapon; damage = &rdamage; }
		else { wd = &sd->left_weapon; damage = &ldamage; }
		if (*damage <= 0) continue;
		//First and Third iterations: race, other two boss/nonboss state
		if (i == 0 || i == 2) 
			type = race;
		else
			type = boss?RC_BOSS:RC_NONBOSS;
		
		hp = wd->hp_drain[type].value;
		if (wd->hp_drain[type].rate)
			hp += battle_calc_drain(*damage, wd->hp_drain[type].rate, wd->hp_drain[type].per);

		sp = wd->sp_drain[type].value;
		if (wd->sp_drain[type].rate)
			sp += battle_calc_drain(*damage, wd->sp_drain[type].rate, wd->sp_drain[type].per);

		if (hp) {
			if (wd->hp_drain[type].type)
				rhp += hp;
			thp += hp;
		}
		if (sp) {
			if (wd->sp_drain[type].type)
				rsp += sp;
			tsp += sp;
		}
	}

	if (sd->sp_vanish_rate && rand()%1000 < sd->sp_vanish_rate)
		status_percent_damage(&sd->bl, tbl, 0, (unsigned char)sd->sp_vanish_per, false);
	if (!thp && !tsp) return;

	status_heal(&sd->bl, thp, tsp, battle_config.show_hp_sp_drain?3:1);
	
	if (rhp || rsp)
		status_zap(tbl, rhp, rsp);
}

/*==========================================
 * ’Ê?í?UŒ‚?ˆ—?‚Ü‚Æ‚ß
 *------------------------------------------*/
enum damage_lv battle_weapon_attack(struct block_list* src, struct block_list* target, unsigned int tick, int flag)
{
	struct map_session_data *sd = NULL, *tsd = NULL;
	struct status_data *sstatus, *tstatus;
	struct status_change *sc, *tsc;
	int damage,rdamage=0,rdelay=0;
	int skillv;
	struct Damage wd;

	nullpo_retr(ATK_NONE, src);
	nullpo_retr(ATK_NONE, target);

	if (src->prev == NULL || target->prev == NULL)
		return ATK_NONE;

	sd = BL_CAST(BL_PC, src);
	tsd = BL_CAST(BL_PC, target);

	sstatus = status_get_status_data(src);
	tstatus = status_get_status_data(target);

	sc = status_get_sc(src);
	tsc = status_get_sc(target);

	if (sc && !sc->count) //Avoid sc checks when there's none to check for. [Skotlex]
		sc = NULL;
	if (tsc && !tsc->count)
		tsc = NULL;
	
	if (sd)
	{
		sd->state.arrow_atk = (sd->status.weapon == W_BOW || (sd->status.weapon >= W_REVOLVER && sd->status.weapon <= W_GRENADE));
		if (sd->state.arrow_atk)
		{
			int index = sd->equip_index[EQI_AMMO];
			if (index<0) {
				clif_arrow_fail(sd,0);
				return ATK_NONE;
			}
			//Ammo check by Ishizu-chan
			if (sd->inventory_data[index])
			switch (sd->status.weapon) {
			case W_BOW:
				if (sd->inventory_data[index]->look != A_ARROW) {
					clif_arrow_fail(sd,0);
					return ATK_NONE;
				}
			break;
			case W_REVOLVER:
			case W_RIFLE:
			case W_GATLING:
			case W_SHOTGUN:
				if (sd->inventory_data[index]->look != A_BULLET) {
					clif_arrow_fail(sd,0);
					return ATK_NONE;
				}
			break;
			case W_GRENADE:
				if (sd->inventory_data[index]->look != A_GRENADE) {
					clif_arrow_fail(sd,0);
					return ATK_NONE;
				}
			break;
			}
		}
	}

	if (sc && sc->data[SC_CLOAKING] && !(sc->data[SC_CLOAKING]->val4&2))
		status_change_end(src,SC_CLOAKING,-1);

	//Check for counter attacks that block your attack. [Skotlex]
	if(tsc)
	{
		if(tsc->data[SC_AUTOCOUNTER] &&
			status_check_skilluse(target, src, KN_AUTOCOUNTER, 1)
		)	{
			int dir = map_calc_dir(target,src->x,src->y);
			int t_dir = unit_getdir(target);
			int dist = distance_bl(src, target);
			if(dist <= 0 || (!map_check_dir(dir,t_dir) && dist <= tstatus->rhw.range+1))
			{
				int skilllv = tsc->data[SC_AUTOCOUNTER]->val1;
				clif_skillcastcancel(target); //Remove the casting bar. [Skotlex]
				clif_damage(src, target, tick, sstatus->amotion, 1, 0, 1, 0, 0); //Display MISS.
				status_change_end(target,SC_AUTOCOUNTER,-1);
				skill_attack(BF_WEAPON,target,target,src,KN_AUTOCOUNTER,skilllv,tick,0);
				return ATK_NONE;
			}
		}
		if (tsc->data[SC_BLADESTOP_WAIT] && !is_boss(src)) {
			int skilllv = tsc->data[SC_BLADESTOP_WAIT]->val1;
			int duration = skill_get_time2(MO_BLADESTOP,skilllv);
			status_change_end(target, SC_BLADESTOP_WAIT, -1);
			if(sc_start4(src, SC_BLADESTOP, 100, sd?pc_checkskill(sd, MO_BLADESTOP):5, 0, 0, target->id, duration))
		  	{	//Target locked.
				clif_damage(src, target, tick, sstatus->amotion, 1, 0, 1, 0, 0); //Display MISS.
				clif_bladestop(target, src->id, 1);
				sc_start4(target, SC_BLADESTOP, 100, skilllv, 0, 0, src->id, duration);
				return ATK_NONE;
			}
		}
	}

	if(sd && (skillv = pc_checkskill(sd,MO_TRIPLEATTACK)) > 0)
	{
		int triple_rate= 30 - skillv; //Base Rate
		if (sc && sc->data[SC_SKILLRATE_UP] && sc->data[SC_SKILLRATE_UP]->val1 == MO_TRIPLEATTACK)
		{
			triple_rate+= triple_rate*(sc->data[SC_SKILLRATE_UP]->val2)/100;
			status_change_end(src,SC_SKILLRATE_UP,-1);
		}
		if (rand()%100 < triple_rate)
			//FIXME: invalid return type!
			return (damage_lv)skill_attack(BF_WEAPON,src,src,target,MO_TRIPLEATTACK,skillv,tick,0);
	}

	if (sc)
	{
		if (sc->data[SC_SACRIFICE])
			//FIXME: invalid return type!
			return (damage_lv)skill_attack(BF_WEAPON,src,src,target,PA_SACRIFICE,sc->data[SC_SACRIFICE]->val1,tick,0);
		if (sc->data[SC_MAGICALATTACK])
			//FIXME: invalid return type!
			return (damage_lv)skill_attack(BF_MAGIC,src,src,target,NPC_MAGICALATTACK,sc->data[SC_MAGICALATTACK]->val1,tick,0);
	}

	wd = battle_calc_weapon_attack(src, target, 0, 0, flag);

	if (sd && sd->state.arrow_atk) //Consume arrow.
		battle_consume_ammo(sd, 0, 0);

	damage = wd.damage + wd.damage2;
	if( damage > 0 && src != target )
	{
		rdamage = battle_calc_return_damage(target, damage, wd.flag);
		if( rdamage > 0 )
		{
			rdelay = clif_damage(src, src, tick, wd.amotion, sstatus->dmotion, rdamage, 1, 4, 0);
			//Use Reflect Shield to signal this kind of skill trigger. [Skotlex]
			skill_additional_effect(target,src,CR_REFLECTSHIELD,1,BF_WEAPON|BF_SHORT|BF_NORMAL,tick);
		}
	}

	wd.dmotion = clif_damage(src, target, tick, wd.amotion, wd.dmotion, wd.damage, wd.div_ , wd.type, wd.damage2);

	if (sd && sd->splash_range > 0 && damage > 0)
		skill_castend_damage_id(src, target, 0, 1, tick, 0);

	map_freeblock_lock();

	battle_delay_damage(tick, wd.amotion, src, target, wd.flag, 0, 0, damage, wd.dmg_lv, wd.dmotion);

	if (sc && sc->data[SC_AUTOSPELL] && rand()%100 < sc->data[SC_AUTOSPELL]->val4) {
		int sp = 0;
		int skillid = sc->data[SC_AUTOSPELL]->val2;
		int skilllv = sc->data[SC_AUTOSPELL]->val3;
		int i = rand()%100;
		if (sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_SAGE)
			i = 0; //Max chance, no skilllv reduction. [Skotlex]
		if (i >= 50) skilllv -= 2;
		else if (i >= 15) skilllv--;
		if (skilllv < 1) skilllv = 1;
		sp = skill_get_sp(skillid,skilllv) * 2 / 3;

		if (status_charge(src, 0, sp)) {
			switch (skill_get_casttype(skillid)) {
				case CAST_GROUND:
					skill_castend_pos2(src, target->x, target->y, skillid, skilllv, tick, flag);
					break;
				case CAST_NODAMAGE:
					skill_castend_nodamage_id(src, target, skillid, skilllv, tick, flag);
					break;
				case CAST_DAMAGE:
					skill_castend_damage_id(src, target, skillid, skilllv, tick, flag);
					break;
			}
		}
	}
	if (sd) {
		if (wd.flag & BF_WEAPON && src != target && damage > 0) {
			if (battle_config.left_cardfix_to_right)
				battle_drain(sd, target, wd.damage, wd.damage, tstatus->race, is_boss(target));
			else
				battle_drain(sd, target, wd.damage, wd.damage2, tstatus->race, is_boss(target));
		}
	}
	if (rdamage > 0) { //By sending attack type "none" skill_additional_effect won't be invoked. [Skotlex]
		if(tsd && src != target)
			battle_drain(tsd, src, rdamage, rdamage, sstatus->race, is_boss(src));
		battle_delay_damage(tick, wd.amotion, target, src, 0, 0, 0, rdamage, ATK_DEF, rdelay);
	}

	if (tsc) {
		if (tsc->data[SC_POISONREACT] && 
			(rand()%100 < tsc->data[SC_POISONREACT]->val3
			|| sstatus->def_ele == ELE_POISON) &&
//			check_distance_bl(src, target, tstatus->rhw.range+1) && Doesn't checks range! o.O;
			status_check_skilluse(target, src, TF_POISON, 0)
		) {	//Poison React
			struct status_change_entry *sce = tsc->data[SC_POISONREACT];
			if (sstatus->def_ele == ELE_POISON) {
				sce->val2 = 0;
				skill_attack(BF_WEAPON,target,target,src,AS_POISONREACT,sce->val1,tick,0);
			} else {
				skill_attack(BF_WEAPON,target,target,src,TF_POISON, 5, tick, 0);
				--sce->val2;
			}
			if (sce->val2 <= 0)
				status_change_end(target, SC_POISONREACT, -1);
		}
	}
	map_freeblock_unlock();
	return wd.dmg_lv;
}

int battle_check_undead(int race,int element)
{
	if(battle_config.undead_detect_type == 0) {
		if(element == ELE_UNDEAD)
			return 1;
	}
	else if(battle_config.undead_detect_type == 1) {
		if(race == RC_UNDEAD)
			return 1;
	}
	else {
		if(element == ELE_UNDEAD || race == RC_UNDEAD)
			return 1;
	}
	return 0;
}

//Returns the upmost level master starting with the given object
struct block_list* battle_get_master(struct block_list *src)
{
	struct block_list *prev; //Used for infinite loop check (master of yourself?)
	do {
		prev = src;
		switch (src->type) {
			case BL_PET:
				if (((TBL_PET*)src)->msd)
					src = (struct block_list*)((TBL_PET*)src)->msd;
				break;
			case BL_MOB:
				if (((TBL_MOB*)src)->master_id)
					src = map_id2bl(((TBL_MOB*)src)->master_id);
				break;
			case BL_HOM:
				if (((TBL_HOM*)src)->master)
					src = (struct block_list*)((TBL_HOM*)src)->master;
				break;
			case BL_MER:
				if (((TBL_MER*)src)->master)
					src = (struct block_list*)((TBL_MER*)src)->master;
				break;
			case BL_SKILL:
				if (((TBL_SKILL*)src)->group && ((TBL_SKILL*)src)->group->src_id)
					src = map_id2bl(((TBL_SKILL*)src)->group->src_id);
				break;
		}
	} while (src && src != prev);
	return prev;
}

/*==========================================
 * Checks the state between two targets (rewritten by Skotlex)
 * (enemy, friend, party, guild, etc)
 * See battle.h for possible values/combinations
 * to be used here (BCT_* constants)
 * Return value is:
 * 1: flag holds true (is enemy, party, etc)
 * -1: flag fails
 * 0: Invalid target (non-targetable ever)
 *------------------------------------------*/
int battle_check_target( struct block_list *src, struct block_list *target,int flag)
{
	int m,state = 0; //Initial state none
	int strip_enemy = 1; //Flag which marks whether to remove the BCT_ENEMY status if it's also friend/ally.
	struct block_list *s_bl = src, *t_bl = target;

	nullpo_retr(0, src);
	nullpo_retr(0, target);

	m = target->m;

	//t_bl/s_bl hold the 'master' of the attack, while src/target are the actual
	//objects involved.
	if( (t_bl = battle_get_master(target)) == NULL )
		t_bl = target;

	if( (s_bl = battle_get_master(src)) == NULL )
		s_bl = src;

	switch( target->type )
	{ // Checks on actual target
		case BL_PC:
			if (((TBL_PC*)target)->invincible_timer != -1 || pc_isinvisible((TBL_PC*)target))
				return -1; //Cannot be targeted yet.
			break;
		case BL_MOB:
			if((((TBL_MOB*)target)->special_state.ai == 2 || //Marine Spheres
				(((TBL_MOB*)target)->special_state.ai == 3 && battle_config.summon_flora&1)) && //Floras
				s_bl->type == BL_PC && src->type != BL_MOB)
			{	//Targettable by players
				state |= BCT_ENEMY;
				strip_enemy = 0;
			}
			break;
		case BL_SKILL:
		{
			TBL_SKILL *su = (TBL_SKILL*)target;
			if( !su->group )
				return 0;
			if( skill_get_inf2(su->group->skill_id)&INF2_TRAP )
			{ //Only a few skills can target traps...
				switch( battle_getcurrentskill(src) )
				{
					case MA_REMOVETRAP:
					case HT_REMOVETRAP:
					case AC_SHOWER:
					case MA_SHOWER:
					case WZ_SIGHTRASHER:
					case WZ_SIGHTBLASTER:
					case SM_MAGNUM:
					case MS_MAGNUM:
						state |= BCT_ENEMY;
						strip_enemy = 0;
						break;
					default:
						return 0;
				}
			} else if (su->group->skill_id==WZ_ICEWALL)
			{
				state |= BCT_ENEMY;
				strip_enemy = 0;
			} else	//Excepting traps and icewall, you should not be able to target skills.
				return 0;
		}
			break;
		//Valid targets with no special checks here.
		case BL_MER:
		case BL_HOM:
			break;
		//All else not specified is an invalid target.
		default:	
			return 0;
	}

	switch( t_bl->type )
	{	//Checks on target master
		case BL_PC:
		{
			struct map_session_data *sd;
			if( t_bl == s_bl ) break;
			sd = BL_CAST(BL_PC, t_bl);

			if( sd->state.monster_ignore && flag&BCT_ENEMY )
				return 0; // Global inminuty only to Attacks
			if( sd->status.karma && s_bl->type == BL_PC && ((TBL_PC*)s_bl)->status.karma )
				state |= BCT_ENEMY; // Characters with bad karma may fight amongst them
			if( sd->state.killable ) {
				state |= BCT_ENEMY; // Everything can kill it
				strip_enemy = 0;
			}
			break;
		}
		case BL_MOB:
		{
			struct mob_data *md = BL_CAST(BL_MOB, t_bl);

			if( !((agit_flag || agit2_flag) && map[m].flag.gvg_castle) && md->guardian_data && md->guardian_data->guild_id )
				return 0; // Disable guardians/emperiums owned by Guilds on non-woe times.
			break;
		}
	}

	switch( src->type )
  	{	//Checks on actual src type
		case BL_PET:
			if (t_bl->type != BL_MOB && flag&BCT_ENEMY)
				return 0; //Pet may not attack non-mobs.
			if (t_bl->type == BL_MOB && ((TBL_MOB*)t_bl)->guardian_data && flag&BCT_ENEMY)
				return 0; //pet may not attack Guardians/Emperium
			break;
		case BL_SKILL:
		{
			struct skill_unit *su = (struct skill_unit *)src;
			if (!su->group)
				return 0;

			if (su->group->src_id == target->id)
			{
				int inf2;
				inf2 = skill_get_inf2(su->group->skill_id);
				if (inf2&INF2_NO_TARGET_SELF)
					return -1;
				if (inf2&INF2_TARGET_SELF)
					return 1;
			}
			break;
		}
	}

	switch( s_bl->type )
	{	//Checks on source master
		case BL_PC:
		{
			struct map_session_data *sd = BL_CAST(BL_PC, s_bl);
			if( s_bl != t_bl )
			{
				if( sd->state.killer )
				{
					state |= BCT_ENEMY; // Can kill anything
					strip_enemy = 0;
				}
				else if( sd->duel_group && !((!battle_config.duel_allow_pvp && map[m].flag.pvp) || (!battle_config.duel_allow_gvg && map_flag_gvg(m))) )
		  		{
					if( t_bl->type == BL_PC && (sd->duel_group == ((TBL_PC*)t_bl)->duel_group) )
						return (BCT_ENEMY&flag)?1:-1; // Duel targets can ONLY be your enemy, nothing else.
					else
						return 0; // You can't target anything out of your duel
				}
			}
			if (map_flag_gvg(m) && !sd->status.guild_id &&
				t_bl->type == BL_MOB && ((TBL_MOB*)t_bl)->guardian_data)
				return 0; //If you don't belong to a guild, can't target guardians/emperium.
			if (t_bl->type != BL_PC)
				state |= BCT_ENEMY; //Natural enemy.
			break;
		}
		case BL_MOB:
		{
			struct mob_data *md = BL_CAST(BL_MOB, s_bl);
			if( !((agit_flag || agit2_flag) && map[m].flag.gvg_castle) && md->guardian_data && md->guardian_data->guild_id )
				return 0; // Disable guardians/emperium owned by Guilds on non-woe times.

			if( !md->special_state.ai )
			{ //Normal mobs.
				if( t_bl->type == BL_MOB && !((TBL_MOB*)t_bl)->special_state.ai )
					state |= BCT_PARTY; //Normal mobs with no ai are friends.
				else
					state |= BCT_ENEMY; //However, all else are enemies.
			}
			else
			{
				if( t_bl->type == BL_MOB && !((TBL_MOB*)t_bl)->special_state.ai )
					state |= BCT_ENEMY; //Natural enemy for AI mobs are normal mobs.
			}
			break;
		}
		default:
		//Need some sort of default behaviour for unhandled types.
			if (t_bl->type != s_bl->type)
				state |= BCT_ENEMY;
			break;
	}
	
	if( (flag&BCT_ALL) == BCT_ALL )
	{ //All actually stands for all attackable chars 
		if( target->type&BL_CHAR )
			return 1;
		else
			return -1;
	}
	else if( flag == BCT_NOONE ) //Why would someone use this? no clue.
		return -1;
	
	if( t_bl == s_bl )
	{ //No need for further testing.
		state |= BCT_SELF|BCT_PARTY|BCT_GUILD;
		if( state&BCT_ENEMY && strip_enemy )
			state&=~BCT_ENEMY;
		return (flag&state)?1:-1;
	}
	
	if( map_flag_vs(m) )
	{ //Check rivalry settings.
		if (flag&(BCT_PARTY|BCT_ENEMY)) {
			int s_party = status_get_party_id(s_bl);
			if (
				!(map[m].flag.pvp && map[m].flag.pvp_noparty) &&
				!(map_flag_gvg(m) && map[m].flag.gvg_noparty) &&
				s_party && s_party == status_get_party_id(t_bl)
			)
				state |= BCT_PARTY;
			else
				state |= BCT_ENEMY;
		}
		if (flag&(BCT_GUILD|BCT_ENEMY)) {
			int s_guild = status_get_guild_id(s_bl);
			int t_guild = status_get_guild_id(t_bl);
			if( !(map[m].flag.pvp && map[m].flag.pvp_noguild) && s_guild && t_guild && (s_guild == t_guild || guild_isallied(s_guild, t_guild)) )
				state |= BCT_GUILD;
			else
				state |= BCT_ENEMY;
		}
		if (state&BCT_ENEMY && battle_config.pk_mode && !map_flag_gvg(m) &&
			s_bl->type == BL_PC && t_bl->type == BL_PC)
		{	//Prevent novice engagement on pk_mode (feature by Valaris)
			TBL_PC *sd = (TBL_PC*)s_bl, *sd2 = (TBL_PC*)t_bl;
			if (
				(sd->class_&MAPID_UPPERMASK) == MAPID_NOVICE ||
				(sd2->class_&MAPID_UPPERMASK) == MAPID_NOVICE ||
				(int)sd->status.base_level < battle_config.pk_min_level ||
			  	(int)sd2->status.base_level < battle_config.pk_min_level ||
				(battle_config.pk_level_range && abs((int)sd->status.base_level - (int)sd2->status.base_level) > battle_config.pk_level_range)
			)
				state&=~BCT_ENEMY;
		}
	} else { //Non pvp/gvg, check party/guild settings.
		if (flag&BCT_PARTY || state&BCT_ENEMY) {
			int s_party = status_get_party_id(s_bl);
			if(s_party && s_party == status_get_party_id(t_bl))
				state |= BCT_PARTY;
		}
		if (flag&BCT_GUILD || state&BCT_ENEMY) {
			int s_guild = status_get_guild_id(s_bl);
			int t_guild = status_get_guild_id(t_bl);
			if(s_guild && t_guild && (s_guild == t_guild || guild_isallied(s_guild, t_guild)))
				state |= BCT_GUILD;
		}
	}
	
	if( !state ) //If not an enemy, nor a guild, nor party, nor yourself, it's neutral.
		state = BCT_NEUTRAL;
	//Alliance state takes precedence over enemy one.
	else if( state&BCT_ENEMY && strip_enemy && state&(BCT_SELF|BCT_PARTY|BCT_GUILD) )
		state&=~BCT_ENEMY;

	return (flag&state)?1:-1;
}
/*==========================================
 * ŽË’ö”»’è
 *------------------------------------------*/
bool battle_check_range(struct block_list *src, struct block_list *bl, int range)
{
	int d;
	nullpo_retr(false, src);
	nullpo_retr(false, bl);

	if( src->m != bl->m )
		return false;

	if( !check_distance_bl(src, bl, range) )
		return false;

	if( (d = distance_bl(src, bl)) < 2 )
		return true;  // No need for path checking.

	if( d > AREA_SIZE )
		return false; // Avoid targetting objects beyond your range of sight.

	return path_search_long(NULL,src->m,src->x,src->y,bl->x,bl->y,CELL_CHKWALL);
}

static const struct _battle_data {
	const char* str;
	int* val;
	int defval;
	int min;
	int max;
} battle_data[] = {
	{ "warp_point_debug",                   &battle_config.warp_point_debug,                0,      0,      1,              },
	{ "enable_critical",                    &battle_config.enable_critical,                 BL_PC,  BL_NUL, BL_ALL,         },
	{ "mob_critical_rate",                  &battle_config.mob_critical_rate,               100,    0,      INT_MAX,        },
	{ "critical_rate",                      &battle_config.critical_rate,                   100,    0,      INT_MAX,        },
	{ "enable_baseatk",                     &battle_config.enable_baseatk,                  BL_PC|BL_HOM, BL_NUL, BL_ALL,   },
	{ "enable_perfect_flee",                &battle_config.enable_perfect_flee,             BL_PC|BL_PET, BL_NUL, BL_ALL,   },
	{ "casting_rate",                       &battle_config.cast_rate,                       100,    0,      INT_MAX,        },
	{ "delay_rate",                         &battle_config.delay_rate,                      100,    0,      INT_MAX,        },
	{ "delay_dependon_dex",                 &battle_config.delay_dependon_dex,              0,      0,      1,              },
	{ "delay_dependon_agi",                 &battle_config.delay_dependon_agi,              0,      0,      1,              },
	{ "skill_delay_attack_enable",          &battle_config.sdelay_attack_enable,            0,      0,      1,              },
	{ "left_cardfix_to_right",              &battle_config.left_cardfix_to_right,           0,      0,      1,              },
	{ "skill_add_range",                    &battle_config.skill_add_range,                 0,      0,      INT_MAX,        },
	{ "skill_out_range_consume",            &battle_config.skill_out_range_consume,         1,      0,      1,              },
	{ "skillrange_by_distance",             &battle_config.skillrange_by_distance,          ~BL_PC, BL_NUL, BL_ALL,         },
	{ "skillrange_from_weapon",             &battle_config.use_weapon_skill_range,          ~BL_PC, BL_NUL, BL_ALL,         },
	{ "player_damage_delay_rate",           &battle_config.pc_damage_delay_rate,            100,    0,      INT_MAX,        },
	{ "defunit_not_enemy",                  &battle_config.defnotenemy,                     0,      0,      1,              },
	{ "gvg_traps_target_all",               &battle_config.vs_traps_bctall,                 BL_PC,  BL_NUL, BL_ALL,         },
	{ "traps_setting",                      &battle_config.traps_setting,                   0,      0,      1,              },
	{ "summon_flora_setting",               &battle_config.summon_flora,                    1|2,    0,      1|2,            },
	{ "clear_skills_on_death",              &battle_config.clear_unit_ondeath,              BL_NUL, BL_NUL, BL_ALL,         },
	{ "clear_skills_on_warp",               &battle_config.clear_unit_onwarp,               BL_ALL, BL_NUL, BL_ALL,         },
	{ "random_monster_checklv",             &battle_config.random_monster_checklv,          1,      0,      1,              },
	{ "attribute_recover",                  &battle_config.attr_recover,                    1,      0,      1,              },
	{ "flooritem_lifetime",                 &battle_config.flooritem_lifetime,              60000,  1000,   INT_MAX,        },
	{ "item_auto_get",                      &battle_config.item_auto_get,                   0,      0,      1,              },
	{ "item_first_get_time",                &battle_config.item_first_get_time,             3000,   0,      INT_MAX,        },
	{ "item_second_get_time",               &battle_config.item_second_get_time,            1000,   0,      INT_MAX,        },
	{ "item_third_get_time",                &battle_config.item_third_get_time,             1000,   0,      INT_MAX,        },
	{ "mvp_item_first_get_time",            &battle_config.mvp_item_first_get_time,         10000,  0,      INT_MAX,        },
	{ "mvp_item_second_get_time",           &battle_config.mvp_item_second_get_time,        10000,  0,      INT_MAX,        },
	{ "mvp_item_third_get_time",            &battle_config.mvp_item_third_get_time,         2000,   0,      INT_MAX,        },
	{ "drop_rate0item",                     &battle_config.drop_rate0item,                  0,      0,      1,              },
	{ "base_exp_rate",                      &battle_config.base_exp_rate,                   100,    0,      INT_MAX,        },
	{ "job_exp_rate",                       &battle_config.job_exp_rate,                    100,    0,      INT_MAX,        },
	{ "pvp_exp",                            &battle_config.pvp_exp,                         1,      0,      1,              },
	{ "death_penalty_type",                 &battle_config.death_penalty_type,              0,      0,      2,              },
	{ "death_penalty_base",                 &battle_config.death_penalty_base,              0,      0,      INT_MAX,        },
	{ "death_penalty_job",                  &battle_config.death_penalty_job,               0,      0,      INT_MAX,        },
	{ "zeny_penalty",                       &battle_config.zeny_penalty,                    0,      0,      INT_MAX,        },
	{ "hp_rate",                            &battle_config.hp_rate,                         100,    1,      INT_MAX,        },
	{ "sp_rate",                            &battle_config.sp_rate,                         100,    1,      INT_MAX,        },
	{ "restart_hp_rate",                    &battle_config.restart_hp_rate,                 0,      0,      100,            },
	{ "restart_sp_rate",                    &battle_config.restart_sp_rate,                 0,      0,      100,            },
	{ "guild_aura",                         &battle_config.guild_aura,                      31,     0,      31,             },
	{ "mvp_hp_rate",                        &battle_config.mvp_hp_rate,                     100,    1,      INT_MAX,        },
	{ "mvp_exp_rate",                       &battle_config.mvp_exp_rate,                    100,    0,      INT_MAX,        },
	{ "monster_hp_rate",                    &battle_config.monster_hp_rate,                 100,    1,      INT_MAX,        },
	{ "monster_max_aspd",                   &battle_config.monster_max_aspd,                199,    100,    199,            },
	{ "view_range_rate",                    &battle_config.view_range_rate,                 100,    0,      INT_MAX,        },
	{ "chase_range_rate",                   &battle_config.chase_range_rate,                100,    0,      INT_MAX,        },
	{ "gtb_sc_immunity",                    &battle_config.gtb_sc_immunity,                 50,     0,      INT_MAX,        },
	{ "guild_max_castles",                  &battle_config.guild_max_castles,               0,      0,      INT_MAX,        },
	{ "guild_skill_relog_delay",            &battle_config.guild_skill_relog_delay,         0,      0,      1,              },
	{ "emergency_call",                     &battle_config.emergency_call,                  11,     0,      31,             },
	{ "lowest_gm_level",                    &battle_config.lowest_gm_level,                 1,      0,      99,             },
	{ "atcommand_gm_only",                  &battle_config.atc_gmonly,                      0,      0,      1,              },
	{ "atcommand_spawn_quantity_limit",     &battle_config.atc_spawn_quantity_limit,        100,    0,      INT_MAX,        },
	{ "atcommand_slave_clone_limit",        &battle_config.atc_slave_clone_limit,           25,     0,      INT_MAX,        },
	{ "partial_name_scan",                  &battle_config.partial_name_scan,               0,      0,      1,              },
	{ "gm_all_skill",                       &battle_config.gm_allskill,                     0,      0,      100,            },
	{ "gm_all_equipment",                   &battle_config.gm_allequip,                     0,      0,      100,            },
	{ "gm_skill_unconditional",             &battle_config.gm_skilluncond,                  0,      0,      100,            },
	{ "gm_join_chat",                       &battle_config.gm_join_chat,                    0,      0,      100,            },
	{ "gm_kick_chat",                       &battle_config.gm_kick_chat,                    0,      0,      100,            },
	{ "gm_can_party",                       &battle_config.gm_can_party,                    0,      0,      1,              },
	{ "gm_cant_party_min_lv",               &battle_config.gm_cant_party_min_lv,            20,     0,      100,            },
	{ "player_skillfree",                   &battle_config.skillfree,                       0,      0,      1,              },
	{ "player_skillup_limit",               &battle_config.skillup_limit,                   1,      0,      1,              },
	{ "weapon_produce_rate",                &battle_config.wp_rate,                         100,    0,      INT_MAX,        },
	{ "potion_produce_rate",                &battle_config.pp_rate,                         100,    0,      INT_MAX,        },
	{ "monster_active_enable",              &battle_config.monster_active_enable,           1,      0,      1,              },
	{ "monster_damage_delay_rate",          &battle_config.monster_damage_delay_rate,       100,    0,      INT_MAX,        },
	{ "monster_loot_type",                  &battle_config.monster_loot_type,               0,      0,      1,              },
//	{ "mob_skill_use",                      &battle_config.mob_skill_use,                   1,      0,      1,              }, //Deprecated
	{ "mob_skill_rate",                     &battle_config.mob_skill_rate,                  100,    0,      INT_MAX,        },
	{ "mob_skill_delay",                    &battle_config.mob_skill_delay,                 100,    0,      INT_MAX,        },
	{ "mob_count_rate",                     &battle_config.mob_count_rate,                  100,    0,      INT_MAX,        },
	{ "mob_spawn_delay",                    &battle_config.mob_spawn_delay,                 100,    0,      INT_MAX,        },
	{ "plant_spawn_delay",                  &battle_config.plant_spawn_delay,               100,    0,      INT_MAX,        },
	{ "boss_spawn_delay",                   &battle_config.boss_spawn_delay,                100,    0,      INT_MAX,        },
	{ "no_spawn_on_player",                 &battle_config.no_spawn_on_player,              0,      0,      100,            },
	{ "force_random_spawn",                 &battle_config.force_random_spawn,              0,      0,      1,              },
	{ "slaves_inherit_mode",                &battle_config.slaves_inherit_mode,             2,      0,      3,              },
	{ "slaves_inherit_speed",               &battle_config.slaves_inherit_speed,            3,      0,      3,              },
	{ "summons_trigger_autospells",         &battle_config.summons_trigger_autospells,      1,      0,      1,              },
	{ "pc_damage_walk_delay_rate",          &battle_config.pc_walk_delay_rate,              20,     0,      INT_MAX,        },
	{ "damage_walk_delay_rate",             &battle_config.walk_delay_rate,                 100,    0,      INT_MAX,        },
	{ "multihit_delay",                     &battle_config.multihit_delay,                  80,     0,      INT_MAX,        },
	{ "quest_skill_learn",                  &battle_config.quest_skill_learn,               0,      0,      1,              },
	{ "quest_skill_reset",                  &battle_config.quest_skill_reset,               0,      0,      1,              },
	{ "basic_skill_check",                  &battle_config.basic_skill_check,               1,      0,      1,              },
	{ "guild_emperium_check",               &battle_config.guild_emperium_check,            1,      0,      1,              },
	{ "guild_exp_limit",                    &battle_config.guild_exp_limit,                 50,     0,      99,             },
	{ "player_invincible_time",             &battle_config.pc_invincible_time,              5000,   0,      INT_MAX,        },
	{ "pet_catch_rate",                     &battle_config.pet_catch_rate,                  100,    0,      INT_MAX,        },
	{ "pet_rename",                         &battle_config.pet_rename,                      0,      0,      1,              },
	{ "pet_friendly_rate",                  &battle_config.pet_friendly_rate,               100,    0,      INT_MAX,        },
	{ "pet_hungry_delay_rate",              &battle_config.pet_hungry_delay_rate,           100,    10,     INT_MAX,        },
	{ "pet_hungry_friendly_decrease",       &battle_config.pet_hungry_friendly_decrease,    5,      0,      INT_MAX,        },
	{ "pet_status_support",                 &battle_config.pet_status_support,              0,      0,      1,              },
	{ "pet_attack_support",                 &battle_config.pet_attack_support,              0,      0,      1,              },
	{ "pet_damage_support",                 &battle_config.pet_damage_support,              0,      0,      1,              },
	{ "pet_support_min_friendly",           &battle_config.pet_support_min_friendly,        900,    0,      950,            },
	{ "pet_equip_min_friendly",             &battle_config.pet_equip_min_friendly,          900,    0,      950,            },
	{ "pet_support_rate",                   &battle_config.pet_support_rate,                100,    0,      INT_MAX,        },
	{ "pet_attack_exp_to_master",           &battle_config.pet_attack_exp_to_master,        0,      0,      1,              },
	{ "pet_attack_exp_rate",                &battle_config.pet_attack_exp_rate,             100,    0,      INT_MAX,        },
	{ "pet_lv_rate",                        &battle_config.pet_lv_rate,                     0,      0,      INT_MAX,        },
	{ "pet_max_stats",                      &battle_config.pet_max_stats,                   99,     0,      INT_MAX,        },
	{ "pet_max_atk1",                       &battle_config.pet_max_atk1,                    750,    0,      INT_MAX,        },
	{ "pet_max_atk2",                       &battle_config.pet_max_atk2,                    1000,   0,      INT_MAX,        },
	{ "pet_disable_in_gvg",                 &battle_config.pet_no_gvg,                      0,      0,      1,              },
	{ "skill_min_damage",                   &battle_config.skill_min_damage,                2|4,    0,      1|2|4,          },
	{ "finger_offensive_type",              &battle_config.finger_offensive_type,           0,      0,      1,              },
	{ "heal_exp",                           &battle_config.heal_exp,                        0,      0,      INT_MAX,        },
	{ "resurrection_exp",                   &battle_config.resurrection_exp,                0,      0,      INT_MAX,        },
	{ "shop_exp",                           &battle_config.shop_exp,                        0,      0,      INT_MAX,        },
	{ "max_heal_lv",                        &battle_config.max_heal_lv,                     11,     1,      INT_MAX,        },
	{ "max_heal",                           &battle_config.max_heal,                        9999,   0,      INT_MAX,        },
	{ "combo_delay_rate",                   &battle_config.combo_delay_rate,                100,    0,      INT_MAX,        },
	{ "item_check",                         &battle_config.item_check,                      0,      0,      1,              },
	{ "item_use_interval",                  &battle_config.item_use_interval,               100,    0,      INT_MAX,        },
	{ "wedding_modifydisplay",              &battle_config.wedding_modifydisplay,           0,      0,      1,              },
	{ "wedding_ignorepalette",              &battle_config.wedding_ignorepalette,           0,      0,      1,              },
	{ "xmas_ignorepalette",                 &battle_config.xmas_ignorepalette,              0,      0,      1,              },
	{ "summer_ignorepalette",               &battle_config.summer_ignorepalette,            0,      0,      1,              },
	{ "natural_healhp_interval",            &battle_config.natural_healhp_interval,         6000,   NATURAL_HEAL_INTERVAL, INT_MAX, },
	{ "natural_healsp_interval",            &battle_config.natural_healsp_interval,         8000,   NATURAL_HEAL_INTERVAL, INT_MAX, },
	{ "natural_heal_skill_interval",        &battle_config.natural_heal_skill_interval,     10000,  NATURAL_HEAL_INTERVAL, INT_MAX, },
	{ "natural_heal_weight_rate",           &battle_config.natural_heal_weight_rate,        50,     50,     101             },
	{ "arrow_decrement",                    &battle_config.arrow_decrement,                 1,      0,      2,              },
	{ "max_aspd",                           &battle_config.max_aspd,                        199,    100,    199,            },
	{ "max_walk_speed",                     &battle_config.max_walk_speed,                  300,    100,    100*DEFAULT_WALK_SPEED, },
	{ "max_lv",                             &battle_config.max_lv,                          99,     0,      127,            },
	{ "aura_lv",                            &battle_config.aura_lv,                         99,     0,      INT_MAX,        },
	{ "max_hp",                             &battle_config.max_hp,                          32500,  100,    1000000000,     },
	{ "max_sp",                             &battle_config.max_sp,                          32500,  100,    1000000000,     },
	{ "max_cart_weight",                    &battle_config.max_cart_weight,                 8000,   100,    1000000,        },
	{ "max_parameter",                      &battle_config.max_parameter,                   99,     10,     10000,          },
	{ "max_baby_parameter",                 &battle_config.max_baby_parameter,              80,     10,     10000,          },
	{ "max_def",                            &battle_config.max_def,                         99,     0,      INT_MAX,        },
	{ "over_def_bonus",                     &battle_config.over_def_bonus,                  0,      0,      1000,           },
	{ "skill_log",                          &battle_config.skill_log,                       BL_NUL, BL_NUL, BL_ALL,         },
	{ "battle_log",                         &battle_config.battle_log,                      0,      0,      1,              },
	{ "save_log",                           &battle_config.save_log,                        0,      0,      1,              },
	{ "etc_log",                            &battle_config.etc_log,                         1,      0,      1,              },
	{ "save_clothcolor",                    &battle_config.save_clothcolor,                 1,      0,      1,              },
	{ "undead_detect_type",                 &battle_config.undead_detect_type,              0,      0,      2,              },
	{ "auto_counter_type",                  &battle_config.auto_counter_type,               BL_ALL, BL_NUL, BL_ALL,         },
	{ "min_hitrate",                        &battle_config.min_hitrate,                     5,      0,      100,            },
	{ "max_hitrate",                        &battle_config.max_hitrate,                     100,    0,      100,            },
	{ "agi_penalty_target",                 &battle_config.agi_penalty_target,              BL_PC,  BL_NUL, BL_ALL,         },
	{ "agi_penalty_type",                   &battle_config.agi_penalty_type,                1,      0,      2,              },
	{ "agi_penalty_count",                  &battle_config.agi_penalty_count,               3,      2,      INT_MAX,        },
	{ "agi_penalty_num",                    &battle_config.agi_penalty_num,                 10,     0,      INT_MAX,        },
	{ "agi_penalty_count_lv",               &battle_config.agi_penalty_count_lv,            ATK_FLEE, 0,    INT_MAX,        },
	{ "vit_penalty_target",                 &battle_config.vit_penalty_target,              BL_PC,  BL_NUL, BL_ALL,         },
	{ "vit_penalty_type",                   &battle_config.vit_penalty_type,                1,      0,      2,              },
	{ "vit_penalty_count",                  &battle_config.vit_penalty_count,               3,      2,      INT_MAX,        },
	{ "vit_penalty_num",                    &battle_config.vit_penalty_num,                 5,      0,      INT_MAX,        },
	{ "vit_penalty_count_lv",               &battle_config.vit_penalty_count_lv,            ATK_DEF, 0,     INT_MAX,        },
	{ "weapon_defense_type",                &battle_config.weapon_defense_type,             0,      0,      INT_MAX,        },
	{ "magic_defense_type",                 &battle_config.magic_defense_type,              0,      0,      INT_MAX,        },
	{ "skill_reiteration",                  &battle_config.skill_reiteration,               BL_NUL, BL_NUL, BL_ALL,         },
	{ "skill_nofootset",                    &battle_config.skill_nofootset,                 BL_PC,  BL_NUL, BL_ALL,         },
	{ "player_cloak_check_type",            &battle_config.pc_cloak_check_type,             1,      0,      1|2|4,          },
	{ "monster_cloak_check_type",           &battle_config.monster_cloak_check_type,        4,      0,      1|2|4,          },
	{ "sense_type",                         &battle_config.estimation_type,                 1|2,    0,      1|2,            },
	{ "gvg_eliminate_time",                 &battle_config.gvg_eliminate_time,              7000,   0,      INT_MAX,        },
	{ "gvg_short_attack_damage_rate",       &battle_config.gvg_short_damage_rate,           80,     0,      INT_MAX,        },
	{ "gvg_long_attack_damage_rate",        &battle_config.gvg_long_damage_rate,            80,     0,      INT_MAX,        },
	{ "gvg_weapon_attack_damage_rate",      &battle_config.gvg_weapon_damage_rate,          60,     0,      INT_MAX,        },
	{ "gvg_magic_attack_damage_rate",       &battle_config.gvg_magic_damage_rate,           60,     0,      INT_MAX,        },
	{ "gvg_misc_attack_damage_rate",        &battle_config.gvg_misc_damage_rate,            60,     0,      INT_MAX,        },
	{ "gvg_flee_penalty",                   &battle_config.gvg_flee_penalty,                20,     0,      INT_MAX,        },
	{ "pk_short_attack_damage_rate",        &battle_config.pk_short_damage_rate,            80,     0,      INT_MAX,        },
	{ "pk_long_attack_damage_rate",         &battle_config.pk_long_damage_rate,             70,     0,      INT_MAX,        },
	{ "pk_weapon_attack_damage_rate",       &battle_config.pk_weapon_damage_rate,           60,     0,      INT_MAX,        },
	{ "pk_magic_attack_damage_rate",        &battle_config.pk_magic_damage_rate,            60,     0,      INT_MAX,        },
	{ "pk_misc_attack_damage_rate",         &battle_config.pk_misc_damage_rate,             60,     0,      INT_MAX,        },
	{ "mob_changetarget_byskill",           &battle_config.mob_changetarget_byskill,        0,      0,      1,              },
	{ "attack_direction_change",            &battle_config.attack_direction_change,         BL_ALL, BL_NUL, BL_ALL,         },
	{ "land_skill_limit",                   &battle_config.land_skill_limit,                BL_ALL, BL_NUL, BL_ALL,         },
	{ "monster_class_change_full_recover",  &battle_config.monster_class_change_recover,    1,      0,      1,              },
	{ "produce_item_name_input",            &battle_config.produce_item_name_input,         0x1|0x2, 0,     0x9F,           },
	{ "display_skill_fail",                 &battle_config.display_skill_fail,              2,      0,      1|2|4|8,        },
	{ "chat_warpportal",                    &battle_config.chat_warpportal,                 0,      0,      1,              },
	{ "mob_warp",                           &battle_config.mob_warp,                        0,      0,      1|2|4,          },
	{ "dead_branch_active",                 &battle_config.dead_branch_active,              1,      0,      1,              },
	{ "vending_max_value",                  &battle_config.vending_max_value,               10000000, 1,    MAX_ZENY,       },
	{ "vending_over_max",                   &battle_config.vending_over_max,                1,      0,      1,              },
	{ "show_steal_in_same_party",           &battle_config.show_steal_in_same_party,        0,      0,      1,              },
	{ "party_hp_mode",                      &battle_config.party_hp_mode,                   0,      0,      1,              },
	{ "show_party_share_picker",            &battle_config.party_show_share_picker,         0,      0,      1,              },
	{ "party_update_interval",              &battle_config.party_update_interval,           1000,   100,    INT_MAX,        },
	{ "party_item_share_type",              &battle_config.party_share_type,                0,      0,      1|2|3,          },
	{ "attack_attr_none",                   &battle_config.attack_attr_none,                ~BL_PC, BL_NUL, BL_ALL,         },
	{ "gx_allhit",                          &battle_config.gx_allhit,                       0,      0,      1,              },
	{ "gx_disptype",                        &battle_config.gx_disptype,                     1,      0,      1,              },
	{ "devotion_level_difference",          &battle_config.devotion_level_difference,       10,     0,      INT_MAX,        },
	{ "player_skill_partner_check",         &battle_config.player_skill_partner_check,      1,      0,      1,              },
	{ "hide_GM_session",                    &battle_config.hide_GM_session,                 0,      0,      1,              },
	{ "invite_request_check",               &battle_config.invite_request_check,            1,      0,      1,              },
	{ "skill_removetrap_type",              &battle_config.skill_removetrap_type,           0,      0,      1,              },
	{ "disp_experience",                    &battle_config.disp_experience,                 0,      0,      1,              },
	{ "disp_zeny",                          &battle_config.disp_zeny,                       0,      0,      1,              },
	{ "castle_defense_rate",                &battle_config.castle_defense_rate,             100,    0,      100,            },
	{ "gm_cant_drop_min_lv",                &battle_config.gm_cant_drop_min_lv,             1,      0,      100,            },
	{ "gm_cant_drop_max_lv",                &battle_config.gm_cant_drop_max_lv,             0,      0,      100,            },
	{ "disp_hpmeter",                       &battle_config.disp_hpmeter,                    0,      0,      100,            },
	{ "bone_drop",                          &battle_config.bone_drop,                       0,      0,      2,              },
	{ "buyer_name",                         &battle_config.buyer_name,                      1,      0,      1,              },
	{ "skill_wall_check",                   &battle_config.skill_wall_check,                1,      0,      1,              },
	{ "cell_stack_limit",                   &battle_config.cell_stack_limit,                1,      1,      255,            },
// eAthena additions
	{ "item_logarithmic_drops",             &battle_config.logarithmic_drops,               0,      0,      1,              },
	{ "item_drop_common_min",               &battle_config.item_drop_common_min,            1,      1,      10000,          },
	{ "item_drop_common_max",               &battle_config.item_drop_common_max,            10000,  1,      10000,          },
	{ "item_drop_equip_min",                &battle_config.item_drop_equip_min,             1,      1,      10000,          },
	{ "item_drop_equip_max",                &battle_config.item_drop_equip_max,             10000,  1,      10000,          },
	{ "item_drop_card_min",                 &battle_config.item_drop_card_min,              1,      1,      10000,          },
	{ "item_drop_card_max",                 &battle_config.item_drop_card_max,              10000,  1,      10000,          },
	{ "item_drop_mvp_min",                  &battle_config.item_drop_mvp_min,               1,      1,      10000,          },
	{ "item_drop_mvp_max",                  &battle_config.item_drop_mvp_max,               10000,  1,      10000,          },
	{ "item_drop_heal_min",                 &battle_config.item_drop_heal_min,              1,      1,      10000,          },
	{ "item_drop_heal_max",                 &battle_config.item_drop_heal_max,              10000,  1,      10000,          },
	{ "item_drop_use_min",                  &battle_config.item_drop_use_min,               1,      1,      10000,          },
	{ "item_drop_use_max",                  &battle_config.item_drop_use_max,               10000,  1,      10000,          },
	{ "item_drop_add_min",                  &battle_config.item_drop_adddrop_min,           1,      1,      10000,          },
	{ "item_drop_add_max",                  &battle_config.item_drop_adddrop_max,           10000,  1,      10000,          },
	{ "item_drop_treasure_min",             &battle_config.item_drop_treasure_min,          1,      1,      10000,          },
	{ "item_drop_treasure_max",             &battle_config.item_drop_treasure_max,          10000,  1,      10000,          },
	{ "item_rate_mvp",                      &battle_config.item_rate_mvp,                   100,    0,      1000000,        },
	{ "item_rate_common",                   &battle_config.item_rate_common,                100,    0,      1000000,        },
	{ "item_rate_common_boss",              &battle_config.item_rate_common_boss,           100,    0,      1000000,        },
	{ "item_rate_equip",                    &battle_config.item_rate_equip,                 100,    0,      1000000,        },
	{ "item_rate_equip_boss",               &battle_config.item_rate_equip_boss,            100,    0,      1000000,        },
	{ "item_rate_card",                     &battle_config.item_rate_card,                  100,    0,      1000000,        },
	{ "item_rate_card_boss",                &battle_config.item_rate_card_boss,             100,    0,      1000000,        },
	{ "item_rate_heal",                     &battle_config.item_rate_heal,                  100,    0,      1000000,        },
	{ "item_rate_heal_boss",                &battle_config.item_rate_heal_boss,             100,    0,      1000000,        },
	{ "item_rate_use",                      &battle_config.item_rate_use,                   100,    0,      1000000,        },
	{ "item_rate_use_boss",                 &battle_config.item_rate_use_boss,              100,    0,      1000000,        },
	{ "item_rate_adddrop",                  &battle_config.item_rate_adddrop,               100,    0,      1000000,        },
	{ "item_rate_treasure",                 &battle_config.item_rate_treasure,              100,    0,      1000000,        },
	{ "prevent_logout",                     &battle_config.prevent_logout,                  10000,  0,      60000,          },
	{ "alchemist_summon_reward",            &battle_config.alchemist_summon_reward,         1,      0,      2,              },
	{ "drops_by_luk",                       &battle_config.drops_by_luk,                    0,      0,      INT_MAX,        },
	{ "drops_by_luk2",                      &battle_config.drops_by_luk2,                   0,      0,      INT_MAX,        },
	{ "equip_natural_break_rate",           &battle_config.equip_natural_break_rate,        0,      0,      INT_MAX,        },
	{ "equip_self_break_rate",              &battle_config.equip_self_break_rate,           100,    0,      INT_MAX,        },
	{ "equip_skill_break_rate",             &battle_config.equip_skill_break_rate,          100,    0,      INT_MAX,        },
	{ "pk_mode",                            &battle_config.pk_mode,                         0,      0,      1,              },
	{ "pk_level_range",                     &battle_config.pk_level_range,                  0,      0,      INT_MAX,        },
	{ "manner_system",                      &battle_config.manner_system,                   0xFFF,  0,      0xFFF,          },
	{ "pet_equip_required",                 &battle_config.pet_equip_required,              0,      0,      1,              },
	{ "multi_level_up",                     &battle_config.multi_level_up,                  0,      0,      1,              },
	{ "max_exp_gain_rate",                  &battle_config.max_exp_gain_rate,               0,      0,      INT_MAX,        },
	{ "backstab_bow_penalty",               &battle_config.backstab_bow_penalty,            0,      0,      1,              },
	{ "night_at_start",                     &battle_config.night_at_start,                  0,      0,      1,              },
	{ "show_mob_info",                      &battle_config.show_mob_info,                   0,      0,      1|2|4,          },
	{ "ban_hack_trade",                     &battle_config.ban_hack_trade,                  0,      0,      INT_MAX,        },
	{ "hack_info_GM_level",                 &battle_config.hack_info_GM_level,              60,     0,      100,            },
	{ "any_warp_GM_min_level",              &battle_config.any_warp_GM_min_level,           20,     0,      100,            },
	{ "who_display_aid",                    &battle_config.who_display_aid,                 40,     0,      100,            },
	{ "packet_ver_flag",                    &battle_config.packet_ver_flag,                 0xFFFF, 0x0000, 0xFFFF,         },
	{ "min_hair_style",                     &battle_config.min_hair_style,                  0,      0,      INT_MAX,        },
	{ "max_hair_style",                     &battle_config.max_hair_style,                  23,     0,      INT_MAX,        },
	{ "min_hair_color",                     &battle_config.min_hair_color,                  0,      0,      INT_MAX,        },
	{ "max_hair_color",                     &battle_config.max_hair_color,                  9,      0,      INT_MAX,        },
	{ "min_cloth_color",                    &battle_config.min_cloth_color,                 0,      0,      INT_MAX,        },
	{ "max_cloth_color",                    &battle_config.max_cloth_color,                 4,      0,      INT_MAX,        },
	{ "pet_hair_style",                     &battle_config.pet_hair_style,                  100,    0,      INT_MAX,        },
	{ "castrate_dex_scale",                 &battle_config.castrate_dex_scale,              150,    1,      INT_MAX,        },
	{ "area_size",                          &battle_config.area_size,                       14,     0,      INT_MAX,        },
	{ "zeny_from_mobs",                     &battle_config.zeny_from_mobs,                  0,      0,      1,              },
	{ "mobs_level_up",                      &battle_config.mobs_level_up,                   0,      0,      1,              },
	{ "mobs_level_up_exp_rate",             &battle_config.mobs_level_up_exp_rate,          1,      1,      INT_MAX,        },
	{ "pk_min_level",                       &battle_config.pk_min_level,                    55,     1,      INT_MAX,        },
	{ "skill_steal_max_tries",              &battle_config.skill_steal_max_tries,           0,      0,      UCHAR_MAX,      },
	{ "motd_type",                          &battle_config.motd_type,                       0,      0,      1,              },
	{ "finding_ore_rate",                   &battle_config.finding_ore_rate,                100,    0,      INT_MAX,        },
	{ "exp_calc_type",                      &battle_config.exp_calc_type,                   0,      0,      1,              },
	{ "exp_bonus_attacker",                 &battle_config.exp_bonus_attacker,              25,     0,      INT_MAX,        },
	{ "exp_bonus_max_attacker",             &battle_config.exp_bonus_max_attacker,          12,     2,      INT_MAX,        },
	{ "min_skill_delay_limit",              &battle_config.min_skill_delay_limit,           100,    10,     INT_MAX,        },
	{ "default_walk_delay",                 &battle_config.default_walk_delay,              300,    0,      INT_MAX,        },
	{ "no_skill_delay",                     &battle_config.no_skill_delay,                  BL_MOB, BL_NUL, BL_ALL,         },
	{ "attack_walk_delay",                  &battle_config.attack_walk_delay,               BL_ALL, BL_NUL, BL_ALL,         },
	{ "require_glory_guild",                &battle_config.require_glory_guild,             0,      0,      1,              },
	{ "idle_no_share",                      &battle_config.idle_no_share,                   0,      0,      INT_MAX,        },
	{ "party_even_share_bonus",             &battle_config.party_even_share_bonus,          0,      0,      INT_MAX,        }, 
	{ "delay_battle_damage",                &battle_config.delay_battle_damage,             1,      0,      1,              },
	{ "hide_woe_damage",                    &battle_config.hide_woe_damage,                 0,      0,      1,              },
	{ "display_version",                    &battle_config.display_version,                 1,      0,      1,              },
	{ "display_hallucination",              &battle_config.display_hallucination,           1,      0,      1,              },
	{ "use_statpoint_table",                &battle_config.use_statpoint_table,             1,      0,      1,              },
	{ "ignore_items_gender",                &battle_config.ignore_items_gender,             1,      0,      1,              },
	{ "copyskill_restrict",                 &battle_config.copyskill_restrict,              2,      0,      2,              },
	{ "berserk_cancels_buffs",              &battle_config.berserk_cancels_buffs,           0,      0,      1,              },
	{ "debuff_on_logout",                   &battle_config.debuff_on_logout,                1|2,    0,      1|2,            },
	{ "monster_ai",                         &battle_config.mob_ai,                          0x000,  0x000,  0x77F,          },
	{ "hom_setting",                        &battle_config.hom_setting,                     0xFFFF, 0x0000, 0xFFFF,         },
	{ "dynamic_mobs",                       &battle_config.dynamic_mobs,                    1,      0,      1,              },
	{ "mob_remove_damaged",                 &battle_config.mob_remove_damaged,              1,      0,      1,              },
	{ "show_hp_sp_drain",                   &battle_config.show_hp_sp_drain,                0,      0,      1,              },
	{ "show_hp_sp_gain",                    &battle_config.show_hp_sp_gain,                 1,      0,      1,              },
	{ "mob_npc_event_type",                 &battle_config.mob_npc_event_type,              1,      0,      1,              },
	{ "mob_clear_delay",                    &battle_config.mob_clear_delay,                 0,      0,      INT_MAX,        },
	{ "character_size",                     &battle_config.character_size,                  1|2,    0,      1|2,            },
	{ "mob_max_skilllvl",                   &battle_config.mob_max_skilllvl,                MAX_SKILL_LEVEL, 1, MAX_SKILL_LEVEL, },
	{ "retaliate_to_master",                &battle_config.retaliate_to_master,             1,      0,      1,              },
	{ "rare_drop_announce",                 &battle_config.rare_drop_announce,              0,      0,      10000,          },
	{ "title_lvl1",                         &battle_config.title_lvl1,                      1,      0,      100,            },
	{ "title_lvl2",                         &battle_config.title_lvl2,                      10,     0,      100,            },
	{ "title_lvl3",                         &battle_config.title_lvl3,                      20,     0,      100,            },
	{ "title_lvl4",                         &battle_config.title_lvl4,                      40,     0,      100,            },
	{ "title_lvl5",                         &battle_config.title_lvl5,                      50,     0,      100,            },
	{ "title_lvl6",                         &battle_config.title_lvl6,                      60,     0,      100,            },
	{ "title_lvl7",                         &battle_config.title_lvl7,                      80,     0,      100,            },
	{ "title_lvl8",                         &battle_config.title_lvl8,                      99,     0,      100,            },
	{ "duel_allow_pvp",                     &battle_config.duel_allow_pvp,                  0,      0,      1,              },
	{ "duel_allow_gvg",                     &battle_config.duel_allow_gvg,                  0,      0,      1,              },
	{ "duel_allow_teleport",                &battle_config.duel_allow_teleport,             0,      0,      1,              },
	{ "duel_autoleave_when_die",            &battle_config.duel_autoleave_when_die,         1,      0,      1,              },
	{ "duel_time_interval",                 &battle_config.duel_time_interval,              60,     0,      INT_MAX,        },
	{ "duel_only_on_same_map",              &battle_config.duel_only_on_same_map,           0,      0,      1,              },
	{ "skip_teleport_lv1_menu",             &battle_config.skip_teleport_lv1_menu,          0,      0,      1,              },
	{ "allow_skill_without_day",            &battle_config.allow_skill_without_day,         0,      0,      1,              },
	{ "allow_es_magic_player",              &battle_config.allow_es_magic_pc,               0,      0,      1,              },
	{ "skill_caster_check",                 &battle_config.skill_caster_check,              1,      0,      1,              },
	{ "status_cast_cancel",                 &battle_config.sc_castcancel,                   BL_NUL, BL_NUL, BL_ALL,         },
	{ "pc_status_def_rate",                 &battle_config.pc_sc_def_rate,                  100,    0,      INT_MAX,        },
	{ "mob_status_def_rate",                &battle_config.mob_sc_def_rate,                 100,    0,      INT_MAX,        },
	{ "pc_luk_status_def",                  &battle_config.pc_luk_sc_def,                   300,    1,      INT_MAX,        },
	{ "mob_luk_status_def",                 &battle_config.mob_luk_sc_def,                  300,    1,      INT_MAX,        },
	{ "pc_max_status_def",                  &battle_config.pc_max_sc_def,                   100,    0,      INT_MAX,        },
	{ "mob_max_status_def",                 &battle_config.mob_max_sc_def,                  100,    0,      INT_MAX,        },
	{ "sg_miracle_skill_ratio",             &battle_config.sg_miracle_skill_ratio,          1,      0,      10000,          },
	{ "sg_angel_skill_ratio",               &battle_config.sg_angel_skill_ratio,            10,     0,      10000,          },
	{ "autospell_stacking",                 &battle_config.autospell_stacking,              0,      0,      1,              },
	{ "override_mob_names",                 &battle_config.override_mob_names,              0,      0,      2,              },
	{ "min_chat_delay",                     &battle_config.min_chat_delay,                  0,      0,      INT_MAX,        },
	{ "friend_auto_add",                    &battle_config.friend_auto_add,                 1,      0,      1,              },
	{ "hom_rename",                         &battle_config.hom_rename,                      0,      0,      1,              },
	{ "homunculus_show_growth",             &battle_config.homunculus_show_growth,          0,      0,      1,              },
	{ "homunculus_friendly_rate",           &battle_config.homunculus_friendly_rate,        100,    0,      INT_MAX,        },
	{ "vending_tax",                        &battle_config.vending_tax,                     0,      0,      10000,          },
	{ "day_duration",                       &battle_config.day_duration,                    0,      0,      INT_MAX,        },
	{ "night_duration",                     &battle_config.night_duration,                  0,      0,      INT_MAX,        },
	{ "mob_remove_delay",                   &battle_config.mob_remove_delay,                60000,  1000,   INT_MAX,        },
	{ "mob_active_time",                    &battle_config.mob_active_time,                 0,      0,      INT_MAX,        },
	{ "boss_active_time",                   &battle_config.boss_active_time,                0,      0,      INT_MAX,        },
	{ "sg_miracle_skill_duration",          &battle_config.sg_miracle_skill_duration,       3600000, 0,     INT_MAX,        },
	{ "hvan_explosion_intimate",            &battle_config.hvan_explosion_intimate,         45000,  0,      100000,         },
	{ "quest_exp_rate",                     &battle_config.quest_exp_rate,                  100,    0,      INT_MAX,        },
	{ "at_mapflag",                         &battle_config.autotrade_mapflag,               0,      0,      1,              },
	{ "at_timeout",                         &battle_config.at_timeout,                      0,      0,      INT_MAX,        },
	{ "homunculus_autoloot",                &battle_config.homunculus_autoloot,             0,      0,      1,              },
	{ "idle_no_autoloot",                   &battle_config.idle_no_autoloot,                0,      0,      INT_MAX,        },
	{ "max_guild_alliance",                 &battle_config.max_guild_alliance,              3,      1,      3,              },
	{ "ksprotection",                       &battle_config.ksprotection,                    5000,   0,      INT_MAX,        },
	{ "auction_feeperhour",                 &battle_config.auction_feeperhour,              12000,  0,      INT_MAX,        },
	{ "auction_maximumprice",               &battle_config.auction_maximumprice,            500000000, 0,   MAX_ZENY,       },
	{ "gm_viewequip_min_lv",                &battle_config.gm_viewequip_min_lv,             0,      0,      99,             },
	{ "homunculus_auto_vapor",              &battle_config.homunculus_auto_vapor,           0,      0,      1,              },
};


int battle_set_value(const char* w1, const char* w2)
{
	int val = config_switch(w2);

	int i;
	ARR_FIND(0, ARRAYLENGTH(battle_data), i, strcmpi(w1, battle_data[i].str) == 0);
	if (i == ARRAYLENGTH(battle_data))
		return 0; // not found

	if (val < battle_data[i].min || val > battle_data[i].max)
	{
		ShowWarning("Value for setting '%s': %s is invalid (min:%i max:%i)! Defaulting to %i...\n", w1, w2, battle_data[i].min, battle_data[i].max, battle_data[i].defval);
		val = battle_data[i].defval;
	}

	*battle_data[i].val = val;
	return 1;
}

int battle_get_value(const char* w1)
{
	int i;
	ARR_FIND(0, ARRAYLENGTH(battle_data), i, strcmpi(w1, battle_data[i].str) == 0);
	if (i == ARRAYLENGTH(battle_data))
		return 0; // not found
	else
		return *battle_data[i].val;
}

void battle_set_defaults()
{
	int i;
	for (i = 0; i < ARRAYLENGTH(battle_data); i++)
		*battle_data[i].val = battle_data[i].defval;
}

void battle_adjust_conf()
{
	battle_config.monster_max_aspd = 2000 - battle_config.monster_max_aspd*10;
	battle_config.max_aspd = 2000 - battle_config.max_aspd*10;
	battle_config.max_walk_speed = 100*DEFAULT_WALK_SPEED/battle_config.max_walk_speed;	
	battle_config.max_cart_weight *= 10;
	
	if(battle_config.max_def > 100 && !battle_config.weapon_defense_type)	 // added by [Skotlex]
		battle_config.max_def = 100;

	if(battle_config.min_hitrate > battle_config.max_hitrate)
		battle_config.min_hitrate = battle_config.max_hitrate;
		
	if(battle_config.pet_max_atk1 > battle_config.pet_max_atk2)	//Skotlex
		battle_config.pet_max_atk1 = battle_config.pet_max_atk2;
	
	if (battle_config.day_duration && battle_config.day_duration < 60000) // added by [Yor]
		battle_config.day_duration = 60000;
	if (battle_config.night_duration && battle_config.night_duration < 60000) // added by [Yor]
		battle_config.night_duration = 60000;
	
#ifndef CELL_NOSTACK
	if (battle_config.cell_stack_limit != 1)
		ShowWarning("Battle setting 'cell_stack_limit' takes no effect as this server was compiled without Cell Stack Limit support.\n");
#endif
}

int battle_config_read(const char* cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE* fp;
	static int count = 0;

	if (count == 0)
		battle_set_defaults();

	count++;

	fp = fopen(cfgName,"r");
	if (fp == NULL)
		ShowError("File not found: %s\n", cfgName);
	else
	{
		while(fgets(line, sizeof(line), fp))
		{
			if (line[0] == '/' && line[1] == '/')
				continue;
			if (sscanf(line, "%1023[^:]:%1023s", w1, w2) != 2)
				continue;
			if (strcmpi(w1, "import") == 0)
				battle_config_read(w2);
			else
			if (battle_set_value(w1, w2) == 0)
				ShowWarning("Unknown setting '%s' in file %s\n", w1, cfgName);
		}

		fclose(fp);
	}

	count--;

	if (count == 0)
		battle_adjust_conf();

	return 0;
}

void do_init_battle(void)
{
	delay_damage_ers = ers_new(sizeof(struct delay_damage));
	add_timer_func_list(battle_delay_damage_sub, "battle_delay_damage_sub");
}

void do_final_battle(void)
{
	ers_destroy(delay_damage_ers);
}
