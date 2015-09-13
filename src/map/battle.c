// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/ers.h"
#include "../common/random.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/utils.h"

#include "map.h"
#include "path.h"
#include "pc.h"
#include "homunculus.h"
#include "mercenary.h"
#include "elemental.h"
#include "pet.h"
#include "party.h"
#include "battleground.h"
#include "chrif.h"

#include <stdlib.h>
#include <math.h>

int attr_fix_table[4][ELE_MAX][ELE_MAX];

struct Battle_Config battle_config;
static struct eri *delay_damage_ers; //For battle delay damage structures.

/**
 * Returns the current/list skill used by the bl
 * @param bl
 * @return skill_id
 */
int battle_getcurrentskill(struct block_list *bl)
{
	struct unit_data *ud;

	if( bl->type == BL_SKILL ) {
		struct skill_unit *su = (struct skill_unit*)bl;
		return (su && su->group?su->group->skill_id:0);
	}

	ud = unit_bl2ud(bl);

	return (ud?ud->skill_id:0);
}

/**
 * Get random targeting enemy
 * @param bl
 * @param ap
 * @return Found target (1) or not found (0)
 */
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

	if ( !(ud = unit_bl2ud(bl)) )
		return 0;

	if (ud->target == target_id || ud->skilltarget == target_id) {
		bl_list[(*c)++] = bl;
		return 1;
	}

	return 0;
}

/**
 * Returns list of targets
 * @param target
 * @return Target list
 */
struct block_list* battle_gettargeted(struct block_list *target)
{
	struct block_list *bl_list[24];
	int c = 0;
	nullpo_retr(NULL, target);

	memset(bl_list, 0, sizeof(bl_list));
	map_foreachinrange(battle_gettargeted_sub, target, AREA_SIZE, BL_CHAR, bl_list, &c, target->id);
	if ( c == 0 )
		return NULL;
	if( c > 24 )
		c = 24;
	return bl_list[rnd()%c];
}

/**
 * Returns the ID of the current targeted character of the passed bl
 * @param bl
 * @return Target Unit ID
 * @author [Skotlex]
 */
int battle_gettarget(struct block_list* bl)
{

	switch (bl->type) {
		case BL_PC:  return ((struct map_session_data*)bl)->ud.target;
		case BL_MOB: return ((struct mob_data*)bl)->target_id;
		case BL_PET: return ((struct pet_data*)bl)->target_id;
		case BL_HOM: return ((struct homun_data*)bl)->ud.target;
		case BL_MER: return ((struct mercenary_data*)bl)->ud.target;
		case BL_ELEM: return ((struct elemental_data*)bl)->ud.target;
	}

	return 0;
}

/**
 * Get random enemy
 * @param bl
 * @param ap
 * @return Found target (1) or not found (0)
 */
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

/**
 * Returns list of enemies within given range
 * @param target
 * @param type
 * @param range
 * @return Target list
 * @author [Skotlex]
 */
struct block_list* battle_getenemy(struct block_list *target, int type, int range)
{
	struct block_list *bl_list[24];
	int c = 0;

	memset(bl_list, 0, sizeof(bl_list));
	map_foreachinrange(battle_getenemy_sub, target, range, type, bl_list, &c, target);

	if ( c == 0 )
		return NULL;

	if( c > 24 )
		c = 24;

	return bl_list[rnd()%c];
}

/**
 * Get random enemy within area
 * @param bl
 * @param ap
 * @return Found target (1) or not found (0)
 */
static int battle_getenemyarea_sub(struct block_list *bl, va_list ap)
{
	struct block_list **bl_list, *src;
	int *c, ignore_id;

	bl_list = va_arg(ap, struct block_list **);
	c = va_arg(ap, int *);
	src = va_arg(ap, struct block_list *);
	ignore_id = va_arg(ap, int);

	if( bl->id == src->id || bl->id == ignore_id )
		return 0; // Ignores Caster and a possible pre-target

	if( *c >= 23 )
		return 0;

	if( status_isdead(bl) )
		return 0;

	if( battle_check_target(src, bl, BCT_ENEMY) > 0 ) {// Is Enemy!...
		bl_list[(*c)++] = bl;
		return 1;
	}

	return 0;
}

/**
 * Returns list of enemies within an area
 * @param src
 * @param x
 * @param y
 * @param range
 * @param type
 * @param ignore_id
 * @return Target list
 */
struct block_list* battle_getenemyarea(struct block_list *src, int x, int y, int range, int type, int ignore_id)
{
	struct block_list *bl_list[24];
	int c = 0;

	memset(bl_list, 0, sizeof(bl_list));
	map_foreachinarea(battle_getenemyarea_sub, src->m, x - range, y - range, x + range, y + range, type, bl_list, &c, src, ignore_id);

	if( c == 0 )
		return NULL;
	if( c >= 24 )
		c = 23;

	return bl_list[rnd()%c];
}

/// Damage Delayed Structure
struct delay_damage {
	int src_id;
	int target_id;
	int64 damage;
	int delay;
	unsigned short distance;
	uint16 skill_lv;
	uint16 skill_id;
	enum damage_lv dmg_lv;
	unsigned short attack_type;
	bool additional_effects;
	enum bl_type src_type;
};

int battle_delay_damage_sub(int tid, unsigned int tick, int id, intptr_t data)
{
	struct delay_damage *dat = (struct delay_damage *)data;

	if ( dat ) {
		struct block_list* src = NULL;
		struct block_list* target = map_id2bl(dat->target_id);

		if( !target || status_isdead(target) ) { /* Nothing we can do */
			if( dat->src_type == BL_PC && (src = map_id2bl(dat->src_id)) &&
				--((TBL_PC*)src)->delayed_damage == 0 && ((TBL_PC*)src)->state.hold_recalc ) {
				((TBL_PC*)src)->state.hold_recalc = 0;
				status_calc_pc(((TBL_PC*)src), SCO_FORCE);
			}
			ers_free(delay_damage_ers, dat);
			return 0;
		}

		src = map_id2bl(dat->src_id);

		if( src && target->m == src->m &&
			(target->type != BL_PC || ((TBL_PC*)target)->invincible_timer == INVALID_TIMER) &&
			check_distance_bl(src, target, dat->distance) ) //Check to see if you haven't teleported. [Skotlex]
		{
			map_freeblock_lock();
			status_fix_damage(src, target, dat->damage, dat->delay);
			if( dat->attack_type && !status_isdead(target) && dat->additional_effects )
				skill_additional_effect(src,target,dat->skill_id,dat->skill_lv,dat->attack_type,dat->dmg_lv,tick);
			if( dat->dmg_lv > ATK_BLOCK && dat->attack_type )
				skill_counter_additional_effect(src,target,dat->skill_id,dat->skill_lv,dat->attack_type,tick);
			map_freeblock_unlock();
		} else if( !src && dat->skill_id == CR_REFLECTSHIELD ) { // it was monster reflected damage, and the monster died, we pass the damage to the character as expected
			map_freeblock_lock();
			status_fix_damage(target, target, dat->damage, dat->delay);
			map_freeblock_unlock();
		}

		if( src && src->type == BL_PC && --((TBL_PC*)src)->delayed_damage == 0 && ((TBL_PC*)src)->state.hold_recalc ) {
			((TBL_PC*)src)->state.hold_recalc = 0;
			status_calc_pc(((TBL_PC*)src), SCO_FORCE);
		}
	}
	ers_free(delay_damage_ers, dat);
	return 0;
}

int battle_delay_damage(unsigned int tick, int amotion, struct block_list *src, struct block_list *target, int attack_type, uint16 skill_id, uint16 skill_lv, int64 damage, enum damage_lv dmg_lv, int ddelay, bool additional_effects)
{
	struct delay_damage *dat;
	struct status_change *sc;
	struct block_list *d_tbl = NULL;
	nullpo_ret(src);
	nullpo_ret(target);

	sc = status_get_sc(target);

	if (sc && sc->data[SC_DEVOTION] && sc->data[SC_DEVOTION]->val1)
		d_tbl = map_id2bl(sc->data[SC_DEVOTION]->val1);

	if( d_tbl && check_distance_bl(target, d_tbl, sc->data[SC_DEVOTION]->val3) &&
		damage > 0 && skill_id != PA_PRESSURE && skill_id != CR_REFLECTSHIELD )
		damage = 0;

	if ( !battle_config.delay_battle_damage || amotion <= 1 ) {
		map_freeblock_lock();
		status_fix_damage(src, target, damage, ddelay); // We have to separate here between reflect damage and others [icescope]
		if( attack_type && !status_isdead(target) && additional_effects )
			skill_additional_effect(src, target, skill_id, skill_lv, attack_type, dmg_lv, gettick());
		if( dmg_lv > ATK_BLOCK && attack_type )
			skill_counter_additional_effect(src, target, skill_id, skill_lv, attack_type, gettick());
		map_freeblock_unlock();
		return 0;
	}
	dat = ers_alloc(delay_damage_ers, struct delay_damage);
	dat->src_id = src->id;
	dat->target_id = target->id;
	dat->skill_id = skill_id;
	dat->skill_lv = skill_lv;
	dat->attack_type = attack_type;
	dat->damage = damage;
	dat->dmg_lv = dmg_lv;
	dat->delay = ddelay;
	dat->distance = distance_bl(src, target) + (battle_config.snap_dodge ? 10 : AREA_SIZE);
	dat->additional_effects = additional_effects;
	dat->src_type = src->type;
	if (src->type != BL_PC && amotion > 1000)
		amotion = 1000; //Aegis places a damage-delay cap of 1 sec to non player attacks. [Skotlex]

	if( src->type == BL_PC )
		((TBL_PC*)src)->delayed_damage++;

	add_timer(tick+amotion, battle_delay_damage_sub, 0, (intptr_t)dat);

	return 0;
}

/**
 * Get attribute ratio
 * @param atk_elem Attack element enum e_element
 * @param def_type Defense element enum e_element
 * @param def_lv Element level 1 ~ MAX_ELE_LEVEL
 */
int battle_attr_ratio(int atk_elem, int def_type, int def_lv)
{
	if (!CHK_ELEMENT(atk_elem) || !CHK_ELEMENT(def_type) || !CHK_ELEMENT_LEVEL(def_lv))
		return 100;

	return attr_fix_table[def_lv-1][atk_elem][def_type];
}

/**
 * Does attribute fix modifiers.
 * Added passing of the chars so that the status changes can affect it. [Skotlex]
 * Note: Passing src/target == NULL is perfectly valid, it skips SC_ checks.
 * @param src
 * @param target
 * @param damage
 * @param atk_elem
 * @param def_type
 * @param def_lv
 * @return damage
 */
int64 battle_attr_fix(struct block_list *src, struct block_list *target, int64 damage,int atk_elem,int def_type, int def_lv)
{
	struct status_change *sc = NULL, *tsc = NULL;
	int ratio;

	if (src) sc = status_get_sc(src);
	if (target) tsc = status_get_sc(target);

	if (!CHK_ELEMENT(atk_elem))
		atk_elem = rnd()%ELE_ALL;

	if (!CHK_ELEMENT(def_type) || !CHK_ELEMENT_LEVEL(def_lv)) {
		ShowError("battle_attr_fix: unknown attribute type: atk=%d def_type=%d def_lv=%d\n",atk_elem,def_type,def_lv);
		return damage;
	}

	ratio = attr_fix_table[def_lv-1][atk_elem][def_type];
	if (sc && sc->count) { //increase dmg by src status
		switch(atk_elem){
			case ELE_FIRE:
				if (sc->data[SC_VOLCANO])
					ratio += sc->data[SC_VOLCANO]->val3;
				break;
			case ELE_WIND:
				if (sc->data[SC_VIOLENTGALE])
					ratio += sc->data[SC_VIOLENTGALE]->val3;
				break;
			case ELE_WATER:
				if (sc->data[SC_DELUGE])
					ratio += sc->data[SC_DELUGE]->val3;
				break;
			case ELE_GHOST:
				if (sc->data[SC_TELEKINESIS_INTENSE])
					ratio += sc->data[SC_TELEKINESIS_INTENSE]->val3;
				break;
		}
	}

	if( target && target->type == BL_SKILL ) {
		if( atk_elem == ELE_FIRE && battle_getcurrentskill(target) == GN_WALLOFTHORN ) {
			struct skill_unit *su = (struct skill_unit*)target;
			struct skill_unit_group *sg;
			struct block_list *src2;

			if( !su || !su->alive || (sg = su->group) == NULL || !sg || sg->val3 == -1 ||
			   (src2 = map_id2bl(sg->src_id)) == NULL || status_isdead(src2) )
				return 0;

			if( sg->unit_id != UNT_FIREWALL ) {
				int x,y;
				x = sg->val3 >> 16;
				y = sg->val3 & 0xffff;
				skill_unitsetting(src2,su->group->skill_id,su->group->skill_lv,x,y,1);
				sg->val3 = -1;
				sg->limit = DIFF_TICK(gettick(),sg->tick)+300;
			}
		}
	}

	if (tsc && tsc->count) { //increase dmg by target status
		switch(atk_elem) {
			case ELE_FIRE:
				if (tsc->data[SC_SPIDERWEB]) {
					tsc->data[SC_SPIDERWEB]->val1 = 0; // free to move now
					if (tsc->data[SC_SPIDERWEB]->val2-- > 0)
						ratio += 100; // double damage
					if (tsc->data[SC_SPIDERWEB]->val2 == 0)
						status_change_end(target, SC_SPIDERWEB, INVALID_TIMER);
				}
				if (tsc->data[SC_THORNSTRAP])
					status_change_end(target, SC_THORNSTRAP, INVALID_TIMER);
				if (tsc->data[SC_CRYSTALIZE] && target->type != BL_MOB)
					status_change_end(target, SC_CRYSTALIZE, INVALID_TIMER);
				if (tsc->data[SC_EARTH_INSIGNIA])
					ratio += 50;
				if (tsc->data[SC_ASH])
					ratio += 50;
				break;
			case ELE_HOLY:
				if (tsc->data[SC_ORATIO])
					ratio += tsc->data[SC_ORATIO]->val1 * 2;
				break;
			case ELE_POISON:
				if (tsc->data[SC_VENOMIMPRESS])
					ratio += tsc->data[SC_VENOMIMPRESS]->val2;
				break;
			case ELE_WIND:
				if (tsc->data[SC_CRYSTALIZE] && target->type != BL_MOB)
					ratio += 50;
				if (tsc->data[SC_WATER_INSIGNIA])
					ratio += 50;
				break;
			case ELE_WATER:
				if (tsc->data[SC_FIRE_INSIGNIA])
					ratio += 50;
				break;
			case ELE_EARTH:
				if (tsc->data[SC_WIND_INSIGNIA])
					ratio += 50;
				status_change_end(target, SC_MAGNETICFIELD, INVALID_TIMER); //freed if received earth dmg
				break;
			case ELE_NEUTRAL:
				if (tsc->data[SC_ANTI_M_BLAST])
					ratio += tsc->data[SC_ANTI_M_BLAST]->val2;
				break;
		}
	}

	if (ratio < 100)
		damage = damage - (damage * (100 - ratio) / 100);
	else
		damage = damage + (damage * (ratio - 100) / 100);

	return max(damage,0);
}

/**
 * Calculates card bonuses damage adjustments.
 * @param attack_type @see enum e_battle_flag
 * @param src Attacker
 * @param target Target
 * @param nk Skill's nk @see enum e_skill_nk [NK_NO_CARDFIX_ATK|NK_NO_ELEFIX|NK_NO_CARDFIX_DEF]
 * @param rh_ele Right-hand weapon element
 * @param lh_ele Left-hand weapon element (BF_MAGIC and BF_MISC ignore this value)
 * @param damage Original damage
 * @param left Left hand flag (BF_MISC and BF_MAGIC ignore flag value)
 *         3: Calculates attacker bonuses in both hands.
 *         2: Calculates attacker bonuses in right-hand only.
 *         0 or 1: Only calculates target bonuses.
 * @param flag Misc value of skill & damage flags
 * @return damage Damage diff between original damage and after calculation
 */
int battle_calc_cardfix(int attack_type, struct block_list *src, struct block_list *target, int nk, int rh_ele, int lh_ele, int64 damage, int left, int flag){
	struct map_session_data *sd, ///< Attacker session data if BL_PC
		*tsd; ///< Target session data if BL_PC
	short cardfix = 1000;
	enum e_classAE s_class, ///< Attacker class
		t_class; ///< Target class
	enum e_race2 s_race2, /// Attacker Race2
		t_race2; ///< Target Race2
	enum e_element s_defele; ///< Attacker Element (not a weapon or skill element!)
	struct status_data *sstatus, ///< Attacker status data
		*tstatus; ///< Target status data
	int64 original_damage;
	int i;

	if( !damage )
		return 0;

	original_damage = damage;

	sd = BL_CAST(BL_PC, src);
	tsd = BL_CAST(BL_PC, target);
	t_class = (enum e_classAE)status_get_class(target);
	s_class = (enum e_classAE)status_get_class(src);
	sstatus = status_get_status_data(src);
	tstatus = status_get_status_data(target);
	s_race2 = (enum e_race2)status_get_race2(src);
	s_defele = (tsd) ? (enum e_element)status_get_element(src) : ELE_NONE;

	switch( attack_type ) {
		case BF_MAGIC:
			// Affected by attacker ATK bonuses
			if( sd && !(nk&NK_NO_CARDFIX_ATK) ) {
				cardfix = cardfix * (100 + sd->magic_addrace[tstatus->race] + sd->magic_addrace[RC_ALL]) / 100;
				if( !(nk&NK_NO_ELEFIX) ) { // Affected by Element modifier bonuses
					cardfix = cardfix * (100 + sd->magic_addele[tstatus->def_ele] + sd->magic_addele[ELE_ALL]) / 100;
					cardfix = cardfix * (100 + sd->magic_atk_ele[rh_ele] + sd->magic_atk_ele[ELE_ALL]) / 100;
				}
				cardfix = cardfix * (100 + sd->magic_addsize[tstatus->size] + sd->magic_addsize[SZ_ALL]) / 100;
				cardfix = cardfix * (100 + sd->magic_addclass[tstatus->class_] + sd->magic_addclass[CLASS_ALL]) / 100;
				for( i = 0; i < ARRAYLENGTH(sd->add_mdmg) && sd->add_mdmg[i].rate; i++ ) {
					if( sd->add_mdmg[i].class_ == t_class ) {
						cardfix = cardfix * (100 + sd->add_mdmg[i].rate) / 100;
						break;
					}
				}
				if( cardfix != 1000 )
					damage = damage * cardfix / 1000;
			}

			// Affected by target DEF bonuses
			if( tsd && !(nk&NK_NO_CARDFIX_DEF) ) {
				cardfix = 1000; // reset var for target

				if( !(nk&NK_NO_ELEFIX) ) { // Affected by Element modifier bonuses
					int ele_fix = tsd->subele[rh_ele] + tsd->subele[ELE_ALL];

					for( i = 0; ARRAYLENGTH(tsd->subele2) > i && tsd->subele2[i].rate != 0; i++ ) {
						if( tsd->subele2[i].ele != rh_ele )
							continue;
						if( !(((tsd->subele2[i].flag)&flag)&BF_WEAPONMASK &&
							((tsd->subele2[i].flag)&flag)&BF_RANGEMASK &&
							((tsd->subele2[i].flag)&flag)&BF_SKILLMASK) )
							continue;
						ele_fix += tsd->subele2[i].rate;
					}
					if (s_defele != ELE_NONE)
						ele_fix += tsd->subdefele[s_defele] + tsd->subdefele[ELE_ALL];
					cardfix = cardfix * (100 - ele_fix) / 100;
				}
				cardfix = cardfix * (100 - tsd->subsize[sstatus->size] - tsd->subsize[SZ_ALL]) / 100;
				cardfix = cardfix * (100 - tsd->subrace2[s_race2]) / 100;
				cardfix = cardfix * (100 - tsd->subrace[sstatus->race] - tsd->subrace[RC_ALL]) / 100;
				cardfix = cardfix * (100 - tsd->subclass[sstatus->class_] - tsd->subclass[CLASS_ALL]) / 100;

				for( i = 0; i < ARRAYLENGTH(tsd->add_mdef) && tsd->add_mdef[i].rate; i++ ) {
					if( tsd->add_mdef[i].class_ == s_class ) {
						cardfix = cardfix * (100 - tsd->add_mdef[i].rate) / 100;
						break;
					}
				}
#ifndef RENEWAL
				//It was discovered that ranged defense also counts vs magic! [Skotlex]
				if( flag&BF_SHORT )
					cardfix = cardfix * (100 - tsd->bonus.near_attack_def_rate) / 100;
				else
					cardfix = cardfix * (100 - tsd->bonus.long_attack_def_rate) / 100;
#endif
				cardfix = cardfix * (100 - tsd->bonus.magic_def_rate) / 100;

				if( tsd->sc.data[SC_MDEF_RATE] )
					cardfix = cardfix * (100 - tsd->sc.data[SC_MDEF_RATE]->val1) / 100;
				if( cardfix != 1000 )
					damage = damage * cardfix / 1000;
			}
			break;

		case BF_WEAPON:
			t_race2 = (enum e_race2)status_get_race2(target);
			// Affected by attacker ATK bonuses
			if( sd && !(nk&NK_NO_CARDFIX_ATK) && (left&2) ) {
				short cardfix_ = 1000;

				if( sd->state.arrow_atk ) { // Ranged attack
					cardfix = cardfix * (100 + sd->right_weapon.addrace[tstatus->race] + sd->arrow_addrace[tstatus->race] +
						sd->right_weapon.addrace[RC_ALL] + sd->arrow_addrace[RC_ALL]) / 100;
					if( !(nk&NK_NO_ELEFIX) ) { // Affected by Element modifier bonuses
						int ele_fix = sd->right_weapon.addele[tstatus->def_ele] + sd->arrow_addele[tstatus->def_ele] +
							sd->right_weapon.addele[ELE_ALL] + sd->arrow_addele[ELE_ALL];

						for( i = 0; ARRAYLENGTH(sd->right_weapon.addele2) > i && sd->right_weapon.addele2[i].rate != 0; i++ ) {
							if( sd->right_weapon.addele2[i].ele != tstatus->def_ele )
								continue;
							if( !(((sd->right_weapon.addele2[i].flag)&flag)&BF_WEAPONMASK &&
								((sd->right_weapon.addele2[i].flag)&flag)&BF_RANGEMASK &&
								((sd->right_weapon.addele2[i].flag)&flag)&BF_SKILLMASK) )
								continue;
							ele_fix += sd->right_weapon.addele2[i].rate;
						}
						cardfix = cardfix * (100 + ele_fix) / 100;
					}
					cardfix = cardfix * (100 + sd->right_weapon.addsize[tstatus->size] + sd->arrow_addsize[tstatus->size] +
						sd->right_weapon.addsize[SZ_ALL] + sd->arrow_addsize[SZ_ALL]) / 100;
					cardfix = cardfix * (100 + sd->right_weapon.addrace2[t_race2]) / 100;
					cardfix = cardfix * (100 + sd->right_weapon.addclass[tstatus->class_] + sd->arrow_addclass[tstatus->class_] +
						sd->right_weapon.addclass[CLASS_ALL] + sd->arrow_addclass[CLASS_ALL]) / 100;
				} else { // Melee attack
					int skill = 0;

					// Calculates each right & left hand weapon bonuses separatedly
					if( !battle_config.left_cardfix_to_right ) {
						// Right-handed weapon
						cardfix = cardfix * (100 + sd->right_weapon.addrace[tstatus->race] + sd->right_weapon.addrace[RC_ALL]) / 100;
						if( !(nk&NK_NO_ELEFIX) ) { // Affected by Element modifier bonuses
							int ele_fix = sd->right_weapon.addele[tstatus->def_ele] + sd->right_weapon.addele[ELE_ALL];

							for( i = 0; ARRAYLENGTH(sd->right_weapon.addele2) > i && sd->right_weapon.addele2[i].rate != 0; i++ ) {
								if( sd->right_weapon.addele2[i].ele != tstatus->def_ele )
									continue;
								if( !(((sd->right_weapon.addele2[i].flag)&flag)&BF_WEAPONMASK &&
									((sd->right_weapon.addele2[i].flag)&flag)&BF_RANGEMASK &&
									((sd->right_weapon.addele2[i].flag)&flag)&BF_SKILLMASK) )
									continue;
								ele_fix += sd->right_weapon.addele2[i].rate;
							}
							cardfix = cardfix * (100 + ele_fix) / 100;
						}
						cardfix = cardfix * (100 + sd->right_weapon.addsize[tstatus->size] + sd->right_weapon.addsize[SZ_ALL]) / 100;
						cardfix = cardfix * (100 + sd->right_weapon.addrace2[t_race2]) / 100;
						cardfix = cardfix * (100 + sd->right_weapon.addclass[tstatus->class_] + sd->right_weapon.addclass[CLASS_ALL]) / 100;

						if( left&1 ) { // Left-handed weapon
							cardfix_ = cardfix_ * (100 + sd->left_weapon.addrace[tstatus->race] + sd->left_weapon.addrace[RC_ALL]) / 100;
							if( !(nk&NK_NO_ELEFIX) ) { // Affected by Element modifier bonuses
								int ele_fix_lh = sd->left_weapon.addele[tstatus->def_ele] + sd->left_weapon.addele[ELE_ALL];

								for( i = 0; ARRAYLENGTH(sd->left_weapon.addele2) > i && sd->left_weapon.addele2[i].rate != 0; i++ ) {
									if( sd->left_weapon.addele2[i].ele != tstatus->def_ele )
										continue;
									if( !(((sd->left_weapon.addele2[i].flag)&flag)&BF_WEAPONMASK &&
										((sd->left_weapon.addele2[i].flag)&flag)&BF_RANGEMASK &&
										((sd->left_weapon.addele2[i].flag)&flag)&BF_SKILLMASK) )
										continue;
									ele_fix_lh += sd->left_weapon.addele2[i].rate;
								}
								cardfix_ = cardfix_ * (100 + ele_fix_lh) / 100;
							}
							cardfix_ = cardfix_ * (100 + sd->left_weapon.addsize[tstatus->size] + sd->left_weapon.addsize[SZ_ALL]) / 100;
							cardfix_ = cardfix_ * (100 + sd->left_weapon.addrace2[t_race2]) / 100;
							cardfix_ = cardfix_ * (100 + sd->left_weapon.addclass[tstatus->class_] + sd->left_weapon.addclass[CLASS_ALL]) / 100;
						}
					}
					// Calculates right & left hand weapon as unity
					else {
						//! CHECKME: If 'left_cardfix_to_right' is yes, doesn't need to check NK_NO_ELEFIX?
						//if( !(nk&NK_NO_ELEFIX) ) { // Affected by Element modifier bonuses
							int ele_fix = sd->right_weapon.addele[tstatus->def_ele] + sd->left_weapon.addele[tstatus->def_ele]
										+ sd->right_weapon.addele[ELE_ALL] + sd->left_weapon.addele[ELE_ALL];

							for( i = 0; ARRAYLENGTH(sd->right_weapon.addele2) > i && sd->right_weapon.addele2[i].rate != 0; i++ ) {
								if( sd->right_weapon.addele2[i].ele != tstatus->def_ele )
									continue;
								if( !(((sd->right_weapon.addele2[i].flag)&flag)&BF_WEAPONMASK &&
									((sd->right_weapon.addele2[i].flag)&flag)&BF_RANGEMASK &&
									((sd->right_weapon.addele2[i].flag)&flag)&BF_SKILLMASK) )
									continue;
								ele_fix += sd->right_weapon.addele2[i].rate;
							}
							for( i = 0; ARRAYLENGTH(sd->left_weapon.addele2) > i && sd->left_weapon.addele2[i].rate != 0; i++ ) {
								if( sd->left_weapon.addele2[i].ele != tstatus->def_ele )
									continue;
								if( !(((sd->left_weapon.addele2[i].flag)&flag)&BF_WEAPONMASK &&
									((sd->left_weapon.addele2[i].flag)&flag)&BF_RANGEMASK &&
									((sd->left_weapon.addele2[i].flag)&flag)&BF_SKILLMASK) )
									continue;
								ele_fix += sd->left_weapon.addele2[i].rate;
							}
							cardfix = cardfix * (100 + ele_fix) / 100;
						//}
						cardfix = cardfix * (100 + sd->right_weapon.addrace[tstatus->race] + sd->left_weapon.addrace[tstatus->race] +
							sd->right_weapon.addrace[RC_ALL] + sd->left_weapon.addrace[RC_ALL]) / 100;
						cardfix = cardfix * (100 + sd->right_weapon.addsize[tstatus->size] + sd->left_weapon.addsize[tstatus->size] +
							sd->right_weapon.addsize[SZ_ALL] + sd->left_weapon.addsize[SZ_ALL]) / 100;
						cardfix = cardfix * (100 + sd->right_weapon.addrace2[t_race2] + sd->left_weapon.addrace2[t_race2]) / 100;
						cardfix = cardfix * (100 + sd->right_weapon.addclass[tstatus->class_] + sd->left_weapon.addclass[tstatus->class_] +
							sd->right_weapon.addclass[CLASS_ALL] + sd->left_weapon.addclass[CLASS_ALL]) / 100;
					}
					if( sd->status.weapon == W_KATAR && (skill = pc_checkskill(sd,ASC_KATAR)) > 0 ) // Adv. Katar Mastery functions similar to a +%ATK card on official [helvetica]
						cardfix = cardfix * (100 + (10 + 2 * skill)) / 100;
				}

				//! CHECKME: These right & left hand weapon ignores 'left_cardfix_to_right'?
				for( i = 0; i < ARRAYLENGTH(sd->right_weapon.add_dmg) && sd->right_weapon.add_dmg[i].rate; i++ ) {
					if( sd->right_weapon.add_dmg[i].class_ == t_class ) {
						cardfix = cardfix * (100 + sd->right_weapon.add_dmg[i].rate) / 100;
						break;
					}
				}
				if( left&1 ) {
					for( i = 0; i < ARRAYLENGTH(sd->left_weapon.add_dmg) && sd->left_weapon.add_dmg[i].rate; i++ ) {
						if( sd->left_weapon.add_dmg[i].class_ == t_class ) {
							cardfix_ = cardfix_ * (100 + sd->left_weapon.add_dmg[i].rate) / 100;
							break;
						}
					}
				}
#ifndef RENEWAL
				if( flag&BF_LONG )
					cardfix = cardfix * (100 + sd->bonus.long_attack_atk_rate) / 100;
#endif
				if( (left&1) && cardfix_ != 1000 )
					damage = damage * cardfix_ / 1000;
				else if( cardfix != 1000 )
					damage = damage * cardfix / 1000;
			}
			// Affected by target DEF bonuses
			else if( tsd && !(nk&NK_NO_CARDFIX_DEF) && !(left&2) ) {
				if( !(nk&NK_NO_ELEFIX) ) { // Affected by Element modifier bonuses
					int ele_fix = tsd->subele[rh_ele] + tsd->subele[ELE_ALL];

					for( i = 0; ARRAYLENGTH(tsd->subele2) > i && tsd->subele2[i].rate != 0; i++ ) {
						if( tsd->subele2[i].ele != rh_ele )
							continue;
						if( !(((tsd->subele2[i].flag)&flag)&BF_WEAPONMASK &&
							((tsd->subele2[i].flag)&flag)&BF_RANGEMASK &&
							((tsd->subele2[i].flag)&flag)&BF_SKILLMASK) )
							continue;
						ele_fix += tsd->subele2[i].rate;
					}
					cardfix = cardfix * (100 - ele_fix) / 100;

					if( left&1 && lh_ele != rh_ele ) {
						int ele_fix_lh = tsd->subele[lh_ele] + tsd->subele[ELE_ALL];

						for( i = 0; ARRAYLENGTH(tsd->subele2) > i && tsd->subele2[i].rate != 0; i++ ) {
							if( tsd->subele2[i].ele != lh_ele )
								continue;
							if( !(((tsd->subele2[i].flag)&flag)&BF_WEAPONMASK &&
								((tsd->subele2[i].flag)&flag)&BF_RANGEMASK &&
								((tsd->subele2[i].flag)&flag)&BF_SKILLMASK) )
								continue;
							ele_fix_lh += tsd->subele2[i].rate;
						}
						cardfix = cardfix * (100 - ele_fix_lh) / 100;
					}

					cardfix = cardfix * (100 - tsd->subdefele[s_defele] - tsd->subdefele[ELE_ALL]) / 100;
				}
				cardfix = cardfix * (100 - tsd->subsize[sstatus->size] - tsd->subsize[SZ_ALL]) / 100;
				cardfix = cardfix * (100 - tsd->subrace2[s_race2]) / 100;
				cardfix = cardfix * (100 - tsd->subrace[sstatus->race] - tsd->subrace[RC_ALL]) / 100;
				cardfix = cardfix * (100 - tsd->subclass[sstatus->class_] - tsd->subclass[CLASS_ALL]) / 100;
				for( i = 0; i < ARRAYLENGTH(tsd->add_def) && tsd->add_def[i].rate; i++ ) {
					if( tsd->add_def[i].class_ == s_class ) {
						cardfix = cardfix * (100 - tsd->add_def[i].rate) / 100;
						break;
					}
				}
				if( flag&BF_SHORT )
					cardfix = cardfix * (100 - tsd->bonus.near_attack_def_rate) / 100;
				else	// BF_LONG (there's no other choice)
					cardfix = cardfix * (100 - tsd->bonus.long_attack_def_rate) / 100;
				if( tsd->sc.data[SC_DEF_RATE] )
					cardfix = cardfix * (100 - tsd->sc.data[SC_DEF_RATE]->val1) / 100;
				if( cardfix != 1000 )
					damage = damage * cardfix / 1000;
			}
			break;

		case BF_MISC:
			// Affected by target DEF bonuses
			if( tsd && !(nk&NK_NO_CARDFIX_DEF) ) {
				if( !(nk&NK_NO_ELEFIX) ) { // Affected by Element modifier bonuses
					int ele_fix = tsd->subele[rh_ele] + tsd->subele[ELE_ALL];

					for( i = 0; ARRAYLENGTH(tsd->subele2) > i && tsd->subele2[i].rate != 0; i++ ) {
						if( tsd->subele2[i].ele != rh_ele )
							continue;
						if( !(((tsd->subele2[i].flag)&flag)&BF_WEAPONMASK &&
							((tsd->subele2[i].flag)&flag)&BF_RANGEMASK &&
							((tsd->subele2[i].flag)&flag)&BF_SKILLMASK))
							continue;
						ele_fix += tsd->subele2[i].rate;
					}
					if (s_defele != ELE_NONE)
						ele_fix += tsd->subdefele[s_defele] + tsd->subdefele[ELE_ALL];
					cardfix = cardfix * (100 - ele_fix) / 100;
				}
				cardfix = cardfix * (100 - tsd->subsize[sstatus->size] - tsd->subsize[SZ_ALL]) / 100;
				cardfix = cardfix * (100 - tsd->subrace2[s_race2]) / 100;
				cardfix = cardfix * (100 - tsd->subrace[sstatus->race] - tsd->subrace[RC_ALL]) / 100;
				cardfix = cardfix * (100 - tsd->subclass[sstatus->class_] - tsd->subclass[CLASS_ALL]) / 100;
				cardfix = cardfix * (100 - tsd->bonus.misc_def_rate) / 100;
				if( flag&BF_SHORT )
					cardfix = cardfix * (100 - tsd->bonus.near_attack_def_rate) / 100;
				else	// BF_LONG (there's no other choice)
					cardfix = cardfix * (100 - tsd->bonus.long_attack_def_rate) / 100;
				if( cardfix != 1000 )
					damage = damage * cardfix / 1000;
			}
			break;
	}

	return (int)cap_value(damage - original_damage, INT_MIN, INT_MAX);
}

/**
* Absorb damage based on criteria
* @param bl
* @param d Damage
**/
static void battle_absorb_damage(struct block_list *bl, struct Damage *d) {
	int64 dmg_ori = 0, dmg_new = 0;

	nullpo_retv(bl);
	nullpo_retv(d);

	if (!d->damage && !d->damage2)
		return;

	switch (bl->type) {
		case BL_PC:
			{
				struct map_session_data *sd = BL_CAST(BL_PC, bl);
				if (!sd)
					return;
				if (sd->bonus.absorb_dmg_maxhp) {
					int hp = sd->bonus.absorb_dmg_maxhp * status_get_max_hp(bl) / 100;
					dmg_ori = dmg_new = d->damage + d->damage2;
					if (dmg_ori > hp)
						dmg_new = dmg_ori - hp;
				}
			}
			break;
	}

	if (dmg_ori == dmg_new)
		return;

	if (!d->damage2)
		d->damage = dmg_new;
	else if (!d->damage)
		d->damage2 = dmg_new;
	else {
		d->damage = dmg_new;
		d->damage2 = dmg_new * d->damage2 / dmg_ori / 100;
		if (d->damage2 < 1)
			d->damage2 = 1;
		d->damage = d->damage - d->damage2;
	}
}

/**
 * Check Safety Wall and Pneuma effect.
 * Maybe expand this to move checks the target's SC from battle_calc_damage?
 * @param src Attacker
 * @param target Target of attack
 * @param sc STatus Change
 * @param d Damage data
 * @param damage Damage received
 * @param skill_id
 * @param skill_lv
 * @return True:Damage inflicted, False:Missed
 **/
bool battle_check_sc(struct block_list *src, struct block_list *target, struct status_change *sc, struct Damage *d, int64 damage, uint16 skill_id, uint16 skill_lv) {
	if (!sc)
		return true;

	if (sc->data[SC_SAFETYWALL] && (d->flag&(BF_SHORT|BF_MAGIC)) == BF_SHORT) {
		struct skill_unit_group* group = skill_id2group(sc->data[SC_SAFETYWALL]->val3);
		uint16 skill_id_val = sc->data[SC_SAFETYWALL]->val2;

		if (group) {
			if (skill_id_val == MH_STEINWAND) {
				if (--group->val2 <= 0)
					skill_delunitgroup(group);
				d->dmg_lv = ATK_BLOCK;
				if( (group->val3 - damage) > 0 )
					group->val3 -= (int)cap_value(damage, INT_MIN, INT_MAX);
				else
					skill_delunitgroup(group);
				return false;
			}
			//in RE, SW possesses a lifetime equal to group val2, (3x caster hp, or homon formula)
			d->dmg_lv = ATK_BLOCK;
#ifdef RENEWAL
			if ( ( group->val2 - damage) > 0 ) {
				group->val2 -= (int)cap_value(damage, INT_MIN, INT_MAX);
			} else
				skill_delunitgroup(group);
			return false;
#else
			if (--group->val2 <= 0)
				skill_delunitgroup(group);
			return false;
#endif
		}
		status_change_end(target, SC_SAFETYWALL, INVALID_TIMER);
	}

	if( (sc->data[SC_NEUTRALBARRIER] || sc->data[SC_NEUTRALBARRIER_MASTER]) && !(skill_get_nk(skill_id)&NK_IGNORE_FLEE) &&
		(skill_id == NPC_EARTHQUAKE || (d->flag&(BF_LONG|BF_WEAPON)) == (BF_LONG|BF_WEAPON)) )
	{
		d->dmg_lv = ATK_MISS;
		return false;
	}

	if( sc->data[SC_PNEUMA] && (d->flag&(BF_MAGIC|BF_LONG)) == BF_LONG ) {
		d->dmg_lv = ATK_BLOCK;
		skill_blown(src,target,skill_get_blewcount(skill_id,skill_lv),-1,0);
		return false;
	}
	return true;
}

/**
 * Check damage through status.
 * ATK may be MISS, BLOCKED FAIL, reduc, increase, end status.
 * After this we apply bg/gvg reduction
 * @param src
 * @param bl
 * @param d
 * @param damage
 * @param skill_id
 * @param skill_lv
 * @return damage
 */
int64 battle_calc_damage(struct block_list *src,struct block_list *bl,struct Damage *d,int64 damage,uint16 skill_id,uint16 skill_lv)
{
	struct map_session_data *sd = NULL;
	struct status_change *sc;
	struct status_change_entry *sce;
	int div_ = d->div_, flag = d->flag;

	nullpo_ret(bl);

	if( !damage )
		return 0;
	if( battle_config.ksprotection && mob_ksprotected(src, bl) )
		return 0;

	if( map_getcell(bl->m, bl->x, bl->y, CELL_CHKMAELSTROM) && skill_get_type(skill_id) != BF_MISC
		&& skill_get_casttype(skill_id) == CAST_GROUND )
		return 0;

	if (bl->type == BL_PC) {
		sd=(struct map_session_data *)bl;
		//Special no damage states
		if(flag&BF_WEAPON && sd->special_state.no_weapon_damage)
			damage -= damage * sd->special_state.no_weapon_damage / 100;

		if(flag&BF_MAGIC && sd->special_state.no_magic_damage)
			damage -= damage * sd->special_state.no_magic_damage / 100;

		if(flag&BF_MISC && sd->special_state.no_misc_damage)
			damage -= damage * sd->special_state.no_misc_damage / 100;

		if(!damage)
			return 0;
	}

	sc = status_get_sc(bl); //check target status

	if( sc && sc->data[SC_INVINCIBLE] && !sc->data[SC_INVINCIBLEOFF] )
		return 1;

	if (skill_id == PA_PRESSURE || skill_id == HW_GRAVITATION)
		return damage; //These skills bypass everything else.

	if( sc && sc->count ) { // SC_* that reduce damage to 0.
		if( sc->data[SC_BASILICA] && !(status_get_mode(src)&MD_BOSS) ) {
			d->dmg_lv = ATK_BLOCK;
			return 0;
		}
		if( sc->data[SC_WHITEIMPRISON] ) { // Gravitation and Pressure do damage without removing the effect
			if( skill_id == MG_NAPALMBEAT ||
				skill_id == MG_SOULSTRIKE ||
				skill_id == WL_SOULEXPANSION ||
				(skill_id && skill_get_ele(skill_id, skill_lv) == ELE_GHOST) ||
				(!skill_id && (status_get_status_data(src))->rhw.ele == ELE_GHOST) )
			{
				if( skill_id == WL_SOULEXPANSION )
					damage <<= 1; // If used against a player in White Imprison, the skill deals double damage.
				status_change_end(bl,SC_WHITEIMPRISON,INVALID_TIMER); // Those skills do damage and removes effect
			} else {
				d->dmg_lv = ATK_BLOCK;
				return 0;
			}
		}

		if( sc->data[SC_ZEPHYR] && !(flag&BF_MAGIC && skill_id) && !(skill_get_inf(skill_id)&(INF_GROUND_SKILL|INF_SELF_SKILL)) ) {
			d->dmg_lv = ATK_BLOCK;
			return 0;
		}

		if (!battle_check_sc(src, bl, sc, d, damage, skill_id, skill_lv))
			return 0;

		if (sc->data[SC__MANHOLE] || (src->type == BL_PC && sc->data[SC_KINGS_GRACE])) {
			d->dmg_lv = ATK_BLOCK;
			return 0;
		}

		if( sc->data[SC_WEAPONBLOCKING] && flag&(BF_SHORT|BF_WEAPON) && rnd()%100 < sc->data[SC_WEAPONBLOCKING]->val2 ) {
			clif_skill_nodamage(bl,src,GC_WEAPONBLOCKING,sc->data[SC_WEAPONBLOCKING]->val1,1);
			sc_start2(src,bl,SC_COMBO,100,GC_WEAPONBLOCKING,src->id,skill_get_time2(GC_WEAPONBLOCKING,sc->data[SC_WEAPONBLOCKING]->val1));
			d->dmg_lv = ATK_BLOCK;
			return 0;
		}

		if( (sce = sc->data[SC_AUTOGUARD]) && flag&BF_WEAPON && !(skill_get_nk(skill_id)&NK_NO_CARDFIX_ATK) && rnd()%100 < sce->val2 ) {
			int delay;
			struct status_change_entry *sce_d = sc->data[SC_DEVOTION];
			struct block_list *d_bl = NULL;

			// different delay depending on skill level [celest]
			if (sce->val1 <= 5)
				delay = 300;
			else if (sce->val1 > 5 && sce->val1 <= 9)
				delay = 200;
			else
				delay = 100;
			if (sd && pc_issit(sd))
				pc_setstand(sd, true);
			if( sce_d && (d_bl = map_id2bl(sce_d->val1)) &&
				((d_bl->type == BL_MER && ((TBL_MER*)d_bl)->master && ((TBL_MER*)d_bl)->master->bl.id == bl->id) ||
				(d_bl->type == BL_PC && ((TBL_PC*)d_bl)->devotion[sce_d->val2] == bl->id)) &&
				check_distance_bl(bl,d_bl,sce_d->val3) )
			{ //If player is target of devotion, show guard effect on the devotion caster rather than the target
				clif_skill_nodamage(d_bl,d_bl,CR_AUTOGUARD,sce->val1,1);
				unit_set_walkdelay(d_bl,gettick(),delay,1);
				d->dmg_lv = ATK_MISS;
				return 0;
			} else {
				clif_skill_nodamage(bl,bl,CR_AUTOGUARD,sce->val1,1);
				unit_set_walkdelay(bl,gettick(),delay,1);
				if( sc->data[SC_SHRINK] && rnd()%100 < 5 * sce->val1 )
					skill_blown(bl,src,skill_get_blewcount(CR_SHRINK,1),-1,0);
				d->dmg_lv = ATK_MISS;
				return 0;
			}
		}

		if( (sce = sc->data[SC_MILLENNIUMSHIELD]) && sce->val2 > 0 && damage > 0 ) {
			sce->val3 -= (int)cap_value(damage,INT_MIN,INT_MAX); // absorb damage
			d->dmg_lv = ATK_BLOCK;
			if( sce->val3 <= 0 ) { // Shield Down
				sce->val2--;
				if( sce->val2 >= 0 ) {
					clif_millenniumshield(bl,sce->val2);
					if( !sce->val2 )
						status_change_end(bl,SC_MILLENNIUMSHIELD,INVALID_TIMER); // All shields down
					else
						sce->val3 = 1000; // Next shield
				}
				status_change_start(src,bl,SC_STUN,10000,0,0,0,0,1000,SCSTART_NOTICKDEF);
			}
			return 0;
		}

		// attack blocked by Parrying
		if( (sce = sc->data[SC_PARRYING]) && flag&BF_WEAPON && skill_id != WS_CARTTERMINATION && rnd()%100 < sce->val2 ) {
			clif_skill_nodamage(bl, bl, LK_PARRYING, sce->val1,1);
			return 0;
		}

		if(sc->data[SC_DODGE] && ( !sc->opt1 || sc->opt1 == OPT1_BURNING ) && (flag&BF_LONG || sc->data[SC_SPURT]) && rnd()%100 < 20) {
			if (sd && pc_issit(sd))
				pc_setstand(sd, true); //Stand it to dodge.
			clif_skill_nodamage(bl,bl,TK_DODGE,1,1);
			if (!sc->data[SC_COMBO])
				sc_start4(src,bl, SC_COMBO, 100, TK_JUMPKICK, src->id, 1, 0, 2000);
			return 0;
		}

		if(sc->data[SC_HERMODE] && flag&BF_MAGIC)
			return 0;

		if(sc->data[SC_TATAMIGAESHI] && (flag&(BF_MAGIC|BF_LONG)) == BF_LONG)
			return 0;

		//Kaupe blocks damage (skill or otherwise) from players, mobs, homuns, mercenaries.
		if ((sce = sc->data[SC_KAUPE]) && rnd()%100 < sce->val2) {
			clif_specialeffect(bl, 462, AREA);
			//Shouldn't end until Breaker's non-weapon part connects.
#ifndef RENEWAL
			if (skill_id != ASC_BREAKER || !(flag&BF_WEAPON))
#endif
				if (--(sce->val3) <= 0) //We make it work like Safety Wall, even though it only blocks 1 time.
					status_change_end(bl, SC_KAUPE, INVALID_TIMER);
			return 0;
		}

#ifdef RENEWAL // Flat +400% damage from melee
		if (sc->data[SC_KAITE] && (flag&(BF_SHORT|BF_MAGIC)) == BF_SHORT)
			damage <<= 2;
#endif

		if( flag&BF_MAGIC && (sce=sc->data[SC_PRESTIGE]) && rnd()%100 < sce->val2) {
			clif_specialeffect(bl, 462, AREA); // Still need confirm it.
			return 0;
		}

		if (((sce = sc->data[SC_UTSUSEMI]) || sc->data[SC_BUNSINJYUTSU]) && flag&BF_WEAPON && !(skill_get_nk(skill_id)&NK_NO_CARDFIX_ATK)) {
			skill_additional_effect (src, bl, skill_id, skill_lv, flag, ATK_BLOCK, gettick() );
			if (!status_isdead(src))
				skill_counter_additional_effect( src, bl, skill_id, skill_lv, flag, gettick() );
			if (sce) {
				clif_specialeffect(bl, 462, AREA);
				skill_blown(src,bl,sce->val3,-1,0);
			}
			//Both need to be consumed if they are active.
			if (sce && --(sce->val2) <= 0)
				status_change_end(bl, SC_UTSUSEMI, INVALID_TIMER);
			if ((sce = sc->data[SC_BUNSINJYUTSU]) && --(sce->val2) <= 0)
				status_change_end(bl, SC_BUNSINJYUTSU, INVALID_TIMER);

			return 0;
		}

		//Now damage increasing effects
		if (sc->data[SC_AETERNA] && skill_id != PF_SOULBURN) {
			if (src->type != BL_MER || !skill_id)
				damage <<= 1; // Lex Aeterna only doubles damage of regular attacks from mercenaries

#ifndef RENEWAL
			if( skill_id != ASC_BREAKER || !(flag&BF_WEAPON) )
#endif
				status_change_end(bl, SC_AETERNA, INVALID_TIMER); //Shouldn't end until Breaker's non-weapon part connects.
		}

#ifdef RENEWAL
		if( sc->data[SC_RAID] ) {
			damage += damage * 20 / 100;

			if (--sc->data[SC_RAID]->val1 == 0)
				status_change_end(bl, SC_RAID, INVALID_TIMER);
		}
#endif

		if( damage ) {
			struct map_session_data *tsd = BL_CAST(BL_PC, src);
			if( sc->data[SC_DEEPSLEEP] ) {
				damage += damage / 2; // 1.5 times more damage while in Deep Sleep.
				status_change_end(bl,SC_DEEPSLEEP,INVALID_TIMER);
			}
			if( tsd && sd && sc->data[SC_CRYSTALIZE] && flag&BF_WEAPON ) {
				switch(tsd->status.weapon) {
					case W_MACE:
					case W_2HMACE:
					case W_1HAXE:
					case W_2HAXE:
						damage += damage / 2;
						break;
					case W_MUSICAL:
					case W_WHIP:
						if(!sd->state.arrow_atk)
							break;
					case W_BOW:
					case W_REVOLVER:
					case W_RIFLE:
					case W_GATLING:
					case W_SHOTGUN:
					case W_GRENADE:
					case W_DAGGER:
					case W_1HSWORD:
					case W_2HSWORD:
						damage -= damage / 2;
						break;
				}
			}
			if( sc->data[SC_VOICEOFSIREN] )
				status_change_end(bl,SC_VOICEOFSIREN,INVALID_TIMER);
		}

		if( sc->data[SC_DEVOTION] ) {
			struct status_change_entry *sce_d = sc->data[SC_DEVOTION];
			struct block_list *d_bl = map_id2bl(sce_d->val1);

			if( d_bl &&
				((d_bl->type == BL_MER && ((TBL_MER*)d_bl)->master && ((TBL_MER*)d_bl)->master->bl.id == bl->id) ||
				(d_bl->type == BL_PC && ((TBL_PC*)d_bl)->devotion[sce_d->val2] == bl->id)) &&
				check_distance_bl(bl,d_bl,sce_d->val3) )
			{
				struct status_change *d_sc = status_get_sc(d_bl);

				if( d_sc && d_sc->data[SC_DEFENDER] && (flag&(BF_LONG|BF_MAGIC)) == BF_LONG && skill_id != ASC_BREAKER && skill_id != CR_ACIDDEMONSTRATION && skill_id != NJ_ZENYNAGE && skill_id != KO_MUCHANAGE )
					damage -= damage * d_sc->data[SC_DEFENDER]->val2 / 100;
			}
		}

		// Damage reductions
		// Assumptio doubles the def & mdef on RE mode, otherwise gives a reduction on the final damage. [Igniz]
#ifndef RENEWAL
		if( sc->data[SC_ASSUMPTIO] ) {
			if( map_flag_vs(bl->m) )
				damage = (int64)damage*2/3; //Receive 66% damage
			else
				damage >>= 1; //Receive 50% damage
		}
#endif

		if (sc->data[SC_DEFENDER] && (flag&(BF_LONG|BF_WEAPON)) == (BF_LONG|BF_WEAPON))
			damage -= damage * sc->data[SC_DEFENDER]->val2 / 100;

		if(sc->data[SC_ADJUSTMENT] && (flag&(BF_LONG|BF_WEAPON)) == (BF_LONG|BF_WEAPON))
			damage -= damage * 20 / 100;

		if(sc->data[SC_FOGWALL] && skill_id != RK_DRAGONBREATH && skill_id != RK_DRAGONBREATH_WATER) {
			if(flag&BF_SKILL) //25% reduction
				damage -= damage * 25 / 100;
			else if ((flag&(BF_LONG|BF_WEAPON)) == (BF_LONG|BF_WEAPON))
				damage >>= 2; //75% reduction
		}

		if(sc->data[SC_ARMORCHANGE]) {
			//On official servers, SC_ARMORCHANGE does not change DEF/MDEF but rather increases/decreases the damage
			if(flag&BF_WEAPON)
				damage -= damage * sc->data[SC_ARMORCHANGE]->val2 / 100;
			else if(flag&BF_MAGIC)
				damage -= damage * sc->data[SC_ARMORCHANGE]->val3 / 100;
		}

		if(sc->data[SC_SMOKEPOWDER]) {
			if( (flag&(BF_SHORT|BF_WEAPON)) == (BF_SHORT|BF_WEAPON) )
				damage -= damage * 15 / 100; // 15% reduction to physical melee attacks
			else if( (flag&(BF_LONG|BF_WEAPON)) == (BF_LONG|BF_WEAPON) )
				damage -= damage * 50 / 100; // 50% reduction to physical ranged attacks
		}

		if (sc->data[SC_WATER_BARRIER])
			damage = damage * 80 / 100; // 20% reduction to all type attacks

		// Compressed code, fixed by map.h [Epoque]
		if (src->type == BL_MOB) {
			int i;

			if (sc->data[SC_MANU_DEF]) {
				for (i=0;ARRAYLENGTH(mob_manuk)>i;i++) {
					if (mob_manuk[i]==((TBL_MOB*)src)->mob_id) {
						damage -= damage * sc->data[SC_MANU_DEF]->val1 / 100;
						break;
					}
				}
			}
			if (sc->data[SC_SPL_DEF]) {
				for (i=0;ARRAYLENGTH(mob_splendide)>i;i++) {
					if (mob_splendide[i]==((TBL_MOB*)src)->mob_id) {
						damage -= damage * sc->data[SC_SPL_DEF]->val1 / 100;
						break;
					}
				}
			}
		}

		if((sce=sc->data[SC_ARMOR]) && //NPC_DEFENDER
			sce->val3&flag && sce->val4&flag)
			damage -= damage * sc->data[SC_ARMOR]->val2 / 100;

		if( sc->data[SC_ENERGYCOAT] && (skill_id == GN_HELLS_PLANT_ATK ||
#ifdef RENEWAL
			((flag&BF_WEAPON || flag&BF_MAGIC) && skill_id != WS_CARTTERMINATION)
#else
			(flag&BF_WEAPON && skill_id != WS_CARTTERMINATION)
#endif
			) )
		{
			struct status_data *status = status_get_status_data(bl);
			int per = 100*status->sp / status->max_sp -1; //100% should be counted as the 80~99% interval
			per /=20; //Uses 20% SP intervals.
			//SP Cost: 1% + 0.5% per every 20% SP
			if (!status_charge(bl, 0, (10+5*per)*status->max_sp/1000))
				status_change_end(bl, SC_ENERGYCOAT, INVALID_TIMER);
			damage -= damage * 6 * (1 + per) / 100; //Reduction: 6% + 6% every 20%
		}

		if(sc->data[SC_GRANITIC_ARMOR])
			damage -= damage * sc->data[SC_GRANITIC_ARMOR]->val2 / 100;

		if(sc->data[SC_PAIN_KILLER])
			damage -= sc->data[SC_PAIN_KILLER]->val3;

		if( sc->data[SC_DARKCROW] && (flag&(BF_SHORT|BF_MAGIC)) == BF_SHORT )
			damage += damage * sc->data[SC_DARKCROW]->val2 / 100;

		if( (sce=sc->data[SC_MAGMA_FLOW]) && (rnd()%100 <= sce->val2) )
			skill_castend_damage_id(bl,src,MH_MAGMA_FLOW,sce->val1,gettick(),0);

		if( damage > 0 && ((flag&(BF_WEAPON|BF_SHORT)) == (BF_WEAPON|BF_SHORT)) && (sce = sc->data[SC_STONEHARDSKIN]) ) {
			sce->val2 -= (int)cap_value(damage,INT_MIN,INT_MAX);
			if( src->type == BL_MOB ) //using explicit call instead break_equip for duration
				sc_start(src,src, SC_STRIPWEAPON, 30, 0, skill_get_time2(RK_STONEHARDSKIN, sce->val1));
			else
				skill_break_equip(src,src, EQP_WEAPON, 3000, BCT_SELF);
			if( sce->val2 <= 0 )
				status_change_end(bl, SC_STONEHARDSKIN, INVALID_TIMER);
		}

#ifdef RENEWAL
		// Renewal: steel body reduces all incoming damage to 1/10 [helvetica]
		if( sc->data[SC_STEELBODY] )
			damage = damage > 10 ? damage / 10 : 1;
#endif

		//Finally added to remove the status of immobile when Aimed Bolt is used. [Jobbie]
		if( skill_id == RA_AIMEDBOLT && (sc->data[SC_BITE] || sc->data[SC_ANKLE] || sc->data[SC_ELECTRICSHOCKER]) ) {
			status_change_end(bl, SC_BITE, INVALID_TIMER);
			status_change_end(bl, SC_ANKLE, INVALID_TIMER);
			status_change_end(bl, SC_ELECTRICSHOCKER, INVALID_TIMER);
		}

		//Finally Kyrie because it may, or not, reduce damage to 0.
		if((sce = sc->data[SC_KYRIE]) && damage > 0){
			sce->val2 -= (int)cap_value(damage,INT_MIN,INT_MAX);
			if(flag&BF_WEAPON || skill_id == TF_THROWSTONE){
				if(sce->val2>=0)
					damage=0;
				else
				  	damage=-sce->val2;
			}
			if((--sce->val3)<=0 || (sce->val2<=0) || skill_id == AL_HOLYLIGHT)
				status_change_end(bl, SC_KYRIE, INVALID_TIMER);
		}

		if( sc->data[SC_MEIKYOUSISUI] && rnd()%100 < 40 ) // custom value
			damage = 0;


		if (!damage)
			return 0;

		if( (sce = sc->data[SC_LIGHTNINGWALK]) && flag&BF_LONG && rnd()%100 < sce->val1 ) {
			int dx[8] = { 0,-1,-1,-1,0,1,1,1 };
			int dy[8] = { 1,1,0,-1,-1,-1,0,1 };
			uint8 dir = map_calc_dir(bl, src->x, src->y);

			if( unit_movepos(bl, src->x-dx[dir], src->y-dy[dir], 1, 1) ) {
				clif_blown(bl);
				unit_setdir(bl, dir);
			}
			d->dmg_lv = ATK_DEF;
			status_change_end(bl, SC_LIGHTNINGWALK, INVALID_TIMER);
			return 0;
		}

		if( sd && (sce = sc->data[SC_FORCEOFVANGUARD]) && flag&BF_WEAPON && rnd()%100 < sce->val2 )
			pc_addspiritball(sd,skill_get_time(LG_FORCEOFVANGUARD,sce->val1),sce->val3);

		if( sd && (sce = sc->data[SC_GT_ENERGYGAIN]) && flag&BF_WEAPON && rnd()%100 < sce->val2 ) {
			int spheres = 5;

			if( sc->data[SC_RAISINGDRAGON] )
				spheres += sc->data[SC_RAISINGDRAGON]->val1;

			pc_addspiritball(sd, skill_get_time2(SR_GENTLETOUCH_ENERGYGAIN, sce->val1), spheres);
		}

		if (sc->data[SC_STYLE_CHANGE] && sc->data[SC_STYLE_CHANGE]->val1 == MH_MD_GRAPPLING) {
			TBL_HOM *hd = BL_CAST(BL_HOM,bl); // We add a sphere for when the Homunculus is being hit

			if (hd && (rnd()%100<50) ) // According to WarpPortal, this is a flat 50% chance
				hom_addspiritball(hd, 10);
		}

		if( sc->data[SC__DEADLYINFECT] && (flag&(BF_SHORT|BF_MAGIC)) == BF_SHORT && damage > 0 && rnd()%100 < 30 + 10 * sc->data[SC__DEADLYINFECT]->val1 )
			status_change_spread(bl, src, 1); // Deadly infect attacked side

	} //End of target SC_ check

	//SC effects from caster side.
	sc = status_get_sc(src);

	if (sc && sc->count) {
		if( sc->data[SC_INVINCIBLE] && !sc->data[SC_INVINCIBLEOFF] )
			damage += damage * 75 / 100;

		if ((sce = sc->data[SC_BLOODLUST]) && flag&BF_WEAPON && damage > 0 && rnd()%100 < sce->val3)
			status_heal(src, damage * sce->val4 / 100, 0, 3);

		// [Epoque]
		if (bl->type == BL_MOB) {
			uint8 i;

			if ( ((sce=sc->data[SC_MANU_ATK]) && (flag&BF_WEAPON)) ||
				 ((sce=sc->data[SC_MANU_MATK]) && (flag&BF_MAGIC))
				) {
				for (i=0;ARRAYLENGTH(mob_manuk)>i;i++) {
					if (((TBL_MOB*)bl)->mob_id==mob_manuk[i]) {
						damage += damage * sce->val1 / 100;
						break;
					}
				}
			}
			if ( ((sce=sc->data[SC_SPL_ATK]) && (flag&BF_WEAPON)) ||
				 ((sce=sc->data[SC_SPL_MATK]) && (flag&BF_MAGIC))
				) {
				for (i=0;ARRAYLENGTH(mob_splendide)>i;i++) {
					if (((TBL_MOB*)bl)->mob_id==mob_splendide[i]) {
						damage += damage * sce->val1 / 100;
						break;
					}
				}
			}
		}
		/* Self Buff that destroys the armor of any target hit with melee or ranged physical attacks */
		if( sc->data[SC_SHIELDSPELL_REF] && sc->data[SC_SHIELDSPELL_REF]->val1 == 1 && flag&BF_WEAPON ) {
			skill_break_equip(src,bl, EQP_ARMOR, 10000, BCT_ENEMY); // 100% chance (http://irowiki.org/wiki/Shield_Spell#Level_3_spells_.28refine_based.29)
			status_change_end(src,SC_SHIELDSPELL_REF,INVALID_TIMER);
		}

		if( sc->data[SC_POISONINGWEAPON]
			&& ((flag&BF_WEAPON) && (!skill_id || skill_id == GC_VENOMPRESSURE)) //check skill type poison_smoke is a unit
			&& (damage > 0 && rnd()%100 < sc->data[SC_POISONINGWEAPON]->val3 )) //did some damage and chance ok (why no additional effect ??)
			sc_start(src,bl,(enum sc_type)sc->data[SC_POISONINGWEAPON]->val2,100,sc->data[SC_POISONINGWEAPON]->val1,skill_get_time2(GC_POISONINGWEAPON, 1));

		if( sc->data[SC__DEADLYINFECT] && (flag&(BF_SHORT|BF_MAGIC)) == BF_SHORT && damage > 0 && rnd()%100 < 30 + 10 * sc->data[SC__DEADLYINFECT]->val1 )
			status_change_spread(src, bl, 0);

		if (sc->data[SC_STYLE_CHANGE] && sc->data[SC_STYLE_CHANGE]->val1 == MH_MD_FIGHTING) {
			TBL_HOM *hd = BL_CAST(BL_HOM,src); //when attacking

			if (hd && (rnd()%100<50) ) hom_addspiritball(hd, 10); // According to WarpPortal, this is a flat 50% chance
		}
	} //End of caster SC_ check

	//PK damage rates
	if (battle_config.pk_mode && sd && bl->type == BL_PC && damage && map[bl->m].flag.pvp) {
		if (flag & BF_SKILL) { //Skills get a different reduction than non-skills. [Skotlex]
			if (flag&BF_WEAPON)
				damage = damage * battle_config.pk_weapon_damage_rate / 100;
			if (flag&BF_MAGIC)
				damage = damage * battle_config.pk_magic_damage_rate / 100;
			if (flag&BF_MISC)
				damage = damage * battle_config.pk_misc_damage_rate / 100;
		} else { //Normal attacks get reductions based on range.
			if (flag & BF_SHORT)
				damage = damage * battle_config.pk_short_damage_rate / 100;
			if (flag & BF_LONG)
				damage = damage * battle_config.pk_long_damage_rate / 100;
		}
		damage = max(damage,1);
	}

	if(battle_config.skill_min_damage && damage > 0 && damage < div_) {
		if ((flag&BF_WEAPON && battle_config.skill_min_damage&1)
			|| (flag&BF_MAGIC && battle_config.skill_min_damage&2)
			|| (flag&BF_MISC && battle_config.skill_min_damage&4)
		)
			damage = div_;
	}

	if( bl->type == BL_MOB && !status_isdead(bl) && src != bl) {
		if (damage > 0 )
			mobskill_event((TBL_MOB*)bl,src,gettick(),flag);
		if (skill_id)
			mobskill_event((TBL_MOB*)bl,src,gettick(),MSC_SKILLUSED|(skill_id<<16));
	}
	if( sd ) {
		if( pc_ismadogear(sd) && rnd()%100 < 50 ) {
			short element = skill_get_ele(skill_id, skill_lv);
			if( !skill_id || element == -1 ) { //Take weapon's element
				struct status_data *sstatus = NULL;
				if( src->type == BL_PC && ((TBL_PC*)src)->bonus.arrow_ele )
					element = ((TBL_PC*)src)->bonus.arrow_ele;
				else if( (sstatus = status_get_status_data(src)) ) {
					element = sstatus->rhw.ele;
				}
			} else if( element == -2 ) //Use enchantment's element
				element = status_get_attack_sc_element(src,status_get_sc(src));
			else if( element == -3 ) //Use random element
				element = rnd()%ELE_ALL;
			if( element == ELE_FIRE || element == ELE_WATER )
				pc_overheat(sd,element == ELE_FIRE ? 1 : -1);
		}
	}

	return damage;
}

/**
 * Calculates BG related damage adjustments.
 * @param src
 * @param bl
 * @param damage
 * @param skill_id
 * @param flag
 * @return damage
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
int64 battle_calc_bg_damage(struct block_list *src, struct block_list *bl, int64 damage, uint16 skill_id, int flag)
{
	if( !damage )
		return 0;

	if( bl->type == BL_MOB ) {
		struct mob_data* md = BL_CAST(BL_MOB, bl);
		if( map[bl->m].flag.battleground && (md->mob_id == MOBID_BLUE_CRYST || md->mob_id == MOBID_PINK_CRYST) && flag&BF_SKILL )
			return 0; // Crystal cannot receive skill damage on battlegrounds
	}

	if(skill_get_inf2(skill_id)&INF2_NO_BG_DMG)
		return damage; //skill that ignore bg map reduction

	if( flag&BF_SKILL ) { //Skills get a different reduction than non-skills. [Skotlex]
		if( flag&BF_WEAPON )
			damage = damage * battle_config.bg_weapon_damage_rate / 100;
		if( flag&BF_MAGIC )
			damage = damage * battle_config.bg_magic_damage_rate / 100;
		if(	flag&BF_MISC )
			damage = damage * battle_config.bg_misc_damage_rate / 100;
	} else { //Normal attacks get reductions based on range.
		if( flag&BF_SHORT )
			damage = damage * battle_config.bg_short_damage_rate / 100;
		if( flag&BF_LONG )
			damage = damage * battle_config.bg_long_damage_rate / 100;
	}

	damage = max(damage,1); //min 1 damage
	return damage;
}

/**
 * Determines whether target can be hit
 * @param src
 * @param bl
 * @param skill_id
 * @param flag
 * @return Can be hit (true) or can't be hit (false)
 */
bool battle_can_hit_gvg_target(struct block_list *src,struct block_list *bl,uint16 skill_id,int flag)
{
	struct mob_data* md = BL_CAST(BL_MOB, bl);
	int class_ = status_get_class(bl);

	if(md && md->guardian_data) {
		if(class_ == MOBID_EMPERIUM && flag&BF_SKILL && !(skill_get_inf3(skill_id)&INF3_HIT_EMP)) //Skill immunity.
			return false;
		if(src->type != BL_MOB) {
			struct guild *g = src->type == BL_PC ? ((TBL_PC *)src)->guild : guild_search(status_get_guild_id(src));

			if (class_ == MOBID_EMPERIUM && (!g || guild_checkskill(g,GD_APPROVAL) <= 0 ))
				return false;

			if (g && battle_config.guild_max_castles && guild_checkcastles(g)>=battle_config.guild_max_castles)
				return false; // [MouseJstr]
		}
	}
	return true;
}

/**
 * Calculates GVG related damage adjustments.
 * @param src
 * @param bl
 * @param damage
 * @param skill_id
 * @param flag
 * @return damage
 */
int64 battle_calc_gvg_damage(struct block_list *src,struct block_list *bl,int64 damage,uint16 skill_id,int flag)
{
	if (!damage) //No reductions to make.
		return 0;

	if (!battle_can_hit_gvg_target(src,bl,skill_id,flag))
		return 0;

	if (skill_get_inf2(skill_id)&INF2_NO_GVG_DMG) //Skills with no gvg damage reduction.
		return damage;
	/* Uncomment if you want god-mode Emperiums at 100 defense. [Kisuka]
	if (md && md->guardian_data)
		damage -= damage * (md->guardian_data->castle->defense/100) * battle_config.castle_defense_rate/100;
	*/
	if (flag & BF_SKILL) { //Skills get a different reduction than non-skills. [Skotlex]
		if (flag&BF_WEAPON)
			damage = damage * battle_config.gvg_weapon_damage_rate / 100;
		if (flag&BF_MAGIC)
			damage = damage * battle_config.gvg_magic_damage_rate / 100;
		if (flag&BF_MISC)
			damage = damage * battle_config.gvg_misc_damage_rate / 100;
	} else { //Normal attacks get reductions based on range.
		if (flag & BF_SHORT)
			damage = damage * battle_config.gvg_short_damage_rate / 100;
		if (flag & BF_LONG)
			damage = damage * battle_config.gvg_long_damage_rate / 100;
	}
	damage = max(damage,1);
	return damage;
}

/**
 * HP/SP drain calculation
 * @param damage Damage inflicted to the enemy
 * @param rate Success chance 1000 = 100%
 * @param per HP/SP drained
 * @return diff
 */
static int battle_calc_drain(int64 damage, int rate, int per)
{
	int64 diff = 0;

	if (per && (rate > 1000 || rnd()%1000 < rate)) {
		diff = (damage * per) / 100;
		if (diff == 0) {
			if (per > 0)
				diff = 1;
			else
				diff = -1;
		}
	}
	return (int)cap_value(diff, INT_MIN, INT_MAX);
}

/**
 * Passive skill damage increases
 * @param sd
 * @param target
 * @param dmg
 * @param type
 * @return damage
 */
int64 battle_addmastery(struct map_session_data *sd,struct block_list *target,int64 dmg,int type)
{
	int64 damage;
	struct status_data *status = status_get_status_data(target);
	int weapon, skill;

#ifdef RENEWAL
	damage = 0;
#else
	damage = dmg;
#endif

	nullpo_ret(sd);

	if((skill = pc_checkskill(sd,AL_DEMONBANE)) > 0 &&
		target->type == BL_MOB && //This bonus doesn't work against players.
		(battle_check_undead(status->race,status->def_ele) || status->race == RC_DEMON) )
		damage += (skill*(int)(3+(sd->status.base_level+1)*0.05));	// submitted by orn
	if( (skill = pc_checkskill(sd, RA_RANGERMAIN)) > 0 && (status->race == RC_BRUTE || status->race == RC_PLANT || status->race == RC_FISH) )
		damage += (skill * 5);
	if( (skill = pc_checkskill(sd,NC_RESEARCHFE)) > 0 && (status->def_ele == ELE_FIRE || status->def_ele == ELE_EARTH) )
		damage += (skill * 10);

	damage += (15 * pc_checkskill(sd, NC_MADOLICENCE)); // Attack bonus is granted even without the Madogear

	if((skill = pc_checkskill(sd,HT_BEASTBANE)) > 0 && (status->race == RC_BRUTE || status->race == RC_INSECT) ) {
		damage += (skill * 4);
		if (sd->sc.data[SC_SPIRIT] && sd->sc.data[SC_SPIRIT]->val2 == SL_HUNTER)
			damage += sd->status.str;
	}

#ifdef RENEWAL
	//Weapon Research bonus applies to all weapons
	if((skill = pc_checkskill(sd,BS_WEAPONRESEARCH)) > 0)
		damage += (skill * 2);
#endif

	if(type == 0)
		weapon = sd->weapontype1;
	else
		weapon = sd->weapontype2;

	switch(weapon) {
		case W_1HSWORD:
#ifdef RENEWAL
			if((skill = pc_checkskill(sd,AM_AXEMASTERY)) > 0)
				damage += (skill * 3);
#endif
		case W_DAGGER:
			if((skill = pc_checkskill(sd,SM_SWORD)) > 0)
				damage += (skill * 4);
			if((skill = pc_checkskill(sd,GN_TRAINING_SWORD)) > 0)
				damage += skill * 10;
			break;
		case W_2HSWORD:
#ifdef RENEWAL
			if((skill = pc_checkskill(sd,AM_AXEMASTERY)) > 0)
				damage += (skill * 3);
#endif
			if((skill = pc_checkskill(sd,SM_TWOHAND)) > 0)
				damage += (skill * 4);
			break;
		case W_1HSPEAR:
		case W_2HSPEAR:
			if((skill = pc_checkskill(sd,KN_SPEARMASTERY)) > 0) {
				if(!pc_isriding(sd) || !pc_isridingdragon(sd))
					damage += (skill * 4);
				else
					damage += (skill * 5);
				// Increase damage by level of KN_SPEARMASTERY * 10
				if(pc_checkskill(sd,RK_DRAGONTRAINING) > 0)
					damage += (skill * 10);
			}
			break;
		case W_1HAXE:
		case W_2HAXE:
			if((skill = pc_checkskill(sd,AM_AXEMASTERY)) > 0)
				damage += (skill * 3);
			if((skill = pc_checkskill(sd,NC_TRAININGAXE)) > 0)
				damage += (skill * 5);
			break;
		case W_MACE:
		case W_2HMACE:
			if((skill = pc_checkskill(sd,PR_MACEMASTERY)) > 0)
				damage += (skill * 3);
			if((skill = pc_checkskill(sd,NC_TRAININGAXE)) > 0)
				damage += (skill * 4);
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

#ifdef RENEWAL
static int battle_calc_sizefix(int64 damage, struct map_session_data *sd, unsigned char t_size, unsigned char weapon_type, short flag)
{
	if (sd && !sd->special_state.no_sizefix && !flag) // Size fix only for players
		damage = damage * (weapon_type == EQI_HAND_L ? sd->left_weapon.atkmods[t_size] : sd->right_weapon.atkmods[t_size]) / 100;

	return (int)cap_value(damage, INT_MIN, INT_MAX);
}

static int battle_calc_status_attack(struct status_data *status, short hand)
{
	//left-hand penalty on sATK is always 50% [Baalberith]
	if (hand == EQI_HAND_L)
		return status->batk;
	else
		return 2 * status->batk;
}

static int battle_calc_base_weapon_attack(struct block_list *src, struct status_data *tstatus, struct weapon_atk *wa, struct map_session_data *sd)
{
	struct status_data *status = status_get_status_data(src);
	uint8 type = (wa == &status->lhw)?EQI_HAND_L:EQI_HAND_R;
	uint16 atkmin = (type == EQI_HAND_L)?status->watk2:status->watk;
	uint16 atkmax = atkmin;
	int64 damage = atkmin;
	uint16 weapon_perfection = 0;
	struct status_change *sc = status_get_sc(src);

	if (sd->equip_index[type] >= 0 && sd->inventory_data[sd->equip_index[type]]) {
		int variance =   wa->atk * (sd->inventory_data[sd->equip_index[type]]->wlv*5)/100;

		atkmin = max(0, (int)(atkmin - variance));
		atkmax = min(UINT16_MAX, (int)(atkmax + variance));

		if (sc && sc->data[SC_MAXIMIZEPOWER])
			damage = atkmax;
		else
			damage = rnd_value(atkmin, atkmax);
	}

	if (sc && sc->data[SC_WEAPONPERFECTION])
		weapon_perfection = 1;

	damage = battle_calc_sizefix(damage, sd, tstatus->size, type, weapon_perfection);

	return (int)cap_value(damage, INT_MIN, INT_MAX);
}
#endif

/*==========================================
 * Calculates the standard damage of a normal attack assuming it hits,
 * it calculates nothing extra fancy, is needed for magnum break's WATK_ELEMENT bonus. [Skotlex]
 * This applies to pre-renewal and non-sd in renewal
 *------------------------------------------
 * Pass damage2 as NULL to not calc it.
 * Flag values:
 * &1 : Critical hit
 * &2 : Arrow attack
 * &4 : Skill is Magic Crasher
 * &8 : Skip target size adjustment (Extremity Fist?)
 * &16: Arrow attack but BOW, REVOLVER, RIFLE, SHOTGUN, GATLING or GRENADE type weapon not equipped (i.e. shuriken, kunai and venom knives not affected by DEX)
 *
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static int64 battle_calc_base_damage(struct status_data *status, struct weapon_atk *wa, struct status_change *sc, unsigned short t_size, struct map_session_data *sd, int flag)
{
	unsigned int atkmin = 0, atkmax = 0;
	short type = 0;
	int64 damage = 0;

	if (!sd) { //Mobs/Pets
		if(flag&4) {
			atkmin = status->matk_min;
			atkmax = status->matk_max;
		} else {
			atkmin = wa->atk;
			atkmax = wa->atk2;
		}
		if (atkmin > atkmax)
			atkmin = atkmax;
	} else { //PCs
		atkmax = wa->atk;
		type = (wa == &status->lhw)?EQI_HAND_L:EQI_HAND_R;

		if (!(flag&1) || (flag&2)) { //Normal attacks
			atkmin = status->dex;

			if (sd->equip_index[type] >= 0 && sd->inventory_data[sd->equip_index[type]])
				atkmin = atkmin*(80 + sd->inventory_data[sd->equip_index[type]]->wlv*20)/100;

			if (atkmin > atkmax)
				atkmin = atkmax;

			if(flag&2 && !(flag&16)) { //Bows
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
		damage = (atkmax>atkmin? rnd()%(atkmax-atkmin):0)+atkmin;
	else
		damage = atkmax;

	if (sd) {
		//rodatazone says the range is 0~arrow_atk-1 for non crit
		if (flag&2 && sd->bonus.arrow_atk)
			damage += ( (flag&1) ? sd->bonus.arrow_atk : rnd()%sd->bonus.arrow_atk );

		// Size fix only for players
		if (!(sd->special_state.no_sizefix || (flag&8)))
			damage = damage * (type == EQI_HAND_L ? sd->left_weapon.atkmods[t_size] : sd->right_weapon.atkmods[t_size]) / 100;
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
				damage += rnd()%sd->left_weapon.overrefine+1;
			if (sd->weapon_atk_rate[sd->weapontype2])
				damage += damage * sd->weapon_atk_rate[sd->weapontype2] / 100;
		} else { //Right hand
			if(sd->right_weapon.overrefine)
				damage += rnd()%sd->right_weapon.overrefine+1;
			if (sd->weapon_atk_rate[sd->weapontype1])
				damage += damage * sd->weapon_atk_rate[sd->weapontype1] / 100;
		}
	}

#ifdef RENEWAL
	if (flag&1)
		damage = (damage * 14) / 10;
#endif

	return damage;
}

/*==========================================
 * Consumes ammo for the given skill.
 *------------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
void battle_consume_ammo(TBL_PC*sd, int skill, int lv)
{
	int qty = 1;

	if (!battle_config.arrow_decrement)
		return;

	if (skill) {
		qty = skill_get_ammo_qty(skill, lv);
		if (!qty) qty = 1;
	}

	if (sd->equip_index[EQI_AMMO] >= 0) //Qty check should have been done in skill_check_condition
		pc_delitem(sd,sd->equip_index[EQI_AMMO],qty,0,1,LOG_TYPE_CONSUME);

	sd->state.arrow_atk = 0;
}

static int battle_range_type(struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv)
{
	// [Akinari] , [Xynvaroth]: Traps are always short range.
	if( skill_get_inf2( skill_id ) & INF2_TRAP )
		return BF_SHORT;

	//Skill Range Criteria
	if (battle_config.skillrange_by_distance &&
		(src->type&battle_config.skillrange_by_distance)
	) { //based on distance between src/target [Skotlex]
		if (check_distance_bl(src, target, 5))
			return BF_SHORT;
		return BF_LONG;
	}

	if (skill_id == SR_GATEOFHELL) {
		if (skill_lv < 5)
			return BF_SHORT;
		else
			return BF_LONG;
	}

	//based on used skill's range
	if (skill_get_range2(src, skill_id, skill_lv) < 5)
		return BF_SHORT;
	return BF_LONG;
}

static int battle_blewcount_bonus(struct map_session_data *sd, uint16 skill_id)
{
	uint8 i;
	if (!sd->skillblown[0].id)
		return 0;
	//Apply the bonus blewcount. [Skotlex]
	for (i = 0; i < ARRAYLENGTH(sd->skillblown) && sd->skillblown[i].id; i++) {
		if (sd->skillblown[i].id == skill_id)
			return sd->skillblown[i].val;
	}
	return 0;
}

#ifdef ADJUST_SKILL_DAMAGE
/**
 * Damage calculation for adjusting skill damage
 * @param caster Applied caster type for damage skill
 * @param type BL_Type of attacker
 */
static bool battle_skill_damage_iscaster(uint8 caster, enum bl_type src_type) {
	if (caster == 0)
		return false;

	switch (src_type) {
		case BL_PC: if (caster&SDC_PC) return true; break;
		case BL_MOB: if (caster&SDC_MOB) return true; break;
		case BL_PET: if (caster&SDC_PET) return true; break;
		case BL_HOM: if (caster&SDC_HOM) return true; break;
		case BL_MER: if (caster&SDC_MER) return true; break;
		case BL_ELEM: if (caster&SDC_ELEM) return true; break;
	}
	return false;
}

/**
 * Gets skill damage rate from a skill (based on skill_damage_db.txt)
 * @param src
 * @param target
 * @param skill_id
 * @return Skill damage rate
 */
static int battle_skill_damage_skill(struct block_list *src, struct block_list *target, uint16 skill_id) {
	uint16 idx = skill_get_index(skill_id), m = src->m;
	struct s_skill_damage *damage = NULL;
	struct map_data *mapd = &map[m];

	if (!idx || !skill_db[idx]->damage.map)
		return 0;

	damage = &skill_db[idx]->damage;

	//check the adjustment works for specified type
	if (!battle_skill_damage_iscaster(damage->caster, src->type))
		return 0;

	if ((damage->map&1 && (!mapd->flag.pvp && !map_flag_gvg(m) && !mapd->flag.battleground && !mapd->flag.skill_damage && !mapd->flag.restricted)) ||
		(damage->map&2 && mapd->flag.pvp) ||
		(damage->map&4 && map_flag_gvg(m)) ||
		(damage->map&8 && mapd->flag.battleground) ||
		(damage->map&16 && mapd->flag.skill_damage) ||
		(mapd->flag.restricted && damage->map&(8*mapd->zone)))
	{
		switch (target->type) {
			case BL_PC:
				return damage->pc;
			case BL_MOB:
				if (is_boss(target))
					return damage->boss;
				else
					return damage->mob;
			default:
				return damage->other;
		}
	}

	return 0;
}

/**
 * Gets skill damage rate from a skill (based on 'skill_damage' mapflag)
 * @param src
 * @param target
 * @param skill_id
 * @return Skill damage rate
 */
static int battle_skill_damage_map(struct block_list *src, struct block_list *target, uint16 skill_id) {
	int rate = 0;
	uint8 i = 0;
	struct map_data *mapd = NULL;

	mapd = &map[src->m];

	if (!mapd || !mapd->flag.skill_damage)
		return 0;

	// Damage rate for all skills at this map
	if (battle_skill_damage_iscaster(mapd->adjust.damage.caster, src->type)) {
		switch (target->type) {
			case BL_PC:
				rate = mapd->adjust.damage.pc;
				break;
			case BL_MOB:
				if (is_boss(target))
					rate = mapd->adjust.damage.boss;
				else
					rate = mapd->adjust.damage.mob;
				break;
			default:
				rate = mapd->adjust.damage.other;
				break;
		}
	}

	if (!mapd->skill_damage.count)
		return rate;

	// Damage rate for specified skill at this map
	for (i = 0; i < mapd->skill_damage.count; i++) {
		if (mapd->skill_damage.entries[i]->skill_id == skill_id &&
			battle_skill_damage_iscaster(mapd->skill_damage.entries[i]->caster, src->type))
		{
			switch (target->type) {
				case BL_PC:
					rate += mapd->skill_damage.entries[i]->pc;
					break;
				case BL_MOB:
					if (is_boss(target))
						rate += mapd->skill_damage.entries[i]->boss;
					else
						rate += mapd->skill_damage.entries[i]->mob;
					break;
				default:
					rate += mapd->skill_damage.entries[i]->other;
					break;
			}
		}
	}
	return rate;
}

/**
 * Check skill damage adjustment based on mapflags and skill_damage_db.txt for specified skill
 * @param src
 * @param target
 * @param skill_id
 * @return Total damage rate
 */
static int battle_skill_damage(struct block_list *src, struct block_list *target, uint16 skill_id) {
	nullpo_ret(src);
	if (!target || !skill_id)
		return 0;
	skill_id = skill_dummy2skill_id(skill_id);
	return battle_skill_damage_skill(src, target, skill_id) + battle_skill_damage_map(src, target, skill_id);
}
#endif

/**
 * Calculates Minstrel/Wanderer bonus for Chorus skills.
 * @param sd Player who has Chorus skill active
 * @return Bonus value based on party count
 */
static int battle_calc_chorusbonus(struct map_session_data *sd) {
	int members = 0;

	if (!sd || !sd->status.party_id)
		return 0;

	members = party_foreachsamemap(party_sub_count_class, sd, MAPID_THIRDMASK, MAPID_MINSTRELWANDERER);

	if (members < 3)
		return 0; // Bonus remains 0 unless 3 or more Minstrels/Wanderers are in the party.
	if (members > 7)
		return 5; // Maximum effect possible from 7 or more Minstrels/Wanderers.
	return members - 2; // Effect bonus from additional Minstrels/Wanderers if not above the max possible.
}

struct Damage battle_calc_magic_attack(struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv,int mflag);
struct Damage battle_calc_misc_attack(struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv,int mflag);

/*=======================================================
 * Should infinite defense be applied on target? (plant)
 *-------------------------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
  *	flag - see e_battle_flag
 */
bool is_infinite_defense(struct block_list *target, int flag)
{
	struct status_data *tstatus = status_get_status_data(target);

	if(target->type == BL_SKILL) {
		TBL_SKILL *su = ((TBL_SKILL*)target);

		if (su && su->group && (su->group->skill_id == WM_REVERBERATION || su->group->skill_id == WM_POEMOFNETHERWORLD))
			return true;
	}

	if(tstatus->mode&MD_IGNOREMELEE && (flag&(BF_WEAPON|BF_SHORT)) == (BF_WEAPON|BF_SHORT) )
		return true;
	if(tstatus->mode&MD_IGNOREMAGIC && flag&(BF_MAGIC) )
		return true;
	if(tstatus->mode&MD_IGNORERANGED && (flag&(BF_WEAPON|BF_LONG)) == (BF_WEAPON|BF_LONG) )
		return true;
	if(tstatus->mode&MD_IGNOREMISC && flag&(BF_MISC) )
		return true;

	return (tstatus->mode&MD_PLANT);
}

/*========================
 * Is attack arrow based?
 *------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static bool is_skill_using_arrow(struct block_list *src, int skill_id)
{
	if(src != NULL) {
		struct status_data *sstatus = status_get_status_data(src);
		struct map_session_data *sd = BL_CAST(BL_PC, src);

		return ((sd && sd->state.arrow_atk) || (!sd && ((skill_id && skill_get_ammotype(skill_id)) || sstatus->rhw.range>3)) || (skill_id == HT_PHANTASMIC));
	} else
		return false;
}

/*=========================================
 * Is attack right handed? By default yes.
 *-----------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static bool is_attack_right_handed(struct block_list *src, int skill_id)
{
	if(src != NULL) {
		struct map_session_data *sd = BL_CAST(BL_PC, src);

		//Skills ALWAYS use ONLY your right-hand weapon (tested on Aegis 10.2)
		if(!skill_id && sd && sd->weapontype1 == 0 && sd->weapontype2 > 0)
			return false;
	}
	return true;
}

/*=======================================
 * Is attack left handed? By default no.
 *---------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static bool is_attack_left_handed(struct block_list *src, int skill_id)
{
	if(src != NULL) {
		struct status_data *sstatus = status_get_status_data(src);
		struct map_session_data *sd = BL_CAST(BL_PC, src);

		//Skills ALWAYS use ONLY your right-hand weapon (tested on Aegis 10.2)
		if(!skill_id) {
			if (sd && sd->weapontype1 == 0 && sd->weapontype2 > 0)
				return true;

			if (sstatus->lhw.atk)
				return true;

			if (sd && sd->status.weapon == W_KATAR)
				return true;
		}
	}
	return false;
}

/*=============================
 * Do we score a critical hit?
 *-----------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static bool is_attack_critical(struct Damage wd, struct block_list *src, struct block_list *target, int skill_id, int skill_lv, bool first_call)
{
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);
	struct status_change *sc = status_get_sc(src);
	struct status_change *tsc = status_get_sc(target);
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	struct map_session_data *tsd = BL_CAST(BL_PC, target);

	if (!first_call)
		return (wd.type == DMG_CRITICAL);

	if (skill_id == NPC_CRITICALSLASH || skill_id == LG_PINPOINTATTACK) //Always critical skills
		return true;

	if( !(wd.type&DMG_MULTI_HIT) && sstatus->cri && (!skill_id ||
		skill_id == KN_AUTOCOUNTER || skill_id == SN_SHARPSHOOTING ||
		skill_id == MA_SHARPSHOOTING || skill_id == NJ_KIRIKAGE))
	{
		short cri = sstatus->cri;

		if (sd) {
			cri += sd->critaddrace[tstatus->race] + sd->critaddrace[RC_ALL];
			if(is_skill_using_arrow(src, skill_id))
				cri += sd->bonus.arrow_cri;
		}

		if(sc && sc->data[SC_CAMOUFLAGE])
			cri += 100 * min(10,sc->data[SC_CAMOUFLAGE]->val3); //max 100% (1K)

		//The official equation is *2, but that only applies when sd's do critical.
		//Therefore, we use the old value 3 on cases when an sd gets attacked by a mob
		cri -= tstatus->luk * ((!sd && tsd) ? 3 : 2);

		if( tsc && tsc->data[SC_SLEEP] )
			cri <<= 1;

		switch(skill_id) {
			case 0:
				if(sc && !sc->data[SC_AUTOCOUNTER])
					break;
				clif_specialeffect(src, 131, AREA);
				status_change_end(src, SC_AUTOCOUNTER, INVALID_TIMER);
			case KN_AUTOCOUNTER:
				if(battle_config.auto_counter_type &&
					(battle_config.auto_counter_type&src->type))
					return true;
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
		if(tsd && tsd->bonus.critical_def)
			cri = cri * ( 100 - tsd->bonus.critical_def ) / 100;
		return (rnd()%1000 < cri);
	}
	return 0;
}

/*==========================================================
 * Is the attack piercing? (Investigate/Ice Pick in pre-re)
 *----------------------------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static int is_attack_piercing(struct Damage wd, struct block_list *src, struct block_list *target, int skill_id, int skill_lv, short weapon_position)
{
	if (skill_id == MO_INVESTIGATE || skill_id == RL_MASS_SPIRAL)
		return 2;

	if(src != NULL) {
		struct map_session_data *sd = BL_CAST(BL_PC, src);
		struct status_data *tstatus = status_get_status_data(target);
#ifdef RENEWAL
		if( skill_id != PA_SACRIFICE && skill_id != CR_GRANDCROSS && skill_id != NPC_GRANDDARKNESS
			&& skill_id != PA_SHIELDCHAIN && skill_id != KO_HAPPOKUNAI && skill_id != ASC_BREAKER ) // Renewal: Soul Breaker no longer gains ice pick effect and ice pick effect gets crit benefit [helvetica]
#else
		if( skill_id != PA_SACRIFICE && skill_id != CR_GRANDCROSS && skill_id != NPC_GRANDDARKNESS
			&& skill_id != PA_SHIELDCHAIN && skill_id != KO_HAPPOKUNAI && !is_attack_critical(wd, src, target, skill_id, skill_lv, false) )
#endif
		{ //Elemental/Racial adjustments
			if( sd && (sd->right_weapon.def_ratio_atk_ele & (1<<tstatus->def_ele) || sd->right_weapon.def_ratio_atk_ele & (1<<ELE_ALL) ||
				sd->right_weapon.def_ratio_atk_race & (1<<tstatus->race) || sd->right_weapon.def_ratio_atk_race & (1<<RC_ALL) ||
				sd->right_weapon.def_ratio_atk_class & (1<<tstatus->class_) || sd->right_weapon.def_ratio_atk_class & (1<<CLASS_ALL))
			)
				if (weapon_position == EQI_HAND_R)
					return 1;

			if(sd && (sd->left_weapon.def_ratio_atk_ele & (1<<tstatus->def_ele) || sd->left_weapon.def_ratio_atk_ele & (1<<ELE_ALL) ||
				sd->left_weapon.def_ratio_atk_race & (1<<tstatus->race) || sd->left_weapon.def_ratio_atk_race & (1<<RC_ALL) ||
				sd->left_weapon.def_ratio_atk_class & (1<<tstatus->class_) || sd->left_weapon.def_ratio_atk_class & (1<<CLASS_ALL))
			)
			{ //Pass effect onto right hand if configured so. [Skotlex]
				if (battle_config.left_cardfix_to_right && is_attack_right_handed(src, skill_id)){
					if (weapon_position == EQI_HAND_R)
						return 1;
				}
				else if (weapon_position == EQI_HAND_L)
					return 1;
			}
		}
	}
	return 0;
}

static bool battle_skill_get_damage_properties(uint16 skill_id, int is_splash)
{
	int nk = skill_get_nk(skill_id);
	if( !skill_id && is_splash ) //If flag, this is splash damage from Baphomet Card and it always hits.
		nk |= NK_NO_CARDFIX_ATK|NK_IGNORE_FLEE;
	return nk;
}

/*=============================
 * Checks if attack is hitting
 *-----------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static bool is_attack_hitting(struct Damage wd, struct block_list *src, struct block_list *target, int skill_id, int skill_lv, bool first_call)
{
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);
	struct status_change *sc = status_get_sc(src);
	struct status_change *tsc = status_get_sc(target);
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	int nk = battle_skill_get_damage_properties(skill_id, wd.miscflag);
	short flee, hitrate;

	if (!first_call)
		return (wd.dmg_lv != ATK_FLEE);
	if (is_attack_critical(wd, src, target, skill_id, skill_lv, false))
		return true;
	else if(sd && sd->bonus.perfect_hit > 0 && rnd()%100 < sd->bonus.perfect_hit)
		return true;
	else if (sc && sc->data[SC_FUSION])
		return true;
	else if (skill_id == AS_SPLASHER && !wd.miscflag)
		return true;
	else if (skill_id == CR_SHIELDBOOMERANG && sc && sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_CRUSADER )
		return true;
	else if (tsc && tsc->opt1 && tsc->opt1 != OPT1_STONEWAIT && tsc->opt1 != OPT1_BURNING)
		return true;
	else if (nk&NK_IGNORE_FLEE)
		return true;

	if( sc && (sc->data[SC_NEUTRALBARRIER] || sc->data[SC_NEUTRALBARRIER_MASTER]) && (wd.flag&(BF_LONG|BF_WEAPON)) == (BF_LONG|BF_WEAPON) )
		return false;

	flee = tstatus->flee;
#ifdef RENEWAL
	hitrate = 0; //Default hitrate
#else
	hitrate = 80; //Default hitrate
#endif

	if(battle_config.agi_penalty_type && battle_config.agi_penalty_target&target->type) {
		unsigned char attacker_count = unit_counttargeted(target); //256 max targets should be a sane max

		if(attacker_count >= battle_config.agi_penalty_count) {
			if (battle_config.agi_penalty_type == 1)
				flee = (flee * (100 - (attacker_count - (battle_config.agi_penalty_count - 1)) * battle_config.agi_penalty_num)) / 100;
			else //assume type 2: absolute reduction
				flee -= (attacker_count - (battle_config.agi_penalty_count - 1)) * battle_config.agi_penalty_num;
			if(flee < 1)
				flee = 1;
		}
	}

	hitrate += sstatus->hit - flee;

	//Fogwall's hit penalty is only for normal ranged attacks.
	if ((wd.flag&(BF_LONG|BF_MAGIC)) == BF_LONG && !skill_id && tsc && tsc->data[SC_FOGWALL])
		hitrate -= 50;

	if(sd && is_skill_using_arrow(src, skill_id))
		hitrate += sd->bonus.arrow_hit;

#ifdef RENEWAL
	if (sd) //in Renewal hit bonus from Vultures Eye is not anymore shown in status window
		hitrate += pc_checkskill(sd,AC_VULTURE);
#endif

	if(skill_id) {
		switch(skill_id) { //Hit skill modifiers
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
				if(sd && pc_checkskill(sd,AS_SONICACCEL) > 0)
					hitrate += hitrate * 50 / 100;
				break;
			case MC_CARTREVOLUTION:
			case GN_CART_TORNADO:
			case GN_CARTCANNON:
				if (sd && pc_checkskill(sd, GN_REMODELING_CART))
					hitrate += pc_checkskill(sd, GN_REMODELING_CART) * 4;
				break;
			case LG_BANISHINGPOINT:
				hitrate += 3 * skill_lv;
				break;
			case GC_VENOMPRESSURE:
				hitrate += 10 + 4 * skill_lv;
				break;
			case SC_FATALMENACE:
				hitrate -= 35 - 5 * skill_lv;
				break;
			case RL_SLUGSHOT:
				{
					int8 dist = distance_bl(src, target);
					if (dist > 3) {
						// Reduce n hitrate for each cell after initial 3 cells. Different each level
						// -10:-9:-8:-7:-6
						dist -= 3;
						hitrate -= ((11 - skill_lv) * dist);
					}
				}
				break;
		}
	} else if (sd && wd.type&DMG_MULTI_HIT && wd.div_ == 2) // +1 hit per level of Double Attack on a successful double attack (making sure other multi attack skills do not trigger this) [helvetica]
		hitrate += pc_checkskill(sd,TF_DOUBLE);

	if (sd) {
		int skill = 0;

#ifdef RENEWAL
		// Weaponry Research hidden bonus
		if ((skill = pc_checkskill(sd,BS_WEAPONRESEARCH)) > 0)
			hitrate += hitrate * ( 2 * skill ) / 100;
#endif

		if( (sd->status.weapon == W_1HSWORD || sd->status.weapon == W_DAGGER) &&
			(skill = pc_checkskill(sd, GN_TRAINING_SWORD))>0 )
			hitrate += 3 * skill;
	}

	if (sc) {
		if (sc->data[SC_MTF_ASPD])
			hitrate += sc->data[SC_MTF_ASPD]->val2;
		if (sc->data[SC_MTF_ASPD2])
			hitrate += sc->data[SC_MTF_ASPD2]->val2;
	}

	hitrate = cap_value(hitrate, battle_config.min_hitrate, battle_config.max_hitrate);
	return (rnd()%100 < hitrate);
}

/*==========================================
 * If attack ignores def.
 *------------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static bool attack_ignores_def(struct Damage wd, struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv, short weapon_position)
{
	struct status_data *tstatus = status_get_status_data(target);
	struct status_change *sc = status_get_sc(src);
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	int nk = battle_skill_get_damage_properties(skill_id, wd.miscflag);

#ifndef RENEWAL
	if (is_attack_critical(wd, src, target, skill_id, skill_lv, false))
		return true;
	else
#endif
	if (sc && sc->data[SC_FUSION])
		return true;
#ifdef RENEWAL
	else if (skill_id != CR_GRANDCROSS && skill_id != NPC_GRANDDARKNESS && skill_id != ASC_BREAKER) // Renewal: Soul Breaker no longer gains ignore DEF from weapon [helvetica]
#else
	else if (skill_id != CR_GRANDCROSS && skill_id != NPC_GRANDDARKNESS)
#endif
	{	//Ignore Defense?
		if (sd && (sd->right_weapon.ignore_def_ele & (1<<tstatus->def_ele) || sd->right_weapon.ignore_def_ele & (1<<ELE_ALL) ||
			sd->right_weapon.ignore_def_race & (1<<tstatus->race) || sd->right_weapon.ignore_def_race & (1<<RC_ALL) ||
			sd->right_weapon.ignore_def_class & (1<<tstatus->class_) || sd->right_weapon.ignore_def_class & (1<<CLASS_ALL)))
			if (weapon_position == EQI_HAND_R)
				return true;

		if (sd && (sd->left_weapon.ignore_def_ele & (1<<tstatus->def_ele) || sd->left_weapon.ignore_def_ele & (1<<ELE_ALL) ||
			sd->left_weapon.ignore_def_race & (1<<tstatus->race) || sd->left_weapon.ignore_def_race & (1<<RC_ALL) ||
			sd->left_weapon.ignore_def_class & (1<<tstatus->class_) || sd->left_weapon.ignore_def_class & (1<<CLASS_ALL)))
		{
			if(battle_config.left_cardfix_to_right && is_attack_right_handed(src, skill_id)) {//Move effect to right hand. [Skotlex]
				if (weapon_position == EQI_HAND_R)
					return true;
			} else if (weapon_position == EQI_HAND_L)
				return true;
		}
	}

	return (nk&NK_IGNORE_DEF);
}

/*================================================
 * Should skill attack consider VVS and masteries?
 *------------------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static bool battle_skill_stacks_masteries_vvs(uint16 skill_id)
{
	if (
#ifndef RENEWAL
		skill_id == PA_SHIELDCHAIN || skill_id == CR_SHIELDBOOMERANG ||
#endif
		skill_id == RK_DRAGONBREATH || skill_id == RK_DRAGONBREATH_WATER || skill_id == NC_SELFDESTRUCTION ||
		skill_id == LG_SHIELDPRESS || skill_id == LG_EARTHDRIVE)
			return false;

	return true;
}

#ifdef RENEWAL
/*========================================
 * Calculate equipment ATK for renewal ATK
 *----------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static int battle_calc_equip_attack(struct block_list *src, int skill_id)
{
	if(src != NULL) {
		int eatk = 0;
		struct status_data *status = status_get_status_data(src);
		struct map_session_data *sd = BL_CAST(BL_PC, src);

		if (sd) // add arrow atk if using an applicable skill
			eatk += (is_skill_using_arrow(src, skill_id) ? sd->bonus.arrow_atk : 0);

		return eatk + status->eatk;
	}
	return 0; // shouldn't happen but just in case
}
#endif

/*========================================
 * Returns the element type of attack
 *----------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static int battle_get_weapon_element(struct Damage wd, struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv, short weapon_position, bool calc_for_damage_only)
{
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	struct status_change *sc = status_get_sc(src);
	struct status_data *sstatus = status_get_status_data(src);
	int element = skill_get_ele(skill_id, skill_lv);

	//Take weapon's element
	if( !skill_id || element == -1 ) {
		if (weapon_position == EQI_HAND_R)
			element = sstatus->rhw.ele;
		else
			element = sstatus->lhw.ele;
		if(is_skill_using_arrow(src, skill_id) && sd && sd->bonus.arrow_ele && weapon_position == EQI_HAND_R)
			element = sd->bonus.arrow_ele;
		if(sd && sd->spiritcharm_type != CHARM_TYPE_NONE && sd->spiritcharm >= MAX_SPIRITCHARM)
			element = sd->spiritcharm_type; // Summoning 10 spiritcharm will endow your weapon
		// on official endows override all other elements [helvetica]
		if(sc && sc->data[SC_ENCHANTARMS]) // Check for endows
			element = sc->data[SC_ENCHANTARMS]->val2;
	} else if( element == -2 ) //Use enchantment's element
		element = status_get_attack_sc_element(src,sc);
	else if( element == -3 ) //Use random element
		element = rnd()%ELE_ALL;

	switch( skill_id ) {
		case GS_GROUNDDRIFT:
			element = wd.miscflag; //element comes in flag.
			break;
		case LK_SPIRALPIERCE:
			if (!sd)
				element = ELE_NEUTRAL; //forced neutral for monsters
			break;
		case LG_HESPERUSLIT:
			if (sc && sc->data[SC_BANDING] && sc->data[SC_BANDING]->val2 > 4)
				element = ELE_HOLY;
			break;
		case RL_H_MINE:
			if (sd && sd->flicker) //Force RL_H_MINE deals fire damage if activated by RL_FLICKER
				element = ELE_FIRE;
			break;
	}

	if (sc && sc->data[SC_GOLDENE_FERSE] && ((!skill_id && (rnd() % 100 < sc->data[SC_GOLDENE_FERSE]->val4)) || skill_id == MH_STAHL_HORN))
		element = ELE_HOLY;

// calc_flag means the element should be calculated for damage only
	if (calc_for_damage_only)
		return element;

#ifdef RENEWAL
	if (skill_id == CR_SHIELDBOOMERANG)
		element = ELE_NEUTRAL;
#endif

	return element;
}

/*========================================
 * Do element damage modifier calculation
 *----------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static struct Damage battle_calc_element_damage(struct Damage wd, struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv)
{
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	struct status_change *sc = status_get_sc(src);
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);
	int element = skill_get_ele(skill_id, skill_lv);
	int left_element = battle_get_weapon_element(wd, src, target, skill_id, skill_lv, EQI_HAND_L, true);
	int right_element = battle_get_weapon_element(wd, src, target, skill_id, skill_lv, EQI_HAND_R, true);
	int nk = battle_skill_get_damage_properties(skill_id, wd.miscflag);

	//Elemental attribute fix
	if(!(nk&NK_NO_ELEFIX)) {
		//Non-pc physical melee attacks (mob, pet, homun) are "non elemental", they deal 100% to all target elements
		//However the "non elemental" attacks still get reduced by "Neutral resistance"
		//Also non-pc units have only a defending element, but can inflict elemental attacks using skills [exneval]
		if(battle_config.attack_attr_none&src->type)
			if(((!skill_id && !right_element) || (skill_id && (element == -1 || !right_element))) &&
				(wd.flag&(BF_SHORT|BF_WEAPON)) == (BF_SHORT|BF_WEAPON))
				return wd;
		if(wd.damage > 0) {
			//Forced to its element
			wd.damage = battle_attr_fix(src, target, wd.damage, right_element, tstatus->def_ele, tstatus->ele_lv);

			switch( skill_id ) {
				case MC_CARTREVOLUTION:
				case SR_GATEOFHELL:
				case KO_BAKURETSU:
					//Forced to neutral element
					wd.damage = battle_attr_fix(src, target, wd.damage, ELE_NEUTRAL, tstatus->def_ele, tstatus->ele_lv);
					break;
				case GN_CARTCANNON:
				case KO_HAPPOKUNAI:
					//Forced to ammo's element
					wd.damage = battle_attr_fix(src, target, wd.damage, (sd && sd->bonus.arrow_ele) ? sd->bonus.arrow_ele : ELE_NEUTRAL, tstatus->def_ele, tstatus->ele_lv);
					break;
			}
		}
		if (is_attack_left_handed(src, skill_id) && wd.damage2 > 0)
			wd.damage2 = battle_attr_fix(src, target, wd.damage2, left_element ,tstatus->def_ele, tstatus->ele_lv);
		if (sc && sc->data[SC_WATK_ELEMENT] && (wd.damage || wd.damage2)) {
			// Descriptions indicate this means adding a percent of a normal attack in another element. [Skotlex]
			int64 damage = battle_calc_base_damage(sstatus, &sstatus->rhw, sc, tstatus->size, sd, (is_skill_using_arrow(src, skill_id)?2:0)) * sc->data[SC_WATK_ELEMENT]->val2 / 100;

			wd.damage += battle_attr_fix(src, target, damage, sc->data[SC_WATK_ELEMENT]->val1, tstatus->def_ele, tstatus->ele_lv);
			if (is_attack_left_handed(src, skill_id)) {
				damage = battle_calc_base_damage(sstatus, &sstatus->lhw, sc, tstatus->size, sd, (is_skill_using_arrow(src, skill_id)?2:0)) * sc->data[SC_WATK_ELEMENT]->val2 / 100;
				wd.damage2 += battle_attr_fix(src, target, damage, sc->data[SC_WATK_ELEMENT]->val1, tstatus->def_ele, tstatus->ele_lv);
			}
		}
	}
	return wd;
}

#define ATK_RATE(damage, damage2, a) do { int64 rate_ = (a); (damage) = (damage) * rate_ / 100; if(is_attack_left_handed(src, skill_id)) (damage2) = (damage2) * rate_ / 100; } while(0);
#define ATK_RATE2(damage, damage2, a , b) do { int64 rate_ = (a), rate2_ = (b); (damage) = (damage) *rate_ / 100; if(is_attack_left_handed(src, skill_id)) (damage2) = (damage2) * rate2_ / 100; } while(0);
#define ATK_RATER(damage, a) { (damage) = (damage) * (a) / 100; }
#define ATK_RATEL(damage2, a) { (damage2) = (damage2) * (a) / 100; }
//Adds dmg%. 100 = +100% (double) damage. 10 = +10% damage
#define ATK_ADDRATE(damage, damage2, a) do { int64 rate_ = (a); (damage) += (damage) * rate_ / 100; if(is_attack_left_handed(src, skill_id)) (damage2) += (damage2) *rate_ / 100; } while(0);
#define ATK_ADDRATE2(damage, damage2, a , b) do { int64 rate_ = (a), rate2_ = (b); (damage) += (damage) * rate_ / 100; if(is_attack_left_handed(src, skill_id)) (damage2) += (damage2) * rate2_ / 100; } while(0);
//Adds an absolute value to damage. 100 = +100 damage
#define ATK_ADD(damage, damage2, a) do { int64 rate_ = (a); (damage) += rate_; if(is_attack_left_handed(src, skill_id)) (damage2) += rate_; } while(0);
#define ATK_ADD2(damage, damage2, a , b) do { int64 rate_ = (a), rate2_ = (b); (damage) += rate_; if(is_attack_left_handed(src, skill_id)) (damage2) += rate2_; } while(0);

#ifdef RENEWAL
	#define RE_ALLATK_ADD(wd, a) do { int64 a_ = (a); ATK_ADD((wd).statusAtk, (wd).statusAtk2, a_); ATK_ADD((wd).weaponAtk, (wd).weaponAtk2, a_); ATK_ADD((wd).equipAtk, (wd).equipAtk2, a_); ATK_ADD((wd).masteryAtk, (wd).masteryAtk2, a_); } while(0);
	#define RE_ALLATK_RATE(wd, a) do { int64 a_ = (a); ATK_RATE((wd).statusAtk, (wd).statusAtk2, a_); ATK_RATE((wd).weaponAtk, (wd).weaponAtk2, a_); ATK_RATE((wd).equipAtk, (wd).equipAtk2, a_); ATK_RATE((wd).masteryAtk, (wd).masteryAtk2, a_); } while(0);
	#define RE_ALLATK_ADDRATE(wd, a) do { int64 a_ = (a); ATK_ADDRATE((wd).statusAtk, (wd).statusAtk2, a_); ATK_ADDRATE((wd).weaponAtk, (wd).weaponAtk2, a_); ATK_ADDRATE((wd).equipAtk, (wd).equipAtk2, a_); ATK_ADDRATE((wd).masteryAtk, (wd).masteryAtk2, a_); } while(0);
#else
	#define RE_ALLATK_ADD(wd, a) {;}
	#define RE_ALLATK_RATE(wd, a) {;}
	#define RE_ALLATK_ADDRATE(wd, a) {;}
#endif

/*==================================
 * Calculate weapon mastery damages
 *----------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static struct Damage battle_calc_attack_masteries(struct Damage wd, struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv)
{
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	struct status_change *sc = status_get_sc(src);
	struct status_data *sstatus = status_get_status_data(src);
	int t_class = status_get_class(target);

	if (sd && battle_skill_stacks_masteries_vvs(skill_id) &&
		skill_id != MO_INVESTIGATE &&
		skill_id != MO_EXTREMITYFIST &&
		skill_id != CR_GRANDCROSS)
	{	//Add mastery damage
		uint16 skill;

		wd.damage = battle_addmastery(sd,target,wd.damage,0);
#ifdef RENEWAL
		wd.masteryAtk = battle_addmastery(sd,target,wd.weaponAtk,0);
#endif
		if (is_attack_left_handed(src, skill_id)) {
			wd.damage2 = battle_addmastery(sd,target,wd.damage2,1);
#ifdef RENEWAL
			wd.masteryAtk2 = battle_addmastery(sd,target,wd.weaponAtk2,1);
#endif
		}

#ifdef RENEWAL
		//General skill masteries
		if(skill_id == TF_POISON) //Additional ATK from Envenom is treated as mastery type damage [helvetica]
			ATK_ADD(wd.masteryAtk, wd.masteryAtk2, 15 * skill_lv);
		if (skill_id != MC_CARTREVOLUTION && pc_checkskill(sd, BS_HILTBINDING) > 0)
			ATK_ADD(wd.masteryAtk, wd.masteryAtk2, 4);
		if (skill_id != CR_SHIELDBOOMERANG)
			ATK_ADD2(wd.masteryAtk, wd.masteryAtk2, ((wd.div_ < 1) ? 1 : wd.div_) * sd->right_weapon.star, ((wd.div_ < 1) ? 1 : wd.div_) * sd->left_weapon.star);
		if (skill_id == MO_FINGEROFFENSIVE) {
			ATK_ADD(wd.masteryAtk, wd.masteryAtk2, ((wd.div_ < 1) ? 1 : wd.div_) * sd->spiritball_old * 3);
		} else
			ATK_ADD(wd.masteryAtk, wd.masteryAtk2, ((wd.div_ < 1) ? 1 : wd.div_) * sd->spiritball * 3);
#endif

		if (skill_id == NJ_SYURIKEN && (skill = pc_checkskill(sd,NJ_TOBIDOUGU)) > 0) {
			ATK_ADD(wd.damage, wd.damage2, 3 * skill);
#ifdef RENEWAL
			ATK_ADD(wd.masteryAtk, wd.masteryAtk2, 3 * skill);
#endif
		}

		if (sc) { // Status change considered as masteries
			uint8 i;

#ifdef RENEWAL
			if (sc->data[SC_NIBELUNGEN]) // With renewal, the level 4 weapon limitation has been removed
				ATK_ADD(wd.masteryAtk, wd.masteryAtk2, sc->data[SC_NIBELUNGEN]->val2);
#endif

			if (sc->data[SC_MIRACLE])
				i = 2; //Star anger
			else
				ARR_FIND(0, MAX_PC_FEELHATE, i, t_class == sd->hate_mob[i]);

			if (i < MAX_PC_FEELHATE && (skill=pc_checkskill(sd,sg_info[i].anger_id))) {
				int skillratio = sd->status.base_level + sstatus->dex + sstatus->luk;

				if (i == 2)
					skillratio += sstatus->str; //Star Anger
				if (skill < 4)
					skillratio /= 12 - 3 * skill;
				ATK_ADDRATE(wd.damage, wd.damage2, skillratio);
#ifdef RENEWAL
				ATK_ADDRATE(wd.masteryAtk, wd.masteryAtk2, skillratio);
#endif
			}

			if(sc->data[SC_CAMOUFLAGE]) {
				ATK_ADD(wd.damage, wd.damage2, 30 * min(10, sc->data[SC_CAMOUFLAGE]->val3));
#ifdef RENEWAL
				ATK_ADD(wd.masteryAtk, wd.masteryAtk2, 30 * min(10, sc->data[SC_CAMOUFLAGE]->val3));
#endif
			}
			if(sc->data[SC_GN_CARTBOOST]) {
				ATK_ADD(wd.damage, wd.damage2, 10 * sc->data[SC_GN_CARTBOOST]->val1);
#ifdef RENEWAL
				ATK_ADD(wd.masteryAtk, wd.masteryAtk2, 10 * sc->data[SC_GN_CARTBOOST]->val1);
#endif
			}
		}
	}

	return wd;
}

#ifdef RENEWAL
/*=========================================
 * Calculate the various Renewal ATK parts
 *-----------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
struct Damage battle_calc_damage_parts(struct Damage wd, struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv)
{
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);
	struct map_session_data *sd = BL_CAST(BL_PC, src);

	int right_element = battle_get_weapon_element(wd, src, target, skill_id, skill_lv, EQI_HAND_R, false);
	int left_element = battle_get_weapon_element(wd, src, target, skill_id, skill_lv, EQI_HAND_L, false);

	wd.statusAtk += battle_calc_status_attack(sstatus, EQI_HAND_R);
	wd.statusAtk2 += battle_calc_status_attack(sstatus, EQI_HAND_L);

	if (skill_id || (sd && sd->sc.data[SC_SEVENWIND])) { // Mild Wind applies element to status ATK as well as weapon ATK [helvetica]
		wd.statusAtk = battle_attr_fix(src, target, wd.statusAtk, right_element, tstatus->def_ele, tstatus->ele_lv);
		wd.statusAtk2 = battle_attr_fix(src, target, wd.statusAtk, left_element, tstatus->def_ele, tstatus->ele_lv);
	} else { // status atk is considered neutral on normal attacks [helvetica]
		wd.statusAtk = battle_attr_fix(src, target, wd.statusAtk, ELE_NEUTRAL, tstatus->def_ele, tstatus->ele_lv);
		wd.statusAtk2 = battle_attr_fix(src, target, wd.statusAtk, ELE_NEUTRAL, tstatus->def_ele, tstatus->ele_lv);
	}

	wd.weaponAtk += battle_calc_base_weapon_attack(src, tstatus, &sstatus->rhw, sd);
	wd.weaponAtk = battle_attr_fix(src, target, wd.weaponAtk, right_element, tstatus->def_ele, tstatus->ele_lv);

	wd.weaponAtk2 += battle_calc_base_weapon_attack(src, tstatus, &sstatus->lhw, sd);
	wd.weaponAtk2 = battle_attr_fix(src, target, wd.weaponAtk2, left_element, tstatus->def_ele, tstatus->ele_lv);

	wd.equipAtk += battle_calc_equip_attack(src, skill_id);
	wd.equipAtk = battle_attr_fix(src, target, wd.equipAtk, right_element, tstatus->def_ele, tstatus->ele_lv);

	wd.equipAtk2 += battle_calc_equip_attack(src, skill_id);
	wd.equipAtk2 = battle_attr_fix(src, target, wd.equipAtk2, left_element, tstatus->def_ele, tstatus->ele_lv);

	//Mastery ATK is a special kind of ATK that has no elemental properties
	//Because masteries are not elemental, they are unaffected by Ghost armors or Raydric Card
	wd = battle_calc_attack_masteries(wd, src, target, skill_id, skill_lv);

	wd.damage = 0;
	wd.damage2 = 0;

	return wd;
}
#endif

/*==========================================================
 * Calculate basic ATK that goes into the skill ATK formula
 *----------------------------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
struct Damage battle_calc_skill_base_damage(struct Damage wd, struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv)
{
	struct status_change *sc = status_get_sc(src);
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	uint16 i;
	int nk = battle_skill_get_damage_properties(skill_id, wd.miscflag);

	switch (skill_id) {	//Calc base damage according to skill
		case PA_SACRIFICE:
			wd.damage = sstatus->max_hp* 9/100;
			wd.damage2 = 0;
#ifdef RENEWAL
			wd.weaponAtk = wd.damage;
			wd.weaponAtk2 = wd.damage2;
#endif
			break;
#ifdef RENEWAL
		case LK_SPIRALPIERCE:
		case ML_SPIRALPIERCE:
			if (sd) {
				short index = sd->equip_index[EQI_HAND_R];

				if (index >= 0 &&
					sd->inventory_data[index] &&
					sd->inventory_data[index]->type == IT_WEAPON)
					wd.equipAtk += sd->inventory_data[index]->weight/20; // weight from spear is treated as equipment ATK on official [helvetica]

				wd = battle_calc_damage_parts(wd, src, target, skill_id, skill_lv);
				wd.masteryAtk = 0; // weapon mastery is ignored for spiral
			} else {
				wd.damage = battle_calc_base_damage(sstatus, &sstatus->rhw, sc, tstatus->size, sd, 0); //Monsters have no weight and use ATK instead
			}

			switch (tstatus->size) { //Size-fix. Is this modified by weapon perfection?
				case SZ_SMALL: //Small: 125%
					ATK_RATE(wd.damage, wd.damage2, 125);
					RE_ALLATK_RATE(wd, 125);
					break;
				//case SZ_MEDIUM: //Medium: 100%
				case SZ_BIG: //Large: 75%
					ATK_RATE(wd.damage, wd.damage2, 75);
					RE_ALLATK_RATE(wd, 75);
					break;
			}
#else
		case NJ_ISSEN:
			wd.damage = 40 * sstatus->str + sstatus->hp * 8 * skill_lv / 100;
			wd.damage2 = 0;
			break;
		case LK_SPIRALPIERCE:
		case ML_SPIRALPIERCE:
			if (sd) {
				short index = sd->equip_index[EQI_HAND_R];

				if (index >= 0 &&
					sd->inventory_data[index] &&
					sd->inventory_data[index]->type == IT_WEAPON)
					wd.damage = sd->inventory_data[index]->weight*8/100; //80% of weight

				ATK_ADDRATE(wd.damage, wd.damage2, 50*skill_lv); //Skill modifier applies to weight only.
			} else {
				wd.damage = battle_calc_base_damage(sstatus, &sstatus->rhw, sc, tstatus->size, sd, 0); //Monsters have no weight and use ATK instead
			}

			i = sstatus->str/10;
			i*=i;
			ATK_ADD(wd.damage, wd.damage2, i); //Add str bonus.
			switch (tstatus->size) { //Size-fix. Is this modified by weapon perfection?
				case SZ_SMALL: //Small: 125%
					ATK_RATE(wd.damage, wd.damage2, 125);
					break;
				//case SZ_MEDIUM: //Medium: 100%
				case SZ_BIG: //Large: 75%
					ATK_RATE(wd.damage, wd.damage2, 75);
					break;
			}
#endif
			break;
		case CR_SHIELDBOOMERANG:
		case PA_SHIELDCHAIN:
			wd.damage = sstatus->batk;
			if (sd) {
				short index = sd->equip_index[EQI_HAND_L];

				if (index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_ARMOR) {
					ATK_ADD(wd.damage, wd.damage2, sd->inventory_data[index]->weight / 10);
#ifdef RENEWAL
					ATK_ADD(wd.weaponAtk, wd.weaponAtk2, sd->inventory_data[index]->weight / 10);
#endif
				}
			} else
				ATK_ADD(wd.damage, wd.damage2, sstatus->rhw.atk2); //Else use Atk2
			break;
		case RK_DRAGONBREATH:
		case RK_DRAGONBREATH_WATER:
			{
				int damagevalue = (sstatus->hp / 50 + status_get_max_sp(src) / 4) * skill_lv;

				if(status_get_lv(src) > 100)
					damagevalue = damagevalue * status_get_lv(src) / 150;
				if(sd)
					damagevalue = damagevalue * (100 + 5 * (pc_checkskill(sd,RK_DRAGONTRAINING) - 1)) / 100;
				ATK_ADD(wd.damage, wd.damage2, damagevalue);
#ifdef RENEWAL
				ATK_ADD(wd.weaponAtk, wd.weaponAtk2, damagevalue);
#endif
				wd.flag |= BF_LONG;
			}
			break;
		case NC_SELFDESTRUCTION: {
				int damagevalue = (skill_lv + 1) * ((sd ? pc_checkskill(sd,NC_MAINFRAME) : 0) + 8) * (status_get_sp(src) + sstatus->vit);

				if(status_get_lv(src) > 100)
					damagevalue = damagevalue * status_get_lv(src) / 100;
				damagevalue = damagevalue + sstatus->hp;
				ATK_ADD(wd.damage, wd.damage2, damagevalue);
#ifdef RENEWAL
				ATK_ADD(wd.weaponAtk, wd.weaponAtk2, damagevalue);
#endif
			}
			break;
		case KO_HAPPOKUNAI:
			if(sd) {
				short index = sd->equip_index[EQI_AMMO];
				int damagevalue = 3 * (
#ifdef RENEWAL
					2 *
#endif
					sstatus->batk + sstatus->rhw.atk + (index >= 0 && sd->inventory_data[index] ?
						sd->inventory_data[index]->atk : 0)) * (skill_lv + 5) / 5;
				ATK_ADD(wd.damage, wd.damage2, damagevalue);
#ifdef RENEWAL
				ATK_ADD(wd.weaponAtk, wd.weaponAtk2, damagevalue);
#endif
			} else
				ATK_ADD(wd.damage, wd.damage2, 5000);
			break;
		case HFLI_SBR44:	//[orn]
			if(src->type == BL_HOM)
				wd.damage = ((TBL_HOM*)src)->homunculus.intimacy ;
			break;

		default:
#ifdef RENEWAL
			if (sd)
				wd = battle_calc_damage_parts(wd, src, target, skill_id, skill_lv);
			else {
				i = (is_attack_critical(wd, src, target, skill_id, skill_lv, false)?1:0)|
					(!skill_id && sc && sc->data[SC_CHANGE]?4:0);

				wd.damage = battle_calc_base_damage(sstatus, &sstatus->rhw, sc, tstatus->size, sd, i);
				if (is_attack_left_handed(src, skill_id))
					wd.damage2 = battle_calc_base_damage(sstatus, &sstatus->lhw, sc, tstatus->size, sd, i);
			}
#else
			i = (is_attack_critical(wd, src, target, skill_id, skill_lv, false)?1:0)|
				(is_skill_using_arrow(src, skill_id)?2:0)|
				(skill_id == HW_MAGICCRASHER?4:0)|
				(!skill_id && sc && sc->data[SC_CHANGE]?4:0)|
				(skill_id == MO_EXTREMITYFIST?8:0)|
				(sc && sc->data[SC_WEAPONPERFECTION]?8:0);
			if (is_skill_using_arrow(src, skill_id) && sd) {
				switch(sd->status.weapon) {
					case W_BOW:
					case W_REVOLVER:
					case W_GATLING:
					case W_SHOTGUN:
					case W_GRENADE:
						break;
					default:
						i |= 16; // for ex. shuriken must not be influenced by DEX
						break;
				}
			}
			wd.damage = battle_calc_base_damage(sstatus, &sstatus->rhw, sc, tstatus->size, sd, i);
			if (is_attack_left_handed(src, skill_id))
				wd.damage2 = battle_calc_base_damage(sstatus, &sstatus->lhw, sc, tstatus->size, sd, i);
#endif
			if (nk&NK_SPLASHSPLIT){ // Divide ATK among targets
				if(wd.miscflag > 0) {
					wd.damage /= wd.miscflag;
#ifdef RENEWAL
					wd.statusAtk /= wd.miscflag;
					wd.weaponAtk /= wd.miscflag;
					wd.equipAtk /= wd.miscflag;
					wd.masteryAtk /= wd.miscflag;
#endif
				} else
					ShowError("0 enemies targeted by %d:%s, divide per 0 avoided!\n", skill_id, skill_get_name(skill_id));
			}

			//Add any bonuses that modify the base atk (pre-skills)
			if(sd) {
				int skill;

				if (sd->bonus.atk_rate) {
					ATK_ADDRATE(wd.damage, wd.damage2, sd->bonus.atk_rate);
					RE_ALLATK_ADDRATE(wd, sd->bonus.atk_rate);
				}
#ifndef RENEWAL
				if(sd->bonus.crit_atk_rate && is_attack_critical(wd, src, target, skill_id, skill_lv, false)) { // add +crit damage bonuses here in pre-renewal mode [helvetica]
					ATK_ADDRATE(wd.damage, wd.damage2, sd->bonus.crit_atk_rate);
				}
#endif
				if(sd->status.party_id && (skill=pc_checkskill(sd,TK_POWER)) > 0) {
					if( (i = party_foreachsamemap(party_sub_count, sd, 0)) > 1 ) { // exclude the player himself [Inkfish]
						ATK_ADDRATE(wd.damage, wd.damage2, 2*skill*i);
						RE_ALLATK_ADDRATE(wd, 2*skill*i);
					}
				}
			}
			break;
	} //End switch(skill_id)
	return wd;
}

//For quick div adjustment.
#define DAMAGE_DIV_FIX(dmg, div) { if ((div) < 0) { (div) *= -1; (dmg) /= (div); } (dmg) *= (div); }
#define DAMAGE_DIV_FIX2(dmg, div) { if ((div) > 1) (dmg) *= div; }
#define DAMAGE_DIV_FIX_RENEWAL(wd, div) do { int div_ = (div); DAMAGE_DIV_FIX2((wd).statusAtk, div_); DAMAGE_DIV_FIX2((wd).weaponAtk, div_); DAMAGE_DIV_FIX2((wd).equipAtk, div_); DAMAGE_DIV_FIX2((wd).masteryAtk, div_); } while(0);
/*================================================= [Playtester]
 * Applies DAMAGE_DIV_FIX and checks for min damage
 * @param d: Damage struct to apply DAMAGE_DIV_FIX to
 * @return Modified damage struct
 *------------------------------------------------*/
static struct Damage battle_apply_div_fix(struct Damage d)
{
	if(d.damage) {
		DAMAGE_DIV_FIX(d.damage, d.div_);
		//Min damage
		if((battle_config.skill_min_damage&d.flag) && d.damage < d.div_)
			d.damage = d.div_;
	} else if (d.div_ < 0) {
		d.div_ *= -1;
	}

	return d;
}

/*=======================================
 * Check for and calculate multi attacks
 *---------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static struct Damage battle_calc_multi_attack(struct Damage wd, struct block_list *src,struct block_list *target, uint16 skill_id, uint16 skill_lv)
{
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	struct status_change *sc = status_get_sc(src);
	struct status_change *tsc = status_get_sc(target);
	struct status_data *tstatus = status_get_status_data(target);

	if( sd && !skill_id ) {	// if no skill_id passed, check for double attack [helvetica]
		short i;
		if( ( ( skill_lv = pc_checkskill(sd,TF_DOUBLE) ) > 0 && sd->weapontype1 == W_DAGGER )
			|| ( sd->bonus.double_rate > 0 && sd->weapontype1 != W_FIST ) //Will fail bare-handed
			|| ( sc && sc->data[SC_KAGEMUSYA] && sd->weapontype1 != W_FIST )) // Need confirmation
		{	//Success chance is not added, the higher one is used [Skotlex]
			if( rnd()%100 < ( 5*skill_lv > sd->bonus.double_rate ? 5*skill_lv : sc && sc->data[SC_KAGEMUSYA]?sc->data[SC_KAGEMUSYA]->val1*3:sd->bonus.double_rate ) ) {
				wd.div_ = skill_get_num(TF_DOUBLE,skill_lv?skill_lv:1);
				wd.type = DMG_MULTI_HIT;
			}
		}
		else if( ((sd->weapontype1 == W_REVOLVER && (skill_lv = pc_checkskill(sd,GS_CHAINACTION)) > 0) //Normal Chain Action effect
			|| (sd && sc->count && sc->data[SC_E_CHAIN] && (skill_lv = sc->data[SC_E_CHAIN]->val2) > 0)) //Chain Action of ETERNAL_CHAIN
			&& rnd()%100 < 5*skill_lv ) //Success rate
		{
			wd.div_ = skill_get_num(GS_CHAINACTION,skill_lv);
			wd.type = DMG_MULTI_HIT;
			sc_start(src,src,SC_QD_SHOT_READY,100,target->id,skill_get_time(RL_QD_SHOT,1));
		}
		else if(sc && sc->data[SC_FEARBREEZE] && sd->weapontype1==W_BOW
			&& (i = sd->equip_index[EQI_AMMO]) >= 0 && sd->inventory_data[i] && sd->status.inventory[i].amount > 1)
		{
			int chance = rnd()%100;
			switch(sc->data[SC_FEARBREEZE]->val1) {
				case 5: if( chance < 4) { wd.div_ = 5; break; } // 3 % chance to attack 5 times.
				case 4: if( chance < 7) { wd.div_ = 4; break; } // 6 % chance to attack 4 times.
				case 3: if( chance < 10) { wd.div_ = 3; break; } // 9 % chance to attack 3 times.
				case 2:
				case 1: if( chance < 13) { wd.div_ = 2; break; } // 12 % chance to attack 2 times.
			}
			wd.div_ = min(wd.div_,sd->status.inventory[i].amount);
			sc->data[SC_FEARBREEZE]->val4 = wd.div_-1;
			if (wd.div_ > 1)
				wd.type = DMG_MULTI_HIT;
		}
	}

	switch (skill_id) {
		case RA_AIMEDBOLT:
			if( tsc && (tsc->data[SC_BITE] || tsc->data[SC_ANKLE] || tsc->data[SC_ELECTRICSHOCKER]) )
				wd.div_ = tstatus->size + 2 + ( (rnd()%100 < 50-tstatus->size*10) ? 1 : 0 );
			break;
		case SC_JYUMONJIKIRI:
			if( tsc && tsc->data[SC_JYUMONJIKIRI] )
				wd.div_ = wd.div_ * -1;// needs more info
			break;
	}

	return wd;
}

/*======================================================
 * Calculate skill level ratios for weapon-based skills
 *------------------------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static int battle_calc_attack_skill_ratio(struct Damage wd, struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv)
{
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	struct map_session_data *tsd = BL_CAST(BL_PC, src);
	struct status_change *sc = status_get_sc(src);
	struct status_change *tsc = status_get_sc(target);
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);
	int skillratio = 100;
	int i;

	//Skill damage modifiers that stack linearly
	if(sc && skill_id != PA_SACRIFICE) {
		if(sc->data[SC_OVERTHRUST])
			skillratio += sc->data[SC_OVERTHRUST]->val3;
		if(sc->data[SC_MAXOVERTHRUST])
			skillratio += sc->data[SC_MAXOVERTHRUST]->val2;
		if(sc->data[SC_BERSERK])
#ifndef RENEWAL
			skillratio += 100;
#else
			skillratio += 200;
		if (sc && sc->data[SC_TRUESIGHT])
			skillratio += 2 * sc->data[SC_TRUESIGHT]->val1;
		if (sc->data[SC_CONCENTRATION])
			skillratio += sc->data[SC_CONCENTRATION]->val2;
#endif
		if (sc->data[SC_CRUSHSTRIKE] && (!skill_id || skill_id == KN_AUTOCOUNTER)) {
			if (sd) { //ATK [{Weapon Level * (Weapon Upgrade Level + 6) * 100} + (Weapon ATK) + (Weapon Weight)]%
				short index = sd->equip_index[EQI_HAND_R];

				if (index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_WEAPON)
					skillratio += -100 + sd->inventory_data[index]->weight / 10 + sstatus->rhw.atk +
						100 * sd->inventory_data[index]->wlv * (sd->status.inventory[index].refine + 6);
			}
			status_change_end(src,SC_CRUSHSTRIKE,INVALID_TIMER);
			skill_break_equip(src,src,EQP_WEAPON,2000,BCT_SELF);
		}
		//!TODO: Verify this placement & skills that affected by these effects [Cydh]
		if (sc->data[SC_HEAT_BARREL])
			skillratio += 200;
		if (sc->data[SC_P_ALTER])
			skillratio += sc->data[SC_P_ALTER]->val2;
	}

	switch(skill_id) {
		case SM_BASH:
		case MS_BASH:
			skillratio += 30 * skill_lv;
			break;
		case SM_MAGNUM:
		case MS_MAGNUM:
			if(wd.miscflag == 1)
				skillratio += 20 * skill_lv; //Inner 3x3 circle takes 100%+20%*level damage [Playtester]
			else
				skillratio += 10 * skill_lv; //Outer 5x5 circle takes 100%+10%*level damage [Playtester]
			break;
		case MC_MAMMONITE:
			skillratio += 50 * skill_lv;
			break;
		case HT_POWER:
			skillratio += -50 + 8 * sstatus->str;
			break;
		case AC_DOUBLE:
		case MA_DOUBLE:
			skillratio += 10 * (skill_lv - 1);
			break;
		case AC_SHOWER:
		case MA_SHOWER:
#ifdef RENEWAL
			skillratio += 50 + 10 * skill_lv;
#else
			skillratio += -25 + 5 * skill_lv;
#endif
			break;
		case AC_CHARGEARROW:
		case MA_CHARGEARROW:
			skillratio += 50;
			break;
#ifndef RENEWAL
		case HT_FREEZINGTRAP:
		case MA_FREEZINGTRAP:
			skillratio += -50 + 10 * skill_lv;
			break;
#endif
		case KN_PIERCE:
		case ML_PIERCE:
			skillratio += 10 * skill_lv;
			break;
		case MER_CRASH:
			skillratio += 10 * skill_lv;
			break;
		case KN_SPEARSTAB:
			skillratio += 15 * skill_lv;
			break;
		case KN_SPEARBOOMERANG:
			skillratio += 50 * skill_lv;
			break;
		case KN_BRANDISHSPEAR:
		case ML_BRANDISH: {
				int ratio = 100 + 20 * skill_lv;

				skillratio += -100 + ratio;
				if(skill_lv > 3 && wd.miscflag == 1)
					skillratio += ratio / 2;
				if(skill_lv > 6 && wd.miscflag == 1)
					skillratio += ratio / 4;
				if(skill_lv > 9 && wd.miscflag == 1)
					skillratio += ratio / 8;
				if(skill_lv > 6 && wd.miscflag == 2)
					skillratio += ratio / 2;
				if(skill_lv > 9 && wd.miscflag == 2)
					skillratio += ratio / 4;
				if(skill_lv > 9 && wd.miscflag == 3)
					skillratio += ratio / 2;
				break;
			}
		case KN_BOWLINGBASH:
		case MS_BOWLINGBASH:
			skillratio+= 40 * skill_lv;
			break;
		case AS_GRIMTOOTH:
			skillratio += 20 * skill_lv;
			break;
		case AS_POISONREACT:
			skillratio += 30 * skill_lv;
			break;
		case AS_SONICBLOW:
			skillratio += 300 + 40 * skill_lv;
			break;
		case TF_SPRINKLESAND:
			skillratio += 30;
			break;
		case MC_CARTREVOLUTION:
			skillratio += 50;
			if(sd && sd->cart_weight)
				skillratio += 100 * sd->cart_weight / sd->cart_weight_max; // +1% every 1% weight
			else if (!sd)
				skillratio += 100; //Max damage for non players.
			break;
		case NPC_PIERCINGATT:
			skillratio += -25; //75% base damage
			break;
		case NPC_COMBOATTACK:
			skillratio += 25 * skill_lv;
			break;
		case NPC_RANDOMATTACK:
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
			skillratio += 100 * (skill_lv - 1);
			break;
		case RG_BACKSTAP:
			if(sd && sd->status.weapon == W_BOW && battle_config.backstab_bow_penalty)
				skillratio += (200 + 40 * skill_lv) / 2;
			else
				skillratio += 200 + 40 * skill_lv;
			break;
		case RG_RAID:
			skillratio += 40 * skill_lv;
			break;
		case RG_INTIMIDATE:
			skillratio += 30 * skill_lv;
			break;
		case CR_SHIELDCHARGE:
			skillratio += 20 * skill_lv;
			break;
		case CR_SHIELDBOOMERANG:
			skillratio += 30 * skill_lv;
			break;
		case NPC_DARKCROSS:
		case CR_HOLYCROSS:
#ifdef RENEWAL
			if(sd && sd->status.weapon == W_2HSPEAR)
				skillratio += 70 * skill_lv;
			else
#endif
				skillratio += 35 * skill_lv;
			break;
		case AM_DEMONSTRATION:
			skillratio += 20 * skill_lv;
			break;
		case AM_ACIDTERROR:
			skillratio += 40 * skill_lv;
			break;
		case MO_FINGEROFFENSIVE:
			skillratio += 50 * skill_lv;
			break;
		case MO_INVESTIGATE:
			skillratio += 100 + 150 * skill_lv;
			break;
		case MO_EXTREMITYFIST:
			skillratio += 100 * (7 + sstatus->sp / 10);
			skillratio = min(500000,skillratio); //We stop at roughly 50k SP for overflow protection
			break;
		case MO_TRIPLEATTACK:
			skillratio += 20 * skill_lv;
			break;
		case MO_CHAINCOMBO:
			skillratio += 50 + 50 * skill_lv;
			break;
		case MO_COMBOFINISH:
			skillratio += 140 + 60 * skill_lv;
			break;
		case BA_MUSICALSTRIKE:
		case DC_THROWARROW:
			skillratio += 25 + 25 * skill_lv;
			break;
		case CH_TIGERFIST:
			skillratio += -60 + 100 * skill_lv;
			break;
		case CH_CHAINCRUSH:
			skillratio += 300 + 100 * skill_lv;
			break;
		case CH_PALMSTRIKE:
			skillratio += 100 + 100 * skill_lv;
			break;
		case LK_HEADCRUSH:
			skillratio += 40 * skill_lv;
			break;
		case LK_JOINTBEAT:
			i = 10 * skill_lv - 50;
			// Although not clear, it's being assumed that the 2x damage is only for the break neck ailment.
			if (wd.miscflag&BREAK_NECK)
				i *= 2;
			skillratio += i;
			break;
#ifdef RENEWAL
		// Renewal: skill ratio applies to entire damage [helvetica]
		case LK_SPIRALPIERCE:
		case ML_SPIRALPIERCE:
			skillratio += 50 * skill_lv;
		break;
#endif
		case ASC_METEORASSAULT:
			skillratio += -60 + 40 * skill_lv;
			break;
		case SN_SHARPSHOOTING:
		case MA_SHARPSHOOTING:
			skillratio += 100 + 50 * skill_lv;
			break;
		case CG_ARROWVULCAN:
			skillratio += 100 + 100 * skill_lv;
			break;
		case AS_SPLASHER:
			skillratio += 400 + 50 * skill_lv;
			if(sd)
				skillratio += 20 * pc_checkskill(sd,AS_POISONREACT);
			break;
#ifndef RENEWAL
		// Pre-Renewal: skill ratio for weapon part of damage [helvetica]
		case ASC_BREAKER:
			skillratio += -100 + 100 * skill_lv;
			break;
#endif
		case PA_SACRIFICE:
			skillratio += -10 + 10 * skill_lv;
			break;
		case PA_SHIELDCHAIN:
			skillratio += 30 * skill_lv;
			break;
		case WS_CARTTERMINATION:
			i = 10 * (16 - skill_lv);
			if (i < 1) i = 1;
			//Preserve damage ratio when max cart weight is changed.
			if (sd && sd->cart_weight)
				skillratio += sd->cart_weight / i * 80000 / battle_config.max_cart_weight - 100;
			else if (!sd)
				skillratio += 80000 / i - 100;
			break;
		case TK_DOWNKICK:
		case TK_STORMKICK:
			skillratio += 60 + 20 * skill_lv + 10 * pc_checkskill(sd,TK_RUN); //+Dmg (to Kick skills, %)
			break;
		case TK_TURNKICK:
		case TK_COUNTER:
			skillratio += 90 + 30 * skill_lv + 10 * pc_checkskill(sd,TK_RUN);
			break;
		case TK_JUMPKICK:
			skillratio += -70 + 10 * skill_lv + 10 * pc_checkskill(sd,TK_RUN);
			if (sc && sc->data[SC_COMBO] && sc->data[SC_COMBO]->val1 == skill_id)
				skillratio += 10 * status_get_lv(src) / 3; //Tumble bonus
			if (wd.miscflag) {
				skillratio += 10 * status_get_lv(src) / 3; //Running bonus (TODO: What is the real bonus?)
				if (sc && sc->data[SC_SPURT]) // Spurt bonus
					skillratio *= 2;
			}
			break;
		case GS_TRIPLEACTION:
			skillratio += 50 * skill_lv;
			break;
		case GS_BULLSEYE:
			//Only works well against brute/demihumans non bosses.
			if((tstatus->race == RC_BRUTE || tstatus->race == RC_DEMIHUMAN || tstatus->race == RC_PLAYER) && !(tstatus->mode&MD_BOSS))
				skillratio += 400;
			break;
		case GS_TRACKING:
			skillratio += 100 * (skill_lv + 1);
			break;
		case GS_PIERCINGSHOT:
#ifdef RENEWAL
			if (sd && sd->weapontype1 == W_RIFLE)
				skillratio += 50 + 30 * skill_lv;
			else
#endif
				skillratio += 20 * skill_lv;
			break;
		case GS_RAPIDSHOWER:
			skillratio += 400 + 50 * skill_lv;
			break;
		case GS_DESPERADO:
			skillratio += 50 * (skill_lv - 1);
			break;
		case GS_DUST:
			skillratio += 50 * skill_lv;
			break;
		case GS_FULLBUSTER:
			skillratio += 100 * (skill_lv + 2);
			break;
		case GS_SPREADATTACK:
#ifdef RENEWAL
			skillratio += 20 * skill_lv;
#else
			skillratio += 20 * (skill_lv - 1);
#endif
			break;
		case NJ_HUUMA:
			skillratio += 50 + 150 * skill_lv;
			break;
		case NJ_TATAMIGAESHI:
			skillratio += 10 * skill_lv;
#ifdef RENEWAL
			skillratio *= 2;
#endif
			break;
		case NJ_KASUMIKIRI:
			skillratio += 10 * skill_lv;
			break;
		case NJ_KIRIKAGE:
			skillratio += 100 * (skill_lv - 1);
			break;
		case NJ_KUNAI:
			skillratio += 200;
			break;
		case KN_CHARGEATK: { // +100% every 3 cells of distance but hard-limited to 500%
				unsigned int k = wd.miscflag / 3;

				if (k < 2)
					k = 0;
				else if (k > 1 && k < 3)
					k = 1;
				else if (k > 2 && k < 4)
					k = 2;
				else if (k > 3 && k < 5)
					k = 3;
				else
					k = 4;
				skillratio += 100 * k;
			}
			break;
		case HT_PHANTASMIC:
			skillratio += 50;
			break;
		case MO_BALKYOUNG:
			skillratio += 200;
			break;
		case HFLI_MOON: //[orn]
			skillratio += 10 + 110 * skill_lv;
			break;
		case HFLI_SBR44: //[orn]
			skillratio += 100 * (skill_lv - 1);
			break;
		case NPC_VAMPIRE_GIFT:
			skillratio += ((skill_lv - 1) % 5 + 1) * 100;
			break;
		case RK_SONICWAVE:
			skillratio += -100 + (skill_lv + 5) * 100; // ATK = {((Skill Level + 5) x 100) x (1 + [(Caster's Base Level - 100) / 200])} %
			skillratio = skillratio * (100 + (status_get_lv(src) - 100) / 2) / 100;
			break;
		case RK_HUNDREDSPEAR:
			skillratio += 500 + (80 * skill_lv);
			if (sd) {
				short index = sd->equip_index[EQI_HAND_R];

				if (index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_WEAPON)
					skillratio += max(10000 - sd->inventory_data[index]->weight, 0) / 10;
				skillratio += 50 * pc_checkskill(sd,LK_SPIRALPIERCE);
			} // (1 + [(Casters Base Level - 100) / 200])
			skillratio = skillratio * (100 + (status_get_lv(src) - 100) / 2) / 100;
			break;
		case RK_WINDCUTTER:
			skillratio += -100 + (skill_lv + 2) * 50;
			RE_LVL_DMOD(100);
			break;
		case RK_IGNITIONBREAK:
			// 3x3 cell Damage = ATK [{(Skill Level x 300) x (1 + [(Caster's Base Level - 100) / 100])}] %
			// 7x7 cell Damage = ATK [{(Skill Level x 250) x (1 + [(Caster's Base Level - 100) / 100])}] %
			// 11x11 cell Damage = ATK [{(Skill Level x 200) x (1 + [(Caster's Base Level - 100) / 100])}] %
			i = distance_bl(src,target);
			if (i < 2)
				skillratio += -100 + 300 * skill_lv;
			else if (i < 4)
				skillratio += -100 + 250 * skill_lv;
			else
				skillratio += -100 + 200 * skill_lv;
			skillratio = skillratio * status_get_lv(src) / 100;
			// Elemental check, 1.5x damage if your weapon element is fire.
			if (sstatus->rhw.ele == ELE_FIRE)
				skillratio += 100 * skill_lv;
			break;
		case RK_STORMBLAST:
			skillratio += -100 + (((sd) ? pc_checkskill(sd,RK_RUNEMASTERY) : 0) + (status_get_int(src) / 8)) * 100; // ATK = [{Rune Mastery Skill Level + (Caster's INT / 8)} x 100] %
			break;
		case RK_PHANTOMTHRUST: // ATK = [{(Skill Level x 50) + (Spear Master Level x 10)} x Caster's Base Level / 150] %
			skillratio += -100 + 50 * skill_lv + 10 * (sd ? pc_checkskill(sd,KN_SPEARMASTERY) : 5);
			RE_LVL_DMOD(150); // Base level bonus.
			break;
		case GC_CROSSIMPACT:
			skillratio += 900 + 100 * skill_lv;
			RE_LVL_DMOD(120);
			break;
		case GC_COUNTERSLASH:
			//ATK [{(Skill Level x 100) + 300} x Caster's Base Level / 120]% + ATK [(AGI x 2) + (Caster's Job Level x 4)]%
			skillratio += 200 + (100 * skill_lv);
			RE_LVL_DMOD(120);
			break;
		case GC_VENOMPRESSURE:
			skillratio += 900;
			break;
		case GC_PHANTOMMENACE:
			skillratio += 200;
			break;
		case GC_ROLLINGCUTTER:
			skillratio += -50 + 50 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case GC_CROSSRIPPERSLASHER:
			skillratio += 300 + 80 * skill_lv;
			RE_LVL_DMOD(100);
			if (sc && sc->data[SC_ROLLINGCUTTER])
				skillratio += sc->data[SC_ROLLINGCUTTER]->val1 * status_get_agi(src);
			break;
		case GC_DARKCROW:
			skillratio += 100 * (skill_lv - 1);
			break;
		case AB_DUPLELIGHT_MELEE:
			skillratio += 10 * skill_lv;
			break;
		case RA_ARROWSTORM:
			skillratio += 900 + 80 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case RA_AIMEDBOLT:
			skillratio += 400 + 50 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case RA_CLUSTERBOMB:
			skillratio += 100 + 100 * skill_lv;
			break;
		case RA_WUGDASH:// ATK 300%
			skillratio += 200;
			if (sc && sc->data[SC_DANCEWITHWUG])
				skillratio += 10 * sc->data[SC_DANCEWITHWUG]->val1 * (2 + battle_calc_chorusbonus(sd));
			break;
		case RA_WUGSTRIKE:
			skillratio += -100 + 200 * skill_lv;
			if (sc && sc->data[SC_DANCEWITHWUG])
				skillratio += 10 * sc->data[SC_DANCEWITHWUG]->val1 * (2 + battle_calc_chorusbonus(sd));
			break;
		case RA_WUGBITE:
			skillratio += 300 + 200 * skill_lv;
			if (skill_lv == 5)
				skillratio += 100;
			break;
		case RA_SENSITIVEKEEN:
			skillratio += 50 * skill_lv;
			break;
		case NC_BOOSTKNUCKLE:
			skillratio += 100 + 100 * skill_lv + status_get_dex(src);
			RE_LVL_DMOD(120);
			break;
		case NC_PILEBUNKER:
			skillratio += 200 + 100 * skill_lv + status_get_str(src);
			RE_LVL_DMOD(100);
			break;
		case NC_VULCANARM:
			skillratio += -100 + 70 * skill_lv + status_get_dex(src);
			RE_LVL_DMOD(120);
			break;
		case NC_FLAMELAUNCHER:
		case NC_COLDSLOWER:
			skillratio += 200 + 300 * skill_lv;
			RE_LVL_DMOD(150);
			break;
		case NC_ARMSCANNON:
			switch( tstatus->size ) {
				case SZ_SMALL: skillratio += 200 + 400 * skill_lv; break;// Small
				case SZ_MEDIUM: skillratio += 200 + 350 * skill_lv; break;// Medium
				case SZ_BIG: skillratio += 200 + 300 * skill_lv; break;// Large
			}
			RE_LVL_DMOD(120);
			break;
		case NC_AXEBOOMERANG:
			skillratio += 150 + 50 * skill_lv;
			if (sd) {
				short index = sd->equip_index[EQI_HAND_R];

				if (index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_WEAPON)
					skillratio += sd->inventory_data[index]->weight / 10;// Weight is divided by 10 since 10 weight in coding make 1 whole actual weight. [Rytech]
			}
			RE_LVL_DMOD(100);
			break;
		case NC_POWERSWING: // According to current sources, only the str + dex gets modified by level [Akinari]
			skillratio += -100 + status_get_str(src) + status_get_dex(src);
			RE_LVL_DMOD(100);
			skillratio += 300 + 100 * skill_lv;
			break;
		case NC_MAGMA_ERUPTION: // 'Slam' damage
			skillratio += 450 + 50 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case NC_AXETORNADO:
			skillratio += 100 + 100 * skill_lv + status_get_vit(src);
			RE_LVL_DMOD(100);
			if (distance_bl(src, target) > 2) // Will deal 75% damage outside of 5x5 area.
				skillratio = skillratio * 75 / 100;
			break;
		case SC_FATALMENACE:
			skillratio += 100 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case SC_TRIANGLESHOT:
			skillratio += 200 + (skill_lv - 1) * status_get_agi(src) / 2;
			RE_LVL_DMOD(120);
			break;
		case SC_FEINTBOMB:
			skillratio += -100 + (skill_lv + 1) * status_get_dex(src) / 2 * ((sd) ? sd->status.job_level / 10 : 1);
			RE_LVL_DMOD(120);
			break;
		case LG_CANNONSPEAR:
			skillratio += -100 + skill_lv * (50 + status_get_str(src));
			RE_LVL_DMOD(100);
			break;
		case LG_BANISHINGPOINT:
			skillratio += -100 + (50 * skill_lv) + ((sd) ? pc_checkskill(sd,SM_BASH) * 30 : 0);
			RE_LVL_DMOD(100);
			break;
		case LG_SHIELDPRESS:
			skillratio += -100 + 150 * skill_lv + status_get_str(src);
			if (sd) {
				short index = sd->equip_index[EQI_HAND_L];

				if (index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_ARMOR)
					skillratio += sd->inventory_data[index]->weight / 10;
			}
			RE_LVL_DMOD(100);
			break;
		case LG_PINPOINTATTACK:
			skillratio += -100 + 100 * skill_lv + 5 * status_get_agi(src);
			RE_LVL_DMOD(120);
			break;
		case LG_RAGEBURST:
			if (sd && sd->spiritball_old)
				skillratio += -100 + 200 * sd->spiritball_old + (status_get_max_hp(src) - status_get_hp(src)) / 100;
			else
				skillratio += 2900 + (status_get_max_hp(src) - status_get_hp(src));
			RE_LVL_DMOD(100);
			break;
		case LG_SHIELDSPELL:
			if (sd && skill_lv == 1) { // [(Casters Base Level x 4) + (Shield DEF x 10) + (Casters VIT x 2)] %
				short index = sd->equip_index[EQI_HAND_L];

				skillratio += -100 + status_get_lv(src) * 4 + status_get_vit(src) * 2;
				if (index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_ARMOR)
					skillratio += sd->inventory_data[index]->def * 10;
			} else
				skillratio = 0; // Prevent damage since level 2 is MATK. [Aleos]
			break;
		case LG_MOONSLASHER:
			skillratio += -100 + 120 * skill_lv + ((sd) ? pc_checkskill(sd,LG_OVERBRAND) * 80 : 0);
			RE_LVL_DMOD(100);
			break;
		case LG_OVERBRAND:
			skillratio += -100 + 400 * skill_lv + ((sd) ? pc_checkskill(sd,CR_SPEARQUICKEN) * 50 : 0);
			RE_LVL_DMOD(100);
			break;
		case LG_OVERBRAND_BRANDISH:
			skillratio += -100 + 300 * skill_lv + status_get_str(src) + status_get_dex(src);
			RE_LVL_DMOD(100);
			break;
		case LG_OVERBRAND_PLUSATK: // Only Piercing and Swing damage get RE_LVL_DMOD bonus damage
			skillratio += -100 + 200 * skill_lv + rnd()%90 + 10;
			break;
		case LG_RAYOFGENESIS:
			skillratio += 200 + 300 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case LG_EARTHDRIVE:
			if (sd) {
				short index = sd->equip_index[EQI_HAND_L];

				if (index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_ARMOR)
					skillratio += -100 + (skill_lv + 1) * sd->inventory_data[index]->weight / 10;
			}
			RE_LVL_DMOD(100);
			break;
		case LG_HESPERUSLIT:
			if (sc) {
				if (sc->data[SC_INSPIRATION])
					skillratio += 1100;
				if (sc->data[SC_BANDING]) {
					skillratio += -100 + 120 * skill_lv + 200 * sc->data[SC_BANDING]->val2;
					if (sc->data[SC_BANDING]->val2 > 5)
						skillratio = skillratio * 150 / 100;
				}
				RE_LVL_DMOD(100);
			}
			break;
		case SR_EARTHSHAKER:
			if( tsc && (tsc->data[SC_HIDING] || tsc->data[SC_CLOAKING] ||
				tsc->data[SC_CHASEWALK] || tsc->data[SC_CLOAKINGEXCEED] ||
				tsc->data[SC__INVISIBILITY]) )
			{ //[(Skill Level x 150) x (Caster Base Level / 100) + (Caster INT x 3)] %
				skillratio += -100 + 150 * skill_lv;
				RE_LVL_DMOD(100);
				skillratio += status_get_int(src) * 3;
			} else { //[(Skill Level x 50) x (Caster Base Level / 100) + (Caster INT x 2)] %
				skillratio += -100 + 50 * skill_lv;
				RE_LVL_DMOD(100);
				skillratio += status_get_int(src) * 2;
			}
			break;

		case SR_DRAGONCOMBO:
		case SR_FLASHCOMBO_ATK_STEP1:
			skillratio += 40 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case SR_FALLENEMPIRE:
		case SR_FLASHCOMBO_ATK_STEP2:
			// ATK [(Skill Level x 150 + 100) x Caster Base Level / 150] %
			skillratio += 150 * skill_lv;
			RE_LVL_DMOD(150);
 			break;
		case SR_TIGERCANNON:
		case SR_FLASHCOMBO_ATK_STEP3:
			{
				int hp = sstatus->max_hp * (10 + 2 * skill_lv) / 100, // skill_get_hp_rate(SR_TIGERCANNON, skill_lv)
					sp = sstatus->max_sp * (5 + 1 * skill_lv) / 100; // skill_get_sp_rate(SR_TIGERCANNON, skill_lv)
				if( sc && sc->data[SC_COMBO] && sc->data[SC_COMBO]->val1 == SR_FALLENEMPIRE )
					// Base_Damage = [((Caster consumed HP + SP) / 2) x Caster Base Level / 100] %
					skillratio += ((hp+sp) / 2);
				else
					// Base_Damage = [((Caster consumed HP + SP) / 4) x Caster Base Level / 100] %
					skillratio += ((hp+sp) / 4);
				RE_LVL_DMOD(100);
			}
			break;
		case SR_SKYNETBLOW:
		case SR_FLASHCOMBO_ATK_STEP4:
			if( sc && sc->data[SC_COMBO] && sc->data[SC_COMBO]->val1 == SR_DRAGONCOMBO )
				//ATK [{(Skill Level x 100) + (Caster AGI) + 150} x Caster Base Level / 100] %
				skillratio += (100 * skill_lv + sstatus->agi + 150);
			else
				//ATK [{(Skill Level x 80) + (Caster AGI)} x Caster Base Level / 100] %
				skillratio += (80 * skill_lv + sstatus->agi);
			RE_LVL_DMOD(100);
			break;

		case SR_RAMPAGEBLASTER:
			if (sc && sc->data[SC_EXPLOSIONSPIRITS]) {
				skillratio += -100 + (20 * sc->data[SC_EXPLOSIONSPIRITS]->val1 + 20 * skill_lv) * ((sd) ? sd->spiritball_old : 1);
				RE_LVL_DMOD(120);
			} else {
				skillratio += -100 + (20 * skill_lv) * ((sd) ? sd->spiritball_old : 1);
				RE_LVL_DMOD(150);
			}
			break;
		case SR_KNUCKLEARROW:
			if (wd.miscflag&4) { // ATK [(Skill Level x 150) + (1000 x Target current weight / Maximum weight) + (Target Base Level x 5) x (Caster Base Level / 150)] %
				skillratio += -100 + 150 * skill_lv + status_get_lv(target) * 5;
				if (tsd && tsd->weight)
					skillratio += 100 * tsd->weight / tsd->max_weight;
				RE_LVL_DMOD(150);
			} else { // ATK [(Skill Level x 100 + 500) x Caster Base Level / 100] %
				skillratio += 400 + 100 * skill_lv;
				RE_LVL_DMOD(100);
			}
			break;
		case SR_WINDMILL: // ATK [(Caster Base Level + Caster DEX) x Caster Base Level / 100] %
			skillratio += -100 + status_get_lv(src) + status_get_dex(src);
			RE_LVL_DMOD(100);
			break;
		case SR_GATEOFHELL:
			if (sc && sc->data[SC_COMBO] && sc->data[SC_COMBO]->val1 == SR_FALLENEMPIRE)
				skillratio += -100 + 800 * skill_lv;
			else
				skillratio += -100 + 500 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case SR_GENTLETOUCH_QUIET:
			skillratio += -100 + 100 * skill_lv + status_get_dex(src);
			RE_LVL_DMOD(100);
			break;
		case SR_HOWLINGOFLION:
			skillratio += -100 + 300 * skill_lv;
			RE_LVL_DMOD(150);
			break;
		case SR_RIDEINLIGHTNING: // ATK [{(Skill Level x 200) + Additional Damage} x Caster Base Level / 100] %
			skillratio += -100 + 200 * skill_lv;
			if ((sstatus->rhw.ele) == ELE_WIND || (sstatus->lhw.ele) == ELE_WIND)
				skillratio += skill_lv * 50;
			RE_LVL_DMOD(100);
			break;
		case WM_REVERBERATION_MELEE:
			// ATK [{(Skill Level x 100) + 300} x Caster Base Level / 100]
			skillratio += 200 + 100 * ((sd) ? pc_checkskill(sd, WM_REVERBERATION) : 1);
			RE_LVL_DMOD(100);
			break;
		case WM_SEVERE_RAINSTORM_MELEE:
			//ATK [{(Caster DEX + AGI) x (Skill Level / 5)} x Caster Base Level / 100] %
			skillratio = (status_get_dex(src) + status_get_agi(src)) * skill_lv / 5;
			RE_LVL_DMOD(100);
			break;
		case WM_GREAT_ECHO:
			skillratio += 300 + 200 * skill_lv;
			if (sd) {
				int chorusbonus = battle_calc_chorusbonus(sd);

				// Chorus bonus don't count the first 2 Minstrels/Wanderers and only increases when their are 3 or more. [Rytech]
				if (chorusbonus >= 1 && chorusbonus <= 5)
					skillratio += 100<<(chorusbonus-1); // 1->100; 2->200; 3->400; 4->800; 5->1600
			}
			RE_LVL_DMOD(100);
			break;
		case GN_CART_TORNADO: { // ATK [( Skill Level x 50 ) + ( Cart Weight / ( 150 - Caster Base STR ))] + ( Cart Remodeling Skill Level x 50 )] %
				skillratio += -100 + 50 * skill_lv;
				if(sd && sd->cart_weight) {
					int strbonus = status_get_base_status(src)->str; // Only use base STR
					skillratio += sd->cart_weight / 10 / (150 - min(strbonus,120)) + pc_checkskill(sd,GN_REMODELING_CART) * 50;
				}
			}
			break;
		case GN_CARTCANNON:
			// ATK [{( Cart Remodeling Skill Level x 50 ) x ( INT / 40 )} + ( Cart Cannon Skill Level x 60 )] %
			skillratio += -100 + 60 * skill_lv + ((sd) ? pc_checkskill(sd, GN_REMODELING_CART) : 1) * 50 * status_get_int(src) / 40;
			break;
		case GN_SPORE_EXPLOSION:
			skillratio += 100 + status_get_int(src) + 100 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case GN_WALLOFTHORN:
			skillratio += 10 * skill_lv;
			break;
		case GN_CRAZYWEED_ATK:
			skillratio += 400 + 100 * skill_lv;
			break;
		case GN_SLINGITEM_RANGEMELEEATK:
			if( sd ) {
				switch( sd->itemid ) {
					case ITEMID_APPLE_BOMB:
						skillratio += 200 + status_get_str(src) + status_get_dex(src);
						break;
					case ITEMID_COCONUT_BOMB:
					case ITEMID_PINEAPPLE_BOMB:
						skillratio += 700 + status_get_str(src) + status_get_dex(src);
						break;
					case ITEMID_MELON_BOMB:
						skillratio += 400 + status_get_str(src) + status_get_dex(src);
						break;
					case ITEMID_BANANA_BOMB:
						skillratio += 777 + status_get_str(src) + status_get_dex(src);
						break;
					case ITEMID_BLACK_LUMP:
						skillratio += -100 + (status_get_str(src) + status_get_agi(src) + status_get_dex(src)) / 3;
						break;
					case ITEMID_BLACK_HARD_LUMP:
						skillratio += -100 + (status_get_str(src) + status_get_agi(src) + status_get_dex(src)) / 2;
						break;
					case ITEMID_VERY_HARD_LUMP:
						skillratio += -100 + status_get_str(src) + status_get_agi(src) + status_get_dex(src);
						break;
				}
				RE_LVL_DMOD(100);
			}
			break;
		case SO_VARETYR_SPEAR://ATK [{( Striking Level x 50 ) + ( Varetyr Spear Skill Level x 50 )} x Caster Base Level / 100 ] %
			skillratio += -100 + 50 * skill_lv + ((sd) ? pc_checkskill(sd, SO_STRIKING) * 50 : 0);
			RE_LVL_DMOD(100);
			if (sc && sc->data[SC_BLAST_OPTION])
				skillratio += (sd ? sd->status.job_level * 5 : 0);
			break;
		// Physical Elemantal Spirits Attack Skills
		case EL_CIRCLE_OF_FIRE:
		case EL_FIRE_BOMB_ATK:
		case EL_STONE_RAIN:
			skillratio += 200;
			break;
		case EL_FIRE_WAVE_ATK:
			skillratio += 500;
			break;
		case EL_TIDAL_WEAPON:
			skillratio += 1400;
			break;
		case EL_WIND_SLASH:
			skillratio += 100;
			break;
		case EL_HURRICANE:
			skillratio += 600;
			break;
		case EL_TYPOON_MIS:
		case EL_WATER_SCREW_ATK:
			skillratio += 900;
			break;
		case EL_STONE_HAMMER:
			skillratio += 400;
			break;
		case EL_ROCK_CRUSHER:
			skillratio += 700;
			break;
		case KO_JYUMONJIKIRI:
			skillratio += -100 + 150 * skill_lv;
			RE_LVL_DMOD(120);
			if(tsc && tsc->data[SC_JYUMONJIKIRI])
				skillratio += skill_lv * status_get_lv(src);
			break;
		case KO_HUUMARANKA:
			skillratio += -100 + 150 * skill_lv + sstatus->agi + sstatus->dex + (sd ? pc_checkskill(sd,NJ_HUUMA) * 100 : 0);
			break;
		case KO_SETSUDAN:
			skillratio += 100 * (skill_lv - 1);
			RE_LVL_DMOD(100);
			if(tsc && tsc->data[SC_SPIRIT])
				skillratio += 200 * tsc->data[SC_SPIRIT]->val1;
			break;
		case KO_BAKURETSU:
			skillratio += -100 + (sd ? pc_checkskill(sd,NJ_TOBIDOUGU) : 1) * (50 + sstatus->dex / 4) * skill_lv * 4 / 10;
			RE_LVL_DMOD(120);
			skillratio += 10 * (sd ? sd->status.job_level : 1);
			break;
		case KO_MAKIBISHI:
			skillratio += -100 + 20 * skill_lv;
			break;
		case MH_NEEDLE_OF_PARALYZE:
			skillratio += 600 + 100 * skill_lv;
			break;
		case MH_STAHL_HORN:
			skillratio += 400 + 100 * skill_lv * status_get_lv(src) / 150;
			break;
		case MH_LAVA_SLIDE:
			skillratio += -100 + 70 * skill_lv;
			break;
		case MH_SONIC_CRAW:
			skillratio += -100 + 40 * skill_lv * status_get_lv(src) / 150;
			break;
		case MH_SILVERVEIN_RUSH:
			skillratio += -100 + 150 * skill_lv * status_get_lv(src) / 100;
			break;
		case MH_MIDNIGHT_FRENZY:
			skillratio += -100 + 300 * skill_lv * status_get_lv(src) / 150;
			break;
		case MH_TINDER_BREAKER:
			skillratio += -100 + (100 * skill_lv + 3 * status_get_str(src)) * status_get_lv(src) / 120;
			break;
		case MH_CBC:
			skillratio += 300 * skill_lv + 4 * status_get_lv(src);
			break;
		case MH_MAGMA_FLOW:
			skillratio += -100 + (100 * skill_lv + 3 * status_get_lv(src)) * status_get_lv(src) / 120;
			break;
		case RL_MASS_SPIRAL:
			skillratio += -100 + 200 * skill_lv;
			break;
		case RL_FIREDANCE:
			skillratio += -100 + 100 * skill_lv;
			skillratio += (skillratio * status_get_lv(src)) / 300; //(custom)
			break;
		case RL_BANISHING_BUSTER:
			skillratio += -100 + (400 * skill_lv); //(custom)
			break;
		case RL_S_STORM:
			skillratio += -100 + (200 * skill_lv); //(custom)
			break;
		case RL_SLUGSHOT: {
				uint16 w = 50;
				int16 idx = -1;

				if (sd && (idx = sd->equip_index[EQI_AMMO]) >= 0 && sd->inventory_data[idx])
					w = sd->inventory_data[idx]->weight / 10;
				skillratio += -100 + (max(w,1) * skill_lv * 30); //(custom)
			}
			break;
		case RL_D_TAIL:
			skillratio += -100 + (2500 + 500 * skill_lv);
			break;
		case RL_R_TRIP:
			skillratio += -100 + 150 * skill_lv; //(custom)
			break;
		case RL_R_TRIP_PLUSATK:
			skillratio += -100 + 100 * skill_lv + rnd()%10 + 100; //(custom)
			break;
		case RL_H_MINE:
			skillratio += 100 + 200 * skill_lv;
			//If damaged by Flicker, explosion damage (800%:1100%:1400%:1700%:2000%)
			if (sd && sd->flicker)
				skillratio += 800 + (skill_lv - 1) * 300;
			break;
		case RL_HAMMER_OF_GOD:
			//! TODO: Please check the right formula. [Cydh]
			//kRO Update 2013-07-24. Ratio: 1600+lv*800
			//kRO Update 2014-02-12. Coins increase the damage
			skillratio += -100 + (2400 + (skill_lv - 1) * 800) + 10 *((sd) ? sd->spiritball_old : 1); //(custom)
			break;
		case RL_QD_SHOT:
			skillratio += -100 + (max(pc_checkskill(sd,GS_CHAINACTION),1) * status_get_dex(src) / 5); //(custom)
			break;
		case RL_FIRE_RAIN:
			skillratio += -100 + 2000 + (200 * (skill_lv - 1)) + status_get_dex(src); //(custom) //kRO Update 2013-07-24. 2,000% + caster's DEX (?) [Cydh]
			break;
		case RL_AM_BLAST:
			skillratio += -100 + 300 * skill_lv + status_get_dex(src) / 5; //(custom)
			break;
	}
	return skillratio;
}

/*==================================================================================================
 * Constant skill damage additions are added before SC modifiers and after skill base ATK calculation
 *--------------------------------------------------------------------------------------------------*
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static int64 battle_calc_skill_constant_addition(struct Damage wd, struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv)
{
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	struct map_session_data *tsd = BL_CAST(BL_PC, target);
	struct status_change *sc = status_get_sc(src);
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);
	int64 atk = 0;

	//Constant/misc additions from skills
	switch (skill_id) {
		case MO_EXTREMITYFIST:
			atk = 250 + 150 * skill_lv;
			break;
#ifndef RENEWAL
		case GS_MAGICALBULLET:
			if (sstatus->matk_max > sstatus->matk_min)
				atk = sstatus->matk_min + rnd()%(sstatus->matk_max - sstatus->matk_min);
			else
				atk = sstatus->matk_min;
			break;
#endif
		case NJ_SYURIKEN:
			atk = 4 * skill_lv;
			break;
#ifdef RENEWAL
		case HT_FREEZINGTRAP:
			if(sd)
				atk = 40 * pc_checkskill(sd, RA_RESEARCHTRAP);
			break;
#endif
		case RA_WUGDASH:
			if (sd && sd->weight)
				atk = (sd->weight / 8) + (30 * pc_checkskill(sd,RA_TOOTHOFWUG));
			if (sc && sc->data[SC_DANCEWITHWUG])
				atk += 10 * sc->data[SC_DANCEWITHWUG]->val1 * (2 + battle_calc_chorusbonus(sd));
			break;
		case RA_WUGSTRIKE:
		case RA_WUGBITE:
			if(sd)
				atk = 30 * pc_checkskill(sd, RA_TOOTHOFWUG);
			if (sc && sc->data[SC_DANCEWITHWUG])
				atk += 10 * sc->data[SC_DANCEWITHWUG]->val1 * (2 + battle_calc_chorusbonus(sd));
			break;
		case GC_COUNTERSLASH:
			atk = sstatus->agi * 2 + (sd ? sd->status.job_level * 4 : 0);
			break;
		case LG_SHIELDPRESS:
			if (sd) {
				int damagevalue = 0;
				short index = sd->equip_index[EQI_HAND_L];

				if (index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_ARMOR)
					damagevalue = sstatus->vit * sd->status.inventory[index].refine;
				atk = damagevalue;
			}
			break;
		case SR_TIGERCANNON:
		case SR_FLASHCOMBO_ATK_STEP3:
			// (Tiger Cannon skill level x 240) + (Target Base Level x 40)
			if( skill_id == SR_FLASHCOMBO_ATK_STEP3 || (sc && sc->data[SC_COMBO] && sc->data[SC_COMBO]->val1 == SR_FALLENEMPIRE) )
				atk = ( skill_lv * 500 + status_get_lv(target) * 40 );
			else
				atk = ( skill_lv * 240 + status_get_lv(target) * 40 );
			break;
		case SR_FALLENEMPIRE:
		case SR_FLASHCOMBO_ATK_STEP2:
			// [(Target Size value + Skill Level - 1) x Caster STR] + [(Target current weight x Caster DEX / 120)]
			atk = ( ((tstatus->size+1)*2 + skill_lv - 1) * sstatus->str);
			if( tsd && tsd->weight )
				atk += ( (tsd->weight/10) * sstatus->dex / 120 );
			else
				atk += ( status_get_lv(target) * 50 ); //mobs
			break;
	}
	return atk;
}

/*==============================================================
 * Stackable SC bonuses added on top of calculated skill damage
 *--------------------------------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
struct Damage battle_attack_sc_bonus(struct Damage wd, struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv)
{
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	struct status_change *sc = status_get_sc(src);
	struct status_data *sstatus = status_get_status_data(src);
#ifdef RENEWAL
	struct status_data *tstatus = status_get_status_data(target);
#endif
	int inf3 = skill_get_inf3(skill_id);

	// Kagerou/Oboro Earth Charm effect +15% wATK
	if(sd && sd->spiritcharm_type == CHARM_TYPE_LAND && sd->spiritcharm > 0) {
		ATK_ADDRATE(wd.damage, wd.damage2, 15 * sd->spiritcharm);
#ifdef RENEWAL
		ATK_ADDRATE(wd.weaponAtk, wd.weaponAtk2, 15 * sd->spiritcharm);
#endif
	}

	//The following are applied on top of current damage and are stackable.
	if (sc) {
#ifdef RENEWAL
		if (sc->data[SC_WATK_ELEMENT] && skill_id != ASC_METEORASSAULT)
			ATK_ADDRATE(wd.weaponAtk, wd.weaponAtk2, sc->data[SC_WATK_ELEMENT]->val2);
		if (sc->data[SC_IMPOSITIO])
			ATK_ADD(wd.equipAtk, wd.equipAtk2, sc->data[SC_IMPOSITIO]->val2);
		if (sc->data[SC_VOLCANO])
			ATK_ADD(wd.equipAtk, wd.equipAtk2, sc->data[SC_VOLCANO]->val2);
		if (sc->data[SC_DRUMBATTLE]) {
			if (tstatus->size == SZ_SMALL) {
				ATK_ADD(wd.equipAtk, wd.equipAtk2, sc->data[SC_DRUMBATTLE]->val2);
			} else if (tstatus->size == SZ_MEDIUM)
				ATK_ADD(wd.equipAtk, wd.equipAtk2, 10 * sc->data[SC_DRUMBATTLE]->val1);
		}
		if (sc->data[SC_MADNESSCANCEL])
			ATK_ADD(wd.equipAtk, wd.equipAtk2, 100);
		if (sc->data[SC_GATLINGFEVER]) {
			if (tstatus->size == SZ_SMALL) {
				ATK_ADD(wd.equipAtk, wd.equipAtk2, 10 * sc->data[SC_GATLINGFEVER]->val1);
			} else if (tstatus->size == SZ_MEDIUM) {
				ATK_ADD(wd.equipAtk, wd.equipAtk2, 5 * sc->data[SC_GATLINGFEVER]->val1);
			} else if (tstatus->size == SZ_BIG)
				ATK_ADD(wd.equipAtk, wd.equipAtk2, sc->data[SC_GATLINGFEVER]->val1);
		}
#else
		if (sc->data[SC_TRUESIGHT])
			ATK_ADDRATE(wd.damage, wd.damage2, 2 * sc->data[SC_TRUESIGHT]->val1);
#endif
		if (sc->data[SC_SPIRIT]) {
			if (skill_id == AS_SONICBLOW && sc->data[SC_SPIRIT]->val2 == SL_ASSASIN) {
				ATK_ADDRATE(wd.damage, wd.damage2, map_flag_gvg(src->m) ? 25 : 100); //+25% dmg on woe/+100% dmg on nonwoe
				RE_ALLATK_ADDRATE(wd, map_flag_gvg(src->m) ? 25 : 100); //+25% dmg on woe/+100% dmg on nonwoe
			} else if (skill_id == CR_SHIELDBOOMERANG && sc->data[SC_SPIRIT]->val2 == SL_CRUSADER) {
				ATK_ADDRATE(wd.damage, wd.damage2, 100);
				RE_ALLATK_ADDRATE(wd, 100);
			}
		}
		if (sc->data[SC_EDP]) {
			switch(skill_id) {
				case AS_SPLASHER:
				// Pre-Renewal only: Soul Breaker and Meteor Assault ignores EDP
				// Renewal only: Grimtooth and Venom Knife ignore EDP
				// Both: Venom Splasher ignores EDP [helvetica]
#ifndef RENEWAL
				case ASC_BREAKER:
				case ASC_METEORASSAULT:
#else
				case AS_GRIMTOOTH:
				case AS_VENOMKNIFE:
#endif
					break; // skills above have no effect with edp

#ifdef RENEWAL
				// renewal EDP mode requires renewal enabled as well
				// Renewal EDP: damage gets a half modifier on top of EDP bonus for skills [helvetica]
				// * Sonic Blow
				// * Soul Breaker
				// * Counter Slash
				// * Cross Impact
				case AS_SONICBLOW:
				case ASC_BREAKER:
				case GC_COUNTERSLASH:
				case GC_CROSSIMPACT:
					ATK_RATE(wd.weaponAtk, wd.weaponAtk2, 50);
					ATK_RATE(wd.equipAtk, wd.equipAtk2, 50);
				default: // fall through to apply EDP bonuses
					// Renewal EDP formula [helvetica]
					// weapon atk * (1 + (edp level * .8))
					// equip atk * (1 + (edp level * .6))
					ATK_RATE(wd.weaponAtk, wd.weaponAtk2, 100 + (sc->data[SC_EDP]->val1 * 80));
					ATK_RATE(wd.equipAtk, wd.equipAtk2, 100 + (sc->data[SC_EDP]->val1 * 60));
					break;
#else
				default:
					ATK_ADDRATE(wd.damage, wd.damage2, sc->data[SC_EDP]->val3);

#endif
			}
		}
		if (sc->data[SC_GLOOMYDAY_SK] && (inf3&INF3_SC_GLOOMYDAY_SK)) {
			ATK_ADDRATE(wd.damage, wd.damage2, sc->data[SC_GLOOMYDAY_SK]->val2);
			RE_ALLATK_ADDRATE(wd, sc->data[SC_GLOOMYDAY_SK]->val2);
		}

		if(sc->data[SC_ZENKAI] && sstatus->rhw.ele == sc->data[SC_ZENKAI]->val2) {
			ATK_ADD(wd.damage, wd.damage2, 200);
#ifdef RENEWAL
			ATK_ADD(wd.equipAtk, wd.equipAtk2, 200);
#endif
		}
		if(sc->data[SC_STYLE_CHANGE]) {
			TBL_HOM *hd = BL_CAST(BL_HOM,src);

			if(hd) {
				ATK_ADD(wd.damage, wd.damage2, hd->homunculus.spiritball * 3);
				RE_ALLATK_ADD(wd, hd->homunculus.spiritball * 3);
			}
		}
		if(sc->data[SC_UNLIMIT] && (wd.flag&(BF_LONG|BF_MAGIC)) == BF_LONG) {
			switch(skill_id) {
				case RA_WUGDASH:
				case RA_WUGSTRIKE:
				case RA_WUGBITE:
					break;
				default:
					ATK_ADDRATE(wd.damage, wd.damage2, sc->data[SC_UNLIMIT]->val2);
					RE_ALLATK_ADDRATE(wd, sc->data[SC_UNLIMIT]->val2);
					break;
			}
		}

		if (sc->data[SC_FLASHCOMBO]) {
			ATK_ADD(wd.damage, wd.damage2, sc->data[SC_FLASHCOMBO]->val2);
#ifdef RENEWAL
			ATK_ADD(wd.equipAtk, wd.equipAtk2, sc->data[SC_FLASHCOMBO]->val2);
#endif
		}

		if((wd.flag&(BF_LONG|BF_MAGIC)) == BF_LONG) { // Monster Transformation bonus
			if (sc->data[SC_MTF_RANGEATK]) {
				ATK_ADDRATE(wd.damage, wd.damage2, sc->data[SC_MTF_RANGEATK]->val1);
				RE_ALLATK_ADDRATE(wd, sc->data[SC_MTF_RANGEATK]->val1);
			}
			if (sc->data[SC_MTF_RANGEATK2]) {
				ATK_ADDRATE(wd.damage, wd.damage2, sc->data[SC_MTF_RANGEATK]->val1);
				RE_ALLATK_ADDRATE(wd, sc->data[SC_MTF_RANGEATK]->val1);
			}
		}
	}

	return wd;
}

/*====================================
 * Calc defense damage reduction
 *------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
struct Damage battle_calc_defense_reduction(struct Damage wd, struct block_list *src,struct block_list *target, uint16 skill_id, uint16 skill_lv)
{
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	struct map_session_data *tsd = BL_CAST(BL_PC, target);
	struct status_change *sc = status_get_sc(src);
	struct status_change *tsc = status_get_sc(target);
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);

	//Defense reduction
	short vit_def;
	defType def1 = status_get_def(target); //Don't use tstatus->def1 due to skill timer reductions.
	short def2 = tstatus->def2;

#ifdef RENEWAL
	if( tsc && tsc->data[SC_ASSUMPTIO] )
		def1 <<= 1; // only eDEF is doubled
#endif

	if (sd) {
		int i = sd->ignore_def_by_race[tstatus->race] + sd->ignore_def_by_race[RC_ALL];

		if (i) {
			i = min(i,100); //cap it to 100 for 0 def min
			def1 -= def1 * i / 100;
			def2 -= def2 * i / 100;
		}

		//Kagerou/Oboro Earth Charm effect +10% eDEF
		if(sd->spiritcharm_type == CHARM_TYPE_LAND && sd->spiritcharm > 0) {
			short i = 10 * sd->spiritcharm;

			def1 = (def1 * (100 + i)) / 100;
		}
	}

	if (sc && sc->data[SC_EXPIATIO]) {
		short i = 5 * sc->data[SC_EXPIATIO]->val1; // 5% per level

		i = min(i,100); //cap it to 100 for 0 def min
		def1 = (def1*(100-i))/100;
		def2 = (def2*(100-i))/100;
	}

	if (tsc) {
		if (tsc->data[SC_FORCEOFVANGUARD]) {
			short i = 2 * tsc->data[SC_FORCEOFVANGUARD]->val1;

			def1 = (def1 * (100 + i)) / 100;
		}

		if( tsc->data[SC_CAMOUFLAGE] ){
			short i = 5 * tsc->data[SC_CAMOUFLAGE]->val3; //5% per second

			i = min(i,100); //cap it to 100 for 0 def min
			def1 = (def1*(100-i))/100;
			def2 = (def2*(100-i))/100;
		}

		if (tsc->data[SC_GT_REVITALIZE])
			def2 += tsc->data[SC_GT_REVITALIZE]->val4;

		if (tsc->data[SC_OVERED_BOOST] && target->type == BL_PC)
			def1 = (def1 * tsc->data[SC_OVERED_BOOST]->val4) / 100;
	}

	if( battle_config.vit_penalty_type && battle_config.vit_penalty_target&target->type ) {
		unsigned char target_count; //256 max targets should be a sane max

		target_count = unit_counttargeted(target);
		if(target_count >= battle_config.vit_penalty_count) {
			if(battle_config.vit_penalty_type == 1) {
				if( !tsc || !tsc->data[SC_STEELBODY] )
					def1 = (def1 * (100 - (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num))/100;
				def2 = (def2 * (100 - (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num))/100;
			} else { //Assume type 2
				if( !tsc || !tsc->data[SC_STEELBODY] )
					def1 -= (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num;
				def2 -= (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num;
			}
		}
		if (skill_id == AM_ACIDTERROR)
#ifdef RENEWAL
			def2 = 0; //Ignore only status defense. [FatalEror]
#else
			def1 = 0; //Ignores only armor defense. [Skotlex]
#endif
		if(def2 < 1)
			def2 = 1;
	}

	//Vitality reduction from rodatazone: http://rodatazone.simgaming.net/mechanics/substats.php#def
	if (tsd) {	//Sd vit-eq
		int skill;
#ifndef RENEWAL
		//[VIT*0.5] + rnd([VIT*0.3], max([VIT*0.3],[VIT^2/150]-1))
		vit_def = def2*(def2-15)/150;
		vit_def = def2/2 + (vit_def>0?rnd()%vit_def:0);
#else
		vit_def = def2;
#endif
		if( src->type == BL_MOB && (battle_check_undead(sstatus->race,sstatus->def_ele) || sstatus->race==RC_DEMON) && //This bonus already doesn't work vs players
			(skill=pc_checkskill(tsd,AL_DP)) > 0 )
			vit_def += skill*(int)(3 +(tsd->status.base_level+1)*0.04);   // submitted by orn
		if( src->type == BL_MOB && (skill=pc_checkskill(tsd,RA_RANGERMAIN))>0 &&
			(sstatus->race == RC_BRUTE || sstatus->race == RC_FISH || sstatus->race == RC_PLANT) )
			vit_def += skill*5;
		if( src->type == BL_MOB && (skill = pc_checkskill(tsd, NC_RESEARCHFE)) > 0 &&
			(sstatus->def_ele == ELE_FIRE || sstatus->def_ele == ELE_EARTH) )
			vit_def += skill * 10;
		if( src->type == BL_MOB && //Only affected from mob
			tsc && tsc->count && tsc->data[SC_P_ALTER] && //If the Platinum Alter is activated
			(battle_check_undead(sstatus->race,sstatus->def_ele) || sstatus->race==RC_UNDEAD) )	//Undead attacker
			vit_def += tsc->data[SC_P_ALTER]->val3;
	} else { //Mob-Pet vit-eq
#ifndef RENEWAL
		//VIT + rnd(0,[VIT/20]^2-1)
		vit_def = (def2/20)*(def2/20);
		vit_def = def2 + (vit_def>0?rnd()%vit_def:0);
#else
		//renewal monsters have their def swapped
		vit_def = def1;
		def1 = def2;
#endif
	}

	if (battle_config.weapon_defense_type) {
		vit_def += def1*battle_config.weapon_defense_type;
		def1 = 0;
	}

#ifdef RENEWAL
	/**
	 * RE DEF Reduction
	 * Damage = Attack * (4000+eDEF)/(4000+eDEF*10) - sDEF
	 * Pierce defence gains 1 atk per def/2
	 */
	if( def1 == -400 ) /* being hit by a gazillion units, -400 creates a division by 0 and subsequently crashes */
		def1 = -399;
	ATK_ADD2(wd.damage, wd.damage2,
		is_attack_piercing(wd, src, target, skill_id, skill_lv, EQI_HAND_R) ? (def1/2) : 0,
		is_attack_piercing(wd, src, target, skill_id, skill_lv, EQI_HAND_L) ? (def1/2) : 0
	);
	if( !attack_ignores_def(wd, src, target, skill_id, skill_lv, EQI_HAND_R) && !is_attack_piercing(wd, src, target, skill_id, skill_lv, EQI_HAND_R) )
		wd.damage = wd.damage * (4000+def1) / (4000+10*def1) - vit_def;
	if( is_attack_left_handed(src, skill_id) && !attack_ignores_def(wd, src, target, skill_id, skill_lv, EQI_HAND_L) && !is_attack_piercing(wd, src, target, skill_id, skill_lv, EQI_HAND_L) )
		wd.damage2 = wd.damage2 * (4000+def1) / (4000+10*def1) - vit_def;

#else
		if (def1 > 100) def1 = 100;
		ATK_RATE2(wd.damage, wd.damage2,
			attack_ignores_def(wd, src, target, skill_id, skill_lv, EQI_HAND_R) ?100:(is_attack_piercing(wd, src, target, skill_id, skill_lv, EQI_HAND_R) ? (int64)is_attack_piercing(wd, src, target, skill_id, skill_lv, EQI_HAND_R)*(def1+vit_def) : (100-def1)),
			attack_ignores_def(wd, src, target, skill_id, skill_lv, EQI_HAND_L) ?100:(is_attack_piercing(wd, src, target, skill_id, skill_lv, EQI_HAND_L) ? (int64)is_attack_piercing(wd, src, target, skill_id, skill_lv, EQI_HAND_L)*(def1+vit_def) : (100-def1))
		);
		ATK_ADD2(wd.damage, wd.damage2,
			attack_ignores_def(wd, src, target, skill_id, skill_lv, EQI_HAND_R) || is_attack_piercing(wd, src, target, skill_id, skill_lv, EQI_HAND_R) ?0:-vit_def,
			attack_ignores_def(wd, src, target, skill_id, skill_lv, EQI_HAND_L) || is_attack_piercing(wd, src, target, skill_id, skill_lv, EQI_HAND_L) ?0:-vit_def
		);
#endif
	return wd;
}

/*====================================
 * Modifiers ignoring DEF
 *------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
struct Damage battle_calc_attack_post_defense(struct Damage wd, struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv)
{
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	struct status_change *sc = status_get_sc(src);
	struct status_data *sstatus = status_get_status_data(src);

	// Post skill/vit reduction damage increases
	if( sc ) { // SC skill damages
		if(sc->data[SC_AURABLADE]
#ifndef RENEWAL
				&& skill_id != LK_SPIRALPIERCE && skill_id != ML_SPIRALPIERCE
#endif
		) {
			int lv = sc->data[SC_AURABLADE]->val1;
#ifdef RENEWAL
			lv *= ((skill_id == LK_SPIRALPIERCE || skill_id == ML_SPIRALPIERCE)?wd.div_:1); // +100 per hit in lv 5
#endif
			ATK_ADD(wd.damage, wd.damage2, 20*lv);
		}
	}

#ifndef RENEWAL
	wd = battle_calc_attack_masteries(wd, src, target, skill_id, skill_lv);

	//Refine bonus
	if (sd && battle_skill_stacks_masteries_vvs(skill_id) && skill_id != MO_INVESTIGATE && skill_id != MO_EXTREMITYFIST) { // Counts refine bonus multiple times
		if (skill_id == MO_FINGEROFFENSIVE) {
			ATK_ADD2(wd.damage, wd.damage2, wd.div_*sstatus->rhw.atk2, wd.div_*sstatus->lhw.atk2);
		} else {
			ATK_ADD2(wd.damage, wd.damage2, sstatus->rhw.atk2, sstatus->lhw.atk2);
		}
	}
#endif
	//Set to min of 1
	if (is_attack_right_handed(src, skill_id) && wd.damage < 1) wd.damage = 1;
	if (is_attack_left_handed(src, skill_id) && wd.damage2 < 1) wd.damage2 = 1;

	switch (skill_id) {
		case AS_SONICBLOW:
			if(sd && pc_checkskill(sd,AS_SONICACCEL)>0)
				ATK_ADDRATE(wd.damage, wd.damage2, 10);
			break;

		case NC_AXETORNADO:
			if( (sstatus->rhw.ele) == ELE_WIND || (sstatus->lhw.ele) == ELE_WIND )
				ATK_ADDRATE(wd.damage, wd.damage2, 25);
			break;
	}

	return wd;
}

/*=================================================================================
 * "Plant"-type (mobs that only take 1 damage from all sources) damage calculation
 *---------------------------------------------------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
struct Damage battle_calc_attack_plant(struct Damage wd, struct block_list *src,struct block_list *target, uint16 skill_id, uint16 skill_lv)
{
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	struct status_data *tstatus = status_get_status_data(target);
	bool attack_hits = is_attack_hitting(wd, src, target, skill_id, skill_lv, false);
	int right_element = battle_get_weapon_element(wd, src, target, skill_id, skill_lv, EQI_HAND_R, false);
	int left_element = battle_get_weapon_element(wd, src, target, skill_id, skill_lv, EQI_HAND_L, false);
	short class_ = status_get_class(target);

	//Plants receive 1 damage when hit
	if( attack_hits || wd.damage > 0 )
		wd.damage = 1; //In some cases, right hand no need to have a weapon to deal a damage
	if( is_attack_left_handed(src, skill_id) && (attack_hits || wd.damage2 > 0) ) {
		if(sd->status.weapon == W_KATAR)
			wd.damage2 = 0; //No backhand damage against plants
		else {
			wd.damage2 = 1; //Deal 1 HP damage as long as there is a weapon in the left hand
		}
	}

	if (attack_hits && target->type == BL_MOB) {
		struct status_change *sc = status_get_sc(target);
		struct mob_data *md = BL_CAST(BL_MOB,target);
		if (sc &&
			class_ != MOBID_EMPERIUM && !mob_is_battleground(md) &&
			!battle_check_sc(src, target, sc, &wd, 1, skill_id, skill_lv))
		{
			wd.damage = wd.damage2 = 0;
			return wd;
		}
	}

	if( attack_hits && class_ == MOBID_EMPERIUM ) {
		if(target && map_flag_gvg2(target->m) && !battle_can_hit_gvg_target(src,target,skill_id,(skill_id)?BF_SKILL:0)) {
			wd.damage = wd.damage2 = 0;
			return wd;
		}
		if (wd.damage > 0) {
			wd.damage = battle_attr_fix(src, target, wd.damage, right_element, tstatus->def_ele, tstatus->ele_lv);
			wd.damage = battle_calc_gvg_damage(src, target, wd.damage, skill_id, wd.flag);
		} else if (wd.damage2 > 0) {
			wd.damage2 = battle_attr_fix(src, target, wd.damage2, left_element, tstatus->def_ele, tstatus->ele_lv);
			wd.damage2 = battle_calc_gvg_damage(src, target, wd.damage2, skill_id, wd.flag);
		}
		return wd;
	}

	//For plants we don't continue with the weapon attack code, so we have to apply DAMAGE_DIV_FIX here
	wd = battle_apply_div_fix(wd);

	//If there is left hand damage, total damage can never exceed 2, even on multiple hits
	if(wd.damage > 1 && wd.damage2 > 0) {
		wd.damage = 1;
		wd.damage2 = 1;
	}

	return wd;
}

/*========================================================================================
 * Perform left/right hand weapon damage calculation based on previously calculated damage
 *----------------------------------------------------------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
struct Damage battle_calc_attack_left_right_hands(struct Damage wd, struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv)
{
	struct map_session_data *sd = BL_CAST(BL_PC, src);

	if (sd) {
		int skill;

		if (!is_attack_right_handed(src, skill_id) && is_attack_left_handed(src, skill_id)) {
			wd.damage = wd.damage2;
			wd.damage2 = 0;
		} else if(sd->status.weapon == W_KATAR && !skill_id) { //Katars (offhand damage only applies to normal attacks, tested on Aegis 10.2)
			skill = pc_checkskill(sd,TF_DOUBLE);
			wd.damage2 = (int64)wd.damage * (1 + (skill * 2))/100;
		} else if(is_attack_right_handed(src, skill_id) && is_attack_left_handed(src, skill_id)) {	//Dual-wield
			if (wd.damage) {
				if( (sd->class_&MAPID_BASEMASK) == MAPID_THIEF ) {
					skill = pc_checkskill(sd,AS_RIGHT);
					ATK_RATER(wd.damage, 50 + (skill * 10))
				}
				else if(sd->class_ == MAPID_KAGEROUOBORO) {
					skill = pc_checkskill(sd,KO_RIGHT);
					ATK_RATER(wd.damage, 70 + (skill * 10))
				}
				if(wd.damage < 1)
					wd.damage = 1;
			}
			if (wd.damage2) {
				if( (sd->class_&MAPID_BASEMASK) == MAPID_THIEF) {
					skill = pc_checkskill(sd,AS_LEFT);
					ATK_RATEL(wd.damage2, 30 + (skill * 10))
				}
				else if(sd->class_ == MAPID_KAGEROUOBORO) {
					skill = pc_checkskill(sd,KO_LEFT);
					ATK_RATEL(wd.damage2, 50 + (skill * 10))
				}
				if(wd.damage2 < 1)
					wd.damage2 = 1;
			}
		}
	}

	if(!is_attack_right_handed(src, skill_id) && !is_attack_left_handed(src, skill_id) && wd.damage)
		wd.damage=0;

	if(!is_attack_left_handed(src, skill_id) && wd.damage2)
		wd.damage2=0;

	return wd;
}

/**
* Check if bl is devoted by someone
* @param bl
* @return 'd_bl' if devoted or NULL if not devoted
*/
struct block_list *battle_check_devotion(struct block_list *bl) {
	struct block_list *d_bl = NULL;

	if (battle_config.devotion_rdamage && battle_config.devotion_rdamage > rnd() % 100) {
		struct status_change *sc = status_get_sc(bl);
		if (sc && sc->data[SC_DEVOTION])
			d_bl = map_id2bl(sc->data[SC_DEVOTION]->val1);
	}
	return d_bl;
}

/*==========================================
 * BG/GvG attack modifiers
 *------------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
struct Damage battle_calc_attack_gvg_bg(struct Damage wd, struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv)
{
	if( wd.damage + wd.damage2 ) { //There is a total damage value
		if( src != target && //Don't reflect your own damage (Grand Cross)
			(!skill_id || skill_id ||
			(src->type == BL_SKILL && (skill_id == SG_SUN_WARM || skill_id == SG_MOON_WARM || skill_id == SG_STAR_WARM))) ) {
				int64 damage = wd.damage + wd.damage2, rdamage = 0;
				struct map_session_data *tsd = BL_CAST(BL_PC, target);
				struct status_data *sstatus = status_get_status_data(src);
				int tick = gettick(), rdelay = 0;

				rdamage = battle_calc_return_damage(target, src, &damage, wd.flag, skill_id, false);
				if( rdamage > 0 ) { //Item reflect gets calculated before any mapflag reducing is applicated
					struct block_list *d_bl = battle_check_devotion(src);

					rdelay = clif_damage(src, (!d_bl) ? src : d_bl, tick, wd.amotion, sstatus->dmotion, rdamage, 1, DMG_ENDURE, 0);
					if( tsd )
						battle_drain(tsd, src, rdamage, rdamage, sstatus->race, sstatus->class_, is_infinite_defense(src,wd.flag));
					//Use Reflect Shield to signal this kind of skill trigger [Skotlex]
					battle_delay_damage(tick, wd.amotion, target, (!d_bl) ? src : d_bl, 0, CR_REFLECTSHIELD, 0, rdamage, ATK_DEF, rdelay, true);
					skill_additional_effect(target, (!d_bl) ? src : d_bl, CR_REFLECTSHIELD, 1, BF_WEAPON|BF_SHORT|BF_NORMAL, ATK_DEF, tick);
				}
		}
		if(!wd.damage2) {
			wd.damage = battle_calc_damage(src,target,&wd,wd.damage,skill_id,skill_lv);
			if( map_flag_gvg2(target->m) )
				wd.damage=battle_calc_gvg_damage(src,target,wd.damage,skill_id,wd.flag);
			else if( map[target->m].flag.battleground )
				wd.damage=battle_calc_bg_damage(src,target,wd.damage,skill_id,wd.flag);
		}
		else if(!wd.damage) {
			wd.damage2 = battle_calc_damage(src,target,&wd,wd.damage2,skill_id,skill_lv);
			if( map_flag_gvg2(target->m) )
				wd.damage2 = battle_calc_gvg_damage(src,target,wd.damage2,skill_id,wd.flag);
			else if( map[target->m].flag.battleground )
				wd.damage2 = battle_calc_bg_damage(src,target,wd.damage2,skill_id,wd.flag);
		}
		else {
			int64 d1 = wd.damage + wd.damage2,d2 = wd.damage2;
			wd.damage = battle_calc_damage(src,target,&wd,d1,skill_id,skill_lv);
			if( map_flag_gvg2(target->m) )
				wd.damage = battle_calc_gvg_damage(src,target,wd.damage,skill_id,wd.flag);
			else if( map[target->m].flag.battleground )
				wd.damage = battle_calc_bg_damage(src,target,wd.damage,skill_id,wd.flag);
			wd.damage2 = (int64)d2*100/d1 * wd.damage/100;
			if(wd.damage > 1 && wd.damage2 < 1) wd.damage2 = 1;
			wd.damage-=wd.damage2;
		}
	}
	return wd;
}

/*==========================================
 * final ATK modifiers - after BG/GvG calc
 *------------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
struct Damage battle_calc_weapon_final_atk_modifiers(struct Damage wd, struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv)
{
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	struct map_session_data *tsd = BL_CAST(BL_PC, target);
	struct status_change *sc = status_get_sc(src);
	struct status_change *tsc = status_get_sc(target);
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);
#ifdef ADJUST_SKILL_DAMAGE
	int skill_damage = 0;
#endif

	//Reject Sword bugreport:4493 by Daegaladh
	if(wd.damage && tsc && tsc->data[SC_REJECTSWORD] &&
		(src->type!=BL_PC || (
			((TBL_PC *)src)->weapontype1 == W_DAGGER ||
			((TBL_PC *)src)->weapontype1 == W_1HSWORD ||
			((TBL_PC *)src)->status.weapon == W_2HSWORD
		)) &&
		rnd()%100 < tsc->data[SC_REJECTSWORD]->val2
		)
	{
		ATK_RATER(wd.damage, 50)
		status_fix_damage(target,src,wd.damage,clif_damage(target,src,gettick(),0,0,wd.damage,0,DMG_NORMAL,0));
		clif_skill_nodamage(target,target,ST_REJECTSWORD,tsc->data[SC_REJECTSWORD]->val1,1);
		if( --(tsc->data[SC_REJECTSWORD]->val3) <= 0 )
			status_change_end(target, SC_REJECTSWORD, INVALID_TIMER);
	}

	if( tsc && tsc->data[SC_CRESCENTELBOW] && !is_boss(src) && wd.flag&BF_SHORT && rnd()%100 < tsc->data[SC_CRESCENTELBOW]->val2 ) {
		//ATK [{(Target HP / 100) x Skill Level} x Caster Base Level / 125] % + [Received damage x {1 + (Skill Level x 0.2)}]
		int64 rdamage = 0;
		int ratio = (int64)(status_get_hp(src) / 100) * tsc->data[SC_CRESCENTELBOW]->val1 * status_get_lv(target) / 125;
		if (ratio > 5000) ratio = 5000; // Maximum of 5000% ATK
		rdamage = battle_calc_base_damage(tstatus,&tstatus->rhw,tsc,sstatus->size,tsd,0);
		rdamage = (int64)rdamage * ratio / 100 + wd.damage * (10 + tsc->data[SC_CRESCENTELBOW]->val1 * 20 / 10) / 10;
		skill_blown(target, src, skill_get_blewcount(SR_CRESCENTELBOW_AUTOSPELL, tsc->data[SC_CRESCENTELBOW]->val1), unit_getdir(src), 0);
		clif_skill_damage(target, src, gettick(), status_get_amotion(src), 0, rdamage,
			1, SR_CRESCENTELBOW_AUTOSPELL, tsc->data[SC_CRESCENTELBOW]->val1, 6); // This is how official does
		clif_damage(src, target, gettick(), status_get_amotion(src)+1000, 0, rdamage/10, 1, DMG_NORMAL, 0);
		status_damage(target, src, rdamage, 0, 0, 0);
		status_damage(src, target, rdamage/10, 0, 0, 1);
		status_change_end(target, SC_CRESCENTELBOW, INVALID_TIMER);
	}

	if( sc ) {
		//SC_FUSION hp penalty [Komurka]
		if (sc->data[SC_FUSION]) {
			int hp= sstatus->max_hp;
			if (sd && tsd) {
				hp = 8*hp/100;
				if (((int64)sstatus->hp * 100) <= ((int64)sstatus->max_hp * 20))
					hp = sstatus->hp;
			} else
				hp = 2*hp/100; //2% hp loss per hit
			status_zap(src, hp, 0);
		}
		// affecting non-skills
		if( !skill_id ) {
			if( sc->data[SC_ENCHANTBLADE] && sd && ( (is_attack_right_handed(src, skill_id) && sd->weapontype1) || (is_attack_left_handed(src, skill_id) && sd->weapontype2) ) ) {
				//[( ( Skill Lv x 20 ) + 100 ) x ( casterBaseLevel / 150 )] + casterInt
				ATK_ADD(wd.damage, wd.damage2, ( sc->data[SC_ENCHANTBLADE]->val1*20+100 ) * status_get_lv(src) / 150 + status_get_int(src) );
			}
		}
		status_change_end(src,SC_CAMOUFLAGE, INVALID_TIMER);
	}
	switch (skill_id) {
		case LG_RAYOFGENESIS:
			{
				struct Damage md = battle_calc_magic_attack(src, target, skill_id, skill_lv, wd.miscflag);
				wd.damage += md.damage;
			}
			break;
#ifndef RENEWAL
		case ASC_BREAKER:
			{	//Breaker's int-based damage (a misc attack?)
				struct Damage md = battle_calc_misc_attack(src, target, skill_id, skill_lv, wd.miscflag);
				wd.damage += md.damage;
			}
			break;
#endif
	}

	// Skill damage adjustment
#ifdef ADJUST_SKILL_DAMAGE
	if ((skill_damage = battle_skill_damage(src, target, skill_id)) != 0)
		ATK_ADDRATE(wd.damage, wd.damage2, skill_damage);
#endif
	return wd;
}

/*====================================================
 * Basic wd init - not influenced by HIT/MISS/DEF/etc.
 *----------------------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static struct Damage initialize_weapon_data(struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv, int wflag)
{
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);
	struct status_change *sc = status_get_sc(src);
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	struct Damage wd;

	wd.type = DMG_NORMAL; //Normal attack
	wd.div_ = skill_id?skill_get_num(skill_id,skill_lv):1;
	wd.amotion = (skill_id && skill_get_inf(skill_id)&INF_GROUND_SKILL)?0:sstatus->amotion; //Amotion should be 0 for ground skills.
	// counter attack DOES obey ASPD delay on official, uncomment if you want the old (bad) behavior [helvetica]
	/*if(skill_id == KN_AUTOCOUNTER)
		wd.amotion >>= 1; */
	wd.dmotion = tstatus->dmotion;
	wd.blewcount =skill_get_blewcount(skill_id,skill_lv);
	wd.miscflag = wflag;
	wd.flag = BF_WEAPON; //Initial Flag
	wd.flag |= (skill_id||wd.miscflag)?BF_SKILL:BF_NORMAL; // Baphomet card's splash damage is counted as a skill. [Inkfish]

	wd.damage = wd.damage2 =
#ifdef RENEWAL
	wd.statusAtk = wd.statusAtk2 = wd.equipAtk = wd.equipAtk2 = wd.weaponAtk = wd.weaponAtk2 = wd.masteryAtk = wd.masteryAtk2 =
#endif
	0;

	wd.dmg_lv=ATK_DEF;	//This assumption simplifies the assignation later

	if(sd)
		wd.blewcount += battle_blewcount_bonus(sd, skill_id);

	if (skill_id) {
		wd.flag |= battle_range_type(src, target, skill_id, skill_lv);
		switch(skill_id)
		{
			case MH_SONIC_CRAW:{
				TBL_HOM *hd = BL_CAST(BL_HOM,src);
				wd.div_ = hd->homunculus.spiritball;
			}
				break;
			case MO_FINGEROFFENSIVE:
				if(sd) {
					if (battle_config.finger_offensive_type)
						wd.div_ = 1;
					else
						wd.div_ = sd->spiritball_old;
				}
				break;

			case KN_PIERCE:
			case ML_PIERCE:
				wd.div_= (wd.div_>0?tstatus->size+1:-(tstatus->size+1));
				break;

			case TF_DOUBLE: //For NPC used skill.
			case GS_CHAINACTION:
				wd.type = DMG_MULTI_HIT;
				break;

			case GS_GROUNDDRIFT:
			case KN_SPEARSTAB:
			case KN_BOWLINGBASH:
			case MS_BOWLINGBASH:
			case MO_BALKYOUNG:
			case TK_TURNKICK:
				wd.blewcount = 0;
				break;

			case KN_AUTOCOUNTER:
				wd.flag = (wd.flag&~BF_SKILLMASK)|BF_NORMAL;
				break;
			case LK_SPIRALPIERCE:
				if (!sd) wd.flag = (wd.flag&~(BF_RANGEMASK|BF_WEAPONMASK))|BF_LONG|BF_MISC;
				break;

			// The number of hits is set to 3 by default for use in Inspiration status.
			// When in Banding, the number of hits is equal to the number of Royal Guards in Banding.
			case LG_HESPERUSLIT:
				if( sc && sc->data[SC_BANDING] && sc->data[SC_BANDING]->val2 > 3 )
					wd.div_ = sc->data[SC_BANDING]->val2;
				break;
		}
	} else {
		wd.flag |= is_skill_using_arrow(src, skill_id)?BF_LONG:BF_SHORT;
	}

	return wd;
}

/**
 * Check if we should reflect the damage and calculate it if so
 * @param attack_type : BL_WEAPON,BL_MAGIC or BL_MISC
 * @param wd : weapon damage
 * @param src : bl who did the attack
 * @param target : target of the attack
 * @param skill_id : id of casted skill, 0 = basic atk
 * @param skill_lv : lvl of skill casted
 */
void battle_do_reflect(int attack_type, struct Damage *wd, struct block_list* src, struct block_list* target, uint16 skill_id, uint16 skill_lv)
{
	// Don't reflect your own damage (Grand Cross)
	if ((wd->damage + wd->damage2) && src && target && src != target && (src->type != BL_SKILL ||
		(src->type == BL_SKILL && (skill_id == SG_SUN_WARM || skill_id == SG_MOON_WARM || skill_id == SG_STAR_WARM ))))
	{
		int64 damage = wd->damage + wd->damage2, rdamage = 0;
		struct map_session_data *tsd = BL_CAST(BL_PC, target);
		struct status_change *tsc = status_get_sc(target);
		struct status_data *sstatus = status_get_status_data(src);
		int tick = gettick(), rdelay = 0;

		if (!tsc)
			return;

		// Calculate skill reflect damage separately
		rdamage = battle_calc_return_damage(target, src, &damage, wd->flag, skill_id,true);
		if( rdamage > 0 ) {
			struct block_list *d_bl = battle_check_devotion(src);

			if( attack_type == BF_WEAPON && tsc->data[SC_REFLECTDAMAGE] ) // Don't reflect your own damage (Grand Cross)
				map_foreachinshootrange(battle_damage_area,target,skill_get_splash(LG_REFLECTDAMAGE,1),BL_CHAR,tick,target,wd->amotion,sstatus->dmotion,rdamage,wd->flag);
			else if( attack_type == BF_WEAPON || attack_type == BF_MISC) {
				rdelay = clif_damage(src, (!d_bl) ? src : d_bl, tick, wd->amotion, sstatus->dmotion, rdamage, 1, DMG_ENDURE, 0);
				if( tsd )
					battle_drain(tsd, src, rdamage, rdamage, sstatus->race, sstatus->class_, is_infinite_defense(src,wd->flag));
				// It appears that official servers give skill reflect damage a longer delay
				battle_delay_damage(tick, wd->amotion, target, (!d_bl) ? src : d_bl, 0, CR_REFLECTSHIELD, 0, rdamage, ATK_DEF, rdelay ,true);
				skill_additional_effect(target, (!d_bl) ? src : d_bl, CR_REFLECTSHIELD, 1, BF_WEAPON|BF_SHORT|BF_NORMAL, ATK_DEF, tick);
			}
		}
	}
}

/*============================================
 * Calculate "weapon"-type attacks and skills
 *--------------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static struct Damage battle_calc_weapon_attack(struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv, int wflag)
{
	struct map_session_data *sd, *tsd;
	struct Damage wd;
	struct status_change *sc = status_get_sc(src);
	struct status_change *tsc = status_get_sc(target);
	struct status_data *tstatus = status_get_status_data(target);
	int right_element, left_element;
	bool infdef = false;

	memset(&wd,0,sizeof(wd));

	if (src == NULL || target == NULL) {
		nullpo_info(NLP_MARK);
		return wd;
	}

	wd = initialize_weapon_data(src, target, skill_id, skill_lv, wflag);

	right_element = battle_get_weapon_element(wd, src, target, skill_id, skill_lv, EQI_HAND_R, false);
	left_element = battle_get_weapon_element(wd, src, target, skill_id, skill_lv, EQI_HAND_L, false);

	if (sc && !sc->count)
		sc = NULL; //Skip checking as there are no status changes active.
	if (tsc && !tsc->count)
		tsc = NULL; //Skip checking as there are no status changes active.

	sd = BL_CAST(BL_PC, src);
	tsd = BL_CAST(BL_PC, target);

	//Check for Lucky Dodge
	if ((!skill_id || skill_id == PA_SACRIFICE) && tstatus->flee2 && rnd()%1000 < tstatus->flee2) {
		wd.type = DMG_LUCY_DODGE;
		wd.dmg_lv = ATK_LUCKY;
		if(wd.div_ < 0)
			wd.div_ *= -1;
		return wd;
	}

	// on official check for multi hit first so we can override crit on double attack [helvetica]
	wd = battle_calc_multi_attack(wd, src, target, skill_id, skill_lv);

	// crit check is next since crits always hit on official [helvetica]
	if (is_attack_critical(wd, src, target, skill_id, skill_lv, true))
		wd.type = DMG_CRITICAL;

	// check if we're landing a hit
	if(!is_attack_hitting(wd, src, target, skill_id, skill_lv, true))
		wd.dmg_lv = ATK_FLEE;
	else if(wd.miscflag&8 || !(infdef = is_infinite_defense(target, wd.flag))) { //no need for math against plants
		int64 ratio = 0;
		int i = 0;

		wd = battle_calc_skill_base_damage(wd, src, target, skill_id, skill_lv); // base skill damage
		ratio = battle_calc_attack_skill_ratio(wd, src, target, skill_id, skill_lv); // skill level ratios

		ATK_RATE(wd.damage, wd.damage2, ratio);
		RE_ALLATK_RATE(wd, ratio);

		ratio = battle_calc_skill_constant_addition(wd, src, target, skill_id, skill_lv); // other skill bonuses

		ATK_ADD(wd.damage, wd.damage2, ratio);
		RE_ALLATK_ADD(wd, ratio);

#ifdef RENEWAL
		if(skill_id == HW_MAGICCRASHER) { // Add weapon attack for MATK onto Magic Crasher
			struct status_data *sstatus = status_get_status_data(src);

			if (sstatus->matk_max > sstatus->matk_min) {
				ATK_ADD(wd.weaponAtk, wd.weaponAtk2, sstatus->matk_min+rnd()%(sstatus->matk_max-sstatus->matk_min));
			} else
				ATK_ADD(wd.weaponAtk, wd.weaponAtk2, sstatus->matk_min);
		}
#endif
		// add any miscellaneous player ATK bonuses
		if( sd && skill_id && (i = pc_skillatk_bonus(sd, skill_id))) {
			ATK_ADDRATE(wd.damage, wd.damage2, i);
			RE_ALLATK_ADDRATE(wd, i);
		}
		if (tsd && (i = pc_sub_skillatk_bonus(tsd, skill_id))) {
			ATK_ADDRATE(wd.damage, wd.damage2, -i);
			RE_ALLATK_ADDRATE(wd, -i);
		}

#ifdef RENEWAL
		// In Renewal we only cardfix to the weapon and equip ATK
		//Card Fix for attacker (sd), 2 is added to the "left" flag meaning "attacker cards only"
		wd.weaponAtk += battle_calc_cardfix(BF_WEAPON, src, target, battle_skill_get_damage_properties(skill_id, wd.miscflag), right_element, left_element, wd.weaponAtk, 2, wd.flag);
		wd.equipAtk += battle_calc_cardfix(BF_WEAPON, src, target, battle_skill_get_damage_properties(skill_id, wd.miscflag), right_element, left_element, wd.equipAtk, 2, wd.flag);
		if (is_attack_left_handed(src, skill_id)) {
			wd.weaponAtk2 += battle_calc_cardfix(BF_WEAPON, src, target, battle_skill_get_damage_properties(skill_id, wd.miscflag), right_element, left_element, wd.weaponAtk2, 3, wd.flag);
			wd.equipAtk2 += battle_calc_cardfix(BF_WEAPON, src, target, battle_skill_get_damage_properties(skill_id, wd.miscflag), right_element, left_element, wd.equipAtk2, 3, wd.flag);
		}

		// final attack bonuses that aren't affected by cards
		wd = battle_attack_sc_bonus(wd, src, target, skill_id, skill_lv);

		if (sd) { //monsters, homuns and pets have their damage computed directly
			wd.damage = wd.statusAtk + wd.weaponAtk + wd.equipAtk + wd.masteryAtk;
			wd.damage2 = wd.statusAtk2 + wd.weaponAtk2 + wd.equipAtk2 + wd.masteryAtk2;
			if(wd.flag&BF_LONG) //Long damage rate addition doesn't use weapon + equip attack
				ATK_ADDRATE(wd.damage, wd.damage2, sd->bonus.long_attack_atk_rate);
			//Custom fix for "a hole" in renewal attack calculation [exneval]
			ATK_ADDRATE(wd.damage, wd.damage2, 6);
		}
#else
		// final attack bonuses that aren't affected by cards
		wd = battle_attack_sc_bonus(wd, src, target, skill_id, skill_lv);
#endif

		if (wd.damage + wd.damage2) { //Check if attack ignores DEF
			if(!attack_ignores_def(wd, src, target, skill_id, skill_lv, EQI_HAND_L) || !attack_ignores_def(wd, src, target, skill_id, skill_lv, EQI_HAND_R))
				wd = battle_calc_defense_reduction(wd, src, target, skill_id, skill_lv);

			wd = battle_calc_attack_post_defense(wd, src, target, skill_id, skill_lv);
		}
	}

#ifdef RENEWAL
	if(!sd) // monsters only have a single ATK for element, in pre-renewal we also apply element to entire ATK on players [helvetica]
#endif
		wd = battle_calc_element_damage(wd, src, target, skill_id, skill_lv);

	if(skill_id == CR_GRANDCROSS || skill_id == NPC_GRANDDARKNESS)
		return wd; //Enough, rest is not needed.

#ifdef RENEWAL
	if (is_attack_critical(wd, src, target, skill_id, skill_lv, false)) {
		if (sd) { //Check for player so we don't crash out, monsters don't have bonus crit rates [helvetica]
			wd.damage = (int)floor((float)((wd.damage * 140) / 100 * (100 + sd->bonus.crit_atk_rate)) / 100);
			if (is_attack_left_handed(src, skill_id))
				wd.damage2 = (int)floor((float)((wd.damage2 * 140) / 100 * (100 + sd->bonus.crit_atk_rate)) / 100);
		} else
			wd.damage = (int)floor((float)(wd.damage * 140) / 100);
	}
#endif

#ifndef RENEWAL
	if (skill_id == NJ_KUNAI) {
		short nk = battle_skill_get_damage_properties(skill_id, wd.miscflag);

		ATK_ADD(wd.damage, wd.damage2, 90);
		nk |= NK_IGNORE_DEF;
	}
#endif

	switch (skill_id) {
		case TK_DOWNKICK:
		case TK_STORMKICK:
		case TK_TURNKICK:
		case TK_COUNTER:
		case TK_JUMPKICK:
			if(sd && pc_checkskill(sd,TK_RUN)) {
				uint8 i;
				uint16 skill = pc_checkskill(sd,TK_RUN);

				switch(skill) {
					case 1: case 4: case 7: case 10: i = 1; break;
					case 2: case 5: case 8: i = 2; break;
					default: i = 0; break;
				}
				if(sd->weapontype1 == W_FIST && sd->weapontype2 == W_FIST)
					ATK_ADD(wd.damage, wd.damage2, 10 * skill - i);
			}
			break;
		case SR_GATEOFHELL: {
			struct status_data *sstatus = status_get_status_data(src);

			ATK_ADD(wd.damage, wd.damage2, sstatus->max_hp - status_get_hp(src));
			if(sc && sc->data[SC_COMBO] && sc->data[SC_COMBO]->val1 == SR_FALLENEMPIRE) {
				ATK_ADD(wd.damage, wd.damage2, (sstatus->max_sp * (1 + skill_lv * 2 / 10)) + 40 * status_get_lv(src));
			} else
				ATK_ADD(wd.damage, wd.damage2, (sstatus->sp * (1 + skill_lv * 2 / 10)) + 10 * status_get_lv(src));
		}
		break;
	}

	if(sd) {
#ifndef RENEWAL
		uint16 skill;

		if ((skill = pc_checkskill(sd, BS_WEAPONRESEARCH)) > 0)
			ATK_ADD(wd.damage, wd.damage2, skill * 2);
		if (skill_id == TF_POISON)
			ATK_ADD(wd.damage, wd.damage2, 15 * skill_lv);
		if (skill_id != CR_SHIELDBOOMERANG) //Only Shield boomerang doesn't takes the Star Crumbs bonus.
			ATK_ADD2(wd.damage, wd.damage2, ((wd.div_ < 1) ? 1 : wd.div_) * sd->right_weapon.star, ((wd.div_ < 1) ? 1 : wd.div_) * sd->left_weapon.star);
		if (skill_id != MC_CARTREVOLUTION && pc_checkskill(sd, BS_HILTBINDING) > 0)
			ATK_ADD(wd.damage, wd.damage2, 4);
		if (skill_id == MO_FINGEROFFENSIVE) { //The finger offensive spheres on moment of attack do count. [Skotlex]
			ATK_ADD(wd.damage, wd.damage2, ((wd.div_ < 1) ? 1 : wd.div_) * sd->spiritball_old * 3);
		} else
			ATK_ADD(wd.damage, wd.damage2, ((wd.div_ < 1) ? 1 : wd.div_) * sd->spiritball * 3);
#endif
		if( skill_id == CR_SHIELDBOOMERANG || skill_id == PA_SHIELDCHAIN ) { //Refine bonus applies after cards and elements.
			short index = sd->equip_index[EQI_HAND_L];

			if( index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_ARMOR )
				ATK_ADD(wd.damage, wd.damage2, 10*sd->status.inventory[index].refine);
		}
#ifndef RENEWAL
		//Card Fix for attacker (sd), 2 is added to the "left" flag meaning "attacker cards only"
		switch(skill_id) {
			case RK_DRAGONBREATH:
			case RK_DRAGONBREATH_WATER:
				if(wd.flag&BF_LONG) { //Add check here, because we want to apply the same behavior in pre-renewal [exneval]
					wd.damage = wd.damage * (100 + sd->bonus.long_attack_atk_rate) / 100;
					if(is_attack_left_handed(src, skill_id))
						wd.damage2 = wd.damage2 * (100 + sd->bonus.long_attack_atk_rate) / 100;
				}
				break;
			default:
				wd.damage += battle_calc_cardfix(BF_WEAPON, src, target, battle_skill_get_damage_properties(skill_id, wd.miscflag), right_element, left_element, wd.damage, 2, wd.flag);
				if( is_attack_left_handed(src, skill_id ))
					wd.damage2 += battle_calc_cardfix(BF_WEAPON, src, target, battle_skill_get_damage_properties(skill_id, wd.miscflag), right_element, left_element, wd.damage2, 3, wd.flag);
				break;
		}
#endif
	}

	if(tsd) { // Card Fix for target (tsd), 2 is not added to the "left" flag meaning "target cards only"
		switch(skill_id) {
#ifdef RENEWAL
			case NJ_ISSEN:
			case GS_MAGICALBULLET:
			case ASC_BREAKER:
			case CR_ACIDDEMONSTRATION:
			case GN_FIRE_EXPANSION_ACID:
#endif
			case SO_VARETYR_SPEAR:
				break; //These skills will do a card fix later
			default:
				wd.damage += battle_calc_cardfix(BF_WEAPON, src, target, battle_skill_get_damage_properties(skill_id, wd.miscflag), right_element, left_element, wd.damage, 0, wd.flag);
				if(is_attack_left_handed(src, skill_id))
					wd.damage2 += battle_calc_cardfix(BF_WEAPON, src, target, battle_skill_get_damage_properties(skill_id, wd.miscflag), right_element, left_element, wd.damage2, 1, wd.flag);
				break;
		}
	}

#ifdef RENEWAL
	// forced to neutral skills [helvetica]
	// skills forced to neutral gain benefits from weapon element
	// but final damage is considered "neutral" and resistances are applied again
	switch (skill_id) {
		case MC_CARTREVOLUTION:
		case MO_INVESTIGATE:
		case CR_ACIDDEMONSTRATION:
		case SR_GATEOFHELL:
		case GN_FIRE_EXPANSION_ACID:
		case KO_BAKURETSU:
			// Forced to neutral element
			wd.damage = battle_attr_fix(src, target, wd.damage, ELE_NEUTRAL, tstatus->def_ele, tstatus->ele_lv);
			break;
		case CR_SHIELDBOOMERANG:
		case LK_SPIRALPIERCE:
		case ML_SPIRALPIERCE:
		case PA_SHIELDCHAIN:
		case PA_SACRIFICE:
		case RK_DRAGONBREATH:
		case RK_DRAGONBREATH_WATER:
		case NC_SELFDESTRUCTION:
		case KO_HAPPOKUNAI: {
				int64 tmp = wd.damage;

				if (sd) {
					if (skill_id == PA_SHIELDCHAIN) {
						wd.damage = battle_attr_fix(src, target, wd.damage, ELE_NEUTRAL, tstatus->def_ele, tstatus->ele_lv);
						if (wd.damage > 0) {
							wd.damage = battle_attr_fix(src, target, tmp, right_element, tstatus->def_ele, tstatus->ele_lv);
							if (!wd.damage)
								wd.damage = battle_attr_fix(src, target, tmp, ELE_NEUTRAL, tstatus->def_ele, tstatus->ele_lv);
						}
					} else if (skill_id == KO_HAPPOKUNAI) {
						wd.damage = battle_attr_fix(src, target, wd.damage, (sd->bonus.arrow_ele) ? sd->bonus.arrow_ele : ELE_NEUTRAL, tstatus->def_ele, tstatus->ele_lv);
						if (wd.damage > 0) {
							wd.damage = battle_attr_fix(src, target, tmp, right_element, tstatus->def_ele, tstatus->ele_lv);
							if (!wd.damage)
								wd.damage = battle_attr_fix(src, target, tmp, (sd->bonus.arrow_ele) ? sd->bonus.arrow_ele : ELE_NEUTRAL, tstatus->def_ele, tstatus->ele_lv);
						}
					} else
						wd.damage = battle_attr_fix(src, target, wd.damage, right_element, tstatus->def_ele, tstatus->ele_lv);
				}
			}
			break;
		case GN_CARTCANNON: // Cart Cannon gets forced to element of cannon ball (neutral or holy/shadow/ghost)
			wd.damage = battle_attr_fix(src, target, wd.damage, (sd && sd->bonus.arrow_ele) ? sd->bonus.arrow_ele : ELE_NEUTRAL, tstatus->def_ele, tstatus->ele_lv);
			break;
	}

	// perform multihit calculations
	DAMAGE_DIV_FIX_RENEWAL(wd, wd.div_);
#endif
	// only do 1 dmg to plant, no need to calculate rest
	if(!(wd.miscflag&8) && infdef)
		return battle_calc_attack_plant(wd, src, target, skill_id, skill_lv);

	//Apply DAMAGE_DIV_FIX and check for min damage
	wd = battle_apply_div_fix(wd);

	wd = battle_calc_attack_left_right_hands(wd, src, target, skill_id, skill_lv);

	switch (skill_id) {
#ifdef RENEWAL
		case NJ_ISSEN:
		case GS_MAGICALBULLET:
		case ASC_BREAKER:
		case CR_ACIDDEMONSTRATION:
		case GN_FIRE_EXPANSION_ACID:
#endif
		case SO_VARETYR_SPEAR:
			return wd; //These skills will do a GVG fix later
		default:
			wd = battle_calc_attack_gvg_bg(wd, src, target, skill_id, skill_lv);
			break;
	}

	wd = battle_calc_weapon_final_atk_modifiers(wd, src, target, skill_id, skill_lv);

	battle_absorb_damage(target, &wd);

	battle_do_reflect(BF_WEAPON,&wd, src, target, skill_id, skill_lv); //WIP [lighta]

	return wd;
}

/*==========================================
 * Calculate "magic"-type attacks and skills
 *------------------------------------------
 * Credits:
 *	Original coder DracoRPG
 *	Refined and optimized by helvetica
 */
struct Damage battle_calc_magic_attack(struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv,int mflag)
{
	int i, nk;
#ifdef ADJUST_SKILL_DAMAGE
	int skill_damage = 0;
#endif
	short s_ele = 0;

	TBL_PC *sd;
	TBL_PC *tsd;
	struct status_change *sc, *tsc;
	struct Damage ad;
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);
	struct {
		unsigned imdef : 1;
		unsigned infdef : 1;
	} flag;

	memset(&ad,0,sizeof(ad));
	memset(&flag,0,sizeof(flag));

	if (src == NULL || target == NULL) {
		nullpo_info(NLP_MARK);
		return ad;
	}
	//Initial Values
	ad.damage = 1;
	ad.div_ = skill_get_num(skill_id,skill_lv);
	ad.amotion = (skill_get_inf(skill_id)&INF_GROUND_SKILL ? 0 : sstatus->amotion); //Amotion should be 0 for ground skills.
	ad.dmotion = tstatus->dmotion;
	ad.blewcount = skill_get_blewcount(skill_id, skill_lv);
	ad.flag = BF_MAGIC|BF_SKILL;
	ad.dmg_lv = ATK_DEF;
	nk = skill_get_nk(skill_id);
	flag.imdef = (nk&NK_IGNORE_DEF ? 1 : 0);

	sd = BL_CAST(BL_PC, src);
	tsd = BL_CAST(BL_PC, target);
	sc = status_get_sc(src);
	tsc = status_get_sc(target);

	//Initialize variables that will be used afterwards
	s_ele = skill_get_ele(skill_id, skill_lv);

	if (s_ele == -1) { // pl=-1 : the skill takes the weapon's element
		s_ele = sstatus->rhw.ele;
		if(sd && sd->spiritcharm_type != CHARM_TYPE_NONE && sd->spiritcharm >= MAX_SPIRITCHARM)
			s_ele = sd->spiritcharm_type; // Summoning 10 spiritcharm will endow your weapon
	} else if (s_ele == -2) //Use status element
		s_ele = status_get_attack_sc_element(src,status_get_sc(src));
	else if (s_ele == -3) //Use random element
		s_ele = rnd()%ELE_ALL;

	switch(skill_id) {
		case LG_SHIELDSPELL:
			if (skill_lv == 2)
				s_ele = ELE_HOLY;
			break;
		case SO_PSYCHIC_WAVE:
			if( sc && sc->count ) {
				if( sc->data[SC_HEATER_OPTION] )
					s_ele = sc->data[SC_HEATER_OPTION]->val3;
				else if( sc->data[SC_COOLER_OPTION] )
					s_ele = sc->data[SC_COOLER_OPTION]->val3;
				else if( sc->data[SC_BLAST_OPTION] )
					s_ele = sc->data[SC_BLAST_OPTION]->val3;
				else if( sc->data[SC_CURSED_SOIL_OPTION] )
					s_ele = sc->data[SC_CURSED_SOIL_OPTION]->val3;
			}
			break;
		case KO_KAIHOU:
			if(sd && sd->spiritcharm_type != CHARM_TYPE_NONE && sd->spiritcharm > 0)
				s_ele = sd->spiritcharm_type;
			break;
	}

	//Set miscellaneous data that needs be filled
	if(sd) {
		sd->state.arrow_atk = 0;
		ad.blewcount += battle_blewcount_bonus(sd, skill_id);
	}

	//Skill Range Criteria
	ad.flag |= battle_range_type(src, target, skill_id, skill_lv);

	//Infinite defense (plant mode)
	flag.infdef = is_infinite_defense(target, ad.flag)?1:0;

	switch(skill_id) {
		case MG_FIREWALL:
			if ( tstatus->def_ele == ELE_FIRE || battle_check_undead(tstatus->race, tstatus->def_ele) )
				ad.blewcount = 0; //No knockback
			//Fall through
		case NJ_KAENSIN:
		case PR_SANCTUARY:
			ad.dmotion = 0; //No flinch animation.
			break;
	}

	if (!flag.infdef) { //No need to do the math for plants
		unsigned int skillratio = 100; //Skill dmg modifiers.
#ifdef RENEWAL
		ad.damage = 0; //reinitialize..
#endif
//MATK_RATE scales the damage. 100 = no change. 50 is halved, 200 is doubled, etc
#define MATK_RATE(a) { ad.damage = ad.damage * (a) / 100; }
//Adds dmg%. 100 = +100% (double) damage. 10 = +10% damage
#define MATK_ADDRATE(a) { ad.damage += ad.damage * (a) / 100; }
//Adds an absolute value to damage. 100 = +100 damage
#define MATK_ADD(a) { ad.damage += a; }

		//Calc base damage according to skill
		switch (skill_id) {
			case AL_HEAL:
			case PR_BENEDICTIO:
			case PR_SANCTUARY:
			case AB_HIGHNESSHEAL:
				ad.damage = skill_calc_heal(src, target, skill_id, skill_lv, false);
				break;
			case PR_ASPERSIO:
				ad.damage = 40;
				break;
			case ALL_RESURRECTION:
			case PR_TURNUNDEAD:
				//Undead check is on skill_castend_damageid code.
#ifdef RENEWAL
				i = 10 * skill_lv + sstatus->luk + sstatus->int_ + status_get_lv(src)
				  	+ 300 - 300 * tstatus->hp / tstatus->max_hp;
#else
				i = 20 * skill_lv + sstatus->luk + sstatus->int_ + status_get_lv(src)
				  	+ 200 - 200 * tstatus->hp / tstatus->max_hp;
#endif
				if(i > 700)
					i = 700;
				if(rnd()%1000 < i && !(tstatus->mode&MD_BOSS))
					ad.damage = tstatus->hp;
				else {
#ifdef RENEWAL
					if (sstatus->matk_max > sstatus->matk_min) {
						MATK_ADD(sstatus->matk_min+rnd()%(sstatus->matk_max-sstatus->matk_min));
					} else {
						MATK_ADD(sstatus->matk_min);
					}
					MATK_RATE(skill_lv);
#else
					ad.damage = status_get_lv(src) + sstatus->int_ + skill_lv * 10;
#endif
				}
				break;
			case PF_SOULBURN:
				ad.damage = tstatus->sp * 2;
				break;
			case AB_RENOVATIO:
				ad.damage = status_get_lv(src) * 10 + sstatus->int_;
				break;
			case GN_FIRE_EXPANSION_ACID:
#ifdef RENEWAL
				{
					struct Damage wd = battle_calc_weapon_attack(src, target, skill_id, skill_lv, 0);

					ad.damage = (int64)(7 * ((wd.damage / skill_lv + ad.damage / skill_lv) * tstatus->vit / 100));
				}
#else
				if(tstatus->vit + sstatus->int_)
					ad.damage = (int64)(7 * tstatus->vit * sstatus->int_ * sstatus->int_ / (10 * (tstatus->vit + sstatus->int_)));
				else
					ad.damage = 0;
				if(tsd)
					ad.damage >>= 1;
#endif
				break;
			default: {
				if (sstatus->matk_max > sstatus->matk_min) {
					MATK_ADD(sstatus->matk_min+rnd()%(sstatus->matk_max-sstatus->matk_min));
				} else {
					MATK_ADD(sstatus->matk_min);
				}

				if (nk&NK_SPLASHSPLIT) { // Divide MATK in case of multiple targets skill
					if (mflag>0)
						ad.damage /= mflag;
					else
						ShowError("0 enemies targeted by %d:%s, divide per 0 avoided!\n", skill_id, skill_get_name(skill_id));
				}

				switch(skill_id) {
					case MG_NAPALMBEAT:
						skillratio += -30 + 10 * skill_lv;
						break;
					case MG_FIREBALL:
#ifdef RENEWAL
						skillratio += 40 + 20 * skill_lv;
						if(ad.miscflag == 2) //Enemies at the edge of the area will take 75% of the damage
							skillratio = skillratio * 3 / 4;
#else
						skillratio += -30 + 10 * skill_lv;
#endif
						break;
					case MG_SOULSTRIKE:
						if (battle_check_undead(tstatus->race,tstatus->def_ele))
							skillratio += 5 * skill_lv;
						break;
					case MG_FIREWALL:
						skillratio -= 50;
						break;
					case MG_FIREBOLT:
					case MG_COLDBOLT:
					case MG_LIGHTNINGBOLT:
						if (sc && sc->data[SC_SPELLFIST] && mflag&BF_SHORT)  {
							skillratio += (sc->data[SC_SPELLFIST]->val4 * 100) + (sc->data[SC_SPELLFIST]->val1 * 50) - 100;// val4 = used bolt level, val2 = used spellfist level. [Rytech]
							ad.div_ = 1; // ad mods, to make it work similar to regular hits [Xazax]
							ad.flag = BF_WEAPON|BF_SHORT;
							ad.type = DMG_NORMAL;
						}
						break;
					case MG_THUNDERSTORM:
						// in Renewal Thunder Storm boost is 100% (in pre-re, 80%)
#ifndef RENEWAL
						skillratio -= 20;
#endif
						break;
					case MG_FROSTDIVER:
						skillratio += 10 * skill_lv;
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
						skillratio += -100 + (100 + skill_lv * 10) * 2 / 3;
						break;
					case WZ_FIREPILLAR:
						if (skill_lv > 10)
							skillratio += 2300; //200% MATK each hit
						else
							skillratio += -60 + 20 * skill_lv; //20% MATK each hit
						break;
					case WZ_SIGHTRASHER:
						skillratio += 20 * skill_lv;
						break;
					case WZ_WATERBALL:
						skillratio += 30 * skill_lv;
						break;
					case WZ_STORMGUST:
						skillratio += 40 * skill_lv;
						break;
					case HW_NAPALMVULCAN:
						skillratio += 25;
						break;
					case SL_STIN: //Target size must be small (0) for full damage
						skillratio += (tstatus->size != SZ_SMALL ? -99 : 10 * skill_lv);
						break;
					case SL_STUN: //Full damage is dealt on small/medium targets
						skillratio += (tstatus->size != SZ_BIG ? 5 * skill_lv : -99);
						break;
					case SL_SMA: //Base damage is 40% + lv%
						skillratio += -60 + status_get_lv(src);
						break;
					case NJ_KOUENKA:
						skillratio -= 10;
						if(sd && sd->spiritcharm_type == CHARM_TYPE_FIRE && sd->spiritcharm > 0)
							skillratio += 20 * sd->spiritcharm;
						break;
					case NJ_KAENSIN:
						skillratio -= 50;
						if(sd && sd->spiritcharm_type == CHARM_TYPE_FIRE && sd->spiritcharm > 0)
							skillratio += 10 * sd->spiritcharm;
						break;
					case NJ_BAKUENRYU:
						skillratio += 50 * (skill_lv - 1);
						if(sd && sd->spiritcharm_type == CHARM_TYPE_FIRE && sd->spiritcharm > 0)
							skillratio += 15 * sd->spiritcharm;
						break;
					case NJ_HYOUSENSOU:
#ifdef RENEWAL
						skillratio -= 30;
#endif
						if(sd && sd->spiritcharm_type == CHARM_TYPE_WATER && sd->spiritcharm > 0)
							skillratio += 5 * sd->spiritcharm;
						break;
					case NJ_HYOUSYOURAKU:
						skillratio += 50 * skill_lv;
						if(sd && sd->spiritcharm_type == CHARM_TYPE_WATER && sd->spiritcharm > 0)
							skillratio += 25 * sd->spiritcharm;
						break;
					case NJ_RAIGEKISAI:
						skillratio += 60 + 40 * skill_lv;
						if(sd && sd->spiritcharm_type == CHARM_TYPE_WIND && sd->spiritcharm > 0)
							skillratio += 15 * sd->spiritcharm;
						break;
					case NJ_KAMAITACHI:
						skillratio += 100 * skill_lv;
						if(sd && sd->spiritcharm_type == CHARM_TYPE_WIND && sd->spiritcharm > 0)
							skillratio += 10 * sd->spiritcharm;
						break;
					case NJ_HUUJIN:
#ifdef RENEWAL
						skillratio += 50;
#endif
						if(sd && sd->spiritcharm_type == CHARM_TYPE_WIND && sd->spiritcharm > 0)
							skillratio += 20 * sd->spiritcharm;
						break;
					case NPC_ENERGYDRAIN:
						skillratio += 100 * skill_lv;
						break;
					case NPC_EARTHQUAKE:
						skillratio += 100 + 100 * skill_lv + 100 * (skill_lv / 2);
						break;
#ifdef RENEWAL
					case WZ_HEAVENDRIVE:
					case WZ_METEOR:
						skillratio += 25;
						break;
					case WZ_VERMILION: {
						int interval = 0, per = interval, ratio = per;

						while((per++) < skill_lv) {
							ratio += interval;
							if(per%3 == 0)
								interval += 20;
						}
							if (skill_lv > 9)
								ratio -= 10;
							skillratio += ratio;
						}
						break;
#else
					case WZ_VERMILION:
						skillratio += 20 * skill_lv - 20;
						break;
#endif
					case AB_JUDEX:
						skillratio += 200 + 20 * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case AB_ADORAMUS:
						skillratio += 400 + 100 * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case AB_DUPLELIGHT_MAGIC:
						skillratio += 100 + 20 * skill_lv;
						break;
					case WL_SOULEXPANSION:
						skillratio += -100 + (skill_lv + 4) * 100 + status_get_int(src);
						RE_LVL_DMOD(100);
						break;
					case WL_FROSTMISTY:
						skillratio += 100 + 100 * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case WL_JACKFROST:
						if (tsc && tsc->data[SC_FREEZING]) {
							skillratio += 900 + 300 * skill_lv;
							RE_LVL_DMOD(100);
						} else {
							skillratio += 400 + 100 * skill_lv;
							RE_LVL_DMOD(150);
						}
						break;
					case WL_DRAINLIFE:
						skillratio += -100 + 200 * skill_lv + status_get_int(src);
						RE_LVL_DMOD(100);
						break;
					case WL_CRIMSONROCK:
						skillratio += 1200 + 300 * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case WL_HELLINFERNO:
						skillratio += -100 + 300 * skill_lv;
						RE_LVL_DMOD(100);
						// Shadow: MATK [{( Skill Level x 300 ) x ( Caster Base Level / 100 ) x 4/5 }] %
						// Fire : MATK [{( Skill Level x 300 ) x ( Caster Base Level / 100 ) /5 }] %
						if (mflag&ELE_DARK)
							skillratio *= 4;
						skillratio /= 5;
						break;
					case WL_COMET:
						i = (sc ? distance_xy(target->x, target->y, sc->comet_x, sc->comet_y) : 8);
						if (i <= 3)
							skillratio += 2400 + 500 * skill_lv; // 7 x 7 cell
						else if (i <= 5)
							skillratio += 1900 + 500 * skill_lv; // 11 x 11 cell
						else if (i <= 7)
							skillratio += 1400 + 500 * skill_lv; // 15 x 15 cell
						else
							skillratio += 900 + 500 * skill_lv; // 19 x 19 cell

						if (sd && sd->status.party_id) {
							struct map_session_data* psd;
							int p_sd[5] = {0, 0, 0, 0, 0}, c; // just limit it to 5

							c = 0;
							memset(p_sd, 0, sizeof(p_sd));
							party_foreachsamemap(skill_check_condition_char_sub, sd, 3, &sd->bl, &c, &p_sd, skill_id);
							c = (c > 1 ? rnd()%c : 0);

							if( (psd = map_id2sd(p_sd[c])) && pc_checkskill(psd,WL_COMET) > 0 ){
								skillratio = skill_lv * 400; //MATK [{( Skill Level x 400 ) x ( Caster's Base Level / 120 )} + 2500 ] %
								RE_LVL_DMOD(120);
								skillratio += 2500;
								status_zap(&psd->bl, 0, skill_get_sp(skill_id, skill_lv) / 2);
							}
						}
						break;
					case WL_CHAINLIGHTNING_ATK:
						skillratio += 400 + 100 * skill_lv;
						RE_LVL_DMOD(100);
						if (mflag > 0)
							skillratio += 100 * (9 - mflag);
						break;
					case WL_EARTHSTRAIN:
						skillratio += 1900 + 100 * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case WL_TETRAVORTEX_FIRE:
					case WL_TETRAVORTEX_WATER:
					case WL_TETRAVORTEX_WIND:
					case WL_TETRAVORTEX_GROUND:
						skillratio += 400 + 500 * skill_lv;
						break;
					case WL_SUMMON_ATK_FIRE:
					case WL_SUMMON_ATK_WATER:
					case WL_SUMMON_ATK_WIND:
					case WL_SUMMON_ATK_GROUND:
						skillratio += -100 + (1 + skill_lv) / 2 * (status_get_lv(src) + (sd ? sd->status.job_level : 0));
						RE_LVL_DMOD(100);
						break;
					case LG_RAYOFGENESIS:
						if(sc) {
							if(sc->data[SC_INSPIRATION])
								skillratio += 1400;
							if(sc->data[SC_BANDING])
								skillratio += -100 + 300 * skill_lv + 200 * sc->data[SC_BANDING]->val2;
							skillratio = skillratio * ((sd) ? sd->status.job_level / 25 : 1);
						}
						break;
					case LG_SHIELDSPELL: // [(Casters Base Level x 4) + (Shield MDEF x 100) + (Casters INT x 2)] %
						if (sd && skill_lv == 2)
							skillratio += -100 + status_get_lv(src) * 4 + sd->bonus.shieldmdef * 100 + status_get_int(src) * 2;
						else
							skillratio = 0;
						break;
					case WM_METALICSOUND:
						skillratio += -100 + 120 * skill_lv + 60 * ((sd) ? pc_checkskill(sd, WM_LESSON) : 1);
						RE_LVL_DMOD(100);
						break;
					case WM_REVERBERATION_MAGIC:
						// MATK [{(Skill Level x 100) + 100} x Casters Base Level / 100] %
						skillratio += 100 * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case SO_FIREWALK:
						skillratio += -100 + 60 * skill_lv;
						RE_LVL_DMOD(100);
						if( sc && sc->data[SC_HEATER_OPTION] )
							skillratio += (sd ? sd->status.job_level / 2 : 0);
						break;
					case SO_ELECTRICWALK:
						skillratio += -100 + 60 * skill_lv;
						RE_LVL_DMOD(100);
						if( sc && sc->data[SC_BLAST_OPTION] )
							skillratio += (sd ? sd->status.job_level / 2 : 0);
						break;
					case SO_EARTHGRAVE:
						skillratio += -100 + sstatus->int_ * skill_lv + ((sd) ? pc_checkskill(sd, SA_SEISMICWEAPON) * 200 : 0);
						RE_LVL_DMOD(100);
						if( sc && sc->data[SC_CURSED_SOIL_OPTION] )
							skillratio += (sd ? sd->status.job_level * 5 : 0);
						break;
					case SO_DIAMONDDUST:
						skillratio = ( 200 * ((sd) ? pc_checkskill(sd, SA_FROSTWEAPON) : 0) + sstatus->int_ * skill_lv );
						RE_LVL_DMOD(100);
						if( sc && sc->data[SC_COOLER_OPTION] )
							skillratio += (sd ? sd->status.job_level * 5 : 0);
						break;
					case SO_POISON_BUSTER:
						skillratio += 900 + 300 * skill_lv;
						RE_LVL_DMOD(120);
						if( sc && sc->data[SC_CURSED_SOIL_OPTION] )
							skillratio += (sd ? sd->status.job_level * 5 : 0);
						break;
					case SO_PSYCHIC_WAVE:
						skillratio += -100 + 70 * skill_lv + 3 * sstatus->int_;
						RE_LVL_DMOD(100);
						if (sc && (sc->data[SC_HEATER_OPTION] || sc->data[SC_COOLER_OPTION] ||
							sc->data[SC_BLAST_OPTION] || sc->data[SC_CURSED_SOIL_OPTION]))
							skillratio += 20;
						break;
					case SO_CLOUD_KILL:
						skillratio += -100 + 40 * skill_lv;
						RE_LVL_DMOD(100);
						if (sc && sc->data[SC_CURSED_SOIL_OPTION])
							skillratio += (sd ? sd->status.job_level : 0);
						break;
					case SO_VARETYR_SPEAR: //MATK [{( Endow Tornado skill level x 50 ) + ( Caster INT x Varetyr Spear Skill level )} x Caster Base Level / 100 ] %
						skillratio += -100 + status_get_int(src) * skill_lv + ((sd) ? pc_checkskill(sd, SA_LIGHTNINGLOADER) * 50 : 0);
						RE_LVL_DMOD(100);
						if (sc && sc->data[SC_BLAST_OPTION])
							skillratio += (sd ? sd->status.job_level * 5 : 0);
						break;
					case GN_DEMONIC_FIRE:
						if (skill_lv > 20)	// Fire expansion Lv.2
							skillratio += 10 + 20 * (skill_lv - 20) + status_get_int(src) * 10;
						else if (skill_lv > 10) { // Fire expansion Lv.1
							skillratio += 10 + 20 * (skill_lv - 10) + status_get_int(src) + ((sd) ? sd->status.job_level : 50);
							RE_LVL_DMOD(100);
						} else
							skillratio += 10 + 20 * skill_lv;
						break;
					case KO_KAIHOU:
						if(sd && sd->spiritcharm_type != CHARM_TYPE_NONE && sd->spiritcharm > 0) {
							skillratio += -100 + 200 * sd->spiritcharm;
							RE_LVL_DMOD(100);
							pc_delspiritcharm(sd, sd->spiritcharm, sd->spiritcharm_type);
						}
						break;
					// Magical Elemental Spirits Attack Skills
					case EL_FIRE_MANTLE:
					case EL_WATER_SCREW:
						skillratio += 900;
						break;
					case EL_FIRE_ARROW:
					case EL_ROCK_CRUSHER_ATK:
						skillratio += 200;
						break;
					case EL_FIRE_BOMB:
					case EL_ICE_NEEDLE:
					case EL_HURRICANE_ATK:
						skillratio += 400;
						break;
					case EL_FIRE_WAVE:
					case EL_TYPOON_MIS_ATK:
						skillratio += 1100;
						break;
					case MH_ERASER_CUTTER:
						skillratio += 400 + 100 * skill_lv + (skill_lv%2 > 0 ? 0 : 300);
						break;
					case MH_XENO_SLASHER:
						if(skill_lv%2)
							skillratio += 350 + 50 * skill_lv; //500:600:700
						else
							skillratio += 400 + 100 * skill_lv; //700:900
						break;
					case MH_HEILIGE_STANGE:
						skillratio += 400 + 250 * skill_lv;
						skillratio = (skillratio * status_get_lv(src)) / 150;
						break;
					case MH_POISON_MIST:
						skillratio += -100 + 40 * skill_lv * status_get_lv(src) / 100;
						break;
				}

				MATK_RATE(skillratio);

				//Constant/misc additions from skills
				if (skill_id == WZ_FIREPILLAR)
					MATK_ADD(100 + 50 * skill_lv);
				break;
			}
		}
#ifdef RENEWAL
		switch(skill_id) { // These skills will do a card fix later
			case CR_ACIDDEMONSTRATION:
			case ASC_BREAKER:
				break;
			default:
				ad.damage += battle_calc_cardfix(BF_MAGIC, src, target, nk, s_ele, 0, ad.damage, 0, ad.flag);
				break;
		}
#endif

		if(sd) {
			//Damage bonuses
			if ((i = pc_skillatk_bonus(sd, skill_id)))
				ad.damage += (int64)ad.damage*i/100;

			//Ignore Defense?
			if (!flag.imdef && (
				sd->bonus.ignore_mdef_ele & ( 1 << tstatus->def_ele ) || sd->bonus.ignore_mdef_ele & ( 1 << ELE_ALL ) ||
				sd->bonus.ignore_mdef_race & ( 1 << tstatus->race ) || sd->bonus.ignore_mdef_race & ( 1 << RC_ALL ) ||
				sd->bonus.ignore_mdef_class & ( 1 << tstatus->class_ ) || sd->bonus.ignore_mdef_class & ( 1 << CLASS_ALL )
			))
				flag.imdef = 1;
		}

		if (tsd && (i = pc_sub_skillatk_bonus(tsd, skill_id)))
			ad.damage -= (int64)ad.damage*i/100;

		if(!flag.imdef){
			defType mdef = tstatus->mdef;
			int mdef2= tstatus->mdef2;
#ifdef RENEWAL
			if(tsc && tsc->data[SC_ASSUMPTIO])
				mdef <<= 1; // only eMDEF is doubled
#endif
			if(sd) {
				i = sd->ignore_mdef_by_race[tstatus->race] + sd->ignore_mdef_by_race[RC_ALL];
				i += sd->ignore_mdef_by_class[tstatus->class_] + sd->ignore_mdef_by_class[CLASS_ALL];
				if (i)
				{
					if (i > 100) i = 100;
					mdef -= mdef * i/100;
					//mdef2-= mdef2* i/100;
				}
			}
#ifdef RENEWAL
			/**
			 * RE MDEF Reduction
			 * Damage = Magic Attack * (1000+eMDEF)/(1000+eMDEF) - sMDEF
			 */
			if (mdef < -99)
				mdef = -99; // Avoid divide by 0

			ad.damage = ad.damage * (1000 + mdef) / (1000 + mdef * 10) - mdef2;
#else
			if(battle_config.magic_defense_type)
				ad.damage = ad.damage - mdef*battle_config.magic_defense_type - mdef2;
			else
				ad.damage = ad.damage * (100-mdef)/100 - mdef2;
#endif
		}

		if (skill_id == NPC_EARTHQUAKE) {
			//Adds atk2 to the damage, should be influenced by number of hits and skill-ratio, but not mdef reductions. [Skotlex]
			//Also divide the extra bonuses from atk2 based on the number in range [Kevin]
			if(mflag>0)
				ad.damage+= (sstatus->rhw.atk2*skillratio/100)/mflag;
			else
				ShowError("Zero range by %d:%s, divide per 0 avoided!\n", skill_id, skill_get_name(skill_id));
		}

		if(ad.damage<1)
			ad.damage=1;
		else if(sc) { //only applies when hit
			switch(skill_id) {
				case MG_LIGHTNINGBOLT:
				case MG_THUNDERSTORM:
					if(sc->data[SC_GUST_OPTION])
						ad.damage += (6 + sstatus->int_ / 4) + max(sstatus->dex - 10, 0) / 30;
					break;
				case MG_FIREBOLT:
				case MG_FIREWALL:
					if(sc->data[SC_PYROTECHNIC_OPTION])
						ad.damage += (6 + sstatus->int_ / 4) + max(sstatus->dex - 10, 0) / 30;
					break;
				case MG_COLDBOLT:
				case MG_FROSTDIVER:
					if(sc->data[SC_AQUAPLAY_OPTION])
						ad.damage += (6 + sstatus->int_ / 4) + max(sstatus->dex - 10, 0) / 30;
					break;
				case WZ_EARTHSPIKE:
				case WZ_HEAVENDRIVE:
					if(sc->data[SC_PETROLOGY_OPTION])
						ad.damage += (6 + sstatus->int_ / 4) + max(sstatus->dex - 10, 0) / 30;
					break;
			}
		}

		if (!(nk&NK_NO_ELEFIX))
			ad.damage = battle_attr_fix(src, target, ad.damage, s_ele, tstatus->def_ele, tstatus->ele_lv);

		//Apply the physical part of the skill's damage. [Skotlex]
		switch(skill_id) {
			case CR_GRANDCROSS:
			case NPC_GRANDDARKNESS: {
					struct Damage wd = battle_calc_weapon_attack(src,target,skill_id,skill_lv,mflag);

					ad.damage = battle_attr_fix(src, target, wd.damage + ad.damage, s_ele, tstatus->def_ele, tstatus->ele_lv) * (100 + 40 * skill_lv) / 100;
					if(src == target) {
						if(src->type == BL_PC)
							ad.damage = ad.damage / 2;
						else
							ad.damage = 0;
					}
				}
				break;
			case SO_VARETYR_SPEAR: {
					struct Damage wd = battle_calc_weapon_attack(src, target, skill_id, skill_lv, mflag);

					ad.damage += wd.damage;
				}
				break;
		}

#ifndef RENEWAL
		ad.damage += battle_calc_cardfix(BF_MAGIC, src, target, nk, s_ele, 0, ad.damage, 0, ad.flag);
#endif
	} //Hint: Against plants damage will still be 1 at this point

	//Apply DAMAGE_DIV_FIX and check for min damage
	ad = battle_apply_div_fix(ad);

	switch(skill_id) { // These skills will do a GVG fix later
#ifdef RENEWAL
		case ASC_BREAKER:
		case CR_ACIDDEMONSTRATION:
			return ad; //These skills will do a GVG fix later
#endif
		default:
			ad.damage = battle_calc_damage(src,target,&ad,ad.damage,skill_id,skill_lv);
			if (map_flag_gvg2(target->m))
				ad.damage = battle_calc_gvg_damage(src,target,ad.damage,skill_id,ad.flag);
			else if (map[target->m].flag.battleground)
				ad.damage = battle_calc_bg_damage(src,target,ad.damage,skill_id,ad.flag);
			break;
	}

	// Skill damage adjustment
#ifdef ADJUST_SKILL_DAMAGE
	if ((skill_damage = battle_skill_damage(src,target,skill_id)) != 0)
		MATK_ADDRATE(skill_damage);
#endif

	battle_absorb_damage(target, &ad);

	//battle_do_reflect(BF_MAGIC,&ad, src, target, skill_id, skill_lv); //WIP [lighta] Magic skill has own handler at skill_attack
	return ad;
}

/*==========================================
 * Calculate "misc"-type attacks and skills
 *------------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Refined and optimized by helvetica
 */
struct Damage battle_calc_misc_attack(struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv,int mflag)
{
#ifdef ADJUST_SKILL_DAMAGE
	int skill_damage = 0;
#endif
	short i, nk;
	short s_ele;

	struct map_session_data *sd, *tsd;
	struct Damage md; //DO NOT CONFUSE with md of mob_data!
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);

	memset(&md,0,sizeof(md));

	if (src == NULL || target == NULL) {
		nullpo_info(NLP_MARK);
		return md;
	}

	//Some initial values
	md.amotion = (skill_get_inf(skill_id)&INF_GROUND_SKILL ? 0 : sstatus->amotion);
	md.dmotion = tstatus->dmotion;
	md.div_ = skill_get_num(skill_id,skill_lv);
	md.blewcount = skill_get_blewcount(skill_id,skill_lv);
	md.dmg_lv = ATK_DEF;
	md.flag = BF_MISC|BF_SKILL;

	nk = skill_get_nk(skill_id);

	sd = BL_CAST(BL_PC, src);
	tsd = BL_CAST(BL_PC, target);

	if(sd) {
		sd->state.arrow_atk = 0;
		md.blewcount += battle_blewcount_bonus(sd, skill_id);
	}

	s_ele = skill_get_ele(skill_id, skill_lv);
	if (s_ele < 0 && s_ele != -3) //Attack that takes weapon's element for misc attacks? Make it neutral [Skotlex]
		s_ele = ELE_NEUTRAL;
	else if (s_ele == -3) { //Use random element
		if (skill_id == SN_FALCONASSAULT) {
			if (sstatus->rhw.ele && !status_get_attack_sc_element(src, status_get_sc(src)))
				s_ele = sstatus->rhw.ele;
			else
				s_ele = status_get_attack_sc_element(src, status_get_sc(src));
		} else
			s_ele = rnd()%ELE_ALL;
	}

	//Skill Range Criteria
	md.flag |= battle_range_type(src, target, skill_id, skill_lv);

	switch (skill_id) {
		case NC_MAGMA_ERUPTION: // 'Eruption' damage
			md.damage = 800 + 200 * skill_lv;
			break;
		case TF_THROWSTONE:
			md.damage = 50;
			md.flag |= BF_WEAPON;
			break;
#ifdef RENEWAL
		case HT_LANDMINE:
		case MA_LANDMINE:
		case HT_BLASTMINE:
		case HT_CLAYMORETRAP:
			md.damage = skill_lv * sstatus->dex * (3 + status_get_lv(src) / 100) * (1 + sstatus->int_ / 35);
			md.damage += md.damage * (rnd()%20 - 10) / 100;
			md.damage += (sd ? pc_checkskill(sd,RA_RESEARCHTRAP) * 40 : 0);
			break;
#else
		case HT_LANDMINE:
		case MA_LANDMINE:
			md.damage = skill_lv * (sstatus->dex + 75) * (100 + sstatus->int_) / 100;
			break;
		case HT_BLASTMINE:
			md.damage = skill_lv * (sstatus->dex / 2 + 50) * (100 + sstatus->int_) / 100;
			break;
		case HT_CLAYMORETRAP:
			md.damage = skill_lv * (sstatus->dex / 2 + 75) * (100 + sstatus->int_) / 100;
			break;
#endif
		case HT_BLITZBEAT:
		case SN_FALCONASSAULT:
			{
				uint16 skill;

				//Blitz-beat Damage
				if(!sd || !(skill = pc_checkskill(sd,HT_STEELCROW)))
					skill = 0;
				md.damage = (sstatus->dex / 10 + sstatus->int_ / 2 + skill * 3 + 40) * 2;
				if(mflag > 1) //Autocasted Blitz
					nk |= NK_SPLASHSPLIT;
				if (skill_id == SN_FALCONASSAULT) {
					//Div fix of Blitzbeat
					DAMAGE_DIV_FIX2(md.damage, skill_get_num(HT_BLITZBEAT, 5));
					//Falcon Assault Modifier
					md.damage = md.damage * (150 + 70 * skill_lv) / 100;
				}
			}
			break;
#ifdef RENEWAL
		case GS_MAGICALBULLET:
			{
				//Official renewal formula [exneval]
				//Damage = (Final ATK + Final MATK) * Skill modifiers - (eDEF + sDEF + eMDEF + sMDEF)
				short totaldef, totalmdef;
				struct Damage atk, matk;

				atk = battle_calc_weapon_attack(src,target,skill_id,skill_lv,md.miscflag);
				matk = battle_calc_magic_attack(src,target,skill_id,skill_lv,md.miscflag);
				md.damage = atk.damage + matk.damage;
				totaldef = (short)status_get_def(target) + tstatus->def2;
				totalmdef = tstatus->mdef + tstatus->mdef2;
				md.damage -= totaldef + totalmdef;
				md.flag |= BF_WEAPON;
				nk |= NK_IGNORE_FLEE; // Flee already checked in battle_calc_weapon_attack, so don't do it again here [exneval]
			}
			break;
#endif
		case BA_DISSONANCE:
			md.damage = 30 + skill_lv * 10;
			if (sd)
				md.damage += 3 * pc_checkskill(sd,BA_MUSICALLESSON);
			break;
		case NPC_SELFDESTRUCTION:
			md.damage = sstatus->hp;
			break;
		case NPC_SMOKING:
			md.damage = 3;
			break;
		case NPC_DARKBREATH:
			md.damage = tstatus->max_hp * skill_lv * 10 / 100;
			break;
		case NPC_EVILLAND:
			md.damage = skill_calc_heal(src,target,skill_id,skill_lv,false);
			break;
		case ASC_BREAKER:
#ifdef RENEWAL
			// Official Renewal formula [helvetica]
			// damage = ((atk + matk) * (3 + (.5 * skill level))) - (edef + sdef + emdef + smdef)
			// atk part takes weapon element, matk part is non-elemental
			// modified def formula
			{
				short totaldef, totalmdef;
				struct Damage atk, matk;

				atk = battle_calc_weapon_attack(src, target, skill_id, skill_lv, 0);
				nk |= NK_NO_ELEFIX; // atk part takes on weapon element, matk part is non-elemental
				matk = battle_calc_magic_attack(src, target, skill_id, skill_lv, 0);

				// (atk + matk) * (3 + (.5 * skill level))
				md.damage = ((30 + (5 * skill_lv)) * (atk.damage + matk.damage)) / 10;

				// modified def reduction, final damage = base damage - (edef + sdef + emdef + smdef)
				totaldef = tstatus->def2 + (short)status_get_def(target);
				totalmdef = tstatus->mdef + tstatus->mdef2;
				md.damage -= totaldef + totalmdef;
			}
#else
			md.damage = 500 + rnd()%500 + 5 * skill_lv * sstatus->int_;
			nk |= NK_IGNORE_FLEE|NK_NO_ELEFIX; //These two are not properties of the weapon based part.
#endif
			break;
		case HW_GRAVITATION:
#ifdef RENEWAL
			md.damage = 500 + 100 * skill_lv;
#else
			md.damage = 200 + 200 * skill_lv;
#endif
			md.dmotion = 0; //No flinch animation
			break;
		case PA_PRESSURE:
			md.damage = 500 + 300 * skill_lv;
			break;
		case PA_GOSPEL:
			md.damage = 1 + rnd()%9999;
			break;
		case CR_ACIDDEMONSTRATION:
#ifdef RENEWAL
			// Official Renewal formula [helvetica]
			// damage = 7 * ((atk + matk)/skill level) * (target vit/100)
			// skill is a "forced neutral" type skill, it benefits from weapon element but final damage
			// 	is considered "neutral" for purposes of resistances
			{
				struct Damage atk = battle_calc_weapon_attack(src, target, skill_id, skill_lv, 0);
				struct Damage matk = battle_calc_magic_attack(src, target, skill_id, skill_lv, 0);
				md.damage = 7 * ((atk.damage/skill_lv + matk.damage/skill_lv) * tstatus->vit / 100 );
	
				// AD benefits from endow/element but damage is forced back to neutral
				md.damage = battle_attr_fix(src, target, md.damage, ELE_NEUTRAL, tstatus->def_ele, tstatus->ele_lv);
			}
#else
			if(tstatus->vit+sstatus->int_) //crash fix
				md.damage = (int)((int64)7*tstatus->vit*sstatus->int_*sstatus->int_ / (10*(tstatus->vit+sstatus->int_)));
			else
				md.damage = 0;
			if (tsd) md.damage>>=1;
#endif
			break;
		case NJ_ZENYNAGE:
		case KO_MUCHANAGE:
				md.damage = skill_get_zeny(skill_id, skill_lv);
				if (!md.damage)
					md.damage = (skill_id == NJ_ZENYNAGE ? 2 : 10);
				md.damage = (skill_id == NJ_ZENYNAGE ? rnd()%md.damage + md.damage : md.damage * rnd_value(50,100)) / (skill_id == NJ_ZENYNAGE ? 1 : 100);
				if (sd && skill_id == KO_MUCHANAGE && !pc_checkskill(sd, NJ_TOBIDOUGU))
					md.damage = md.damage / 2;
				if (is_boss(target))
					md.damage = md.damage / (skill_id == NJ_ZENYNAGE ? 3 : 2);
				else if (tsd && skill_id == NJ_ZENYNAGE)
					md.damage = md.damage / 2;
			break;
#ifdef RENEWAL
		case NJ_ISSEN:
			// Official Renewal formula [helvetica]
			// base damage = currenthp + ((atk * currenthp * skill level) / maxhp)
			// final damage = base damage + ((mirror image count + 1) / 5 * base damage) - (edef + sdef)
			// modified def formula
			{
				short totaldef;
				struct Damage atk = battle_calc_weapon_attack(src, target, skill_id, skill_lv, 0);
				struct status_change *sc = status_get_sc(src);

				md.damage = (int64)sstatus->hp + (atk.damage * (int64)sstatus->hp * skill_lv) / (int64)sstatus->max_hp;

				if (sc && sc->data[SC_BUNSINJYUTSU] && (i = sc->data[SC_BUNSINJYUTSU]->val2) > 0) { // mirror image bonus only occurs if active
					md.div_ = -(i + 2); // mirror image count + 2
					md.damage += (md.damage * (((i + 1) * 10) / 5)) / 10;
				}
				// modified def reduction, final damage = base damage - (edef + sdef)
				totaldef = tstatus->def2 + (short)status_get_def(target);
				md.damage -= totaldef;
				md.flag |= BF_WEAPON;
			}
			break;
#endif
		case GS_FLING:
			md.damage = (sd ? sd->status.job_level : status_get_lv(src));
			break;
		case GS_GROUNDDRIFT:
			// Official formula [helvetica]
			// bonus damage = 50 * skill level (fixed damage)
			s_ele = ELE_NEUTRAL;
			md.damage = battle_attr_fix(src, target, 50 * skill_lv, s_ele, tstatus->def_ele, tstatus->ele_lv);
			break;
		case HVAN_EXPLOSION: //[orn]
			md.damage = (int64)sstatus->max_hp * (50 + 50 * skill_lv) / 100;
			break;
		case RA_CLUSTERBOMB:
		case RA_FIRINGTRAP:
		case RA_ICEBOUNDTRAP:
			md.damage = skill_lv * status_get_dex(src) + status_get_int(src) * 5 ;
			RE_LVL_TMDMOD();
			if(sd) {
				int researchskill_lv = pc_checkskill(sd,RA_RESEARCHTRAP);
				if(researchskill_lv)
					md.damage = md.damage * 20 * researchskill_lv / (skill_id == RA_CLUSTERBOMB ? 50 : 100);
				else
					md.damage = 0;
			} else
				md.damage = md.damage * 200 / (skill_id == RA_CLUSTERBOMB ? 50 : 100);
			nk |= NK_NO_ELEFIX|NK_IGNORE_FLEE|NK_NO_CARDFIX_DEF;
			break;
		case WM_SOUND_OF_DESTRUCTION:
			md.damage = 1000 * skill_lv + sstatus->int_ * ((sd) ? pc_checkskill(sd,WM_LESSON) : 1);
			md.damage += md.damage * 10 * ((sd) ? battle_calc_chorusbonus(sd) / 100 : 0);
			break;
		case GN_THORNS_TRAP:
			md.damage = 100 + 200 * skill_lv + status_get_int(src);
			break;
		case GN_HELLS_PLANT_ATK:
			//[{( Hell Plant Skill Level x Casters Base Level ) x 10 } + {( Casters INT x 7 ) / 2 } x { 18 + ( Casters Job Level / 4 )] x ( 5 / ( 10 - Summon Flora Skill Level ))
			md.damage = ( skill_lv * status_get_lv(src) * 10 ) + ( status_get_int(src) * 7 / 2 ) * ( 18 + (sd?sd->status.job_level:0) / 4 ) * ( 5 / (10 - ((sd) ? pc_checkskill(sd,AM_CANNIBALIZE) : 0)) );
			break;
		case RL_B_TRAP:
			// kRO 2014-02-12: Damage: Caster's DEX, Target's current HP, Skill Level
			md.damage = ((200 + status_get_dex(src)) * skill_lv * 10) + sstatus->hp; // (custom)
			break;
	}

	if (nk&NK_SPLASHSPLIT) { // Divide ATK among targets
		if(mflag > 0)
			md.damage /= mflag;
		else
			ShowError("0 enemies targeted by %d:%s, divide per 0 avoided!\n", skill_id, skill_get_name(skill_id));
	}

	if (!(nk&NK_IGNORE_FLEE)) {
		struct status_change *sc = status_get_sc(target);

		i = 0; //Temp for "hit or no hit"
		if(sc && sc->opt1 && sc->opt1 != OPT1_STONEWAIT && sc->opt1 != OPT1_BURNING)
			i = 1;
		else {
			short
				flee = tstatus->flee,
#ifdef RENEWAL
				hitrate = 0; //Default hitrate
#else
				hitrate = 80; //Default hitrate
#endif

			if(battle_config.agi_penalty_type && battle_config.agi_penalty_target&target->type) {
				unsigned char attacker_count = unit_counttargeted(target); //256 max targets should be a sane max

				if(attacker_count >= battle_config.agi_penalty_count) {
					if (battle_config.agi_penalty_type == 1)
						flee = (flee * (100 - (attacker_count - (battle_config.agi_penalty_count - 1))*battle_config.agi_penalty_num))/100;
					else //assume type 2: absolute reduction
						flee -= (attacker_count - (battle_config.agi_penalty_count - 1))*battle_config.agi_penalty_num;
					if(flee < 1)
						flee = 1;
				}
			}

			hitrate += sstatus->hit - flee;
#ifdef RENEWAL
			if( sd ) //in Renewal hit bonus from Vultures Eye is not shown anymore in status window
				hitrate += pc_checkskill(sd,AC_VULTURE);
#endif
			hitrate = cap_value(hitrate, battle_config.min_hitrate, battle_config.max_hitrate);

			if(rnd()%100 < hitrate)
				i = 1;
		}
		if (!i) {
			md.damage = 0;
			md.dmg_lv = ATK_FLEE;
		}
	}

	switch(skill_id) {
#ifdef RENEWAL
		case GS_MAGICALBULLET:
			break; // Card fix already done
#endif
		default:
			md.damage += battle_calc_cardfix(BF_MISC, src, target, nk, s_ele, 0, md.damage, 0, md.flag);
			break;
	}

	if (sd && (i = pc_skillatk_bonus(sd, skill_id)))
		md.damage += (int64)md.damage*i/100;

	if (tsd && (i = pc_sub_skillatk_bonus(tsd, skill_id)))
		md.damage -= (int64)md.damage*i/100;

	if(!(nk&NK_NO_ELEFIX))
		md.damage=battle_attr_fix(src, target, md.damage, s_ele, tstatus->def_ele, tstatus->ele_lv);

	//Plant damage
	if(md.damage < 0)
		md.damage = 0;
	else if(md.damage && is_infinite_defense(target, md.flag)) {
		md.damage = 1;
	}

	//Apply DAMAGE_DIV_FIX and check for min damage
	md = battle_apply_div_fix(md);

	switch(skill_id) {
		case RA_FIRINGTRAP:
 		case RA_ICEBOUNDTRAP:
			if (md.damage == 1)
				break;
		case RA_CLUSTERBOMB:
			{
				struct Damage wd = battle_calc_weapon_attack(src,target,skill_id,skill_lv,mflag);

				md.damage += wd.damage;
			}
			break;
		case NJ_ZENYNAGE:
			if (sd) {
				if (md.damage > sd->status.zeny)
					md.damage = sd->status.zeny;
				pc_payzeny(sd,(int)cap_value(md.damage, INT_MIN, INT_MAX),LOG_TYPE_STEAL,NULL);
			}
			break;
		case GS_GROUNDDRIFT:
			{
				struct Damage wd = battle_calc_weapon_attack(src,target,skill_id,skill_lv,mflag);
				int blewcount = skill_get_blewcount(skill_id, skill_lv);

				md.damage += wd.damage;
				// Knockback only from Fire Element (except from bonuses?)
				if (mflag != ELE_FIRE && md.blewcount >= blewcount)
					md.blewcount -= blewcount;
			}
			break;
	}

	switch(skill_id) {
#ifdef RENEWAL
		case GS_MAGICALBULLET:
			break; // GVG fix already done
#endif
		default:
			md.damage = battle_calc_damage(src,target,&md,md.damage,skill_id,skill_lv);
			if(map_flag_gvg2(target->m))
				md.damage = battle_calc_gvg_damage(src,target,md.damage,skill_id,md.flag);
			else if(map[target->m].flag.battleground)
				md.damage = battle_calc_bg_damage(src,target,md.damage,skill_id,md.flag);
			break;
	}

	// Skill damage adjustment
#ifdef ADJUST_SKILL_DAMAGE
	if ((skill_damage = battle_skill_damage(src,target,skill_id)) != 0)
		md.damage += (int64)md.damage * skill_damage / 100;
#endif

	battle_absorb_damage(target, &md);

	battle_do_reflect(BF_MISC,&md, src, target, skill_id, skill_lv); //WIP [lighta]

	return md;
}

/*==========================================
 * Battle main entry, from skill_attack
 *------------------------------------------
 * Credits:
 *	Original coder unknown
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
struct Damage battle_calc_attack(int attack_type,struct block_list *bl,struct block_list *target,uint16 skill_id,uint16 skill_lv,int flag)
{
	struct Damage d;

	switch(attack_type) {
		case BF_WEAPON: d = battle_calc_weapon_attack(bl,target,skill_id,skill_lv,flag); break;
		case BF_MAGIC:  d = battle_calc_magic_attack(bl,target,skill_id,skill_lv,flag);  break;
		case BF_MISC:   d = battle_calc_misc_attack(bl,target,skill_id,skill_lv,flag);   break;
		default:
			ShowError("battle_calc_attack: unknown attack type! %d (skill_id=%d, skill_lv=%d)\n", attack_type, skill_id, skill_lv);
			memset(&d,0,sizeof(d));
			break;
		}
	if( d.damage + d.damage2 < 1 )
	{	//Miss/Absorbed
		//Weapon attacks should go through to cause additional effects.
		if (d.dmg_lv == ATK_DEF /*&& attack_type&(BF_MAGIC|BF_MISC)*/) // Isn't it that additional effects don't apply if miss?
			d.dmg_lv = ATK_MISS;
		d.dmotion = 0;
	}
	else // Some skills like Weaponry Research will cause damage even if attack is dodged
		d.dmg_lv = ATK_DEF;
	return d;
}

/*==========================================
 * Final damage return function
 *------------------------------------------
 * Credits:
 *	Original coder unknown
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
int64 battle_calc_return_damage(struct block_list* bl, struct block_list *src, int64 *dmg, int flag, uint16 skill_id, bool status_reflect){
	struct map_session_data* sd;
	int64 rdamage = 0, damage = *dmg;
	int max_damage = status_get_max_hp(bl);
	struct status_change *sc, *ssc;

	sd = BL_CAST(BL_PC, bl);
	sc = status_get_sc(bl);
	ssc = status_get_sc(src);

	if (flag & BF_SHORT) {//Bounces back part of the damage.
		if ( !status_reflect && sd && sd->bonus.short_weapon_damage_return ) {
			rdamage += damage * sd->bonus.short_weapon_damage_return / 100;
			rdamage = max(rdamage,1);
		} else if( status_reflect && sc && sc->count ) {
			if( sc->data[SC_REFLECTSHIELD] ) {
				struct status_change_entry *sce_d;
				struct block_list *d_bl = NULL;

				if( (sce_d = sc->data[SC_DEVOTION]) && (d_bl = map_id2bl(sce_d->val1)) &&
					((d_bl->type == BL_MER && ((TBL_MER*)d_bl)->master && ((TBL_MER*)d_bl)->master->bl.id == bl->id) ||
					(d_bl->type == BL_PC && ((TBL_PC*)d_bl)->devotion[sce_d->val2] == bl->id)) )
				{ //Don't reflect non-skill attack if has SC_REFLECTSHIELD from Devotion bonus inheritance
					if( (!skill_id && battle_config.devotion_rdamage_skill_only && sc->data[SC_REFLECTSHIELD]->val4) ||
						!check_distance_bl(bl,d_bl,sce_d->val3) )
						return 0;
				}
			}
			if( sc->data[SC_REFLECTDAMAGE] && !(skill_get_inf2(skill_id)&INF2_TRAP)) {
				if( rnd()%100 <= sc->data[SC_REFLECTDAMAGE]->val1*10 + 30 ){
					max_damage = (int64)max_damage * status_get_lv(bl) / 100;
					rdamage = (*dmg) * sc->data[SC_REFLECTDAMAGE]->val2 / 100;
					if( --(sc->data[SC_REFLECTDAMAGE]->val3) < 1)
						status_change_end(bl,SC_REFLECTDAMAGE,INVALID_TIMER);
				}
			} else {
				if ( sc->data[SC_REFLECTSHIELD] && skill_id != WS_CARTTERMINATION ) {
					// Don't reflect non-skill attack if has SC_REFLECTSHIELD from Devotion bonus inheritance
					if (!skill_id && battle_config.devotion_rdamage_skill_only && sc->data[SC_REFLECTSHIELD]->val4)
						rdamage = 0;
					else {
						rdamage += damage * sc->data[SC_REFLECTSHIELD]->val2 / 100;
						if (rdamage < 1)
							rdamage = 1;
					}
				}

				if(sc->data[SC_DEATHBOUND] && skill_id != WS_CARTTERMINATION && !(src->type == BL_MOB && is_boss(src)) ) {
					uint8 dir = map_calc_dir(bl,src->x,src->y),
					t_dir = unit_getdir(bl);

					if( distance_bl(src,bl) <= 0 || !map_check_dir(dir,t_dir) ) {
						int64 rd1 = 0;
						rd1 = min(damage,status_get_max_hp(bl)) * sc->data[SC_DEATHBOUND]->val2 / 100; // Amplify damage.
						*dmg = rd1 * 30 / 100; // Received damage = 30% of amplified damage.
						clif_skill_damage(src,bl,gettick(), status_get_amotion(src), 0, -30000, 1, RK_DEATHBOUND, sc->data[SC_DEATHBOUND]->val1,6);
						status_change_end(bl,SC_DEATHBOUND,INVALID_TIMER);
						rdamage += rd1 * 70 / 100; // Target receives 70% of the amplified damage. [Rytech]
					}
				}

				if( sc->data[SC_SHIELDSPELL_DEF] && sc->data[SC_SHIELDSPELL_DEF]->val1 == 2 && !(src->type == BL_MOB && is_boss(src)) ){
						rdamage += damage * sc->data[SC_SHIELDSPELL_DEF]->val2 / 100;
						if (rdamage < 1) rdamage = 1;
				}
			}
		}
	} else {
		if (!status_reflect && sd && sd->bonus.long_weapon_damage_return) {
			rdamage += damage * sd->bonus.long_weapon_damage_return / 100;
			if (rdamage < 1) rdamage = 1;
		}
	}

	if (ssc && ssc->data[SC_INSPIRATION]) {
		rdamage += damage / 100;
#ifdef RENEWAL
		rdamage = cap_value(rdamage, 1, max_damage);
#else
		rdamage = max(rdamage,1);
#endif
	}

	if( sc && sc->data[SC_KYOMU] ) // Nullify reflecting ability
		rdamage = 0;

	return cap_value(min(rdamage,max_damage),INT_MIN,INT_MAX);
}

/*===========================================
 * Perform battle drain effects (HP/SP loss)
 *-------------------------------------------*/
void battle_drain(struct map_session_data *sd, struct block_list *tbl, int64 rdamage, int64 ldamage, int race, int class_, bool infdef)
{
	struct weapon_data *wd;
	int64 *damage;
	int thp = 0, // HP gained
		tsp = 0, // SP gained
		//rhp = 0, // HP reduced from target
		//rsp = 0, // SP reduced from target
		hp = 0, sp = 0;
	uint8 i = 0;
	short vrate_hp = 0, vrate_sp = 0, v_hp = 0, v_sp = 0;

	if (!CHK_RACE(race) && !CHK_CLASS(class_))
		return;

	// Check for vanish HP/SP. !CHECKME: Which first, drain or vanish?
	hp = (sd->bonus.hp_vanish_rate*10) + sd->hp_vanish_race[race].rate + sd->hp_vanish_race[RC_ALL].rate;
	vrate_hp = cap_value(hp, 0, SHRT_MAX);
	hp = sd->bonus.hp_vanish_per + sd->hp_vanish_race[race].per + sd->hp_vanish_race[RC_ALL].per;
	v_hp = cap_value(hp, SHRT_MIN, SHRT_MAX);

	sp = (sd->bonus.sp_vanish_rate*10) + sd->sp_vanish_race[race].rate + sd->sp_vanish_race[RC_ALL].rate;
	vrate_sp = cap_value(sp, 0, SHRT_MAX);
	sp = sd->bonus.sp_vanish_per + sd->sp_vanish_race[race].per + sd->sp_vanish_race[RC_ALL].per;
	v_sp = cap_value(sp, INT8_MIN, INT8_MAX);

	if (v_hp > 0 && vrate_hp > 0 && (vrate_hp >= 10000 || rnd()%10000 < vrate_hp))
		i |= 1;
	if (v_sp > 0 && vrate_sp > 0 && (vrate_sp >= 10000 || rnd()%10000 < vrate_sp))
		i |= 2;
	if (i) {
		if (infdef)
			status_zap(tbl, v_hp ? v_hp/100 : 0, v_sp ? v_sp/100 : 0);
		else
			status_percent_damage(&sd->bl, tbl, (i&1 ? (int8)(-v_hp): 0), (i&2 ? (int8)(-v_sp) : 0), false);
	}

	// Check for drain HP/SP
	hp = sp = i = 0;
	for (i = 0; i < 4; i++) {
		//First two iterations: Right hand
		if (i < 2) {
			wd = &sd->right_weapon;
			damage = &rdamage;
		}
		else {
			wd = &sd->left_weapon;
			damage = &ldamage;
		}

		if (*damage <= 0)
			continue;

		if( i == 1 || i == 3 ) {
			hp = wd->hp_drain_class[class_] + wd->hp_drain_class[CLASS_ALL];
			hp += battle_calc_drain(*damage, wd->hp_drain_rate.rate, wd->hp_drain_rate.per);

			sp = wd->sp_drain_class[class_] + wd->sp_drain_class[CLASS_ALL];
			sp += battle_calc_drain(*damage, wd->sp_drain_rate.rate, wd->sp_drain_rate.per);

			if( hp ) {
				//rhp += hp;
				thp += hp;
			}

			if( sp ) {
				//rsp += sp;
				tsp += sp;
			}
		} else {
			hp = wd->hp_drain_race[race] + wd->hp_drain_race[RC_ALL];
			sp = wd->sp_drain_race[race] + wd->sp_drain_race[RC_ALL];

			if( hp ) {
				//rhp += hp;
				thp += hp;
			}

			if( sp ) {
				//rsp += sp;
				tsp += sp;
			}
		}
	}

	if (!thp && !tsp)
		return;

	status_heal(&sd->bl, thp, tsp, battle_config.show_hp_sp_drain?3:1);

	//if (rhp || rsp)
	//	status_zap(tbl, rhp, rsp);
}
/*===========================================
 * Deals the same damage to targets in area.
 *-------------------------------------------
 * Credits:
 *	Original coder pakpil
 */
int battle_damage_area(struct block_list *bl, va_list ap) {
	unsigned int tick;
	int64 damage;
	int amotion, dmotion, flag;
	struct block_list *src;

	nullpo_ret(bl);

	tick = va_arg(ap, unsigned int);
	src = va_arg(ap,struct block_list *);
	amotion = va_arg(ap,int);
	dmotion = va_arg(ap,int);
	damage = va_arg(ap,int);
	flag = va_arg(ap,int);

	if( bl->type == BL_MOB && ((TBL_MOB*)bl)->mob_id == MOBID_EMPERIUM )
		return 0;
	if( bl != src && battle_check_target(src,bl,BCT_ENEMY) > 0 ) {
		map_freeblock_lock();
		if( src->type == BL_PC )
			battle_drain((TBL_PC*)src, bl, damage, damage, status_get_race(bl), status_get_class_(bl), is_infinite_defense(bl,flag));
		if( amotion )
			battle_delay_damage(tick, amotion,src,bl,0,CR_REFLECTSHIELD,0,damage,ATK_DEF,0,true);
		else
			status_fix_damage(src,bl,damage,0);
		clif_damage(bl,bl,tick,amotion,dmotion,damage,1,DMG_ENDURE,0);
		skill_additional_effect(src, bl, CR_REFLECTSHIELD, 1, BF_WEAPON|BF_SHORT|BF_NORMAL,ATK_DEF,tick);
		map_freeblock_unlock();
	}

	return 0;
}
/*==========================================
 * Do a basic physical attack (call through unit_attack_timer)
 *------------------------------------------*/
enum damage_lv battle_weapon_attack(struct block_list* src, struct block_list* target, unsigned int tick, int flag) {
	struct map_session_data *sd = NULL, *tsd = NULL;
	struct status_data *sstatus, *tstatus;
	struct status_change *sc, *tsc;
	int64 damage;
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
			short index = sd->equip_index[EQI_AMMO];
			if (index < 0) {
				if (sd->weapontype1 > W_KATAR || sd->weapontype1 < W_HUUMA)
					clif_skill_fail(sd,0,USESKILL_FAIL_NEED_MORE_BULLET,0);
				else
					clif_arrow_fail(sd,0);
				return ATK_NONE;
			}
			//Ammo check by Ishizu-chan
			if (sd->inventory_data[index]) {
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
							clif_skill_fail(sd,0,USESKILL_FAIL_NEED_MORE_BULLET,0);
							return ATK_NONE;
						}
						break;
					case W_GRENADE:
						if (sd->inventory_data[index]->look != A_GRENADE) {
							clif_skill_fail(sd,0,USESKILL_FAIL_NEED_MORE_BULLET,0);
							return ATK_NONE;
						}
						break;
				}
			}
		}
	}
	if (sc && sc->count) {
		if (sc->data[SC_CLOAKING] && !(sc->data[SC_CLOAKING]->val4 & 2))
			status_change_end(src, SC_CLOAKING, INVALID_TIMER);
		else if (sc->data[SC_CLOAKINGEXCEED] && !(sc->data[SC_CLOAKINGEXCEED]->val4 & 2))
			status_change_end(src, SC_CLOAKINGEXCEED, INVALID_TIMER);
	}
	if (tsc && tsc->data[SC_AUTOCOUNTER] && status_check_skilluse(target, src, KN_AUTOCOUNTER, 1)) {
		uint8 dir = map_calc_dir(target,src->x,src->y);
		int t_dir = unit_getdir(target);
		int dist = distance_bl(src, target);

		if (dist <= 0 || (!map_check_dir(dir,t_dir) && dist <= tstatus->rhw.range+1)) {
			uint16 skill_lv = tsc->data[SC_AUTOCOUNTER]->val1;

			clif_skillcastcancel(target); //Remove the casting bar. [Skotlex]
			clif_damage(src, target, tick, sstatus->amotion, 1, 0, 1, DMG_NORMAL, 0); //Display MISS.
			status_change_end(target, SC_AUTOCOUNTER, INVALID_TIMER);
			skill_attack(BF_WEAPON,target,target,src,KN_AUTOCOUNTER,skill_lv,tick,0);
			return ATK_BLOCK;
		}
	}

	if( tsc && tsc->data[SC_BLADESTOP_WAIT] && !is_boss(src) && (src->type == BL_PC || tsd == NULL || distance_bl(src, target) <= (tsd->status.weapon == W_FIST ? 1 : 2)) )
	{
		uint16 skill_lv = tsc->data[SC_BLADESTOP_WAIT]->val1;
		int duration = skill_get_time2(MO_BLADESTOP,skill_lv);
		status_change_end(target, SC_BLADESTOP_WAIT, INVALID_TIMER);
		if(sc_start4(src,src, SC_BLADESTOP, 100, sd?pc_checkskill(sd, MO_BLADESTOP):5, 0, 0, target->id, duration))
		{	//Target locked.
			clif_damage(src, target, tick, sstatus->amotion, 1, 0, 1, DMG_NORMAL, 0); //Display MISS.
			clif_bladestop(target, src->id, 1);
			sc_start4(src,target, SC_BLADESTOP, 100, skill_lv, 0, 0, src->id, duration);
			return ATK_BLOCK;
		}
	}

	if(sd && (skillv = pc_checkskill(sd,MO_TRIPLEATTACK)) > 0) {
		int triple_rate= 30 - skillv; //Base Rate
		if (sc && sc->data[SC_SKILLRATE_UP] && sc->data[SC_SKILLRATE_UP]->val1 == MO_TRIPLEATTACK) {
			triple_rate+= triple_rate*(sc->data[SC_SKILLRATE_UP]->val2)/100;
			status_change_end(src, SC_SKILLRATE_UP, INVALID_TIMER);
		}
		if (rnd()%100 < triple_rate) {
			//Need to apply canact_tick here because it doesn't go through skill_castend_id
			sd->ud.canact_tick = tick + skill_delayfix(src, MO_TRIPLEATTACK, skillv);
			if( skill_attack(BF_WEAPON,src,src,target,MO_TRIPLEATTACK,skillv,tick,0) )
				return ATK_DEF;
			return ATK_MISS;
		}
	}

	if (sc) {
		if (sc->data[SC_SACRIFICE]) {
			uint16 skill_lv = sc->data[SC_SACRIFICE]->val1;
			damage_lv ret_val;

			if( --sc->data[SC_SACRIFICE]->val2 <= 0 )
				status_change_end(src, SC_SACRIFICE, INVALID_TIMER);

			/**
			 * We need to calculate the DMG before the hp reduction, because it can kill the source.
			 * For further information: bugreport:4950
			 */
			ret_val = (damage_lv)skill_attack(BF_WEAPON,src,src,target,PA_SACRIFICE,skill_lv,tick,0);

			status_zap(src, sstatus->max_hp*9/100, 0);//Damage to self is always 9%
			if( ret_val == ATK_NONE )
				return ATK_MISS;
			return ret_val;
		}
		if (sc->data[SC_MAGICALATTACK]) {
			if( skill_attack(BF_MAGIC,src,src,target,NPC_MAGICALATTACK,sc->data[SC_MAGICALATTACK]->val1,tick,0) )
				return ATK_DEF;
			return ATK_MISS;
		}
		if( sc->data[SC_GT_ENERGYGAIN] ) {
			int spheres = 5;

			if( sc->data[SC_RAISINGDRAGON] )
				spheres += sc->data[SC_RAISINGDRAGON]->val1;

			if( sd && rnd()%100 < sc->data[SC_GT_ENERGYGAIN]->val2 )
				pc_addspiritball(sd, skill_get_time2(SR_GENTLETOUCH_ENERGYGAIN, sc->data[SC_GT_ENERGYGAIN]->val1), spheres);
		}
	}

	if( tsc && tsc->data[SC_GT_ENERGYGAIN] ) {
		int spheres = 5;

		if( tsc->data[SC_RAISINGDRAGON] )
			spheres += tsc->data[SC_RAISINGDRAGON]->val1;

		if( tsd && rnd()%100 < tsc->data[SC_GT_ENERGYGAIN]->val2 )
			pc_addspiritball(tsd, skill_get_time2(SR_GENTLETOUCH_ENERGYGAIN, tsc->data[SC_GT_ENERGYGAIN]->val1), spheres);
	}

	if (tsc && tsc->data[SC_MTF_MLEATKED] && rnd()%100 < tsc->data[SC_MTF_MLEATKED]->val2)
		clif_skill_nodamage(target, target, SM_ENDURE, tsc->data[SC_MTF_MLEATKED]->val1, sc_start(src, target, SC_ENDURE, 100, tsc->data[SC_MTF_MLEATKED]->val1, skill_get_time(SM_ENDURE, tsc->data[SC_MTF_MLEATKED]->val1)));

	if(tsc && tsc->data[SC_KAAHI] && tstatus->hp < tstatus->max_hp && status_charge(target, 0, tsc->data[SC_KAAHI]->val3)) {
		int hp_heal = tstatus->max_hp - tstatus->hp;
		if (hp_heal > tsc->data[SC_KAAHI]->val2)
			hp_heal = tsc->data[SC_KAAHI]->val2;
		if (hp_heal)
			status_heal(target, hp_heal, 0, 2);
	}

	wd = battle_calc_attack(BF_WEAPON, src, target, 0, 0, flag);

	if( sc && sc->count ) {
		if (sc->data[SC_EXEEDBREAK]) {
			wd.damage *= sc->data[SC_EXEEDBREAK]->val1 / 100;
			status_change_end(src, SC_EXEEDBREAK, INVALID_TIMER);
		}
		if( sc->data[SC_SPELLFIST] ) {
			if( --(sc->data[SC_SPELLFIST]->val1) >= 0 ){
				struct Damage ad = battle_calc_attack(BF_MAGIC,src,target,sc->data[SC_SPELLFIST]->val3,sc->data[SC_SPELLFIST]->val4,flag|BF_SHORT);
				wd.damage = ad.damage;
				if (wd.div_ > 1)
					wd.damage *= 2; // Double the damage for multiple hits.
			} else
				status_change_end(src,SC_SPELLFIST,INVALID_TIMER);
		}
		if( sc->data[SC_GIANTGROWTH] && (wd.flag&BF_SHORT) && rnd()%100 < sc->data[SC_GIANTGROWTH]->val2 )
			wd.damage *= 3; // Triple Damage

		if( sd && battle_config.arrow_decrement && sc->data[SC_FEARBREEZE] && sc->data[SC_FEARBREEZE]->val4 > 0) {
			short idx = sd->equip_index[EQI_AMMO];
			if (idx >= 0 && sd->status.inventory[idx].amount >= sc->data[SC_FEARBREEZE]->val4) {
				pc_delitem(sd,idx,sc->data[SC_FEARBREEZE]->val4,0,1,LOG_TYPE_CONSUME);
				sc->data[SC_FEARBREEZE]->val4 = 0;
			}
		}
	}
	if (sd && sd->state.arrow_atk) //Consume arrow.
		battle_consume_ammo(sd, 0, 0);

	damage = wd.damage + wd.damage2;
	if( damage > 0 && src != target )
	{
		if( sc && sc->data[SC_DUPLELIGHT] && (wd.flag&BF_SHORT) && rnd()%100 <= 10+2*sc->data[SC_DUPLELIGHT]->val1 )
		{	// Activates it only from melee damage
			uint16 skill_id;
			if( rnd()%2 == 1 )
				skill_id = AB_DUPLELIGHT_MELEE;
			else
				skill_id = AB_DUPLELIGHT_MAGIC;
			skill_attack(skill_get_type(skill_id), src, src, target, skill_id, sc->data[SC_DUPLELIGHT]->val1, tick, SD_LEVEL);
		}
	}

	wd.dmotion = clif_damage(src, target, tick, wd.amotion, wd.dmotion, wd.damage, wd.div_ , (enum e_damage_type)wd.type, wd.damage2);

	if (sd && sd->bonus.splash_range > 0 && damage > 0)
		skill_castend_damage_id(src, target, 0, 1, tick, 0);
	if ( target->type == BL_SKILL && damage > 0 ) {
		TBL_SKILL *su = (TBL_SKILL*)target;

		if (su && su->group) {
			if (su->group->skill_id == HT_BLASTMINE)
				skill_blown(src, target, 3, -1, 0);
			if (su->group->skill_id == GN_WALLOFTHORN) {
				if (--su->val2 <= 0)
					skill_delunit(su);
			}
		}
	}

	map_freeblock_lock();

	if( skill_check_shadowform(target, damage, wd.div_) ) {
		if( !status_isdead(target) )
			skill_additional_effect(src, target, 0, 0, wd.flag, wd.dmg_lv, tick);
		if( wd.dmg_lv > ATK_BLOCK )
			skill_counter_additional_effect(src, target, 0, 0, wd.flag, tick);
	} else
		battle_delay_damage(tick, wd.amotion, src, target, wd.flag, 0, 0, damage, wd.dmg_lv, wd.dmotion, true);
	if( tsc ) {
		if( tsc->data[SC_DEVOTION] ) {
			struct status_change_entry *sce = tsc->data[SC_DEVOTION];
			struct block_list *d_bl = map_id2bl(sce->val1);

			if( d_bl && (
				(d_bl->type == BL_MER && ((TBL_MER*)d_bl)->master && ((TBL_MER*)d_bl)->master->bl.id == target->id) ||
				(d_bl->type == BL_PC && ((TBL_PC*)d_bl)->devotion[sce->val2] == target->id)
				) && check_distance_bl(target, d_bl, sce->val3) )
			{
				clif_damage(d_bl, d_bl, gettick(), 0, 0, damage, 0, DMG_NORMAL, 0);
				status_fix_damage(NULL, d_bl, damage, 0);
			}
			else
				status_change_end(target, SC_DEVOTION, INVALID_TIMER);
		}
		if( target->type == BL_PC && (wd.flag&BF_SHORT) && tsc->data[SC_CIRCLE_OF_FIRE_OPTION] ) {
			struct elemental_data *ed = ((TBL_PC*)target)->ed;
			if( ed ) {
				clif_skill_damage(&ed->bl, target, tick, status_get_amotion(src), 0, -30000, 1, EL_CIRCLE_OF_FIRE, tsc->data[SC_CIRCLE_OF_FIRE_OPTION]->val1, 6);
				skill_attack(BF_WEAPON,&ed->bl,&ed->bl,src,EL_CIRCLE_OF_FIRE,tsc->data[SC_CIRCLE_OF_FIRE_OPTION]->val1,tick,wd.flag);
			}
		}
		if( tsc->data[SC_WATER_SCREEN_OPTION] && tsc->data[SC_WATER_SCREEN_OPTION]->val1 ) {
			struct block_list *e_bl = map_id2bl(tsc->data[SC_WATER_SCREEN_OPTION]->val1);
			if( e_bl && !status_isdead(e_bl) ) {
				clif_damage(e_bl,e_bl,tick,wd.amotion,wd.dmotion,damage,wd.div_,(enum e_damage_type)wd.type,wd.damage2);
				status_damage(target,e_bl,damage,0,0,0);
				// Just show damage in target.
				clif_damage(src, target, tick, wd.amotion, wd.dmotion, damage, wd.div_, (enum e_damage_type)wd.type, wd.damage2 );
				map_freeblock_unlock();
				return ATK_BLOCK;
			}
		}
	}
	if (sc && sc->data[SC_AUTOSPELL] && rnd()%100 < sc->data[SC_AUTOSPELL]->val4) {
		int sp = 0;
		uint16 skill_id = sc->data[SC_AUTOSPELL]->val2;
		uint16 skill_lv = sc->data[SC_AUTOSPELL]->val3;
		int i = rnd()%100;
		if (sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_SAGE)
			i = 0; //Max chance, no skill_lv reduction. [Skotlex]
		//reduction only for skill_lv > 1
		if (skill_lv > 1) {
			if (i >= 50) skill_lv /= 2;
			else if (i >= 15) skill_lv--;
		}
		sp = skill_get_sp(skill_id,skill_lv) * 2 / 3;

		if (status_charge(src, 0, sp)) {
			switch (skill_get_casttype(skill_id)) {
				case CAST_GROUND:
					skill_castend_pos2(src, target->x, target->y, skill_id, skill_lv, tick, flag);
					break;
				case CAST_NODAMAGE:
					skill_castend_nodamage_id(src, target, skill_id, skill_lv, tick, flag);
					break;
				case CAST_DAMAGE:
					skill_castend_damage_id(src, target, skill_id, skill_lv, tick, flag);
					break;
			}
		}
	}
	if (sd) {
		uint16 r_skill = 0, sk_idx = 0;
		if( wd.flag&BF_SHORT && sc && sc->data[SC__AUTOSHADOWSPELL] && rnd()%100 < sc->data[SC__AUTOSHADOWSPELL]->val3 &&
			(r_skill = (uint16)sc->data[SC__AUTOSHADOWSPELL]->val1) && (sk_idx = skill_get_index(r_skill)) &&
			sd->status.skill[sk_idx].id != 0 && sd->status.skill[sk_idx].flag == SKILL_FLAG_PLAGIARIZED )
		{
			int r_lv = sc->data[SC__AUTOSHADOWSPELL]->val2;

			if (r_skill != AL_HOLYLIGHT && r_skill != PR_MAGNUS) {
				int type;
				if( (type = skill_get_casttype(r_skill)) == CAST_GROUND ) {
					int maxcount = 0;

					if( !(BL_PC&battle_config.skill_reiteration) &&
						skill_get_unit_flag(r_skill)&UF_NOREITERATION )
							type = -1;

					if( BL_PC&battle_config.skill_nofootset &&
						skill_get_unit_flag(r_skill)&UF_NOFOOTSET )
							type = -1;

					if( BL_PC&battle_config.land_skill_limit &&
						(maxcount = skill_get_maxcount(r_skill, r_lv)) > 0
					  ) {
						int v;
						for(v=0;v<MAX_SKILLUNITGROUP && sd->ud.skillunit[v] && maxcount;v++) {
							if(sd->ud.skillunit[v]->skill_id == r_skill)
								maxcount--;
						}
						if( maxcount == 0 )
							type = -1;
					}

					if( type != CAST_GROUND ){
						clif_skill_fail(sd,r_skill,USESKILL_FAIL_LEVEL,0);
						map_freeblock_unlock();
						return wd.dmg_lv;
					}
				}

				if (sd->state.autocast == 0) {
					sd->state.autocast = 1;
					skill_consume_requirement(sd, r_skill, r_lv, 3);
					switch (type) {
						case CAST_GROUND:
							skill_castend_pos2(src, target->x, target->y, r_skill, r_lv, tick, flag);
							break;
						case CAST_NODAMAGE:
							skill_castend_nodamage_id(src, target, r_skill, r_lv, tick, flag);
							break;
						case CAST_DAMAGE:
							skill_castend_damage_id(src, target, r_skill, r_lv, tick, flag);
							break;
					}
				}
				sd->state.autocast = 0;

				sd->ud.canact_tick = tick + skill_delayfix(src, r_skill, r_lv);
				clif_status_change(src, SI_ACTIONDELAY, 1, skill_delayfix(src, r_skill, r_lv), 0, 0, 1);
			}
		}

		if (wd.flag & BF_WEAPON && src != target && damage > 0) {
			if (battle_config.left_cardfix_to_right)
				battle_drain(sd, target, wd.damage, wd.damage, tstatus->race, tstatus->class_, is_infinite_defense(target,wd.flag));
			else
				battle_drain(sd, target, wd.damage, wd.damage2, tstatus->race, tstatus->class_, is_infinite_defense(target,wd.flag));
		}
	}

	if (tsc) {
		if (damage > 0 && tsc->data[SC_POISONREACT] &&
			(rnd()%100 < tsc->data[SC_POISONREACT]->val3
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
				status_change_end(target, SC_POISONREACT, INVALID_TIMER);
		}
	}
	map_freeblock_unlock();
	return wd.dmg_lv;
}

/*=========================
 * Check for undead status
 *-------------------------
 * Credits:
 *	Original coder Skotlex
 *  Refactored by Baalberith
 */
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

/*================================================================
 * Returns the upmost level master starting with the given object
 *----------------------------------------------------------------*/
struct block_list* battle_get_master(struct block_list *src)
{
	struct block_list *prev; //Used for infinite loop check (master of yourself?)
	do {
		prev = src;
		switch (src->type) {
			case BL_PET:
				if (((TBL_PET*)src)->master)
					src = (struct block_list*)((TBL_PET*)src)->master;
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
			case BL_ELEM:
				if (((TBL_ELEM*)src)->master)
					src = (struct block_list*)((TBL_ELEM*)src)->master;
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
 * Checks the state between two targets
 * (enemy, friend, party, guild, etc)
 *------------------------------------------
 * Usage:
 * See battle.h for possible values/combinations
 * to be used here (BCT_* constants)
 * Return value is:
 * 1: flag holds true (is enemy, party, etc)
 * -1: flag fails
 * 0: Invalid target (non-targetable ever)
 *
 * Credits:
 *	Original coder unknown
 *	Rewritten by Skotlex
*/
int battle_check_target( struct block_list *src, struct block_list *target,int flag)
{
	int16 m; //map
	int state = 0; //Initial state none
	int strip_enemy = 1; //Flag which marks whether to remove the BCT_ENEMY status if it's also friend/ally.
	struct block_list *s_bl = src, *t_bl = target;
	struct unit_data *ud = NULL;

	nullpo_ret(src);
	nullpo_ret(target);

	ud = unit_bl2ud(target);
	m = target->m;

	//t_bl/s_bl hold the 'master' of the attack, while src/target are the actual
	//objects involved.
	if( (t_bl = battle_get_master(target)) == NULL )
		t_bl = target;

	if( (s_bl = battle_get_master(src)) == NULL )
		s_bl = src;

	if ( s_bl->type == BL_PC ) {
		switch( t_bl->type ) {
			case BL_MOB: // Source => PC, Target => MOB
				if ( pc_has_permission((TBL_PC*)s_bl, PC_PERM_DISABLE_PVM) )
					return 0;
				break;
			case BL_PC:
				if (pc_has_permission((TBL_PC*)s_bl, PC_PERM_DISABLE_PVP))
					return 0;
				break;
			default:/* anything else goes */
				break;
		}
	}

	switch( target->type ) { // Checks on actual target
		case BL_PC: {
				struct status_change* sc = status_get_sc(src);

				if (((TBL_PC*)target)->invincible_timer != INVALID_TIMER || pc_isinvisible((TBL_PC*)target))
					return -1; //Cannot be targeted yet.
				if( sc && sc->count ) {
					if( sc->data[SC_VOICEOFSIREN] && sc->data[SC_VOICEOFSIREN]->val2 == target->id )
						return -1;
				}
			}
			break;
		case BL_MOB:
		{
			struct mob_data *md = ((TBL_MOB*)target);

			if (ud && ud->immune_attack)
				return 0;
			if(((md->special_state.ai == AI_SPHERE || //Marine Spheres
				(md->special_state.ai == AI_FLORA && battle_config.summon_flora&1)) && s_bl->type == BL_PC && src->type != BL_MOB) || //Floras
				(md->special_state.ai == AI_ZANZOU && t_bl->id != s_bl->id) || //Zanzou
				(md->special_state.ai == AI_FAW && (t_bl->id != s_bl->id || (s_bl->type == BL_PC && src->type != BL_MOB)))
			){	//Targettable by players
				state |= BCT_ENEMY;
				strip_enemy = 0;
			}
			break;
		}
		case BL_SKILL:
		{
			TBL_SKILL *su = (TBL_SKILL*)target;
			if( !su || !su->group)
				return 0;
			if( skill_get_inf2(su->group->skill_id)&INF2_TRAP && su->group->unit_id != UNT_USED_TRAPS) {
				uint16 skill_id = battle_getcurrentskill(src);

				if (!skill_id || su->group->skill_id == WM_REVERBERATION || su->group->skill_id == WM_POEMOFNETHERWORLD) {
					;
				}
				else if (skill_get_inf2(skill_id)&INF2_HIT_TRAP) { // Only a few skills can target traps
					switch (skill_id) {
						case RK_DRAGONBREATH:
						case RK_DRAGONBREATH_WATER:
							// Can only hit traps in PVP/GVG maps
							if( !map[m].flag.pvp && !map[m].flag.gvg )
								return 0;
					}
				}
				else
					return 0;
				state |= BCT_ENEMY;
				strip_enemy = 0;
			} else if (su->group->skill_id == WZ_ICEWALL || su->group->skill_id == GN_WALLOFTHORN) {
				state |= BCT_ENEMY;
				strip_enemy = 0;
			} else	//Excepting traps, Icewall, and Wall of Thorns, you should not be able to target skills.
				return 0;
		}
			break;
		case BL_MER:
		case BL_HOM:
		case BL_ELEM:
			if (ud && ud->immune_attack)
				return 0;
			break;
		//All else not specified is an invalid target.
		default:
			return 0;
	} //end switch actual target

	switch( t_bl->type ) { //Checks on target master
		case BL_PC: {
			struct map_session_data *sd;
			struct status_change *sc = NULL;

			if( t_bl == s_bl )
				break;

			sd = BL_CAST(BL_PC, t_bl);
			sc = status_get_sc(t_bl);

			if( (sd->state.monster_ignore || (sc->data[SC_KINGS_GRACE] && s_bl->type != BL_PC)) && flag&BCT_ENEMY )
				return 0; // Global immunity only to Attacks
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
		default: break; //other type doesn't have slave yet
    } //end switch master target

	switch( src->type ) { //Checks on actual src type
		case BL_PET:
			if (t_bl->type != BL_MOB && flag&BCT_ENEMY)
				return 0; //Pet may not attack non-mobs.
			if (t_bl->type == BL_MOB && ((TBL_MOB*)t_bl)->guardian_data && flag&BCT_ENEMY)
				return 0; //pet may not attack Guardians/Emperium
			break;
		case BL_SKILL: {
				struct skill_unit *su = (struct skill_unit *)src;
				struct status_change* sc = status_get_sc(target);
				if (!su || !su->group)
					return 0;
				if (su->group->src_id == target->id) {
					int inf2 = skill_get_inf2(su->group->skill_id);
					if (inf2&INF2_NO_TARGET_SELF)
						return -1;
					if (inf2&INF2_TARGET_SELF)
						return 1;
				}
				//Status changes that prevent traps from triggering
				if (sc && sc->count && skill_get_inf2(su->group->skill_id)&INF2_TRAP) {
					if( sc->data[SC_SIGHTBLASTER] && sc->data[SC_SIGHTBLASTER]->val2 > 0 && sc->data[SC_SIGHTBLASTER]->val4%2 == 0)
						return -1;
				}
			}
			break;
		case BL_MER:
			if (t_bl->type == BL_MOB && ((TBL_MOB*)t_bl)->mob_id == MOBID_EMPERIUM && flag&BCT_ENEMY)
				return 0; //mercenary may not attack Emperium
			break;
    } //end switch actual src

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
			if( map_flag_gvg(m) && !sd->status.guild_id && t_bl->type == BL_MOB && ((TBL_MOB*)t_bl)->mob_id == MOBID_EMPERIUM )
				return 0; //If you don't belong to a guild, can't target emperium.
			if( t_bl->type != BL_PC )
				state |= BCT_ENEMY; //Natural enemy.
			break;
		}
		case BL_MOB:
		{
			struct mob_data *md = BL_CAST(BL_MOB, s_bl);
			if( !((agit_flag || agit2_flag) && map[m].flag.gvg_castle) && md->guardian_data && md->guardian_data->guild_id )
				return 0; // Disable guardians/emperium owned by Guilds on non-woe times.

			if( !md->special_state.ai )
			{ //Normal mobs
				if(
					( target->type == BL_MOB && t_bl->type == BL_PC && ( ((TBL_MOB*)target)->special_state.ai != AI_ZANZOU && ((TBL_MOB*)target)->special_state.ai != AI_ATTACK ) ) ||
					( t_bl->type == BL_MOB && !((TBL_MOB*)t_bl)->special_state.ai )
				  )
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
    } //end switch on src master

	if( (flag&BCT_ALL) == BCT_ALL )
	{ //All actually stands for all attackable chars
		if( target->type&BL_CHAR )
			return 1;
		else
			return -1;
	}
	if( flag == BCT_NOONE ) //Why would someone use this? no clue.
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
		int sbg_id = 0, tbg_id = 0;
		if( map[m].flag.battleground )
		{
			sbg_id = bg_team_get_id(s_bl);
			tbg_id = bg_team_get_id(t_bl);
		}
		if( flag&(BCT_PARTY|BCT_ENEMY) )
		{
			int s_party = status_get_party_id(s_bl);
			if( s_party && s_party == status_get_party_id(t_bl) && !(map[m].flag.pvp && map[m].flag.pvp_noparty) && !(map_flag_gvg(m) && map[m].flag.gvg_noparty) && (!map[m].flag.battleground || sbg_id == tbg_id) )
				state |= BCT_PARTY;
			else
				state |= BCT_ENEMY;
		}
		if( flag&(BCT_GUILD|BCT_ENEMY) )
		{
			int s_guild = status_get_guild_id(s_bl);
			int t_guild = status_get_guild_id(t_bl);
			if( !(map[m].flag.pvp && map[m].flag.pvp_noguild) && s_guild && t_guild && (s_guild == t_guild || (!(flag&BCT_SAMEGUILD) && guild_isallied(s_guild, t_guild))) && (!map[m].flag.battleground || sbg_id == tbg_id) )
				state |= BCT_GUILD;
			else
				state |= BCT_ENEMY;
		}
		if( state&BCT_ENEMY && map[m].flag.battleground && sbg_id && sbg_id == tbg_id )
			state &= ~BCT_ENEMY;

		if( state&BCT_ENEMY && battle_config.pk_mode && !map_flag_gvg(m) && s_bl->type == BL_PC && t_bl->type == BL_PC )
		{ // Prevent novice engagement on pk_mode (feature by Valaris)
			TBL_PC *sd = (TBL_PC*)s_bl, *sd2 = (TBL_PC*)t_bl;
			if (
				(sd->class_&MAPID_UPPERMASK) == MAPID_NOVICE ||
				(sd2->class_&MAPID_UPPERMASK) == MAPID_NOVICE ||
				(int)sd->status.base_level < battle_config.pk_min_level ||
			  	(int)sd2->status.base_level < battle_config.pk_min_level ||
				(battle_config.pk_level_range && abs((int)sd->status.base_level - (int)sd2->status.base_level) > battle_config.pk_level_range)
			)
				state &= ~BCT_ENEMY;
		}
    }//end map_flag_vs chk rivality
	else
	{ //Non pvp/gvg, check party/guild settings.
		if( flag&BCT_PARTY || state&BCT_ENEMY )
		{
			int s_party = status_get_party_id(s_bl);
			if(s_party && s_party == status_get_party_id(t_bl))
				state |= BCT_PARTY;
		}
		if( flag&BCT_GUILD || state&BCT_ENEMY )
		{
			int s_guild = status_get_guild_id(s_bl);
			int t_guild = status_get_guild_id(t_bl);
			if(s_guild && t_guild && (s_guild == t_guild || (!(flag&BCT_SAMEGUILD) && guild_isallied(s_guild, t_guild))))
				state |= BCT_GUILD;
		}
	} //end non pvp/gvg chk rivality

	if( !state ) //If not an enemy, nor a guild, nor party, nor yourself, it's neutral.
		state = BCT_NEUTRAL;
	//Alliance state takes precedence over enemy one.
	else if( state&BCT_ENEMY && strip_enemy && state&(BCT_SELF|BCT_PARTY|BCT_GUILD) )
		state&=~BCT_ENEMY;

	return (flag&state)?1:-1;
}
/*==========================================
 * Check if can attack from this range
 * Basic check then calling path_search for obstacle etc..
 *------------------------------------------
 */
bool battle_check_range(struct block_list *src, struct block_list *bl, int range)
{
	int d;
	nullpo_retr(false, src);
	nullpo_retr(false, bl);

	if( src->m != bl->m )
		return false;

#ifndef CIRCULAR_AREA
	if( src->type == BL_PC ) { // Range for players' attacks and skills should always have a circular check. [Angezerus]
		if ( !check_distance_client_bl(src, bl, range) )
			return false;
	} else
#endif
	if( !check_distance_bl(src, bl, range) )
		return false;

	if( (d = distance_bl(src, bl)) < 2 )
		return true;  // No need for path checking.

	if( d > AREA_SIZE )
		return false; // Avoid targetting objects beyond your range of sight.

	return path_search_long(NULL,src->m,src->x,src->y,bl->x,bl->y,CELL_CHKWALL);
}

/*=============================================
 * Battle.conf settings and default/max values
 *---------------------------------------------
 */
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
	{ "skillrange_from_weapon",             &battle_config.use_weapon_skill_range,          BL_NUL, BL_NUL, BL_ALL,         },
	{ "player_damage_delay_rate",           &battle_config.pc_damage_delay_rate,            100,    0,      INT_MAX,        },
	{ "defunit_not_enemy",                  &battle_config.defnotenemy,                     0,      0,      1,              },
	{ "gvg_traps_target_all",               &battle_config.vs_traps_bctall,                 BL_PC,  BL_NUL, BL_ALL,         },
	{ "traps_setting",                      &battle_config.traps_setting,                   0,      0,      1,              },
	{ "summon_flora_setting",               &battle_config.summon_flora,                    1|2,    0,      1|2,            },
	{ "clear_skills_on_death",              &battle_config.clear_unit_ondeath,              BL_NUL, BL_NUL, BL_ALL,         },
	{ "clear_skills_on_warp",               &battle_config.clear_unit_onwarp,               BL_ALL, BL_NUL, BL_ALL,         },
	{ "random_monster_checklv",             &battle_config.random_monster_checklv,          0,      0,      1,              },
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
	{ "guild_skill_relog_delay",            &battle_config.guild_skill_relog_delay,         300000, 0,      INT_MAX,        },
	{ "emergency_call",                     &battle_config.emergency_call,                  11,     0,      31,             },
	{ "atcommand_spawn_quantity_limit",     &battle_config.atc_spawn_quantity_limit,        100,    0,      INT_MAX,        },
	{ "atcommand_slave_clone_limit",        &battle_config.atc_slave_clone_limit,           25,     0,      INT_MAX,        },
	{ "partial_name_scan",                  &battle_config.partial_name_scan,               0,      0,      1,              },
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
	{ "item_check",                         &battle_config.item_check,                      0x0,    0x0,    0x7,            },
	{ "item_use_interval",                  &battle_config.item_use_interval,               100,    0,      INT_MAX,        },
	{ "cashfood_use_interval",              &battle_config.cashfood_use_interval,           60000,  0,      INT_MAX,        },
	{ "wedding_modifydisplay",              &battle_config.wedding_modifydisplay,           0,      0,      1,              },
	{ "wedding_ignorepalette",              &battle_config.wedding_ignorepalette,           0,      0,      1,              },
	{ "xmas_ignorepalette",                 &battle_config.xmas_ignorepalette,              0,      0,      1,              },
	{ "summer_ignorepalette",               &battle_config.summer_ignorepalette,            0,      0,      1,              },
	{ "hanbok_ignorepalette",               &battle_config.hanbok_ignorepalette,            0,      0,      1,              },
	{ "oktoberfest_ignorepalette",          &battle_config.oktoberfest_ignorepalette,       0,      0,      1,              },
	{ "natural_healhp_interval",            &battle_config.natural_healhp_interval,         6000,   NATURAL_HEAL_INTERVAL, INT_MAX, },
	{ "natural_healsp_interval",            &battle_config.natural_healsp_interval,         8000,   NATURAL_HEAL_INTERVAL, INT_MAX, },
	{ "natural_heal_skill_interval",        &battle_config.natural_heal_skill_interval,     10000,  NATURAL_HEAL_INTERVAL, INT_MAX, },
	{ "natural_heal_weight_rate",           &battle_config.natural_heal_weight_rate,        50,     50,     101             },
	{ "arrow_decrement",                    &battle_config.arrow_decrement,                 1,      0,      2,              },
	{ "max_aspd",                           &battle_config.max_aspd,                        190,    100,    199,            },
	{ "max_third_aspd",                     &battle_config.max_third_aspd,                  193,    100,    199,            },
	{ "max_walk_speed",                     &battle_config.max_walk_speed,                  300,    100,    100*DEFAULT_WALK_SPEED, },
	{ "max_lv",                             &battle_config.max_lv,                          99,     0,      MAX_LEVEL,      },
	{ "aura_lv",                            &battle_config.aura_lv,                         99,     0,      INT_MAX,        },
	{ "max_hp",                             &battle_config.max_hp,                          32500,  100,    1000000000,     },
	{ "max_sp",                             &battle_config.max_sp,                          32500,  100,    1000000000,     },
	{ "max_cart_weight",                    &battle_config.max_cart_weight,                 8000,   100,    1000000,        },
	{ "max_parameter",                      &battle_config.max_parameter,                   99,     10,     SHRT_MAX,       },
	{ "max_baby_parameter",                 &battle_config.max_baby_parameter,              80,     10,     SHRT_MAX,       },
	{ "max_def",                            &battle_config.max_def,                         99,     0,      INT_MAX,        },
	{ "over_def_bonus",                     &battle_config.over_def_bonus,                  0,      0,      1000,           },
	{ "skill_log",                          &battle_config.skill_log,                       BL_NUL, BL_NUL, BL_ALL,         },
	{ "battle_log",                         &battle_config.battle_log,                      0,      0,      1,              },
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
	{ "vit_penalty_target",                 &battle_config.vit_penalty_target,              BL_PC,  BL_NUL, BL_ALL,         },
	{ "vit_penalty_type",                   &battle_config.vit_penalty_type,                1,      0,      2,              },
	{ "vit_penalty_count",                  &battle_config.vit_penalty_count,               3,      2,      INT_MAX,        },
	{ "vit_penalty_num",                    &battle_config.vit_penalty_num,                 5,      0,      INT_MAX,        },
	{ "weapon_defense_type",                &battle_config.weapon_defense_type,             0,      0,      INT_MAX,        },
	{ "magic_defense_type",                 &battle_config.magic_defense_type,              0,      0,      INT_MAX,        },
	{ "skill_reiteration",                  &battle_config.skill_reiteration,               BL_NUL, BL_NUL, BL_ALL,         },
	{ "skill_nofootset",                    &battle_config.skill_nofootset,                 BL_PC,  BL_NUL, BL_ALL,         },
	{ "player_cloak_check_type",            &battle_config.pc_cloak_check_type,             1,      0,      1|2|4,          },
	{ "monster_cloak_check_type",           &battle_config.monster_cloak_check_type,        4,      0,      1|2|4,          },
	{ "sense_type",                         &battle_config.estimation_type,                 1|2,    0,      1|2,            },
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
	{ "show_party_share_picker",            &battle_config.party_show_share_picker,         1,      0,      1,              },
	{ "show_picker.item_type",              &battle_config.show_picker_item_type,           112,    0,      INT_MAX,        },
	{ "party_update_interval",              &battle_config.party_update_interval,           1000,   100,    INT_MAX,        },
	{ "party_item_share_type",              &battle_config.party_share_type,                0,      0,      1|2|3,          },
	{ "attack_attr_none",                   &battle_config.attack_attr_none,                ~BL_PC, BL_NUL, BL_ALL,         },
	{ "gx_allhit",                          &battle_config.gx_allhit,                       0,      0,      1,              },
	{ "gx_disptype",                        &battle_config.gx_disptype,                     1,      0,      1,              },
	{ "devotion_level_difference",          &battle_config.devotion_level_difference,       10,     0,      INT_MAX,        },
	{ "player_skill_partner_check",         &battle_config.player_skill_partner_check,      1,      0,      1,              },
	{ "invite_request_check",               &battle_config.invite_request_check,            1,      0,      1,              },
	{ "skill_removetrap_type",              &battle_config.skill_removetrap_type,           0,      0,      1,              },
	{ "disp_experience",                    &battle_config.disp_experience,                 0,      0,      1,              },
	{ "disp_zeny",                          &battle_config.disp_zeny,                       0,      0,      1,              },
	{ "castle_defense_rate",                &battle_config.castle_defense_rate,             100,    0,      100,            },
	{ "bone_drop",                          &battle_config.bone_drop,                       0,      0,      2,              },
	{ "buyer_name",                         &battle_config.buyer_name,                      1,      0,      1,              },
	{ "skill_wall_check",                   &battle_config.skill_wall_check,                1,      0,      1,              },
	{ "official_cell_stack_limit",          &battle_config.official_cell_stack_limit,       1,      0,      255,            },
	{ "custom_cell_stack_limit",            &battle_config.custom_cell_stack_limit,         1,      1,      255,            },
	{ "dancing_weaponswitch_fix",           &battle_config.dancing_weaponswitch_fix,        1,      0,      1,              },

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
	{ "item_drop_mvp_mode",                 &battle_config.item_drop_mvp_mode,              0,      0,      2,              },
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
	{ "pk_mode",                            &battle_config.pk_mode,                         0,      0,      2,              },
	{ "pk_level_range",                     &battle_config.pk_level_range,                  0,      0,      INT_MAX,        },
	{ "manner_system",                      &battle_config.manner_system,                   0xFFF,  0,      0xFFF,          },
	{ "pet_equip_required",                 &battle_config.pet_equip_required,              0,      0,      1,              },
	{ "multi_level_up",                     &battle_config.multi_level_up,                  0,      0,      1,              },
	{ "max_exp_gain_rate",                  &battle_config.max_exp_gain_rate,               0,      0,      INT_MAX,        },
	{ "backstab_bow_penalty",               &battle_config.backstab_bow_penalty,            0,      0,      1,              },
	{ "night_at_start",                     &battle_config.night_at_start,                  0,      0,      1,              },
	{ "show_mob_info",                      &battle_config.show_mob_info,                   0,      0,      1|2|4,          },
	{ "ban_hack_trade",                     &battle_config.ban_hack_trade,                  0,      0,      INT_MAX,        },
	{ "packet_ver_flag",                    &battle_config.packet_ver_flag,                 0x7FFFFFFF,0,   INT_MAX,        },
	{ "packet_ver_flag2",                   &battle_config.packet_ver_flag2,                0x7FFFFFFF,0,   INT_MAX,        },
	{ "min_hair_style",                     &battle_config.min_hair_style,                  0,      0,      INT_MAX,        },
	{ "max_hair_style",                     &battle_config.max_hair_style,                  23,     0,      INT_MAX,        },
	{ "min_hair_color",                     &battle_config.min_hair_color,                  0,      0,      INT_MAX,        },
	{ "max_hair_color",                     &battle_config.max_hair_color,                  9,      0,      INT_MAX,        },
	{ "min_cloth_color",                    &battle_config.min_cloth_color,                 0,      0,      INT_MAX,        },
	{ "max_cloth_color",                    &battle_config.max_cloth_color,                 4,      0,      INT_MAX,        },
	{ "pet_hair_style",                     &battle_config.pet_hair_style,                  100,    0,      INT_MAX,        },
	{ "castrate_dex_scale",                 &battle_config.castrate_dex_scale,              150,    1,      INT_MAX,        },
	{ "vcast_stat_scale",                   &battle_config.vcast_stat_scale,                530,    1,      INT_MAX,        },
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
	{ "berserk_cancels_buffs",              &battle_config.berserk_cancels_buffs,           0,      0,      1,              },
	{ "debuff_on_logout",                   &battle_config.debuff_on_logout,                1|2,    0,      1|2,            },
	{ "monster_ai",                         &battle_config.mob_ai,                          0x000,  0x000,  0x77F,          },
	{ "hom_setting",                        &battle_config.hom_setting,                     0xFFFF, 0x0000, 0xFFFF,         },
	{ "dynamic_mobs",                       &battle_config.dynamic_mobs,                    1,      0,      1,              },
	{ "mob_remove_damaged",                 &battle_config.mob_remove_damaged,              1,      0,      1,              },
	{ "show_hp_sp_drain",                   &battle_config.show_hp_sp_drain,                0,      0,      1,              },
	{ "show_hp_sp_gain",                    &battle_config.show_hp_sp_gain,                 1,      0,      1,              },
	{ "mob_npc_event_type",                 &battle_config.mob_npc_event_type,              1,      0,      1,              },
	{ "character_size",                     &battle_config.character_size,                  1|2,    0,      1|2,            },
	{ "mob_max_skilllvl",                   &battle_config.mob_max_skilllvl,                MAX_MOBSKILL_LEVEL, 1, MAX_MOBSKILL_LEVEL, },
	{ "retaliate_to_master",                &battle_config.retaliate_to_master,             1,      0,      1,              },
	{ "rare_drop_announce",                 &battle_config.rare_drop_announce,              0,      0,      10000,          },
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
	{ "max_guild_alliance",                 &battle_config.max_guild_alliance,              3,      0,      3,              },
	{ "ksprotection",                       &battle_config.ksprotection,                    5000,   0,      INT_MAX,        },
	{ "auction_feeperhour",                 &battle_config.auction_feeperhour,              12000,  0,      INT_MAX,        },
	{ "auction_maximumprice",               &battle_config.auction_maximumprice,            500000000, 0,   MAX_ZENY,       },
	{ "homunculus_auto_vapor",              &battle_config.homunculus_auto_vapor,           1,      0,      1,              },
	{ "display_status_timers",              &battle_config.display_status_timers,           1,      0,      1,              },
	{ "skill_add_heal_rate",                &battle_config.skill_add_heal_rate,             7,      0,      INT_MAX,        },
	{ "eq_single_target_reflectable",       &battle_config.eq_single_target_reflectable,    1,      0,      1,              },
	{ "invincible.nodamage",                &battle_config.invincible_nodamage,             0,      0,      1,              },
	{ "mob_slave_keep_target",              &battle_config.mob_slave_keep_target,           0,      0,      1,              },
	{ "autospell_check_range",              &battle_config.autospell_check_range,           0,      0,      1,              },
	{ "knockback_left",                     &battle_config.knockback_left,                  1,      0,      1,              },
	{ "client_reshuffle_dice",              &battle_config.client_reshuffle_dice,           0,      0,      1,              },
	{ "client_sort_storage",                &battle_config.client_sort_storage,             0,      0,      1,              },
	{ "feature.buying_store",               &battle_config.feature_buying_store,            1,      0,      1,              },
	{ "feature.search_stores",              &battle_config.feature_search_stores,           1,      0,      1,              },
	{ "searchstore_querydelay",             &battle_config.searchstore_querydelay,         10,      0,      INT_MAX,        },
	{ "searchstore_maxresults",             &battle_config.searchstore_maxresults,         30,      1,      INT_MAX,        },
	{ "display_party_name",                 &battle_config.display_party_name,              0,      0,      1,              },
	{ "cashshop_show_points",               &battle_config.cashshop_show_points,            0,      0,      1,              },
	{ "mail_show_status",                   &battle_config.mail_show_status,                0,      0,      2,              },
	{ "client_limit_unit_lv",               &battle_config.client_limit_unit_lv,            0,      0,      BL_ALL,         },
// BattleGround Settings
	{ "bg_update_interval",                 &battle_config.bg_update_interval,              1000,   100,    INT_MAX,        },
	{ "bg_short_attack_damage_rate",        &battle_config.bg_short_damage_rate,            80,     0,      INT_MAX,        },
	{ "bg_long_attack_damage_rate",         &battle_config.bg_long_damage_rate,             80,     0,      INT_MAX,        },
	{ "bg_weapon_attack_damage_rate",       &battle_config.bg_weapon_damage_rate,           60,     0,      INT_MAX,        },
	{ "bg_magic_attack_damage_rate",        &battle_config.bg_magic_damage_rate,            60,     0,      INT_MAX,        },
	{ "bg_misc_attack_damage_rate",         &battle_config.bg_misc_damage_rate,             60,     0,      INT_MAX,        },
	{ "bg_flee_penalty",                    &battle_config.bg_flee_penalty,                 20,     0,      INT_MAX,        },
// rAthena
	{ "max_third_parameter",				&battle_config.max_third_parameter,				135,	10,		SHRT_MAX,		},
	{ "max_baby_third_parameter",			&battle_config.max_baby_third_parameter,		108,	10,		SHRT_MAX,		},
	{ "max_trans_parameter",				&battle_config.max_trans_parameter,				99,		10,		SHRT_MAX,		},
	{ "max_third_trans_parameter",			&battle_config.max_third_trans_parameter,		135,	10,		SHRT_MAX,		},
	{ "max_extended_parameter",				&battle_config.max_extended_parameter,			125,	10,		SHRT_MAX,		},
	{ "skill_amotion_leniency",             &battle_config.skill_amotion_leniency,          90,     0,      300             },
	{ "mvp_tomb_enabled",                   &battle_config.mvp_tomb_enabled,                1,      0,      1               },
	{ "feature.atcommand_suggestions",      &battle_config.atcommand_suggestions_enabled,   0,      0,      1               },
	{ "min_npc_vendchat_distance",          &battle_config.min_npc_vendchat_distance,       3,      0,      100             },
	{ "atcommand_mobinfo_type",             &battle_config.atcommand_mobinfo_type,          0,      0,      1               },
	{ "homunculus_max_level",               &battle_config.hom_max_level,                   99,     0,      MAX_LEVEL,      },
	{ "homunculus_S_max_level",             &battle_config.hom_S_max_level,                 150,    0,      MAX_LEVEL,      },
	{ "mob_size_influence",                 &battle_config.mob_size_influence,              0,      0,      1,              },
	{ "skill_trap_type",                    &battle_config.skill_trap_type,                 0,      0,      3,              },
	{ "allow_consume_restricted_item",      &battle_config.allow_consume_restricted_item,   1,      0,      1,              },
	{ "allow_equip_restricted_item",        &battle_config.allow_equip_restricted_item,     1,      0,      1,              },
	{ "max_walk_path",                      &battle_config.max_walk_path,                   17,     1,      MAX_WALKPATH,   },
	{ "item_enabled_npc",                   &battle_config.item_enabled_npc,                1,      0,      1,              },
	{ "item_flooritem_check",               &battle_config.item_onfloor,                    1,      0,      1,              },
	{ "bowling_bash_area",                  &battle_config.bowling_bash_area,               0,      0,      20,             },
	{ "drop_rateincrease",                  &battle_config.drop_rateincrease,               0,      0,      1,              },
	{ "feature.auction",                    &battle_config.feature_auction,                 0,      0,      2,              },
	{ "feature.banking",                    &battle_config.feature_banking,                 1,      0,      1,              },
#ifdef VIP_ENABLE
	{ "vip_storage_increase",               &battle_config.vip_storage_increase,            0,      0,      MAX_STORAGE-MIN_STORAGE, },
#else
	{ "vip_storage_increase",               &battle_config.vip_storage_increase,            0,      0,      MAX_STORAGE, },
#endif
	{ "vip_base_exp_increase",              &battle_config.vip_base_exp_increase,           0,      0,      INT_MAX,        },
	{ "vip_job_exp_increase",               &battle_config.vip_job_exp_increase,            0,      0,      INT_MAX,        },
	{ "vip_exp_penalty_base_normal",        &battle_config.vip_exp_penalty_base_normal,     0,      0,      INT_MAX,        },
	{ "vip_exp_penalty_job_normal",         &battle_config.vip_exp_penalty_job_normal,      0,      0,      INT_MAX,        },
	{ "vip_exp_penalty_base",               &battle_config.vip_exp_penalty_base,            0,      0,      INT_MAX,        },
	{ "vip_exp_penalty_job",                &battle_config.vip_exp_penalty_job,             0,      0,      INT_MAX,        },
	{ "vip_bm_increase",                    &battle_config.vip_bm_increase,                 0,      0,      INT_MAX,        },
	{ "vip_drop_increase",                  &battle_config.vip_drop_increase,               0,      0,      INT_MAX,        },
	{ "vip_gemstone",                       &battle_config.vip_gemstone,                    0,      0,      1,              },
	{ "vip_disp_rate",                      &battle_config.vip_disp_rate,                   1,      0,      1,              },
	{ "mon_trans_disable_in_gvg",           &battle_config.mon_trans_disable_in_gvg,        0,      0,      1,              },
	{ "homunculus_S_growth_level",          &battle_config.hom_S_growth_level,             99,      0,      MAX_LEVEL,      },
	{ "emblem_woe_change",                  &battle_config.emblem_woe_change,               0,      0,      1,              },
	{ "emblem_transparency_limit",          &battle_config.emblem_transparency_limit,      80,      0,      100,            },
	{ "discount_item_point_shop",			&battle_config.discount_item_point_shop,		0,		0,		3,				},
	{ "update_enemy_position",				&battle_config.update_enemy_position,			0,		0,		1,				},
	{ "devotion_rdamage",					&battle_config.devotion_rdamage,				0,		0,		100,			},
	{ "feature.autotrade",					&battle_config.feature_autotrade,				1,		0,		1,				},
	{ "feature.autotrade_direction",		&battle_config.feature_autotrade_direction,		4,		-1,		7,				},
	{ "feature.autotrade_head_direction",	&battle_config.feature_autotrade_head_direction,0,		-1,		2,				},
	{ "feature.autotrade_sit",				&battle_config.feature_autotrade_sit,			1,		-1,		1,				},
	{ "feature.autotrade_open_delay",		&battle_config.feature_autotrade_open_delay,	5000,	1000,	INT_MAX,		},
	{ "disp_serverbank_msg",				&battle_config.disp_serverbank_msg,				0,		0,		1,				},
	{ "disp_servervip_msg",					&battle_config.disp_servervip_msg,				0,		0,		1,				},
	{ "warg_can_falcon",                    &battle_config.warg_can_falcon,                 0,      0,      1,              },
	{ "path_blown_halt",                    &battle_config.path_blown_halt,                 1,      0,      1,              },
	{ "rental_mount_speed_boost",           &battle_config.rental_mount_speed_boost,        25,     0,      100,        	},
	{ "feature.warp_suggestions",           &battle_config.warp_suggestions_enabled,        0,      0,      1,              },
	{ "taekwon_mission_mobname",            &battle_config.taekwon_mission_mobname,         0,      0,      2,              },
	{ "teleport_on_portal",                 &battle_config.teleport_on_portal,              0,      0,      1,              },
	{ "cart_revo_knockback",                &battle_config.cart_revo_knockback,             1,      0,      1,              },
	{ "guild_notice_changemap",             &battle_config.guild_notice_changemap,          2,      0,      2,              },
	{ "transcendent_status_points",         &battle_config.transcendent_status_points,     52,      1,      INT_MAX,        },
	{ "taekwon_ranker_min_lv",              &battle_config.taekwon_ranker_min_lv,          90,      1,      MAX_LEVEL,      },
	{ "revive_onwarp",                      &battle_config.revive_onwarp,                   1,      0,      1,              },
	{ "fame_taekwon_mission",               &battle_config.fame_taekwon_mission,            1,      0,      INT_MAX,        },
	{ "fame_refine_lv1",                    &battle_config.fame_refine_lv1,                 1,      0,      INT_MAX,        },
	{ "fame_refine_lv1",                    &battle_config.fame_refine_lv1,                 1,      0,      INT_MAX,        },
	{ "fame_refine_lv2",                    &battle_config.fame_refine_lv2,                 25,     0,      INT_MAX,        },
	{ "fame_refine_lv3",                    &battle_config.fame_refine_lv3,                 1000,   0,      INT_MAX,        },
	{ "fame_forge",                         &battle_config.fame_forge,                      10,     0,      INT_MAX,        },
	{ "fame_pharmacy_3",                    &battle_config.fame_pharmacy_3,                 1,      0,      INT_MAX,        },
	{ "fame_pharmacy_5",                    &battle_config.fame_pharmacy_5,                 3,      0,      INT_MAX,        },
	{ "fame_pharmacy_7",                    &battle_config.fame_pharmacy_7,                 10,     0,      INT_MAX,        },
	{ "fame_pharmacy_10",                   &battle_config.fame_pharmacy_10,                50,     0,      INT_MAX,        },
	{ "mail_delay",                         &battle_config.mail_delay,                      1000,   1000,   INT_MAX,        },
	{ "at_monsterignore",                   &battle_config.autotrade_monsterignore,         0,      0,      1,              },
	{ "idletime_option",                    &battle_config.idletime_option,                 0x25,   1,      INT_MAX,        },
	{ "spawn_direction",                    &battle_config.spawn_direction,                 0,      0,      1,              },
	{ "arrow_shower_knockback",             &battle_config.arrow_shower_knockback,          1,      0,      1,              },
	{ "devotion_rdamage_skill_only",        &battle_config.devotion_rdamage_skill_only,     1,      0,      1,              },
	{ "max_extended_aspd",                  &battle_config.max_extended_aspd,               193,    100,    199,            },
	{ "monster_chase_refresh",              &battle_config.mob_chase_refresh,               1,      0,      30,             },
	{ "mob_icewall_walk_block",             &battle_config.mob_icewall_walk_block,          75,     0,      255,            },
	{ "boss_icewall_walk_block",            &battle_config.boss_icewall_walk_block,         0,      0,      255,            },
	{ "snap_dodge",                         &battle_config.snap_dodge,                      0,      0,      1,              },
	{ "stormgust_knockback",                &battle_config.stormgust_knockback,             1,      0,      1,              },
	{ "default_fixed_castrate",             &battle_config.default_fixed_castrate,          20,     0,      100,            },
	{ "default_bind_on_equip",              &battle_config.default_bind_on_equip,           BOUND_CHAR, BOUND_NONE, BOUND_MAX-1, },
	{ "pet_ignore_infinite_def",            &battle_config.pet_ignore_infinite_def,         0,      0,      1,              },
	{ "homunculus_evo_intimacy_need",       &battle_config.homunculus_evo_intimacy_need,    91100,  0,      INT_MAX,        },
	{ "homunculus_evo_intimacy_reset",      &battle_config.homunculus_evo_intimacy_reset,   1000,   0,      INT_MAX,        },
	{ "monster_loot_search_type",           &battle_config.monster_loot_search_type,        1,      0,      1,              },
	{ "feature.roulette",                   &battle_config.feature_roulette,                1,      0,      1,              },
};

#ifndef STATS_OPT_OUT
// rAthena anonymous statistic usage report -- packet is built here, and sent to char server to report.
void rAthena_report(char* date, char *time_c) {
	int i, rev = 0, bd_size = ARRAYLENGTH(battle_data);
	unsigned int config = 0;
	const char* rev_str;
	char timestring[25];
	time_t curtime;
	char* buf;

	enum config_table {
		C_CIRCULAR_AREA         = 0x0001,
		C_CELLNOSTACK           = 0x0002,
		C_BETA_THREAD_TEST      = 0x0004,
		C_SCRIPT_CALLFUNC_CHECK = 0x0008,
		C_OFFICIAL_WALKPATH     = 0x0010,
		C_RENEWAL               = 0x0020,
		C_RENEWAL_CAST          = 0x0040,
		C_RENEWAL_DROP          = 0x0080,
		C_RENEWAL_EXP           = 0x0100,
		C_RENEWAL_LVDMG         = 0x0200,
		C_RENEWAL_ASPD          = 0x0400,
		C_SECURE_NPCTIMEOUT     = 0x0800,
		C_SQL_DBS               = 0x1000,
		C_SQL_LOGS              = 0x2000,
	};

	if( (rev_str = get_svn_revision()) != 0 )
		rev = atoi(rev_str);

	/* we get the current time */
	time(&curtime);
	strftime(timestring, 24, "%Y-%m-%d %H:%M:%S", localtime(&curtime));

// Various compile-time options
#ifdef CIRCULAR_AREA
	config |= C_CIRCULAR_AREA;
#endif

#ifdef CELL_NOSTACK
	config |= C_CELLNOSTACK;
#endif

#ifdef BETA_THREAD_TEST
	config |= C_BETA_THREAD_TEST;
#endif

#ifdef SCRIPT_CALLFUNC_CHECK
	config |= C_SCRIPT_CALLFUNC_CHECK;
#endif

#ifdef OFFICIAL_WALKPATH
	config |= C_OFFICIAL_WALKPATH;
#endif

#ifdef RENEWAL
	config |= C_RENEWAL;
#endif

#ifdef RENEWAL_CAST
	config |= C_RENEWAL_CAST;
#endif

#ifdef RENEWAL_DROP
	config |= C_RENEWAL_DROP;
#endif

#ifdef RENEWAL_EXP
	config |= C_RENEWAL_EXP;
#endif

#ifdef RENEWAL_LVDMG
	config |= C_RENEWAL_LVDMG;
#endif

#ifdef RENEWAL_ASPD
	config |= C_RENEWAL_ASPD;
#endif

#ifdef SECURE_NPCTIMEOUT
	config |= C_SECURE_NPCTIMEOUT;
#endif
	/* non-define part */
	if( db_use_sqldbs )
		config |= C_SQL_DBS;

	if( log_config.sql_logs )
		config |= C_SQL_LOGS;

#define BFLAG_LENGTH 35

	CREATE(buf, char, 6 + 12 + 9 + 24 + 4 + 4 + 4 + 4 + ( bd_size * ( BFLAG_LENGTH + 4 ) ) + 1 );

	/* build packet */
	WBUFW(buf,0) = 0x3000;
	WBUFW(buf,2) = 6 + 12 + 9 + 24 + 4 + 4 + 4 + 4 + ( bd_size * ( BFLAG_LENGTH + 4 ) );
	WBUFW(buf,4) = 0x9c;

	safestrncpy((char*)WBUFP(buf,6), date, 12);
	safestrncpy((char*)WBUFP(buf,6 + 12), time_c, 9);
	safestrncpy((char*)WBUFP(buf,6 + 12 + 9), timestring, 24);

	WBUFL(buf,6 + 12 + 9 + 24)         = rev;
	WBUFL(buf,6 + 12 + 9 + 24 + 4)     = map_getusers();

	WBUFL(buf,6 + 12 + 9 + 24 + 4 + 4) = config;
	WBUFL(buf,6 + 12 + 9 + 24 + 4 + 4 + 4) = bd_size;

	for( i = 0; i < bd_size; i++ ) {
		safestrncpy((char*)WBUFP(buf,6 + 12 + 9+ 24  + 4 + 4 + 4 + 4 + ( i * ( BFLAG_LENGTH + 4 ) ) ), battle_data[i].str, 35);
		WBUFL(buf,6 + 12 + 9 + 24 + 4 + 4 + 4 + 4 + BFLAG_LENGTH + ( i * ( BFLAG_LENGTH + 4 )  )  ) = *battle_data[i].val;
	}

	chrif_send_report(buf, 6 + 12 + 9 + 24 + 4 + 4 + 4 + 4 + ( bd_size * ( BFLAG_LENGTH + 4 ) ) );

	aFree(buf);

#undef BFLAG_LENGTH
}
static int rAthena_report_timer(int tid, unsigned int tick, int id, intptr_t data) {
	if( chrif_isconnected() ) { /* char server relays it, so it must be online. */
		rAthena_report(__DATE__,__TIME__);
	}
	return 0;
}
#endif

/*==========================
 * Set battle settings
 *--------------------------*/
int battle_set_value(const char* w1, const char* w2)
{
	int val = config_switch(w2);

	int i;
	ARR_FIND(0, ARRAYLENGTH(battle_data), i, strcmpi(w1, battle_data[i].str) == 0);
	if (i == ARRAYLENGTH(battle_data))
		return 0; // not found

	if (val < battle_data[i].min || val > battle_data[i].max) {
		ShowWarning("Value for setting '%s': %s is invalid (min:%i max:%i)! Defaulting to %i...\n", w1, w2, battle_data[i].min, battle_data[i].max, battle_data[i].defval);
		val = battle_data[i].defval;
	}

	*battle_data[i].val = val;
	return 1;
}

/*===========================
 * Get battle settings
 *---------------------------*/
int battle_get_value(const char* w1)
{
	int i;
	ARR_FIND(0, ARRAYLENGTH(battle_data), i, strcmpi(w1, battle_data[i].str) == 0);
	if (i == ARRAYLENGTH(battle_data))
		return 0; // not found
	else
		return *battle_data[i].val;
}

/*======================
 * Set default settings
 *----------------------*/
void battle_set_defaults()
{
	int i;
	for (i = 0; i < ARRAYLENGTH(battle_data); i++)
		*battle_data[i].val = battle_data[i].defval;
}

/*==================================
 * Cap certain battle.conf settings
 *----------------------------------*/
void battle_adjust_conf()
{
	battle_config.monster_max_aspd = 2000 - battle_config.monster_max_aspd * 10;
	battle_config.max_aspd = 2000 - battle_config.max_aspd * 10;
	battle_config.max_third_aspd = 2000 - battle_config.max_third_aspd * 10;
	battle_config.max_extended_aspd = 2000 - battle_config.max_extended_aspd * 10;
	battle_config.max_walk_speed = 100 * DEFAULT_WALK_SPEED / battle_config.max_walk_speed;
	battle_config.max_cart_weight *= 10;

	if (battle_config.max_def > 100 && !battle_config.weapon_defense_type) // added by [Skotlex]
		battle_config.max_def = 100;

	if (battle_config.min_hitrate > battle_config.max_hitrate)
		battle_config.min_hitrate = battle_config.max_hitrate;

	if (battle_config.pet_max_atk1 > battle_config.pet_max_atk2) //Skotlex
		battle_config.pet_max_atk1 = battle_config.pet_max_atk2;

	if (battle_config.day_duration && battle_config.day_duration < 60000) // added by [Yor]
		battle_config.day_duration = 60000;
	if (battle_config.night_duration && battle_config.night_duration < 60000) // added by [Yor]
		battle_config.night_duration = 60000;

#if PACKETVER < 20100427
	if (battle_config.feature_buying_store) {
		ShowWarning("conf/battle/feature.conf:buying_store is enabled but it requires PACKETVER 2010-04-27 or newer, disabling...\n");
		battle_config.feature_buying_store = 0;
	}
#endif

#if PACKETVER < 20100803
	if (battle_config.feature_search_stores) {
		ShowWarning("conf/battle/feature.conf:search_stores is enabled but it requires PACKETVER 2010-08-03 or newer, disabling...\n");
		battle_config.feature_search_stores = 0;
	}
#endif

#if PACKETVER > 20120000 && PACKETVER < 20130515 /* Exact date (when it started) not known */
	if (battle_config.feature_auction) {
		ShowWarning("conf/battle/feature.conf:feature.auction is enabled but it is not stable on PACKETVER "EXPAND_AND_QUOTE(PACKETVER)", disabling...\n");
		ShowWarning("conf/battle/feature.conf:feature.auction change value to '2' to silence this warning and maintain it enabled\n");
		battle_config.feature_auction = 0;
	}
#endif

#if PACKETVER < 20130724
	if (battle_config.feature_banking) {
		ShowWarning("conf/battle/feature.conf banking is enabled but it requires PACKETVER 2013-07-24 or newer, disabling...\n");
		battle_config.feature_banking = 0;
	}
#endif

#if PACKETVER < 20141022
	if (battle_config.feature_roulette) {
		ShowWarning("conf/battle/feature.conf roulette is enabled but it requires PACKETVER 2014-10-22 or newer, disabling...\n");
		battle_config.feature_roulette = 0;
	}
#endif

#ifndef CELL_NOSTACK
	if (battle_config.custom_cell_stack_limit != 1)
		ShowWarning("Battle setting 'custom_cell_stack_limit' takes no effect as this server was compiled without Cell Stack Limit support.\n");
#endif
}

/*=====================================
 * Read battle.conf settings from file
 *-------------------------------------*/
int battle_config_read(const char* cfgName)
{
	FILE* fp;
	static int count = 0;

	if (count == 0)
		battle_set_defaults();

	count++;

	fp = fopen(cfgName,"r");
	if (fp == NULL)
		ShowError("File not found: %s\n", cfgName);
	else {
		char line[1024], w1[1024], w2[1024];

		while(fgets(line, sizeof(line), fp)) {
			if (line[0] == '/' && line[1] == '/')
				continue;
			if (sscanf(line, "%1023[^:]:%1023s", w1, w2) != 2)
				continue;
			if (strcmpi(w1, "import") == 0)
				battle_config_read(w2);
			else if
				(battle_set_value(w1, w2) == 0)
				ShowWarning("Unknown setting '%s' in file %s\n", w1, cfgName);
		}

		fclose(fp);
	}

	count--;

	if (count == 0)
		battle_adjust_conf();

	return 0;
}

/*==========================
 * initialize battle timer
 *--------------------------*/
void do_init_battle(void)
{
	delay_damage_ers = ers_new(sizeof(struct delay_damage),"battle.c::delay_damage_ers",ERS_OPT_CLEAR);
	add_timer_func_list(battle_delay_damage_sub, "battle_delay_damage_sub");

#ifndef STATS_OPT_OUT
	add_timer_func_list(rAthena_report_timer, "rAthena_report_timer");
	add_timer_interval(gettick() + 30000, rAthena_report_timer, 0, 0, 60000 * 30);
#endif

}

/*==================
 * end battle timer
 *------------------*/
void do_final_battle(void)
{
	ers_destroy(delay_damage_ers);
}
