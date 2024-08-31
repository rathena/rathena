// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "battle.hpp"

#include <cmath>
#include <cstdlib>

#include <common/cbasetypes.hpp>
#include <common/ers.hpp>
#include <common/malloc.hpp>
#include <common/nullpo.hpp>
#include <common/random.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>
#include <common/utils.hpp>

#include "atcommand.hpp"
#include "battleground.hpp"
#include "chrif.hpp"
#include "clif.hpp"
#include "elemental.hpp"
#include "guild.hpp"
#include "homunculus.hpp"
#include "log.hpp"
#include "map.hpp"
#include "mercenary.hpp"
#include "mob.hpp"
#include "party.hpp"
#include "path.hpp"
#include "pc.hpp"
#include "pc_groups.hpp"
#include "pet.hpp"

using namespace rathena;

struct Battle_Config battle_config;
static struct eri *delay_damage_ers; //For battle delay damage structures.

// Early declaration
int battle_get_weapon_element(struct Damage *wd, struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv, short weapon_position, bool calc_for_damage_only);
int battle_get_magic_element(struct block_list* src, struct block_list* target, uint16 skill_id, uint16 skill_lv, int mflag);
int battle_get_misc_element(struct block_list* src, struct block_list* target, uint16 skill_id, uint16 skill_lv, int mflag);
static void battle_calc_defense_reduction(struct Damage* wd, struct block_list* src, struct block_list* target, uint16 skill_id, uint16 skill_lv);

/**
 * Returns the current/list skill used by the bl
 * @param bl
 * @return skill_id
 */
uint16 battle_getcurrentskill(struct block_list *bl)
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
	nullpo_retr(nullptr, target);

	memset(bl_list, 0, sizeof(bl_list));
	map_foreachinallrange(battle_gettargeted_sub, target, AREA_SIZE, BL_CHAR, bl_list, &c, target->id);
	if ( c == 0 )
		return nullptr;
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
		case BL_PC:  return ((map_session_data*)bl)->ud.target;
		case BL_MOB: return ((struct mob_data*)bl)->target_id;
		case BL_PET: return ((struct pet_data*)bl)->target_id;
		case BL_HOM: return ((struct homun_data*)bl)->ud.target;
		case BL_MER: return ((s_mercenary_data*)bl)->ud.target;
		case BL_ELEM: return ((s_elemental_data*)bl)->ud.target;
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
	map_foreachinallrange(battle_getenemy_sub, target, range, type, bl_list, &c, target);

	if ( c == 0 )
		return nullptr;

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
	map_foreachinallarea(battle_getenemyarea_sub, src->m, x - range, y - range, x + range, y + range, type, bl_list, &c, src, ignore_id);

	if( c == 0 )
		return nullptr;
	if( c >= 24 )
		c = 23;

	return bl_list[rnd()%c];
}

/**
* Deals damage without delay, applies additional effects and triggers monster events
* This function is called from battle_delay_damage or battle_delay_damage_sub
* All other instances of battle damage should also go through this function (i.e. anything that displays a damage number)
* Consider calling this function or battle_fix_damage instead status_fix_damage directly
* @param src: Source of damage
* @param target: Target of damage
* @param damage: Damage to be dealt
* @param delay: Damage delay
* @param skill_lv: Level of skill used
* @param skill_id: ID of skill used
* @param dmg_lv: State of the attack (miss, etc.)
* @param attack_type: Type of the attack (BF_NORMAL|BF_SKILL|BF_SHORT|BF_LONG|BF_WEAPON|BF_MAGIC|BF_MISC)
* @param additional_effects: Whether additional effects should be applied (otherwise it's just damage+coma)
* @param isspdamage: If the damage is done to SP
* @param tick: Current tick
* @return HP+SP+AP (0 if HP/SP/AP remained unchanged)
*/
int battle_damage(struct block_list *src, struct block_list *target, int64 damage, t_tick delay, uint16 skill_lv, uint16 skill_id, enum damage_lv dmg_lv, unsigned short attack_type, bool additional_effects, t_tick tick, bool isspdamage) {
	int dmg_change;
	map_session_data* sd = nullptr;
	if (src)
		sd = BL_CAST(BL_PC, src);
	map_freeblock_lock();
	if (isspdamage)
		dmg_change = status_fix_spdamage(src, target, damage, delay, skill_id);
	else if (sd && battle_check_coma(*sd, *target, (e_battle_flag)attack_type))
		dmg_change = status_damage(src, target, damage, 0, delay, 16, skill_id); // Coma attack
	else
		dmg_change = status_fix_damage(src, target, damage, delay, skill_id);
	if (attack_type && !status_isdead(target) && additional_effects)
		skill_additional_effect(src, target, skill_id, skill_lv, attack_type, dmg_lv, tick);
	if (dmg_lv > ATK_BLOCK && attack_type && additional_effects)
		skill_counter_additional_effect(src, target, skill_id, skill_lv, attack_type, tick);
	// This is the last place where we have access to the actual damage type, so any monster events depending on type must be placed here
	if (target->type == BL_MOB && additional_effects) {
		mob_data *md = BL_CAST(BL_MOB, target);

		if (md != nullptr) {
			// Trigger monster skill condition for non-skill attacks.
			if (!status_isdead(target) && src != target) {
				if (damage > 0)
					mobskill_event(md, src, tick, attack_type, damage);
				if (skill_id > 0)
					mobskill_event(md, src, tick, MSC_SKILLUSED | (skill_id << 16));
			}

			// Monsters differentiate whether they have been attacked by a skill or a normal attack
			if (damage > 0 && (attack_type & BF_NORMAL))
				md->norm_attacked_id = md->attacked_id;
		}
	}
	map_freeblock_unlock();
	return dmg_change;
}

/// Damage Delayed Structure
struct delay_damage {
	int src_id;
	int target_id;
	int64 damage;
	t_tick delay;
	unsigned short distance;
	uint16 skill_lv;
	uint16 skill_id;
	enum damage_lv dmg_lv;
	unsigned short attack_type;
	bool additional_effects;
	enum bl_type src_type;
	bool isspdamage;
};

TIMER_FUNC(battle_delay_damage_sub){
	struct delay_damage *dat = (struct delay_damage *)data;

	if ( dat ) {
		struct block_list* src = map_id2bl(dat->src_id);
		struct block_list* target = map_id2bl(dat->target_id);

		if (target && !status_isdead(target)) {
			if( src && target->m == src->m &&
				(target->type != BL_PC || ((TBL_PC*)target)->invincible_timer == INVALID_TIMER) &&
				check_distance_bl(src, target, dat->distance) ) //Check to see if you haven't teleported. [Skotlex]
			{
				//Deal damage
				battle_damage(src, target, dat->damage, dat->delay, dat->skill_lv, dat->skill_id, dat->dmg_lv, dat->attack_type, dat->additional_effects, tick, dat->isspdamage);
			} else if( !src && dat->skill_id == CR_REFLECTSHIELD ) { // it was monster reflected damage, and the monster died, we pass the damage to the character as expected
				battle_fix_damage(target, target, dat->damage, dat->delay, dat->skill_id);
			}
		}

		map_session_data *sd = BL_CAST(BL_PC, src);

		if (sd && --sd->delayed_damage == 0 && sd->state.hold_recalc) {
			sd->state.hold_recalc = false;
			status_calc_pc(sd, SCO_FORCE);
		}
	}
	ers_free(delay_damage_ers, dat);
	return 0;
}

int battle_delay_damage(t_tick tick, int amotion, struct block_list *src, struct block_list *target, int attack_type, uint16 skill_id, uint16 skill_lv, int64 damage, enum damage_lv dmg_lv, t_tick ddelay, bool additional_effects, bool isspdamage)
{
	struct delay_damage *dat;
	status_change *sc;
	struct block_list *d_tbl = nullptr;
	struct block_list *e_tbl = nullptr;

	nullpo_ret(src);
	nullpo_ret(target);

	sc = status_get_sc(target);

	if (sc) {
		if (sc->getSCE(SC_DEVOTION) && sc->getSCE(SC_DEVOTION)->val1)
			d_tbl = map_id2bl(sc->getSCE(SC_DEVOTION)->val1);
		if (sc->getSCE(SC_WATER_SCREEN_OPTION) && sc->getSCE(SC_WATER_SCREEN_OPTION)->val1)
			e_tbl = map_id2bl(sc->getSCE(SC_WATER_SCREEN_OPTION)->val1);
	}

	if( ((d_tbl && check_distance_bl(target, d_tbl, sc->getSCE(SC_DEVOTION)->val3)) || e_tbl) &&
		damage > 0 && skill_id != CR_REFLECTSHIELD
#ifndef RENEWAL
		&& skill_id != PA_PRESSURE
#endif
		) {
		map_session_data* tsd = BL_CAST( BL_PC, target );

		if( tsd && pc_issit( tsd ) && battle_config.devotion_standup_fix ){
			pc_setstand( tsd, true );
			skill_sit( tsd, 0 );
		}

		damage = 0;
	}

	// The client refuses to display animations slower than 1x speed
	// So we need to shorten AttackMotion to be in-sync with the client in this case
	if (battle_config.synchronize_damage && skill_id == 0 && src->type == BL_MOB && amotion > status_get_clientamotion(src))
		amotion = status_get_clientamotion(src);
	// Check for delay battle damage config
	else if (!battle_config.delay_battle_damage)
		amotion = 1;
	// Aegis places a damage-delay cap of 1 sec to non player attacks
	// We only want to apply this cap if damage was not synchronized
	else if (src->type != BL_PC && amotion > 1000)
		amotion = 1000;

	// Skip creation of timer
	if (amotion <= 1) {
		//Deal damage
		battle_damage(src, target, damage, ddelay, skill_lv, skill_id, dmg_lv, attack_type, additional_effects, gettick(), isspdamage);
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
	dat->isspdamage = isspdamage;

	if( src->type == BL_PC )
		((TBL_PC*)src)->delayed_damage++;

	add_timer(tick+amotion, battle_delay_damage_sub, 0, (intptr_t)dat);

	return 0;
}

int battle_fix_damage(struct block_list* src, struct block_list* target, int64 damage, t_tick walkdelay, uint16 skill_id) {
	return battle_damage(src, target, damage, walkdelay, 0, skill_id, ATK_DEF, BF_MISC, false, gettick(), false);
}

/**
 * Does attribute fix modifiers.
 * Added passing of the chars so that the status changes can affect it. [Skotlex]
 * Note: Passing src/target == nullptr is perfectly valid, it skips SC_ checks.
 * @param src
 * @param target
 * @param damage
 * @param atk_elem
 * @param def_type
 * @param def_lv
 * @param flag 0x1 = allow to return negative values even if config for healing through negative resist is disabled
 * @return damage
 */
int64 battle_attr_fix(struct block_list *src, struct block_list *target, int64 damage,int atk_elem,int def_type, int def_lv, int flag)
{
	status_change *sc = nullptr, *tsc = nullptr;
	int ratio;

	if (src) sc = status_get_sc(src);
	if (target) tsc = status_get_sc(target);

	if (!CHK_ELEMENT(atk_elem))
		atk_elem = rnd()%ELE_ALL;

	if (!CHK_ELEMENT(def_type) || !CHK_ELEMENT_LEVEL(def_lv)) {
		ShowError("battle_attr_fix: unknown attribute type: atk=%d def_type=%d def_lv=%d\n",atk_elem,def_type,def_lv);
		return damage;
	}

	ratio = elemental_attribute_db.getAttribute(def_lv-1, atk_elem, def_type);
	if (sc && sc->count) { //increase dmg by src status
		switch(atk_elem){
			case ELE_FIRE:
				if (sc->getSCE(SC_VOLCANO))
#ifdef RENEWAL
					ratio += sc->getSCE(SC_VOLCANO)->val3;
#else
					damage += (int64)((damage*sc->getSCE(SC_VOLCANO)->val3) / 100);
#endif
				break;
			case ELE_WIND:
				if (sc->getSCE(SC_VIOLENTGALE))
#ifdef RENEWAL
					ratio += sc->getSCE(SC_VIOLENTGALE)->val3;
#else
					damage += (int64)((damage*sc->getSCE(SC_VIOLENTGALE)->val3) / 100);
#endif
				break;
			case ELE_WATER:
				if (sc->getSCE(SC_DELUGE))
#ifdef RENEWAL
					ratio += sc->getSCE(SC_DELUGE)->val3;
#else
					damage += (int64)((damage*sc->getSCE(SC_DELUGE)->val3) / 100);
#endif
				break;
			case ELE_GHOST:
				if (sc->getSCE(SC_TELEKINESIS_INTENSE))
					ratio += sc->getSCE(SC_TELEKINESIS_INTENSE)->val3;
				break;
		}
	}

	if( target && target->type == BL_SKILL ) {
		if( atk_elem == ELE_FIRE && battle_getcurrentskill(target) == GN_WALLOFTHORN ) {
			struct skill_unit *su = (struct skill_unit*)target;
			std::shared_ptr<s_skill_unit_group> sg;
			struct block_list *src2;

			if( !su || !su->alive || (sg = su->group) == nullptr || !sg || sg->val3 == -1 ||
			   (src2 = map_id2bl(sg->src_id)) == nullptr || status_isdead(src2) )
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
				if (tsc->getSCE(SC_SPIDERWEB)) { //Double damage
#ifdef RENEWAL
					ratio += 100;
#else
					damage *= 2;
#endif
					//Remove a unit group or end whole status change
					status_change_end(target, SC_SPIDERWEB);
				}
				if (tsc->getSCE(SC_THORNSTRAP) && battle_getcurrentskill(src) != GN_CARTCANNON)
					status_change_end(target, SC_THORNSTRAP);
				if (tsc->getSCE(SC_CRYSTALIZE))
					status_change_end(target, SC_CRYSTALIZE);
				if (tsc->getSCE(SC_EARTH_INSIGNIA))
#ifdef RENEWAL
					ratio += 50;
#else
					damage += (int64)(damage * 50 / 100);
#endif
				if( tsc->getSCE(SC_WIDEWEB) ) {
#ifdef RENEWAL
					ratio += 100;
#else
					damage *= 2;
#endif
					status_change_end(target,SC_WIDEWEB);
				}
				if( tsc->getSCE(SC_BURNT) ) {
#ifdef RENEWAL
					ratio += 400;
#else
					damage += (int64)(damage * 400 / 100);
#endif
				}
				break;
			case ELE_HOLY:
				if (tsc->getSCE(SC_ORATIO))
#ifdef RENEWAL
					ratio += tsc->getSCE(SC_ORATIO)->val1 * 2;
#else
					damage += (int64)(damage * (tsc->getSCE(SC_ORATIO)->val1 * 2) / 100);
#endif
				break;
			case ELE_POISON:
				if (tsc->getSCE(SC_VENOMIMPRESS))
#ifdef RENEWAL
					ratio += tsc->getSCE(SC_VENOMIMPRESS)->val2;
#else
					damage += (int64)(damage * tsc->getSCE(SC_VENOMIMPRESS)->val2 / 100);
#endif
				if (tsc->getSCE(SC_CLOUD_POISON)) {
#ifdef RENEWAL
					ratio += 5 * tsc->getSCE(SC_CLOUD_POISON)->val1;
#else
					damage += (int64)(damage * 5 * tsc->getSCE(SC_CLOUD_POISON)->val1 / 100);
#endif
				}
				break;
			case ELE_WIND:
				if (tsc->getSCE(SC_WATER_INSIGNIA))
#ifdef RENEWAL
					ratio += 50;
#else
					damage += (int64)(damage * 50 / 100);
#endif
				if (tsc->getSCE(SC_CRYSTALIZE)) {
					uint16 skill_id = battle_getcurrentskill(src);

					if (skill_get_type(skill_id)&BF_MAGIC)
#ifdef RENEWAL
						ratio += 50;
#else
						damage += (int64)(damage * 50 / 100);
#endif
				}
				break;
			case ELE_WATER:
				if (tsc->getSCE(SC_FIRE_INSIGNIA))
#ifdef RENEWAL
					ratio += 50;
#else
					damage += (int64)(damage * 50 / 100);
#endif
				if (tsc->getSCE(SC_MISTYFROST))
#ifdef RENEWAL
					ratio += 15;
#else
					damage += (int64)(damage * 15 / 100);
#endif
				break;
			case ELE_EARTH:
				if (tsc->getSCE(SC_WIND_INSIGNIA))
#ifdef RENEWAL
					ratio += 50;
#else
					damage += (int64)(damage * 50 / 100);
#endif
				status_change_end(target, SC_MAGNETICFIELD); //freed if received earth dmg
				break;
			case ELE_NEUTRAL:
				if (tsc->getSCE(SC_ANTI_M_BLAST))
#ifdef RENEWAL
					ratio += tsc->getSCE(SC_ANTI_M_BLAST)->val2;
#else
					damage += (int64)(damage * tsc->getSCE(SC_ANTI_M_BLAST)->val2 / 100);
#endif
				break;
			case ELE_DARK:
				if (tsc->getSCE(SC_SOULCURSE)) {
					if (status_get_class_(target) == CLASS_BOSS)
#ifdef RENEWAL
						ratio += 20;
#else
						damage += (int64)(damage * 20 / 100);
#endif
					else
#ifdef RENEWAL
						ratio += 100;
#else
						damage *= 2;
#endif
				}
				break;
		}

		if (tsc->getSCE(SC_MAGIC_POISON))
#ifdef RENEWAL
			ratio += 50;
#else
			damage += (int64)(damage * 50 / 100);
#endif
	}

	if (battle_config.attr_recover == 0 && !(flag & 1) && ratio < 0)
		ratio = 0;

#ifdef RENEWAL
	//In renewal, reductions are always rounded down so damage can never reach 0 unless ratio is 0
	damage = damage - (int64)((damage * (100 - ratio)) / 100);
#else
	damage = (int64)((damage*ratio)/100);
#endif

	//Damage can be negative, see battle_config.attr_recover
	return damage;
}

/**
 * Calculates card bonuses damage adjustments.
 * @param attack_type @see enum e_battle_flag
 * @param src Attacker
 * @param target Target
 * @param nk Skill's nk @see enum e_skill_nk [NK_IGNOREATKCARD|NK_IGNOREELEMENT|NK_IGNOREDEFCARD]
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
int battle_calc_cardfix(int attack_type, struct block_list *src, struct block_list *target, std::bitset<NK_MAX> nk, int rh_ele, int lh_ele, int64 damage, int left, int flag){
	map_session_data *sd, ///< Attacker session data if BL_PC
		*tsd; ///< Target session data if BL_PC
	int cardfix = 1000;
	int s_class, ///< Attacker class
		t_class; ///< Target class
	std::vector<e_race2> s_race2, /// Attacker Race2
		t_race2; ///< Target Race2
	enum e_element s_defele; ///< Attacker Element (not a weapon or skill element!)
	struct status_data *sstatus, ///< Attacker status data
		*tstatus; ///< Target status data
	int64 original_damage;

	if( !damage )
		return 0;

	original_damage = damage;

	sd = BL_CAST(BL_PC, src);
	tsd = BL_CAST(BL_PC, target);
	t_class = status_get_class(target);
	s_class = status_get_class(src);
	sstatus = status_get_status_data(src);
	tstatus = status_get_status_data(target);
	s_race2 = status_get_race2(src);
	t_race2 = status_get_race2(target);
	s_defele = (tsd) ? (enum e_element)status_get_element(src) : ELE_NONE;

	// When the attacker is a monster, then all bonuses on BF_WEAPON will work and no bonuses on BF_MAGIC
	// Does not impact the attack type
	if (src && src->type == BL_MOB && battle_config.cardfix_monster_physical) {
		flag |= BF_WEAPON;
		flag &= ~BF_MAGIC;
	}

//Official servers apply the cardfix value on a base of 1000 and round down the reduction/increase
#define APPLY_CARDFIX(damage, fix) { (damage) = (damage) - (int64)(((damage) * (1000 - max(0, fix))) / 1000); }

	switch( attack_type ) {
		case BF_MAGIC:
			// Affected by attacker ATK bonuses
			if( sd && !nk[NK_IGNOREATKCARD] ) {
				int32 race2_val = 0;

				for (const auto &raceit : t_race2)
					race2_val += sd->indexed_bonus.magic_addrace2[raceit];
				cardfix = cardfix * (100 + sd->indexed_bonus.magic_addrace[tstatus->race] + sd->indexed_bonus.magic_addrace[RC_ALL] + race2_val) / 100;
				if( !nk[NK_IGNOREELEMENT] ) { // Affected by Element modifier bonuses
					cardfix = cardfix * (100 + sd->indexed_bonus.magic_addele[tstatus->def_ele] + sd->indexed_bonus.magic_addele[ELE_ALL] +
						sd->indexed_bonus.magic_addele_script[tstatus->def_ele] + sd->indexed_bonus.magic_addele_script[ELE_ALL]) / 100;
					cardfix = cardfix * (100 + sd->indexed_bonus.magic_atk_ele[rh_ele] + sd->indexed_bonus.magic_atk_ele[ELE_ALL]) / 100;
				}
				cardfix = cardfix * (100 + sd->indexed_bonus.magic_addsize[tstatus->size] + sd->indexed_bonus.magic_addsize[SZ_ALL]) / 100;
				cardfix = cardfix * (100 + sd->indexed_bonus.magic_addclass[tstatus->class_] + sd->indexed_bonus.magic_addclass[CLASS_ALL]) / 100;
				for (const auto &it : sd->add_mdmg) {
					if (it.id == t_class) {
						cardfix = cardfix * (100 + it.val) / 100;
						break;
					}
				}
				APPLY_CARDFIX(damage, cardfix);
			}

			// Affected by target DEF bonuses
			if( tsd && !nk[NK_IGNOREDEFCARD] ) {
				cardfix = 1000; // reset var for target

				if( !nk[NK_IGNOREELEMENT] ) { // Affected by Element modifier bonuses
					int ele_fix = tsd->indexed_bonus.subele[rh_ele] + tsd->indexed_bonus.subele[ELE_ALL] + tsd->indexed_bonus.subele_script[rh_ele] + tsd->indexed_bonus.subele_script[ELE_ALL];

					for (const auto &it : tsd->subele2) {
						if (it.ele != ELE_ALL && it.ele != rh_ele)
							continue;
						if (!(((it.flag)&flag)&BF_WEAPONMASK &&
							((it.flag)&flag)&BF_RANGEMASK &&
							((it.flag)&flag)&BF_SKILLMASK))
							continue;
						ele_fix += it.rate;
					}
					if (s_defele != ELE_NONE)
						ele_fix += tsd->indexed_bonus.magic_subdefele[s_defele] + tsd->indexed_bonus.magic_subdefele[ELE_ALL];
					cardfix = cardfix * (100 - ele_fix) / 100;
				}
				cardfix = cardfix * (100 - tsd->indexed_bonus.subsize[sstatus->size] - tsd->indexed_bonus.subsize[SZ_ALL]) / 100;
				cardfix = cardfix * (100 - tsd->indexed_bonus.magic_subsize[sstatus->size] - tsd->indexed_bonus.magic_subsize[SZ_ALL]) / 100;

				int32 race_fix = 0;

				for (const auto &raceit : s_race2)
					race_fix += tsd->indexed_bonus.subrace2[raceit];
				cardfix = cardfix * (100 - race_fix) / 100;
				race_fix = tsd->indexed_bonus.subrace[sstatus->race] + tsd->indexed_bonus.subrace[RC_ALL];
				for (const auto &it : tsd->subrace3) {
					if (it.race != RC_ALL && it.race != sstatus->race)
						continue;
					if (!(((it.flag)&flag)&BF_WEAPONMASK &&
						((it.flag)&flag)&BF_RANGEMASK &&
						((it.flag)&flag)&BF_SKILLMASK))
						continue;
					race_fix += it.rate;
				}
				cardfix = cardfix * (100 - race_fix) / 100;
				cardfix = cardfix * (100 - tsd->indexed_bonus.subclass[sstatus->class_] - tsd->indexed_bonus.subclass[CLASS_ALL]) / 100;

				for (const auto &it : tsd->add_mdef) {
					if (it.id == s_class) {
						cardfix = cardfix * (100 - it.val) / 100;
						break;
					}
				}
#ifndef RENEWAL
				//It was discovered that ranged defense also counts vs magic! [Skotlex]
				if( flag&BF_SHORT )
					cardfix = cardfix * (100 - tsd->bonus.near_attack_def_rate) / 100;
				else if (!nk[NK_IGNORELONGCARD])
					cardfix = cardfix * (100 - tsd->bonus.long_attack_def_rate) / 100;
#endif
				cardfix = cardfix * (100 - tsd->bonus.magic_def_rate) / 100;

				if( tsd->sc.getSCE(SC_MDEF_RATE) )
					cardfix = cardfix * (100 - tsd->sc.getSCE(SC_MDEF_RATE)->val1) / 100;
				APPLY_CARDFIX(damage, cardfix);
			}
			break;

		case BF_WEAPON:
			// Affected by attacker ATK bonuses
			if( sd && !nk[NK_IGNOREATKCARD] && (left&2) ) {
				short cardfix_ = 1000;

				if( sd->state.arrow_atk ) { // Ranged attack
					cardfix = cardfix * (100 + sd->right_weapon.addrace[tstatus->race] + sd->indexed_bonus.arrow_addrace[tstatus->race] +
						sd->right_weapon.addrace[RC_ALL] + sd->indexed_bonus.arrow_addrace[RC_ALL]) / 100;
					if( !nk[NK_IGNOREELEMENT] ) { // Affected by Element modifier bonuses
						int ele_fix = sd->right_weapon.addele[tstatus->def_ele] + sd->indexed_bonus.arrow_addele[tstatus->def_ele] +
							sd->right_weapon.addele[ELE_ALL] + sd->indexed_bonus.arrow_addele[ELE_ALL];

						for (const auto &it : sd->right_weapon.addele2) {
							if (it.ele != ELE_ALL && it.ele != tstatus->def_ele)
								continue;
							if (!(((it.flag)&flag)&BF_WEAPONMASK &&
								((it.flag)&flag)&BF_RANGEMASK &&
								((it.flag)&flag)&BF_SKILLMASK))
								continue;
							ele_fix += it.rate;
						}
						cardfix = cardfix * (100 + ele_fix) / 100;
					}
					cardfix = cardfix * (100 + sd->right_weapon.addsize[tstatus->size] + sd->indexed_bonus.arrow_addsize[tstatus->size] +
						sd->right_weapon.addsize[SZ_ALL] + sd->indexed_bonus.arrow_addsize[SZ_ALL]) / 100;

					int32 race_fix = 0;

					for (const auto &raceit : t_race2)
						race_fix += sd->right_weapon.addrace2[raceit];
					cardfix = cardfix * (100 + race_fix) / 100;
					cardfix = cardfix * (100 + sd->right_weapon.addclass[tstatus->class_] + sd->indexed_bonus.arrow_addclass[tstatus->class_] +
						sd->right_weapon.addclass[CLASS_ALL] + sd->indexed_bonus.arrow_addclass[CLASS_ALL]) / 100;
				} else { // Melee attack
					int skill = 0;

					// Calculates each right & left hand weapon bonuses separatedly
					if( !battle_config.left_cardfix_to_right ) {
						// Right-handed weapon
						cardfix = cardfix * (100 + sd->right_weapon.addrace[tstatus->race] + sd->right_weapon.addrace[RC_ALL]) / 100;
						if( !nk[NK_IGNOREELEMENT] ) { // Affected by Element modifier bonuses
							int ele_fix = sd->right_weapon.addele[tstatus->def_ele] + sd->right_weapon.addele[ELE_ALL];

							for (const auto &it : sd->right_weapon.addele2) {
								if (it.ele != ELE_ALL && it.ele != tstatus->def_ele)
									continue;
								if (!(((it.flag)&flag)&BF_WEAPONMASK &&
									((it.flag)&flag)&BF_RANGEMASK &&
									((it.flag)&flag)&BF_SKILLMASK))
									continue;
								ele_fix += it.rate;
							}
							cardfix = cardfix * (100 + ele_fix) / 100;
						}
						cardfix = cardfix * (100 + sd->right_weapon.addsize[tstatus->size] + sd->right_weapon.addsize[SZ_ALL]) / 100;
						for (const auto &raceit : t_race2)
							cardfix = cardfix * (100 + sd->right_weapon.addrace2[raceit]) / 100;
						cardfix = cardfix * (100 + sd->right_weapon.addclass[tstatus->class_] + sd->right_weapon.addclass[CLASS_ALL]) / 100;

						if( left&1 ) { // Left-handed weapon
							cardfix_ = cardfix_ * (100 + sd->left_weapon.addrace[tstatus->race] + sd->left_weapon.addrace[RC_ALL]) / 100;
							if( !nk[NK_IGNOREELEMENT] ) { // Affected by Element modifier bonuses
								int ele_fix_lh = sd->left_weapon.addele[tstatus->def_ele] + sd->left_weapon.addele[ELE_ALL];

								for (const auto &it : sd->left_weapon.addele2) {
									if (it.ele != ELE_ALL && it.ele != tstatus->def_ele)
										continue;
									if (!(((it.flag)&flag)&BF_WEAPONMASK &&
										((it.flag)&flag)&BF_RANGEMASK &&
										((it.flag)&flag)&BF_SKILLMASK))
										continue;
									ele_fix_lh += it.rate;
								}
								cardfix_ = cardfix_ * (100 + ele_fix_lh) / 100;
							}
							cardfix_ = cardfix_ * (100 + sd->left_weapon.addsize[tstatus->size] + sd->left_weapon.addsize[SZ_ALL]) / 100;
							for (const auto &raceit : t_race2)
								cardfix_ = cardfix_ * (100 + sd->left_weapon.addrace2[raceit]) / 100;
							cardfix_ = cardfix_ * (100 + sd->left_weapon.addclass[tstatus->class_] + sd->left_weapon.addclass[CLASS_ALL]) / 100;
						}
					}
					// Calculates right & left hand weapon as unity
					else {
						//! CHECKME: If 'left_cardfix_to_right' is yes, doesn't need to check NK_IGNOREELEMENT?
						//if( !nk[&]K_IGNOREELEMENT) ) { // Affected by Element modifier bonuses
							int ele_fix = sd->right_weapon.addele[tstatus->def_ele] + sd->left_weapon.addele[tstatus->def_ele]
										+ sd->right_weapon.addele[ELE_ALL] + sd->left_weapon.addele[ELE_ALL];

							for (const auto &it : sd->right_weapon.addele2) {
								if (it.ele != ELE_ALL && it.ele != tstatus->def_ele)
									continue;
								if (!(((it.flag)&flag)&BF_WEAPONMASK &&
									((it.flag)&flag)&BF_RANGEMASK &&
									((it.flag)&flag)&BF_SKILLMASK))
									continue;
								ele_fix += it.rate;
							}
							for (const auto &it : sd->left_weapon.addele2) {
								if (it.ele != ELE_ALL && it.ele != tstatus->def_ele)
									continue;
								if (!(((it.flag)&flag)&BF_WEAPONMASK &&
									((it.flag)&flag)&BF_RANGEMASK &&
									((it.flag)&flag)&BF_SKILLMASK))
									continue;
								ele_fix += it.rate;
							}
							cardfix = cardfix * (100 + ele_fix) / 100;
						//}
						cardfix = cardfix * (100 + sd->right_weapon.addrace[tstatus->race] + sd->left_weapon.addrace[tstatus->race] +
							sd->right_weapon.addrace[RC_ALL] + sd->left_weapon.addrace[RC_ALL]) / 100;
						cardfix = cardfix * (100 + sd->right_weapon.addsize[tstatus->size] + sd->left_weapon.addsize[tstatus->size] +
							sd->right_weapon.addsize[SZ_ALL] + sd->left_weapon.addsize[SZ_ALL]) / 100;
						for (const auto &raceit : t_race2)
							cardfix = cardfix * (100 + sd->right_weapon.addrace2[raceit] + sd->left_weapon.addrace2[raceit]) / 100;
						cardfix = cardfix * (100 + sd->right_weapon.addclass[tstatus->class_] + sd->left_weapon.addclass[tstatus->class_] +
							sd->right_weapon.addclass[CLASS_ALL] + sd->left_weapon.addclass[CLASS_ALL]) / 100;
					}
#ifndef RENEWAL
					if( sd->status.weapon == W_KATAR && (skill = pc_checkskill(sd,ASC_KATAR)) > 0 ) // Adv. Katar Mastery functions similar to a +%ATK card on official [helvetica]
						cardfix = cardfix * (100 + (10 + 2 * skill)) / 100;
#endif
				}

				//! CHECKME: These right & left hand weapon ignores 'left_cardfix_to_right'?
				for (const auto &it : sd->right_weapon.add_dmg) {
					if (it.id == t_class) {
						cardfix = cardfix * (100 + it.val) / 100;
						break;
					}
				}
				if( left&1 ) {
					for (const auto &it : sd->left_weapon.add_dmg) {
						if (it.id == t_class) {
							cardfix_ = cardfix_ * (100 + it.val) / 100;
							break;
						}
					}
				}
#ifndef RENEWAL
				if (flag & BF_SHORT)
					cardfix = cardfix * (100 + sd->bonus.short_attack_atk_rate) / 100;
				if( flag&BF_LONG )
					cardfix = cardfix * (100 + sd->bonus.long_attack_atk_rate) / 100;
#endif
				if (left&1) {
					APPLY_CARDFIX(damage, cardfix_);
				} else {
					APPLY_CARDFIX(damage, cardfix);
				}
			}
			// Affected by target DEF bonuses
			else if( tsd && !nk[NK_IGNOREDEFCARD] && !(left&2) ) {
				if( !nk[NK_IGNOREELEMENT] ) { // Affected by Element modifier bonuses
					int ele_fix = tsd->indexed_bonus.subele[rh_ele] + tsd->indexed_bonus.subele[ELE_ALL] + tsd->indexed_bonus.subele_script[rh_ele] + tsd->indexed_bonus.subele_script[ELE_ALL];

					for (const auto &it : tsd->subele2) {
						if (it.ele != ELE_ALL && it.ele != rh_ele)
							continue;
						if (!(((it.flag)&flag)&BF_WEAPONMASK &&
							((it.flag)&flag)&BF_RANGEMASK &&
							((it.flag)&flag)&BF_SKILLMASK))
							continue;
						ele_fix += it.rate;
					}
					cardfix = cardfix * (100 - ele_fix) / 100;

					if( left&1 && lh_ele != rh_ele ) {
						int ele_fix_lh = tsd->indexed_bonus.subele[lh_ele] + tsd->indexed_bonus.subele[ELE_ALL] + tsd->indexed_bonus.subele_script[lh_ele] + tsd->indexed_bonus.subele_script[ELE_ALL];

						for (const auto &it : tsd->subele2) {
							if (it.ele != ELE_ALL && it.ele != lh_ele)
								continue;
							if (!(((it.flag)&flag)&BF_WEAPONMASK &&
								((it.flag)&flag)&BF_RANGEMASK &&
								((it.flag)&flag)&BF_SKILLMASK))
								continue;
							ele_fix_lh += it.rate;
						}
						cardfix = cardfix * (100 - ele_fix_lh) / 100;
					}

					cardfix = cardfix * (100 - tsd->indexed_bonus.subdefele[s_defele] - tsd->indexed_bonus.subdefele[ELE_ALL]) / 100;
				}

				int32 race_fix = 0;

				cardfix = cardfix * (100 - tsd->indexed_bonus.subsize[sstatus->size] - tsd->indexed_bonus.subsize[SZ_ALL]) / 100;
				cardfix = cardfix * (100 - tsd->indexed_bonus.weapon_subsize[sstatus->size] - tsd->indexed_bonus.weapon_subsize[SZ_ALL]) / 100;
				for (const auto &raceit : s_race2)
					race_fix += tsd->indexed_bonus.subrace2[raceit];
				cardfix = cardfix * (100 - race_fix) / 100;
				race_fix = tsd->indexed_bonus.subrace[sstatus->race] + tsd->indexed_bonus.subrace[RC_ALL];
				for (const auto &it : tsd->subrace3) {
					if (it.race != RC_ALL && it.race != sstatus->race)
						continue;
					if (!(((it.flag)&flag)&BF_WEAPONMASK &&
						((it.flag)&flag)&BF_RANGEMASK &&
						((it.flag)&flag)&BF_SKILLMASK))
						continue;
					race_fix += it.rate;
				}
				cardfix = cardfix * (100 - race_fix) / 100;
				cardfix = cardfix * (100 - tsd->indexed_bonus.subclass[sstatus->class_] - tsd->indexed_bonus.subclass[CLASS_ALL]) / 100;
				for (const auto &it : tsd->add_def) {
					if (it.id == s_class) {
						cardfix = cardfix * (100 - it.val) / 100;
						break;
					}
				}
				if( flag&BF_SHORT )
					cardfix = cardfix * (100 - tsd->bonus.near_attack_def_rate) / 100;
				else if (!nk[NK_IGNORELONGCARD])	// BF_LONG (there's no other choice)
					cardfix = cardfix * (100 - tsd->bonus.long_attack_def_rate) / 100;
				if( tsd->sc.getSCE(SC_DEF_RATE) )
					cardfix = cardfix * (100 - tsd->sc.getSCE(SC_DEF_RATE)->val1) / 100;
				APPLY_CARDFIX(damage, cardfix);
			}
			break;

		case BF_MISC:
			// Affected by target DEF bonuses
			if( tsd && !nk[NK_IGNOREDEFCARD] ) {
				if( !nk[NK_IGNOREELEMENT] ) { // Affected by Element modifier bonuses
					int ele_fix = tsd->indexed_bonus.subele[rh_ele] + tsd->indexed_bonus.subele[ELE_ALL] + tsd->indexed_bonus.subele_script[rh_ele] + tsd->indexed_bonus.subele_script[ELE_ALL];

					for (const auto &it : tsd->subele2) {
						if (it.ele != rh_ele)
							continue;
						if (!(((it.flag)&flag)&BF_WEAPONMASK &&
							((it.flag)&flag)&BF_RANGEMASK &&
							((it.flag)&flag)&BF_SKILLMASK))
							continue;
						ele_fix += it.rate;
					}
					if (s_defele != ELE_NONE)
						ele_fix += tsd->indexed_bonus.subdefele[s_defele] + tsd->indexed_bonus.subdefele[ELE_ALL];
					cardfix = cardfix * (100 - ele_fix) / 100;
				}
				int race_fix = tsd->indexed_bonus.subrace[sstatus->race] + tsd->indexed_bonus.subrace[RC_ALL];
				for (const auto &it : tsd->subrace3) {
					if (it.race != RC_ALL && it.race != sstatus->race)
						continue;
					if (!(((it.flag)&flag)&BF_WEAPONMASK &&
						((it.flag)&flag)&BF_RANGEMASK &&
						((it.flag)&flag)&BF_SKILLMASK))
						continue;
					race_fix += it.rate;
				}
				cardfix = cardfix * (100 - race_fix) / 100;
				cardfix = cardfix * (100 - tsd->indexed_bonus.subsize[sstatus->size] - tsd->indexed_bonus.subsize[SZ_ALL]) / 100;
				race_fix = 0;
				for (const auto &raceit : s_race2)
					race_fix += tsd->indexed_bonus.subrace2[raceit];
				cardfix = cardfix * (100 - race_fix) / 100;
				cardfix = cardfix * (100 - tsd->indexed_bonus.subclass[sstatus->class_] - tsd->indexed_bonus.subclass[CLASS_ALL]) / 100;
				cardfix = cardfix * (100 - tsd->bonus.misc_def_rate) / 100;
				if( flag&BF_SHORT )
					cardfix = cardfix * (100 - tsd->bonus.near_attack_def_rate) / 100;
				else if (!nk[NK_IGNORELONGCARD])	// BF_LONG (there's no other choice)
					cardfix = cardfix * (100 - tsd->bonus.long_attack_def_rate) / 100;
				APPLY_CARDFIX(damage, cardfix);
			}
			break;
	}

#undef APPLY_CARDFIX

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
				map_session_data *sd = BL_CAST(BL_PC, bl);
				if (!sd)
					return;
				dmg_ori = dmg_new = d->damage + d->damage2;
				if (sd->bonus.absorb_dmg_maxhp) {
					int hp = sd->bonus.absorb_dmg_maxhp * status_get_max_hp(bl) / 100;
					if (dmg_ori > hp)
						dmg_new = dmg_ori - hp;
				}
				if (sd->bonus.absorb_dmg_maxhp2) {
					int hp = sd->bonus.absorb_dmg_maxhp2 * status_get_max_hp(bl) / 100;
					if (dmg_ori > hp) {
						dmg_new = hp;
					}
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
 * Check for active statuses that block damage
 * @param src: Attacker
 * @param target: Target of attack
 * @param sc: Status Change data
 * @param d: Damage data
 * @param damage: Damage received as a reference
 * @param skill_id: Skill ID
 * @param skill_lv: Skill level
 * @return True: Damage inflicted, False: Missed
 **/
bool battle_status_block_damage(struct block_list *src, struct block_list *target, status_change *sc, struct Damage *d, int64 &damage, uint16 skill_id, uint16 skill_lv) {
	if (!src || !target || !sc || !d)
		return true;

	status_change_entry *sce;
	int flag = d->flag;

	// SC Types that must be first because they may or may not block damage
	if ((sce = sc->getSCE(SC_KYRIE)) && damage > 0) {
		sce->val2 -= static_cast<int>(cap_value(damage, INT_MIN, INT_MAX));
		if (flag & BF_WEAPON || skill_id == TF_THROWSTONE) {
			if (sce->val2 >= 0)
				damage = 0;
			else
				damage = -sce->val2;
		}
		if ((--sce->val3) <= 0 || (sce->val2 <= 0) || skill_id == AL_HOLYLIGHT)
			status_change_end(target, SC_KYRIE);
	}

	int element;
	if (flag & BF_WEAPON) {
		struct status_data *sstatus = status_get_status_data(src);
		if(sstatus->rhw.ele == ELE_NEUTRAL && sstatus->lhw.ele > sstatus->rhw.ele)
			element = battle_get_weapon_element(d, src, target, skill_id, skill_lv, EQI_HAND_L, false);
		else
			element = battle_get_weapon_element(d, src, target, skill_id, skill_lv, EQI_HAND_R, false);
	} else if(flag & BF_MAGIC)
		element = battle_get_magic_element(src, target, skill_id, skill_lv, d->miscflag);
	else
		element = battle_get_misc_element(src, target, skill_id, skill_lv, d->miscflag);
	
	switch( element ){
		case ELE_NEUTRAL:
			if( sc->getSCE( SC_IMMUNE_PROPERTY_NOTHING ) ){
				damage = 0;
				return false;
			}
			break;
		case ELE_WATER:
			if( sc->getSCE( SC_IMMUNE_PROPERTY_WATER ) ){
				damage = 0;
				return false;
			}
			break;
		case ELE_EARTH:
			if( sc->getSCE( SC_IMMUNE_PROPERTY_GROUND ) ){
				damage = 0;
				return false;
			}
			break;
		case ELE_FIRE:
			if( sc->getSCE( SC_IMMUNE_PROPERTY_FIRE ) ){
				damage = 0;
				return false;
			}
			break;
		case ELE_WIND:
			if( sc->getSCE( SC_IMMUNE_PROPERTY_WIND ) ){
				damage = 0;
				return false;
			}
			break;
		case ELE_DARK:
			if( sc->getSCE( SC_IMMUNE_PROPERTY_DARKNESS ) ){
				damage = 0;
				return false;
			}
			break;
		case ELE_HOLY:
			if( sc->getSCE( SC_IMMUNE_PROPERTY_SAINT ) ){
				damage = 0;
				return false;
			}
			break;
		case ELE_POISON:
			if( sc->getSCE( SC_IMMUNE_PROPERTY_POISON ) ){
				damage = 0;
				return false;
			}
			break;
		case ELE_GHOST:
			if( sc->getSCE( SC_IMMUNE_PROPERTY_TELEKINESIS ) ){
				damage = 0;
				return false;
			}
			break;
		case ELE_UNDEAD:
			if( sc->getSCE( SC_IMMUNE_PROPERTY_UNDEAD ) ){
				damage = 0;
				return false;
			}
			break;
	}

	if ((sce = sc->getSCE(SC_P_ALTER)) && damage > 0) {
		clif_specialeffect(target, EF_GUARD, AREA);
		sce->val3 -= static_cast<int>(cap_value(damage, INT_MIN, INT_MAX));
		if (sce->val3 >= 0)
			damage = 0;
		else
			damage = -sce->val3;
		if (sce->val3 <= 0)
			status_change_end(target, SC_P_ALTER);
	}

	if ((sce = sc->getSCE(SC_TUNAPARTY)) && damage > 0) {
		sce->val2 -= static_cast<int>(cap_value(damage, INT_MIN, INT_MAX));
		if (sce->val2 >= 0)
			damage = 0;
		else
			damage = -sce->val2;
		if (sce->val2 <= 0)
			status_change_end(target, SC_TUNAPARTY);
	}

	if ((sce = sc->getSCE(SC_DIMENSION1)) && damage > 0) {
		sce->val2 -= static_cast<int>(cap_value(damage, INT_MIN, INT_MAX));
		if (sce->val2 <= 0)
			status_change_end(target, SC_DIMENSION1);
		return false;
	}

	if ((sce = sc->getSCE(SC_DIMENSION2)) && damage > 0) {
		sce->val2 -= static_cast<int>(cap_value(damage, INT_MIN, INT_MAX));
		if (sce->val2 <= 0)
			status_change_end(target, SC_DIMENSION2);
		return false;
	}

	if ((sce = sc->getSCE(SC_GUARDIAN_S)) && damage > 0) {
		clif_specialeffect(target, EF_GUARD3, AREA);// Not official but we gotta show some way the barrier is working. [Rytech]
		sce->val2 -= static_cast<int>(cap_value(damage, INT_MIN, INT_MAX));
		if (flag & BF_WEAPON) {
			if (sce->val2 >= 0)
				damage = 0;
			else
				damage = -sce->val2;
		}
		if (sce->val2 <= 0)
			status_change_end(target, SC_GUARDIAN_S);
	}

	// Weapon Blocking can be triggered while the above statuses are active.
	if ((sce = sc->getSCE(SC_WEAPONBLOCKING)) && flag & (BF_SHORT | BF_WEAPON) && rnd() % 100 < sce->val2) {
		clif_skill_nodamage(target, src, GC_WEAPONBLOCKING, sce->val1, 1);
		sc_start(src, target, SC_WEAPONBLOCK_ON, 100, src->id, skill_get_time2(GC_WEAPONBLOCKING, sce->val1));
		d->dmg_lv = ATK_BLOCK;
		return false;
	}

	if (damage == 0)
		return false;

	// ATK_BLOCK Type
	if ((sce = sc->getSCE(SC_SAFETYWALL)) && (flag&(BF_SHORT | BF_MAGIC)) == BF_SHORT) {
		std::shared_ptr<s_skill_unit_group> group = skill_id2group(sce->val3);

		if (group) {
			d->dmg_lv = ATK_BLOCK;

			switch (sce->val2) {
			case MG_SAFETYWALL:
				if (--group->val2 <= 0) {
					skill_delunitgroup(group);
					break;
				}
#ifdef RENEWAL
				if (group->val3 - damage > 0)
					group->val3 -= static_cast<int>(cap_value(damage, INT_MIN, INT_MAX));
				else
					skill_delunitgroup(group);
#endif
				break;
			case MH_STEINWAND:
				if (--group->val2 <= 0) {
					skill_delunitgroup(group);
					break;
				}
				if (group->val3 - damage > 0)
					group->val3 -= static_cast<int>(cap_value(damage, INT_MIN, INT_MAX));
				else
					skill_delunitgroup(group);
				break;
			}
			return false;
		}
		status_change_end(target, SC_SAFETYWALL);
	}

	if ((sc->getSCE(SC_PNEUMA) && (flag&(BF_MAGIC | BF_LONG)) == BF_LONG) ||
#ifdef RENEWAL
		(sc->getSCE(SC_BASILICA_CELL)
#else
		(sc->getSCE(SC_BASILICA)
#endif
		&& !status_bl_has_mode(src, MD_STATUSIMMUNE) && skill_id != SP_SOULEXPLOSION) ||
		(sc->getSCE(SC_ZEPHYR) && !(flag&BF_MAGIC && skill_id) && !(skill_get_inf(skill_id)&(INF_GROUND_SKILL | INF_SELF_SKILL))) ||
		sc->getSCE(SC__MANHOLE) ||
		sc->getSCE(SC_KINGS_GRACE) ||
		sc->getSCE(SC_GRAVITYCONTROL)
		)
	{
		d->dmg_lv = ATK_BLOCK;
		return false;
	}

	if (sc->getSCE(SC_WHITEIMPRISON)) { // Gravitation and Pressure do damage without removing the effect
		if (skill_id == MG_NAPALMBEAT ||
			skill_id == MG_SOULSTRIKE ||
			skill_id == WL_SOULEXPANSION ||
			skill_id == AG_SOUL_VC_STRIKE ||
			(skill_id && skill_get_ele(skill_id, skill_lv) == ELE_GHOST) ||
			(skill_id == 0 && (status_get_status_data(src))->rhw.ele == ELE_GHOST))
		{
			if (skill_id == WL_SOULEXPANSION)
				damage *= 2; // If used against a player in White Imprison, the skill deals double damage.
			status_change_end(target, SC_WHITEIMPRISON); // Those skills do damage and removes effect
		} else {
			d->dmg_lv = ATK_BLOCK;
			return false;
		}
	}

	if ((sce = sc->getSCE(SC_MILLENNIUMSHIELD)) && sce->val2 > 0 && damage > 0) {
		sce->val3 -= static_cast<int>(cap_value(damage, INT_MIN, INT_MAX)); // absorb damage
		d->dmg_lv = ATK_BLOCK;
		if (sce->val3 <= 0) { // Shield Down
			sce->val2--;
			if (sce->val2 > 0) {
				clif_millenniumshield( *target, sce->val2 );
				sce->val3 = 1000; // Next shield
			} else
				status_change_end(target, SC_MILLENNIUMSHIELD); // All shields down
			status_change_start(src, target, SC_STUN, 10000, 0, 0, 0, 0, 1000, SCSTART_NOTICKDEF);
		}
		return false;
	}

	// ATK_MISS Type
	if ((sce = sc->getSCE(SC_AUTOGUARD)) && flag&BF_WEAPON && rnd() % 100 < sce->val2 && !skill_get_inf2(skill_id, INF2_IGNOREAUTOGUARD)) {
		status_change_entry *sce_d = sc->getSCE(SC_DEVOTION);
		block_list *d_bl;
		int delay;

		// different delay depending on skill level [celest]
		if (sce->val1 <= 5)
			delay = 300;
		else if (sce->val1 > 5 && sce->val1 <= 9)
			delay = 200;
		else
			delay = 100;

		map_session_data *sd = map_id2sd(target->id);

		if (sd && pc_issit(sd))
			pc_setstand(sd, true);
		if (sce_d && (d_bl = map_id2bl(sce_d->val1)) &&
			((d_bl->type == BL_MER && ((TBL_MER*)d_bl)->master && ((TBL_MER*)d_bl)->master->bl.id == target->id) ||
			(d_bl->type == BL_PC && ((TBL_PC*)d_bl)->devotion[sce_d->val2] == target->id)) &&
			check_distance_bl(target, d_bl, sce_d->val3))
		{ //If player is target of devotion, show guard effect on the devotion caster rather than the target
			clif_skill_nodamage(d_bl, d_bl, CR_AUTOGUARD, sce->val1, 1);
			unit_set_walkdelay(d_bl, gettick(), delay, 1);
			d->dmg_lv = ATK_MISS;
			return false;
		} else {
			clif_skill_nodamage(target, target, CR_AUTOGUARD, sce->val1, 1);
			unit_set_walkdelay(target, gettick(), delay, 1);
#ifdef RENEWAL
			if (sc->getSCE(SC_SHRINK))
				sc_start(target, src, SC_STUN, 50, skill_lv, skill_get_time2(skill_id, skill_lv));
#else
			if (sc->getSCE(SC_SHRINK) && rnd() % 100 < 5 * sce->val1)
				skill_blown(target, src, skill_get_blewcount(CR_SHRINK, 1), -1, BLOWN_NONE);
#endif
			d->dmg_lv = ATK_MISS;
			return false;
		}
	}

	if (sc->getSCE(SC_NEUTRALBARRIER) && ((flag&(BF_LONG|BF_MAGIC)) == BF_LONG
#ifndef RENEWAL
		|| skill_id == CR_ACIDDEMONSTRATION
#endif
		)) {
		d->dmg_lv = ATK_MISS;
		return false;
	}

	// ATK_DEF Type
	if ((sce = sc->getSCE(SC_LIGHTNINGWALK)) && !(flag & BF_MAGIC) && flag&BF_LONG && rnd() % 100 < sce->val1) {
		uint8 dir = map_calc_dir(target, src->x, src->y);

		if (unit_movepos(target, src->x - dirx[dir], src->y - diry[dir], 1, 1)) {
			clif_blown(target);
			unit_setdir(target, dir);
		}
		d->dmg_lv = ATK_DEF;
		status_change_end(target, SC_LIGHTNINGWALK);
		return false;
	}

	// Other
	if ((sc->getSCE(SC_HERMODE) && flag&BF_MAGIC) ||
		(sc->getSCE(SC_TATAMIGAESHI) && (flag&(BF_MAGIC | BF_LONG)) == BF_LONG) ||
		(sc->getSCE(SC_MEIKYOUSISUI) && rnd() % 100 < 40)) // custom value
		return false;

	if ((sce = sc->getSCE(SC_PARRYING)) && flag&BF_WEAPON && skill_id != WS_CARTTERMINATION && rnd() % 100 < sce->val2) {
		clif_skill_nodamage(target, target, LK_PARRYING, sce->val1, 1);

		if (skill_id == LK_PARRYING) {
			unit_data *ud = unit_bl2ud(target);

			if (ud != nullptr) // Delay the next attack
				ud->attackabletime = gettick() + status_get_adelay(target);
		}
		return false;
	}

	if (sc->getSCE(SC_DODGE) && (flag&BF_LONG || sc->getSCE(SC_SPURT)) && (skill_id != NPC_EARTHQUAKE || (skill_id == NPC_EARTHQUAKE && flag & NPC_EARTHQUAKE_FLAG)) && rnd() % 100 < 20) {
		map_session_data *sd = map_id2sd(target->id);

		if (sd && pc_issit(sd))
			pc_setstand(sd, true); //Stand it to dodge.
		clif_skill_nodamage(target, target, TK_DODGE, 1, 1);
		sc_start4(src, target, SC_COMBO, 100, TK_JUMPKICK, src->id, 1, 0, 2000);
		return false;
	}

	if ((sce = sc->getSCE(SC_KAUPE)) && (skill_id != NPC_EARTHQUAKE || (skill_id == NPC_EARTHQUAKE && flag & NPC_EARTHQUAKE_FLAG)) && rnd() % 100 < sce->val2) { //Kaupe blocks damage (skill or otherwise) from players, mobs, homuns, mercenaries.
		clif_specialeffect(target, EF_STORMKICK4, AREA);
		//Shouldn't end until Breaker's non-weapon part connects.
#ifndef RENEWAL
		if (skill_id != ASC_BREAKER || !(flag&BF_WEAPON))
#endif
			if (--sce->val3 <= 0) //We make it work like Safety Wall, even though it only blocks 1 time.
				status_change_end(target, SC_KAUPE);
		return false;
	}

	if (flag&BF_MAGIC && (sce = sc->getSCE(SC_PRESTIGE)) && rnd() % 100 < sce->val2) {
		clif_specialeffect(target, EF_STORMKICK4, AREA); // Still need confirm it.
		return false;
	}

	if (((sce = sc->getSCE(SC_UTSUSEMI)) || sc->getSCE(SC_BUNSINJYUTSU)) && flag&BF_WEAPON && !skill_get_inf2(skill_id, INF2_IGNORECICADA)) {
		skill_additional_effect(src, target, skill_id, skill_lv, flag, ATK_BLOCK, gettick());
		if (!status_isdead(src))
			skill_counter_additional_effect(src, target, skill_id, skill_lv, flag, gettick());
		if (sce) {
			clif_specialeffect(target, EF_STORMKICK4, AREA);
			skill_blown(src, target, sce->val3, -1, BLOWN_NONE);
		}
		//Both need to be consumed if they are active.
		if (sce && --sce->val2 <= 0)
			status_change_end(target, SC_UTSUSEMI);
		if ((sce = sc->getSCE(SC_BUNSINJYUTSU)) && --sce->val2 <= 0)
			status_change_end(target, SC_BUNSINJYUTSU);
		return false;
	}
	
	if ((sce = sc->getSCE(SC_DAMAGE_HEAL))) {
		if (damage > 0 && (flag & sce->val2)) {
			int32 heal = static_cast<int32>( i64min( damage, INT32_MAX ) );
			if(flag & BF_WEAPON) {
				clif_specialeffect_value(target, EF_HEAL, heal, AREA);
			} else {
				clif_specialeffect_value(target, 1143, heal, AREA);
			}
			clif_specialeffect_value(target, EF_GREEN_NUMBER, heal, AREA);
			status_heal(target, damage, 0, 0);
			damage = 0;
			return false;
		}
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
	map_session_data *sd = nullptr, *tsd = BL_CAST(BL_PC, src);
	struct status_change_entry *sce;
	int div_ = d->div_, flag = d->flag;

	nullpo_ret(bl);

	if( !damage )
		return 0;
	if( battle_config.ksprotection && mob_ksprotected(src, bl) )
		return 0;

	if( map_getcell(bl->m, bl->x, bl->y, CELL_CHKMAELSTROM) && skill_id && skill_get_type(skill_id) != BF_MISC
		&& skill_get_casttype(skill_id) == CAST_GROUND )
		return 0;

	if (bl->type == BL_PC) {
		sd=(map_session_data *)bl;
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

	switch (skill_id) {
#ifndef RENEWAL
		case PA_PRESSURE:
		case HW_GRAVITATION:
#endif
		case SP_SOULEXPLOSION:
			// Adjust these based on any possible PK damage rates.
			if (battle_config.pk_mode == 1 && map_getmapflag(bl->m, MF_PVP) > 0)
				damage = battle_calc_pk_damage(*src, *bl, damage, skill_id, flag);

			return damage; //These skills bypass everything else.
	}

	status_change* tsc = status_get_sc(bl); //check target status

	// Nothing can reduce the damage, but Safety Wall and Millennium Shield can block it completely.
	// So can defense sphere's but what the heck is that??? [Rytech]
	if (skill_id == SJ_NOVAEXPLOSING && !(tsc && (tsc->getSCE(SC_SAFETYWALL) || tsc->getSCE(SC_MILLENNIUMSHIELD)))) {
		// Adjust this based on any possible PK damage rates.
		if (battle_config.pk_mode == 1 && map_getmapflag(bl->m, MF_PVP) > 0)
			damage = battle_calc_pk_damage(*src, *bl, damage, skill_id, flag);

		return damage;
	}

	if( tsc && tsc->count ) {
		// Damage increasing effects
#ifdef RENEWAL // Flat +400% damage from melee
		if (tsc->getSCE(SC_KAITE) && (flag&(BF_SHORT|BF_MAGIC)) == BF_SHORT)
			damage *= 4;
#endif

		if (tsc->getSCE(SC_GROUNDGRAVITY) && flag&(BF_MAGIC|BF_WEAPON))
			damage += damage * 15 / 100;
		if (tsc->getSCE(SC_SHIELDCHAINRUSH))
			damage += damage / 10;

		if (tsc->getSCE(SC_AETERNA) && skill_id != PF_SOULBURN) {
			if (src->type != BL_MER || !skill_id)
				damage *= 2; // Lex Aeterna only doubles damage of regular attacks from mercenaries

#ifndef RENEWAL
			if( skill_id != ASC_BREAKER || !(flag&BF_WEAPON) )
#endif
				status_change_end(bl, SC_AETERNA); //Shouldn't end until Breaker's non-weapon part connects.
		}

#ifdef RENEWAL
		if( tsc->getSCE(SC_RAID) ) {
			if (status_get_class_(bl) == CLASS_BOSS)
				damage += damage * 15 / 100;
			else
				damage += damage * 30 / 100;
		}
#endif

		if( damage ) {
			if( tsc->getSCE(SC_DEEPSLEEP) ) {
				damage += damage / 2; // 1.5 times more damage while in Deep Sleep.
				status_change_end(bl,SC_DEEPSLEEP);
			}
			if( tsd && sd && tsc->getSCE(SC_CRYSTALIZE) && flag&BF_WEAPON ) {
				switch(tsd->status.weapon) {
					case W_MACE:
					case W_2HMACE:
					case W_1HAXE:
					case W_2HAXE:
						damage += damage / 2;
						break;
					case W_MUSICAL:
					case W_WHIP:
						if(!tsd->state.arrow_atk)
							break;
						[[fallthrough]];
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
			if( tsc->getSCE(SC_VOICEOFSIREN) )
				status_change_end(bl,SC_VOICEOFSIREN);
		}

		if (tsc->getSCE(SC_SOUNDOFDESTRUCTION))
			damage *= 2;
		if (tsc->getSCE(SC_DARKCROW) && (flag&(BF_SHORT|BF_MAGIC)) == BF_SHORT) {
			int bonus = tsc->getSCE(SC_DARKCROW)->val2;
		if( tsc->getSCE(SC_BURNT) && status_get_element(src) == ELE_FIRE )
			damage += damage * 666 / 100; //Custom value

			if (status_get_class_(bl) == CLASS_BOSS)
				bonus /= 2;

			damage += damage * bonus / 100;
		}
		if (tsc->getSCE(SC_HOLY_OIL) && (flag&(BF_LONG|BF_WEAPON)) == (BF_LONG|BF_WEAPON))
			damage += damage * (3 * tsc->getSCE(SC_HOLY_OIL)->val1) / 100;

		if( tsc->getSCE( SC_RUSH_QUAKE1 ) && ( flag&BF_WEAPON ) == BF_WEAPON ){
			damage += damage * 50 / 100;
		}

		if (tsc->getSCE(SC_SHADOW_SCAR)) // !TODO: Need official adjustment for this too.
			damage += damage * (3 * tsc->getSCE(SC_SHADOW_SCAR)->val1) / 100;

		// Damage reductions
		// Assumptio increases DEF on RE mode, otherwise gives a reduction on the final damage. [Igniz]
#ifndef RENEWAL
		if( tsc->getSCE(SC_ASSUMPTIO) ) {
			if( map_flag_vs(bl->m) )
				damage = (int64)damage*2/3; //Receive 66% damage
			else
				damage /= 2; //Receive 50% damage
		}
#endif

		if (tsc->getSCE(SC_DEFENDER) &&
			skill_id != NJ_ZENYNAGE && skill_id != KO_MUCHANAGE &&
#ifdef RENEWAL
			((flag&(BF_LONG|BF_WEAPON)) == (BF_LONG|BF_WEAPON) || skill_id == GN_FIRE_EXPANSION_ACID))
#else
			(flag&(BF_LONG|BF_WEAPON)) == (BF_LONG|BF_WEAPON))
#endif
			damage -= damage * tsc->getSCE(SC_DEFENDER)->val2 / 100;

		if(tsc->getSCE(SC_ADJUSTMENT) && (flag&(BF_LONG|BF_WEAPON)) == (BF_LONG|BF_WEAPON))
			damage -= damage * 20 / 100;

		if(tsc->getSCE(SC_FOGWALL) && skill_id != RK_DRAGONBREATH && skill_id != RK_DRAGONBREATH_WATER && skill_id != NPC_DRAGONBREATH) {
			if(flag&BF_SKILL) //25% reduction
				damage -= damage * 25 / 100;
			else if ((flag&(BF_LONG|BF_WEAPON)) == (BF_LONG|BF_WEAPON))
				damage /= 4; //75% reduction
		}

		if (tsc->getSCE(SC_SPORE_EXPLOSION) && (flag & BF_LONG) == BF_LONG)
			damage += damage * (status_get_class(bl) == CLASS_BOSS ? 5 : 10) / 100;

		if(tsc->getSCE(SC_ARMORCHANGE)) {
			//On official servers, SC_ARMORCHANGE does not change DEF/MDEF but rather increases/decreases the damage
			if(flag&BF_WEAPON)
				damage -= damage * tsc->getSCE(SC_ARMORCHANGE)->val2 / 100;
			else if(flag&BF_MAGIC)
				damage -= damage * tsc->getSCE(SC_ARMORCHANGE)->val3 / 100;
		}

		if(tsc->getSCE(SC_SMOKEPOWDER)) {
			if( (flag&(BF_SHORT|BF_WEAPON)) == (BF_SHORT|BF_WEAPON) )
				damage -= damage * 15 / 100; // 15% reduction to physical melee attacks
			else if( (flag&(BF_LONG|BF_WEAPON)) == (BF_LONG|BF_WEAPON) )
				damage -= damage * 50 / 100; // 50% reduction to physical ranged attacks
		}

		if (tsc->getSCE(SC_WATER_BARRIER))
			damage = damage * 80 / 100; // 20% reduction to all type attacks

		if (tsc->getSCE(SC_SU_STOOP))
			damage -= damage * 90 / 100;

		// Compressed code, fixed by map.hpp [Epoque]
		if (src->type == BL_MOB) {
			std::vector<e_race2> race2 = status_get_race2(src);

			for (const auto &raceit : race2) {
				switch (raceit) {
					case RC2_MANUK:
						if (sce = tsc->getSCE(SC_MANU_DEF))
							damage -= damage * sce->val1 / 100;
						break;
					case RC2_SPLENDIDE:
						if (sce = tsc->getSCE(SC_SPL_DEF))
							damage -= damage * sce->val1 / 100;
						break;
					case RC2_OGH_ATK_DEF:
						if (tsc->getSCE(SC_GLASTHEIM_DEF))
							return 0;
						break;
					case RC2_OGH_HIDDEN:
						if (sce = tsc->getSCE(SC_GLASTHEIM_HIDDEN))
							damage -= damage * sce->val1 / 100;
						break;
					case RC2_BIO5_ACOLYTE_MERCHANT:
						if (sce = tsc->getSCE(SC_LHZ_DUN_N1))
							damage -= damage * sce->val2 / 100;
						break;
					case RC2_BIO5_MAGE_ARCHER:
						if (sce = tsc->getSCE(SC_LHZ_DUN_N2))
							damage -= damage * sce->val2 / 100;
						break;
					case RC2_BIO5_SWORDMAN_THIEF:
						if (sce = tsc->getSCE(SC_LHZ_DUN_N3))
							damage -= damage * sce->val2 / 100;
						break;
					case RC2_BIO5_MVP:
						if (sce = tsc->getSCE(SC_LHZ_DUN_N4))
							damage -= damage * sce->val2 / 100;
						break;
				}
			}
		}

		if((sce=tsc->getSCE(SC_ARMOR)) && //NPC_DEFENDER
			sce->val3&flag && sce->val4&flag)
			damage /= tsc->getSCE(SC_ARMOR)->val2;

		if( tsc->getSCE(SC_ENERGYCOAT) && (skill_id == GN_HELLS_PLANT_ATK ||
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
				status_change_end(bl, SC_ENERGYCOAT);
			damage -= damage * 6 * (1 + per) / 100; //Reduction: 6% + 6% every 20%
		}

		if(tsc->getSCE(SC_GRANITIC_ARMOR))
			damage -= damage * tsc->getSCE(SC_GRANITIC_ARMOR)->val2 / 100;

		if(tsc->getSCE(SC_PAIN_KILLER)) {
			damage -= tsc->getSCE(SC_PAIN_KILLER)->val2;
			damage = i64max(damage, 1);
		}

		if( (sce=tsc->getSCE(SC_MAGMA_FLOW)) && (rnd()%100 <= sce->val2) )
			skill_castend_damage_id(bl,src,MH_MAGMA_FLOW,sce->val1,gettick(),0);

		if( damage > 0 && (sce = tsc->getSCE(SC_STONEHARDSKIN)) ) {
			if( src->type == BL_MOB ) //using explicit call instead break_equip for duration
				sc_start(src,src, SC_STRIPWEAPON, 30, 0, skill_get_time2(RK_STONEHARDSKIN, sce->val1));
			else if (flag&(BF_WEAPON|BF_SHORT))
				skill_break_equip(src,src, EQP_WEAPON, 3000, BCT_SELF);
		}

		if (src->type == BL_PC && tsc->getSCE(SC_GVG_GOLEM)) {
			if (flag&BF_WEAPON)
				damage -= damage * tsc->getSCE(SC_GVG_GOLEM)->val3 / 100;
			if (flag&BF_MAGIC)
				damage -= damage * tsc->getSCE(SC_GVG_GOLEM)->val4 / 100;
		}

#ifdef RENEWAL
		// Renewal: steel body reduces all incoming damage to 1/10 [helvetica]
		if( tsc->getSCE(SC_STEELBODY) )
			damage = damage > 10 ? damage / 10 : 1;
#endif

		//Finally added to remove the status of immobile when Aimed Bolt is used. [Jobbie]
		if( skill_id == RA_AIMEDBOLT && (tsc->getSCE(SC_BITE) || tsc->getSCE(SC_ANKLE) || tsc->getSCE(SC_ELECTRICSHOCKER)) ) {
			status_change_end(bl, SC_BITE);
			status_change_end(bl, SC_ANKLE);
			status_change_end(bl, SC_ELECTRICSHOCKER);
		}

		if (!damage)
			return 0;

		if( sd && (sce = tsc->getSCE(SC_FORCEOFVANGUARD)) && flag&BF_WEAPON && rnd()%100 < sce->val2 )
			pc_addspiritball(sd,skill_get_time(LG_FORCEOFVANGUARD,sce->val1),sce->val3);

		if( sd && (sce = tsc->getSCE(SC_GT_ENERGYGAIN)) && flag&BF_WEAPON && rnd()%100 < sce->val2 ) {
			int spheres = 5;

			if( tsc->getSCE(SC_RAISINGDRAGON) )
				spheres += tsc->getSCE(SC_RAISINGDRAGON)->val1;

			pc_addspiritball(sd, skill_get_time2(SR_GENTLETOUCH_ENERGYGAIN, sce->val1), spheres);
		}

		if (tsc->getSCE(SC_STYLE_CHANGE) && tsc->getSCE(SC_STYLE_CHANGE)->val1 == MH_MD_GRAPPLING) {
			TBL_HOM *hd = BL_CAST(BL_HOM,bl); // We add a sphere for when the Homunculus is being hit

			if (hd && (rnd()%100<50) ) // According to WarpPortal, this is a flat 50% chance
				hom_addspiritball(hd, 10);
		}

		if( tsc->getSCE(SC__DEADLYINFECT) && (flag&(BF_SHORT|BF_MAGIC)) == BF_SHORT && damage > 0 && rnd()%100 < 30 + 10 * tsc->getSCE(SC__DEADLYINFECT)->val1 )
			status_change_spread(bl, src);

	} //End of target SC_ check

	//SC effects from caster side.
	status_change* sc = status_get_sc(src);

	if (sc && sc->count) {
		if (sc->getSCE(SC_BREAKINGLIMIT)) {
			switch (skill_id) {
				case HN_SHIELD_CHAIN_RUSH:
				case HN_DOUBLEBOWLINGBASH:
					damage += damage * 120 / 100; 
					break;
				case HN_MEGA_SONIC_BLOW:
				case HN_SPIRAL_PIERCE_MAX:
					damage *= 2;
					break;
			}
		}

		if (sc->getSCE(SC_RULEBREAK)) {
			switch (skill_id) {
				case HN_METEOR_STORM_BUSTER:
				case HN_GROUND_GRAVITATION:
					damage += damage / 2;
					break;
				case HN_JUPITEL_THUNDER_STORM:
				case HN_JACK_FROST_NOVA:
				case HN_HELLS_DRIVE:
					damage += damage * 70 / 100;
					break;
				case HN_NAPALM_VULCAN_STRIKE:
					damage += damage * 40 / 100;
					break;
			}
		}

		if ((sce = sc->getSCE(SC_BLOODLUST)) && flag&BF_WEAPON && damage > 0 && rnd()%100 < sce->val3)
			status_heal(src, damage * sce->val4 / 100, 0, 3);

		if ((sce = sc->getSCE(SC_BLOODSUCKER)) && flag & BF_WEAPON && damage > 0 && rnd() % 100 < (2 * sce->val1 - 1))
			status_heal(src, damage * sce->val1 / 100, 0, 3);

		if (flag&BF_MAGIC && bl->type == BL_PC && sc->getSCE(SC_GVG_GIANT) && sc->getSCE(SC_GVG_GIANT)->val4)
			damage += damage * sc->getSCE(SC_GVG_GIANT)->val4 / 100;

		// [Epoque]
		if (bl->type == BL_MOB) {
			if ((flag&BF_WEAPON) || (flag&BF_MAGIC)) {
				std::vector<e_race2> race2 = status_get_race2(bl);

				for (const auto &raceit : race2) {
					switch (raceit) {
						case RC2_MANUK:
							if (sce = sc->getSCE(SC_MANU_ATK))
								damage += damage * sce->val1 / 100;
							break;
						case RC2_SPLENDIDE:
							if (sce = sc->getSCE(SC_SPL_ATK))
								damage += damage * sce->val1 / 100;
							break;
						case RC2_OGH_ATK_DEF:
							if (sc->getSCE(SC_GLASTHEIM_ATK))
								damage *= 2;
							break;
						case RC2_BIO5_SWORDMAN_THIEF:
							if (sce = sc->getSCE(SC_LHZ_DUN_N1))
								damage += damage * sce->val1 / 100;
							break;
						case RC2_BIO5_ACOLYTE_MERCHANT:
							if (sce = sc->getSCE(SC_LHZ_DUN_N2))
								damage += damage * sce->val1 / 100;
							break;
						case RC2_BIO5_MAGE_ARCHER:
							if (sce = sc->getSCE(SC_LHZ_DUN_N3))
								damage += damage * sce->val1 / 100;
							break;
						case RC2_BIO5_MVP:
							if (sce = sc->getSCE(SC_LHZ_DUN_N4))
								damage += damage * sce->val1 / 100;
							break;
					}
				}
			}
		}

		if (sc->getSCE(SC_POISONINGWEAPON) && flag&BF_SHORT && damage > 0) {
			damage += damage * 10 / 100;
			if (rnd() % 100 < sc->getSCE(SC_POISONINGWEAPON)->val3)
				sc_start4(src, bl, (sc_type)sc->getSCE(SC_POISONINGWEAPON)->val2, 100, sc->getSCE(SC_POISONINGWEAPON)->val1, 0, 1, 0, (sc->getSCE(SC_POISONINGWEAPON)->val2 == SC_VENOMBLEED ? skill_get_time2(GC_POISONINGWEAPON, 1) : skill_get_time2(GC_POISONINGWEAPON, 2)));
		}

		if( sc->getSCE(SC__DEADLYINFECT) && (flag&(BF_SHORT|BF_MAGIC)) == BF_SHORT && damage > 0 && rnd()%100 < 30 + 10 * sc->getSCE(SC__DEADLYINFECT)->val1 )
			status_change_spread(src, bl);

		if (sc->getSCE(SC_STYLE_CHANGE) && sc->getSCE(SC_STYLE_CHANGE)->val1 == MH_MD_FIGHTING) {
			TBL_HOM *hd = BL_CAST(BL_HOM,src); //when attacking

			if (hd && (rnd()%100<50) ) hom_addspiritball(hd, 10); // According to WarpPortal, this is a flat 50% chance
		}

		if (flag & BF_WEAPON && (sce = sc->getSCE(SC_ADD_ATK_DAMAGE)))
			damage += damage * sce->val1 / 100;
		if (flag & BF_MAGIC && (sce = sc->getSCE(SC_ADD_MATK_DAMAGE)))
			damage += damage * sce->val1 / 100;
		if (sc->getSCE(SC_DANCEWITHWUG) && (flag&(BF_LONG|BF_WEAPON)) == (BF_LONG|BF_WEAPON))
			damage += damage * sc->getSCE(SC_DANCEWITHWUG)->val1 / 100;
		if (sc->getSCE(SC_UNLIMITEDHUMMINGVOICE) && flag&BF_MAGIC)
			damage += damage * sc->getSCE(SC_UNLIMITEDHUMMINGVOICE)->val3 / 100;

		if (tsd && (sce = sc->getSCE(SC_SOULREAPER))) {
			if (rnd()%100 < sce->val2 && tsd->soulball < MAX_SOUL_BALL) {
				clif_specialeffect(src, 1208, AREA);
				pc_addsoulball(tsd, 5 + 3 * pc_checkskill(tsd, SP_SOULENERGY));
			}
		}
	} //End of caster SC_ check

	//PK damage rates
	if (battle_config.pk_mode == 1 && map_getmapflag(bl->m, MF_PVP) > 0)
		damage = battle_calc_pk_damage(*src, *bl, damage, skill_id, flag);

	if(battle_config.skill_min_damage && damage > 0 && damage < div_) {
		if ((flag&BF_WEAPON && battle_config.skill_min_damage&1)
			|| (flag&BF_MAGIC && battle_config.skill_min_damage&2)
			|| (flag&BF_MISC && battle_config.skill_min_damage&4)
		)
			damage = div_;
	}

	if (sd && pc_ismadogear(sd)) {
		pc_overheat(*sd, (battle_get_weapon_element(d, src, bl, skill_id, skill_lv, EQI_HAND_R, false) == ELE_FIRE ? 3 : 1));
	}

	// Target status (again), required for RELIEVE
	sc = status_get_sc(bl);

	// !TODO: Should RELIEVE needed to be down here or after some other check? Should the skill be independent of damagetaken from mob_db.yml?
	if (sc && sc->count) {
		if ((sce = sc->getSCE(SC_RELIEVE_ON)) && !sc->getSCE(SC_RELIEVE_OFF))
			damage = i64max((damage - damage * sce->val2 / 100), 1);
	}

	if (bl->type == BL_MOB) { // Reduces damage received for Green Aura MVP
		mob_data *md = BL_CAST(BL_MOB, bl);

		if (md && md->damagetaken != 100)
			damage = i64max(damage * md->damagetaken / 100, 1);
	}
	
	if (tsc && tsc->count) {
		if (!battle_status_block_damage(src, bl, tsc, d, damage, skill_id, skill_lv)) // Statuses that reduce damage to 0.
			return 0;
	}

	return damage;
}

/**
 * Determines whether battleground target can be hit
 * @param src: Source of attack
 * @param bl: Target of attack
 * @param skill_id: Skill ID used
 * @param flag: Special flags
 * @return Can be hit (true) or can't be hit (false)
 */
bool battle_can_hit_bg_target(struct block_list *src, struct block_list *bl, uint16 skill_id, int flag)
{
	struct mob_data* md = BL_CAST(BL_MOB, bl);
	struct unit_data *ud = unit_bl2ud(bl);

	if (ud && ud->immune_attack)
		return false;
	if (md && md->bg_id) {
		if (status_bl_has_mode(bl, MD_SKILLIMMUNE) && flag&BF_SKILL) //Skill immunity.
			return false;
		if (src->type == BL_PC) {
			map_session_data *sd = map_id2sd(src->id);

			if (sd && sd->bg_id == md->bg_id)
				return false;
		}
	}
	return true;
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

	if (!battle_can_hit_bg_target(src, bl, skill_id, flag))
		return 0;

	if(skill_get_inf2(skill_id, INF2_IGNOREBGREDUCTION))
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

	damage = i64max(damage,1); //min 1 damage
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
	struct unit_data *ud = unit_bl2ud(bl);
	int class_ = status_get_class(bl);

	if (ud && ud->immune_attack)
		return false;
	if(md && (md->guardian_data || md->special_state.ai == AI_GUILD)) {
		if ((status_bl_has_mode(bl,MD_SKILLIMMUNE) || (class_ == MOBID_EMPERIUM && !skill_get_inf2(skill_id, INF2_TARGETEMPERIUM))) && flag&BF_SKILL) //Skill immunity.
			return false;
		if( src->type != BL_MOB || mob_is_clone( ((struct mob_data*)src)->mob_id ) ){
			auto g = src->type == BL_PC ? ((TBL_PC *)src)->guild : guild_search(status_get_guild_id(src));

			if (class_ == MOBID_EMPERIUM && (!g || guild_checkskill(g->guild, GD_APPROVAL) <= 0 ))
				return false;

			if (g != nullptr) {
				if (battle_config.guild_max_castles && guild_checkcastles(g->guild)>=battle_config.guild_max_castles)
					return false; // [MouseJstr]

				if (md->special_state.ai == AI_GUILD && g->guild.guild_id == md->master_id)
					return false;
			}
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

	if (skill_get_inf2(skill_id, INF2_IGNOREGVGREDUCTION)) //Skills with no gvg damage reduction.
		return damage;

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
	damage = i64max(damage,1);
	return damage;
}

/**
 * Calculates PK related damage adjustments (between players only).
 * @param src: Source object
 * @param bl: Target object
 * @param damage: Damage being done
 * @param skill_id: Skill used
 * @param flag: Battle flag type
 * @return Modified damage
 */
int64 battle_calc_pk_damage(block_list &src, block_list &bl, int64 damage, uint16 skill_id, int flag) {
	if (damage == 0) // No reductions to make.
		return 0;

	if (battle_config.pk_mode == 0) // PK mode is disabled.
		return damage;

	if (src.type == BL_PC && bl.type == BL_PC) {
		if (flag & BF_SKILL) { //Skills get a different reduction than non-skills. [Skotlex]
			if (flag & BF_WEAPON)
				damage = damage * battle_config.pk_weapon_damage_rate / 100;
			if (flag & BF_MAGIC)
				damage = damage * battle_config.pk_magic_damage_rate / 100;
			if (flag & BF_MISC)
				damage = damage * battle_config.pk_misc_damage_rate / 100;
		} else { //Normal attacks get reductions based on range.
			if (flag & BF_SHORT)
				damage = damage * battle_config.pk_short_damage_rate / 100;
			if (flag & BF_LONG)
				damage = damage * battle_config.pk_long_damage_rate / 100;
		}
	}

	return i64max(damage, 1);
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
int64 battle_addmastery(map_session_data *sd,struct block_list *target,int64 dmg,int type)
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
	if( (skill = pc_checkskill(sd, RA_RANGERMAIN)) > 0 && (status->race == RC_BRUTE || status->race == RC_PLAYER_DORAM || status->race == RC_PLANT || status->race == RC_FISH) )
		damage += (skill * 5);
	if( (skill = pc_checkskill(sd,NC_RESEARCHFE)) > 0 && (status->def_ele == ELE_FIRE || status->def_ele == ELE_EARTH) )
		damage += (skill * 10);

	damage += (15 * pc_checkskill(sd, NC_MADOLICENCE)); // Attack bonus is granted even without the Madogear

	if((skill = pc_checkskill(sd,HT_BEASTBANE)) > 0 && (status->race == RC_INSECT || status->race == RC_BRUTE || status->race == RC_PLAYER_DORAM) ) {
		damage += (skill * 4);
		if (sd->sc.getSCE(SC_SPIRIT) && sd->sc.getSCE(SC_SPIRIT)->val2 == SL_HUNTER)
			damage += sd->status.str;
	}

#ifdef RENEWAL
	//Weapon Research bonus applies to all weapons
	if((skill = pc_checkskill(sd,BS_WEAPONRESEARCH)) > 0)
		damage += (skill * 2);
#endif

	if ((skill = pc_checkskill(sd, NV_BREAKTHROUGH)) > 0)
		damage += 15 * skill + (skill > 4 ? 25 : 0);

	// Kagerou/Oboro Spirit Charm bonus
	if (sd->spiritcharm >= MAX_SPIRITCHARM) {
		if ((sd->spiritcharm_type == CHARM_TYPE_FIRE && status->def_ele == ELE_EARTH) ||
			(sd->spiritcharm_type == CHARM_TYPE_WATER && status->def_ele == ELE_FIRE) ||
			(sd->spiritcharm_type == CHARM_TYPE_LAND && status->def_ele == ELE_WIND) ||
			(sd->spiritcharm_type == CHARM_TYPE_WIND && status->def_ele == ELE_WATER))
			damage += damage * 30 / 100;
	}

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
			if((skill = pc_checkskill(sd,SM_TWOHAND)) > 0)
				damage += (skill * 4);
			break;
		case W_1HSPEAR:
		case W_2HSPEAR:
			if((skill = pc_checkskill(sd,KN_SPEARMASTERY)) > 0) {
				if(!pc_isriding(sd) && !pc_isridingdragon(sd))
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
			[[fallthrough]];
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

/** Calculates overrefine damage bonus and weapon related bonuses (unofficial)
* @param sd Player
* @param damage Current damage
* @param lr_type EQI_HAND_L:left-hand weapon, EQI_HAND_R:right-hand weapon
*/
static void battle_add_weapon_damage(map_session_data *sd, int64 *damage, int lr_type) {
	if (!sd)
		return;
	//rodatazone says that Overrefine bonuses are part of baseatk
	//Here we also apply the weapon_damage_rate bonus so it is correctly applied on left/right hands.
	if (lr_type == EQI_HAND_L) {
		if (sd->left_weapon.overrefine)
			(*damage) = (*damage) + rnd() % sd->left_weapon.overrefine + 1;
		if (sd->indexed_bonus.weapon_damage_rate[sd->weapontype2])
			(*damage) += (*damage) * sd->indexed_bonus.weapon_damage_rate[sd->weapontype2] / 100;
	}
	else if (lr_type == EQI_HAND_R) {
		if (sd->right_weapon.overrefine)
			(*damage) = (*damage) + rnd() % sd->right_weapon.overrefine + 1;
		if (sd->indexed_bonus.weapon_damage_rate[sd->weapontype1])
			(*damage) += (*damage) * sd->indexed_bonus.weapon_damage_rate[sd->weapontype1] / 100;
	}
}

#ifdef RENEWAL
static int battle_calc_sizefix(int64 damage, map_session_data *sd, unsigned char t_size, unsigned char weapon_type, short flag)
{
	if (sd && !sd->special_state.no_sizefix && !flag) // Size fix only for players
		damage = damage * (weapon_type == EQI_HAND_L ? sd->left_weapon.atkmods[t_size] : sd->right_weapon.atkmods[t_size]) / 100;

	return (int)cap_value(damage, INT_MIN, INT_MAX);
}

/**
 * Calculates renewal Variance, OverUpgradeBonus, and SizePenaltyMultiplier of weapon damage parts for player
 * @param src Block list of attacker
 * @param tstatus Target's status data
 * @param wa Weapon attack data
 * @param sd Player
 * @return Base weapon damage
 */
static int battle_calc_base_weapon_attack(struct block_list *src, struct status_data *tstatus, struct weapon_atk *wa, map_session_data *sd, bool critical)
{
	struct status_data *status = status_get_status_data(src);
	uint8 type = (wa == &status->lhw)?EQI_HAND_L:EQI_HAND_R;
	uint16 atkmin = (type == EQI_HAND_L)?status->watk2:status->watk;
	uint16 atkmax = atkmin;
	int64 damage = atkmin;
	bool weapon_perfection = false;
	status_change *sc = status_get_sc(src);

	if (sd && sd->equip_index[type] >= 0 && sd->inventory_data[sd->equip_index[type]]) {
		short base_stat;

		switch (sd->status.weapon) {
			case W_BOW:
			case W_MUSICAL:
			case W_WHIP:
			case W_REVOLVER:
			case W_RIFLE:
			case W_GATLING:
			case W_SHOTGUN:
			case W_GRENADE:
				if (pc_checkskill(sd, SU_SOULATTACK) > 0)
					base_stat = status->str;
				else
					base_stat = status->dex;
				break;
			default:
				base_stat = status->str;
				break;
		}

		float variance = 5.0f * wa->atk * wa->wlv / 100.0f;
		float base_stat_bonus = wa->atk * base_stat / 200.0f;

		atkmin = max(0, (int)(atkmin - variance + base_stat_bonus));
		atkmax = min(UINT16_MAX, (int)(atkmax + variance + base_stat_bonus));

		if ((sc && sc->getSCE(SC_MAXIMIZEPOWER)) || critical == true)
			damage = atkmax;
		else
			damage = rnd_value(atkmin, atkmax);
	}

	if (sc && sc->getSCE(SC_WEAPONPERFECTION))
		weapon_perfection = true;

	battle_add_weapon_damage(sd, &damage, type);

	damage = battle_calc_sizefix(damage, sd, tstatus->size, type, weapon_perfection);

	return (int)cap_value(damage, INT_MIN, INT_MAX);
}
#endif

/*==========================================
 * Calculates the standard damage of a normal attack assuming it hits
 * This applies to pre-renewal and non-sd in renewal
 *------------------------------------------
 * Pass damage2 as nullptr to not calc it.
 * Flag values (see e_base_damage_flag):
 * &1 : Critical hit
 * &2 : Arrow attack
 * &4 : Skill is Magic Crasher
 * &8 : Skip target size adjustment (Weapon Perfection)
 * &16: Arrow attack but BOW, REVOLVER, RIFLE, SHOTGUN, GATLING or GRENADE type weapon not equipped (i.e. shuriken, kunai and venom knives not affected by DEX)
 *
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static int64 battle_calc_base_damage(struct block_list *src, struct status_data *status, struct weapon_atk *wa, status_change *sc, unsigned short t_size, int flag)
{
	unsigned int atkmin = 0, atkmax = 0;
	short type = 0;
	int64 damage = 0;
	map_session_data *sd = nullptr;

	nullpo_retr(damage, src);

	sd = BL_CAST(BL_PC, src);

	if (!sd) { //Mobs/Pets
#ifndef RENEWAL
		if (sc != nullptr && sc->getSCE(SC_CHANGE) != nullptr)
			return status->matk_max; // [Aegis] simply uses raw max matk for base damage when Mental Charge active
#endif
		if(flag&BDMG_MAGIC) {
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

		if (!(flag&BDMG_CRIT) || (flag&BDMG_ARROW)) { //Normal attacks
			atkmin = status->dex;

			if (sd->equip_index[type] >= 0 && sd->inventory_data[sd->equip_index[type]] && sd->inventory_data[sd->equip_index[type]]->type == IT_WEAPON)
				atkmin = atkmin*(80 + sd->inventory_data[sd->equip_index[type]]->weapon_level*20)/100;

			if (atkmin > atkmax)
				atkmin = atkmax;

			if(flag&BDMG_ARROW) { //Bows
				atkmin = atkmin*atkmax/100;
				if (atkmin > atkmax)
					atkmax = atkmin;
			}
		}
	}

	if (sc && sc->getSCE(SC_MAXIMIZEPOWER))
		atkmin = atkmax;

	//Weapon Damage calculation
	if (!(flag&BDMG_CRIT))
		damage = (atkmax>atkmin? rnd()%(atkmax-atkmin):0)+atkmin;
	else
		damage = atkmax;

	if (sd) {
		//rodatazone says the range is 0~arrow_atk-1 for non crit
		if (flag&BDMG_ARROW && sd->bonus.arrow_atk)
			damage += ( (flag&BDMG_CRIT) ? sd->bonus.arrow_atk : rnd()%sd->bonus.arrow_atk );

		// Size fix only for players
		if (!(sd->special_state.no_sizefix || (flag&BDMG_NOSIZE)))
			damage = damage * (type == EQI_HAND_L ? sd->left_weapon.atkmods[t_size] : sd->right_weapon.atkmods[t_size]) / 100;
	} else if (src->type == BL_ELEM) {
		status_change *ele_sc = status_get_sc(src);
		int ele_class = status_get_class(src);

		if (ele_sc) {
			switch (ele_class) {
			case ELEMENTALID_AGNI_S:
			case ELEMENTALID_AGNI_M:
			case ELEMENTALID_AGNI_L:
			case ELEMENTALID_ARDOR:
				if (ele_sc->getSCE(SC_FIRE_INSIGNIA) && ele_sc->getSCE(SC_FIRE_INSIGNIA)->val1 == 1)
					damage += damage * 20 / 100;
				break;
			case ELEMENTALID_AQUA_S:
			case ELEMENTALID_AQUA_M:
			case ELEMENTALID_AQUA_L:
			case ELEMENTALID_DILUVIO:
				if (ele_sc->getSCE(SC_WATER_INSIGNIA) && ele_sc->getSCE(SC_WATER_INSIGNIA)->val1 == 1)
					damage += damage * 20 / 100;
				break;
			case ELEMENTALID_VENTUS_S:
			case ELEMENTALID_VENTUS_M:
			case ELEMENTALID_VENTUS_L:
			case ELEMENTALID_PROCELLA:
				if (ele_sc->getSCE(SC_WIND_INSIGNIA) && ele_sc->getSCE(SC_WIND_INSIGNIA)->val1 == 1)
					damage += damage * 20 / 100;
				break;
			case ELEMENTALID_TERA_S:
			case ELEMENTALID_TERA_M:
			case ELEMENTALID_TERA_L:
			case ELEMENTALID_TERREMOTUS:
			case ELEMENTALID_SERPENS:
				if (ele_sc->getSCE(SC_EARTH_INSIGNIA) && ele_sc->getSCE(SC_EARTH_INSIGNIA)->val1 == 1)
					damage += damage * 20 / 100;
				break;
			}
		}
	}

	//Finally, add baseatk
	if(flag&BDMG_MAGIC)
		damage += status->matk_min;
	else
		damage += status->batk;

	if (sd)
		battle_add_weapon_damage(sd, &damage, type);

#ifdef RENEWAL
	if (flag&BDMG_CRIT)
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
void battle_consume_ammo(map_session_data*sd, int skill, int lv)
{
	int qty = 1;

	if( sd == nullptr ){
		return;
	}

	if (!battle_config.arrow_decrement)
		return;

	if (skill) {
		qty = skill_get_ammo_qty(skill, lv);
		if (!qty) qty = 1;

		if( skill == NW_MAGAZINE_FOR_ONE && sd->weapontype1 == W_GATLING ){
			qty += 4;
		}
	}

	if (sd->equip_index[EQI_AMMO] >= 0) //Qty check should have been done in skill_check_condition
		pc_delitem(sd,sd->equip_index[EQI_AMMO],qty,0,1,LOG_TYPE_CONSUME);

	sd->state.arrow_atk = 0;
}

static int battle_range_type(struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv)
{
	// [Akinari] , [Xynvaroth]: Traps are always short range.
	if (skill_get_inf2(skill_id, INF2_ISTRAP))
		return BF_SHORT;

	switch (skill_id) {
		case AC_SHOWER:
		case AM_DEMONSTRATION:
			// When monsters use Arrow Shower or Bomb, it is always short range
			if (src->type == BL_MOB)
				return BF_SHORT;
			break;
#ifdef RENEWAL
		case KN_BRANDISHSPEAR:
		// Renewal changes to ranged physical damage
#endif
		case SR_RAMPAGEBLASTER:
		case BO_ACIDIFIED_ZONE_WATER_ATK:
		case BO_ACIDIFIED_ZONE_FIRE_ATK:
		case BO_ACIDIFIED_ZONE_GROUND_ATK:
		case BO_ACIDIFIED_ZONE_WIND_ATK:
		case NW_THE_VIGILANTE_AT_NIGHT:
			return BF_LONG;
		case NJ_KIRIKAGE: // Cast range mimics NJ_SHADOWJUMP but damage is considered melee
		case GC_CROSSIMPACT: // Cast range is 7 cells and player jumps to target but skill is considered melee
		case DK_SERVANT_W_PHANTOM: // 9 cell cast range.
		case SHC_SAVAGE_IMPACT: // 7 cell cast range.
		case SHC_FATAL_SHADOW_CROW: // 9 cell cast range.
		case MT_RUSH_QUAKE: // 9 cell cast range.
		case ABC_UNLUCKY_RUSH: // 7 cell cast range.
		case MH_THE_ONE_FIGHTER_RISES: // 7 cell cast range.
		//case ABC_DEFT_STAB: // 2 cell cast range???
		case NPC_MAXPAIN_ATK:
			return BF_SHORT;
		case CD_PETITIO: { // Skill range is 2 but damage is melee with books and ranged with mace.
			map_session_data *sd = BL_CAST(BL_PC, src);

			if (sd && (sd->status.weapon == W_MACE || sd->status.weapon == W_2HMACE))
				return BF_LONG;

			break;
		}
		case DK_HACKANDSLASHER:
		case DK_HACKANDSLASHER_ATK: {
			map_session_data* sd = BL_CAST( BL_PC, src );

			if( sd != nullptr && ( sd->status.weapon == W_1HSPEAR || sd->status.weapon == W_2HSPEAR ) ){
				return BF_LONG;
			}

			break;
		}
	}

	//Skill Range Criteria
	if (battle_config.skillrange_by_distance &&
		(src->type&battle_config.skillrange_by_distance)
	) { //based on distance between src/target [Skotlex]
		if (check_distance_bl(src, target, 3))
			return BF_SHORT;
		return BF_LONG;
	}

	//based on used skill's range
	if (skill_get_range2(src, skill_id, skill_lv, true) < 4)
		return BF_SHORT;
	return BF_LONG;
}

static int battle_blewcount_bonus(map_session_data *sd, uint16 skill_id)
{
	if (sd->skillblown.empty())
		return 0;
	//Apply the bonus blewcount. [Skotlex]
	for (const auto &it : sd->skillblown) {
		if (it.id == skill_id)
			return it.val;
	}
	return 0;
}

static enum e_skill_damage_type battle_skill_damage_type( struct block_list* bl ){
	switch( bl->type ){
		case BL_PC:
			return SKILLDMG_PC;
		case BL_MOB:
			if( status_get_class_(bl) == CLASS_BOSS ){
				return SKILLDMG_BOSS;
			}else{
				return SKILLDMG_MOB;
			}
		default:
			return SKILLDMG_OTHER;
	}
}

/**
 * Gets skill damage rate from a skill (based on skill_damage_db.txt)
 * @param src
 * @param target
 * @param skill_id
 * @return Skill damage rate
 */
static int battle_skill_damage_skill(struct block_list *src, struct block_list *target, uint16 skill_id) {
	std::shared_ptr<s_skill_db> skill = skill_db.find(skill_id);

	if (!skill || !skill->damage.map)
		return 0;

	s_skill_damage *damage = &skill->damage;

	//check the adjustment works for specified type
	if (!(damage->caster&src->type))
		return 0;

	map_data *mapdata = map_getmapdata(src->m);

	if ((damage->map&1 && (!mapdata->getMapFlag(MF_PVP) && !mapdata_flag_gvg2(mapdata) && !mapdata->getMapFlag(MF_BATTLEGROUND) && !mapdata->getMapFlag(MF_SKILL_DAMAGE) && !mapdata->getMapFlag(MF_RESTRICTED))) ||
		(damage->map&2 && mapdata->getMapFlag(MF_PVP)) ||
		(damage->map&4 && mapdata_flag_gvg2(mapdata)) ||
		(damage->map&8 && mapdata->getMapFlag(MF_BATTLEGROUND)) ||
		(damage->map&16 && mapdata->getMapFlag(MF_SKILL_DAMAGE)) ||
		(damage->map&mapdata->zone && mapdata->getMapFlag(MF_RESTRICTED)))
	{
		return damage->rate[battle_skill_damage_type(target)];
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
	map_data *mapdata = map_getmapdata(src->m);

	if (!mapdata || !mapdata->getMapFlag(MF_SKILL_DAMAGE))
		return 0;

	int rate = 0;

	// Damage rate for all skills at this map
	if (mapdata->damage_adjust.caster&src->type)
		rate = mapdata->damage_adjust.rate[battle_skill_damage_type(target)];

	if (mapdata->skill_damage.empty())
		return rate;

	// Damage rate for specified skill at this map
	if (mapdata->skill_damage.find(skill_id) != mapdata->skill_damage.end() && mapdata->skill_damage[skill_id].caster&src->type) {
		rate += mapdata->skill_damage[skill_id].rate[battle_skill_damage_type(target)];
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

/**
 * Calculates Minstrel/Wanderer bonus for Chorus skills.
 * @param sd: Player who has Chorus skill active
 * @return Bonus value based on party count
 */
int battle_calc_chorusbonus(map_session_data *sd) {
#ifdef RENEWAL // No bonus in renewal
	return 0;
#endif

	int members = 0;

	if (!sd || !sd->status.party_id)
		return 0;

	members = party_foreachsamemap(party_sub_count_class, sd, 0, MAPID_THIRDMASK, MAPID_MINSTRELWANDERER);

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

		if (su && su->group && (su->group->skill_id == NPC_REVERBERATION || su->group->skill_id == WM_POEMOFNETHERWORLD))
			return true;
	}

	if(status_has_mode(tstatus,MD_IGNOREMELEE) && (flag&(BF_WEAPON|BF_SHORT)) == (BF_WEAPON|BF_SHORT) )
		return true;
	if(status_has_mode(tstatus,MD_IGNOREMAGIC) && flag&(BF_MAGIC) )
		return true;
	if(status_has_mode(tstatus,MD_IGNORERANGED) && (flag&(BF_WEAPON|BF_LONG)) == (BF_WEAPON|BF_LONG) )
		return true;
	if(status_has_mode(tstatus,MD_IGNOREMISC) && flag&(BF_MISC) )
		return true;

	status_change* tsc = status_get_sc(target);
	if (tsc && tsc->getSCE(SC_INVINCIBLE))
		return true;

	return false;
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
	if(src != nullptr) {
		struct status_data *sstatus = status_get_status_data(src);
		map_session_data *sd = BL_CAST(BL_PC, src);

		return ((sd && sd->state.arrow_atk) || (!sd && ((skill_id && skill_get_ammotype(skill_id)) || sstatus->rhw.range>3))
			|| skill_id == HT_FREEZINGTRAP || (skill_id == HT_PHANTASMIC) || (skill_id == GS_GROUNDDRIFT));
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
	if(src != nullptr) {
		map_session_data *sd = BL_CAST(BL_PC, src);

		//Skills ALWAYS use ONLY your right-hand weapon (tested on Aegis 10.2)
		if(!skill_id && sd && sd->weapontype1 == W_FIST && sd->weapontype2 != W_FIST)
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
	if(src != nullptr) {
		//Skills ALWAYS use ONLY your right-hand weapon (tested on Aegis 10.2)
		if(!skill_id) {
			map_session_data *sd = BL_CAST(BL_PC, src);

			if (sd) {
				if (sd->weapontype1 == W_FIST && sd->weapontype2 != W_FIST)
					return true;
				if (sd->status.weapon == W_KATAR)
					return true;
			}

			struct status_data *sstatus = status_get_status_data(src);

			if (sstatus->lhw.atk)
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
static bool is_attack_critical(struct Damage* wd, struct block_list *src, struct block_list *target, int skill_id, int skill_lv, bool first_call)
{
	if (!first_call)
		return (wd->type == DMG_CRITICAL || wd->type == DMG_MULTI_HIT_CRITICAL);

	if (skill_id == NPC_CRITICALSLASH || skill_id == LG_PINPOINTATTACK) //Always critical skills
		return true;

	if( skill_id && !skill_get_nk(skill_id,NK_CRITICAL) )
		return false;

	struct status_data *sstatus = status_get_status_data(src);

	if( sstatus->cri )
	{
		map_session_data *sd = BL_CAST(BL_PC, src);

		if(wd->type == DMG_MULTI_HIT){	//Multiple Hit Attack Skills.
			if(pc_checkskill(sd,GS_CHAINACTION) && !skill_get_nk(GS_CHAINACTION,NK_CRITICAL)) //Chain Action
				return false;

			if(pc_checkskill(sd,TF_DOUBLE) && !skill_get_nk(TF_DOUBLE,NK_CRITICAL)) //Double Attack
				return false;
		}

		struct status_data *tstatus = status_get_status_data(target);
		status_change *sc = status_get_sc(src);
		status_change *tsc = status_get_sc(target);
		map_session_data *tsd = BL_CAST(BL_PC, target);
		short cri = sstatus->cri;

		if (sd) {
			cri += sd->indexed_bonus.critaddrace[tstatus->race] + sd->indexed_bonus.critaddrace[RC_ALL];
			if(!skill_id && is_skill_using_arrow(src, skill_id)) {
				cri += sd->bonus.arrow_cri;
				cri += sd->bonus.critical_rangeatk;
			}
		}

		if(sc && sc->getSCE(SC_CAMOUFLAGE))
			cri += 100 * min(10,sc->getSCE(SC_CAMOUFLAGE)->val3); //max 100% (1K)

		//The official equation is *2, but that only applies when sd's do critical.
		//Therefore, we use the old value 3 on cases when an sd gets attacked by a mob
		cri -= tstatus->luk * ((!sd && tsd) ? 3 : 2);

		if( tsc && tsc->getSCE(SC_SLEEP) )
			cri *= 2;

		switch(skill_id) {
			case 0:
				if(sc && !sc->getSCE(SC_AUTOCOUNTER))
					break;
				clif_specialeffect(src, EF_AUTOCOUNTER, AREA);
				status_change_end(src, SC_AUTOCOUNTER);
				[[fallthrough]];
			case KN_AUTOCOUNTER:
				if(battle_config.auto_counter_type &&
					(battle_config.auto_counter_type&src->type))
					return true;
				else
					cri *= 2;
				break;
			case SN_SHARPSHOOTING:
			case MA_SHARPSHOOTING:
#ifdef RENEWAL
				cri += 300; // !TODO: Confirm new bonus
#else
				cri += 200;
#endif
				break;
			case NJ_KIRIKAGE:
				cri += 250 + 50*skill_lv;
				break;
#ifdef RENEWAL
			case ASC_BREAKER:
#endif
			case GC_CROSSIMPACT:
			case SHC_SAVAGE_IMPACT:
			case SHC_ETERNAL_SLASH:
			case SHC_IMPACT_CRATER:
				cri /= 2;
				break;
			case WH_GALESTORM:
				if (sc && !sc->getSCE(SC_CALAMITYGALE))
					return false;
				break;
			case NW_ONLY_ONE_BULLET:
			case NW_SPIRAL_SHOOTING:
				if( sd == nullptr || sd->weapontype1 != W_RIFLE ){
					return false;
				}
				break;
			case NW_MAGAZINE_FOR_ONE:
				if( sd == nullptr || sd->weapontype1 != W_REVOLVER ){
					return false;
				}
				break;
		}
		if(tsd && tsd->bonus.critical_def)
			cri = cri * ( 100 - tsd->bonus.critical_def ) / 100;
		return (rnd()%1000 < cri);
	}
	return false;
}

/*==========================================================
 * Is the attack piercing? (Investigate/Ice Pick in pre-re)
 *----------------------------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static int is_attack_piercing(struct Damage* wd, struct block_list *src, struct block_list *target, int skill_id, int skill_lv, short weapon_position)
{
	if (skill_id == MO_INVESTIGATE || skill_id == RL_MASS_SPIRAL)
		return 2;

	if(src != nullptr) {
		map_session_data *sd = BL_CAST(BL_PC, src);
		struct status_data *tstatus = status_get_status_data(target);

		if( skill_id != PA_SACRIFICE && skill_id != CR_GRANDCROSS && skill_id != NPC_GRANDDARKNESS && skill_id != PA_SHIELDCHAIN && skill_id != KO_HAPPOKUNAI
#ifndef RENEWAL
			&& !is_attack_critical(wd, src, target, skill_id, skill_lv, false)
#endif
		)
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

static std::bitset<NK_MAX> battle_skill_get_damage_properties(uint16 skill_id, int is_splash)
{
	if (skill_id == 0) {
		if (is_splash) {
			std::bitset<NK_MAX> tmp_nk;

			tmp_nk.set(NK_IGNOREATKCARD);
			tmp_nk.set(NK_IGNOREFLEE);

			return tmp_nk;
		} else
			return 0;
	} else
		return skill_db.find(skill_id)->nk;
}

/*=============================
 * Checks if attack is hitting
 *-----------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static bool is_attack_hitting(struct Damage* wd, struct block_list *src, struct block_list *target, int skill_id, int skill_lv, bool first_call)
{
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);
	status_change *sc = status_get_sc(src);
	status_change *tsc = status_get_sc(target);
	map_session_data *sd = BL_CAST(BL_PC, src);
	std::bitset<NK_MAX> nk = battle_skill_get_damage_properties(skill_id, wd->miscflag);
	short flee, hitrate;

	if (!first_call)
		return (wd->dmg_lv != ATK_FLEE);
	if (is_attack_critical(wd, src, target, skill_id, skill_lv, false))
		return true;
	else if(sd && sd->bonus.perfect_hit > 0 && rnd()%100 < sd->bonus.perfect_hit)
		return true;
	else if (sc && sc->getSCE(SC_FUSION))
		return true;
	else if ((skill_id == AS_SPLASHER || skill_id == GN_SPORE_EXPLOSION) && !wd->miscflag)
		return true;
	else if (skill_id == CR_SHIELDBOOMERANG && sc && sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_CRUSADER )
		return true;
	else if (tsc && tsc->opt1 && tsc->opt1 != OPT1_STONEWAIT && tsc->opt1 != OPT1_BURNING)
		return true;
	else if (nk[NK_IGNOREFLEE])
		return true;

	if( tsc && tsc->getSCE(SC_NEUTRALBARRIER) && (wd->flag&(BF_LONG|BF_MAGIC)) == BF_LONG )
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
	if ((wd->flag&(BF_LONG|BF_MAGIC)) == BF_LONG && !skill_id && tsc && tsc->getSCE(SC_FOGWALL))
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
			case NPC_WATERATTACK:
			case NPC_GROUNDATTACK:
			case NPC_FIREATTACK:
			case NPC_WINDATTACK:
			case NPC_POISONATTACK:
			case NPC_HOLYATTACK:
			case NPC_DARKNESSATTACK:
			case NPC_TELEKINESISATTACK:
			case NPC_UNDEADATTACK:
			case NPC_CHANGEUNDEAD:
			case NPC_EARTHQUAKE:
			case NPC_POISON:
			case NPC_BLINDATTACK:
			case NPC_SILENCEATTACK:
			case NPC_STUNATTACK:
			case NPC_PETRIFYATTACK:
			case NPC_CURSEATTACK:
			case NPC_SLEEPATTACK:
			case NPC_BLEEDING:
			case NPC_BLEEDING2:
				hitrate += hitrate * 20 / 100;
				break;
			case NPC_FIREBREATH:
			case NPC_ICEBREATH:
			case NPC_ICEBREATH2:
			case NPC_THUNDERBREATH:
			case NPC_ACIDBREATH:
			case NPC_DARKNESSBREATH:
				hitrate *= 2;
				break;
			case KN_PIERCE:
			case ML_PIERCE:
				hitrate += hitrate * 5 * skill_lv / 100;
				break;
			case AS_SONICBLOW:
				if(sd && pc_checkskill(sd,AS_SONICACCEL) > 0)
#ifdef RENEWAL
					hitrate += hitrate * 90 / 100;
#else
					hitrate += hitrate * 50 / 100;
#endif
				break;
#ifdef RENEWAL
			case RG_BACKSTAP:
				hitrate += skill_lv; // !TODO: What's the rate increase?
				break;
#endif
			case RK_SONICWAVE:
				hitrate += hitrate * 3 * skill_lv / 100; // !TODO: Confirm the hitrate bonus
				break;
			case MC_CARTREVOLUTION:
			case GN_CART_TORNADO:
			case GN_CARTCANNON:
				if (sd && pc_checkskill(sd, GN_REMODELING_CART))
					hitrate += pc_checkskill(sd, GN_REMODELING_CART) * 4;
				break;
			case LG_BANISHINGPOINT:
				hitrate += 5 * skill_lv;
				break;
			case GC_VENOMPRESSURE:
				hitrate += 10 + 4 * skill_lv;
				break;
			case SC_FATALMENACE:
				if (skill_lv < 6)
					hitrate -= 35 - 5 * skill_lv;
				else if (skill_lv > 6)
					hitrate += 5 * skill_lv - 30;
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
	} else if (sd && wd->type&DMG_MULTI_HIT && wd->div_ == 2) // +1 hit per level of Double Attack on a successful double attack (making sure other multi attack skills do not trigger this) [helvetica]
		hitrate += pc_checkskill(sd,TF_DOUBLE);

	if (sd) {
		int skill = 0;

		// Weaponry Research hidden bonus
		if ((skill = pc_checkskill(sd,BS_WEAPONRESEARCH)) > 0)
			hitrate += hitrate * ( 2 * skill ) / 100;

		if( (sd->status.weapon == W_1HSWORD || sd->status.weapon == W_DAGGER) &&
			(skill = pc_checkskill(sd, GN_TRAINING_SWORD))>0 )
			hitrate += 3 * skill;
	}

	if (sc) {
		if (sc->getSCE(SC_MTF_ASPD))
			hitrate += sc->getSCE(SC_MTF_ASPD)->val2;
		if (sc->getSCE(SC_MTF_ASPD2))
			hitrate += sc->getSCE(SC_MTF_ASPD2)->val2;
	}

	hitrate = cap_value(hitrate, battle_config.min_hitrate, battle_config.max_hitrate);

	if(skill_id == PA_SHIELDCHAIN)
		hitrate += 20; //Rapid Smiting gives a flat +20 hit after the hitrate was capped

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
static bool attack_ignores_def(struct Damage* wd, struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv, short weapon_position)
{
	struct status_data *tstatus = status_get_status_data(target);
	status_change *sc = status_get_sc(src);
	map_session_data *sd = BL_CAST(BL_PC, src);
	std::bitset<NK_MAX> nk = battle_skill_get_damage_properties(skill_id, wd->miscflag);

#ifndef RENEWAL
	if (is_attack_critical(wd, src, target, skill_id, skill_lv, false))
		return true;
	else
#endif
	if (sc && sc->getSCE(SC_FUSION))
		return true;

	if( sd != nullptr ){
		switch( skill_id ){
			case RK_WINDCUTTER:
				if( sd->status.weapon == W_2HSWORD ){
					return true;
				}
				break;
			case NW_THE_VIGILANTE_AT_NIGHT:
				if( sd->status.weapon == W_GATLING ){
					return true;
				}
				break;
			case NW_ONLY_ONE_BULLET:
				if( sd->status.weapon == W_REVOLVER ){
					return true;
				}
				break;
		}
	}

	if (skill_id != CR_GRANDCROSS && skill_id != NPC_GRANDDARKNESS)
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

	return nk[NK_IGNOREDEFENSE] != 0;
}

/**
 * This function lists which skills are unaffected by refine bonus, masteries, Star Crumbs and Spirit Spheres
 * This function is also used to determine if atkpercent applies
 * @param skill_id: Skill being used
 * @param chk_flag: The bonus that is currently being checked for, see e_bonus_chk_flag
 * @return true = bonus applies; false = bonus does not apply
 */
static bool battle_skill_stacks_masteries_vvs(uint16 skill_id, e_bonus_chk_flag chk_flag)
{
	switch (skill_id) {
		// PC skills that are unaffected
		case PA_SHIELDCHAIN:
		case CR_SHIELDBOOMERANG:
		case AM_ACIDTERROR:
		case MO_INVESTIGATE:
		case MO_EXTREMITYFIST:
		case PA_SACRIFICE:
		case RK_DRAGONBREATH:
		case RK_DRAGONBREATH_WATER:
		case NC_SELFDESTRUCTION:
		case LG_SHIELDPRESS:
		case LG_EARTHDRIVE:
		case NPC_DRAGONBREATH:
			return false;
#ifndef RENEWAL
		case LK_SPIRALPIERCE:
			// In Pre-Renewal Spiral Pierce is influenced only by refine bonus and Star Crumbs for players
			if (chk_flag != BCHK_REFINE && chk_flag != BCHK_STAR)
				return false;
#endif
			break;
	}

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
	if(src != nullptr) {
		int eatk = 0;
		struct status_data *status = status_get_status_data(src);
		map_session_data *sd = BL_CAST(BL_PC, src);

		// Add arrow atk if using an applicable skill
		if (sd != nullptr && is_skill_using_arrow(src, skill_id)) {
			int16 ammo_idx = sd->equip_index[EQI_AMMO];
			// Attack of cannon balls is not added to equip attack, it needs to be added by the skills that use them
			if (ammo_idx >= 0 && sd->inventory_data[ammo_idx] != nullptr && sd->inventory_data[ammo_idx]->subtype != AMMO_CANNONBALL)
				eatk += sd->bonus.arrow_atk;
		}

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
int battle_get_weapon_element(struct Damage* wd, struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv, short weapon_position, bool calc_for_damage_only)
{
	map_session_data *sd = BL_CAST(BL_PC, src);
	status_change *sc = status_get_sc(src);
	struct status_data *sstatus = status_get_status_data(src);
	int element = skill_get_ele(skill_id, skill_lv);

	//Take weapon's element
	if( !skill_id || element == ELE_WEAPON ) {
		if (weapon_position == EQI_HAND_R)
			element = sstatus->rhw.ele;
		else
			element = sstatus->lhw.ele;
		if(is_skill_using_arrow(src, skill_id) && sd && sd->bonus.arrow_ele && weapon_position == EQI_HAND_R)
			element = sd->bonus.arrow_ele;
		if(sd && sd->spiritcharm_type != CHARM_TYPE_NONE && sd->spiritcharm >= MAX_SPIRITCHARM)
			element = sd->spiritcharm_type; // Summoning 10 spiritcharm will endow your weapon
		// on official endows override all other elements [helvetica]
		if(sc && sc->getSCE(SC_ENCHANTARMS)) // Check for endows
			element = sc->getSCE(SC_ENCHANTARMS)->val1;
	} else if( element == ELE_ENDOWED ) //Use enchantment's element
		element = status_get_attack_sc_element(src,sc);
	else if( element == ELE_RANDOM ) //Use random element
		element = rnd()%ELE_ALL;

	switch( skill_id ) {
		case GS_GROUNDDRIFT:
			element = wd->miscflag; //element comes in flag.
			break;
		case LK_SPIRALPIERCE:
			if (!sd)
				element = ELE_NEUTRAL; //forced neutral for monsters
			break;
		case RK_DRAGONBREATH:
			if (sc) {
				if (sc->getSCE(SC_LUXANIMA)) // Lux Anima has priority over Giant Growth
					element = ELE_DARK;
				else if (sc->getSCE(SC_GIANTGROWTH))
					element = ELE_HOLY;
			}
			break;
		case RK_DRAGONBREATH_WATER:
			if (sc) {
				if (sc->getSCE(SC_LUXANIMA)) // Lux Anima has priority over Fighting Spirit
					element = ELE_NEUTRAL;
				else if (sc->getSCE(SC_FIGHTINGSPIRIT))
					element = ELE_GHOST;
			}
			break;
		case LG_HESPERUSLIT:
			if (sc && sc->getSCE(SC_BANDING) && sc->getSCE(SC_BANDING)->val2 > 4)
				element = ELE_HOLY;
			break;
		case GN_CARTCANNON:
		case NC_ARMSCANNON:
			if (sd && sd->state.arrow_atk > 0)
				element = sd->bonus.arrow_ele;
			break;
		case SJ_PROMINENCEKICK:
 				element = ELE_FIRE;
 			break;
		case RL_H_MINE:
			if (sd && sd->flicker) //Force RL_H_MINE deals fire damage if activated by RL_FLICKER
				element = ELE_FIRE;
			break;
		case NW_BASIC_GRENADE:
		case NW_HASTY_FIRE_IN_THE_HOLE:
		case NW_GRENADES_DROPPING:
		case NW_MISSION_BOMBARD:
			// Night Watch Grenade Fragment elementals affecting those skills.
			if( sc != nullptr ){
				if( sc->getSCE( SC_GRENADE_FRAGMENT_1 ) != nullptr ){
					element = ELE_WATER;
				}else if( sc->getSCE( SC_GRENADE_FRAGMENT_2 ) != nullptr ){
					element = ELE_WIND;
				}else if( sc->getSCE( SC_GRENADE_FRAGMENT_3 ) != nullptr ){
					element = ELE_EARTH;
				}else if( sc->getSCE( SC_GRENADE_FRAGMENT_4 ) != nullptr ){
					element = ELE_FIRE;
				}else if( sc->getSCE( SC_GRENADE_FRAGMENT_5 ) != nullptr ){
					element = ELE_DARK;
				}else if( sc->getSCE( SC_GRENADE_FRAGMENT_6 ) != nullptr ){
					element = ELE_HOLY;
				}
			}
			break;
	}

	if (sc && sc->getSCE(SC_GOLDENE_FERSE) && ((!skill_id && (rnd() % 100 < sc->getSCE(SC_GOLDENE_FERSE)->val4)) || skill_id == MH_STAHL_HORN))
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

int battle_get_magic_element(struct block_list* src, struct block_list* target, uint16 skill_id, uint16 skill_lv, int mflag) {
	int element = skill_get_ele(skill_id, skill_lv);
	map_session_data *sd = BL_CAST(BL_PC, src);
	status_change *sc = status_get_sc(src);
	struct status_data *sstatus = status_get_status_data(src);
	
	if (element == ELE_WEAPON) { // pl=-1 : the skill takes the weapon's element
		element = sstatus->rhw.ele;
		if(sd && sd->spiritcharm_type != CHARM_TYPE_NONE && sd->spiritcharm >= MAX_SPIRITCHARM)
			element = sd->spiritcharm_type; // Summoning 10 spiritcharm will endow your weapon
	} else if (element == ELE_ENDOWED) //Use status element
		element = status_get_attack_sc_element(src,status_get_sc(src));
	else if (element == ELE_RANDOM) //Use random element
		element = rnd()%ELE_ALL;

	switch(skill_id) {
		case NPC_EARTHQUAKE:
			element = ELE_NEUTRAL;
			break;
		case WL_HELLINFERNO:
			if (mflag & 2) { // ELE_DARK
				element = ELE_DARK;
			}
			break;
		case NPC_PSYCHIC_WAVE:
		case SO_PSYCHIC_WAVE:
			if( sc && sc->count ) {
				static const std::vector<sc_type> types = {
					SC_HEATER_OPTION,
					SC_COOLER_OPTION,
					SC_BLAST_OPTION,
					SC_CURSED_SOIL_OPTION,
					SC_FLAMETECHNIC_OPTION,
					SC_COLD_FORCE_OPTION,
					SC_GRACE_BREEZE_OPTION,
					SC_EARTH_CARE_OPTION,
					SC_DEEP_POISONING_OPTION
				};
				for( sc_type type : types ){
					if( sc->getSCE( type ) ){
						element = sc->getSCE( type )->val3;
						break;
					}
				}
			}
			break;
		case KO_KAIHOU:
			if(sd && sd->spiritcharm_type != CHARM_TYPE_NONE && sd->spiritcharm > 0)
				element = sd->spiritcharm_type;
			break;
		case AB_ADORAMUS:
			if (sc && sc->getSCE(SC_ANCILLA))
				element = ELE_NEUTRAL;
			break;
		case LG_RAYOFGENESIS:
			if (sc && sc->getSCE(SC_INSPIRATION))
				element = ELE_NEUTRAL;
			break;
		case WM_REVERBERATION:
		case TR_METALIC_FURY:
		case TR_SOUNDBLEND:
			if (sd)
				element = sd->bonus.arrow_ele;
			break;
	}

	return element;
}

int battle_get_misc_element(struct block_list* src, struct block_list* target, uint16 skill_id, uint16 skill_lv, int mflag) {
	int element = skill_get_ele(skill_id, skill_lv);
	
	if (element == ELE_WEAPON || element == ELE_ENDOWED) //Attack that takes weapon's element for misc attacks? Make it neutral [Skotlex]
		element = ELE_NEUTRAL;
	else if (element == ELE_RANDOM) //Use random element
		element = rnd()%ELE_ALL;

	return element;
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
	#define RE_ALLATK_ADD(wd, a) do { int64 a_ = (a); ATK_ADD((wd)->statusAtk, (wd)->statusAtk2, a_); ATK_ADD((wd)->weaponAtk, (wd)->weaponAtk2, a_); ATK_ADD((wd)->equipAtk, (wd)->equipAtk2, a_); ATK_ADD((wd)->masteryAtk, (wd)->masteryAtk2, a_); } while(0);
	#define RE_ALLATK_RATE(wd, a) do { int64 a_ = (a); ATK_RATE((wd)->statusAtk, (wd)->statusAtk2, a_); ATK_RATE((wd)->weaponAtk, (wd)->weaponAtk2, a_); ATK_RATE((wd)->equipAtk, (wd)->equipAtk2, a_); ATK_RATE((wd)->masteryAtk, (wd)->masteryAtk2, a_); } while(0);
	#define RE_ALLATK_ADDRATE(wd, a) do { int64 a_ = (a); ATK_ADDRATE((wd)->statusAtk, (wd)->statusAtk2, a_); ATK_ADDRATE((wd)->weaponAtk, (wd)->weaponAtk2, a_); ATK_ADDRATE((wd)->equipAtk, (wd)->equipAtk2, a_); ATK_ADDRATE((wd)->masteryAtk, (wd)->masteryAtk2, a_); } while(0);
#else
	#define RE_ALLATK_ADD(wd, a) {;}
	#define RE_ALLATK_RATE(wd, a) {;}
	#define RE_ALLATK_ADDRATE(wd, a) {;}
#endif

/**
 * Cap both damage and basedamage of damage struct to a minimum value
 * @param wd: Weapon damage structure
 * @param src: Source of the attack
 * @param skill_id: Skill ID of the skill used by source
 * @param min: Minimum value to which damage should be capped
 */
static void battle_min_damage(struct Damage &wd, struct block_list &src, uint16 skill_id, int64 min) {
	if (is_attack_right_handed(&src, skill_id)) {
		wd.damage = cap_value(wd.damage, min, INT64_MAX);
#ifndef RENEWAL
		wd.basedamage = cap_value(wd.basedamage, min, INT64_MAX);
#endif
	}
	// Left-hand damage is always capped to 0
	if (is_attack_left_handed(&src, skill_id)) {
		wd.damage2 = cap_value(wd.damage2, 0, INT64_MAX);
	}
}

/**
 * Returns the bonus damage granted by Spirit Spheres
 * As we delete the spheres before calculating the damage, we need some kind of logic to figure out how many were available
 * Each skill handles this in its own way, this function handles the different cases
 * @param wd: Weapon damage structure
 * @param src: Source of the attack
 * @param skill_id: Skill ID of the skill used by source
 * @param min: Minimum value to which damage should be capped
 */
static int battle_get_spiritball_damage(struct Damage& wd, struct block_list& src, uint16 skill_id) {

	map_session_data* sd = BL_CAST(BL_PC, &src);

	// Return 0 for non-players
	if (!sd)
		return 0;

	int damage = 0;

	switch (skill_id) {
		case MO_INVESTIGATE:
#ifndef RENEWAL
		case MO_FINGEROFFENSIVE:
#endif
			// These skills used as many spheres as they do hits
			damage = (wd.div_ + sd->spiritball) * 3;
			break;
		case MO_EXTREMITYFIST:
			// These skills store the number of spheres the player had before cast
			damage = sd->spiritball_old * 3;
			break;
		default:
			// Any skills that do not consume spheres or do not care
			damage = sd->spiritball * 3;
			break;
	}

	return damage;
}

/*========================================
 * Do element damage modifier calculation
 *----------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static void battle_calc_element_damage(struct Damage* wd, struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv)
{
	std::bitset<NK_MAX> nk = battle_skill_get_damage_properties(skill_id, wd->miscflag);
	map_session_data* sd = BL_CAST(BL_PC, src);
	status_change* sc = status_get_sc(src);
	struct status_data* sstatus = status_get_status_data(src);
	struct status_data* tstatus = status_get_status_data(target);
	int right_element = battle_get_weapon_element(wd, src, target, skill_id, skill_lv, EQI_HAND_R, true);

	// Elemental attribute fix
	if(!nk[NK_IGNOREELEMENT] && (wd->damage > 0 || wd->damage2 > 0)) {
		int left_element = battle_get_weapon_element(wd, src, target, skill_id, skill_lv, EQI_HAND_L, true);

		switch (skill_id) {
			case PA_SACRIFICE:
			case RK_DRAGONBREATH:
			case RK_DRAGONBREATH_WATER:
			case NC_SELFDESTRUCTION:
			case HFLI_SBR44:
				wd->damage = battle_attr_fix(src, target, wd->damage, right_element, tstatus->def_ele, tstatus->ele_lv, 1);
				if (is_attack_left_handed(src, skill_id))
					wd->damage2 = battle_attr_fix(src, target, wd->damage2, left_element, tstatus->def_ele, tstatus->ele_lv, 1);
				break;
			default:
				if (skill_id == 0 && (battle_config.attack_attr_none & src->type))
					return; // Non-player basic attacks are non-elemental, they deal 100% against all defense elements
#ifdef RENEWAL
				if (sd == nullptr) { // Renewal player's elemental damage calculation is already done before this point, only calculate for everything else
#endif
					wd->damage = battle_attr_fix(src, target, wd->damage, right_element, tstatus->def_ele, tstatus->ele_lv, 1);
					if (is_attack_left_handed(src, skill_id))
						wd->damage2 = battle_attr_fix(src, target, wd->damage2, left_element, tstatus->def_ele, tstatus->ele_lv, 1);
#ifdef RENEWAL
				}
#endif
				break;
		}

		// Forced to neutral skills [helvetica]
		// Skills forced to neutral gain benefits from weapon element but final damage is considered "neutral" and resistances are applied again
		switch (skill_id) {
#ifdef RENEWAL
			case MO_INVESTIGATE:
			case CR_SHIELDBOOMERANG:
			case PA_SHIELDCHAIN:
#endif
			case MC_CARTREVOLUTION:
			case HW_MAGICCRASHER:
			case SR_FALLENEMPIRE:
			case SR_CRESCENTELBOW_AUTOSPELL:
			case SR_GATEOFHELL:
			case GN_FIRE_EXPANSION_ACID:
				wd->damage = battle_attr_fix(src, target, wd->damage, ELE_NEUTRAL, tstatus->def_ele, tstatus->ele_lv, 1);
				if (is_attack_left_handed(src, skill_id))
					wd->damage2 = battle_attr_fix(src, target, wd->damage2, ELE_NEUTRAL, tstatus->def_ele, tstatus->ele_lv, 1);
				break;
		}
	}

#ifndef RENEWAL
	// These mastery bonuses are non-elemental and should apply even if the attack misses
	// They are still increased by the EDP/Magnum Break bonus damage (WATK_ELEMENT)
	// In renewal these bonuses do not apply when the attack misses
	if (sd && battle_skill_stacks_masteries_vvs(skill_id, BCHK_STAR)) {
		// Star Crumb bonus damage
		ATK_ADD2(wd->damage, wd->damage2, sd->right_weapon.star, sd->left_weapon.star);
	}
	// Check if general mastery bonuses apply (above check is only for Star Crumb)
	if (battle_skill_stacks_masteries_vvs(skill_id, BCHK_ALL)) {
		// Spirit Sphere bonus damage
		ATK_ADD(wd->damage, wd->damage2, battle_get_spiritball_damage(*wd, *src, skill_id));

		// Skill-specific bonuses
		switch(skill_id) {
			case TF_POISON:
				ATK_ADD(wd->damage, wd->damage2, 15 * skill_lv);
				// Envenom applies the attribute table to the base damage and then again to the final damage
				wd->damage = battle_attr_fix(src, target, wd->damage, right_element, tstatus->def_ele, tstatus->ele_lv, 1);
				break;
			case NJ_SYURIKEN:
				ATK_ADD(wd->damage, wd->damage2, 4 * skill_lv);
				if (sd) {
					ATK_ADD(wd->damage, wd->damage2, 3 * pc_checkskill(sd, NJ_TOBIDOUGU));
					ATK_ADD(wd->damage, wd->damage2, sd->bonus.arrow_atk);
				}
				// Applies attribute table on neutral element to the final damage
				wd->damage = battle_attr_fix(src, target, wd->damage, ELE_NEUTRAL, tstatus->def_ele, tstatus->ele_lv, 1);
				break;
			case NJ_KUNAI:
				if (sd) {
					ATK_ADD(wd->damage, wd->damage2, 3 * sd->bonus.arrow_atk);
				}
				// Applies attribute table on neutral element to the final damage
				wd->damage = battle_attr_fix(src, target, wd->damage, ELE_NEUTRAL, tstatus->def_ele, tstatus->ele_lv, 1);
				break;
		}
	}

	// These bonuses do not apply to skills that ignore +% damage cards
	// If damage was reduced below 0 and was not increased again to a positive value through mastery bonuses, these bonuses are ignored
	// Any of these are only applied to your right hand weapon in pre-renewal
	if (!nk[NK_IGNOREELEMENT] && !nk[NK_IGNOREATKCARD] && (wd->damage > 0 || wd->damage2 > 0) && sc) {

		// EDP bonus damage
		// This has to be applied after mastery bonuses but still before the elemental extra damage
		if (sc->getSCE(SC_EDP))
			wd->damage += (wd->damage * sc->getSCE(SC_EDP)->val3) / 100;

		// This adds a percentual damage bonus based on the damage you would deal with a normal attack
		// Applies only to player damage; monsters and mercenaries don't get this damage boost
		if (sd && sc->getSCE(SC_WATK_ELEMENT)) {
			// Pretend the normal attack was of the element stored in the status change
			wd->basedamage = battle_attr_fix(src, target, wd->basedamage, sc->getSCE(SC_WATK_ELEMENT)->val1, tstatus->def_ele, tstatus->ele_lv, 1);
			// Star Crumb bonus damage
			wd->basedamage += sd->right_weapon.star;
			// Spirit Sphere bonus damage
			wd->basedamage += battle_get_spiritball_damage(*wd, *src, skill_id);
			// Add percent of the base damage to the damage
			wd->damage += (wd->basedamage * sc->getSCE(SC_WATK_ELEMENT)->val2) / 100;
		}
	}
#endif
	// Cap damage to 0
	if (battle_config.attr_recover == 0)
		battle_min_damage(*wd, *src, skill_id, 0);
}

/*==================================
 * Calculate weapon mastery damages
 *----------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static void battle_calc_attack_masteries(struct Damage* wd, struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv)
{
	map_session_data *sd = BL_CAST(BL_PC, src);
	status_change *sc = status_get_sc(src);
	struct status_data *sstatus = status_get_status_data(src);
	int t_class = status_get_class(target);

#ifndef RENEWAL
	if (sd) {
		wd->basedamage = battle_addmastery(sd, target, wd->basedamage, 0);
	}
#endif

	// Check if mastery damage applies to current skill
	if (sd && battle_skill_stacks_masteries_vvs(skill_id, BCHK_ALL))
	{	//Add mastery damage
		uint16 skill;

		wd->damage = battle_addmastery(sd,target,wd->damage,0);
#ifdef RENEWAL
		wd->masteryAtk = battle_addmastery(sd,target,wd->weaponAtk,0);
#endif
		if (is_attack_left_handed(src, skill_id)) {
			wd->damage2 = battle_addmastery(sd,target,wd->damage2,1);
#ifdef RENEWAL
			wd->masteryAtk2 = battle_addmastery(sd,target,wd->weaponAtk2,1);
#endif
		}

#ifdef RENEWAL
		//General skill masteries
		if(skill_id == TF_POISON) //Additional ATK from Envenom is treated as mastery type damage [helvetica]
			ATK_ADD(wd->masteryAtk, wd->masteryAtk2, 15 * skill_lv);
		if (skill_id != MC_CARTREVOLUTION && pc_checkskill(sd, BS_HILTBINDING) > 0)
			ATK_ADD(wd->masteryAtk, wd->masteryAtk2, 4);
		// Star Crumb bonus damage
		ATK_ADD2(wd->masteryAtk, wd->masteryAtk2, sd->right_weapon.star, sd->left_weapon.star);
		// Spirit Sphere bonus damage
		ATK_ADD(wd->masteryAtk, wd->masteryAtk2, battle_get_spiritball_damage(*wd, *src, skill_id));

		if (skill_id == NJ_SYURIKEN && (skill = pc_checkskill(sd,NJ_TOBIDOUGU)) > 0) {
			ATK_ADD(wd->damage, wd->damage2, 3 * skill);
			ATK_ADD(wd->masteryAtk, wd->masteryAtk2, 3 * skill);
		}
#endif

		switch(skill_id) {
			case RA_WUGDASH:
			case RA_WUGSTRIKE:
			case RA_WUGBITE:
				if (sd) {
					skill = pc_checkskill(sd, RA_TOOTHOFWUG);

					ATK_ADD(wd->damage, wd->damage2, 30 * skill);
#ifdef RENEWAL
					ATK_ADD(wd->masteryAtk, wd->masteryAtk2, 30 * skill);
#endif
				}
				break;
#ifdef RENEWAL
			case GN_CARTCANNON:
			case NC_ARMSCANNON:
				// Arrow attack of these skills is not influenced by P.ATK so we add it as mastery attack
				if (sd != nullptr) {
					struct status_data* tstatus = status_get_status_data(target);
					ATK_ADD(wd->masteryAtk, wd->masteryAtk2, battle_attr_fix(src, target, sd->bonus.arrow_atk, sd->bonus.arrow_ele, tstatus->def_ele, tstatus->ele_lv));
				}
				break;
#endif
		}

		if (sc) { // Status change considered as masteries
#ifdef RENEWAL
			if (sc->getSCE(SC_NIBELUNGEN)) // With renewal, the level 4 weapon limitation has been removed
				ATK_ADD(wd->masteryAtk, wd->masteryAtk2, sc->getSCE(SC_NIBELUNGEN)->val2);
#endif

			if(sc->getSCE(SC_CAMOUFLAGE)) {
				ATK_ADD(wd->damage, wd->damage2, 30 * min(10, sc->getSCE(SC_CAMOUFLAGE)->val3));
#ifdef RENEWAL
				ATK_ADD(wd->masteryAtk, wd->masteryAtk2, 30 * min(10, sc->getSCE(SC_CAMOUFLAGE)->val3));
#endif
			}
			if(sc->getSCE(SC_GN_CARTBOOST)) {
				ATK_ADD(wd->damage, wd->damage2, 10 * sc->getSCE(SC_GN_CARTBOOST)->val1);
#ifdef RENEWAL
				ATK_ADD(wd->masteryAtk, wd->masteryAtk2, 10 * sc->getSCE(SC_GN_CARTBOOST)->val1);
#endif
			}
			if (sc->getSCE(SC_P_ALTER)) {
				ATK_ADD(wd->damage, wd->damage2, sc->getSCE(SC_P_ALTER)->val2);
#ifdef RENEWAL
				ATK_ADD(wd->masteryAtk, wd->masteryAtk2, sc->getSCE(SC_P_ALTER)->val2);
#endif
			}
		}
	}
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
static void battle_calc_damage_parts(struct Damage* wd, struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv)
{
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);
	map_session_data *sd = BL_CAST(BL_PC, src);
	bool critical = false;

	int right_element = battle_get_weapon_element(wd, src, target, skill_id, skill_lv, EQI_HAND_R, false);
	int left_element = battle_get_weapon_element(wd, src, target, skill_id, skill_lv, EQI_HAND_L, false);

	wd->statusAtk += sstatus->batk;
	wd->statusAtk2 += sstatus->batk;

	if (sd && sd->sc.getSCE(SC_SEVENWIND)) { // Mild Wind applies element to status ATK as well as weapon ATK [helvetica]
		wd->statusAtk = battle_attr_fix(src, target, wd->statusAtk, right_element, tstatus->def_ele, tstatus->ele_lv);
		wd->statusAtk2 = battle_attr_fix(src, target, wd->statusAtk2, left_element, tstatus->def_ele, tstatus->ele_lv);
	} else { // status atk is considered neutral on normal attacks [helvetica]
		wd->statusAtk = battle_attr_fix(src, target, wd->statusAtk, ELE_NEUTRAL, tstatus->def_ele, tstatus->ele_lv);
		wd->statusAtk2 = battle_attr_fix(src, target, wd->statusAtk2, ELE_NEUTRAL, tstatus->def_ele, tstatus->ele_lv);
	}

	// Right-hand status attack is doubled after elemental adjustments
	wd->statusAtk *= 2;

	// Check critical
	if (wd->type == DMG_MULTI_HIT_CRITICAL || wd->type == DMG_CRITICAL)
		critical = true;

	wd->weaponAtk += battle_calc_base_weapon_attack(src, tstatus, &sstatus->rhw, sd, critical);
	wd->weaponAtk2 += battle_calc_base_weapon_attack(src, tstatus, &sstatus->lhw, sd, critical);

	// Weapon ATK gain bonus from SC_SUB_WEAPONPROPERTY here ( +x% pseudo element damage)
	if (sd && sd->sc.getSCE(SC_SUB_WEAPONPROPERTY)) {
		int64 bonus_atk = (int64)floor((float)( wd->weaponAtk *  sd->sc.getSCE(SC_SUB_WEAPONPROPERTY)->val2 / 100));
		int64 bonus_atk2 = (int64)floor((float)( wd->weaponAtk2 *  sd->sc.getSCE(SC_SUB_WEAPONPROPERTY)->val2 / 100));
		bonus_atk = battle_attr_fix(src, target, bonus_atk, sd->sc.getSCE(SC_SUB_WEAPONPROPERTY)->val1, tstatus->def_ele, tstatus->ele_lv);
		bonus_atk2 = battle_attr_fix(src, target, bonus_atk2, sd->sc.getSCE(SC_SUB_WEAPONPROPERTY)->val1, tstatus->def_ele, tstatus->ele_lv);

		wd->weaponAtk += bonus_atk;
		wd->weaponAtk2 += bonus_atk2;
	}

	wd->weaponAtk = battle_attr_fix(src, target, wd->weaponAtk, right_element, tstatus->def_ele, tstatus->ele_lv);
	wd->weaponAtk2 = battle_attr_fix(src, target, wd->weaponAtk2, left_element, tstatus->def_ele, tstatus->ele_lv);

	wd->equipAtk += battle_calc_equip_attack(src, skill_id);
	wd->equipAtk = battle_attr_fix(src, target, wd->equipAtk, right_element, tstatus->def_ele, tstatus->ele_lv);

	wd->equipAtk2 += battle_calc_equip_attack(src, skill_id);
	wd->equipAtk2 = battle_attr_fix(src, target, wd->equipAtk2, left_element, tstatus->def_ele, tstatus->ele_lv);

	// AtkRate gives a static bonus from (W.ATK + E.ATK)
	if (sd && sd->bonus.atk_rate) {
		wd->percentAtk = (wd->weaponAtk + wd->equipAtk) * sd->bonus.atk_rate / 100;
		wd->percentAtk2 = (wd->weaponAtk2 + wd->equipAtk2) * sd->bonus.atk_rate / 100;
	}

	//Mastery ATK is a special kind of ATK that has no elemental properties
	//Because masteries are not elemental, they are unaffected by Ghost armors or Raydric Card
	battle_calc_attack_masteries(wd, src, target, skill_id, skill_lv);

	wd->damage = 0;
	wd->damage2 = 0;
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
static void battle_calc_skill_base_damage(struct Damage* wd, struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv)
{
	status_change *sc = status_get_sc(src);
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);
	map_session_data *sd = BL_CAST(BL_PC, src);
	map_session_data *tsd = BL_CAST(BL_PC, target);

	uint16 bflag = BDMG_NONE;
#ifndef RENEWAL
	uint16 i;
#endif
	std::bitset<NK_MAX> nk = battle_skill_get_damage_properties(skill_id, wd->miscflag);

	switch (skill_id) {	//Calc base damage according to skill
		case PA_SACRIFICE:
			wd->damage = sstatus->max_hp* 9/100;
			wd->damage2 = 0;
#ifdef RENEWAL
			wd->weaponAtk = wd->damage;
			wd->weaponAtk2 = wd->damage2;
#endif
			break;
#ifdef RENEWAL
		case LK_SPIRALPIERCE:
		case ML_SPIRALPIERCE:
			if (sd) {
				battle_calc_damage_parts(wd, src, target, skill_id, skill_lv);

				// Officially statusAtk + weaponAtk + equipAtk make base attack
				// We simulate this here by adding them all into equip attack
				ATK_ADD2(wd->equipAtk, wd->equipAtk2, wd->statusAtk + wd->weaponAtk, wd->statusAtk2 + wd->weaponAtk2);
				// Set statusAtk and weaponAtk to 0
				ATK_RATE(wd->statusAtk, wd->statusAtk2, 0);
				ATK_RATE(wd->weaponAtk, wd->weaponAtk2, 0);

				// Add weight
				short index = sd->equip_index[EQI_HAND_R];
				if (index >= 0 &&
					sd->inventory_data[index] &&
					sd->inventory_data[index]->type == IT_WEAPON)
					wd->equipAtk += sd->inventory_data[index]->weight / 10;

				// 70% damage modifier is applied to base attack + weight
				ATK_RATE(wd->equipAtk, wd->equipAtk2, 70);

				// Additional skill-specific size fix
				switch (tstatus->size) {
					case SZ_SMALL: //Small: 130%
						ATK_RATE(wd->equipAtk, wd->equipAtk2, 130);
						break;
					case SZ_MEDIUM: //Medium: 115%
						ATK_RATE(wd->equipAtk, wd->equipAtk2, 115);
						break;
					//case SZ_BIG: //Large: 100%
				}
			} else {
				wd->damage = battle_calc_base_damage(src, sstatus, &sstatus->rhw, sc, tstatus->size, 0); //Monsters have no weight and use ATK instead
			}
#else
		case NJ_ISSEN:
			wd->damage = 40 * sstatus->str + sstatus->hp * 8 * skill_lv / 100;
			wd->damage2 = 0;
			break;
		case LK_SPIRALPIERCE:
		case ML_SPIRALPIERCE:
			if (sd) {
				short index = sd->equip_index[EQI_HAND_R];

				if (index >= 0 &&
					sd->inventory_data[index] &&
					sd->inventory_data[index]->type == IT_WEAPON)
					wd->damage = sd->inventory_data[index]->weight*8/100; //80% of weight

				ATK_ADDRATE(wd->damage, wd->damage2, 50*skill_lv); //Skill modifier applies to weight only.
			} else {
				wd->damage = battle_calc_base_damage(src, sstatus, &sstatus->rhw, sc, tstatus->size, 0); //Monsters have no weight and use ATK instead
			}

			i = sstatus->str/10;
			i*=i;
			ATK_ADD(wd->damage, wd->damage2, i); //Add str bonus.
			switch (tstatus->size) { //Size-fix. Is this modified by weapon perfection?
				case SZ_SMALL: //Small: 125%
					ATK_RATE(wd->damage, wd->damage2, 125);
					break;
				//case SZ_MEDIUM: //Medium: 100%
				case SZ_BIG: //Large: 75%
					ATK_RATE(wd->damage, wd->damage2, 75);
					break;
			}
#endif
			break;
		case CR_SHIELDBOOMERANG:
		case PA_SHIELDCHAIN:
			wd->damage = sstatus->batk;
			if (sd) {
				short index = sd->equip_index[EQI_HAND_L];

				//Base damage of shield skills is [batk + 4*refine + weight]
				if (index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_ARMOR) {
					ATK_ADD(wd->damage, wd->damage2, 4 * sd->inventory.u.items_inventory[index].refine);
					ATK_ADD(wd->damage, wd->damage2, sd->inventory_data[index]->weight / 10);
#ifdef RENEWAL
					ATK_ADD(wd->weaponAtk, wd->weaponAtk2, 4 * sd->inventory.u.items_inventory[index].refine);
					ATK_ADD(wd->weaponAtk, wd->weaponAtk2, sd->inventory_data[index]->weight / 10);
#endif
				}
#ifndef RENEWAL
				// Shield Boomerang and Rapid Smiting calculate DEF before the skill ratio
				battle_calc_defense_reduction(wd, src, target, skill_id, skill_lv);
#endif
			} else
				ATK_ADD(wd->damage, wd->damage2, sstatus->rhw.atk2); //Else use Atk2
			break;
		case RK_DRAGONBREATH:
		case RK_DRAGONBREATH_WATER:
			{
				int damagevalue = (sstatus->hp / 50 + status_get_max_sp(src) / 4) * skill_lv;
				if(status_get_lv(src) > 100)
					damagevalue = damagevalue * status_get_lv(src) / 100;
				if(sd) {
					if (pc_checkskill( sd, DK_DRAGONIC_AURA ) >= 1) {
						damagevalue = damagevalue * (90 + 10 * pc_checkskill( sd, RK_DRAGONTRAINING ) + sstatus->pow / 5 ) / 100;
					} else {
						damagevalue = damagevalue * (90 + 10 * pc_checkskill( sd, RK_DRAGONTRAINING )) / 100;
					}
				}
				if (sc && sc->getSCE(SC_DRAGONIC_AURA))
					damagevalue += damagevalue * sc->getSCE(SC_DRAGONIC_AURA)->val1 * 10 / 100;
				ATK_ADD(wd->damage, wd->damage2, damagevalue);
#ifdef RENEWAL
				ATK_ADD(wd->weaponAtk, wd->weaponAtk2, damagevalue);
#endif
				wd->flag |= BF_LONG;
			}
			break;
		case NC_SELFDESTRUCTION: {
				int damagevalue = (skill_lv + 1) * ((sd ? pc_checkskill(sd,NC_MAINFRAME) : 0) + 8) * (status_get_sp(src) + sstatus->vit);

				if(status_get_lv(src) > 100)
					damagevalue = damagevalue * status_get_lv(src) / 100;
				damagevalue = damagevalue + sstatus->hp;
				ATK_ADD(wd->damage, wd->damage2, damagevalue);
#ifdef RENEWAL
				ATK_ADD(wd->weaponAtk, wd->weaponAtk2, damagevalue);
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
				if (sc && sc->getSCE(SC_KAGEMUSYA))
					damagevalue += damagevalue * sc->getSCE(SC_KAGEMUSYA)->val2 / 100;
				ATK_ADD(wd->damage, wd->damage2, damagevalue);
#ifdef RENEWAL
				ATK_ADD(wd->weaponAtk, wd->weaponAtk2, damagevalue);
#endif
			} else
				ATK_ADD(wd->damage, wd->damage2, 5000);
			break;
		case HFLI_SBR44:	//[orn]
			if(src->type == BL_HOM)
				wd->damage = ((TBL_HOM*)src)->homunculus.intimacy ;
			break;

		default:
			// Flags that apply to both pre-renewal and renewal
			bflag = BDMG_NONE;
			if (is_attack_critical(wd, src, target, skill_id, skill_lv, false)) bflag |= BDMG_CRIT;
			if (!skill_id && sc && sc->getSCE(SC_CHANGE)) bflag |= BDMG_MAGIC;
#ifdef RENEWAL
			if (sd)
				battle_calc_damage_parts(wd, src, target, skill_id, skill_lv);
			else {
				wd->damage = battle_calc_base_damage(src, sstatus, &sstatus->rhw, sc, tstatus->size, bflag);
				if (is_attack_left_handed(src, skill_id))
					wd->damage2 = battle_calc_base_damage(src, sstatus, &sstatus->lhw, sc, tstatus->size, bflag);
			}
#else
			// Pre-renewal exclusive flags
			if (skill_id == HW_MAGICCRASHER) bflag |= BDMG_MAGIC;
			if (sc && sc->getSCE(SC_WEAPONPERFECTION)) bflag |= BDMG_NOSIZE;
			if (is_skill_using_arrow(src, skill_id) && sd) {
				switch(sd->status.weapon) {
					case W_BOW:
						bflag |= BDMG_ARROW;
						break;
					case W_REVOLVER:
					case W_RIFLE:
					case W_GATLING:
					case W_SHOTGUN:
					case W_GRENADE:
						// Criticals with any guns are calculated like melee criticals
						if (!(bflag & BDMG_CRIT)) {
							bflag |= BDMG_ARROW;
						}
						break;
					default:
						// Attacks that use ammo are calculated like melee attacks as long as no ranged weapon is equipped
						// Some skills manually add arrow_atk as mastery bonus later (e.g. Throw Shuriken, Throw Kunai)
						break;
				}
			}
			if (skill_id == SN_SHARPSHOOTING || skill_id == MA_SHARPSHOOTING)
				bflag &= ~(BDMG_CRIT); // Sharpshooting just ignores DEF/FLEE but damage is like a normal attack
			wd->damage = battle_calc_base_damage(src, sstatus, &sstatus->rhw, sc, tstatus->size, bflag);
			if (is_attack_left_handed(src, skill_id))
				wd->damage2 = battle_calc_base_damage(src, sstatus, &sstatus->lhw, sc, tstatus->size, bflag);
#endif
			if (nk[NK_SPLASHSPLIT]){ // Divide ATK among targets
				if(wd->miscflag > 0) {
					wd->damage /= wd->miscflag;
#ifdef RENEWAL
					wd->statusAtk /= wd->miscflag;
					wd->weaponAtk /= wd->miscflag;
					wd->equipAtk /= wd->miscflag;
					wd->masteryAtk /= wd->miscflag;
#endif
				} else
					ShowError("0 enemies targeted by %d:%s, divide per 0 avoided!\n", skill_id, skill_get_name(skill_id));
			}

			//Add any bonuses that modify the base atk (pre-skills)
			if(sd) {
				int skill;

#ifndef RENEWAL
				if (sd->bonus.crit_atk_rate && (bflag & BDMG_CRIT)) { // add +crit damage bonuses here in pre-renewal mode [helvetica]
					ATK_ADDRATE(wd->damage, wd->damage2, sd->bonus.crit_atk_rate);
				}
				if(sd->status.party_id && (skill=pc_checkskill(sd,TK_POWER)) > 0) {
					if( (i = party_foreachsamemap(party_sub_count, sd, 0)) > 1 ) { // exclude the player himself [Inkfish]
						// Reduce count by one (self) [Tydus1]
						i -= 1; 
						ATK_ADDRATE(wd->damage, wd->damage2, 2*skill*i);
					}
				}
#else
				if ((skill = pc_checkskill(sd, TK_POWER)) > 0) {
					ATK_ADDRATE(wd->damage, wd->damage2, 10 + 15 * skill);
					RE_ALLATK_ADDRATE(wd, 10 + 15 * skill);
				}
#endif
			}
#ifndef RENEWAL
			if (tsd != nullptr && tsd->bonus.crit_def_rate != 0 && !skill_id && (bflag & BDMG_CRIT)) {
				ATK_ADDRATE(wd->damage, wd->damage2, -tsd->bonus.crit_def_rate);
			}
			//Acid Terror ignores DEF but will substract VIT from base attack value instead
			if (skill_id == AM_ACIDTERROR)
				ATK_ADD(wd->damage, wd->damage2, -tstatus->def2);
#endif
			break;
	} //End switch(skill_id)
}

//For quick div adjustment.
#define DAMAGE_DIV_FIX(dmg, div) { if ((div) < 0) { (div) *= -1; (dmg) /= (div); } (dmg) *= (div); }
#define DAMAGE_DIV_FIX2(dmg, div) { if ((div) > 1) (dmg) *= div; }

/*================================================= [Playtester]
 * Applies DAMAGE_DIV_FIX and checks for min damage
 * @param d: Damage struct to apply DAMAGE_DIV_FIX to
 * @param skill_id: ID of the skill that deals damage
 * @return Modified damage struct
 *------------------------------------------------*/
static void battle_apply_div_fix(struct Damage* d, uint16 skill_id)
{
	if(d->damage) {
		DAMAGE_DIV_FIX(d->damage, d->div_);
		//Min damage
		if(d->damage < d->div_ && (skill_id == SU_LUNATICCARROTBEAT || skill_id == SU_LUNATICCARROTBEAT2 || skill_id == SU_CN_METEOR || skill_id == SU_CN_METEOR2 || (battle_config.skill_min_damage&d->flag)))
			d->damage = d->div_;
	} else if (d->div_ < 0) {
		d->div_ *= -1;
	}
}

/*=======================================
 * Check for and calculate multi attacks
 *---------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static void battle_calc_multi_attack(struct Damage* wd, struct block_list *src,struct block_list *target, uint16 skill_id, uint16 skill_lv)
{
	map_session_data *sd = BL_CAST(BL_PC, src);
	status_change *sc = status_get_sc(src);
	status_change *tsc = status_get_sc(target);
	struct status_data *tstatus = status_get_status_data(target);

	if( sd && !skill_id ) {	// if no skill_id passed, check for double attack [helvetica]
		short i;
		if(sc && sc->getSCE(SC_FEARBREEZE) && sd->weapontype1==W_BOW
			&& (i = sd->equip_index[EQI_AMMO]) >= 0 && sd->inventory_data[i] && sd->inventory.u.items_inventory[i].amount > 1)
		{
			int chance = rnd()%100;
			switch(sc->getSCE(SC_FEARBREEZE)->val1) {
				case 5:
					if (chance < 4) {
						wd->div_ = 5;
						break;
					} // 3 % chance to attack 5 times.
					[[fallthrough]];
				case 4:
					if (chance < 7) {
						wd->div_ = 4;
						break;
					} // 6 % chance to attack 4 times.
					[[fallthrough]];
				case 3:
					if (chance < 10) {
						wd->div_ = 3;
						break;
					} // 9 % chance to attack 3 times.
					[[fallthrough]];
				case 2:
				case 1:
					if (chance < 13) {
						wd->div_ = 2;
						break;
					} // 12 % chance to attack 2 times.
			}
			wd->div_ = min(wd->div_,sd->inventory.u.items_inventory[i].amount);
			sc->getSCE(SC_FEARBREEZE)->val4 = wd->div_-1;
			if (wd->div_ > 1)
				wd->type = DMG_MULTI_HIT;
		}
		if( wd->div_ == 1 && ( ( skill_lv = pc_checkskill(sd,TF_DOUBLE) ) > 0 && sd->weapontype1 == W_DAGGER )
			|| ( pc_checkskill_flag(*sd, TF_DOUBLE) > SKILL_FLAG_PERMANENT && sd->weapontype1 != W_FIST )
			|| ( sd->bonus.double_rate > 0 && sd->weapontype1 != W_FIST ) // Will fail bare-handed
			|| ( sc && sc->getSCE(SC_KAGEMUSYA) && sd->weapontype1 != W_FIST )) // Will fail bare-handed
		{	//Success chance is not added, the higher one is used [Skotlex]
			int max_rate = 0;

			if (sc && sc->getSCE(SC_KAGEMUSYA))
				max_rate = sc->getSCE(SC_KAGEMUSYA)->val1 * 10; // Same rate as even levels of TF_DOUBLE
			else
#ifdef RENEWAL
				max_rate = max(7 * skill_lv, sd->bonus.double_rate);
#else
				max_rate = max(5 * skill_lv, sd->bonus.double_rate);
#endif

			if( rnd()%100 < max_rate ) {
				wd->div_ = skill_get_num(TF_DOUBLE,skill_lv?skill_lv:1);
				wd->type = DMG_MULTI_HIT;
			}
		}
		if( wd->div_ == 1 && ((sd->weapontype1 == W_REVOLVER && (skill_lv = pc_checkskill(sd,GS_CHAINACTION)) > 0) //Normal Chain Action effect
			|| (sc && sc->count && sc->getSCE(SC_E_CHAIN) && (skill_lv = sc->getSCE(SC_E_CHAIN)->val1) > 0)) //Chain Action of ETERNAL_CHAIN
			&& rnd()%100 < 5*skill_lv ) //Success rate
		{
			wd->div_ = skill_get_num(GS_CHAINACTION,skill_lv);
			wd->type = DMG_MULTI_HIT;

			sc_start(src,src,SC_QD_SHOT_READY,100,target->id,skill_get_time(RL_QD_SHOT,1));
		}
	}

	switch (skill_id) {
		case RK_WINDCUTTER:
			if (sd && sd->weapontype1 == W_2HSWORD)
				wd->div_ = 2;
			break;
		case SC_FATALMENACE:
			if (sd && sd->weapontype1 == W_DAGGER)
				wd->div_++;
			break;
		case SR_RIDEINLIGHTNING:
			wd->div_ = (sd ? max(1, skill_lv) : 1);
			break;
		case RL_QD_SHOT:
			wd->div_ = 1 + (sd ? sd->status.job_level : 1) / 20 + (tsc && tsc->getSCE(SC_C_MARKER) ? 2 : 0);
			break;
		case KO_JYUMONJIKIRI:
			if( tsc && tsc->getSCE(SC_JYUMONJIKIRI) )
				wd->div_ = wd->div_ * -1;// needs more info
			break;
		case MH_BLAZING_AND_FURIOUS: {
			struct homun_data *hd = BL_CAST(BL_HOM, src);
			if (hd) {
				wd->div_ = hd->homunculus.spiritball;
				hom_delspiritball(hd, MAX_SPIRITBALL, 1);
			}
			break;
		}
		case ABC_FRENZY_SHOT:
			if( rnd_chance( 5 * skill_lv, 100 ) ){
				wd->div_ = 3;
			}
			break;
#ifdef RENEWAL
		case AS_POISONREACT:
			skill_lv = pc_checkskill(sd, TF_DOUBLE);
			if (skill_lv > 0) {
				if(rnd()%100 < (7 * skill_lv)) {
					wd->div_++;
				}
			}
		break;
#endif
		case NW_SPIRAL_SHOOTING:
			if (sd && sd->weapontype1 == W_GRENADE)
				wd->div_ += 1;
			break;
		case NW_MAGAZINE_FOR_ONE:
			if (sd && sd->weapontype1 == W_GATLING)
				wd->div_ += 4;
			break;
		case NW_THE_VIGILANTE_AT_NIGHT:
			if (sd && sd->weapontype1 == W_GATLING)
				wd->div_ += 3;
			break;
	}
}

/**
 * Calculates the percentual attack modificator (ATKpercent) based on status changes
 * These bonuses are added together and the percent is applied to the damage before the defense reduction in pre-renewal
 * In renewal this simply sets the base skillratio before the actual skill ratio of the skill used is added
 * This bonus works as a separate unit to the rest (e.g., if one of these is not applied to a skill, then we know none are)
 * Do not add additional status changes here unless they are confirmed to use ATKpercent
 * @param bl: Object to calc atkpercent for
 * @param skill_id: Skill used by object
 * @param sc: Object's status change information
 * @return atkpercent with cap_value(watk,0,USHRT_MAX)
 */
static unsigned short battle_get_atkpercent(struct block_list& bl, uint16 skill_id, status_change& sc)
{
	if (bl.type == BL_PC && !battle_skill_stacks_masteries_vvs(skill_id, BCHK_ALL))
		return 100;

	int atkpercent = 100;

	if (sc.getSCE(SC_CURSE))
		atkpercent -= 25;
	if (sc.getSCE(SC_PROVOKE))
		atkpercent += sc.getSCE(SC_PROVOKE)->val2;
	if (sc.getSCE(SC_STRIPWEAPON) && bl.type != BL_PC)
		atkpercent -= sc.getSCE(SC_STRIPWEAPON)->val2;
	if (sc.getSCE(SC_CONCENTRATION))
		atkpercent += sc.getSCE(SC_CONCENTRATION)->val2;
	if (sc.getSCE(SC_TRUESIGHT))
		atkpercent += 2 * sc.getSCE(SC_TRUESIGHT)->val1;
	if (sc.getSCE(SC_JOINTBEAT) && sc.getSCE(SC_JOINTBEAT)->val2 & BREAK_WAIST)
		atkpercent -= 25;
	if (sc.getSCE(SC_INCATKRATE))
		atkpercent += sc.getSCE(SC_INCATKRATE)->val1;
	if (sc.getSCE(SC_POWERUP))
		atkpercent += sc.getSCE(SC_POWERUP)->val1;
	if (sc.getSCE(SC_SKE))
		atkpercent += 300;
	if (sc.getSCE(SC_BLOODLUST))
		atkpercent += sc.getSCE(SC_BLOODLUST)->val2;
	if (sc.getSCE(SC_FLEET))
		atkpercent += sc.getSCE(SC_FLEET)->val3;
	if (sc.getSCE(SC_INVINCIBLE))
		atkpercent += sc.getSCE(SC_INVINCIBLE)->val2;

	/* Only few selected skills should use this function, DO NOT ADD any that are not caused by the skills listed below
	* TODO:
	* GD_GUARDUP (2*skLevel+8)
	* EL_WATERBARRIER (-3)
	* SC_ENERVATION (-30/-40/-50)
	* MH_EQC (skLevel*5)
	* MH_VOLCANIC_ASH (-50)
	*/

	return (unsigned short)cap_value(atkpercent, 0, USHRT_MAX);
}

/*======================================================
 * Calculate skill level ratios for weapon-based skills
 *------------------------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static int battle_calc_attack_skill_ratio(struct Damage* wd, struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv)
{
	map_session_data *sd = BL_CAST(BL_PC, src);
	map_session_data *tsd = BL_CAST(BL_PC, target);
	status_change *sc = status_get_sc(src);
	status_change *tsc = status_get_sc(target);
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);
	int skillratio = 100;
	int i;

	//Skill damage modifiers that stack linearly
	if(sc && skill_id != PA_SACRIFICE) {
#ifdef RENEWAL
		//ATK percent modifier (in renewal, it's applied before the skillratio)
		skillratio = battle_get_atkpercent(*src, skill_id, *sc);
#endif
		if(sc->getSCE(SC_OVERTHRUST))
			skillratio += sc->getSCE(SC_OVERTHRUST)->val3;
		if(sc->getSCE(SC_MAXOVERTHRUST))
			skillratio += sc->getSCE(SC_MAXOVERTHRUST)->val2;
		if(sc->getSCE(SC_BERSERK))
#ifndef RENEWAL
			skillratio += 100;
#else
			skillratio += 200;
#endif
		if (!skill_id || skill_id == KN_AUTOCOUNTER) {
			if (sc->getSCE(SC_CRUSHSTRIKE)) {
				if (sd) { //ATK [{Weapon Level * (Weapon Upgrade Level + 6) * 100} + (Weapon ATK) + (Weapon Weight)]%
					short index = sd->equip_index[EQI_HAND_R];

					if (index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_WEAPON)
						skillratio += -100 + sd->inventory_data[index]->weight / 10 + sd->inventory_data[index]->atk +
							100 * sd->inventory_data[index]->weapon_level * (sd->inventory.u.items_inventory[index].refine + 6);
				}
				status_change_end(src,SC_CRUSHSTRIKE);
				skill_break_equip(src,src,EQP_WEAPON,2000,BCT_SELF);
			} else {
				if (sc->getSCE(SC_GIANTGROWTH) && (sd->class_&MAPID_THIRDMASK) == MAPID_RUNE_KNIGHT) { // Increase damage again if Crush Strike is not active
					if (map_flag_vs(src->m)) // Only half of the 2.5x increase on versus-type maps
						skillratio += 125;
					else
						skillratio += 250;
				}
			}
		}
	}

	switch(skill_id) {
		case SM_BASH:
		case MS_BASH:
			skillratio += 30 * skill_lv;
			break;
		case SM_MAGNUM:
		case MS_MAGNUM:
			if(wd->miscflag == 1)
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
		case KN_PIERCE:
			skillratio += 10 * skill_lv;
			if (sc && sc->getSCE(SC_CHARGINGPIERCE_COUNT) && sc->getSCE(SC_CHARGINGPIERCE_COUNT)->val1 >= 10)
				skillratio *= 2;
			break;
		case ML_PIERCE:
			skillratio += 10 * skill_lv;
			break;
		case MER_CRASH:
			skillratio += 10 * skill_lv;
			break;
		case KN_SPEARSTAB:
			skillratio += 20 * skill_lv;
			break;
		case KN_SPEARBOOMERANG:
			skillratio += 50 * skill_lv;
			break;
#ifdef RENEWAL
		case KN_BRANDISHSPEAR:
			skillratio += -100 + 400 + 100 * skill_lv + sstatus->str * 3;
			break;
#else
		case KN_BRANDISHSPEAR:
#endif
		case ML_BRANDISH:
			{
				int ratio = 100 + 20 * skill_lv;

				skillratio += -100 + ratio;
				if(skill_lv > 3 && wd->miscflag == 0)
					skillratio += ratio / 2;
				if(skill_lv > 6 && wd->miscflag == 0)
					skillratio += ratio / 4;
				if(skill_lv > 9 && wd->miscflag == 0)
					skillratio += ratio / 8;
				if(skill_lv > 6 && wd->miscflag == 1)
					skillratio += ratio / 2;
				if(skill_lv > 9 && wd->miscflag == 1)
					skillratio += ratio / 4;
				if(skill_lv > 9 && wd->miscflag == 2)
					skillratio += ratio / 2;
			}
			break;
		case KN_BOWLINGBASH:
		case MS_BOWLINGBASH:
			skillratio += 40 * skill_lv;
			break;
		case AS_GRIMTOOTH:
			skillratio += 20 * skill_lv;
			break;
		case AS_POISONREACT:
			skillratio += 30 * skill_lv;
			break;
		case AS_SONICBLOW:
#ifdef RENEWAL
			skillratio += 100 + 100 * skill_lv;
			if (tstatus->hp < (tstatus->max_hp / 2))
				skillratio += skillratio / 2;
#else
			skillratio += 200 + 50 * skill_lv;
			if (sd && pc_checkskill(sd, AS_SONICACCEL) > 0)
				skillratio += skillratio / 10;
#endif
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
		case NPC_ICEBREATH2:
		case NPC_THUNDERBREATH:
		case NPC_HELLJUDGEMENT:
		case NPC_HELLJUDGEMENT2:
		case NPC_PULSESTRIKE:
			skillratio += 100 * (skill_lv - 1);
			break;
		case NPC_REVERBERATION_ATK:
			skillratio += 400 + 200 * skill_lv;
			break;
		case RG_BACKSTAP:
			if(sd && sd->status.weapon == W_BOW && battle_config.backstab_bow_penalty)
				skillratio += (200 + 40 * skill_lv) / 2;
			else
				skillratio += 200 + 40 * skill_lv;
			break;
		case RG_RAID:
#ifdef RENEWAL
			skillratio += -100 + 50 + skill_lv * 150;
#else
			skillratio += 40 * skill_lv;
#endif
			break;
		case RG_INTIMIDATE:
			skillratio += 30 * skill_lv;
			break;
		case CR_SHIELDCHARGE:
			skillratio += 20 * skill_lv;
			break;
		case CR_SHIELDBOOMERANG:
#ifdef RENEWAL
			skillratio += -100 + skill_lv * 80;
#else
			skillratio += 30 * skill_lv;
#endif
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
#ifdef RENEWAL
			skillratio += -100 + 200 * skill_lv;
			if (sd && pc_checkskill(sd, AM_LEARNINGPOTION))
				skillratio += 100; // !TODO: What's this bonus increase?
#else
			skillratio += -50 + 50 * skill_lv;
#endif
			break;
		case MO_FINGEROFFENSIVE:
#ifdef RENEWAL
			skillratio += 500 + skill_lv * 200;
			if (tsc && tsc->getSCE(SC_BLADESTOP))
				skillratio += skillratio / 2;
#else
			skillratio += 50 * skill_lv;
#endif
			break;
		case MO_INVESTIGATE:
#ifdef RENEWAL
			skillratio += -100 + 100 * skill_lv;
			if (tsc && tsc->getSCE(SC_BLADESTOP))
				skillratio += skillratio / 2;
#else
			skillratio += 75 * skill_lv;
#endif
			break;
		case MO_EXTREMITYFIST:
			skillratio += 700 + sstatus->sp * 10;
#ifdef RENEWAL
			if (wd->miscflag&1)
				skillratio *= 2; // More than 5 spirit balls active
#endif
			skillratio = min(500000,skillratio); //We stop at roughly 50k SP for overflow protection
			break;
		case MO_TRIPLEATTACK:
			skillratio += 20 * skill_lv;
			break;
		case MO_CHAINCOMBO:
#ifdef RENEWAL
			skillratio += 150 + 50 * skill_lv;
			if (sd && sd->status.weapon == W_KNUCKLE)
				skillratio *= 2;
#else
			skillratio += 50 + 50 * skill_lv;
#endif
			break;
		case MO_COMBOFINISH:
#ifdef RENEWAL
			skillratio += 450 + 50 * skill_lv + sstatus->str; // !TODO: How does STR play a role?
#else
			skillratio += 140 + 60 * skill_lv;
#endif
			if (sc->getSCE(SC_GT_ENERGYGAIN))
				skillratio += skillratio * 50 / 100;
			break;
		case BA_MUSICALSTRIKE:
		case DC_THROWARROW:
#ifdef RENEWAL
			skillratio += 10 + 40 * skill_lv;
#else
			skillratio += 25 + 25 * skill_lv;
#endif
			break;
		case CH_TIGERFIST:
#ifdef RENEWAL
			skillratio += 400 + 150 * skill_lv;
			RE_LVL_DMOD(100);
#else
			skillratio += -60 + 100 * skill_lv;
#endif
			if (sc->getSCE(SC_GT_ENERGYGAIN))
				skillratio += skillratio * 50 / 100;
			break;
		case CH_CHAINCRUSH:
#ifdef RENEWAL
			skillratio += -100 + 200 * skill_lv;
			RE_LVL_DMOD(100);
#else
			skillratio += 300 + 100 * skill_lv;
#endif
			if (sc->getSCE(SC_GT_ENERGYGAIN))
				skillratio += skillratio * 50 / 100;
			break;
		case CH_PALMSTRIKE:
#ifdef RENEWAL
			skillratio += 100 + 100 * skill_lv + sstatus->str; // !TODO: How does STR play a role?
			RE_LVL_DMOD(100);
#else
			skillratio += 100 + 100 * skill_lv;
#endif
			break;
		case LK_HEADCRUSH:
			skillratio += 40 * skill_lv;
			break;
		case LK_JOINTBEAT:
			skillratio += 10 * skill_lv - 50;
			if (wd->miscflag & BREAK_NECK || (tsc && tsc->getSCE(SC_JOINTBEAT) && tsc->getSCE(SC_JOINTBEAT)->val2 & BREAK_NECK)) // The 2x damage is only for the BREAK_NECK ailment.
				skillratio *= 2;
			break;
#ifdef RENEWAL
		// Renewal: skill ratio applies to entire damage [helvetica]
		case LK_SPIRALPIERCE:
			skillratio += 50 + 50 * skill_lv;
			RE_LVL_DMOD(100);
			if (sc && sc->getSCE(SC_CHARGINGPIERCE_COUNT) && sc->getSCE(SC_CHARGINGPIERCE_COUNT)->val1 >= 10)
				skillratio *= 2;
			break;
		case ML_SPIRALPIERCE:
			skillratio += 50 + 50 * skill_lv;
			RE_LVL_DMOD(100);
		break;
#endif
		case ASC_METEORASSAULT:
#ifdef RENEWAL
			skillratio += 100 + 120 * skill_lv;
			RE_LVL_DMOD(100);
#else
			skillratio += -60 + 40 * skill_lv;
#endif
			break;
		case SN_SHARPSHOOTING:
			if (src->type == BL_MOB) { // TODO: Did these formulas change in the renewal balancing?
				if (wd->miscflag & 2) // Splash damage bonus
					skillratio += -100 + 140 * skill_lv;
				else
					skillratio += 100 + 50 * skill_lv;
				break;
			}
			[[fallthrough]];
		case MA_SHARPSHOOTING:
#ifdef RENEWAL
			skillratio += -100 + 300 + 300 * skill_lv;
			RE_LVL_DMOD(100);
#else
			skillratio += 100 + 50 * skill_lv;
#endif
			break;
#ifdef RENEWAL
		case CR_ACIDDEMONSTRATION:
			skillratio += -100 + 200 * skill_lv + sstatus->int_ + tstatus->vit; // !TODO: Confirm status bonus
			if (target->type == BL_PC)
				skillratio /= 2;
			break;
#endif
		case CG_ARROWVULCAN:
#ifdef RENEWAL
			skillratio += 400 + 100 * skill_lv;
			RE_LVL_DMOD(100);
#else
			skillratio += 100 + 100 * skill_lv;
#endif
			break;
		case AS_SPLASHER:
#ifdef RENEWAL
			skillratio += -100 + 400 + 100 * skill_lv;
#else
			skillratio += 400 + 50 * skill_lv;
#endif
			if(sd)
				skillratio += 20 * pc_checkskill(sd,AS_POISONREACT);
			break;
		case ASC_BREAKER:
#ifdef RENEWAL
			skillratio += -100 + 150 * skill_lv + sstatus->str + sstatus->int_; // !TODO: Confirm stat modifier
			RE_LVL_DMOD(100);
#else
			// Pre-Renewal: skill ratio for weapon part of damage [helvetica]
			skillratio += -100 + 100 * skill_lv;
#endif
			break;
		case PA_SACRIFICE:
			skillratio += -10 + 10 * skill_lv;
			break;
		case PA_SHIELDCHAIN:
#ifdef RENEWAL
			skillratio = -100 + 300 + 200 * skill_lv;

			if( sd != nullptr ){
				int16 index = sd->equip_index[EQI_HAND_L];

				// Damage affected by the shield's weight and refine.
				if( index >= 0 && sd->inventory_data[index] != nullptr && sd->inventory_data[index]->type == IT_ARMOR ){
					skillratio += sd->inventory_data[index]->weight / 10 + 4 * sd->inventory.u.items_inventory[index].refine;
				}

				// Damage affected by shield mastery
				if( sc != nullptr && sc->getSCE( SC_SHIELD_POWER ) ){
					skillratio += skill_lv * 14 * pc_checkskill( sd, IG_SHIELD_MASTERY );
				}
			}

			RE_LVL_DMOD(100);
#else
			skillratio += 30 * skill_lv;
#endif
			if (sc && sc->getSCE(SC_SHIELD_POWER))// Whats the official increase? [Rytech]
				skillratio += skillratio * 50 / 100;
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
			skillratio += 60 + 20 * skill_lv;
			break;
		case TK_TURNKICK:
		case TK_COUNTER:
			skillratio += 90 + 30 * skill_lv;
			break;
		case TK_JUMPKICK:
			//Different damage formulas depending on damage trigger
			if (sc && sc->getSCE(SC_COMBO) && sc->getSCE(SC_COMBO)->val1 == skill_id)
				skillratio += -100 + 4 * status_get_lv(src); //Tumble formula [4%*baselevel]
			else if (wd->miscflag) {
				skillratio += -100 + 4 * status_get_lv(src); //Running formula [4%*baselevel]
				if (sc && sc->getSCE(SC_SPURT)) //Spurt formula [8%*baselevel]
					skillratio *= 2;
			}
			else
				skillratio += -70 + 10 * skill_lv;
			break;
		case GS_TRIPLEACTION:
			skillratio += 50 * skill_lv;
			break;
		case GS_BULLSEYE:
			//Only works well against brute/demihumans non bosses.
			if((tstatus->race == RC_BRUTE || tstatus->race == RC_DEMIHUMAN || tstatus->race == RC_PLAYER_HUMAN || tstatus->race == RC_PLAYER_DORAM) && !status_has_mode(tstatus,MD_STATUSIMMUNE))
				skillratio += 400;
			break;
		case GS_TRACKING:
			skillratio += 100 * (skill_lv + 1);
			break;
		case GS_PIERCINGSHOT:
#ifdef RENEWAL
			if (sd && sd->weapontype1 == W_RIFLE)
				skillratio += 150 + 30 * skill_lv;
			else
				skillratio += 100 + 20 * skill_lv;
#else
			skillratio += 20 * skill_lv;
#endif
			break;
		case GS_RAPIDSHOWER:
			skillratio += 400 + 50 * skill_lv;
			break;
		case GS_DESPERADO:
			skillratio += 50 * (skill_lv - 1);
			if (sc && sc->getSCE(SC_FALLEN_ANGEL))
				skillratio *= 2;
			break;
		case GS_DUST:
			skillratio += 50 * skill_lv;
			break;
		case GS_FULLBUSTER:
			skillratio += 100 * (skill_lv + 2);
			break;
		case GS_SPREADATTACK:
#ifdef RENEWAL
			skillratio += 30 * skill_lv;
#else
			skillratio += 20 * (skill_lv - 1);
#endif
			break;
#ifdef RENEWAL
		case GS_GROUNDDRIFT:
			skillratio += 100 + 20 * skill_lv;
			break;
#endif
		case NJ_HUUMA:
#ifdef RENEWAL
			skillratio += -150 + 250 * skill_lv;
#else
			skillratio += 50 + 150 * skill_lv;
#endif
			break;
		case NJ_TATAMIGAESHI:
			skillratio += 10 * skill_lv;
#ifdef RENEWAL
			skillratio *= 2;
#endif
			break;
		case NJ_KASUMIKIRI:
#ifdef RENEWAL
			skillratio += 20 * skill_lv;
#else
			skillratio += 10 * skill_lv;
#endif
			break;
		case NJ_KIRIKAGE:
#ifdef RENEWAL
			skillratio += -50 + 150 * skill_lv;
#else
			skillratio += 100 * (skill_lv - 1);
#endif
			break;
#ifdef RENEWAL
		case NJ_SYURIKEN:
			skillratio += 5 * skill_lv;
			break;
		case NJ_KUNAI:
			skillratio += -100 + 100 * skill_lv;
			break;
		case KN_CHARGEATK:
			skillratio += 600;
			break;
		case AS_VENOMKNIFE:
			skillratio += 400;
			break;
#else
		case KN_CHARGEATK: { // +100% every 3 cells of distance but hard-limited to 500%
				int k = (wd->miscflag-1)/3;
				if (k < 0)
					k = 0;
				else if (k > 4)
					k = 4;
				skillratio += 100 * k;
			}
			break;
#endif
		case HT_PHANTASMIC:
#ifdef RENEWAL
			skillratio += 400;
#else
			skillratio += 50;
#endif
			break;
		case MO_BALKYOUNG:
#ifdef RENEWAL
			skillratio += 700;
#else
			skillratio += 200;
#endif
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
			skillratio += -100 + 1050 + 150 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case RK_HUNDREDSPEAR:
			skillratio += -100 + 600 + 200 * skill_lv;
			if (sd)
				skillratio += 50 * pc_checkskill(sd,LK_SPIRALPIERCE);
			if (sc) {
				if( sc->getSCE( SC_DRAGONIC_AURA ) ){
					skillratio += sc->getSCE( SC_DRAGONIC_AURA )->val1 * 160;
				}

				if (sc->getSCE(SC_CHARGINGPIERCE_COUNT) && sc->getSCE(SC_CHARGINGPIERCE_COUNT)->val1 >= 10)
					skillratio *= 2;
			}
			RE_LVL_DMOD(100);
			break;
		case RK_WINDCUTTER:
			if (sd) {
				if (sd->weapontype1 == W_2HSWORD)
					skillratio += -100 + 250 * skill_lv;
				else if (sd->weapontype1 == W_1HSPEAR || sd->weapontype1 == W_2HSPEAR)
					skillratio += -100 + 400 * skill_lv;
				else
					skillratio += -100 + 300 * skill_lv;
			} else
				skillratio += -100 + 300 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case RK_IGNITIONBREAK:
			skillratio += -100 + 450 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case NPC_IGNITIONBREAK:
			// 3x3 cell Damage   = 1000  1500  2000  2500  3000 %
			// 7x7 cell Damage   = 750   1250  1750  2250  2750 %
			// 11x11 cell Damage = 500   1000  1500  2000  2500 %
			i = distance_bl(src,target);
			if (i < 2)
				skillratio += -100 + 500 * (skill_lv + 1);
			else if (i < 4)
				skillratio += -100 + 250 + 500 * skill_lv;
			else
				skillratio += -100 + 500 * skill_lv;
			break;
		case RK_STORMBLAST:
			skillratio += -100 + (((sd) ? pc_checkskill(sd,RK_RUNEMASTERY) : 0) + sstatus->str / 6) * 100; // ATK = [{Rune Mastery Skill Level + (Caster's STR / 6)} x 100] %
			RE_LVL_DMOD(100);
			break;
		case RK_PHANTOMTHRUST: // ATK = [{(Skill Level x 50) + (Spear Master Level x 10)} x Caster's Base Level / 150] %
			skillratio += -100 + 50 * skill_lv + 10 * (sd ? pc_checkskill(sd,KN_SPEARMASTERY) : 5);
			RE_LVL_DMOD(150); // Base level bonus.
			break;
		// case NPC_PHANTOMTHRUST:	// ATK = 100% for all level
		case GC_CROSSIMPACT:
			skillratio += -100 + 1400 + 150 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case GC_COUNTERSLASH:
			//ATK [{(Skill Level x 150) + 300} x Caster's Base Level / 120]% + ATK [(AGI x 2) + (Caster's Job Level x 4)]%
			skillratio += -100 + 300 + 150 * skill_lv;
			RE_LVL_DMOD(120);
			skillratio += sstatus->agi * 2;
			// If 4th job, job level of your 3rd job counts
			skillratio += (sd ? (sd->class_&JOBL_FOURTH ? sd->change_level_4th : sd->status.job_level) * 4 : 0);
			break;
		case GC_VENOMPRESSURE:
			skillratio += 900;
			break;
		case GC_PHANTOMMENACE:
			skillratio += 200;
			break;
		case GC_ROLLINGCUTTER:
			skillratio += -100 + 50 + 80 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case GC_CROSSRIPPERSLASHER:
			skillratio += -100 + 80 * skill_lv + (sstatus->agi * 3);
			RE_LVL_DMOD(100);
			if (sc && sc->getSCE(SC_ROLLINGCUTTER))
				skillratio += sc->getSCE(SC_ROLLINGCUTTER)->val1 * 200;
			break;
		case GC_DARKCROW:
			skillratio += 100 * (skill_lv - 1);
			break;
		case AB_DUPLELIGHT_MELEE:
			skillratio += 50 + 15 * skill_lv;
			break;
		case NPC_ARROWSTORM:
			if (skill_lv > 4)
				skillratio += 1900;
			else
				skillratio += 900;
			break;
		case NPC_DRAGONBREATH:
			if (skill_lv > 5)
				skillratio += 500 + 500 * (skill_lv - 5);	// Level 6-10 is using water element, like RK_DRAGONBREATH_WATER
			else
				skillratio += 500 + 500 * skill_lv;	// Level 1-5 is using fire element, like RK_DRAGONBREATH
			break;
		case RA_ARROWSTORM:
			if (sc && sc->getSCE(SC_FEARBREEZE))
				skillratio += -100 + 200 + 250 * skill_lv;
			else
				skillratio += -100 + 200 + 180 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case RA_AIMEDBOLT:
			if (sc && sc->getSCE(SC_FEARBREEZE))
				skillratio += -100 + 800 + 35 * skill_lv;
			else
				skillratio += -100 + 500 + 20 * skill_lv;	
			RE_LVL_DMOD(100);
			break;
		case RA_CLUSTERBOMB:
			skillratio += 100 + 100 * skill_lv;
			break;
		case RA_WUGDASH:// ATK 300%
			skillratio += 200;
			break;
		case RA_WUGSTRIKE:
			skillratio += -100 + 200 * skill_lv;
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
			skillratio += -100 + 260 * skill_lv + sstatus->dex; // !TODO: What's the DEX bonus?
			RE_LVL_DMOD(100);
			break;
		case NC_PILEBUNKER:
			skillratio += 200 + 100 * skill_lv + status_get_str(src);
			RE_LVL_DMOD(100);
			break;
		case NC_VULCANARM:
			skillratio += -100 + 230 * skill_lv + sstatus->dex; // !TODO: What's the DEX bonus?
			RE_LVL_DMOD(100);
			break;
		case NC_FLAMELAUNCHER:
		case NC_COLDSLOWER:
			skillratio += 200 + 300 * skill_lv;
			RE_LVL_DMOD(150);
			break;
		case NC_ARMSCANNON:
			skillratio += -100 + 400 + 350 * skill_lv;
			RE_LVL_DMOD(100);
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
			skillratio += -100 + ((sstatus->str + sstatus->dex)/ 2) + 300 + 100 * skill_lv;
			RE_LVL_DMOD(100);
			if (sc && sc->getSCE(SC_ABR_BATTLE_WARIOR))
				skillratio *= 2;
			break;
		case NC_MAGMA_ERUPTION: // 'Slam' damage
			skillratio += 350 + 50 * skill_lv;
			break;
		case NC_AXETORNADO:
			skillratio += -100 + 200 + 180 * skill_lv + sstatus->vit * 2;
			if (sc && sc->getSCE(SC_AXE_STOMP))
				skillratio += 380;
			RE_LVL_DMOD(100);
			break;
		case SC_FATALMENACE:
			skillratio += 120 * skill_lv + sstatus->agi; // !TODO: What's the AGI bonus?

			if( sc != nullptr && sc->getSCE( SC_ABYSS_DAGGER ) ){
				skillratio += 30 * skill_lv;
			}

			RE_LVL_DMOD(100);
			break;
		case SC_TRIANGLESHOT:
			skillratio += -100 + 230 * skill_lv + 3 * sstatus->agi;
			RE_LVL_DMOD(100);
			break;
		case SC_FEINTBOMB:
			skillratio += -100 + (skill_lv + 1) * sstatus->dex / 2 * ((sd) ? sd->status.job_level / 10 : 1);
			RE_LVL_DMOD(120);
			break;
		case LG_CANNONSPEAR:
			skillratio += -100 + skill_lv * ( 120 + sstatus->str );

			if( sc != nullptr && sc->getSCE( SC_SPEAR_SCAR ) ){
				skillratio += 400;
			}

			RE_LVL_DMOD(100);
			break;
		case LG_BANISHINGPOINT:
			skillratio += -100 + ( 100 * skill_lv );

			if( sd != nullptr ){
				skillratio += pc_checkskill( sd, SM_BASH ) * 70;
			}

			if( sc != nullptr && sc->getSCE( SC_SPEAR_SCAR ) ){
				skillratio += 800;
			}

			RE_LVL_DMOD(100);
			break;
		case LG_SHIELDPRESS:
			skillratio += -100 + 200 * skill_lv;
			if (sd) {
				// Shield Press only considers base STR without job bonus
				skillratio += sd->status.str;

				if( sc != nullptr && sc->getSCE( SC_SHIELD_POWER ) ){
					skillratio += skill_lv * 15 * pc_checkskill( sd, IG_SHIELD_MASTERY );
				}

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
		case LG_MOONSLASHER:
			skillratio += -100 + 120 * skill_lv + ((sd) ? pc_checkskill(sd,LG_OVERBRAND) * 80 : 0);
			RE_LVL_DMOD(100);
			break;
		case LG_OVERBRAND:
			if(sc && sc->getSCE(SC_OVERBRANDREADY))
				skillratio += -100 + 500 * skill_lv;
			else
				skillratio += -100 + 350 * skill_lv;
			skillratio += ((sd) ? pc_checkskill(sd, CR_SPEARQUICKEN) * 50 : 0);
			RE_LVL_DMOD(100);
			break;
		case LG_EARTHDRIVE:
			skillratio += -100 + 380 * skill_lv + sstatus->str + sstatus->vit; // !TODO: What's the STR/VIT bonus?

			if( sc != nullptr && sc->getSCE( SC_SHIELD_POWER ) ){
				skillratio += skill_lv * 37 * pc_checkskill( sd, IG_SHIELD_MASTERY );
			}

			RE_LVL_DMOD(100);
			break;
		case LG_HESPERUSLIT:
			if (sc && sc->getSCE(SC_INSPIRATION))
				skillratio += -100 + 450 * skill_lv;
			else
				skillratio += -100 + 300 * skill_lv;
			skillratio += sstatus->vit / 6; // !TODO: What's the VIT bonus?
			RE_LVL_DMOD(100);
			break;
		case SR_EARTHSHAKER:
			if (tsc && ((tsc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK)) || tsc->getSCE(SC_CAMOUFLAGE) || tsc->getSCE(SC_STEALTHFIELD) || tsc->getSCE(SC__SHADOWFORM))) {
				//[(Skill Level x 300) x (Caster Base Level / 100) + (Caster STR x 3)] %
				skillratio += -100 + 300 * skill_lv;
				RE_LVL_DMOD(100);
				skillratio += status_get_str(src) * 3;
			} else { //[(Skill Level x 400) x (Caster Base Level / 100) + (Caster STR x 2)] %
				skillratio += -100 + 400 * skill_lv;
				RE_LVL_DMOD(100);
				skillratio += status_get_str(src) * 2;
			}
			break;

		case SR_DRAGONCOMBO:
			skillratio += 100 + 80 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case SR_FALLENEMPIRE:
			// ATK [(Skill Level x 300 + 100) x Caster Base Level / 150] %
			skillratio += 300 * skill_lv;
			RE_LVL_DMOD(150);
 			break;
		case SR_TIGERCANNON:
			{
				unsigned int hp = sstatus->max_hp * (10 + (skill_lv * 2)) / 100,
							 sp = sstatus->max_sp * (5 + skill_lv) / 100;

				if (wd->miscflag&8)
					// Base_Damage = [((Caster consumed HP + SP) / 2) x Caster Base Level / 100] %
					skillratio += -100 + (hp + sp) / 2;
				else
					// Base_Damage = [((Caster consumed HP + SP) / 4) x Caster Base Level / 100] %
					skillratio += -100 + (hp + sp) / 4;
				RE_LVL_DMOD(100);
			}
			if (sc->getSCE(SC_GT_REVITALIZE))
				skillratio += skillratio * 30 / 100;
			break;
		case SR_SKYNETBLOW:
			//ATK [{(Skill Level x 200) + (Caster AGI)} x Caster Base Level / 100] %
			skillratio += -100 + 200 * skill_lv + sstatus->agi / 6; // !TODO: Confirm AGI bonus
			RE_LVL_DMOD(100);
			break;

		case SR_RAMPAGEBLASTER:
			if (tsc && tsc->getSCE(SC_EARTHSHAKER)) {
				skillratio += 1400 + 550 * skill_lv;
				RE_LVL_DMOD(120);
			} else {
				skillratio += 900 + 350 * skill_lv;
				RE_LVL_DMOD(150);
			}
			if (sc->getSCE(SC_GT_CHANGE))
				skillratio += skillratio * 30 / 100;
			break;
		case SR_KNUCKLEARROW:
			if (wd->miscflag&4) { // ATK [(Skill Level x 150) + (1000 x Target current weight / Maximum weight) + (Target Base Level x 5) x (Caster Base Level / 150)] %
				skillratio += -100 + 150 * skill_lv + status_get_lv(target) * 5;
				if (tsd && tsd->weight)
					skillratio += 100 * tsd->weight / tsd->max_weight;
				RE_LVL_DMOD(150);
			} else {
				if (status_get_class_(target) == CLASS_BOSS)
					skillratio += 400 + 200 * skill_lv;
				else // ATK [(Skill Level x 100 + 500) x Caster Base Level / 100] %
					skillratio += 400 + 100 * skill_lv;
				RE_LVL_DMOD(100);
			}
			if (sc->getSCE(SC_GT_CHANGE))
				skillratio += skillratio * 30 / 100;
			break;
		case SR_WINDMILL: // ATK [(Caster Base Level + Caster DEX) x Caster Base Level / 100] %
			skillratio += -100 + status_get_lv(src) + sstatus->dex;
			RE_LVL_DMOD(100);
			break;
		case SR_GATEOFHELL:
			if (sc && sc->getSCE(SC_COMBO) && sc->getSCE(SC_COMBO)->val1 == SR_FALLENEMPIRE)
				skillratio += -100 + 800 * skill_lv;
			else
				skillratio += -100 + 500 * skill_lv;
			RE_LVL_DMOD(100);
			if (sc->getSCE(SC_GT_REVITALIZE))
				skillratio += skillratio * 30 / 100;
			break;
		case SR_GENTLETOUCH_QUIET:
			skillratio += -100 + 100 * skill_lv + sstatus->dex;
			RE_LVL_DMOD(100);
			break;
		case SR_HOWLINGOFLION:
			skillratio += -100 + 500 * skill_lv;
			RE_LVL_DMOD(100);	
			break;
		case SR_RIDEINLIGHTNING: 
			skillratio += -100 + 40 * skill_lv;
			if (sd && sd->status.weapon == W_KNUCKLE)
				skillratio += 50 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case WM_SEVERE_RAINSTORM_MELEE:
			//ATK [{(Caster DEX / 300 + AGI / 200)} x Caster Base Level / 100] %
			skillratio += -100 + 100 * skill_lv + (sstatus->dex / 300 + sstatus->agi / 200);
			if (wd->miscflag&4) // Whip/Instrument equipped
				skillratio += 20 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case WM_GREAT_ECHO:
			skillratio += -100 + 250 + 500 * skill_lv;
			if (sd) {
				skillratio += pc_checkskill(sd, WM_LESSON) * 50; // !TODO: Confirm bonus
				if (skill_check_pc_partner(sd, skill_id, &skill_lv, AREA_SIZE, 0) > 0)
					skillratio *= 2;
			}
			RE_LVL_DMOD(100);
			break;
		case GN_CART_TORNADO: { // ATK [( Skill Level x 200 ) + ( Cart Weight / ( 150 - Caster Base STR ))] + ( Cart Remodeling Skill Level x 50 )] %
				skillratio += -100 + 200 * skill_lv;
				if(sd && sd->cart_weight)
					skillratio += sd->cart_weight / 10 / (150 - min(sd->status.str,120)) + pc_checkskill(sd,GN_REMODELING_CART) * 50;
				if (sc && sc->getSCE(SC_BIONIC_WOODENWARRIOR))
					skillratio *= 2;
			}
			break;
		case GN_CARTCANNON:
			skillratio += -100 + (250 + 20 * pc_checkskill(sd, GN_REMODELING_CART)) * skill_lv + 2 * sstatus->int_ / (6 - pc_checkskill(sd, GN_REMODELING_CART));
			RE_LVL_DMOD(100);
			break;
		case GN_SPORE_EXPLOSION:
			skillratio += -100 + 400 + 200 * skill_lv;
			RE_LVL_DMOD(100);
			if (sc && sc->getSCE(SC_BIONIC_WOODEN_FAIRY))
				skillratio *= 2;
			break;
		case GN_WALLOFTHORN:
			skillratio += 10 * skill_lv;
			break;
		case GN_CRAZYWEED_ATK:
			skillratio += -100 + 700 + 100 * skill_lv;
			RE_LVL_DMOD(100);
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
		case GN_HELLS_PLANT_ATK:
			skillratio += -100 + 100 * skill_lv + sstatus->int_ * (sd ? pc_checkskill(sd, AM_CANNIBALIZE) : 5); // !TODO: Confirm INT and Cannibalize bonus
			RE_LVL_DMOD(100);
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
			skillratio += -100 + 200 * skill_lv;
			RE_LVL_DMOD(120);
			if(tsc && tsc->getSCE(SC_JYUMONJIKIRI))
				skillratio += skill_lv * status_get_lv(src);
			if (sc && sc->getSCE(SC_KAGEMUSYA))
				skillratio += skillratio * sc->getSCE(SC_KAGEMUSYA)->val2 / 100;
			break;
		case KO_HUUMARANKA:
			skillratio += -100 + 150 * skill_lv + sstatus->str + (sd ? pc_checkskill(sd,NJ_HUUMA) * 100 : 0);
			RE_LVL_DMOD(100);
			if (sc && sc->getSCE(SC_KAGEMUSYA))
				skillratio += skillratio * sc->getSCE(SC_KAGEMUSYA)->val2 / 100;
			break;
		case KO_SETSUDAN:
			skillratio += 100 * (skill_lv - 1);
			RE_LVL_DMOD(100);
			if (tsc) {
				struct status_change_entry *sce;

				if ((sce = tsc->getSCE(SC_SPIRIT)) || (sce = tsc->getSCE(SC_SOULGOLEM)) || (sce = tsc->getSCE(SC_SOULSHADOW)) || (sce = tsc->getSCE(SC_SOULFALCON)) || (sce = tsc->getSCE(SC_SOULFAIRY))) // Bonus damage added when target is soul linked.
					skillratio += 200 * sce->val1;
			}
			break;
		case KO_BAKURETSU:
			skillratio += -100 + (sd ? pc_checkskill(sd,NJ_TOBIDOUGU) : 1) * (50 + sstatus->dex / 4) * skill_lv * 4 / 10;
			RE_LVL_DMOD(120);
			skillratio += 10 * (sd ? sd->status.job_level : 1);
			if (sc && sc->getSCE(SC_KAGEMUSYA))
				skillratio += skillratio * sc->getSCE(SC_KAGEMUSYA)->val2 / 100;
			break;
		case KO_MAKIBISHI:
			skillratio += -100 + 20 * skill_lv;
			break;
		case MH_NEEDLE_OF_PARALYZE:
			skillratio += -100 + 450 * skill_lv * status_get_lv(src) / 100 + sstatus->dex; // !TODO: Confirm Base Level and DEX bonus
			break;
		case MH_TOXIN_OF_MANDARA:
			skillratio += -100 + 400 + 450 * skill_lv * status_get_lv(src) / 100 + sstatus->dex; // !TODO: Confirm Base Level and DEX bonus
			break;
		case MH_NEEDLE_STINGER:
			skillratio += -100 + 200 + 500 * skill_lv * status_get_lv(src) / 100 + sstatus->dex; // !TODO: Confirm Base Level and DEX bonus
			break;
		case MH_STAHL_HORN:
			skillratio += -100 + 1000 + 300 * skill_lv * status_get_lv(src) / 150 + sstatus->vit; // !TODO: Confirm VIT bonus
			break;
		case MH_GLANZEN_SPIES:
			skillratio += -100 + 300 + 450 * skill_lv * status_get_lv(src) / 100 + sstatus->vit; // !TODO: Confirm VIT bonus
			break;
		case MH_LAVA_SLIDE:
			skillratio += -100 + 50 * skill_lv;
			break;
		case MH_BLAST_FORGE:
			skillratio += -100 + 70 * skill_lv * status_get_lv(src) / 100 + sstatus->str;
			break;
		case MH_SONIC_CRAW:
			skillratio += -100 + 60 * skill_lv * status_get_lv(src) / 150;
			break;
		case MH_BLAZING_AND_FURIOUS:
			skillratio += -100 + 80 * skill_lv * status_get_lv(src) / 100 + sstatus->str;
			break;
		case MH_THE_ONE_FIGHTER_RISES:
			skillratio += -100 + 580 * skill_lv * status_get_lv(src) / 100 + sstatus->str;
			break;
		case MH_SILVERVEIN_RUSH:
			skillratio += -100 + 250 * skill_lv * status_get_lv(src) / 100 + sstatus->str; // !TODO: Confirm STR bonus
			break;
		case MH_MIDNIGHT_FRENZY:
			skillratio += -100 + 450 * skill_lv * status_get_lv(src) / 150 + sstatus->str; // !TODO: Confirm STR bonus
			break;
		case MH_MAGMA_FLOW:
			skillratio += -100 + (100 * skill_lv + 3 * status_get_lv(src)) * status_get_lv(src) / 120;
			break;
		case RL_MASS_SPIRAL:
			skillratio += -100 + 200 * skill_lv;
			break;
		case RL_FIREDANCE:
			skillratio += 100 + 100 * skill_lv;
			skillratio += (sd ? pc_checkskill(sd, GS_DESPERADO) * 20 : 0);
			RE_LVL_DMOD(100);
			break;
		case RL_BANISHING_BUSTER:
			skillratio += -100 + 1000 + 200 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case RL_S_STORM:
			skillratio += -100 + 1700 + 200 * skill_lv;
			break;
		case RL_SLUGSHOT:
			if (target->type == BL_MOB)
				skillratio += -100 + 1200 * skill_lv;
			else
				skillratio += -100 + 2000 * skill_lv;
			skillratio *= 2 + tstatus->size;
			break;
		case RL_D_TAIL:
			skillratio += -100 + 500 + 200 * skill_lv;
			if (sd && (wd->miscflag & 8))
				skillratio *= 2;
			RE_LVL_DMOD(100);
			break;
		case RL_R_TRIP:
			skillratio += -100 + 350 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case RL_R_TRIP_PLUSATK:
			skillratio += -100 + 300 + 300 * skill_lv;
			break;
		case RL_H_MINE:
			if (sd && sd->flicker) // Flicker explosion damage: 500 + 300 * SkillLv
				skillratio += -100 + 500 + 300 * skill_lv;
			else // 200 + 200 * SkillLv
				skillratio += -100 + 200 + 200 * skill_lv;
			break;
		case RL_HAMMER_OF_GOD:
			skillratio += -100 + 100 * skill_lv;
			if (sd) {
				if (wd->miscflag & 8)
					skillratio += 400 * sd->spiritball_old;
				else
					skillratio += 150 * sd->spiritball_old;
			}
			RE_LVL_DMOD(100);
			break;
		case RL_FIRE_RAIN:
		case RL_AM_BLAST:
			skillratio += -100 + 3500 + 300 * skill_lv;
			break;
		case SU_BITE:
			skillratio += 100;
			break;
		case SU_SCRATCH:
			skillratio += -50 + 50 * skill_lv;
			break;
		case SU_SCAROFTAROU:
			skillratio += -100 + 100 * skill_lv;
			if (sd && pc_checkskill(sd, SU_SPIRITOFLIFE))
				skillratio += skillratio * status_get_hp(src) / status_get_max_hp(src);
			break;
		case SU_PICKYPECK:
		case SU_PICKYPECK_DOUBLE_ATK:
			skillratio += 100 + 100 * skill_lv;
			if (status_get_hp(target) < (status_get_max_hp(target) / 2))
				skillratio *= 2;
			if (sd && pc_checkskill(sd, SU_SPIRITOFLIFE))
				skillratio += skillratio * status_get_hp(src) / status_get_max_hp(src);
			break;
		case SU_LUNATICCARROTBEAT:
		case SU_LUNATICCARROTBEAT2:
			skillratio += 100 + 100 * skill_lv;
			if (sd && pc_checkskill(sd, SU_SPIRITOFLIFE))
				skillratio += skillratio * status_get_hp(src) / status_get_max_hp(src);
			if (status_get_lv(src) > 99)
				skillratio += sstatus->str;
			RE_LVL_DMOD(100);
			break;
		case SU_SVG_SPIRIT:
			skillratio += 150 + 150 * skill_lv;
			if (sd && pc_checkskill(sd, SU_SPIRITOFLIFE))
				skillratio += skillratio * status_get_hp(src) / status_get_max_hp(src);
			break;
		case SJ_FULLMOONKICK:
			skillratio += 1000 + 100 * skill_lv;
			RE_LVL_DMOD(100);
			if (sc && sc->getSCE(SC_LIGHTOFMOON))
				skillratio += skillratio * sc->getSCE(SC_LIGHTOFMOON)->val2 / 100;
			break;
		case SJ_NEWMOONKICK:
			skillratio += 600 + 100 * skill_lv;
			break;
		case SJ_STAREMPEROR:
			skillratio += 700 + 200 * skill_lv;
			break;
		case SJ_SOLARBURST:
			skillratio += 900 + 220 * skill_lv;
			RE_LVL_DMOD(100);
			if (sc && sc->getSCE(SC_LIGHTOFSUN))
				skillratio += skillratio * sc->getSCE(SC_LIGHTOFSUN)->val2 / 100;
			break;
		case SJ_PROMINENCEKICK:
				skillratio += 50 + 50 * skill_lv;
			break;
		case SJ_FALLINGSTAR_ATK:
		case SJ_FALLINGSTAR_ATK2:
			skillratio += 100 * skill_lv;
			RE_LVL_DMOD(100);
			if (sc && sc->getSCE(SC_LIGHTOFSTAR))
				skillratio += skillratio * sc->getSCE(SC_LIGHTOFSTAR)->val2 / 100;
			break;
		case DK_SERVANTWEAPON_ATK:
			skillratio += -100 + 600 + 850 * skill_lv;
			skillratio += 5 * sstatus->pow;
			RE_LVL_DMOD(100);
			break;
		case DK_SERVANT_W_PHANTOM:
			skillratio += -100 + 200 + 300 * skill_lv + 5 * sstatus->pow;
			RE_LVL_DMOD(100);
			break;
		case DK_SERVANT_W_DEMOL:
			skillratio += -100 + 500 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case DK_HACKANDSLASHER:
		case DK_HACKANDSLASHER_ATK:
			skillratio += -100 + 350 + 820 * skill_lv;
			skillratio += 7 * sstatus->pow;
			RE_LVL_DMOD(100);
			break;
		case DK_DRAGONIC_AURA:
			skillratio += 3650 * skill_lv + 10 * sstatus->pow;
			if (tstatus->race == RC_DEMIHUMAN || tstatus->race == RC_ANGEL)
				skillratio += 150 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case DK_MADNESS_CRUSHER:
			skillratio += -100 + 1000 + 3800 * skill_lv;
			skillratio += 10 * sstatus->pow;
			if( sd != nullptr ){
				int16 index = sd->equip_index[EQI_HAND_R];

				if( index >= 0 && sd->inventory_data[index] != nullptr ){
					skillratio += sd->inventory_data[index]->weight / 10 * sd->inventory_data[index]->weapon_level;
				}
			}
			RE_LVL_DMOD(100);
			if (sc && sc->getSCE(SC_CHARGINGPIERCE_COUNT) && sc->getSCE(SC_CHARGINGPIERCE_COUNT)->val1 >= 10)
				skillratio *= 2;
			break;
		case DK_STORMSLASH:
			skillratio += -100 + 300 + 750 * skill_lv;
			skillratio += 5 * sstatus->pow;
			RE_LVL_DMOD(100);
			if (sc && sc->getSCE(SC_GIANTGROWTH) && rnd_chance(60, 100))
				skillratio *= 2;
			break;
		case DK_DRAGONIC_BREATH:
			skillratio += -100 + 50 + 350 * skill_lv;
			skillratio += 7 * sstatus->pow;

			if (sc && sc->getSCE(SC_DRAGONIC_AURA)) {
				skillratio += 3 * sstatus->pow;
				skillratio += (skill_lv * (sstatus->max_hp * 25 / 100) * 7) / 100;	// Skill level x 0.07 x ((MaxHP / 4) + (MaxSP / 2))
				skillratio += (skill_lv * (sstatus->max_sp * 50 / 100) * 7) / 100;
			} else {
				skillratio += (skill_lv * (sstatus->max_hp * 25 / 100) * 5) / 100;	// Skill level x 0.05 x ((MaxHP / 4) + (MaxSP / 2))
				skillratio += (skill_lv * (sstatus->max_sp * 50 / 100) * 5) / 100;
			}

			RE_LVL_DMOD(100);
			break;
		case IQ_OLEUM_SANCTUM:
			skillratio += -100 + 500 + 2000 * skill_lv + 5 * sstatus->pow;
			RE_LVL_DMOD(100);
			break;
		case IQ_MASSIVE_F_BLASTER:
			skillratio += -100 + 2300 * skill_lv + 15 * sstatus->pow;
			if (tstatus->race == RC_BRUTE || tstatus->race == RC_DEMON)
				skillratio += 150 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case IQ_EXPOSION_BLASTER:
			skillratio += -100 + 450 + 2600 * skill_lv;
			skillratio += 10 * sstatus->pow;

			if( tsc != nullptr && tsc->getSCE( SC_HOLY_OIL ) ){
				skillratio += 950 * skill_lv;
			}

			RE_LVL_DMOD(100);
			break;
		case IQ_FIRST_BRAND:
			skillratio += -100 + 1200 * skill_lv + 5 * sstatus->pow;
			RE_LVL_DMOD(100);
			break;
		case IQ_SECOND_FLAME:
			skillratio += -100 + 200 + 2900 * skill_lv + 9 * sstatus->pow;
			RE_LVL_DMOD(100);
			break;
		case IQ_SECOND_FAITH:
			skillratio += -100 + 100 + 2300 * skill_lv + 5 * sstatus->pow;
			RE_LVL_DMOD(100);
			break;
		case IQ_SECOND_JUDGEMENT:
			skillratio += -100 + 150 + 2600 * skill_lv + 7 * sstatus->pow;
			RE_LVL_DMOD(100);
			break;
		case IQ_THIRD_PUNISH:
			skillratio += -100 + 450 + 1800 * skill_lv;
			skillratio += 10 * sstatus->pow;
			RE_LVL_DMOD(100);
			break;
		case IQ_THIRD_FLAME_BOMB:
			skillratio += -100 + 650 * skill_lv + 10 * sstatus->pow;
			skillratio += sstatus->max_hp * 20 / 100;
			RE_LVL_DMOD(100);
			break;
		case IQ_THIRD_CONSECRATION:
			skillratio += -100 + 700 * skill_lv + 10 * sstatus->pow;
			RE_LVL_DMOD(100);
			break;
		case IG_GRAND_JUDGEMENT:
			skillratio += -100 + 250 + 1500 * skill_lv + 10 * sstatus->pow;
			if (tstatus->race == RC_PLANT || tstatus->race == RC_INSECT)
				skillratio += 100 + 150 * skill_lv;
			RE_LVL_DMOD(100);
			if ((i = pc_checkskill_imperial_guard(sd, 3)) > 0)
				skillratio += skillratio * i / 100;
			break;
		case IG_SHIELD_SHOOTING:
			skillratio += -100 + 1000 + 3500 * skill_lv;
			skillratio += 10 * sstatus->pow;
			skillratio += skill_lv * 150 * pc_checkskill( sd, IG_SHIELD_MASTERY );
			if (sd) { // Damage affected by the shield's weight and refine. Need official formula. [Rytech]
				short index = sd->equip_index[EQI_HAND_L];

				if (index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_ARMOR) {
					skillratio += (sd->inventory_data[index]->weight * 7 / 6) / 10;
					skillratio += sd->inventory.u.items_inventory[index].refine * 100;
				}
			}
			RE_LVL_DMOD(100);
			break;
		case IG_OVERSLASH:
			skillratio += -100 + 220 * skill_lv;
			skillratio += pc_checkskill(sd, IG_SPEAR_SWORD_M) * 50 * skill_lv;
			skillratio += 7 * sstatus->pow;
			RE_LVL_DMOD(100);
			if ((i = pc_checkskill_imperial_guard(sd, 3)) > 0)
				skillratio += skillratio * i / 100;
			break;
		case CD_EFFLIGO:
			skillratio += -100 + 1650 * skill_lv + 7 * sstatus->pow;
			skillratio += 8 * pc_checkskill( sd, CD_MACE_BOOK_M );
			if (tstatus->race == RC_UNDEAD || tstatus->race == RC_DEMON) {
				skillratio += 150 * skill_lv;
				skillratio += 7 * pc_checkskill( sd, CD_MACE_BOOK_M );
			}
			RE_LVL_DMOD(100);
			break;
		case CD_PETITIO:
			skillratio += -100 + (1050 + pc_checkskill(sd,CD_MACE_BOOK_M) * 50) * skill_lv + 5 * sstatus->pow;
			RE_LVL_DMOD(100);
			break;
		case SHC_DANCING_KNIFE:
			skillratio += -100 + 200 * skill_lv + 5 * sstatus->pow;
			RE_LVL_DMOD(100);
			break;
		case SHC_SAVAGE_IMPACT:
			skillratio += -100 + 105 * skill_lv + 5 * sstatus->pow;

			if( sc != nullptr && sc->getSCE( SC_SHADOW_EXCEED ) ){
				skillratio += 20 * skill_lv + 3 * sstatus->pow;	// !TODO: check POW ratio
			}

			RE_LVL_DMOD(100);
			break;
		case SHC_ETERNAL_SLASH:
			skillratio += -100 + 300 * skill_lv + 2 * sstatus->pow;

			if( sc != nullptr && sc->getSCE( SC_SHADOW_EXCEED ) ){
				skillratio += 120 * skill_lv + sstatus->pow;
			}

			RE_LVL_DMOD(100);
			break;
		case SHC_SHADOW_STAB:
			skillratio += -100 + 550 * skill_lv;
			skillratio += 5 * sstatus->pow;

			if (wd->miscflag & SKILL_ALTDMG_FLAG) {
				skillratio += 100 * skill_lv + 2 * sstatus->pow;
			}

			RE_LVL_DMOD(100);
			break;
		case SHC_IMPACT_CRATER:
			skillratio += -100 + 80 * skill_lv + 5 * sstatus->pow;
			RE_LVL_DMOD(100);
			break;
		case SHC_FATAL_SHADOW_CROW:
			skillratio += -100 + 1300 * skill_lv + 10 * sstatus->pow;
			if (tstatus->race == RC_DEMIHUMAN || tstatus->race == RC_DRAGON)
				skillratio += 150 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case MT_AXE_STOMP:
			skillratio += -100 + 450 + 1150 * skill_lv;
			skillratio += 5 * sstatus->pow;
			RE_LVL_DMOD(100);
			break;
		case MT_MIGHTY_SMASH:
			skillratio += -100 + 80 + 240 * skill_lv;
			skillratio += 5 * sstatus->pow;
			if (sc && sc->getSCE(SC_AXE_STOMP)) {
				skillratio += 20;
				skillratio += 5 * sstatus->pow;
			}
			RE_LVL_DMOD(100);
			break;
		case MT_RUSH_QUAKE:
			skillratio += -100 + 3600 * skill_lv + 10 * sstatus->pow;
			if (tstatus->race == RC_FORMLESS || tstatus->race == RC_INSECT)
				skillratio += 150 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case MT_A_MACHINE:// Formula unknown. Using Dancing Knife's formula for now. [Rytech]
			skillratio += -100 + 200 * skill_lv + 5 * sstatus->pow;
			RE_LVL_DMOD(100);
			break;
		case MT_SPARK_BLASTER:
			skillratio += -100 + 600 + 1400 * skill_lv;
			skillratio += 5 * sstatus->pow;
			RE_LVL_DMOD(100);
			break;
		case MT_TRIPLE_LASER:
			skillratio += -100 + 650 + 1150 * skill_lv;
			skillratio += 12 * sstatus->pow;
			RE_LVL_DMOD(100);
			break;
		case ABC_ABYSS_DAGGER:
			skillratio += -100 + 350 + 1400 * skill_lv;
			skillratio += 5 * sstatus->pow;
			RE_LVL_DMOD(100);
			break;
		case ABC_UNLUCKY_RUSH:
			skillratio += -100 + 100 + 300 * skill_lv + 5 * sstatus->pow;
			RE_LVL_DMOD(100);
			break;
		case ABC_CHAIN_REACTION_SHOT:
			skillratio += -100 + 850 * skill_lv;
			skillratio += 15 * sstatus->con;
			RE_LVL_DMOD(100);
			break;
		case ABC_CHAIN_REACTION_SHOT_ATK:
			skillratio += -100 + 800 + 2550 * skill_lv;
			skillratio += 15 * sstatus->con;
			RE_LVL_DMOD(100);
			break;
		case ABC_DEFT_STAB:
			skillratio += -100 + 700 + 550 * skill_lv;
			skillratio += 7 * sstatus->pow;
			RE_LVL_DMOD(100);
			break;
		case ABC_FRENZY_SHOT:
			skillratio += -100 + 250 + 800 * skill_lv;
			skillratio += 15 * sstatus->con;
			RE_LVL_DMOD(100);
			break;
		case WH_HAWKRUSH:
			skillratio += -100 + 500 * skill_lv + 5 * sstatus->con;
			if (sd)
				skillratio += skillratio * pc_checkskill(sd, WH_NATUREFRIENDLY) / 10;
			RE_LVL_DMOD(100);
			break;
		case WH_HAWKBOOMERANG:
			skillratio += -100 + 600 * skill_lv + 10 * sstatus->con;
			if (sd)
				skillratio += skillratio * pc_checkskill(sd, WH_NATUREFRIENDLY) / 10;
			if (tstatus->race == RC_BRUTE || tstatus->race == RC_FISH)
				skillratio += skillratio * 50 / 100;
			RE_LVL_DMOD(100);
			break;
		case WH_GALESTORM:
			skillratio += -100 + 1350 * skill_lv;
			skillratio += 10 * sstatus->con;
			RE_LVL_DMOD(100);
			if (sc && sc->getSCE(SC_CALAMITYGALE) && (tstatus->race == RC_BRUTE || tstatus->race == RC_FISH))
				skillratio += skillratio * 50 / 100;
			break;
		case WH_CRESCIVE_BOLT:
			skillratio += -100 + 500 + 1300 * skill_lv;
			skillratio += 5 * sstatus->con;
			RE_LVL_DMOD(100);
			if (sc) {
				if (sc->getSCE(SC_CRESCIVEBOLT))
					skillratio += skillratio * (20 * sc->getSCE(SC_CRESCIVEBOLT)->val1) / 100;

				if (sc->getSCE(SC_CALAMITYGALE)) {
					skillratio += skillratio * 20 / 100;

					if (tstatus->race == RC_BRUTE || tstatus->race == RC_FISH)
						skillratio += skillratio * 50 / 100;
				}
			}
			break;
		case WH_DEEPBLINDTRAP:
		case WH_SOLIDTRAP:
		case WH_SWIFTTRAP:
		case WH_FLAMETRAP:
			skillratio += -100 + 850 * skill_lv + 5 * sstatus->con;
			RE_LVL_DMOD(100);
			skillratio += skillratio * (20 * (sd ? pc_checkskill(sd, WH_ADVANCED_TRAP) : 5)) / 100;
			break;
		case BO_ACIDIFIED_ZONE_WATER:
		case BO_ACIDIFIED_ZONE_GROUND:
		case BO_ACIDIFIED_ZONE_WIND:
		case BO_ACIDIFIED_ZONE_FIRE:
		case BO_ACIDIFIED_ZONE_WATER_ATK:// These deal the same damage? [Rytech]
		case BO_ACIDIFIED_ZONE_GROUND_ATK:
		case BO_ACIDIFIED_ZONE_WIND_ATK:
		case BO_ACIDIFIED_ZONE_FIRE_ATK:
			skillratio += -100 + 400 * skill_lv + 5 * sstatus->pow;

			if( sc != nullptr && sc->getSCE( SC_RESEARCHREPORT ) ){
				skillratio += skillratio * 50 / 100;

				if (tstatus->race == RC_FORMLESS || tstatus->race == RC_PLANT)
					skillratio += skillratio * 50 / 100;
			}

			RE_LVL_DMOD(100);
			break;
		case BO_EXPLOSIVE_POWDER:
			skillratio += -100 + 500 + 650 * skill_lv;
			skillratio += 5 * sstatus->pow;
			if (sc && sc->getSCE(SC_RESEARCHREPORT))
				skillratio += 100 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case BO_MAYHEMIC_THORNS:
			skillratio += -100 + 200 + 300 * skill_lv;
			skillratio += 5 * sstatus->pow;
			if (sc && sc->getSCE(SC_RESEARCHREPORT))
				skillratio += 150;
			RE_LVL_DMOD(100);
			break;
		case TR_ROSEBLOSSOM:
			skillratio += -100 + 200 + 2000 * skill_lv;

			if (sd && pc_checkskill(sd, TR_STAGE_MANNER) > 0)
				skillratio += 3 * sstatus->con;

			if( tsc != nullptr && tsc->getSCE( SC_SOUNDBLEND ) ){
				skillratio += 200 * skill_lv;
			}

			RE_LVL_DMOD(100);
			if (sc && sc->getSCE(SC_MYSTIC_SYMPHONY)) {
				skillratio *= 2;

				if (tstatus->race == RC_FISH || tstatus->race == RC_DEMIHUMAN)
					skillratio += skillratio * 50 / 100;
			}
			break;
		case TR_ROSEBLOSSOM_ATK:
			skillratio += -100 + 250 + 2800 * skill_lv;

			if (sd && pc_checkskill(sd, TR_STAGE_MANNER) > 0)
				skillratio += 3 * sstatus->con;

			if( tsc != nullptr && tsc->getSCE( SC_SOUNDBLEND ) ){
				skillratio += 200 * skill_lv;
			}

			RE_LVL_DMOD(100);
			if (sc && sc->getSCE(SC_MYSTIC_SYMPHONY)) {
				skillratio *= 2;

				if (tstatus->race == RC_FISH || tstatus->race == RC_DEMIHUMAN)
					skillratio += skillratio * 50 / 100;
			}
			break;
		case TR_RHYTHMSHOOTING:
			skillratio += -100 + 550 + 950 * skill_lv;

			if (sd && pc_checkskill(sd, TR_STAGE_MANNER) > 0)
				skillratio += 5 * sstatus->con;

			if (tsc && tsc->getSCE(SC_SOUNDBLEND)) {
				skillratio += 300 + 100 * skill_lv;
				skillratio += 2 * sstatus->con;
			}

			RE_LVL_DMOD(100);
			if (sc && sc->getSCE(SC_MYSTIC_SYMPHONY)) {
				skillratio *= 2;

				if (tstatus->race == RC_FISH || tstatus->race == RC_DEMIHUMAN)
					skillratio += skillratio * 50 / 100;
			}
			break;
		case ABR_BATTLE_BUSTER:// Need official formula.
		case ABR_DUAL_CANNON_FIRE:// Need official formula.
			skillratio += -100 + 8000;
			break;
		case ABR_INFINITY_BUSTER:// Need official formula.
			skillratio += -100 + 50000;
			break;
		case HN_SPIRAL_PIERCE_MAX:
			skillratio += -100 + 1000 + 1500 * skill_lv;
			skillratio += pc_checkskill(sd, HN_SELFSTUDY_TATICS) * 3 * skill_lv;
			skillratio += 5 * sstatus->pow;
			switch (status_get_size(target)){
				case SZ_SMALL:
					skillratio = skillratio * 150 / 100;
					break;
				case SZ_MEDIUM:
					skillratio = skillratio * 130 / 100;
					break;
				case SZ_BIG:
					skillratio = skillratio * 120 / 100;
					break;
			}
			RE_LVL_DMOD(100);
			break;
		case HN_SHIELD_CHAIN_RUSH:
			skillratio += -100 + 850 + 1050 * skill_lv;
			skillratio += pc_checkskill(sd, HN_SELFSTUDY_TATICS) * 3 * skill_lv;
			skillratio += 5 * sstatus->pow;
			RE_LVL_DMOD(100);
			break;
		case HN_MEGA_SONIC_BLOW:
			skillratio += -100 + 900 + 750 * skill_lv;
			skillratio += pc_checkskill(sd, HN_SELFSTUDY_TATICS) * 5 * skill_lv;
			skillratio += 5 * sstatus->pow;
			if (status_get_hp(target) < status_get_max_hp(target) / 2)
				skillratio *= 2;
			RE_LVL_DMOD(100);
			break;
		case HN_DOUBLEBOWLINGBASH:
			skillratio += -100 + 250 + 400 * skill_lv;
			skillratio += pc_checkskill(sd, HN_SELFSTUDY_TATICS) * 3 * skill_lv;
			skillratio += 5 * sstatus->pow;
			RE_LVL_DMOD(100);
			break;
		case NW_HASTY_FIRE_IN_THE_HOLE:
			skillratio += -100 + 1500 + 1500 * skill_lv;
			skillratio += pc_checkskill( sd, NW_GRENADE_MASTERY ) * 20;
			skillratio += 5 * sstatus->con;
			RE_LVL_DMOD(100);
			break;
		case NW_BASIC_GRENADE:
			skillratio += -100 + 1500 + 2100 * skill_lv;
			skillratio += pc_checkskill( sd, NW_GRENADE_MASTERY ) * 50;
			skillratio += 5 * sstatus->con;
			RE_LVL_DMOD(100);
			break;
		case NW_GRENADES_DROPPING:
			skillratio += -100 + 550 + 850 * skill_lv;
			skillratio += pc_checkskill( sd, NW_GRENADE_MASTERY ) * 30;
			skillratio += 5 * sstatus->con;
			RE_LVL_DMOD(100);
			break;
		case NW_WILD_FIRE:
			skillratio += -100 + 1500 + 3000 * skill_lv;
			skillratio += 5 * sstatus->con;
			if (sc && sc->getSCE(SC_INTENSIVE_AIM_COUNT))
				skillratio += sc->getSCE(SC_INTENSIVE_AIM_COUNT)->val1 * 500 * skill_lv;
			if (sd && sd->weapontype1 == W_SHOTGUN)
				skillratio += 200 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case NW_MAGAZINE_FOR_ONE:
			skillratio += -100 + 250 + 500 * skill_lv;
			skillratio += 5 * sstatus->con;
			if (sc && sc->getSCE(SC_INTENSIVE_AIM_COUNT))
				skillratio += sc->getSCE(SC_INTENSIVE_AIM_COUNT)->val1 * 100 * skill_lv;
			if (sd && sd->weapontype1 == W_REVOLVER)
				skillratio += 50 + 300 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case NW_SPIRAL_SHOOTING:
			skillratio += -100 + 1200 + 1700 * skill_lv;
			skillratio += 5 * sstatus->con;
			if (sc && sc->getSCE(SC_INTENSIVE_AIM_COUNT))
				skillratio += sc->getSCE(SC_INTENSIVE_AIM_COUNT)->val1 * 150 * skill_lv;
			if (sd && sd->weapontype1 == W_RIFLE) 
				skillratio += 200 + 1100 * skill_lv;
			RE_LVL_DMOD(100);
			break;
		case NW_ONLY_ONE_BULLET:
			skillratio += -100 + 1200 + 3000 * skill_lv;
			skillratio += 5 * sstatus->con;
			if (sc && sc->getSCE(SC_INTENSIVE_AIM_COUNT))
				skillratio += sc->getSCE(SC_INTENSIVE_AIM_COUNT)->val1 * 350 * skill_lv;
			if (sd && sd->weapontype1 == W_REVOLVER) {
				skillratio += 400 * skill_lv;
			}
			RE_LVL_DMOD(100);
			break;
		case NW_THE_VIGILANTE_AT_NIGHT:
			if (sd && sd->weapontype1 == W_GATLING) {
				skillratio += -100 + 300 * skill_lv;
				if (sc && sc->getSCE(SC_INTENSIVE_AIM_COUNT))
					skillratio += sc->getSCE(SC_INTENSIVE_AIM_COUNT)->val1 * 100 * skill_lv;
			} else {
				skillratio += -100 + 800 + 700 * skill_lv;
				if (sc && sc->getSCE(SC_INTENSIVE_AIM_COUNT))
					skillratio += sc->getSCE(SC_INTENSIVE_AIM_COUNT)->val1 * 200 * skill_lv;
			}
			skillratio += 5 * sstatus->con;
			RE_LVL_DMOD(100);
			break;
		case NW_MISSION_BOMBARD:
			if( wd->miscflag&SKILL_ALTDMG_FLAG ){
				skillratio += -100 + 5000 + 1800 * skill_lv;
				skillratio += pc_checkskill( sd, NW_GRENADE_MASTERY ) * 100;
			}else{
				skillratio += -100 + 800 + 200 * skill_lv;
				skillratio += pc_checkskill( sd, NW_GRENADE_MASTERY ) * 30;
			}
			skillratio += 5 * sstatus->con;
			RE_LVL_DMOD(100);
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
static int64 battle_calc_skill_constant_addition(struct Damage* wd, struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv)
{
	map_session_data *sd = BL_CAST(BL_PC, src);
	map_session_data *tsd = BL_CAST(BL_PC, target);
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);
	int64 atk = 0;

	//Constant/misc additions from skills
	switch (skill_id) {
		case MO_EXTREMITYFIST:
			atk = 250 + 150 * skill_lv;
			break;
		case PA_SHIELDCHAIN:
			if (sd) {
				short index = sd->equip_index[EQI_HAND_L];
				// Bonus damage: [max(100, Random(100, 0.7*weight + pow(skill level + refine)))]
				if (index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_ARMOR) {
					// First calculate the random part of the bonus
					int bonus = (7 * sd->inventory_data[index]->weight) / 100;
					bonus += static_cast<decltype(bonus)>(pow(skill_lv + sd->inventory.u.items_inventory[index].refine, 2));
					// Now get a random value between 100 and the random part
					atk = max(100, rnd_value(100, bonus));
				}
			}
			break;
#ifndef RENEWAL
		case GS_MAGICALBULLET:
			if (sstatus->matk_max > sstatus->matk_min)
				atk = sstatus->matk_min + rnd()%(sstatus->matk_max - sstatus->matk_min);
			else
				atk = sstatus->matk_min;
			break;
#endif
#ifdef RENEWAL
		case HT_FREEZINGTRAP:
			if(sd)
				atk = 40 * pc_checkskill(sd, RA_RESEARCHTRAP);
			break;
#endif
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
static void battle_attack_sc_bonus(struct Damage* wd, struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv)
{
	map_session_data *sd = BL_CAST(BL_PC, src);
	status_change *sc = status_get_sc(src);
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);
	uint8 anger_id = 0; // SLS Anger

	// Kagerou/Oboro Earth Charm effect +15% wATK
	if(sd && sd->spiritcharm_type == CHARM_TYPE_LAND && sd->spiritcharm > 0) {
		ATK_ADDRATE(wd->damage, wd->damage2, 15 * sd->spiritcharm);
#ifdef RENEWAL
		ATK_ADDRATE(wd->weaponAtk, wd->weaponAtk2, 15 * sd->spiritcharm);
#endif
	}

	//The following are applied on top of current damage and are stackable.
	if (sc) {
#ifdef RENEWAL
		if (sc->getSCE(SC_WATK_ELEMENT) && skill_id != ASC_METEORASSAULT)
			ATK_ADDRATE(wd->weaponAtk, wd->weaponAtk2, sc->getSCE(SC_WATK_ELEMENT)->val2);
		if (sc->getSCE(SC_DRUMBATTLE))
			ATK_ADD(wd->equipAtk, wd->equipAtk2, sc->getSCE(SC_DRUMBATTLE)->val2);
		if (sc->getSCE(SC_MADNESSCANCEL))
			ATK_ADD(wd->equipAtk, wd->equipAtk2, 100);
		if (sc->getSCE(SC_MAGICALBULLET)) {
			short tmdef = tstatus->mdef + tstatus->mdef2;

			if (sstatus->matk_min > tmdef && sstatus->matk_max > sstatus->matk_min) {
				ATK_ADD(wd->weaponAtk, wd->weaponAtk2, i64max((sstatus->matk_min + rnd() % (sstatus->matk_max - sstatus->matk_min)) - tmdef, 0));
			} else {
				ATK_ADD(wd->weaponAtk, wd->weaponAtk2, i64max(sstatus->matk_min - tmdef, 0));
			}
		}
		if (sc->getSCE(SC_GATLINGFEVER))
			ATK_ADD(wd->equipAtk, wd->equipAtk2, sc->getSCE(SC_GATLINGFEVER)->val3);
#else
		//ATK percent modifier (in pre-renewal, it's applied multiplicatively after the skill ratio)
		ATK_RATE(wd->damage, wd->damage2, battle_get_atkpercent(*src, skill_id, *sc));
		ATK_RATER(wd->basedamage, battle_get_atkpercent(*src, 0, *sc));
#endif
		if (sc->getSCE(SC_SPIRIT)) {
			if (skill_id == AS_SONICBLOW && sc->getSCE(SC_SPIRIT)->val2 == SL_ASSASIN) {
				ATK_ADDRATE(wd->damage, wd->damage2, map_flag_gvg2(src->m) ? 25 : 100); //+25% dmg on woe/+100% dmg on nonwoe
				RE_ALLATK_ADDRATE(wd, map_flag_gvg2(src->m) ? 25 : 100); //+25% dmg on woe/+100% dmg on nonwoe
			} else if (skill_id == CR_SHIELDBOOMERANG && sc->getSCE(SC_SPIRIT)->val2 == SL_CRUSADER) {
				ATK_ADDRATE(wd->damage, wd->damage2, 100);
				RE_ALLATK_ADDRATE(wd, 100);
			}
		}
		if (sc->getSCE(SC_GT_CHANGE))
			ATK_ADDRATE(wd->damage, wd->damage2, sc->getSCE(SC_GT_CHANGE)->val1);
#ifdef RENEWAL
		if (sc->getSCE(SC_EDP)) {
			switch(skill_id) {
				// Renewal: Venom Splasher, Meteor Assault, Grimtooth and Venom Knife ignore EDP
				case TF_SPRINKLESAND:
				case AS_SPLASHER:
				case ASC_METEORASSAULT:
				case AS_GRIMTOOTH:
				case AS_VENOMKNIFE:
					break; // skills above have no effect with EDP

				default: // fall through to apply EDP bonuses
					// Renewal EDP formula [helvetica]
					// weapon atk * (2.5 + (edp level * .3))
					// equip atk * (2.5 + (edp level * .3))
					ATK_RATE(wd->weaponAtk, wd->weaponAtk2, 250 + (sc->getSCE(SC_EDP)->val1 * 30));
					ATK_RATE(wd->equipAtk, wd->equipAtk2, 250 + (sc->getSCE(SC_EDP)->val1 * 30));
					break;
			}
		}
#endif
		if (sc->getSCE(SC_DANCEWITHWUG)) {
			if (skill_get_inf2(skill_id, INF2_INCREASEDANCEWITHWUGDAMAGE)) {
				ATK_ADDRATE(wd->damage, wd->damage2, sc->getSCE(SC_DANCEWITHWUG)->val1 * 10 * battle_calc_chorusbonus(sd));
				RE_ALLATK_ADDRATE(wd, sc->getSCE(SC_DANCEWITHWUG)->val1 * 10 * battle_calc_chorusbonus(sd));
			}
			ATK_ADDRATE(wd->damage, wd->damage2, sc->getSCE(SC_DANCEWITHWUG)->val1 * 2 * battle_calc_chorusbonus(sd));
#ifdef RENEWAL
			ATK_ADDRATE(wd->equipAtk, wd->equipAtk2, sc->getSCE(SC_DANCEWITHWUG)->val1 * 2 * battle_calc_chorusbonus(sd));
#endif
		}
		if(sc->getSCE(SC_ZENKAI) && sstatus->rhw.ele == sc->getSCE(SC_ZENKAI)->val2) {
			ATK_ADD(wd->damage, wd->damage2, 200);
#ifdef RENEWAL
			ATK_ADD(wd->equipAtk, wd->equipAtk2, 200);
#endif
		}
		if (sc->getSCE(SC_EQC)) {
			ATK_ADDRATE(wd->damage, wd->damage2, -sc->getSCE(SC_EQC)->val2);
#ifdef RENEWAL
			ATK_ADDRATE(wd->equipAtk, wd->equipAtk2, -sc->getSCE(SC_EQC)->val2);
#endif
		}
		if(sc->getSCE(SC_STYLE_CHANGE)) {
			TBL_HOM *hd = BL_CAST(BL_HOM,src);

			if(hd) {
				ATK_ADD(wd->damage, wd->damage2, hd->homunculus.spiritball * 3);
				RE_ALLATK_ADD(wd, hd->homunculus.spiritball * 3);
			}
		}
		if(sc->getSCE(SC_UNLIMIT) && (wd->flag&(BF_LONG|BF_MAGIC)) == BF_LONG) {
			switch(skill_id) {
				case RA_WUGDASH:
				case RA_WUGSTRIKE:
				case RA_WUGBITE:
					break;
				default:
					ATK_ADDRATE(wd->damage, wd->damage2, sc->getSCE(SC_UNLIMIT)->val2);
					RE_ALLATK_ADDRATE(wd, sc->getSCE(SC_UNLIMIT)->val2);
					break;
			}
		}
		if (sc->getSCE(SC_HEAT_BARREL)) {
			ATK_ADDRATE(wd->damage, wd->damage2, sc->getSCE(SC_HEAT_BARREL)->val3);
			RE_ALLATK_ADDRATE(wd, sc->getSCE(SC_HEAT_BARREL)->val3);
		}
		if((wd->flag&(BF_LONG|BF_MAGIC)) == BF_LONG) {
			if (sc->getSCE(SC_MTF_RANGEATK)) { // Monster Transformation bonus
				ATK_ADDRATE(wd->damage, wd->damage2, sc->getSCE(SC_MTF_RANGEATK)->val1);
				RE_ALLATK_ADDRATE(wd, sc->getSCE(SC_MTF_RANGEATK)->val1);
			}
			if (sc->getSCE(SC_MTF_RANGEATK2)) { // Monster Transformation bonus
				ATK_ADDRATE(wd->damage, wd->damage2, sc->getSCE(SC_MTF_RANGEATK2)->val1);
				RE_ALLATK_ADDRATE(wd, sc->getSCE(SC_MTF_RANGEATK2)->val1);
			}
			if (sc->getSCE(SC_ARCLOUSEDASH) && sc->getSCE(SC_ARCLOUSEDASH)->val4) {
				ATK_ADDRATE(wd->damage, wd->damage2, sc->getSCE(SC_ARCLOUSEDASH)->val4);
				RE_ALLATK_ADDRATE(wd, sc->getSCE(SC_ARCLOUSEDASH)->val4);
			}
		}

		if (sd && wd->flag&BF_WEAPON && sc->getSCE(SC_GVG_GIANT) && sc->getSCE(SC_GVG_GIANT)->val3) {
			ATK_ADDRATE(wd->damage, wd->damage2, sc->getSCE(SC_GVG_GIANT)->val3);
			RE_ALLATK_ADDRATE(wd, sc->getSCE(SC_GVG_GIANT)->val3);
		}

		if (skill_id == 0 && sc->getSCE(SC_EXEEDBREAK)) {
			ATK_ADDRATE(wd->damage, wd->damage2, sc->getSCE(SC_EXEEDBREAK)->val2);
			RE_ALLATK_ADDRATE(wd, sc->getSCE(SC_EXEEDBREAK)->val2);
		}
		if (sc->getSCE(SC_PYREXIA) && sc->getSCE(SC_PYREXIA)->val3 == 0 && skill_id == 0) {
			ATK_ADDRATE(wd->damage, wd->damage2, sc->getSCE(SC_PYREXIA)->val2);
			RE_ALLATK_ADDRATE(wd, sc->getSCE(SC_PYREXIA)->val2);
		}

		if (sc->getSCE(SC_MIRACLE))
			anger_id = 2; // Always treat all monsters as star flagged monster when in miracle state
		if (sc->getSCE(SC_HIDDEN_CARD) && (wd->flag&BF_LONG) == BF_LONG)
			RE_ALLATK_ADDRATE(wd, sc->getSCE(SC_HIDDEN_CARD)->val3);
	}

	if ((wd->flag&(BF_LONG|BF_MAGIC)) == BF_LONG) {
		if (sd && pc_checkskill(sd, SU_POWEROFLIFE) > 0 && pc_checkskill_summoner(sd, SUMMONER_POWER_LIFE) >= 20) {
			ATK_ADDRATE(wd->damage, wd->damage2, 20);
			RE_ALLATK_ADDRATE(wd, 20);
		}
	}

	if (sd != nullptr && !anger_id)
		ARR_FIND(0, MAX_PC_FEELHATE, anger_id, status_get_class(target) == sd->hate_mob[anger_id]);

	uint16 anger_level;
	if (sd != nullptr && anger_id < MAX_PC_FEELHATE && (anger_level = pc_checkskill(sd, sg_info[anger_id].anger_id))) {
		int skillratio = sd->status.base_level + sstatus->dex + sstatus->luk;

		if (anger_id == 2)
			skillratio += sstatus->str; // SG_STAR_ANGER additionally has STR added in its formula.
		if (anger_level < 4)
			skillratio /= 12 - 3 * anger_level;
		ATK_ADDRATE(wd->damage, wd->damage2, skillratio);
#ifdef RENEWAL
		RE_ALLATK_ADDRATE(wd, skillratio);
#endif
	}
}

/*====================================
 * Calc defense damage reduction
 *------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static void battle_calc_defense_reduction(struct Damage* wd, struct block_list *src,struct block_list *target, uint16 skill_id, uint16 skill_lv)
{
	map_session_data *sd = BL_CAST(BL_PC, src);
	map_session_data *tsd = BL_CAST(BL_PC, target);
	status_change *sc = status_get_sc(src);
	status_change *tsc = status_get_sc(target);
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);

	//Defense reduction
	short vit_def;
	defType def1 = status_get_def(target); //Don't use tstatus->def1 due to skill timer reductions.
	short def2 = tstatus->def2;

	if (sd) {
		int i = sd->indexed_bonus.ignore_def_by_race[tstatus->race] + sd->indexed_bonus.ignore_def_by_race[RC_ALL];
		i += sd->indexed_bonus.ignore_def_by_class[tstatus->class_] + sd->indexed_bonus.ignore_def_by_class[CLASS_ALL];
		if (i) {
			i = min(i,100); //cap it to 100 for 0 def min
			def1 -= def1 * i / 100;
			def2 -= def2 * i / 100;
		}

		//Kagerou/Oboro Earth Charm effect +10% eDEF
		if(sd->spiritcharm_type == CHARM_TYPE_LAND && sd->spiritcharm > 0) {
			short si = 10 * sd->spiritcharm;
			def1 = (def1 * (100 + si)) / 100;
		}
	}

	if (sc && sc->getSCE(SC_EXPIATIO)) {
		short i = 5 * sc->getSCE(SC_EXPIATIO)->val1; // 5% per level

		i = min(i,100); //cap it to 100 for 0 def min
		def1 = (def1*(100-i))/100;
		def2 = (def2*(100-i))/100;
	}

	if (tsc) {
		if (tsc->getSCE(SC_FORCEOFVANGUARD)) {
			short i = 2 * tsc->getSCE(SC_FORCEOFVANGUARD)->val1;

			def1 = (def1 * (100 + i)) / 100;
		}

		if( tsc->getSCE(SC_CAMOUFLAGE) ){
			short i = 5 * tsc->getSCE(SC_CAMOUFLAGE)->val3; //5% per second

			i = min(i,100); //cap it to 100 for 0 def min
			def1 = (def1*(100-i))/100;
			def2 = (def2*(100-i))/100;
		}

		if (tsc->getSCE(SC_GT_REVITALIZE))
			def1 += tsc->getSCE(SC_GT_REVITALIZE)->val4;

		if (tsc->getSCE(SC_OVERED_BOOST) && target->type == BL_PC)
			def1 = (def1 * tsc->getSCE(SC_OVERED_BOOST)->val4) / 100;
	}

	if( battle_config.vit_penalty_type && battle_config.vit_penalty_target&target->type ) {
		unsigned char target_count; //256 max targets should be a sane max

		//Official servers limit the count to 22 targets
		target_count = min(unit_counttargeted(target), (100 / battle_config.vit_penalty_num) + (battle_config.vit_penalty_count - 1));
		if(target_count >= battle_config.vit_penalty_count) {
			if(battle_config.vit_penalty_type == 1) {
				if( !tsc || !tsc->getSCE(SC_STEELBODY) )
					def1 = (def1 * (100 - (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num))/100;
				def2 = (def2 * (100 - (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num))/100;
			} else { //Assume type 2
				if( !tsc || !tsc->getSCE(SC_STEELBODY) )
					def1 -= (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num;
				def2 -= (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num;
			}
		}
		if(def2 < 1)
			def2 = 1;
	}

#ifdef RENEWAL
	if (skill_id == AM_ACIDTERROR)
		def2 = 0; // Ignore only status defense.
#endif

	//Damage reduction based on vitality
	if (tsd) {	//Sd vit-eq
		int skill;
#ifndef RENEWAL
		//Damage reduction: [VIT*0.3] + RND(0, [VIT^2/150] - [VIT*0.3] - 1) + [VIT*0.5]
		vit_def = ((3 * def2) / 10);
		vit_def += rnd_value(0, (def2 * def2) / 150 - ((3 * def2) / 10) - 1);
		vit_def += (def2 / 2);
#else
		vit_def = def2;
#endif
		if (src->type == BL_MOB && (battle_check_undead(sstatus->race, sstatus->def_ele) || sstatus->race == RC_DEMON) && //This bonus already doesn't work vs players
			(skill = pc_checkskill(tsd, AL_DP)) > 0)
			vit_def += (int)(((float)tsd->status.base_level / 25.0 + 3.0) * skill + 0.5);
		if( src->type == BL_MOB && (skill=pc_checkskill(tsd,RA_RANGERMAIN))>0 &&
			(sstatus->race == RC_BRUTE || sstatus->race == RC_PLAYER_DORAM || sstatus->race == RC_FISH || sstatus->race == RC_PLANT) )
			vit_def += skill*5;
		if( src->type == BL_MOB && (skill = pc_checkskill(tsd, NC_RESEARCHFE)) > 0 &&
			(sstatus->def_ele == ELE_FIRE || sstatus->def_ele == ELE_EARTH) )
			vit_def += skill * 10;
	} else { //Mob-Pet vit-eq
#ifndef RENEWAL
		//VIT + rnd(0,[VIT/20]^2-1)
		vit_def = (def2/20)*(def2/20);
		if (tsc && tsc->getSCE(SC_SKA))
			vit_def += 100; //Eska increases the random part of the formula by 100
		vit_def = def2 + (vit_def>0?rnd()%vit_def:0);
#else
		//SoftDEF of monsters is floor((BaseLevel+Vit)/2)
		vit_def = def2;
#endif
	}

	if (battle_config.weapon_defense_type) {
		vit_def += def1*battle_config.weapon_defense_type;
		def1 = 0;
	}

#ifdef RENEWAL
	std::bitset<NK_MAX> nk = battle_skill_get_damage_properties(skill_id, wd->miscflag);

	if (nk[NK_SIMPLEDEFENSE]) {
		// Defense reduction by flat value.
		// This completely bypasses the normal RE DEF Reduction formula.
		wd->damage -= (def1 + vit_def);
		if (is_attack_left_handed(src, skill_id))
			wd->damage2 -= (def1 + vit_def);
	}
	else {
		/**
		 * RE DEF Reduction
		 * Damage = Attack * (4000+eDEF)/(4000+eDEF*10) - sDEF
		 * Pierce defence gains 1 atk per def/2
		 */
		if (def1 == -400) /* -400 creates a division by 0 and subsequently crashes */
			def1 = -399;
		ATK_ADD2(wd->damage, wd->damage2,
			is_attack_piercing(wd, src, target, skill_id, skill_lv, EQI_HAND_R) ? (def1 * battle_calc_attack_skill_ratio(wd, src, target, skill_id, skill_lv)) / 200 : 0,
			is_attack_piercing(wd, src, target, skill_id, skill_lv, EQI_HAND_L) ? (def1 * battle_calc_attack_skill_ratio(wd, src, target, skill_id, skill_lv)) / 200 : 0
		);
		if (!attack_ignores_def(wd, src, target, skill_id, skill_lv, EQI_HAND_R) && !is_attack_piercing(wd, src, target, skill_id, skill_lv, EQI_HAND_R))
			wd->damage = wd->damage * (4000 + def1) / (4000 + 10 * def1) - vit_def;
		if (is_attack_left_handed(src, skill_id) && !attack_ignores_def(wd, src, target, skill_id, skill_lv, EQI_HAND_L) && !is_attack_piercing(wd, src, target, skill_id, skill_lv, EQI_HAND_L))
			wd->damage2 = wd->damage2 * (4000 + def1) / (4000 + 10 * def1) - vit_def;
	}
#else
		if (def1 > 100) def1 = 100;
		ATK_RATE2(wd->damage, wd->damage2,
			attack_ignores_def(wd, src, target, skill_id, skill_lv, EQI_HAND_R) ?100:(is_attack_piercing(wd, src, target, skill_id, skill_lv, EQI_HAND_R) ? (int64)is_attack_piercing(wd, src, target, skill_id, skill_lv, EQI_HAND_R)*(def1+vit_def) : (100-def1)),
			attack_ignores_def(wd, src, target, skill_id, skill_lv, EQI_HAND_L) ?100:(is_attack_piercing(wd, src, target, skill_id, skill_lv, EQI_HAND_L) ? (int64)is_attack_piercing(wd, src, target, skill_id, skill_lv, EQI_HAND_L)*(def1+vit_def) : (100-def1))
		);
		ATK_RATER(wd->basedamage, 100 - def1);
		ATK_ADD2(wd->damage, wd->damage2,
			attack_ignores_def(wd, src, target, skill_id, skill_lv, EQI_HAND_R) || is_attack_piercing(wd, src, target, skill_id, skill_lv, EQI_HAND_R) ?0:-vit_def,
			attack_ignores_def(wd, src, target, skill_id, skill_lv, EQI_HAND_L) || is_attack_piercing(wd, src, target, skill_id, skill_lv, EQI_HAND_L) ?0:-vit_def
		);
		wd->basedamage -= vit_def;
#endif
}

/*====================================
 * Modifiers ignoring DEF
 *------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static void battle_calc_attack_post_defense(struct Damage* wd, struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv)
{
	map_session_data *sd = BL_CAST(BL_PC, src);
	status_change *sc = status_get_sc(src);
	struct status_data *sstatus = status_get_status_data(src);

	// Post skill/vit reduction damage increases
#ifndef RENEWAL
	//Refine bonus
	if (sd) {
		if (battle_skill_stacks_masteries_vvs(skill_id, BCHK_REFINE)) {
			ATK_ADD2(wd->damage, wd->damage2, sstatus->rhw.atk2, sstatus->lhw.atk2);
		}
		wd->basedamage += sstatus->rhw.atk2;
	}

	//After DEF reduction, damage can be negative, refine bonus works against that value
	//After refinement bonus was applied, damage is capped to 1, then masteries are applied
	battle_min_damage(*wd, *src, skill_id, 1);

	battle_calc_attack_masteries(wd, src, target, skill_id, skill_lv);
#endif
	if (sc) { // SC skill damages
		if (sc->getSCE(SC_AURABLADE)
#ifndef RENEWAL
			&& skill_id != LK_SPIRALPIERCE && skill_id != ML_SPIRALPIERCE
#endif
			) {
#ifdef RENEWAL
			ATK_ADD(wd->damage, wd->damage2, (3 + sc->getSCE(SC_AURABLADE)->val1) * status_get_lv(src)); // !TODO: Confirm formula
#else
			ATK_ADD(wd->damage, wd->damage2, 20 * sc->getSCE(SC_AURABLADE)->val1);
#endif
		}
	}

	//Set to min of 1
	battle_min_damage(*wd, *src, skill_id, 1);

#ifdef RENEWAL
	switch (skill_id) {
		case AS_SONICBLOW:
			if(sd && pc_checkskill(sd,AS_SONICACCEL)>0)
				ATK_ADDRATE(wd->damage, wd->damage2, 90);
			break;
	}
#endif
}

/*=================================================================================
 * "Plant"-type (mobs that only take 1 damage from all sources) damage calculation
 *---------------------------------------------------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static void battle_calc_attack_plant(struct Damage* wd, struct block_list *src,struct block_list *target, uint16 skill_id, uint16 skill_lv)
{
	struct status_data *tstatus = status_get_status_data(target);
	bool attack_hits = is_attack_hitting(wd, src, target, skill_id, skill_lv, false);

	if (skill_id != SN_SHARPSHOOTING && skill_id != RA_ARROWSTORM)
		status_change_end(src, SC_CAMOUFLAGE);

	//Plants receive 1 damage when hit
	if( attack_hits || wd->damage > 0 )
		wd->damage = 1; //In some cases, right hand no need to have a weapon to deal a damage
	if( is_attack_left_handed(src, skill_id) && (attack_hits || wd->damage2 > 0) ) {
		map_session_data *sd = BL_CAST(BL_PC, src);

		if (sd && sd->status.weapon == W_KATAR)
			wd->damage2 = 0; //No backhand damage against plants
		else
			wd->damage2 = 1; //Deal 1 HP damage as long as there is a weapon in the left hand
	}

	if (attack_hits && target->type == BL_MOB) {
		status_change *sc = status_get_sc(target);
		int64 damage_dummy = 1;

		if (sc && !battle_status_block_damage(src, target, sc, wd, damage_dummy, skill_id, skill_lv)) { // Statuses that reduce damage to 0.
			wd->damage = wd->damage2 = 0;
			return;
		}
	}

	if( attack_hits && status_get_class(target) == MOBID_EMPERIUM ) {
		if(target && !battle_can_hit_gvg_target(src,target,skill_id,(skill_id)?BF_SKILL:0) && map_flag_gvg2(target->m)) {
			wd->damage = wd->damage2 = 0;
			return;
		}

		const int right_element = battle_get_weapon_element(wd, src, target, skill_id, skill_lv, EQI_HAND_R, false);
		const int left_element = battle_get_weapon_element(wd, src, target, skill_id, skill_lv, EQI_HAND_L, false);

		if (wd->damage > 0) {
			wd->damage = battle_attr_fix(src, target, wd->damage, right_element, tstatus->def_ele, tstatus->ele_lv);
			wd->damage = battle_calc_gvg_damage(src, target, wd->damage, skill_id, wd->flag);
		} else if (wd->damage2 > 0) {
			wd->damage2 = battle_attr_fix(src, target, wd->damage2, left_element, tstatus->def_ele, tstatus->ele_lv);
			wd->damage2 = battle_calc_gvg_damage(src, target, wd->damage2, skill_id, wd->flag);
		}
		return;
	}

	// Triple Attack has a special property that it does not split damage on plant mode
	// In pre-renewal, it requires the monster to have exactly 100 def
	if (skill_id == MO_TRIPLEATTACK && wd->div_ < 0
#ifndef RENEWAL
		&& tstatus->def == 100
#endif
		)
		wd->div_ *= -1;

	//For plants we don't continue with the weapon attack code, so we have to apply DAMAGE_DIV_FIX here
	battle_apply_div_fix(wd, skill_id);

	//If there is left hand damage, total damage can never exceed 2, even on multiple hits
	if(wd->damage > 1 && wd->damage2 > 0) {
		wd->damage = 1;
		wd->damage2 = 1;
	}
}

/*========================================================================================
 * Perform left/right hand weapon damage calculation based on previously calculated damage
 *----------------------------------------------------------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static void battle_calc_attack_left_right_hands(struct Damage* wd, struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv)
{
	map_session_data *sd = BL_CAST(BL_PC, src);

	if (sd) {
		int skill;

		if(sd->status.weapon == W_KATAR && !skill_id) { //Katars (offhand damage only applies to normal attacks, tested on Aegis 10.2)
			skill = pc_checkskill(sd,TF_DOUBLE);
			wd->damage2 = (int64)wd->damage * (1 + (skill * 2))/100;
#ifdef RENEWAL
		} else if(is_attack_left_handed(src, skill_id) && sd->status.weapon != W_KATAR) {	//Dual-wield
#else
		} else if(is_attack_left_handed(src, skill_id)) {	//Dual-wield
#endif
			// If you only have a weapon in the left hand, then your main hand damage will be identical to an unarmed attack
			if (is_attack_right_handed(src, skill_id) && wd->damage) {
				if( (sd->class_&MAPID_BASEMASK) == MAPID_THIEF ) {
					skill = pc_checkskill(sd,AS_RIGHT);
					ATK_RATER(wd->damage, 50 + (skill * 10))
				}
				else if(sd->class_ == MAPID_KAGEROUOBORO) {
					skill = pc_checkskill(sd,KO_RIGHT);
					ATK_RATER(wd->damage, 70 + (skill * 10))
				}
				if(wd->damage < 1)
					wd->damage = 1;
			}
			// Left hand damage will always be adjusted, even if you don't have a weapon in the main hand
			if (wd->damage2) {
				if( (sd->class_&MAPID_BASEMASK) == MAPID_THIEF) {
					skill = pc_checkskill(sd,AS_LEFT);
					ATK_RATEL(wd->damage2, 30 + (skill * 10))
				}
				else if(sd->class_ == MAPID_KAGEROUOBORO) {
					skill = pc_checkskill(sd,KO_LEFT);
					ATK_RATEL(wd->damage2, 50 + (skill * 10))
				}
				if(wd->damage2 < 1)
					wd->damage2 = 1;
			}
		}
	}

	if(!is_attack_right_handed(src, skill_id) && !is_attack_left_handed(src, skill_id) && wd->damage)
		wd->damage=0;

	if(!is_attack_left_handed(src, skill_id) && wd->damage2)
		wd->damage2=0;
}

/**
* Check if bl is devoted by someone
* @param bl
* @return 'd_bl' if devoted or nullptr if not devoted
*/
struct block_list *battle_check_devotion(struct block_list *bl) {
	struct block_list *d_bl = nullptr;

	if (battle_config.devotion_rdamage && battle_config.devotion_rdamage > rnd() % 100) {
		status_change *sc = status_get_sc(bl);
		if (sc && sc->getSCE(SC_DEVOTION))
			d_bl = map_id2bl(sc->getSCE(SC_DEVOTION)->val1);
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
static void battle_calc_attack_gvg_bg(struct Damage* wd, struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv)
{
	if( wd->damage + wd->damage2 ) { //There is a total damage value
		if( src != target && //Don't reflect your own damage (Grand Cross)
			(!skill_id || skill_id ||
			(src->type == BL_SKILL && (skill_id == SG_SUN_WARM || skill_id == SG_MOON_WARM || skill_id == SG_STAR_WARM))) ) {
				int64 damage = wd->damage + wd->damage2, rdamage = 0;
				map_session_data *tsd = BL_CAST(BL_PC, target);
				struct status_data *sstatus = status_get_status_data(src);
				t_tick tick = gettick(), rdelay = 0;

				rdamage = battle_calc_return_damage(target, src, &damage, wd->flag, skill_id, false);
				if( rdamage > 0 ) { //Item reflect gets calculated before any mapflag reducing is applicated
					struct block_list *d_bl = battle_check_devotion(src);

					rdelay = clif_damage(src, (!d_bl) ? src : d_bl, tick, wd->amotion, sstatus->dmotion, rdamage, 1, DMG_ENDURE, 0, false);
					if( tsd )
						battle_drain(tsd, src, rdamage, rdamage, sstatus->race, sstatus->class_);
					//Use Reflect Shield to signal this kind of skill trigger [Skotlex]
					battle_delay_damage(tick, wd->amotion, target, (!d_bl) ? src : d_bl, 0, CR_REFLECTSHIELD, 0, rdamage, ATK_DEF, rdelay, true, false);
					skill_additional_effect(target, (!d_bl) ? src : d_bl, CR_REFLECTSHIELD, 1, BF_WEAPON|BF_SHORT|BF_NORMAL, ATK_DEF, tick);
				}
		}

		struct map_data *mapdata = map_getmapdata(target->m);

		if(!wd->damage2) {
			wd->damage = battle_calc_damage(src,target,wd,wd->damage,skill_id,skill_lv);
			if( mapdata_flag_gvg2(mapdata) )
				wd->damage=battle_calc_gvg_damage(src,target,wd->damage,skill_id,wd->flag);
			else if( mapdata->getMapFlag(MF_BATTLEGROUND) )
				wd->damage=battle_calc_bg_damage(src,target,wd->damage,skill_id,wd->flag);
		}
		else if(!wd->damage) {
			wd->damage2 = battle_calc_damage(src,target,wd,wd->damage2,skill_id,skill_lv);
			if( mapdata_flag_gvg2(mapdata) )
				wd->damage2 = battle_calc_gvg_damage(src,target,wd->damage2,skill_id,wd->flag);
			else if( mapdata->getMapFlag(MF_BATTLEGROUND) )
				wd->damage2 = battle_calc_bg_damage(src,target,wd->damage2,skill_id,wd->flag);
		}
		else {
			wd->damage = battle_calc_damage(src, target, wd, wd->damage, skill_id, skill_lv);
			wd->damage2 = battle_calc_damage(src, target, wd, wd->damage2, skill_id, skill_lv);
			if (mapdata_flag_gvg2(mapdata)) {
				wd->damage = battle_calc_gvg_damage(src, target, wd->damage, skill_id, wd->flag);
				wd->damage2 = battle_calc_gvg_damage(src, target, wd->damage2, skill_id, wd->flag);
			}
			else if (mapdata->getMapFlag(MF_BATTLEGROUND)) {
				wd->damage = battle_calc_bg_damage(src, target, wd->damage, skill_id, wd->flag);
				wd->damage2 = battle_calc_bg_damage(src, target, wd->damage2, skill_id, wd->flag);
			}
			if(wd->damage > 1 && wd->damage2 < 1) wd->damage2 = 1;
		}
	}
}

/*==========================================
 * final ATK modifiers - after BG/GvG calc
 *------------------------------------------
 * Credits:
 *	Original coder Skotlex
 *	Initial refactoring by Baalberith
 *	Refined and optimized by helvetica
 */
static void battle_calc_weapon_final_atk_modifiers(struct Damage* wd, struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv)
{
	map_session_data *sd = BL_CAST(BL_PC, src);
	map_session_data *tsd = BL_CAST(BL_PC, target);
	status_change *sc = status_get_sc(src);
	status_change *tsc = status_get_sc(target);
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);
	int skill_damage = 0;

	//Reject Sword bugreport:4493 by Daegaladh
	if(wd->damage && tsc && tsc->getSCE(SC_REJECTSWORD) &&
		(src->type!=BL_PC || (
			((TBL_PC *)src)->weapontype1 == W_DAGGER ||
			((TBL_PC *)src)->weapontype1 == W_1HSWORD ||
			((TBL_PC *)src)->status.weapon == W_2HSWORD
		)) &&
		rnd()%100 < tsc->getSCE(SC_REJECTSWORD)->val2
		)
	{
		ATK_RATER(wd->damage, 50)
		clif_skill_nodamage(target,target,ST_REJECTSWORD, tsc->getSCE(SC_REJECTSWORD)->val1,1);
		battle_fix_damage(target,src,wd->damage,clif_damage(target,src,gettick(),0,0,wd->damage,0,DMG_NORMAL,0,false),ST_REJECTSWORD);
		if (status_isdead(target))
			return;
		if( --(tsc->getSCE(SC_REJECTSWORD)->val3) <= 0 )
			status_change_end(target, SC_REJECTSWORD);
	}

	if( tsc && tsc->getSCE(SC_CRESCENTELBOW) && wd->flag&BF_SHORT && rnd()%100 < tsc->getSCE(SC_CRESCENTELBOW)->val2 ) {
		//ATK [{(Target HP / 100) x Skill Level} x Caster Base Level / 125] % + [Received damage x {1 + (Skill Level x 0.2)}]
		int64 rdamage = 0;
		int ratio = (int64)(status_get_hp(src) / 100) * tsc->getSCE(SC_CRESCENTELBOW)->val1 * status_get_lv(target) / 125;
		if (ratio > 5000) ratio = 5000; // Maximum of 5000% ATK
		rdamage = battle_calc_base_damage(target,tstatus,&tstatus->rhw,tsc,sstatus->size,0);
		rdamage = (int64)rdamage * ratio / 100 + wd->damage * (10 + tsc->getSCE(SC_CRESCENTELBOW)->val1 * 20 / 10) / 10;
		skill_blown(target, src, skill_get_blewcount(SR_CRESCENTELBOW_AUTOSPELL, tsc->getSCE(SC_CRESCENTELBOW)->val1), unit_getdir(src), BLOWN_NONE);
		clif_skill_damage(target, src, gettick(), status_get_amotion(src), 0, rdamage,
			1, SR_CRESCENTELBOW_AUTOSPELL, tsc->getSCE(SC_CRESCENTELBOW)->val1, DMG_SINGLE); // This is how official does
		clif_damage(src, target, gettick(), status_get_amotion(src)+1000, 0, rdamage/10, 1, DMG_NORMAL, 0, false);
		battle_fix_damage(target, src, rdamage, 0, SR_CRESCENTELBOW);
		status_damage(src, target, rdamage/10, 0, 0, 1, 0);
		status_change_end(target, SC_CRESCENTELBOW);
	}

	if( sc ) {
		//SC_FUSION hp penalty [Komurka]
		if (sc->getSCE(SC_FUSION)) {
			unsigned int hp = sstatus->max_hp;

			if (sd && tsd) {
				hp = hp / 13;
				if (((int64)sstatus->hp * 100) <= ((int64)sstatus->max_hp * 20))
					hp = sstatus->hp;
			} else
				hp = 2*hp/100; //2% hp loss per hit
			status_zap(src, hp, 0);
		}
		if (sc->getSCE(SC_VIGOR))
			status_zap(src, sc->getSCE(SC_VIGOR)->val2, 0);
		// Only affecting non-skills
		if (!skill_id && wd->dmg_lv > ATK_BLOCK) {
			if (sc->getSCE(SC_ENCHANTBLADE)) {
				//[((Skill Lv x 20) + 100) x (casterBaseLevel / 150)] + casterInt + MATK - MDEF - MDEF2
				int64 enchant_dmg = sc->getSCE(SC_ENCHANTBLADE)->val2;
				if (sstatus->matk_max > sstatus->matk_min)
					enchant_dmg = enchant_dmg + sstatus->matk_min + rnd() % (sstatus->matk_max - sstatus->matk_min);
				else
					enchant_dmg = enchant_dmg + sstatus->matk_min;
				enchant_dmg = enchant_dmg - (tstatus->mdef + tstatus->mdef2);
				if (enchant_dmg > 0)
					ATK_ADD(wd->damage, wd->damage2, enchant_dmg);
			}
		}
		if (skill_id != SN_SHARPSHOOTING && skill_id != RA_ARROWSTORM)
			status_change_end(src, SC_CAMOUFLAGE);
	}

#ifndef RENEWAL
	if (skill_id == ASC_BREAKER) { //Breaker's int-based damage (a misc attack?)
		struct Damage md = battle_calc_misc_attack(src, target, skill_id, skill_lv, wd->miscflag);

		wd->damage += md.damage;
	}
#endif

	// Skill damage adjustment
	if ((skill_damage = battle_skill_damage(src, target, skill_id)) != 0)
		ATK_ADDRATE(wd->damage, wd->damage2, skill_damage);
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
	status_change *sc = status_get_sc(src);
	map_session_data *sd = BL_CAST(BL_PC, src);
	struct Damage wd;

	wd.type = DMG_NORMAL; //Normal attack
	wd.div_ = skill_id?skill_get_num(skill_id,skill_lv):1;
	wd.amotion = (skill_id && skill_get_inf(skill_id)&INF_GROUND_SKILL)?0:sstatus->amotion; //Amotion should be 0 for ground skills.
	// counter attack DOES obey ASPD delay on official, uncomment if you want the old (bad) behavior [helvetica]
	/*if(skill_id == KN_AUTOCOUNTER)
		wd.amotion /= 2; */
	wd.dmotion = tstatus->dmotion;
	wd.blewcount =skill_get_blewcount(skill_id,skill_lv);
	wd.miscflag = wflag;
	wd.flag = BF_WEAPON; //Initial Flag
	wd.flag |= (skill_id||wd.miscflag)?BF_SKILL:BF_NORMAL; // Baphomet card's splash damage is counted as a skill. [Inkfish]
	wd.isspdamage = false;
	wd.damage = wd.damage2 =
#ifdef RENEWAL
	wd.statusAtk = wd.statusAtk2 = wd.equipAtk = wd.equipAtk2 = wd.weaponAtk = wd.weaponAtk2 = wd.masteryAtk = wd.masteryAtk2 =
	wd.percentAtk = wd.percentAtk2 =
#else
	wd.basedamage =
#endif
	0;

	wd.dmg_lv=ATK_DEF;	//This assumption simplifies the assignation later

	if(sd)
		wd.blewcount += battle_blewcount_bonus(sd, skill_id);

	if (skill_id) {
		wd.flag |= battle_range_type(src, target, skill_id, skill_lv);
		switch(skill_id)
		{
#ifdef RENEWAL
			case RG_BACKSTAP:
				if (sd && sd->status.weapon == W_DAGGER)
					wd.div_ = 2;
				break;
			case MO_CHAINCOMBO:
				if (sd && sd->status.weapon == W_KNUCKLE)
					wd.div_ = -6;
				break;
#endif
			case MH_SONIC_CRAW:{
				TBL_HOM *hd = BL_CAST(BL_HOM,src);
				wd.div_ = hd->homunculus.spiritball;
			}
				break;
			case MO_FINGEROFFENSIVE:
				if (sd) {
					if (battle_config.finger_offensive_type)
						wd.div_ = 1;
#ifndef RENEWAL
					else if ((sd->spiritball + sd->spiritball_old) < wd.div_)
						wd.div_ = sd->spiritball + sd->spiritball_old;
#endif
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
				wd.amotion = sstatus->amotion;
				[[fallthrough]];
			case KN_SPEARSTAB:
#ifndef RENEWAL
			case KN_BOWLINGBASH:
#endif
			case MS_BOWLINGBASH:
			case MO_BALKYOUNG:
			case TK_TURNKICK:
				wd.blewcount = 0;
				break;
#ifdef RENEWAL
			case KN_BOWLINGBASH:
				if (sd && sd->status.weapon == W_2HSWORD) {
					if (wd.miscflag >= 2 && wd.miscflag <= 3)
						wd.div_ = 3;
					else if (wd.miscflag >= 4)
						wd.div_ = 4;
				}
				break;
#endif
			case KN_AUTOCOUNTER:
				wd.flag = (wd.flag&~BF_SKILLMASK)|BF_NORMAL;
				break;
			case LK_SPIRALPIERCE:
				if (!sd) wd.flag = (wd.flag&~(BF_RANGEMASK|BF_WEAPONMASK))|BF_LONG|BF_MISC;
				break;
			case RK_WINDCUTTER:
				if (sd && (sd->status.weapon == W_1HSPEAR || sd->status.weapon == W_2HSPEAR))
					wd.flag |= BF_LONG;
				break;
			case NC_BOOSTKNUCKLE:
			case NC_VULCANARM:
			case NC_ARMSCANNON:
				if (sc && sc->getSCE(SC_ABR_DUAL_CANNON))
					wd.div_ = 2;
				break;
			case NC_POWERSWING:
				if (sc && sc->getSCE(SC_ABR_BATTLE_WARIOR))
					wd.div_ = -2;
				break;
			case GN_CARTCANNON:
				if (sc && sc->getSCE(SC_BIONIC_WOODENWARRIOR))
					wd.div_ = 2;
				break;
			case DK_SERVANT_W_PHANTOM:
			case DK_SERVANT_W_DEMOL:
				if (sd && (sd->servantball + sd->servantball_old) < wd.div_)
					wd.div_ = sd->servantball + sd->servantball_old;
				break;
			case IQ_THIRD_FLAME_BOMB:
				wd.div_ = min(wd.div_ + wd.miscflag, 3); // Number of hits doesn't go above 3.
				break;
			case IG_OVERSLASH:
				if( wd.miscflag >= 4 ){
					wd.div_ = 7;
				}else if( wd.miscflag >= 2 ){
					wd.div_ = 5;
				}
				break;
			case SHC_ETERNAL_SLASH:
				if (sc && sc->getSCE(SC_E_SLASH_COUNT))
					wd.div_ = sc->getSCE(SC_E_SLASH_COUNT)->val1;
				break;
			case SHC_IMPACT_CRATER:
				if (sc && sc->getSCE(SC_ROLLINGCUTTER))
					wd.div_ = sc->getSCE(SC_ROLLINGCUTTER)->val1;
				break;
			case MT_AXE_STOMP:
				if (sd && sd->status.weapon == W_2HAXE)
					wd.div_ = 3;
				break;
			case SHC_SAVAGE_IMPACT:
				wd.div_ = wd.div_ + wd.miscflag;
				break;
			case MT_MIGHTY_SMASH:
				if (sc && sc->getSCE(SC_AXE_STOMP))
					wd.div_ = 7;
				break;
			case BO_EXPLOSIVE_POWDER:
				if (sc && sc->getSCE(SC_RESEARCHREPORT))
					wd.div_ = 5;
				break;
			case BO_MAYHEMIC_THORNS:
				if (sc && sc->getSCE(SC_RESEARCHREPORT))
					wd.div_ = 4;
				break;
			case HN_DOUBLEBOWLINGBASH:
				if (wd.miscflag > 1)
					wd.div_ += min(4, wd.miscflag);
				break;
		}
	} else {
		bool is_long = false;

		if (is_skill_using_arrow(src, skill_id) || (sc && sc->getSCE(SC_SOULATTACK)))
			is_long = true;
		wd.flag |= is_long ? BF_LONG : BF_SHORT;
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
		map_session_data *tsd = BL_CAST(BL_PC, target);
		status_change *tsc = status_get_sc(target);
		struct status_data *sstatus = status_get_status_data(src);
		struct unit_data *ud = unit_bl2ud(target);
		t_tick tick = gettick(), rdelay = 0;

		if (!tsc)
			return;

		auto * sce = tsc->getSCE(SC_MAXPAIN);
		if (sce) {
			sce->val2 = (int)damage;
			if (!tsc->getSCE(SC_KYOMU) && !(tsc->getSCE(SC_DARKCROW) && (wd->flag&BF_SHORT))) //SC_KYOMU invalidates reflecting ability. SC_DARKCROW also does, but only for short weapon attack.
				skill_castend_damage_id(target, src, NPC_MAXPAIN_ATK, sce->val1, tick, ((wd->flag & 1) ? wd->flag - 1 : wd->flag));
		}
		
		// Calculate skill reflect damage separately
		if ((ud && !ud->immune_attack) || !status_bl_has_mode(target, MD_SKILLIMMUNE))
			rdamage = battle_calc_return_damage(target, src, &damage, wd->flag, skill_id,true);
		if( rdamage > 0 ) {
			struct block_list *d_bl = battle_check_devotion(src);
			status_change *sc = status_get_sc(src);

			if (sc && sc->getSCE(SC_VITALITYACTIVATION))
				rdamage /= 2;
			if( attack_type == BF_WEAPON && tsc->getSCE(SC_REFLECTDAMAGE) ) // Don't reflect your own damage (Grand Cross)
				map_foreachinshootrange(battle_damage_area,target,skill_get_splash(LG_REFLECTDAMAGE,1),BL_CHAR,tick,target,wd->amotion,sstatus->dmotion,rdamage,wd->flag);
			else if( attack_type == BF_WEAPON || attack_type == BF_MISC) {
				rdelay = clif_damage(src, (!d_bl) ? src : d_bl, tick, wd->amotion, sstatus->dmotion, rdamage, 1, DMG_ENDURE, 0, false);
				if( tsd )
					battle_drain(tsd, src, rdamage, rdamage, sstatus->race, sstatus->class_);
				// It appears that official servers give skill reflect damage a longer delay
				battle_delay_damage(tick, wd->amotion, target, (!d_bl) ? src : d_bl, 0, CR_REFLECTSHIELD, 0, rdamage, ATK_DEF, rdelay ,true, false);
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
	map_session_data *sd, *tsd;
	struct Damage wd;
	status_change *sc = status_get_sc(src);
	status_change *tsc = status_get_sc(target);
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);
	int right_element, left_element;
	bool infdef = false;

	memset(&wd,0,sizeof(wd));

	if (src == nullptr || target == nullptr) {
		nullpo_info(NLP_MARK);
		return wd;
	}

	wd = initialize_weapon_data(src, target, skill_id, skill_lv, wflag);

	right_element = battle_get_weapon_element(&wd, src, target, skill_id, skill_lv, EQI_HAND_R, false);
	left_element = battle_get_weapon_element(&wd, src, target, skill_id, skill_lv, EQI_HAND_L, false);

	if (sc && !sc->count)
		sc = nullptr; //Skip checking as there are no status changes active.
	if (tsc && !tsc->count)
		tsc = nullptr; //Skip checking as there are no status changes active.

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
	battle_calc_multi_attack(&wd, src, target, skill_id, skill_lv);

	// crit check is next since crits always hit on official [helvetica]
	if (is_attack_critical(&wd, src, target, skill_id, skill_lv, true)) {
#if PACKETVER >= 20161207
		if (wd.type&DMG_MULTI_HIT)
			wd.type = DMG_MULTI_HIT_CRITICAL;
		else
			wd.type = DMG_CRITICAL;
#else
		wd.type = DMG_CRITICAL;
#endif
	}

	std::bitset<NK_MAX> nk = battle_skill_get_damage_properties(skill_id, wd.miscflag);

	// check if we're landing a hit
	if(!is_attack_hitting(&wd, src, target, skill_id, skill_lv, true))
		wd.dmg_lv = ATK_FLEE;
	else if(!(infdef = is_infinite_defense(target, wd.flag))) { //no need for math against plants

#ifndef RENEWAL
		// First call function with skill_id 0 to get base damage of a normal attack
		battle_calc_skill_base_damage(&wd, src, target, 0, 0); // base damage
		wd.basedamage = wd.damage;
		// Now get actual skill damage
		if (skill_id != 0)
#endif
			battle_calc_skill_base_damage(&wd, src, target, skill_id, skill_lv); // base skill damage

#ifndef RENEWAL
		// Skill ratio
		ATK_RATE(wd.damage, wd.damage2, battle_calc_attack_skill_ratio(&wd, src, target, skill_id, skill_lv));

		// Additive damage bonus
		ATK_ADD(wd.damage, wd.damage2, battle_calc_skill_constant_addition(&wd, src, target, skill_id, skill_lv));
#endif

#ifdef RENEWAL
		if(skill_id == HW_MAGICCRASHER) { // Add weapon attack for MATK onto Magic Crasher
			struct status_data *sstatus = status_get_status_data(src);

			if (sstatus->matk_max > sstatus->matk_min) {
				ATK_ADD(wd.weaponAtk, wd.weaponAtk2, sstatus->matk_min+rnd()%(sstatus->matk_max-sstatus->matk_min));
			} else
				ATK_ADD(wd.weaponAtk, wd.weaponAtk2, sstatus->matk_min);
		}
#endif

		int i = 0;

#ifndef RENEWAL
		// add any miscellaneous player ATK bonuses
		if( sd && skill_id && (i = pc_skillatk_bonus(sd, skill_id)))
			ATK_ADDRATE(wd.damage, wd.damage2, i);
		if (tsd && (i = pc_sub_skillatk_bonus(tsd, skill_id)))
			ATK_ADDRATE(wd.damage, wd.damage2, -i);
#endif

#ifdef RENEWAL
		// In Renewal we only cardfix to the weapon and equip ATK
		//Card Fix for attacker (sd), 2 is added to the "left" flag meaning "attacker cards only"
		if (sd) {
			wd.weaponAtk += battle_calc_cardfix(BF_WEAPON, src, target, nk, right_element, left_element, wd.weaponAtk, 2, wd.flag);
			wd.equipAtk += battle_calc_cardfix(BF_WEAPON, src, target, nk, right_element, left_element, wd.equipAtk, 2, wd.flag);
			if (is_attack_left_handed(src, skill_id)) {
				wd.weaponAtk2 += battle_calc_cardfix(BF_WEAPON, src, target, nk, right_element, left_element, wd.weaponAtk2, 3, wd.flag);
				wd.equipAtk2 += battle_calc_cardfix(BF_WEAPON, src, target, nk, right_element, left_element, wd.equipAtk2, 3, wd.flag);
			}
		}

		//Card Fix for target (tsd), 2 is not added to the "left" flag meaning "target cards only"
		if (tsd && skill_id != NJ_ISSEN && skill_id != GN_FIRE_EXPANSION_ACID) { // These skills will do a card fix later
			std::bitset<NK_MAX> ignoreele_nk = nk;

			ignoreele_nk.set(NK_IGNOREELEMENT);
			wd.statusAtk += battle_calc_cardfix(BF_WEAPON, src, target, ignoreele_nk, right_element, left_element, wd.statusAtk, 0, wd.flag);
			wd.weaponAtk += battle_calc_cardfix(BF_WEAPON, src, target, nk, right_element, left_element, wd.weaponAtk, 0, wd.flag);
			wd.equipAtk += battle_calc_cardfix(BF_WEAPON, src, target, nk, right_element, left_element, wd.equipAtk, 0, wd.flag);
			wd.masteryAtk += battle_calc_cardfix(BF_WEAPON, src, target, ignoreele_nk, right_element, left_element, wd.masteryAtk, 0, wd.flag);
			if (is_attack_left_handed(src, skill_id)) {
				wd.statusAtk2 += battle_calc_cardfix(BF_WEAPON, src, target, ignoreele_nk, right_element, left_element, wd.statusAtk2, 1, wd.flag);
				wd.weaponAtk2 += battle_calc_cardfix(BF_WEAPON, src, target, nk, right_element, left_element, wd.weaponAtk2, 1, wd.flag);
				wd.equipAtk2 += battle_calc_cardfix(BF_WEAPON, src, target, nk, right_element, left_element, wd.equipAtk2, 1, wd.flag);
				wd.masteryAtk2 += battle_calc_cardfix(BF_WEAPON, src, target, ignoreele_nk, right_element, left_element, wd.masteryAtk2, 1, wd.flag);
			}
		}

		// final attack bonuses that aren't affected by cards
		battle_attack_sc_bonus(&wd, src, target, skill_id, skill_lv);

		if (sd) { //monsters, homuns and pets have their damage computed directly
			wd.damage = wd.statusAtk + wd.weaponAtk + wd.equipAtk + wd.percentAtk;
			if( is_attack_left_handed( src, skill_id ) ){
				wd.damage2 = wd.statusAtk2 + wd.weaponAtk2 + wd.equipAtk2 + wd.percentAtk2;
			}
			// Apply P.ATK mod
			// But for Dragonbreaths it only applies if Dragonic Aura is skilled
			if( ( skill_id != RK_DRAGONBREATH && skill_id != RK_DRAGONBREATH_WATER ) || pc_checkskill( sd, DK_DRAGONIC_AURA ) > 0 ){
				wd.damage = (int64)floor( (float)( wd.damage * ( 100 + sstatus->patk ) / 100 ) );
				if( is_attack_left_handed( src, skill_id ) ){
					wd.damage2 = (int64)floor( (float)( wd.damage2 * ( 100 + sstatus->patk ) / 100 ) );
				}
			}
			// Apply MasteryATK
			wd.damage += wd.masteryAtk;
			if( is_attack_left_handed( src, skill_id ) ){
				wd.damage2 += wd.masteryAtk2;
			}

			// CritAtkRate modifier
			if (wd.type == DMG_CRITICAL || wd.type == DMG_MULTI_HIT_CRITICAL) {
				if (skill_id > 0) {
					wd.damage += (int64)floor((float)(wd.damage * sd->bonus.crit_atk_rate / 200));
					if (is_attack_left_handed(src, skill_id))
						wd.damage2 += (int64)floor((float)(wd.damage2 * sd->bonus.crit_atk_rate / 200));
				}
				else {
					wd.damage += (int64)floor((float)(wd.damage * sd->bonus.crit_atk_rate / 100));
					if (is_attack_left_handed(src, skill_id))
						wd.damage2 += (int64)floor((float)(wd.damage2 * sd->bonus.crit_atk_rate / 100));
				}
			}

			if (wd.flag & BF_SHORT)
				ATK_ADDRATE(wd.damage, wd.damage2, sd->bonus.short_attack_atk_rate);
			if(wd.flag&BF_LONG && (skill_id != RA_WUGBITE && skill_id != RA_WUGSTRIKE)) //Long damage rate addition doesn't use weapon + equip attack
				ATK_ADDRATE(wd.damage, wd.damage2, sd->bonus.long_attack_atk_rate);
		}

		// Skill ratio
		ATK_RATE(wd.damage, wd.damage2, battle_calc_attack_skill_ratio(&wd, src, target, skill_id, skill_lv));

		// Additive damage bonus
		ATK_ADD(wd.damage, wd.damage2, battle_calc_skill_constant_addition(&wd, src, target, skill_id, skill_lv));

		// Advance Katar Mastery
		if (sd) {
			uint16 katar_skill;

			if (sd->status.weapon == W_KATAR && (katar_skill = pc_checkskill(sd, ASC_KATAR)) > 0) // Adv. Katar Mastery applied after calculate with skillratio.
				ATK_ADDRATE(wd.damage, wd.damage2, (10 + 2 * katar_skill));
		}

		// Res reduces physical damage by a percentage and
		// is calculated before DEF and other reductions.
		// All skills that use the simple defense formula (damage substracted by DEF+DEF2) ignore Res
		// TODO: Res formula probably should be: (2000+x)/(2000+5x), but with the reduction rounded down
		if ((wd.damage + wd.damage2) && tstatus->res > 0 && !nk[NK_SIMPLEDEFENSE]) {
			short res = tstatus->res;
			short ignore_res = 0;// Value used as a percentage.

			if (sd)
				ignore_res += sd->indexed_bonus.ignore_res_by_race[tstatus->race] + sd->indexed_bonus.ignore_res_by_race[RC_ALL];

			// Attacker status's that pierce Res.
			if (sc) {
				if (sc->getSCE(SC_A_TELUM))
					ignore_res += sc->getSCE(SC_A_TELUM)->val2;
				if (sc->getSCE(SC_POTENT_VENOM))
					ignore_res += sc->getSCE(SC_POTENT_VENOM)->val2;
			}

			ignore_res = min(ignore_res, battle_config.max_res_mres_ignored);

			if (ignore_res > 0)
				res -= res * ignore_res / 100;

			// Apply damage reduction.
			wd.damage = wd.damage * (5000 + res) / (5000 + 10 * res);
			wd.damage2 = wd.damage2 * (5000 + res) / (5000 + 10 * res);
		}

#else
		// final attack bonuses that aren't affected by cards
		battle_attack_sc_bonus(&wd, src, target, skill_id, skill_lv);
#endif

		if (wd.damage + wd.damage2) {
#ifdef RENEWAL
			// Check if attack ignores DEF (in pre-renewal we need to update base damage even when the skill ignores DEF)
			if(!attack_ignores_def(&wd, src, target, skill_id, skill_lv, EQI_HAND_L) || !attack_ignores_def(&wd, src, target, skill_id, skill_lv, EQI_HAND_R))
#else
			// Shield Boomerang and Rapid Smiting already calculated the defense before the skill ratio was applied
			if(skill_id != PA_SHIELDCHAIN && skill_id != CR_SHIELDBOOMERANG)
#endif
				battle_calc_defense_reduction(&wd, src, target, skill_id, skill_lv);

			battle_calc_attack_post_defense(&wd, src, target, skill_id, skill_lv);
		}

#ifdef RENEWAL
		// add any miscellaneous player ATK bonuses
		if( sd && skill_id && (i = pc_skillatk_bonus(sd, skill_id))) {
			ATK_ADDRATE(wd.damage, wd.damage2, i);
			RE_ALLATK_ADDRATE(&wd, i);
		}
		if (tsd && (i = pc_sub_skillatk_bonus(tsd, skill_id))) {
			ATK_ADDRATE(wd.damage, wd.damage2, -i);
			RE_ALLATK_ADDRATE(&wd, -i);
		}
#endif
	}

	battle_calc_element_damage(&wd, src, target, skill_id, skill_lv);

#ifdef RENEWAL
	if (is_attack_critical(&wd, src, target, skill_id, skill_lv, false)) {
		if (sd) { //Check for player so we don't crash out, monsters don't have bonus crit rates [helvetica]
			wd.damage = (int64)floor((float)((wd.damage * (1.4f + (0.01f * sstatus->crate)))));
			if (is_attack_left_handed(src, skill_id))
				wd.damage2 = (int64)floor((float)((wd.damage2 * (1.4f + (0.01f * sstatus->crate)))));
		} else
			wd.damage = (int64)floor((float)(wd.damage * 1.4f));

		if (tsd && tsd->bonus.crit_def_rate != 0)
			ATK_ADDRATE(wd.damage, wd.damage2, -tsd->bonus.crit_def_rate);
	}
#endif

	switch (skill_id) {
		case TK_DOWNKICK:
		case TK_STORMKICK:
		case TK_TURNKICK:
		case TK_COUNTER:
			if(sd && sd->weapontype1 == W_FIST && sd->weapontype2 == W_FIST)
				ATK_ADD(wd.damage, wd.damage2, 10 * pc_checkskill(sd, TK_RUN));
			break;
		case LG_SHIELDPRESS:
			if (sd) {
				int damagevalue = 0;
				short index = sd->equip_index[EQI_HAND_L];

				if (index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_ARMOR)
					damagevalue = sstatus->vit * sd->inventory.u.items_inventory[index].refine;
				ATK_ADD(wd.damage, wd.damage2, damagevalue);
			}
			break;
		case SR_TIGERCANNON:
			// (Tiger Cannon skill level x 240) + (Target Base Level x 40)
			if (wd.miscflag&8) {
				ATK_ADD(wd.damage, wd.damage2, skill_lv * 500 + status_get_lv(target) * 40);
			} else
				ATK_ADD(wd.damage, wd.damage2, skill_lv * 240 + status_get_lv(target) * 40);
			break;
		case SR_GATEOFHELL: {
			status_data *sstatus = status_get_status_data(src);
			double bonus = 1 + skill_lv * 2 / 10;

			ATK_ADD(wd.damage, wd.damage2, sstatus->max_hp - sstatus->hp);
			if(sc && sc->getSCE(SC_COMBO) && sc->getSCE(SC_COMBO)->val1 == SR_FALLENEMPIRE) {
				ATK_ADD(wd.damage, wd.damage2, static_cast<int64>(sstatus->max_sp * bonus) + 40 * status_get_lv(src));
			} else
				ATK_ADD(wd.damage, wd.damage2, static_cast<int64>(sstatus->sp * bonus) + 10 * status_get_lv(src));
		}
			break;
		case MH_TINDER_BREAKER:
			ATK_ADD(wd.damage, wd.damage2, 2500 * skill_lv + status_get_lv(src)); // !TODO: Confirm base level bonus
			break;
		case MH_CBC:
			ATK_ADD(wd.damage, wd.damage2, 4000 * skill_lv + status_get_lv(src)); // !TODO: Confirm base level bonus
			break;
		case MH_EQC:
			ATK_ADD(wd.damage, wd.damage2, 6000 * skill_lv + status_get_lv(src)); // !TODO: Confirm base level bonus
			break;
		case NPC_MAXPAIN_ATK:
			if (sc) {
				auto * sce = sc->getSCE(SC_MAXPAIN);
				if (sce) {
					if (sce->val2)
						wd.damage = sce->val2 * skill_lv / 10;
					else if (sce->val3)
						wd.damage = sce->val3 * skill_lv / 10;
				}
			}
			else 
				wd.damage = 0;
			break;
	}

	if(sd) {
#ifndef RENEWAL
		uint16 skill;

		if ((skill = pc_checkskill(sd, BS_WEAPONRESEARCH)) > 0)
			ATK_ADD(wd.damage, wd.damage2, skill * 2);
		if (skill_id == GS_GROUNDDRIFT)
			ATK_ADD(wd.damage, wd.damage2, 50 * skill_lv);
		if (skill_id != MC_CARTREVOLUTION && pc_checkskill(sd, BS_HILTBINDING) > 0)
			ATK_ADD(wd.damage, wd.damage2, 4);

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
				wd.damage += battle_calc_cardfix(BF_WEAPON, src, target, nk, right_element, left_element, wd.damage, 2, wd.flag);
				if( is_attack_left_handed(src, skill_id ))
					wd.damage2 += battle_calc_cardfix(BF_WEAPON, src, target, nk, right_element, left_element, wd.damage2, 3, wd.flag);
				break;
		}
#endif
	}

#ifdef RENEWAL
	// In renewal only do it for non player attacks
	if( tsd && !sd ){
#else
	if( tsd ){
#endif
		// Card Fix for target (tsd), 2 is not added to the "left" flag meaning "target cards only"
		wd.damage += battle_calc_cardfix(BF_WEAPON, src, target, nk, right_element, left_element, wd.damage, 0, wd.flag);
		if(is_attack_left_handed(src, skill_id))
			wd.damage2 += battle_calc_cardfix(BF_WEAPON, src, target, nk, right_element, left_element, wd.damage2, 1, wd.flag);
	}

	// only do 1 dmg to plant, no need to calculate rest
	if(infdef){
		battle_calc_attack_plant(&wd, src, target, skill_id, skill_lv);
		return wd;
	}

	//Apply DAMAGE_DIV_FIX and check for min damage
	battle_apply_div_fix(&wd, skill_id);

	battle_calc_attack_left_right_hands(&wd, src, target, skill_id, skill_lv);

#ifdef RENEWAL
	switch (skill_id) {
		case NJ_ISSEN:
			case GN_FIRE_EXPANSION_ACID:
			return wd; //These skills will do a GVG fix later
		default:
#endif
			battle_calc_attack_gvg_bg(&wd, src, target, skill_id, skill_lv);
#ifdef RENEWAL
			break;
	}
#endif

	battle_calc_weapon_final_atk_modifiers(&wd, src, target, skill_id, skill_lv);

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
	int i, skill_damage = 0;
	short s_ele = 0;

	TBL_PC *sd;
	TBL_PC *tsd;
	status_change *sc, *tsc;
	struct Damage ad;
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);
	struct {
		unsigned imdef : 1;
		unsigned infdef : 1;
	} flag;

	memset(&ad,0,sizeof(ad));
	memset(&flag,0,sizeof(flag));

	if (src == nullptr || target == nullptr) {
		nullpo_info(NLP_MARK);
		return ad;
	}
	//Initial Values
	// Initial Values
	// Set to 1 because magic damage on plants is 1 per hit; if target is not a plant this gets reinitialized to 0 later
	ad.damage = 1;
	ad.div_ = skill_get_num(skill_id,skill_lv);
	ad.amotion = (skill_get_inf(skill_id)&INF_GROUND_SKILL ? 0 : sstatus->amotion); //Amotion should be 0 for ground skills.
	ad.dmotion = tstatus->dmotion;
	ad.blewcount = skill_get_blewcount(skill_id, skill_lv);
	ad.flag = BF_MAGIC|BF_SKILL;
	ad.miscflag = mflag;
	ad.dmg_lv = ATK_DEF;

	std::shared_ptr<s_skill_db> skill = skill_db.find(skill_id);
	std::bitset<NK_MAX> nk;

	if (skill)
		nk = skill->nk;

	flag.imdef = nk[NK_IGNOREDEFENSE] ? 1 : 0;

	sd = BL_CAST(BL_PC, src);
	tsd = BL_CAST(BL_PC, target);
	s_elemental_data* ed = BL_CAST(BL_ELEM, src);
	sc = status_get_sc(src);
	tsc = status_get_sc(target);

	//Initialize variables that will be used afterwards
	s_ele = battle_get_magic_element(src, target, skill_id, skill_lv, mflag);

	switch(skill_id) {
		case WL_HELLINFERNO:
			if (mflag & 2) { // ELE_DARK
				ad.div_ = 3;
			}
			break;
		case NPC_PSYCHIC_WAVE:
		case SO_PSYCHIC_WAVE:
			if (sd && (sd->weapontype1 == W_STAFF || sd->weapontype1 == W_2HSTAFF || sd->weapontype1 == W_BOOK))
				ad.div_ = 2;
			break;
		case AG_DESTRUCTIVE_HURRICANE:
			if (sc && sc->getSCE(SC_CLIMAX) && sc->getSCE(SC_CLIMAX)->val1 == 2)
				ad.blewcount = 2;
			break;
		case AG_CRYSTAL_IMPACT:
			if (sc && sc->getSCE(SC_CLIMAX) && sc->getSCE(SC_CLIMAX)->val1 == 2)
				ad.div_ = 2;
			break;
		case ABC_ABYSS_SQUARE:
			if (mflag == 2)
				ad.div_ = 2;
			break;
		case AG_CRIMSON_ARROW_ATK:
			if( sc != nullptr && sc->getSCE( SC_CLIMAX ) ){
				ad.div_ = 2;
			}
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
			[[fallthrough]];
		case NJ_KAENSIN:
		case PR_SANCTUARY:
			ad.dmotion = 1; //No flinch animation.
			break;
	}

	if (!flag.infdef) { //No need to do the math for plants
		unsigned int skillratio = 100; //Skill dmg modifiers.
		//Damage was set to 1 to simulate plant damage; if not plant, need to reinitialize damage with 0
		ad.damage = 0;
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
				if(rnd()%1000 < i && !status_has_mode(tstatus,MD_STATUSIMMUNE))
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
			case NPC_DARKBREATH:
				ad.damage = tstatus->hp * (skill_lv <= 5 ? 100 / (2 * (6 - skill_lv)) : 50) / 100;
				break;
			case PF_SOULBURN:
				ad.damage = tstatus->sp * 2;
				break;
			case AB_RENOVATIO:
				ad.damage = status_get_lv(src) * 10 + sstatus->int_;
				break;
			case NPC_EARTHQUAKE:
				if (mflag & NPC_EARTHQUAKE_FLAG) {
					ad.flag |= NPC_EARTHQUAKE_FLAG; // Pass flag to battle_calc_damage
					mflag &= ~NPC_EARTHQUAKE_FLAG; // Remove before NK_SPLASHSPLIT check
				}

				if (src->type == BL_PC)
					ad.damage = sstatus->str * 2 + battle_calc_weapon_attack(src, target, skill_id, skill_lv, mflag).damage;
				else
					ad.damage = battle_calc_base_damage(src, sstatus, &sstatus->rhw, sc, tstatus->size, 0);

				MATK_RATE(200 + 100 * skill_lv + 100 * (skill_lv / 2) + ((skill_lv > 4) ? 100 : 0));

				if (nk[NK_SPLASHSPLIT] && mflag > 1)
					ad.damage /= mflag;
				break;
			case NPC_ICEMINE:
			case NPC_FLAMECROSS:
				ad.damage = static_cast<int64>( sstatus->rhw.atk ) * static_cast<int64>( 20 ) * static_cast<int64>( skill_lv );
				break;
			default: {
				if (sstatus->matk_max > sstatus->matk_min) {
					MATK_ADD(sstatus->matk_min+rnd()%(sstatus->matk_max-sstatus->matk_min));
				} else {
					MATK_ADD(sstatus->matk_min);
				}

				if (sd) { // Soul energy spheres adds MATK.
					MATK_ADD(3*sd->soulball);
				}

				if (nk[NK_SPLASHSPLIT]) { // Divide MATK in case of multiple targets skill
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
						if (sc) {
							if ((skill_id == MG_FIREBOLT && sc->getSCE(SC_FLAMETECHNIC_OPTION)) ||
								(skill_id == MG_COLDBOLT && sc->getSCE(SC_COLD_FORCE_OPTION)) ||
								(skill_id == MG_LIGHTNINGBOLT && sc->getSCE(SC_GRACE_BREEZE_OPTION)))
								skillratio *= 5;

							if (sc->getSCE(SC_SPELLFIST) && mflag & BF_SHORT) {
								skillratio += (sc->getSCE(SC_SPELLFIST)->val3 * 100) + (sc->getSCE(SC_SPELLFIST)->val1 * 50 - 50) - 100; // val3 = used bolt level, val1 = used spellfist level. [Rytech]
								ad.div_ = 1; // ad mods, to make it work similar to regular hits [Xazax]
								ad.flag = BF_WEAPON | BF_SHORT;
								ad.type = DMG_NORMAL;
							}
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
						if (sd && sd->sc.getSCE(SC_SPIRIT) && sd->sc.getSCE(SC_SPIRIT)->val2 == SL_PRIEST)
							skillratio *= 5; //Does 5x damage include bonuses from other skills?
						break;
					case AL_RUWACH:
						skillratio += 45;
						break;
					case WZ_FROSTNOVA:
						skillratio += -100 + (100 + skill_lv * 10) * 2 / 3;
						break;
					case WZ_FIREPILLAR:
						if (sd && ad.div_ > 0)
							ad.div_ *= -1; //For players, damage is divided by number of hits
						skillratio += -60 + 20 * skill_lv; //20% MATK each hit
						break;
					case WZ_SIGHTRASHER:
						skillratio += 20 * skill_lv;
						break;
					case WZ_WATERBALL:
						skillratio += 30 * skill_lv;
						break;
					case WZ_STORMGUST:
#ifdef RENEWAL
						skillratio -= 30; // Offset only once
						skillratio += 50 * skill_lv;
#else
						skillratio += 40 * skill_lv;
#endif
						break;
#ifdef RENEWAL
					case WZ_EARTHSPIKE:
						skillratio += 100;
						if (sc && sc->getSCE(SC_EARTH_CARE_OPTION))
							skillratio += skillratio * 800 / 100;
						break;
#endif
					case HW_NAPALMVULCAN:
#ifdef RENEWAL
						skillratio += -100 + 70 * skill_lv;
						RE_LVL_DMOD(100);
#else
						skillratio += 25;
#endif
						break;
					case SL_STIN: //Target size must be small (0) for full damage
						skillratio += (tstatus->size != SZ_SMALL ? -99 : 10 * skill_lv);
						break;
					case SL_STUN:
						skillratio += 5 * skill_lv;
						break;
					case SL_SMA: //Base damage is 40% + lv%
						skillratio += -60 + status_get_lv(src);
						break;
					case NJ_KOUENKA:
						skillratio -= 10;
						if(sd && sd->spiritcharm_type == CHARM_TYPE_FIRE && sd->spiritcharm > 0)
							skillratio += 10 * sd->spiritcharm;
						break;
					case NJ_KAENSIN:
						skillratio -= 50;
						if(sd && sd->spiritcharm_type == CHARM_TYPE_FIRE && sd->spiritcharm > 0)
							skillratio += 20 * sd->spiritcharm;
						break;
					case NJ_BAKUENRYU:
						skillratio += 50 + 150 * skill_lv;
						if(sd && sd->spiritcharm_type == CHARM_TYPE_FIRE && sd->spiritcharm > 0)
							skillratio += 100 * sd->spiritcharm;
						break;
					case NJ_HYOUSENSOU:
#ifdef RENEWAL
						skillratio -= 30;
						if (sc && sc->getSCE(SC_SUITON))
							skillratio += 2 * skill_lv;
#endif
						if(sd && sd->spiritcharm_type == CHARM_TYPE_WATER && sd->spiritcharm > 0)
							skillratio += 20 * sd->spiritcharm;
						break;
					case NJ_HYOUSYOURAKU:
						skillratio += 50 * skill_lv;
						if(sd && sd->spiritcharm_type == CHARM_TYPE_WATER && sd->spiritcharm > 0)
							skillratio += 100 * sd->spiritcharm;
						break;
					case NJ_RAIGEKISAI:
#ifdef RENEWAL
						skillratio += 100 * skill_lv;
#else
						skillratio += 60 + 40 * skill_lv;
#endif
						if(sd && sd->spiritcharm_type == CHARM_TYPE_WIND && sd->spiritcharm > 0)
							skillratio += 20 * sd->spiritcharm;
						break;
					case NJ_KAMAITACHI:
						skillratio += 100 * skill_lv;
						if(sd && sd->spiritcharm_type == CHARM_TYPE_WIND && sd->spiritcharm > 0)
							skillratio += 100 * sd->spiritcharm;
						break;
					case NJ_HUUJIN:
#ifdef RENEWAL
						skillratio += 50;
#endif
						if(sd && sd->spiritcharm_type == CHARM_TYPE_WIND && sd->spiritcharm > 0)
							skillratio += 10 * sd->spiritcharm;
						break;
					case NPC_ENERGYDRAIN:
						skillratio += 100 * skill_lv;
						break;
#ifdef RENEWAL
					case WZ_HEAVENDRIVE:
					case NPC_GROUNDDRIVE:
						skillratio += 25;
						break;
					case WZ_METEOR:
						skillratio += 25;
						break;
					case WZ_VERMILION:
						if(sd)
							skillratio += 300 + skill_lv * 100;
						else
							skillratio += 20 * skill_lv - 20; //Monsters use old formula
						break;
					case PR_MAGNUS:
						if (battle_check_undead(tstatus->race, tstatus->def_ele) || tstatus->race == RC_DEMON)
							skillratio += 30;
						break;
					case BA_DISSONANCE:
						skillratio += skill_lv * 10;
						if (sd)
							skillratio += 3 * pc_checkskill(sd, BA_MUSICALLESSON);
						break;
					case HW_GRAVITATION:
						skillratio += -100 + 100 * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case PA_PRESSURE:
						skillratio += -100 + 500 + 150 * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case WZ_SIGHTBLASTER:
						skillratio += 500;
						break;
#else
					case WZ_VERMILION:
						skillratio += 20 * skill_lv - 20;
						break;
#endif
					case AB_JUDEX:
						skillratio += -100 + 300 + 70 * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case AB_ADORAMUS:
						skillratio += - 100 + 300 + 250 * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case AB_DUPLELIGHT_MAGIC:
						skillratio += 300 + 40 * skill_lv;
						break;
					case WL_SOULEXPANSION:
						skillratio += -100 + 1000 + skill_lv * 200 + sstatus->int_ / 6; // !TODO: Confirm INT bonus
						RE_LVL_DMOD(100);
						break;
					case WL_FROSTMISTY:
						skillratio += -100 + 200 + 100 * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case NPC_JACKFROST:
						if (tsc && tsc->getSCE(SC_FREEZING)) {
							skillratio += 900 + 300 * skill_lv;
							RE_LVL_DMOD(100);
						} else {
							skillratio += 400 + 100 * skill_lv;
							RE_LVL_DMOD(150);
						}
						break;
					case WL_JACKFROST:
						if (tsc && tsc->getSCE(SC_MISTY_FROST))
							skillratio += -100 + 1200 + 600 * skill_lv;
						else
							skillratio += -100 + 1000 + 300 * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case WL_DRAINLIFE:
						skillratio += -100 + 200 * skill_lv + sstatus->int_;
						RE_LVL_DMOD(100);
						break;
					case WL_CRIMSONROCK:
						skillratio += -100 + 700 + 600 * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case WL_HELLINFERNO:
						skillratio += -100 + 400 * skill_lv;
						if (mflag & 2) // ELE_DARK
							skillratio += 200;
						RE_LVL_DMOD(100);
						break;
					case WL_COMET:
						skillratio += -100 + 2500 + 700 * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case WL_CHAINLIGHTNING_ATK:
						skillratio += 400 + 100 * skill_lv;
						RE_LVL_DMOD(100);
						if (mflag > 0)
							skillratio += 100 * mflag;
						break;
					case WL_EARTHSTRAIN:
						skillratio += -100 + 1000 + 600 * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case WL_TETRAVORTEX_FIRE:
					case WL_TETRAVORTEX_WATER:
					case WL_TETRAVORTEX_WIND:
					case WL_TETRAVORTEX_GROUND:
						skillratio += -100 + 800 + 400 * skill_lv;
						break;
					case WL_SUMMON_ATK_FIRE:
					case WL_SUMMON_ATK_WATER:
					case WL_SUMMON_ATK_WIND:
					case WL_SUMMON_ATK_GROUND:
						skillratio += 200;
						RE_LVL_DMOD(100);
						break;
					case LG_RAYOFGENESIS:
						skillratio += -100 + 350 * skill_lv + sstatus->int_; // !TODO: What's the INT bonus?
						RE_LVL_DMOD(100);
						break;
					case NPC_RAYOFGENESIS:
						skillratio += -100 + 200 * skill_lv;
						break;
					case WM_METALICSOUND:
						skillratio += -100 + 120 * skill_lv + 60 * ((sd) ? pc_checkskill(sd, WM_LESSON) : 1);
						if (tsc && tsc->getSCE(SC_SLEEP))
							skillratio += 100; // !TODO: Confirm target sleeping bonus
						RE_LVL_DMOD(100);
						if (tsc && tsc->getSCE(SC_SOUNDBLEND))
							skillratio += skillratio * 50 / 100;
						break;
					case WM_REVERBERATION:
						// MATK [{(Skill Level x 300) + 400} x Casters Base Level / 100] %
						skillratio += -100 + 700 + 300 * skill_lv;
						RE_LVL_DMOD(100);
						if (tsc && tsc->getSCE(SC_SOUNDBLEND))
							skillratio += skillratio * 50 / 100;
						break;
					case SO_FIREWALK:
						skillratio += -100 + 60 * skill_lv;
						RE_LVL_DMOD(100);
						if( sc && sc->getSCE(SC_HEATER_OPTION) )
							skillratio += (sd ? sd->status.job_level / 2 : 0);
						break;
					case SO_ELECTRICWALK:
						skillratio += -100 + 60 * skill_lv;
						RE_LVL_DMOD(100);
						if( sc && sc->getSCE(SC_BLAST_OPTION) )
							skillratio += (sd ? sd->status.job_level / 2 : 0);
						break;
					case NPC_FIREWALK:
					case NPC_ELECTRICWALK:
						skillratio += -100 + 100 * skill_lv;
						break;
					case SO_EARTHGRAVE:
						skillratio += -100 + 2 * sstatus->int_ + 300 * pc_checkskill(sd, SA_SEISMICWEAPON) + sstatus->int_ * skill_lv;
						RE_LVL_DMOD(100);
						if( sc && sc->getSCE(SC_CURSED_SOIL_OPTION) )
							skillratio += (sd ? sd->status.job_level * 5 : 0);
						break;
					case SO_DIAMONDDUST:
						skillratio += -100 + 2 * sstatus->int_ + 300 * pc_checkskill(sd, SA_FROSTWEAPON) + sstatus->int_ * skill_lv;
						RE_LVL_DMOD(100);
						if( sc && sc->getSCE(SC_COOLER_OPTION) )
							skillratio += (sd ? sd->status.job_level * 5 : 0);
						break;
					case SO_POISON_BUSTER:
						skillratio += -100 + 1000 + 300 * skill_lv + sstatus->int_ / 6; // !TODO: Confirm INT bonus
						if( tsc && tsc->getSCE(SC_CLOUD_POISON) )
							skillratio += 200 * skill_lv;
						RE_LVL_DMOD(100);
						if( sc && sc->getSCE(SC_CURSED_SOIL_OPTION) )
							skillratio += (sd ? sd->status.job_level * 5 : 0);
						break;
					case NPC_POISON_BUSTER:
						skillratio += -100 + 1500 * skill_lv;
						break;
					case SO_PSYCHIC_WAVE:
						skillratio += -100 + 70 * skill_lv + 3 * sstatus->int_;
						RE_LVL_DMOD(100);
						if (sc && (sc->getSCE(SC_HEATER_OPTION) || sc->getSCE(SC_COOLER_OPTION) ||
							sc->getSCE(SC_BLAST_OPTION) || sc->getSCE(SC_CURSED_SOIL_OPTION)))
							skillratio += 20;
						break;
					case NPC_PSYCHIC_WAVE:
						skillratio += -100 + 500 * skill_lv;
						break;
					case SO_CLOUD_KILL:
						skillratio += -100 + 40 * skill_lv;
						RE_LVL_DMOD(100);
						if (sc) {
							if (sc->getSCE(SC_CURSED_SOIL_OPTION))
								skillratio += (sd ? sd->status.job_level : 0);

							if (sc->getSCE(SC_DEEP_POISONING_OPTION))
								skillratio += skillratio * 1500 / 100;
						}
						break;
					case NPC_CLOUD_KILL:
						skillratio += -100 + 50 * skill_lv;
						break;
					case SO_VARETYR_SPEAR:
						skillratio += -100 + (2 * sstatus->int_ + 150 * (pc_checkskill(sd, SO_STRIKING) + pc_checkskill(sd, SA_LIGHTNINGLOADER)) + sstatus->int_ * skill_lv / 2) / 3;
						RE_LVL_DMOD(100);
						if (sc && sc->getSCE(SC_BLAST_OPTION))
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
					case MH_XENO_SLASHER:
						skillratio += -100 + 450 * skill_lv * status_get_lv(src) / 100 + sstatus->int_; // !TODO: Confirm Base Level and INT bonus
						break;
					case MH_TWISTER_CUTTER:
						skillratio += -100 + 480 * skill_lv * status_get_lv(src) / 100 + sstatus->int_; // !TODO: Confirm Base Level and INT bonus
						break;
					case MH_ABSOLUTE_ZEPHYR:
						skillratio += -100 + 1000 + 450 * skill_lv * status_get_lv(src) / 100 + sstatus->int_; // !TODO: Confirm Base Level and INT bonus
						break;
					case MH_HEILIGE_STANGE:
						skillratio += -100 + 1500 + 250 * skill_lv * status_get_lv(src) / 150 + sstatus->vit; // !TODO: Confirm VIT bonus
						break;
					case MH_HEILIGE_PFERD:
						skillratio += -100 + 1200 + 350 * skill_lv * status_get_lv(src) / 100 + sstatus->vit; // !TODO: Confirm VIT bonus
						break;
					case MH_POISON_MIST:
						skillratio += -100 + 200 * skill_lv * status_get_lv(src) / 100 + sstatus->dex; // ! TODO: Confirm DEX bonus
						break;
					case SU_SV_STEMSPEAR:
						skillratio += 600;
						break;
					case SU_CN_METEOR:
					case SU_CN_METEOR2:
						skillratio += 100 + 100 * skill_lv + sstatus->int_ * 5; // !TODO: Confirm INT bonus
						RE_LVL_DMOD(100);
						break;
					case NPC_VENOMFOG:
						skillratio += 600 + 100 * skill_lv;
						break;
					case NPC_COMET:
						i = (sc ? distance_xy(target->x, target->y, sc->comet_x, sc->comet_y) : 8) / 2;
						i = cap_value(i, 1, 4);
						skillratio = 2500 + ((skill_lv - i + 1) * 500);
						break;
					case NPC_FIRESTORM:
						skillratio += 200;
						break;
					case NPC_HELLBURNING:
						skillratio += 900;
						break;
					case NPC_PULSESTRIKE2:
						skillratio += 100;
						break;
					case SP_CURSEEXPLOSION:
						if (tsc && tsc->getSCE(SC_SOULCURSE))
							skillratio += 1400 + 200 * skill_lv;
						else
							skillratio += 300 + 100 * skill_lv;
						break;
					case SP_SPA:
						skillratio += 400 + 250 * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case SP_SHA:
						skillratio += -100 + 5 * skill_lv;
						break;
					case SP_SWHOO:
						skillratio += 1000 + 200 * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case NPC_STORMGUST2:
						skillratio += 200 * skill_lv;
						break;
					case AG_DEADLY_PROJECTION:
						skillratio += -100 + 2800 * skill_lv + 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						break;
					case AG_DESTRUCTIVE_HURRICANE:
						skillratio += -100 + 600 + 2850 * skill_lv + 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						if (sc && sc->getSCE(SC_CLIMAX))
						{
							if (sc->getSCE(SC_CLIMAX)->val1 == 3)
								skillratio += skillratio * 150 / 100;
							else if (sc->getSCE(SC_CLIMAX)->val1 == 5)
								skillratio -= skillratio * 20 / 100;
						}
						break;
					case AG_RAIN_OF_CRYSTAL:
						skillratio += -100 + 180 + 760 * skill_lv + 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						break;
					case AG_MYSTERY_ILLUSION:
						skillratio += -100 + 950 * skill_lv + 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						break;
					case AG_VIOLENT_QUAKE_ATK:
						skillratio += -100 + 200 + 1200 * skill_lv + 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						if (sc && sc->getSCE(SC_CLIMAX)) {
							if (sc->getSCE(SC_CLIMAX)->val1 == 1)
								skillratio /= 2;
							else if (sc->getSCE(SC_CLIMAX)->val1 == 3)
								skillratio *= 3;
						}
						break;
					case AG_SOUL_VC_STRIKE:
						skillratio += -100 + 300 * skill_lv + 3 * sstatus->spl;
						RE_LVL_DMOD(100);
						break;
					case AG_STRANTUM_TREMOR:
						skillratio += -100 + 100 + 730 * skill_lv + 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						break;
					case AG_ALL_BLOOM_ATK:
						skillratio += -100 + 200 + 1200 * skill_lv + 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						if (sc && sc->getSCE(SC_CLIMAX)) {
							if (sc->getSCE(SC_CLIMAX)->val1 == 3)
								skillratio *= 4;
						}
						break;
					case AG_ALL_BLOOM_ATK2:// Is this affected by BaseLV and SPL too??? [Rytech]
						skillratio += -100 + 85000 + 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						break;
					case AG_CRYSTAL_IMPACT:
						skillratio += -100 + 250 + 1300 * skill_lv + 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						if (sc && sc->getSCE(SC_CLIMAX)) {
							if (sc->getSCE(SC_CLIMAX)->val1 == 3)
								skillratio += skillratio * 50 / 100;
							else if (sc->getSCE(SC_CLIMAX)->val1 == 4)
								skillratio /= 2;
						}
						break;
					case AG_CRYSTAL_IMPACT_ATK:// Said to deal the same damage as the main attack.
						skillratio += -100 + 800 * skill_lv + 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						if (sc && sc->getSCE(SC_CLIMAX) && sc->getSCE(SC_CLIMAX)->val1 == 4)
							skillratio += skillratio * 150 / 100;
						break;
					case AG_TORNADO_STORM:
						skillratio += -100 + 100 + 760 * skill_lv + 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						break;
					case AG_FLORAL_FLARE_ROAD:
						skillratio += -100 + 50 + 740 * skill_lv + 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						break;
					case AG_ASTRAL_STRIKE:
						skillratio += -100 + 1800 * skill_lv + 10 * sstatus->spl;
						if (tstatus->race == RC_UNDEAD || tstatus->race == RC_DRAGON)
							skillratio += 340 * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case AG_ASTRAL_STRIKE_ATK:
						skillratio += -100 + 650 * skill_lv + 10 * sstatus->spl;
						// Not confirmed, but if the main hit deal additional damage
						// on certain races then the repeated damage should too right?
						// Guessing a formula here for now. [Rytech]
						if (tstatus->race == RC_UNDEAD || tstatus->race == RC_DRAGON)
							skillratio += 200 * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case AG_ROCK_DOWN:
						skillratio += -100 + 1550 * skill_lv + 5 * sstatus->spl;

						if( sc != nullptr && sc->getSCE( SC_CLIMAX ) ){
							skillratio += 300 * skill_lv;
						}

						RE_LVL_DMOD(100);
						break;
					case AG_STORM_CANNON:
						skillratio += -100 + 1550 * skill_lv + 5 * sstatus->spl;

						if( sc != nullptr && sc->getSCE( SC_CLIMAX ) ){
							skillratio += 300 * skill_lv;
						}

						RE_LVL_DMOD(100);
						break;
					case AG_CRIMSON_ARROW:
						skillratio += -100 + 400 * skill_lv + 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						break;
					case AG_CRIMSON_ARROW_ATK:
						skillratio += -100 + 750 * skill_lv + 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						break;
					case AG_FROZEN_SLASH:
						skillratio += -100 + 450 + 950 * skill_lv + 5 * sstatus->spl;

						if( sc != nullptr && sc->getSCE( SC_CLIMAX ) ){
							skillratio += 150 + 350 * skill_lv;
						}

						RE_LVL_DMOD(100);
						break;
					case IG_JUDGEMENT_CROSS:
						skillratio += -100 + 1950 * skill_lv + 10 * sstatus->spl;
						if (tstatus->race == RC_PLANT || tstatus->race == RC_INSECT)
							skillratio += 150 * skill_lv;
						RE_LVL_DMOD(100);
						if ((i = pc_checkskill_imperial_guard(sd, 3)) > 0)
							skillratio += skillratio * i / 100;
						break;
					case IG_CROSS_RAIN:
						if( sc && sc->getSCE( SC_HOLY_S ) ){
							skillratio += -100 + ( 650 + 15 * pc_checkskill( sd, IG_SPEAR_SWORD_M ) ) * skill_lv;
						}else{
							skillratio += -100 + ( 450 + 10 * pc_checkskill( sd, IG_SPEAR_SWORD_M ) ) * skill_lv;
						}
						skillratio += 7 * sstatus->spl;
						RE_LVL_DMOD(100);
						break;
					case CD_ARBITRIUM:
						skillratio += -100 + 1000 * skill_lv + 10 * sstatus->spl;
						skillratio += 10 * pc_checkskill( sd, CD_FIDUS_ANIMUS ) * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case CD_ARBITRIUM_ATK:
						skillratio += -100 + 1750 * skill_lv + 10 * sstatus->spl;
						skillratio += 50 * pc_checkskill( sd, CD_FIDUS_ANIMUS ) * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case CD_PNEUMATICUS_PROCELLA:
						skillratio += -100 + 150 + 2100 * skill_lv + 10 * sstatus->spl;
						skillratio += 3 * pc_checkskill( sd, CD_FIDUS_ANIMUS );
						if (tstatus->race == RC_UNDEAD || tstatus->race == RC_DEMON) {
							skillratio += 50 + 150 * skill_lv;
							skillratio += 2 * pc_checkskill( sd, CD_FIDUS_ANIMUS );
						}
						RE_LVL_DMOD(100);
						break;
					case CD_FRAMEN:
						skillratio += -100 + 1300 * skill_lv;
						skillratio += 5 * pc_checkskill(sd,CD_FIDUS_ANIMUS) * skill_lv;
						skillratio += 5 * sstatus->spl;
						if (tstatus->race == RC_UNDEAD || tstatus->race == RC_DEMON)
							skillratio += 50 * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case AG_DESTRUCTIVE_HURRICANE_CLIMAX:// Is this affected by BaseLV and SPL too??? [Rytech]
						skillratio += -100 + 500 + 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						break;
					case ABC_ABYSS_STRIKE:
						skillratio += -100 + 2650 * skill_lv;
						skillratio += 10 * sstatus->spl;
						if (tstatus->race == RC_DEMON || tstatus->race == RC_ANGEL)
							skillratio += 200 * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case ABC_ABYSS_SQUARE:
						skillratio += -100 + 750 * skill_lv;
						skillratio += 40 * pc_checkskill( sd, ABC_MAGIC_SWORD_M ) * skill_lv;
						skillratio += 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						break;
					case TR_METALIC_FURY:
						skillratio += -100 + 3850 * skill_lv;
						// !Todo: skill affected by SPL (without SC_SOUNDBLEND) as well?
						if (tsc && tsc->getSCE(SC_SOUNDBLEND)) {
							skillratio += 800 * skill_lv;
							skillratio += 2 * pc_checkskill(sd, TR_STAGE_MANNER) * sstatus->spl;
						}
						RE_LVL_DMOD(100);
						break;
					case TR_SOUNDBLEND:
						skillratio += -100 + 120 * skill_lv + 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						if (sc && sc->getSCE(SC_MYSTIC_SYMPHONY)) {
							skillratio += skillratio * 100 / 100;

							if (tstatus->race == RC_FISH || tstatus->race == RC_DEMIHUMAN)
								skillratio += skillratio * 50 / 100;
						}
						break;
					case EM_DIAMOND_STORM:
						skillratio += -100 + 500 + 2400 * skill_lv;
						skillratio += 5 * sstatus->spl;

						if( sc != nullptr && sc->getSCE( SC_SUMMON_ELEMENTAL_DILUVIO ) ){
							skillratio += 7300 + 200 * skill_lv;
							skillratio += 5 * sstatus->spl;
						}

						RE_LVL_DMOD(100);
						break;
					case EM_LIGHTNING_LAND:
						skillratio += -100 + 700 + 1100 * skill_lv;
						skillratio += 5 * sstatus->spl;

						if( sc != nullptr && sc->getSCE( SC_SUMMON_ELEMENTAL_PROCELLA ) ){
							skillratio += 200 * skill_lv;
						}

						RE_LVL_DMOD(100);
						break;
					case EM_VENOM_SWAMP:
						skillratio += -100 + 700 + 1100 * skill_lv;
						skillratio += 5 * sstatus->spl;

						if( sc && sc->getSCE( SC_SUMMON_ELEMENTAL_SERPENS ) ){
							skillratio += 200 * skill_lv;
						}

						RE_LVL_DMOD(100);
						break;
					case EM_CONFLAGRATION:
						skillratio += -100 + 700 + 1100 * skill_lv;
						skillratio += 5 * sstatus->spl;

						if( sc != nullptr && sc->getSCE( SC_SUMMON_ELEMENTAL_ARDOR ) ){
							skillratio += 200 * skill_lv;
						}

						RE_LVL_DMOD(100);
						break;
					case EM_TERRA_DRIVE:
						skillratio += -100 + 500 + 2400 * skill_lv;
						skillratio += 5 * sstatus->spl;

						if( sc != nullptr && sc->getSCE( SC_SUMMON_ELEMENTAL_TERREMOTUS ) ){
							skillratio += 7300 + 200 * skill_lv;
							skillratio += 5 * sstatus->spl;
						}

						RE_LVL_DMOD(100);
						break;
					case ABC_FROM_THE_ABYSS_ATK:
						skillratio += -100 + 150 + 650 * skill_lv;
						skillratio += 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						break;
					case EM_ELEMENTAL_BUSTER_FIRE:
					case EM_ELEMENTAL_BUSTER_WATER:
					case EM_ELEMENTAL_BUSTER_WIND:
					case EM_ELEMENTAL_BUSTER_GROUND:
					case EM_ELEMENTAL_BUSTER_POISON:
						skillratio += -100 + 550 + 2650 * skill_lv;
						skillratio += 10 * sstatus->spl;
						if (tstatus->race == RC_FORMLESS || tstatus->race == RC_DRAGON)
							skillratio += 150 * skill_lv;
						RE_LVL_DMOD(100);
						break;
					case EM_EL_FLAMEROCK:
						skillratio += -100 + 2400;
						if (ed)
							skillratio += skillratio * status_get_lv(&ed->master->bl) / 100;
						break;
					case EM_EL_AGE_OF_ICE:
						skillratio += -100 + 3700;
						if (ed)
							skillratio += skillratio * status_get_lv(&ed->master->bl) / 100;
						break;
					case EM_EL_STORM_WIND:
						skillratio += -100 + 2600;
						if (ed)
							skillratio += skillratio * status_get_lv(&ed->master->bl) / 100;
						break;
					case EM_EL_AVALANCHE:
						skillratio += -100 + 450;
						if (ed)
							skillratio += skillratio * status_get_lv(&ed->master->bl) / 100;
						break;
					case EM_EL_DEADLY_POISON:
						skillratio += -100 + 700;
						if (ed)
							skillratio += skillratio * status_get_lv(&ed->master->bl) / 100;
						break;
					case NPC_RAINOFMETEOR:
						skillratio += 350;	// unknown ratio
						break;
					case HN_NAPALM_VULCAN_STRIKE:
						skillratio += -100 + 350 + 650 * skill_lv;
						skillratio += pc_checkskill(sd, HN_SELFSTUDY_SOCERY) * 4 * skill_lv;
						skillratio += 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						break;
					case HN_JUPITEL_THUNDER_STORM:
						skillratio += -100 + 1800 * skill_lv;
						skillratio += pc_checkskill(sd, HN_SELFSTUDY_SOCERY) * 3 * skill_lv;
						skillratio += 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						break;
					case HN_HELLS_DRIVE:
						skillratio += -100 + 1700 + 900 * skill_lv;
						skillratio += pc_checkskill(sd, HN_SELFSTUDY_SOCERY) * 4 * skill_lv;
						skillratio += 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						break;
					case HN_GROUND_GRAVITATION:
						if (mflag & SKILL_ALTDMG_FLAG) {
							skillratio += -100 + 3000 + 1500 * skill_lv;
							skillratio += pc_checkskill(sd, HN_SELFSTUDY_SOCERY) * 4 * skill_lv;
							ad.div_ = -2;
						} else {
							skillratio += -100 + 800 + 700 * skill_lv;
							skillratio += pc_checkskill(sd, HN_SELFSTUDY_SOCERY) * 2 * skill_lv;
						}
						skillratio += 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						break;
					case HN_JACK_FROST_NOVA:
						if (mflag & SKILL_ALTDMG_FLAG) {
							skillratio += -100 + 200 * skill_lv;
						} else {
							skillratio += -100 + 400 + 500 * skill_lv;
						}
						skillratio += pc_checkskill(sd, HN_SELFSTUDY_SOCERY) * 3 * skill_lv;
						skillratio += 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						break;
					case HN_METEOR_STORM_BUSTER:
						if (mflag & SKILL_ALTDMG_FLAG) {
							skillratio += -100 + 300 + 160 * skill_lv * 2;
							ad.div_ = -3;
						} else {
							skillratio += -100 + 450 + 160 * skill_lv;
						}
						skillratio += pc_checkskill(sd, HN_SELFSTUDY_SOCERY) * 5 * skill_lv;
						skillratio += 5 * sstatus->spl;
						RE_LVL_DMOD(100);
						break;
				}

				if (sc) {// Insignia's increases the damage of offensive magic by a fixed percentage depending on the element.
					if ((sc->getSCE(SC_FIRE_INSIGNIA) && sc->getSCE(SC_FIRE_INSIGNIA)->val1 == 3 && s_ele == ELE_FIRE) ||
						(sc->getSCE(SC_WATER_INSIGNIA) && sc->getSCE(SC_WATER_INSIGNIA)->val1 == 3 && s_ele == ELE_WATER) ||
						(sc->getSCE(SC_WIND_INSIGNIA) && sc->getSCE(SC_WIND_INSIGNIA)->val1 == 3 && s_ele == ELE_WIND) ||
						(sc->getSCE(SC_EARTH_INSIGNIA) && sc->getSCE(SC_EARTH_INSIGNIA)->val1 == 3 && s_ele == ELE_EARTH))
						skillratio += 25;
				}
#ifdef RENEWAL
				// S.MATK needs to be applied before the skill ratio to prevent rounding issues
				if (sd && sstatus->smatk > 0)
					ad.damage += ad.damage * sstatus->smatk / 100;
#endif
				MATK_RATE(skillratio);

				//Constant/misc additions from skills
				if (skill_id == WZ_FIREPILLAR)
					MATK_ADD(100 + 50 * skill_lv);
				break;
			}
		}
#ifdef RENEWAL
		ad.damage += battle_calc_cardfix(BF_MAGIC, src, target, nk, s_ele, 0, ad.damage, 0, ad.flag);
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

#ifdef RENEWAL
		// MRes reduces magical damage by a percentage and
		// is calculated before MDEF and other reductions.
		// TODO: MRes formula probably should be: (2000+x)/(2000+5x), but with the reduction rounded down
		if (ad.damage && tstatus->mres > 0) {
			short mres = tstatus->mres;
			short ignore_mres = 0;// Value used as percentage.

			if (sd)
				ignore_mres += sd->indexed_bonus.ignore_mres_by_race[tstatus->race] + sd->indexed_bonus.ignore_mres_by_race[RC_ALL];

			// Attacker status's that pierce MRes.
			if (sc && sc->getSCE(SC_A_VITA))
				ignore_mres += sc->getSCE(SC_A_VITA)->val2;

			ignore_mres = min(ignore_mres, battle_config.max_res_mres_ignored);

			if (ignore_mres > 0)
				mres -= mres * ignore_mres / 100;

			// Apply damage reduction.
			ad.damage = ad.damage * (5000 + mres) / (5000 + 10 * mres);
		}
#endif

		if(!flag.imdef){
			defType mdef = tstatus->mdef;
			int mdef2= tstatus->mdef2;

			if (sc && sc->getSCE(SC_EXPIATIO)) {
				i = 5 * sc->getSCE(SC_EXPIATIO)->val1; // 5% per level

				i = min(i, 100); //cap it to 100 for 5 mdef min
				mdef -= mdef * i / 100;
				//mdef2 -= mdef2 * i / 100;
			}

			if(sd) {
				i = sd->indexed_bonus.ignore_mdef_by_race[tstatus->race] + sd->indexed_bonus.ignore_mdef_by_race[RC_ALL];
				i += sd->indexed_bonus.ignore_mdef_by_class[tstatus->class_] + sd->indexed_bonus.ignore_mdef_by_class[CLASS_ALL];

				std::vector<e_race2> race2 = status_get_race2(target);

				for (const auto &raceit : race2)
					i += sd->indexed_bonus.ignore_mdef_by_race2[raceit];
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
			if (mdef < 0)
				mdef = 0; // Negative eMDEF is treated as 0 on official

			ad.damage = ad.damage * (1000 + mdef) / (1000 + mdef * 10) - mdef2;
#else
			if(battle_config.magic_defense_type)
				ad.damage = ad.damage - mdef*battle_config.magic_defense_type - mdef2;
			else
				ad.damage = ad.damage * (100-mdef)/100 - mdef2;
#endif
		}

		//Apply the physical part of the skill's damage. [Skotlex]
		switch (skill_id) {
			case CR_GRANDCROSS:
			case NPC_GRANDDARKNESS: {
				// Pre-re ATK = Take atk, apply def reduction and add refine bonus
				// Final Damage = (ATK+MATK)*RATIO
				// Renewal ATK = Take total atk
				// Final Damage = ((ATK+MATK)/2)*RATIO - (tDEF + tMDEF)
				// No need to go through the whole physical damage code
				struct Damage wd = initialize_weapon_data(src, target, skill_id, skill_lv, mflag);
				battle_calc_skill_base_damage(&wd, src, target, skill_id, skill_lv);
				// Calculate ATK
#ifdef RENEWAL
				if (sd)
					wd.damage = wd.statusAtk + wd.weaponAtk + wd.equipAtk + wd.percentAtk;
#else
				battle_calc_defense_reduction(&wd, src, target, skill_id, skill_lv);
				if (sd) {
					wd.damage += sstatus->rhw.atk2;
				}
#endif
				// Combine ATK and MATK
#ifdef RENEWAL
				ad.damage = (wd.damage + ad.damage) / 2;
#else
				ad.damage = std::max((int64)1, wd.damage + ad.damage);
#endif
				// Ratio
				skillratio += 40 * skill_lv;
				MATK_RATE(skillratio);
#ifdef RENEWAL
				// Total defense reduction (renewal only)
				battle_calc_defense_reduction(&ad, src, target, skill_id, skill_lv);
				ad.damage -= (tstatus->mdef + tstatus->mdef2);
#endif
			}
			break;
		}

		if(ad.damage<1)
			ad.damage=1;
		else if(sc) { //only applies when hit
			switch(skill_id) {
				case MG_LIGHTNINGBOLT:
				case MG_THUNDERSTORM:
					if(sc->getSCE(SC_GUST_OPTION))
						ad.damage += (6 + sstatus->int_ / 4) + max(sstatus->dex - 10, 0) / 30;
					break;
				case MG_FIREBOLT:
				case MG_FIREWALL:
					if(sc->getSCE(SC_PYROTECHNIC_OPTION))
						ad.damage += (6 + sstatus->int_ / 4) + max(sstatus->dex - 10, 0) / 30;
					break;
				case MG_COLDBOLT:
				case MG_FROSTDIVER:
					if(sc->getSCE(SC_AQUAPLAY_OPTION))
						ad.damage += (6 + sstatus->int_ / 4) + max(sstatus->dex - 10, 0) / 30;
					break;
				case WZ_EARTHSPIKE:
				case WZ_HEAVENDRIVE:
					if(sc->getSCE(SC_PETROLOGY_OPTION))
						ad.damage += (6 + sstatus->int_ / 4) + max(sstatus->dex - 10, 0) / 30;
					break;
			}
		}

		if (!nk[NK_IGNOREELEMENT])
			ad.damage = battle_attr_fix(src, target, ad.damage, s_ele, tstatus->def_ele, tstatus->ele_lv);

#ifndef RENEWAL
		ad.damage += battle_calc_cardfix(BF_MAGIC, src, target, nk, s_ele, 0, ad.damage, 0, ad.flag);
#endif

		switch (skill_id) {
			case CR_GRANDCROSS:
			case NPC_GRANDDARKNESS:
				if (src == target) {
					// Grand Cross on self first applies attr_fix, then cardfix and finally halves the damage
					if (src->type == BL_PC)
						ad.damage = ad.damage / 2;
					else
						ad.damage = 0;
				}
				else
					// Grand Cross on target applies attr_fix, then cardfix and then attr_fix a second time
					ad.damage = battle_attr_fix(src, target, ad.damage, s_ele, tstatus->def_ele, tstatus->ele_lv);
				break;
		}

	} //Hint: Against plants damage will still be 1 at this point

	//Apply DAMAGE_DIV_FIX and check for min damage
	battle_apply_div_fix(&ad, skill_id);

	struct map_data *mapdata = map_getmapdata(target->m);

	ad.damage = battle_calc_damage(src,target,&ad,ad.damage,skill_id,skill_lv);
	if (mapdata_flag_gvg2(mapdata))
		ad.damage = battle_calc_gvg_damage(src,target,ad.damage,skill_id,ad.flag);
	else if (mapdata->getMapFlag(MF_BATTLEGROUND))
		ad.damage = battle_calc_bg_damage(src,target,ad.damage,skill_id,ad.flag);

	// Skill damage adjustment
	if ((skill_damage = battle_skill_damage(src,target,skill_id)) != 0)
		MATK_ADDRATE(skill_damage);

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
	int skill_damage = 0;
	short i, s_ele;

	map_session_data *sd, *tsd;
	struct Damage md; //DO NOT CONFUSE with md of mob_data!
	struct status_data *sstatus = status_get_status_data(src);
	struct status_data *tstatus = status_get_status_data(target);
	status_change *ssc = status_get_sc(src);

	memset(&md,0,sizeof(md));

	if (src == nullptr || target == nullptr) {
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
	md.miscflag = mflag;

	std::shared_ptr<s_skill_db> skill = skill_db.find(skill_id);
	std::bitset<NK_MAX> nk;

	if (skill)
		nk = skill->nk;

	sd = BL_CAST(BL_PC, src);
	tsd = BL_CAST(BL_PC, target);

	if(sd) {
		sd->state.arrow_atk = 0;
		md.blewcount += battle_blewcount_bonus(sd, skill_id);
	}

	s_ele = battle_get_misc_element(src, target, skill_id, skill_lv, mflag);

	//Skill Range Criteria
	md.flag |= battle_range_type(src, target, skill_id, skill_lv);

	switch (skill_id) {
		case TF_THROWSTONE:
			if (sd)
				md.damage = 50;
			else
				md.damage = 30;
			md.flag |= BF_WEAPON;
			break;
		case NPC_KILLING_AURA:
			md.damage = 10000;
			break;
#ifdef RENEWAL
		case HT_LANDMINE:
		case MA_LANDMINE:
		case HT_BLASTMINE:
		case HT_CLAYMORETRAP:
			md.damage = (int64)(skill_lv * sstatus->dex * (3.0 + (float)status_get_lv(src) / 100.0) * (1.0 + (float)sstatus->int_ / 35.0));
			md.damage += md.damage * (rnd()%20 - 10) / 100;
			md.damage += (sd ? pc_checkskill(sd,RA_RESEARCHTRAP) * 40 : 0);
			break;
#else
		case HT_LANDMINE:
		case MA_LANDMINE:
			md.damage = static_cast<decltype(md.damage)>(skill_lv * (sstatus->dex + 75.0) * (100.0 + sstatus->int_) / 100.0);
			break;
		case HT_BLASTMINE:
			md.damage = static_cast<decltype(md.damage)>(skill_lv * (sstatus->dex / 2.0 + 50.0) * (100.0 + sstatus->int_) / 100.0);
			break;
		case HT_CLAYMORETRAP:
			md.damage = static_cast<decltype(md.damage)>(skill_lv * (sstatus->dex / 2.0 + 75.0) * (100.0 + sstatus->int_) / 100.0);
			break;
#endif
		case HT_BLITZBEAT:
		case SN_FALCONASSAULT:
			{
				uint16 skill;

				//Blitz-beat Damage
				if(!sd || !(skill = pc_checkskill(sd,HT_STEELCROW)))
					skill = 0;
#ifdef RENEWAL
				md.damage = (sstatus->dex / 10 + sstatus->agi / 2 + skill * 3 + 40) * 2;
				RE_LVL_MDMOD(100);
#else
				md.damage = (sstatus->dex / 10 + sstatus->int_ / 2 + skill * 3 + 40) * 2;
				if(mflag > 1) //Autocasted Blitz
					nk.set(NK_SPLASHSPLIT);
#endif
				if (skill_id == SN_FALCONASSAULT) {
					//Div fix of Blitzbeat
					DAMAGE_DIV_FIX2(md.damage, skill_get_num(HT_BLITZBEAT, 5));
					//Falcon Assault Modifier
					md.damage = md.damage * (150 + 70 * skill_lv) / 100;
				}
			}
			break;
#ifndef RENEWAL
		case BA_DISSONANCE:
			md.damage = 30 + skill_lv * 10;
			if (sd)
				md.damage += 3 * pc_checkskill(sd,BA_MUSICALLESSON);
			break;
#endif
		case NPC_SELFDESTRUCTION:
			md.damage = sstatus->hp;
			break;
		case NPC_SMOKING:
			md.damage = 3;
			break;
		case NPC_EVILLAND:
			md.damage = skill_calc_heal(src,target,skill_id,skill_lv,false);
			break;
#ifndef RENEWAL
		case ASC_BREAKER:
			md.damage = 500 + rnd()%500 + 5 * skill_lv * sstatus->int_;
			nk.set(NK_IGNOREFLEE);
			nk.set(NK_IGNOREELEMENT); //These two are not properties of the weapon based part.
			break;
		case HW_GRAVITATION:
			md.damage = 200 + 200 * skill_lv;
			md.dmotion = 0; //No flinch animation
			break;
		case PA_PRESSURE:
			md.damage = 500 + 300 * skill_lv;
			break;
#endif
		case PA_GOSPEL:
			if (mflag > 0)
				md.damage = (rnd() % 4000) + 1500;
			else {
				md.damage = (rnd() % 5000) + 3000;
#ifdef RENEWAL
				md.damage -= (int64)status_get_def(target);
#else
				md.damage -= (md.damage * (int64)status_get_def(target)) / 100;
#endif
				md.damage -= tstatus->def2;
				if (md.damage < 0)
					md.damage = 0;
			}
			break;
		case GN_FIRE_EXPANSION_ACID:
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
			// Fall through
#else
		case CR_ACIDDEMONSTRATION:
			if(tstatus->vit+sstatus->int_) //crash fix
				md.damage = (int)((int64)7*tstatus->vit*sstatus->int_*sstatus->int_ / (10*(tstatus->vit+sstatus->int_)));
			else
				md.damage = 0;
			if (tsd)
				md.damage /= 2;
#endif
			break;
		case NJ_ZENYNAGE:
			md.damage = skill_get_zeny( skill_id, skill_lv );

			if( md.damage == 0 ){
				md.damage = 2;
			}

			md.damage += rnd_value( static_cast<decltype(md.damage)>( 0 ), md.damage );

			// Specific to Boss Class
			if( status_get_class_( target ) == CLASS_BOSS ){
				md.damage /= 3;
			}

			if( tsd != nullptr ){
				md.damage /= 2;
			}
			break;
		case KO_MUCHANAGE:
			md.damage = skill_get_zeny( skill_id, skill_lv );

			if( md.damage == 0 ){
				md.damage = 10;
			}

			md.damage = rnd_value( md.damage / 2, md.damage );

			if( pc_checkskill( sd, NJ_TOBIDOUGU ) == 0 ){
				md.damage /= 2;
			}

			// Specific to Boss Class
			if( status_get_class_( target ) == CLASS_BOSS ){
				md.damage /= 2;
			}
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
				status_change *sc = status_get_sc(src);

				md.damage = (int64)sstatus->hp + (atk.damage * (int64)sstatus->hp * skill_lv) / (int64)sstatus->max_hp;

				if (sc && sc->getSCE(SC_BUNSINJYUTSU) && (i = sc->getSCE(SC_BUNSINJYUTSU)->val2) > 0) { // mirror image bonus only occurs if active
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
			nk.set(NK_IGNOREELEMENT);
			nk.set(NK_IGNOREFLEE);
			nk.set(NK_IGNOREDEFCARD);
			break;
		case NC_MAGMA_ERUPTION_DOTDAMAGE: // 'Eruption' damage
			md.damage = 800 + 200 * skill_lv;
			break;
		case NPC_MAGMA_ERUPTION_DOTDAMAGE:
			md.damage = 1000 * skill_lv;
			break;
		case GN_THORNS_TRAP:
			md.damage = 100 + 200 * skill_lv + status_get_int(src);
			break;
		case RL_B_TRAP:
			// kRO 2014-02-12: Damage: Caster's DEX, Target's current HP, Skill Level
			md.damage = status_get_dex(src) * 10 + (skill_lv * 3 * status_get_hp(target)) / 100;
			if (status_bl_has_mode(target, MD_STATUSIMMUNE))
				md.damage /= 10;
			break;
		case NPC_WIDESUCK:
			md.damage = tstatus->max_hp * 15 / 100;
			break;
		case SU_SV_ROOTTWIST_ATK:
			md.damage = 100;
			break;
		case SP_SOULEXPLOSION:
			md.damage = tstatus->hp * (20 + 10 * skill_lv) / 100;
			break;
		case SJ_NOVAEXPLOSING:
			// (Base ATK + Weapon ATK) * Ratio
			md.damage = (sstatus->batk + sstatus->rhw.atk) * (200 + 100 * skill_lv) / 100;

			// Additional Damage
			md.damage += sstatus->max_hp / (6 - min(5, skill_lv)) + status_get_max_sp(src) * (2 * skill_lv);
			break;
		case NPC_CANE_OF_EVIL_EYE:
			md.damage = 15000;
			break;
	}

	if (nk[NK_SPLASHSPLIT]) { // Divide ATK among targets
		if(mflag > 0)
			md.damage /= mflag;
		else
			ShowError("0 enemies targeted by %d:%s, divide per 0 avoided!\n", skill_id, skill_get_name(skill_id));
	}

	if (!nk[NK_IGNOREFLEE]) {
		status_change *sc = status_get_sc(target);

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

	md.damage += battle_calc_cardfix(BF_MISC, src, target, nk, s_ele, 0, md.damage, 0, md.flag);

	if (sd && (i = pc_skillatk_bonus(sd, skill_id)))
		md.damage += (int64)md.damage*i/100;

	if (tsd && (i = pc_sub_skillatk_bonus(tsd, skill_id)))
		md.damage -= (int64)md.damage*i/100;

	if(!nk[NK_IGNOREELEMENT])
		md.damage=battle_attr_fix(src, target, md.damage, s_ele, tstatus->def_ele, tstatus->ele_lv);

	//Plant damage
	if(md.damage < 0)
		md.damage = 0;
	else if(md.damage && is_infinite_defense(target, md.flag)) {
		md.damage = 1;
	}

	//Apply DAMAGE_DIV_FIX and check for min damage
	battle_apply_div_fix(&md, skill_id);

	switch(skill_id) {
		case RA_FIRINGTRAP:
 		case RA_ICEBOUNDTRAP:
			if (md.damage == 1)
				break;
			[[fallthrough]];
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
				pc_payzeny( sd, static_cast<int32>( md.damage ), LOG_TYPE_CONSUME );
			}
			break;
	}

	struct map_data *mapdata = map_getmapdata(target->m);

	md.damage = battle_calc_damage(src,target,&md,md.damage,skill_id,skill_lv);
	if(mapdata_flag_gvg2(mapdata))
		md.damage = battle_calc_gvg_damage(src,target,md.damage,skill_id,md.flag);
	else if(mapdata->getMapFlag(MF_BATTLEGROUND))
		md.damage = battle_calc_bg_damage(src,target,md.damage,skill_id,md.flag);

	// Skill damage adjustment
	if ((skill_damage = battle_skill_damage(src,target,skill_id)) != 0)
		md.damage += (int64)md.damage * skill_damage / 100;

	battle_absorb_damage(target, &md);

	battle_do_reflect(BF_MISC,&md, src, target, skill_id, skill_lv); //WIP [lighta]

	return md;
}

/**
 * Calculate vanish damage on a target
 * @param sd: Player with vanish item
 * @param target: Target to vanish HP/SP
 * @param flag: Damage struct battle flag
 */
void battle_vanish_damage(map_session_data *sd, struct block_list *target, int flag)
{
	nullpo_retv(sd);
	nullpo_retv(target);

	// bHPVanishRate
	int16 vanish_hp = 0;
	if (!sd->hp_vanish.empty()) {
		for (auto &it : sd->hp_vanish) {
			if (!(((it.flag)&flag)&BF_WEAPONMASK &&
				((it.flag)&flag)&BF_RANGEMASK &&
				((it.flag)&flag)&BF_SKILLMASK))
				continue;
			if (it.rate && (it.rate >= 1000 || rnd() % 1000 < it.rate))
				vanish_hp += it.per;
		}
	}

	// bSPVanishRate
	int16 vanish_sp = 0;
	if (!sd->sp_vanish.empty()) {
		for (auto &it : sd->sp_vanish) {
			if (!(((it.flag)&flag)&BF_WEAPONMASK &&
				((it.flag)&flag)&BF_RANGEMASK &&
				((it.flag)&flag)&BF_SKILLMASK))
				continue;
			if (it.rate && (it.rate >= 1000 || rnd() % 1000 < it.rate))
				vanish_sp += it.per;
		}
	}

	if (vanish_hp > 0 || vanish_sp > 0)
		status_percent_damage(&sd->bl, target, -vanish_hp, -vanish_sp, false); // Damage HP/SP applied once
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
		if(bl->type == BL_PC)
			d.div_ = 1;
		
		status_change *tsc = status_get_sc(target);

		// Weapon Blocking has the ability to trigger on ATK_MISS as well.
		if (tsc != nullptr && tsc->getSCE(SC_WEAPONBLOCKING)) {
			status_change_entry *tsce = tsc->getSCE(SC_WEAPONBLOCKING);

			if (attack_type == BF_WEAPON && rnd() % 100 < tsce->val2) {
				clif_skill_nodamage(target, bl, GC_WEAPONBLOCKING, tsce->val1, 1);
				sc_start(bl, target, SC_WEAPONBLOCK_ON, 100, bl->id, skill_get_time2(GC_WEAPONBLOCKING, tsce->val1));
			}
		}
	}
	else // Some skills like Weaponry Research will cause damage even if attack is dodged
		d.dmg_lv = ATK_DEF;

	map_session_data *sd = BL_CAST(BL_PC, bl);

	if (sd && d.damage + d.damage2 > 1)
		battle_vanish_damage(sd, target, d.flag);

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
int64 battle_calc_return_damage(struct block_list* tbl, struct block_list *src, int64 *dmg, int flag, uint16 skill_id, bool status_reflect){
	status_change *tsc = status_get_sc(tbl);

	if (tsc) { // These statuses do not reflect any damage (off the target)
		if (tsc->getSCE(SC_WHITEIMPRISON) || tsc->getSCE(SC_DARKCROW) || tsc->getSCE(SC_KYOMU))
			return 0;
	}

	status_change *sc = status_get_sc(src);

	if (sc) {
		if (skill_id == GN_HELLS_PLANT_ATK && sc->getSCE(SC_HELLS_PLANT))
			return 0;
	}

	map_session_data *tsd = BL_CAST(BL_PC, tbl);
	int64 rdamage = 0, damage = *dmg;

	if (flag & BF_SHORT) {//Bounces back part of the damage.
		if ( (skill_get_inf2(skill_id, INF2_ISTRAP) || !status_reflect) && tsd && tsd->bonus.short_weapon_damage_return ) {
			rdamage += damage * tsd->bonus.short_weapon_damage_return / 100;
		} else if( status_reflect && tsc && tsc->count ) {
			if( tsc->getSCE(SC_REFLECTSHIELD) ) {
				status_change_entry *sce_d;
				block_list *d_bl;

				if( (sce_d = tsc->getSCE(SC_DEVOTION)) && (d_bl = map_id2bl(sce_d->val1)) &&
					((d_bl->type == BL_MER && ((TBL_MER*)d_bl)->master && ((TBL_MER*)d_bl)->master->bl.id == tbl->id) ||
					(d_bl->type == BL_PC && ((TBL_PC*)d_bl)->devotion[sce_d->val2] == tbl->id)) )
				{ //Don't reflect non-skill attack if has SC_REFLECTSHIELD from Devotion bonus inheritance
					if( (!skill_id && battle_config.devotion_rdamage_skill_only && tsc->getSCE(SC_REFLECTSHIELD)->val4) ||
						!check_distance_bl(tbl,d_bl,sce_d->val3) )
						return 0;
				}
			}
			if ( tsc->getSCE(SC_REFLECTSHIELD) && skill_id != WS_CARTTERMINATION && skill_id != NPC_MAXPAIN_ATK ) {
				// Don't reflect non-skill attack if has SC_REFLECTSHIELD from Devotion bonus inheritance
				if (!skill_id && battle_config.devotion_rdamage_skill_only && tsc->getSCE(SC_REFLECTSHIELD)->val4)
					rdamage = 0;
				else {
					rdamage += damage * tsc->getSCE(SC_REFLECTSHIELD)->val2 / 100;
				}
			}

			if (tsc->getSCE(SC_DEATHBOUND) && skill_id != WS_CARTTERMINATION && skill_id != GN_HELLS_PLANT_ATK && !status_bl_has_mode(src,MD_STATUSIMMUNE)) {
				if (distance_bl(src,tbl) <= 0 || !map_check_dir(map_calc_dir(tbl,src->x,src->y), unit_getdir(tbl))) {
					int64 rd1 = i64min(damage, status_get_max_hp(tbl)) * tsc->getSCE(SC_DEATHBOUND)->val2 / 100; // Amplify damage.

					*dmg = rd1 * 30 / 100; // Received damage = 30% of amplified damage.
					clif_skill_damage(src, tbl, gettick(), status_get_amotion(src), 0, -30000, 1, RK_DEATHBOUND, tsc->getSCE(SC_DEATHBOUND)->val1, DMG_SINGLE);
					skill_blown(tbl, src, skill_get_blewcount(RK_DEATHBOUND, tsc->getSCE(SC_DEATHBOUND)->val1), unit_getdir(src), BLOWN_NONE);
					status_change_end(tbl, SC_DEATHBOUND);
					rdamage += rd1 * 70 / 100; // Target receives 70% of the amplified damage. [Rytech]
				}
			}
		}
	} else {
		if (!status_reflect && tsd && tsd->bonus.long_weapon_damage_return) {
			rdamage += damage * tsd->bonus.long_weapon_damage_return / 100;
		}
	}

	// Config damage adjustment
	map_data *mapdata = map_getmapdata(src->m);

	if (mapdata_flag_gvg2(mapdata))
		rdamage = battle_calc_gvg_damage(src, tbl, rdamage, skill_id, flag);
	else if (mapdata->getMapFlag(MF_BATTLEGROUND))
		rdamage = battle_calc_bg_damage(src, tbl, rdamage, skill_id, flag);
	else if (mapdata->getMapFlag(MF_PVP))
		rdamage = battle_calc_pk_damage(*src, *tbl, rdamage, skill_id, flag);

	// Skill damage adjustment
	int skill_damage = battle_skill_damage(src, tbl, skill_id);

	if (skill_damage != 0) {
		rdamage += rdamage * skill_damage / 100;
	}

	int64 reduce = 0;
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd && sd->bonus.reduce_damage_return != 0) {
		reduce += (sd->bonus.reduce_damage_return);
	}

	if (sc) {
		if (status_reflect && sc->getSCE(SC_REFLECTDAMAGE)) {
			reduce += sc->getSCE(SC_REFLECTDAMAGE)->val2;
		}
		if (sc->getSCE(SC_VENOMBLEED) && sc->getSCE(SC_VENOMBLEED)->val3 == 0) {
			reduce += sc->getSCE(SC_VENOMBLEED)->val2;
		}
		if (sc->getSCE(SC_REF_T_POTION))
			reduce += 100;
	}

	if (rdamage > 0) {
		rdamage -= rdamage * i64min(100, reduce) / 100;
		rdamage = i64max(rdamage, 1);
	}

	if (rdamage == 0)
		return 0; // No reflecting damage calculated.
	else
		return cap_value(rdamage, 1, status_get_max_hp(tbl));
}

/** Check for Coma damage
 * @param sd: Source player
 * @param bl: Target
 * @param attack_type: Attack type
 * @return True if Coma applies, false if Coma does not apply
 */
bool battle_check_coma(map_session_data& sd, struct block_list& target, e_battle_flag attack_type)
{
	struct status_data* tstatus = status_get_status_data(&target);
	mob_data* dstmd = BL_CAST(BL_MOB, &target);

	// Coma
	if (sd.special_state.bonus_coma && (!dstmd || (!util::vector_exists(status_get_race2(&dstmd->bl), RC2_GVG) && status_get_class(&dstmd->bl) != CLASS_BATTLEFIELD))) {
		int rate = 0;
		rate += sd.indexed_bonus.coma_class[tstatus->class_] + sd.indexed_bonus.coma_class[CLASS_ALL];
		if(!status_bl_has_mode(&target, MD_STATUSIMMUNE))
			rate += sd.indexed_bonus.coma_race[tstatus->race] + sd.indexed_bonus.coma_race[RC_ALL];
		if (attack_type&BF_WEAPON) {
			rate += sd.indexed_bonus.weapon_coma_class[tstatus->class_] + sd.indexed_bonus.weapon_coma_class[CLASS_ALL];
			if (!status_bl_has_mode(&target, MD_STATUSIMMUNE)) {
				rate += sd.indexed_bonus.weapon_coma_ele[tstatus->def_ele] + sd.indexed_bonus.weapon_coma_ele[ELE_ALL];
				rate += sd.indexed_bonus.weapon_coma_race[tstatus->race] + sd.indexed_bonus.weapon_coma_race[RC_ALL];
			}
		}
		if (rate > 0 && rnd_chance(rate, 10000))
			return true;
	}
	return false;
}

/**
 * Calculate Vellum damage on a target
 * @param sd: Player with vanish item
 * @param target: Target to vanish HP/SP
 * @param wd: Damage struct reference
 * @return True on damage done or false if not
 */
bool battle_vellum_damage(map_session_data *sd, struct block_list *target, struct Damage *wd)
{
	nullpo_retr(false, sd);
	nullpo_retr(false, target);
	nullpo_retr(false, wd);

	struct status_data *tstatus = status_get_status_data(target);
	// bHPVanishRaceRate
	int16 vellum_rate_hp = cap_value(sd->hp_vanish_race[tstatus->race].rate + sd->hp_vanish_race[RC_ALL].rate, 0, INT16_MAX);
	int8 vellum_hp = cap_value(sd->hp_vanish_race[tstatus->race].per + sd->hp_vanish_race[RC_ALL].per, INT8_MIN, INT8_MAX);
	// bSPVanishRaceRate
	int16 vellum_rate_sp = cap_value(sd->sp_vanish_race[tstatus->race].rate + sd->sp_vanish_race[RC_ALL].rate, 0, INT16_MAX);
	int8 vellum_sp = cap_value(sd->sp_vanish_race[tstatus->race].per + sd->sp_vanish_race[RC_ALL].per, INT8_MIN, INT8_MAX);

	// The HP and SP damage bonus from these items don't stack because of the special damage display for SP.
	// Vellum damage overrides any other damage done as well.
	if (vellum_hp && vellum_rate_hp && (vellum_rate_hp >= 1000 || rnd() % 1000 < vellum_rate_hp)) {
		wd->damage = apply_rate(tstatus->max_hp, vellum_hp);
		wd->damage2 = 0;
	} else if (vellum_sp && vellum_rate_sp && (vellum_rate_sp >= 1000 || rnd() % 1000 < vellum_rate_sp)) {
		wd->damage = apply_rate(tstatus->max_sp, vellum_sp);
		wd->damage2 = 0;
		wd->isspdamage = true;
	} else
		return false;

	return true;
}

/*===========================================
 * Perform battle drain effects (HP/SP loss)
 *-------------------------------------------*/
void battle_drain(map_session_data *sd, struct block_list *tbl, int64 rdamage, int64 ldamage, int race, int class_)
{
	struct weapon_data *wd;
	int64 *damage;
	int thp = 0, // HP gained
		tsp = 0, // SP gained
		//rhp = 0, // HP reduced from target
		//rsp = 0, // SP reduced from target
		hp = 0, sp = 0;

	if (!CHK_RACE(race) && !CHK_CLASS(class_))
		return;

	for (int i = 0; i < 4; i++) {
		//First two iterations: Right hand
		if (i < 2) {
			wd = &sd->right_weapon;
			damage = &rdamage;
		} else {
			wd = &sd->left_weapon;
			damage = &ldamage;
		}

		if (*damage <= 0)
			continue;

		if (i == 1 || i == 3) {
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
	t_tick tick;
	int64 damage;
	int amotion, dmotion;
	struct block_list *src;

	nullpo_ret(bl);

	tick = va_arg(ap, t_tick);
	src = va_arg(ap,struct block_list *);
	amotion = va_arg(ap,int);
	dmotion = va_arg(ap,int);
	damage = va_arg(ap,int);

	if (status_bl_has_mode(bl, MD_SKILLIMMUNE) || status_get_class(bl) == MOBID_EMPERIUM)
		return 0;
	if( bl != src && battle_check_target(src,bl,BCT_ENEMY) > 0 ) {
		map_freeblock_lock();
		if( src->type == BL_PC )
			battle_drain((TBL_PC*)src, bl, damage, damage, status_get_race(bl), status_get_class_(bl));
		if( amotion )
			battle_delay_damage(tick, amotion,src,bl,0,CR_REFLECTSHIELD,0,damage,ATK_DEF,0,true,false);
		else
			battle_fix_damage(src,bl,damage,0,LG_REFLECTDAMAGE);
		clif_damage(bl,bl,tick,amotion,dmotion,damage,1,DMG_ENDURE,0,false);
		skill_additional_effect(src, bl, CR_REFLECTSHIELD, 1, BF_WEAPON|BF_SHORT|BF_NORMAL,ATK_DEF,tick);
		map_freeblock_unlock();
	}

	return 0;
}
/**
 * Triggers aftercast delay for autocasted skills.
 * @param src: Source data
 * @param skill_id: Skill used
 * @param skill_lv: Skill level used
 * @param tick: Server tick
 */
void battle_autocast_aftercast(struct block_list* src, uint16 skill_id, uint16 skill_lv, t_tick tick)
{
	unit_data *ud = unit_bl2ud(src);

	if (ud) {
		int autocast_tick = skill_delayfix(src, skill_id, skill_lv);

		if (DIFF_TICK(ud->canact_tick, tick + autocast_tick) < 0) {
			ud->canact_tick = i64max(tick + autocast_tick, ud->canact_tick);

			if (battle_config.display_status_timers && src->type == BL_PC)
				clif_status_change(src, EFST_POSTDELAY, 1, autocast_tick, 0, 0, 0);
		}
	}
}

/**
 * Triggers autocasted skills from super elemental supportive buffs.
 * @param sd: Player data
 * @param target: Target data
 * @param skill_id: Skill used
 * @param tick: Server tick
 * @param flag: Special skill flags
 */
void battle_autocast_elembuff_skill(map_session_data* sd, struct block_list* target, uint16 skill_id, t_tick tick, int flag)
{
	uint16 skill_lv = pc_checkskill(sd, skill_id);

	skill_lv = max(1, skill_lv);

	sd->state.autocast = 1;
	if (status_charge(&sd->bl, 0, skill_get_sp(skill_id, skill_lv))) {
		skill_castend_damage_id(&sd->bl, target, skill_id, skill_lv, tick, flag);
		battle_autocast_aftercast(&sd->bl, skill_id, skill_lv, tick);
	}
	sd->state.autocast = 0;
}

/*==========================================
 * Do a basic physical attack (call through unit_attack_timer)
 *------------------------------------------*/
enum damage_lv battle_weapon_attack(struct block_list* src, struct block_list* target, t_tick tick, int flag) {
	map_session_data *sd = nullptr, *tsd = nullptr;
	struct status_data *sstatus, *tstatus;
	status_change *sc, *tsc;
	int64 damage;
	int skillv;
	struct Damage wd;
	bool vellum_damage = false;

	nullpo_retr(ATK_NONE, src);
	nullpo_retr(ATK_NONE, target);

	if (src->prev == nullptr || target->prev == nullptr)
		return ATK_NONE;

	sd = BL_CAST(BL_PC, src);
	tsd = BL_CAST(BL_PC, target);

	sstatus = status_get_status_data(src);
	tstatus = status_get_status_data(target);

	sc = status_get_sc(src);
	tsc = status_get_sc(target);

	if (sc && !sc->count) //Avoid sc checks when there's none to check for. [Skotlex]
		sc = nullptr;
	if (tsc && !tsc->count)
		tsc = nullptr;

	if (sd)
	{
		sd->state.arrow_atk = (sd->status.weapon == W_BOW || (sd->status.weapon >= W_REVOLVER && sd->status.weapon <= W_GRENADE));
		if (sd->state.arrow_atk)
		{
			short index = sd->equip_index[EQI_AMMO];
			if (index < 0) {
				if (sd->weapontype1 > W_KATAR && sd->weapontype1 < W_HUUMA)
					clif_skill_fail( *sd, 0, USESKILL_FAIL_NEED_MORE_BULLET );
				else
					clif_arrow_fail( *sd, ARROWFAIL_NO_AMMO );
				return ATK_NONE;
			}
			//Ammo check by Ishizu-chan
			if (sd->inventory_data[index]) {
				switch (sd->status.weapon) {
					case W_BOW:
						if (sd->inventory_data[index]->subtype != AMMO_ARROW) {
							clif_arrow_fail( *sd, ARROWFAIL_NO_AMMO );
							return ATK_NONE;
						}
						break;
					case W_REVOLVER:
					case W_RIFLE:
					case W_GATLING:
					case W_SHOTGUN:
						if (sd->inventory_data[index]->subtype != AMMO_BULLET) {
							clif_skill_fail( *sd, 0, USESKILL_FAIL_NEED_MORE_BULLET );
							return ATK_NONE;
						}
						break;
					case W_GRENADE:
						if (sd->inventory_data[index]->subtype !=
#ifdef RENEWAL
							AMMO_BULLET) {
#else
							AMMO_GRENADE) {
#endif
							clif_skill_fail( *sd, 0, USESKILL_FAIL_NEED_MORE_BULLET );
							return ATK_NONE;
						}
						break;
				}
			}
		}
	}
	if (sc && sc->count) {
		if (sc->getSCE(SC_CLOAKING) && !(sc->getSCE(SC_CLOAKING)->val4 & 2))
			status_change_end(src, SC_CLOAKING);
		else if (sc->getSCE(SC_CLOAKINGEXCEED) && !(sc->getSCE(SC_CLOAKINGEXCEED)->val4 & 2))
			status_change_end(src, SC_CLOAKINGEXCEED);
		else if (sc->getSCE(SC_NEWMOON) && --(sc->getSCE(SC_NEWMOON)->val2) <= 0)
			status_change_end(src, SC_NEWMOON);
	}
	if (tsc && tsc->getSCE(SC_AUTOCOUNTER) && status_check_skilluse(target, src, KN_AUTOCOUNTER, 1)) {
		uint8 dir = map_calc_dir(target,src->x,src->y);
		int t_dir = unit_getdir(target);
		int dist = distance_bl(src, target);

		if (dist <= 0 || (!map_check_dir(dir,t_dir) && dist <= tstatus->rhw.range+1)) {
			uint16 skill_lv = tsc->getSCE(SC_AUTOCOUNTER)->val1;

			clif_skillcastcancel( *target ); //Remove the casting bar. [Skotlex]
			clif_damage(src, target, tick, sstatus->amotion, 1, 0, 1, DMG_NORMAL, 0, false); //Display MISS.
			status_change_end(target, SC_AUTOCOUNTER);
			skill_attack(BF_WEAPON,target,target,src,KN_AUTOCOUNTER,skill_lv,tick,0);
			return ATK_BLOCK;
		}
	}

	if( tsc && tsc->getSCE(SC_BLADESTOP_WAIT) &&
#ifndef RENEWAL
		status_get_class_(src) != CLASS_BOSS &&
#endif
		(src->type == BL_PC || tsd == nullptr || distance_bl(src, target) <= (tsd->status.weapon == W_FIST ? 1 : 2)) )
	{
		uint16 skill_lv = tsc->getSCE(SC_BLADESTOP_WAIT)->val1;
		int duration = skill_get_time2(MO_BLADESTOP,skill_lv);

#ifdef RENEWAL
			if (status_get_class_(src) == CLASS_BOSS)
				duration = 2000; // Only lasts 2 seconds for Boss monsters
#endif
		status_change_end(target, SC_BLADESTOP_WAIT);
		if(sc_start4(src,src, SC_BLADESTOP, 100, sd?pc_checkskill(sd, MO_BLADESTOP):5, 0, 0, target->id, duration))
		{	//Target locked.
			clif_damage(src, target, tick, sstatus->amotion, 1, 0, 1, DMG_NORMAL, 0, false); //Display MISS.
			clif_bladestop(target, src->id, 1);
			sc_start4(src,target, SC_BLADESTOP, 100, skill_lv, 0, 0, src->id, duration);
			return ATK_BLOCK;
		}
	}

	if(sd && (skillv = pc_checkskill(sd,MO_TRIPLEATTACK)) > 0) {
#ifdef RENEWAL
		int triple_rate = 30; //Base Rate
#else
		int triple_rate = 30 - skillv; //Base Rate
#endif

		if (sc && sc->getSCE(SC_SKILLRATE_UP) && sc->getSCE(SC_SKILLRATE_UP)->val1 == MO_TRIPLEATTACK) {
			triple_rate+= triple_rate*(sc->getSCE(SC_SKILLRATE_UP)->val2)/100;
			status_change_end(src, SC_SKILLRATE_UP);
		}
		if (rnd()%100 < triple_rate) {
			//Need to apply canact_tick here because it doesn't go through skill_castend_id
			sd->ud.canact_tick = i64max(tick + skill_delayfix(src, MO_TRIPLEATTACK, skillv), sd->ud.canact_tick);
			if( skill_attack(BF_WEAPON,src,src,target,MO_TRIPLEATTACK,skillv,tick,0) )
				return ATK_DEF;
			return ATK_MISS;
		}
	}

	if (sc) {
		if (sc->getSCE(SC_SACRIFICE)) {
			uint16 skill_lv = sc->getSCE(SC_SACRIFICE)->val1;
			damage_lv ret_val;

			if( --sc->getSCE(SC_SACRIFICE)->val2 <= 0 )
				status_change_end(src, SC_SACRIFICE);

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
		if (sc->getSCE(SC_MAGICALATTACK)) {
			if( skill_attack(BF_MAGIC,src,src,target,NPC_MAGICALATTACK,sc->getSCE(SC_MAGICALATTACK)->val1,tick,0) )
				return ATK_DEF;
			return ATK_MISS;
		}
		if( sc->getSCE(SC_GT_ENERGYGAIN) ) {
			int spheres = 5;

			if( sc->getSCE(SC_RAISINGDRAGON) )
				spheres += sc->getSCE(SC_RAISINGDRAGON)->val1;

			if( sd && rnd()%100 < sc->getSCE(SC_GT_ENERGYGAIN)->val2 )
				pc_addspiritball(sd, skill_get_time2(SR_GENTLETOUCH_ENERGYGAIN, sc->getSCE(SC_GT_ENERGYGAIN)->val1), spheres);
		}
	}

	if( tsc && tsc->getSCE(SC_GT_ENERGYGAIN) ) {
		int spheres = 5;

		if( tsc->getSCE(SC_RAISINGDRAGON) )
			spheres += tsc->getSCE(SC_RAISINGDRAGON)->val1;

		if( tsd && rnd()%100 < tsc->getSCE(SC_GT_ENERGYGAIN)->val2 )
			pc_addspiritball(tsd, skill_get_time2(SR_GENTLETOUCH_ENERGYGAIN, tsc->getSCE(SC_GT_ENERGYGAIN)->val1), spheres);
	}

	if (tsc && tsc->getSCE(SC_MTF_MLEATKED) && rnd()%100 < tsc->getSCE(SC_MTF_MLEATKED)->val2)
		clif_skill_nodamage(target, target, SM_ENDURE, tsc->getSCE(SC_MTF_MLEATKED)->val1, sc_start(src, target, SC_ENDURE, 100, tsc->getSCE(SC_MTF_MLEATKED)->val1, skill_get_time(SM_ENDURE, tsc->getSCE(SC_MTF_MLEATKED)->val1)));

	if(tsc && tsc->getSCE(SC_KAAHI) && tstatus->hp < tstatus->max_hp && status_charge(target, 0, tsc->getSCE(SC_KAAHI)->val3)) {
		int hp_heal = tstatus->max_hp - tstatus->hp;
		if (hp_heal > tsc->getSCE(SC_KAAHI)->val2)
			hp_heal = tsc->getSCE(SC_KAAHI)->val2;
		if (hp_heal)
			status_heal(target, hp_heal, 0, 2);
	}

	wd = battle_calc_attack(BF_WEAPON, src, target, 0, 0, flag);

	if (sd && wd.damage + wd.damage2 > 0 && battle_vellum_damage(sd, target, &wd))
		vellum_damage = true;

	if( sc && sc->count ) {
		if (sc->getSCE(SC_EXEEDBREAK))
			status_change_end(src, SC_EXEEDBREAK);
		if( sc->getSCE(SC_SPELLFIST) && !vellum_damage ){
			if (status_charge(src, 0, 20)) {
				if (!is_infinite_defense(target, wd.flag)) {
					struct Damage ad = battle_calc_attack(BF_MAGIC, src, target, sc->getSCE(SC_SPELLFIST)->val2, sc->getSCE(SC_SPELLFIST)->val3, flag | BF_SHORT);

					wd.damage = ad.damage;
					DAMAGE_DIV_FIX(wd.damage, wd.div_); // Double the damage for multiple hits.
				} else {
					wd.damage = 1;
					DAMAGE_DIV_FIX(wd.damage, wd.div_);
				}
			} else
				status_change_end(src,SC_SPELLFIST);
		}
		if (sc->getSCE(SC_GIANTGROWTH) && (wd.flag&BF_SHORT) && rnd()%100 < sc->getSCE(SC_GIANTGROWTH)->val2 && !is_infinite_defense(target, wd.flag) && !vellum_damage)
			wd.damage += wd.damage * 150 / 100; // 2.5 times damage

		if( sc->getSCE( SC_VIGOR ) && ( wd.flag&BF_SHORT ) && !is_infinite_defense( target, wd.flag ) && !vellum_damage ){
			int mod = 100 + sc->getSCE(SC_VIGOR)->val1 * 15;

			if (tstatus->race == RC_DEMIHUMAN || tstatus->race == RC_ANGEL)
				mod += sc->getSCE(SC_VIGOR)->val1 * 10;

			wd.damage += wd.damage * mod / 100;
		}

		if( sd && battle_config.arrow_decrement && sc->getSCE(SC_FEARBREEZE) && sc->getSCE(SC_FEARBREEZE)->val4 > 0) {
			short idx = sd->equip_index[EQI_AMMO];
			if (idx >= 0 && sd->inventory.u.items_inventory[idx].amount >= sc->getSCE(SC_FEARBREEZE)->val4) {
				pc_delitem(sd,idx,sc->getSCE(SC_FEARBREEZE)->val4,0,1,LOG_TYPE_CONSUME);
				sc->getSCE(SC_FEARBREEZE)->val4 = 0;
			}
		}
	}
	if (sd && sd->state.arrow_atk) //Consume arrow.
		battle_consume_ammo(sd, 0, 0);

	damage = wd.damage + wd.damage2;
	if( damage > 0 && src != target )
	{
		if (sc && sc->getSCE(SC_DUPLELIGHT) && (wd.flag & BF_SHORT)) { // Activates only from regular melee damage. Success chance is seperate for both duple light attacks.
			uint16 duple_rate = 10 + 2 * sc->getSCE(SC_DUPLELIGHT)->val1;

			if (rand() % 100 < duple_rate)
				skill_castend_damage_id(src, target, AB_DUPLELIGHT_MELEE, sc->getSCE(SC_DUPLELIGHT)->val1, tick, flag | SD_LEVEL);

			if (rand() % 100 < duple_rate)
				skill_castend_damage_id(src, target, AB_DUPLELIGHT_MAGIC, sc->getSCE(SC_DUPLELIGHT)->val1, tick, flag | SD_LEVEL);
		}
	}

	wd.dmotion = clif_damage(src, target, tick, wd.amotion, wd.dmotion, wd.damage, wd.div_ , wd.type, wd.damage2, wd.isspdamage);

	if (sd && sd->bonus.splash_range > 0 && damage > 0)
		skill_castend_damage_id(src, target, 0, 1, tick, 0);
	if ( target->type == BL_SKILL && damage > 0 ) {
		TBL_SKILL *su = (TBL_SKILL*)target;

		if (su && su->group) {
			if (su->group->skill_id == HT_BLASTMINE)
				skill_blown(src, target, 3, -1, BLOWN_NONE);
			if (su->group->skill_id == GN_WALLOFTHORN) {
				if (--su->val2 <= 0)
					skill_delunit(su);
			}
		}
	}

	map_freeblock_lock();

	if( !(tsc && tsc->getSCE(SC_DEVOTION)) && !vellum_damage && skill_check_shadowform(target, damage, wd.div_) ) {
		if( !status_isdead(target) )
			skill_additional_effect(src, target, 0, 0, wd.flag, wd.dmg_lv, tick);
		if( wd.dmg_lv > ATK_BLOCK )
			skill_counter_additional_effect(src, target, 0, 0, wd.flag, tick);
	} else
		battle_delay_damage(tick, wd.amotion, src, target, wd.flag, 0, 0, damage, wd.dmg_lv, wd.dmotion, true, wd.isspdamage);
	if( tsc ) {
		if( tsc->getSCE(SC_DEVOTION) ) {
			struct status_change_entry *sce = tsc->getSCE(SC_DEVOTION);
			struct block_list *d_bl = map_id2bl(sce->val1);

			if( d_bl && (
				(d_bl->type == BL_MER && ((TBL_MER*)d_bl)->master && ((TBL_MER*)d_bl)->master->bl.id == target->id) ||
				(d_bl->type == BL_PC && ((TBL_PC*)d_bl)->devotion[sce->val2] == target->id)
				) && check_distance_bl(target, d_bl, sce->val3) )
			{
				// Only trigger if the devoted player was hit
				if( damage > 0 ){
					int64 devotion_damage = damage;
					map_session_data* dsd = BL_CAST( BL_PC, d_bl );

					// Needed to check the devotion master for Rebound Shield status.
					status_change *d_sc = status_get_sc(d_bl);

					// The devoting player needs to stand up
					if( dsd && pc_issit( dsd ) ){
						pc_setstand( dsd, true );
						skill_sit( dsd, 0 );
					}

					if (d_sc && d_sc->getSCE(SC_REBOUND_S))
						devotion_damage -= devotion_damage * d_sc->getSCE(SC_REBOUND_S)->val2 / 100;

					clif_damage(d_bl, d_bl, gettick(), wd.amotion, wd.dmotion, devotion_damage, 1, DMG_NORMAL, 0, false);
					battle_fix_damage(src, d_bl, devotion_damage, 0, CR_DEVOTION);
				}
			}
			else
				status_change_end(target, SC_DEVOTION);
		}
		if (target->type == BL_PC && (wd.flag&BF_SHORT) && tsc->getSCE(SC_CIRCLE_OF_FIRE_OPTION)) {
			s_elemental_data *ed = ((TBL_PC*)target)->ed;

			if (ed) {
				clif_skill_damage(&ed->bl, target, tick, status_get_amotion(src), 0, -30000, 1, EL_CIRCLE_OF_FIRE, tsc->getSCE(SC_CIRCLE_OF_FIRE_OPTION)->val1, DMG_SINGLE);
				skill_attack(BF_WEAPON,&ed->bl,&ed->bl,src,EL_CIRCLE_OF_FIRE,tsc->getSCE(SC_CIRCLE_OF_FIRE_OPTION)->val1,tick,wd.flag);
			}
		}
		if (tsc->getSCE(SC_WATER_SCREEN_OPTION)) {
			struct block_list *e_bl = map_id2bl(tsc->getSCE(SC_WATER_SCREEN_OPTION)->val1);

			if (e_bl && !status_isdead(e_bl)) {
				clif_damage(e_bl, e_bl, tick, 0, 0, damage, wd.div_, DMG_NORMAL, 0, false);
				battle_fix_damage(src, e_bl, damage, 0, EL_WATER_SCREEN);
			}
		}
	}
	if (sc && sc->getSCE(SC_AUTOSPELL) && rnd()%100 < sc->getSCE(SC_AUTOSPELL)->val4) {
		int sp = 0;
		uint16 skill_id = sc->getSCE(SC_AUTOSPELL)->val2;
		uint16 skill_lv = sc->getSCE(SC_AUTOSPELL)->val3;
		int i = rnd()%100;
		if (sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_SAGE)
			i = 0; //Max chance, no skill_lv reduction. [Skotlex]
		//reduction only for skill_lv > 1
		if (skill_lv > 1) {
			if (i >= 50) skill_lv /= 2;
			else if (i >= 15) skill_lv--;
		}
		sp = skill_get_sp(skill_id,skill_lv) * 2 / 3;

		if (status_charge(src, 0, sp)) {
			struct unit_data *ud = unit_bl2ud(src);

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
			if (ud) {
				int autospell_tick = skill_delayfix(src, skill_id, skill_lv);

				if (DIFF_TICK(ud->canact_tick, tick + autospell_tick) < 0) {
					ud->canact_tick = i64max(tick + autospell_tick, ud->canact_tick);
					if (battle_config.display_status_timers && sd)
						clif_status_change(src, EFST_POSTDELAY, 1, autospell_tick, 0, 0, 0);
				}
			}
		}
	}
	if (sd) {
		uint16 r_skill = 0, sk_idx = 0;
		if( wd.flag&BF_WEAPON && sc && sc->getSCE(SC__AUTOSHADOWSPELL) && rnd()%100 < sc->getSCE(SC__AUTOSHADOWSPELL)->val3 &&
			(r_skill = (uint16)sc->getSCE(SC__AUTOSHADOWSPELL)->val1) && (sk_idx = skill_get_index(r_skill)) &&
			sd->status.skill[sk_idx].id != 0 && sd->status.skill[sk_idx].flag == SKILL_FLAG_PLAGIARIZED )
		{
			if (r_skill != AL_HOLYLIGHT && r_skill != PR_MAGNUS) {
				int r_lv = sc->getSCE(SC__AUTOSHADOWSPELL)->val2, type;

				if( (type = skill_get_casttype(r_skill)) == CAST_GROUND ) {
					int maxcount = 0;
					std::shared_ptr<s_skill_db> skill = skill_db.find(r_skill);

					if( !(BL_PC&battle_config.skill_reiteration) && skill->unit_flag[UF_NOREITERATION] )
							type = -1;

					if( BL_PC&battle_config.skill_nofootset && skill->unit_flag[UF_NOFOOTSET] )
							type = -1;

					if( BL_PC&battle_config.land_skill_limit &&
						(maxcount = skill_get_maxcount(r_skill, r_lv)) > 0
					  ) {
						unit_skillunit_maxcount(sd->ud, r_skill, maxcount);

						if( maxcount == 0 )
							type = -1;
					}

					if( type != CAST_GROUND ){
						clif_skill_fail( *sd, r_skill );
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

				sd->ud.canact_tick = i64max(tick + skill_delayfix(src, r_skill, r_lv), sd->ud.canact_tick);
				clif_status_change(src, EFST_POSTDELAY, 1, skill_delayfix(src, r_skill, r_lv), 0, 0, 1);
			}
		}
		if (wd.flag&BF_WEAPON && sc && sc->getSCE(SC_FALLINGSTAR) && rand()%100 < sc->getSCE(SC_FALLINGSTAR)->val2) {
			if (sd)
				sd->state.autocast = 1;
			if (status_charge(src, 0, skill_get_sp(SJ_FALLINGSTAR_ATK, sc->getSCE(SC_FALLINGSTAR)->val1)))
				skill_castend_nodamage_id(src, src, SJ_FALLINGSTAR_ATK, sc->getSCE(SC_FALLINGSTAR)->val1, tick, flag);
			if (sd)
				sd->state.autocast = 0;
		}

		if( sc ){
			if( sc->getSCE( SC_SERVANTWEAPON ) && sd->servantball > 0 && rnd_chance( 5 * sc->getSCE( SC_SERVANTWEAPON )->val1, 100 ) ){
				uint16 skill_id = DK_SERVANTWEAPON_ATK;
				uint16 skill_lv = sc->getSCE(SC_SERVANTWEAPON)->val1;

				sd->state.autocast = 1;
				pc_delservantball( *sd );
				skill_castend_damage_id( src, target, skill_id, skill_lv, tick, flag );
				battle_autocast_aftercast( src, skill_id, skill_lv, tick );
				sd->state.autocast = 0;
			}

			// TODO: Whats the official success chance? Is SP consumed for every autocast? [Rytech]
			if( sc->getSCE(SC_DUPLELIGHT) && pc_checkskill(sd, CD_PETITIO) > 0 && rnd() % 100 < 20 ){
				uint16 skill_id = CD_PETITIO;
				uint16 skill_lv = pc_checkskill( sd, CD_PETITIO );

				sd->state.autocast = 1;
				skill_castend_damage_id( src, target, skill_id, skill_lv, tick, flag );
				battle_autocast_aftercast( src, skill_id, skill_lv, tick );
				sd->state.autocast = 0;
			}

			if( sc->getSCE(SC_ABYSSFORCEWEAPON) && sd->abyssball > 0 && rnd_chance( 25, 100 ) ){
				uint16 skill_id = ABC_FROM_THE_ABYSS_ATK;
				uint16 skill_lv = sc->getSCE(SC_ABYSSFORCEWEAPON)->val1;

				sd->state.autocast = 1;
				pc_delabyssball( *sd );
				skill_castend_damage_id( src, target, skill_id, skill_lv, tick, flag );
				battle_autocast_aftercast( src, skill_id, skill_lv, tick );
				sd->state.autocast = 0;
			}

			// It has a success chance of triggering even tho the description says nothing about it.
			// TODO: Need to find out what the official success chance is. [Rytech]
			if( sc->getSCE(SC_ABYSSFORCEWEAPON) && rnd() % 100 < 20 ){
				uint16 skill_id = ABC_ABYSS_SQUARE;
				uint16 skill_lv = pc_checkskill(sd, ABC_ABYSS_SQUARE);

				sd->state.autocast = 1;
				skill_castend_pos2( src, target->x, target->y, skill_id, skill_lv, tick, flag );
				battle_autocast_aftercast( src, skill_id, skill_lv, tick );
				sd->state.autocast = 0;
			}

			if( sc->getSCE( SC_AUTO_FIRING_LAUNCHER ) ){
				uint16 skill_id;
				uint16 skill_lv;

				switch( sc->getSCE( SC_AUTO_FIRING_LAUNCHER )->val1 ){
					case 1:
						skill_id = NW_BASIC_GRENADE;
						skill_lv = pc_checkskill( sd, skill_id );

						if( skill_lv > 0 && rnd_chance( 6, 100 ) ){
							sd->state.autocast = 1;
							skill_castend_pos2( src, target->x, target->y, skill_id, skill_lv, tick, flag );
							battle_autocast_aftercast( src, skill_id, skill_lv, tick );
							sd->state.autocast = 0;
						}
						break;

					case 2:
						skill_id = NW_BASIC_GRENADE;
						skill_lv = pc_checkskill( sd, skill_id );

						if( skill_lv > 0 && rnd_chance( 7, 100 ) ){
							sd->state.autocast = 1;
							skill_castend_pos2( src, target->x, target->y, skill_id, skill_lv, tick, flag );
							battle_autocast_aftercast( src, skill_id, skill_lv, tick );
							sd->state.autocast = 0;
						}
						break;

					case 3:
						skill_id = NW_HASTY_FIRE_IN_THE_HOLE;
						skill_lv = pc_checkskill( sd, skill_id );

						if( skill_lv > 0 && rnd_chance( 3, 100 ) ){
							sd->state.autocast = 1;
							skill_castend_pos2( src, target->x, target->y, skill_id, skill_lv, tick, flag );
							battle_autocast_aftercast( src, skill_id, skill_lv, tick );
							sd->state.autocast = 0;
						}

						skill_id = NW_BASIC_GRENADE;
						skill_lv = pc_checkskill( sd, skill_id );

						if( skill_lv > 0 && rnd_chance( 8, 100 ) ){
							sd->state.autocast = 1;
							skill_castend_pos2( src, target->x, target->y, skill_id, skill_lv, tick, flag );
							battle_autocast_aftercast( src, skill_id, skill_lv, tick );
							sd->state.autocast = 0;
						}
						break;

					case 4:
						skill_id = NW_HASTY_FIRE_IN_THE_HOLE;
						skill_lv = pc_checkskill( sd, skill_id );

						if( skill_lv > 0 && rnd_chance( 5, 100 ) ){
							sd->state.autocast = 1;
							skill_castend_pos2( src, target->x, target->y, skill_id, skill_lv, tick, flag );
							battle_autocast_aftercast( src, skill_id, skill_lv, tick );
							sd->state.autocast = 0;
						}

						skill_id = NW_BASIC_GRENADE;
						skill_lv = pc_checkskill( sd, skill_id );

						if( skill_lv > 0 && rnd_chance( 9, 100 ) ){
							sd->state.autocast = 1;
							skill_castend_pos2( src, target->x, target->y, skill_id, skill_lv, tick, flag );
							battle_autocast_aftercast( src, skill_id, skill_lv, tick );
							sd->state.autocast = 0;
						}
						break;

					case 5:
						skill_id = NW_GRENADES_DROPPING;
						skill_lv = pc_checkskill( sd, skill_id );

						if( skill_lv > 0 && rnd_chance( 3, 100 ) ){
							sd->state.autocast = 1;
							skill_castend_pos2( src, target->x, target->y, skill_id, skill_lv, tick, flag );
							battle_autocast_aftercast( src, skill_id, skill_lv, tick );
							sd->state.autocast = 0;
						}

						skill_id = NW_HASTY_FIRE_IN_THE_HOLE;
						skill_lv = pc_checkskill( sd, skill_id );

						if( skill_lv > 0 && rnd_chance( 7, 100 ) ){
							sd->state.autocast = 1;
							skill_castend_pos2( src, target->x, target->y, skill_id, skill_lv, tick, flag );
							battle_autocast_aftercast( src, skill_id, skill_lv, tick );
							sd->state.autocast = 0;
						}

						skill_id = NW_BASIC_GRENADE;
						skill_lv = pc_checkskill( sd, skill_id );

						if( skill_lv > 0 && rnd_chance( 10, 100 ) ){
							sd->state.autocast = 1;
							skill_castend_pos2( src, target->x, target->y, skill_id, skill_lv, tick, flag );
							battle_autocast_aftercast( src, skill_id, skill_lv, tick );
							sd->state.autocast = 0;
						}
						break;
				}
			}

			// Autocasted skills from super elemental supportive buffs.
			if (sc->getSCE(SC_FLAMETECHNIC_OPTION) && rnd() % 100 < 7)
				battle_autocast_elembuff_skill(sd, target, MG_FIREBOLT, tick, flag);
			if (sc->getSCE(SC_COLD_FORCE_OPTION) && rnd() % 100 < 7)
				battle_autocast_elembuff_skill(sd, target, MG_COLDBOLT, tick, flag);
			if (sc->getSCE(SC_GRACE_BREEZE_OPTION) && rnd() % 100 < 7)
				battle_autocast_elembuff_skill(sd, target, MG_LIGHTNINGBOLT, tick, flag);
			if (sc->getSCE(SC_EARTH_CARE_OPTION) && rnd() % 100 < 7)
				battle_autocast_elembuff_skill(sd, target, WZ_EARTHSPIKE, tick, flag);
			if (sc->getSCE(SC_DEEP_POISONING_OPTION) && rnd() % 100 < 7)
				battle_autocast_elembuff_skill(sd, target, SO_POISON_BUSTER, tick, flag);
		}
		if (wd.flag & BF_WEAPON && src != target && damage > 0) {
			if (battle_config.left_cardfix_to_right)
				battle_drain(sd, target, wd.damage, wd.damage, tstatus->race, tstatus->class_);
			else
				battle_drain(sd, target, wd.damage, wd.damage2, tstatus->race, tstatus->class_);
		}
	}

	if (tsc) {
		if (damage > 0 && tsc->getSCE(SC_POISONREACT) &&
			(rnd()%100 < tsc->getSCE(SC_POISONREACT)->val3
			|| sstatus->def_ele == ELE_POISON) &&
//			check_distance_bl(src, target, tstatus->rhw.range+1) && Doesn't checks range! o.O;
			status_check_skilluse(target, src, TF_POISON, 0)
		) {	//Poison React
			struct status_change_entry *sce = tsc->getSCE(SC_POISONREACT);
			if (sstatus->def_ele == ELE_POISON) {
				sce->val2 = 0;
				skill_attack(BF_WEAPON,target,target,src,AS_POISONREACT,sce->val1,tick,0);
			} else {
				skill_attack(BF_WEAPON,target,target,src,TF_POISON, 5, tick, 0);
				--sce->val2;
			}
			if (sce->val2 <= 0)
				status_change_end(target, SC_POISONREACT);
		}
	}

	if (sd && tsc && wd.flag&BF_LONG && tsc->getSCE(SC_WINDSIGN) && rand()%100 < tsc->getSCE(SC_WINDSIGN)->val2)
		status_heal(src, 0, 0, 1, 0);

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

bool battle_get_exception_ai(block_list &src) {
	mob_data *md = BL_CAST(BL_MOB, &src);

	if (!md)
		return false;

	switch (md->special_state.ai) {
		case AI_ABR:
		case AI_ATTACK:
		case AI_BIONIC:
		case AI_ZANZOU:
			return true;
	}
	return false;
}

/*==========================================
 * Checks the state between two targets
 * (enemy, friend, party, guild, etc)
 *------------------------------------------
 * Usage:
 * See battle.hpp for possible values/combinations
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
	struct unit_data *ud = nullptr;

	nullpo_ret(src);
	nullpo_ret(target);

	ud = unit_bl2ud(target);
	m = target->m;

	//t_bl/s_bl hold the 'master' of the attack, while src/target are the actual
	//objects involved.
	if( (t_bl = battle_get_master(target)) == nullptr )
		t_bl = target;

	if( (s_bl = battle_get_master(src)) == nullptr )
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

	struct map_data *mapdata = map_getmapdata(m);

	switch( target->type ) { // Checks on actual target
		case BL_PC: {
				status_change* sc = status_get_sc(src);

				if (((TBL_PC*)target)->invincible_timer != INVALID_TIMER || pc_isinvisible((TBL_PC*)target))
					return -1; //Cannot be targeted yet.
				if( sc && sc->count ) {
					if( sc->getSCE(SC_VOICEOFSIREN) && sc->getSCE(SC_VOICEOFSIREN)->val2 == target->id )
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
			uint16 skill_id = battle_getcurrentskill(src);
			if( !su || !su->group)
				return 0;
			if( skill_get_inf2(su->group->skill_id, INF2_ISTRAP) && su->group->unit_id != UNT_USED_TRAPS) {
				if (!skill_id || su->group->skill_id == NPC_REVERBERATION || su->group->skill_id == WM_POEMOFNETHERWORLD) {
					;
				}
				else if (skill_get_inf2(skill_id, INF2_TARGETTRAP)) { // Only a few skills can target traps
					switch (skill_id) {
						case RK_DRAGONBREATH:
						case RK_DRAGONBREATH_WATER:
						case NC_SELFDESTRUCTION:
						case NC_AXETORNADO:
						case SR_SKYNETBLOW:
							// Can only hit traps in PVP/GVG maps
							if (!mapdata->getMapFlag(MF_PVP) && !mapdata->getMapFlag(MF_GVG))
								return 0;
							break;
					}
				}
				else
					return 0;
				state |= BCT_ENEMY;
				strip_enemy = 0;
			} else if (su->group->skill_id == WZ_ICEWALL || (su->group->skill_id == GN_WALLOFTHORN && skill_id != GN_CARTCANNON)) {
				switch (skill_id) {
					case RK_DRAGONBREATH:
					case RK_DRAGONBREATH_WATER:
					case NC_SELFDESTRUCTION:
					case NC_AXETORNADO:
					case SR_SKYNETBLOW:
						// Can only hit icewall in PVP/GVG maps
						if (!mapdata->getMapFlag(MF_PVP) && !mapdata->getMapFlag(MF_GVG))
							return 0;
						break;
					case HT_CLAYMORETRAP:
						// Can't hit icewall
						return 0;
					default:
						// Usually BCT_ALL stands for only hitting chars, but skills specifically set to hit traps also hit icewall
						if ((flag&BCT_ALL) == BCT_ALL && !skill_get_inf2(skill_id, INF2_TARGETTRAP))
							return -1;
				}
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
			map_session_data *sd;
			status_change *sc = nullptr;

			if( t_bl == s_bl )
				break;

			sd = BL_CAST(BL_PC, t_bl);
			sc = status_get_sc(t_bl);

			if( ((sd->state.block_action & PCBLOCK_IMMUNE) || (sc->getSCE(SC_KINGS_GRACE) && s_bl->type != BL_PC)) && flag&BCT_ENEMY )
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

			if( md->guardian_data && md->guardian_data->guild_id && !mapdata_flag_gvg(mapdata) )
				return 0; // Disable guardians/emperiums owned by Guilds on non-woe times.
			break;
		}
		default: break; //other type doesn't have slave yet
    } //end switch master target

	switch( src->type ) { //Checks on actual src type
		case BL_PET:
			if (t_bl->type != BL_MOB && flag&BCT_ENEMY)
				return 0; //Pet may not attack non-mobs.
			if (t_bl->type == BL_MOB && flag & BCT_ENEMY) {
				mob_data *md = BL_CAST(BL_MOB, t_bl);

				if (md->guardian_data || md->special_state.ai == AI_GUILD)
					return 0; //pet may not attack Guardians/Emperium
			}
			break;
		case BL_SKILL: {
				struct skill_unit *su = (struct skill_unit *)src;
				status_change* sc = status_get_sc(target);
				if (!su || !su->group)
					return 0;

				std::bitset<INF2_MAX> inf2 = skill_db.find(su->group->skill_id)->inf2;

				if (su->group->src_id == target->id) {
					if (inf2[INF2_NOTARGETSELF])
						return -1;
					if (inf2[INF2_TARGETSELF])
						return 1;
				}
				//Status changes that prevent traps from triggering
				if (sc && sc->count && inf2[INF2_ISTRAP]) {
					if( sc->getSCE(SC_SIGHTBLASTER) && sc->getSCE(SC_SIGHTBLASTER)->val2 > 0 && sc->getSCE(SC_SIGHTBLASTER)->val4%2 == 0)
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
			map_session_data *sd = BL_CAST(BL_PC, s_bl);
			if( s_bl != t_bl )
			{
				if( sd->state.killer )
				{
					state |= BCT_ENEMY; // Can kill anything
					strip_enemy = 0;
				}
				else if( sd->duel_group && !((!battle_config.duel_allow_pvp && mapdata->getMapFlag(MF_PVP)) || (!battle_config.duel_allow_gvg && mapdata_flag_gvg(mapdata))) )
				{
					if( t_bl->type == BL_PC && (sd->duel_group == ((TBL_PC*)t_bl)->duel_group) )
						return (BCT_ENEMY&flag)?1:-1; // Duel targets can ONLY be your enemy, nothing else.
					else
						return 0; // You can't target anything out of your duel
				}
			}
			if( !sd->status.guild_id && t_bl->type == BL_MOB && ((TBL_MOB*)t_bl)->mob_id == MOBID_EMPERIUM && mapdata_flag_gvg(mapdata) )
				return 0; //If you don't belong to a guild, can't target emperium.
			if( t_bl->type != BL_PC )
				state |= BCT_ENEMY; //Natural enemy.
			break;
		}
		case BL_MOB:
		{
			struct mob_data *md = BL_CAST(BL_MOB, s_bl);
			if( md->guardian_data && md->guardian_data->guild_id && !mapdata_flag_gvg(mapdata) )
				return 0; // Disable guardians/emperium owned by Guilds on non-woe times.

			if( !md->special_state.ai )
			{ //Normal mobs
				if(
					( target->type == BL_MOB && t_bl->type == BL_PC && !battle_get_exception_ai(*target) ) ||
					( t_bl->type == BL_MOB && (((TBL_MOB*)t_bl)->special_state.ai == AI_NONE || ((TBL_MOB*)t_bl)->special_state.ai == AI_WAVEMODE ))
				  )
					state |= BCT_PARTY; //Normal mobs with no ai or with AI_WAVEMODE are friends.
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
	{ //All actually stands for all attackable chars, icewall and traps
		if(target->type&(BL_CHAR|BL_SKILL))
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

	if( mapdata_flag_vs(mapdata) )
	{ //Check rivalry settings.
		int sbg_id = 0, tbg_id = 0;
		if(mapdata->getMapFlag(MF_BATTLEGROUND) )
		{
			sbg_id = bg_team_get_id(s_bl);
			tbg_id = bg_team_get_id(t_bl);
		}
		if( flag&(BCT_PARTY|BCT_ENEMY) )
		{
			int s_party = status_get_party_id(s_bl);
			if( s_party && s_party == status_get_party_id(t_bl) && !(mapdata->getMapFlag(MF_PVP) && mapdata->getMapFlag(MF_PVP_NOPARTY)) && !(mapdata_flag_gvg(mapdata) && mapdata->getMapFlag(MF_GVG_NOPARTY)) && (!mapdata->getMapFlag(MF_BATTLEGROUND) || sbg_id == tbg_id) )
				state |= BCT_PARTY;
			else
				state |= BCT_ENEMY;
		}
		if( flag&(BCT_GUILD|BCT_ENEMY) )
		{
			int s_guild = status_get_guild_id(s_bl);
			int t_guild = status_get_guild_id(t_bl);
			if( !(mapdata->getMapFlag(MF_PVP) && mapdata->getMapFlag(MF_PVP_NOGUILD)) && s_guild && t_guild && (s_guild == t_guild || (!(flag&BCT_SAMEGUILD) && guild_isallied(s_guild, t_guild))) && (!mapdata->getMapFlag(MF_BATTLEGROUND) || sbg_id == tbg_id) )
				state |= BCT_GUILD;
			else
				state |= BCT_ENEMY;
		}
		if( state&BCT_ENEMY && mapdata->getMapFlag(MF_BATTLEGROUND) && sbg_id && sbg_id == tbg_id )
			state &= ~BCT_ENEMY;

		if( state&BCT_ENEMY && battle_config.pk_mode && !mapdata_flag_gvg(mapdata) && s_bl->type == BL_PC && t_bl->type == BL_PC )
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

	return path_search_long(nullptr,src->m,src->x,src->y,bl->x,bl->y,CELL_CHKWALL);
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
	{ "enable_baseatk",                     &battle_config.enable_baseatk,                  BL_CHAR|BL_HOM, BL_NUL, BL_ALL, },
	{ "enable_baseatk_renewal",             &battle_config.enable_baseatk_renewal,          BL_ALL, BL_NUL, BL_ALL,         },
	{ "enable_perfect_flee",                &battle_config.enable_perfect_flee,             BL_PC|BL_PET, BL_NUL, BL_ALL,   },
	{ "casting_rate",                       &battle_config.cast_rate,                       100,    0,      INT_MAX,        },
	{ "delay_rate",                         &battle_config.delay_rate,                      100,    0,      INT_MAX,        },
	{ "delay_dependon_dex",                 &battle_config.delay_dependon_dex,              0,      0,      1,              },
	{ "delay_dependon_agi",                 &battle_config.delay_dependon_agi,              0,      0,      1,              },
	{ "skill_delay_attack_enable",          &battle_config.sdelay_attack_enable,            0,      0,      1,              },
	{ "left_cardfix_to_right",              &battle_config.left_cardfix_to_right,           0,      0,      1,              },
	{ "cardfix_monster_physical",           &battle_config.cardfix_monster_physical,        1,      0,      1,              },
	{ "skill_add_range",                    &battle_config.skill_add_range,                 0,      0,      INT_MAX,        },
	{ "skill_out_range_consume",            &battle_config.skill_out_range_consume,         1,      0,      1,              },
	{ "skillrange_by_distance",             &battle_config.skillrange_by_distance,          ~BL_PC, BL_NUL, BL_ALL,         },
	{ "skillrange_from_weapon",             &battle_config.use_weapon_skill_range,          BL_NUL, BL_NUL, BL_ALL,         },
	{ "player_damage_delay_rate",           &battle_config.pc_damage_delay_rate,            100,    0,      INT_MAX,        },
	{ "defunit_not_enemy",                  &battle_config.defnotenemy,                     0,      0,      1,              },
	{ "gvg_traps_target_all",               &battle_config.vs_traps_bctall,                 BL_PC,  BL_NUL, BL_ALL,         },
#ifdef RENEWAL
	{ "traps_setting",                      &battle_config.traps_setting,                   2,      0,      2,              },
#else
	{ "traps_setting",                      &battle_config.traps_setting,                   0,      0,      2,              },
#endif
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
#ifdef RENEWAL
	{ "guild_skill_relog_type",             &battle_config.guild_skill_relog_type,          0,      0,      1,              },
#else
	{ "guild_skill_relog_type",             &battle_config.guild_skill_relog_type,          1,      0,      1,              },
#endif
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
	{ "slaves_inherit_mode",                &battle_config.slaves_inherit_mode,             4,      0,      4,              },
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
	{ "pet_support_rate",                   &battle_config.pet_support_rate,                100,    0,      INT_MAX,        },
	{ "pet_attack_exp_to_master",           &battle_config.pet_attack_exp_to_master,        0,      0,      1,              },
	{ "pet_attack_exp_rate",                &battle_config.pet_attack_exp_rate,             100,    0,      INT_MAX,        },
	{ "pet_lv_rate",                        &battle_config.pet_lv_rate,                     0,      0,      INT_MAX,        },
	{ "pet_max_stats",                      &battle_config.pet_max_stats,                   99,     0,      INT_MAX,        },
	{ "pet_max_atk1",                       &battle_config.pet_max_atk1,                    750,    0,      INT_MAX,        },
	{ "pet_max_atk2",                       &battle_config.pet_max_atk2,                    1000,   0,      INT_MAX,        },
	{ "pet_disable_in_gvg",                 &battle_config.pet_no_gvg,                      0,      0,      1,              },
	{ "pet_master_dead",                    &battle_config.pet_master_dead,                 0,      0,      1,              },
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
	{ "natural_heal_weight_rate",           &battle_config.natural_heal_weight_rate,        50,     0,      100             },
	{ "natural_heal_weight_rate_renewal",   &battle_config.natural_heal_weight_rate_renewal,70,     0,      100             },
	{ "arrow_decrement",                    &battle_config.arrow_decrement,                 1,      0,      2,              },
	{ "ammo_unequip",                       &battle_config.ammo_unequip,                    1,      0,      1,              },
	{ "ammo_check_weapon",                  &battle_config.ammo_check_weapon,               1,      0,      1,              },
	{ "max_aspd",                           &battle_config.max_aspd,                        190,    100,    199,            },
	{ "max_third_aspd",                     &battle_config.max_third_aspd,                  193,    100,    199,            },
	{ "max_summoner_aspd",                  &battle_config.max_summoner_aspd,               193,    100,    199,            },
	{ "max_walk_speed",                     &battle_config.max_walk_speed,                  300,    100,    100*DEFAULT_WALK_SPEED, },
	{ "max_lv",                             &battle_config.max_lv,                          99,     0,      MAX_LEVEL,      },
	{ "aura_lv",                            &battle_config.aura_lv,                         99,     0,      INT_MAX,        },
	{ "max_hp_lv99",                        &battle_config.max_hp_lv99,                    330000,  100,    1000000000,     },
	{ "max_hp_lv150",                       &battle_config.max_hp_lv150,                   660000,  100,    1000000000,     },
	{ "max_hp",                             &battle_config.max_hp,                        1100000,  100,    1000000000,     },
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
	{ "vit_penalty_num",                    &battle_config.vit_penalty_num,                 5,      1,      INT_MAX,        },
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
	{ "mob_warp",                           &battle_config.mob_warp,                        0,      0,      1|2|4|8,          },
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
	{ "bone_drop",                          &battle_config.bone_drop,                       0,      0,      2,              },
	{ "buyer_name",                         &battle_config.buyer_name,                      1,      0,      1,              },
	{ "skill_wall_check",                   &battle_config.skill_wall_check,                1,      0,      1,              },
	{ "official_cell_stack_limit",          &battle_config.official_cell_stack_limit,       1,      0,      255,            },
	{ "custom_cell_stack_limit",            &battle_config.custom_cell_stack_limit,         1,      1,      255,            },
	{ "dancing_weaponswitch_fix",           &battle_config.dancing_weaponswitch_fix,        1,      0,      1,              },

	// eAthena additions
	{ "item_logarithmic_drops",             &battle_config.logarithmic_drops,               0,      0,      1,              },
	{ "item_drop_common_min",               &battle_config.item_drop_common_min,            1,      0,      10000,          },
	{ "item_drop_common_max",               &battle_config.item_drop_common_max,            10000,  1,      10000,          },
	{ "item_drop_equip_min",                &battle_config.item_drop_equip_min,             1,      0,      10000,          },
	{ "item_drop_equip_max",                &battle_config.item_drop_equip_max,             10000,  1,      10000,          },
	{ "item_drop_card_min",                 &battle_config.item_drop_card_min,              1,      0,      10000,          },
	{ "item_drop_card_max",                 &battle_config.item_drop_card_max,              10000,  1,      10000,          },
	{ "item_drop_mvp_min",                  &battle_config.item_drop_mvp_min,               1,      0,      10000,          },
	{ "item_drop_mvp_max",                  &battle_config.item_drop_mvp_max,               10000,  1,      10000,          },
	{ "item_drop_mvp_mode",                 &battle_config.item_drop_mvp_mode,              0,      0,      2,              },
	{ "item_drop_heal_min",                 &battle_config.item_drop_heal_min,              1,      0,      10000,          },
	{ "item_drop_heal_max",                 &battle_config.item_drop_heal_max,              10000,  1,      10000,          },
	{ "item_drop_use_min",                  &battle_config.item_drop_use_min,               1,      0,      10000,          },
	{ "item_drop_use_max",                  &battle_config.item_drop_use_max,               10000,  1,      10000,          },
	{ "item_drop_add_min",                  &battle_config.item_drop_adddrop_min,           1,      0,      10000,          },
	{ "item_drop_add_max",                  &battle_config.item_drop_adddrop_max,           10000,  1,      10000,          },
	{ "item_drop_treasure_min",             &battle_config.item_drop_treasure_min,          1,      0,      10000,          },
	{ "item_drop_treasure_max",             &battle_config.item_drop_treasure_max,          10000,  1,      10000,          },
	{ "item_rate_mvp",                      &battle_config.item_rate_mvp,                   100,    0,      1000000,        },
	{ "item_rate_common",                   &battle_config.item_rate_common,                100,    0,      1000000,        },
	{ "item_rate_common_boss",              &battle_config.item_rate_common_boss,           100,    0,      1000000,        },
	{ "item_rate_common_mvp",               &battle_config.item_rate_common_mvp,            100,    0,      1000000,        },
	{ "item_rate_equip",                    &battle_config.item_rate_equip,                 100,    0,      1000000,        },
	{ "item_rate_equip_boss",               &battle_config.item_rate_equip_boss,            100,    0,      1000000,        },
	{ "item_rate_equip_mvp",                &battle_config.item_rate_equip_mvp,             100,    0,      1000000,        },
	{ "item_rate_card",                     &battle_config.item_rate_card,                  100,    0,      1000000,        },
	{ "item_rate_card_boss",                &battle_config.item_rate_card_boss,             100,    0,      1000000,        },
	{ "item_rate_card_mvp",                 &battle_config.item_rate_card_mvp,              100,    0,      1000000,        },
	{ "item_rate_heal",                     &battle_config.item_rate_heal,                  100,    0,      1000000,        },
	{ "item_rate_heal_boss",                &battle_config.item_rate_heal_boss,             100,    0,      1000000,        },
	{ "item_rate_heal_mvp",                 &battle_config.item_rate_heal_mvp,              100,    0,      1000000,        },
	{ "item_rate_use",                      &battle_config.item_rate_use,                   100,    0,      1000000,        },
	{ "item_rate_use_boss",                 &battle_config.item_rate_use_boss,              100,    0,      1000000,        },
	{ "item_rate_use_mvp",                  &battle_config.item_rate_use_mvp,               100,    0,      1000000,        },
	{ "item_rate_adddrop",                  &battle_config.item_rate_adddrop,               100,    0,      1000000,        },
	{ "item_rate_treasure",                 &battle_config.item_rate_treasure,              100,    0,      1000000,        },
	{ "prevent_logout",                     &battle_config.prevent_logout,                  10000,  0,      60000,          },
	{ "prevent_logout_trigger",             &battle_config.prevent_logout_trigger,          0xE,    0,      0xF,            },
	{ "alchemist_summon_reward",            &battle_config.alchemist_summon_reward,         1,      0,      2,              },
	{ "drops_by_luk",                       &battle_config.drops_by_luk,                    0,      0,      INT_MAX,        },
	{ "drops_by_luk2",                      &battle_config.drops_by_luk2,                   0,      0,      INT_MAX,        },
	{ "equip_natural_break_rate",           &battle_config.equip_natural_break_rate,        0,      0,      INT_MAX,        },
	{ "equip_self_break_rate",              &battle_config.equip_self_break_rate,           100,    0,      INT_MAX,        },
	{ "equip_skill_break_rate",             &battle_config.equip_skill_break_rate,          100,    0,      INT_MAX,        },
	{ "pk_mode",                            &battle_config.pk_mode,                         0,      0,      2,              },
	{ "pk_mode_mes",                        &battle_config.pk_mode_mes,                     1,      0,      1,              },
	{ "pk_level_range",                     &battle_config.pk_level_range,                  0,      0,      INT_MAX,        },
	{ "manner_system",                      &battle_config.manner_system,                   0xFFF,  0,      0xFFF,          },
	{ "pet_equip_required",                 &battle_config.pet_equip_required,              0,      0,      1,              },
	{ "pet_unequip_destroy",                &battle_config.pet_unequip_destroy,             1,      0,      1,              },
	{ "multi_level_up",                     &battle_config.multi_level_up,                  0,      0,      1,              },
	{ "multi_level_up_base",                &battle_config.multi_level_up_base,             0,      0,      MAX_LEVEL,      },
	{ "multi_level_up_job",                 &battle_config.multi_level_up_job,              0,      0,      MAX_LEVEL,      },
	{ "max_exp_gain_rate",                  &battle_config.max_exp_gain_rate,               0,      0,      INT_MAX,        },
	{ "backstab_bow_penalty",               &battle_config.backstab_bow_penalty,            0,      0,      1,              },
	{ "night_at_start",                     &battle_config.night_at_start,                  0,      0,      1,              },
	{ "show_mob_info",                      &battle_config.show_mob_info,                   0,      0,      1|2|4,          },
	{ "ban_hack_trade",                     &battle_config.ban_hack_trade,                  0,      0,      INT_MAX,        },
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
	{ "skill_steal_random_options",         &battle_config.skill_steal_random_options,      0,      0,      1,              },
	{ "motd_type",                          &battle_config.motd_type,                       0,      0,      1,              },
	{ "finding_ore_rate",                   &battle_config.finding_ore_rate,                100,    0,      INT_MAX,        },
	{ "exp_calc_type",                      &battle_config.exp_calc_type,                   0,      0,      2,              },
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
	{ "debuff_on_logout",                   &battle_config.debuff_on_logout,                0,      0,      1|2,            },
	{ "monster_ai",                         &battle_config.mob_ai,                          0x0000, 0x0000, 0x1FFF,         },
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
	{ "drop_rate_cap",                      &battle_config.drop_rate_cap,                   9000,   0,      10000,          },
	{ "drop_rate_cap_vip",                  &battle_config.drop_rate_cap_vip,               9000,   0,      10000,          },
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
	{ "sg_miracle_skill_ratio",             &battle_config.sg_miracle_skill_ratio,          1,      0,      20000,          },
	{ "sg_angel_skill_ratio",               &battle_config.sg_angel_skill_ratio,            10,     0,      10000,          },
	{ "autospell_stacking",                 &battle_config.autospell_stacking,              0,      0,      1,              },
	{ "override_mob_names",                 &battle_config.override_mob_names,              0,      0,      2,              },
	{ "min_chat_delay",                     &battle_config.min_chat_delay,                  0,      0,      INT_MAX,        },
	{ "friend_auto_add",                    &battle_config.friend_auto_add,                 1,      0,      1,              },
	{ "hom_rename",                         &battle_config.hom_rename,                      0,      0,      1,              },
	{ "homunculus_show_growth",             &battle_config.homunculus_show_growth,          0,      0,      1,              },
	{ "homunculus_friendly_rate",           &battle_config.homunculus_friendly_rate,        100,    0,      INT_MAX,        },
	{ "vending_tax",                        &battle_config.vending_tax,                     0,      0,      10000,          },
	{ "vending_tax_min",                    &battle_config.vending_tax_min,                 0,      0,      MAX_ZENY,       },
	{ "day_duration",                       &battle_config.day_duration,                    0,      0,      INT_MAX,        },
	{ "night_duration",                     &battle_config.night_duration,                  0,      0,      INT_MAX,        },
	{ "mob_remove_delay",                   &battle_config.mob_remove_delay,                60000,  1000,   INT_MAX,        },
	{ "mob_active_time",                    &battle_config.mob_active_time,                 5000,   0,      INT_MAX,        },
	{ "boss_active_time",                   &battle_config.boss_active_time,                5000,   0,      INT_MAX,        },
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
	{ "homunculus_auto_vapor",              &battle_config.homunculus_auto_vapor,           80,     0,      100,            },
	{ "display_status_timers",              &battle_config.display_status_timers,           1,      0,      1,              },
	{ "skill_add_heal_rate",                &battle_config.skill_add_heal_rate,           487,      0,      INT_MAX,        },
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
	{ "land_protector_behavior",            &battle_config.land_protector_behavior,         0,      0,      1,              },
	{ "npc_emotion_behavior",               &battle_config.npc_emotion_behavior,            0,      0,      1,              },
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
	{ "max_summoner_parameter",				&battle_config.max_summoner_parameter,			120,	10,		SHRT_MAX,		},
	{ "max_fourth_parameter",				&battle_config.max_fourth_parameter,			135,	10,		SHRT_MAX,		},
	{ "skill_amotion_leniency",             &battle_config.skill_amotion_leniency,          0,      0,      300             },
	{ "mvp_tomb_enabled",                   &battle_config.mvp_tomb_enabled,                1,      0,      1               },
	{ "mvp_tomb_delay",                     &battle_config.mvp_tomb_delay,                  9000,   0,      INT_MAX,        },
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
#ifdef RENEWAL
	{ "item_enabled_npc",                   &battle_config.item_enabled_npc,                0,      0,      1,              },
#else
	{ "item_enabled_npc",                   &battle_config.item_enabled_npc,                1,      0,      1,              },
#endif
	{ "item_flooritem_check",               &battle_config.item_onfloor,                    1,      0,      1,              },
	{ "bowling_bash_area",                  &battle_config.bowling_bash_area,               0,      0,      20,             },
	{ "drop_rateincrease",                  &battle_config.drop_rateincrease,               0,      0,      1,              },
	{ "feature.auction",                    &battle_config.feature_auction,                 0,      0,      2,              },
	{ "feature.banking",                    &battle_config.feature_banking,                 1,      0,      1,              },
#ifdef VIP_ENABLE
	{ "vip_storage_increase",               &battle_config.vip_storage_increase,          300,      0,      MAX_STORAGE-MIN_STORAGE, },
#else
	{ "vip_storage_increase",               &battle_config.vip_storage_increase,          300,      0,      MAX_STORAGE, },
#endif
	{ "vip_base_exp_increase",              &battle_config.vip_base_exp_increase,          50,      0,      INT_MAX,        },
	{ "vip_job_exp_increase",               &battle_config.vip_job_exp_increase,           50,      0,      INT_MAX,        },
	{ "vip_exp_penalty_base",               &battle_config.vip_exp_penalty_base,          100,      0,      INT_MAX,        },
	{ "vip_exp_penalty_job",                &battle_config.vip_exp_penalty_job,           100,      0,      INT_MAX,        },
	{ "vip_zeny_penalty",                   &battle_config.vip_zeny_penalty,                0,      0,      INT_MAX,        },
	{ "vip_bm_increase",                    &battle_config.vip_bm_increase,                 2,      0,      INT_MAX,        },
	{ "vip_drop_increase",                  &battle_config.vip_drop_increase,              50,      0,      INT_MAX,        },
	{ "vip_gemstone",                       &battle_config.vip_gemstone,                    2,      0,      2,              },
	{ "vip_disp_rate",                      &battle_config.vip_disp_rate,                   1,      0,      1,              },
	{ "mon_trans_disable_in_gvg",           &battle_config.mon_trans_disable_in_gvg,        0,      0,      1,              },
	{ "homunculus_S_growth_level",          &battle_config.hom_S_growth_level,             99,      0,      MAX_LEVEL,      },
	{ "discount_item_point_shop",			&battle_config.discount_item_point_shop,		0,		0,		3,				},
	{ "update_enemy_position",				&battle_config.update_enemy_position,			0,		0,		1,				},
	{ "devotion_rdamage",					&battle_config.devotion_rdamage,				0,		0,		100,			},
	{ "feature.autotrade",					&battle_config.feature_autotrade,				1,		0,		1,				},
	{ "feature.autotrade_direction",		&battle_config.feature_autotrade_direction,		4,		-1,		7,				},
	{ "feature.autotrade_head_direction",	&battle_config.feature_autotrade_head_direction,0,		-1,		2,				},
	{ "feature.autotrade_sit",				&battle_config.feature_autotrade_sit,			1,		-1,		1,				},
	{ "feature.autotrade_open_delay",		&battle_config.feature_autotrade_open_delay,	5000,	1000,	INT_MAX,		},
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
	{ "idletime_option",                    &battle_config.idletime_option,                 0x7C1F, 1,      0xFFFF,         },
	{ "spawn_direction",                    &battle_config.spawn_direction,                 0,      0,      1,              },
	{ "arrow_shower_knockback",             &battle_config.arrow_shower_knockback,          1,      0,      1,              },
	{ "devotion_rdamage_skill_only",        &battle_config.devotion_rdamage_skill_only,     1,      0,      1,              },
	{ "max_extended_aspd",                  &battle_config.max_extended_aspd,               193,    100,    199,            },
	{ "monster_chase_refresh",              &battle_config.mob_chase_refresh,               30,     0,      MAX_MINCHASE,   },
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
	{ "feature.roulette_bonus_reward",      &battle_config.feature_roulette_bonus_reward,   1,      0,      1,              },
	{ "monster_hp_bars_info",               &battle_config.monster_hp_bars_info,            1,      0,      1,              },
	{ "min_body_style",                     &battle_config.min_body_style,                  0,      0,      SHRT_MAX,       },
	{ "max_body_style",                     &battle_config.max_body_style,                  1,      0,      SHRT_MAX,       },
	{ "save_body_style",                    &battle_config.save_body_style,                 1,      0,      1,              },
	{ "monster_eye_range_bonus",            &battle_config.mob_eye_range_bonus,             0,      0,      10,             },
	{ "monster_stuck_warning",              &battle_config.mob_stuck_warning,               0,      0,      1,              },
	{ "skill_eightpath_algorithm",          &battle_config.skill_eightpath_algorithm,       1,      0,      1,              },
	{ "skill_eightpath_same_cell",          &battle_config.skill_eightpath_same_cell,       1,      0,      1,              },
	{ "death_penalty_maxlv",                &battle_config.death_penalty_maxlv,             0,      0,      3,              },
	{ "exp_cost_redemptio",                 &battle_config.exp_cost_redemptio,              1,      0,      100,            },
	{ "exp_cost_redemptio_limit",           &battle_config.exp_cost_redemptio_limit,        5,      0,      MAX_PARTY,      },
	{ "mvp_exp_reward_message",             &battle_config.mvp_exp_reward_message,          0,      0,      1,              },
	{ "can_damage_skill",                   &battle_config.can_damage_skill,                1,      0,      BL_ALL,         },
	{ "atcommand_levelup_events",			&battle_config.atcommand_levelup_events,		0,		0,		1,				},
	{ "atcommand_disable_npc",				&battle_config.atcommand_disable_npc,			1,		0,		1,				},
	{ "block_account_in_same_party",		&battle_config.block_account_in_same_party,		1,		0,		1,				},
	{ "tarotcard_equal_chance",             &battle_config.tarotcard_equal_chance,          0,      0,      1,              },
	{ "change_party_leader_samemap",        &battle_config.change_party_leader_samemap,     1,      0,      1,              },
	{ "dispel_song",                        &battle_config.dispel_song,                     0,      0,      1,              },
	{ "guild_maprespawn_clones",			&battle_config.guild_maprespawn_clones,			0,		0,		1,				},
	{ "hide_fav_sell", 			&battle_config.hide_fav_sell,			0,      0,      1,              },
	{ "mail_daily_count",					&battle_config.mail_daily_count,				100,	0,		INT32_MAX,		},
	{ "mail_zeny_fee",						&battle_config.mail_zeny_fee,					2,		0,		100,			},
	{ "mail_attachment_price",				&battle_config.mail_attachment_price,			2500,	0,		INT32_MAX,		},
	{ "mail_attachment_weight",				&battle_config.mail_attachment_weight,			2000,	0,		INT32_MAX,		},
	{ "banana_bomb_duration",				&battle_config.banana_bomb_duration,			0,		0,		UINT16_MAX,		},
	{ "guild_leaderchange_delay",			&battle_config.guild_leaderchange_delay,		1440,	0,		INT32_MAX,		},
	{ "guild_leaderchange_woe",				&battle_config.guild_leaderchange_woe,			0,		0,		1,				},
	{ "guild_alliance_onlygm",              &battle_config.guild_alliance_onlygm,           0,      0,      1, },
	{ "feature.achievement",                &battle_config.feature_achievement,             1,      0,      1,              },
	{ "allow_bound_sell",                   &battle_config.allow_bound_sell,                0,      0,      0xF,            },
	{ "autoloot_adjust",                    &battle_config.autoloot_adjust,                 0,      0,      1,              },
	{ "feature.petevolution",               &battle_config.feature_petevolution,            1,      0,      1,              },
	{ "feature.petautofeed",                &battle_config.feature_pet_autofeed,            1,      0,      1,              },
	{ "feature.pet_autofeed_rate",          &battle_config.feature_pet_autofeed_rate,      89,      0,    100,              },
	{ "pet_autofeed_always",                &battle_config.pet_autofeed_always,             1,      0,      1,              },
	{ "broadcast_hide_name",                &battle_config.broadcast_hide_name,             2,      0,      NAME_LENGTH,    },
	{ "skill_drop_items_full",              &battle_config.skill_drop_items_full,           0,      0,      1,              },
	{ "switch_remove_edp",                  &battle_config.switch_remove_edp,               2,      0,      3,              },
	{ "feature.homunculus_autofeed",        &battle_config.feature_homunculus_autofeed,     1,      0,      1,              },
	{ "feature.homunculus_autofeed_rate",   &battle_config.feature_homunculus_autofeed_rate,30,     0,    100,              },
	{ "summoner_race",                      &battle_config.summoner_race,                   RC_PLAYER_DORAM,      RC_FORMLESS,      RC_PLAYER_DORAM,              },
	{ "summoner_size",                      &battle_config.summoner_size,                   SZ_SMALL,                SZ_SMALL,               SZ_BIG,              },
	{ "homunculus_autofeed_always",         &battle_config.homunculus_autofeed_always,      1,      0,      1,              },
	{ "feature.attendance",                 &battle_config.feature_attendance,              1,      0,      1,              },
	{ "feature.privateairship",             &battle_config.feature_privateairship,          1,      0,      1,              },
	{ "rental_transaction",                 &battle_config.rental_transaction,              1,      0,      1,              },
	{ "min_shop_buy",                       &battle_config.min_shop_buy,                    1,      0,      INT_MAX,        },
	{ "min_shop_sell",                      &battle_config.min_shop_sell,                   0,      0,      INT_MAX,        },
	{ "feature.equipswitch",                &battle_config.feature_equipswitch,             1,      0,      1,              },
	{ "pet_walk_speed",                     &battle_config.pet_walk_speed,                  1,      1,      3,              },
	{ "blacksmith_fame_refine_threshold",   &battle_config.blacksmith_fame_refine_threshold,10,     1,      MAX_REFINE,     },
	{ "mob_nopc_idleskill_rate",            &battle_config.mob_nopc_idleskill_rate,         100,    0,    100,              },
	{ "mob_nopc_move_rate",                 &battle_config.mob_nopc_move_rate,              100,    0,    100,              },
	{ "boss_nopc_idleskill_rate",           &battle_config.boss_nopc_idleskill_rate,        100,    0,    100,              },
	{ "boss_nopc_move_rate",                &battle_config.boss_nopc_move_rate,             100,    0,    100,              },
	{ "hom_idle_no_share",                  &battle_config.hom_idle_no_share,               0,      0,      INT_MAX,        },
	{ "idletime_hom_option",                &battle_config.idletime_hom_option,             0x1F,   0x1,    0xFFF,          },
	{ "devotion_standup_fix",               &battle_config.devotion_standup_fix,            1,      0,      1,              },
	{ "feature.bgqueue",                    &battle_config.feature_bgqueue,                 1,      0,      1,              },
	{ "bgqueue_nowarp_mapflag",             &battle_config.bgqueue_nowarp_mapflag,          0,      0,      1,              },
	{ "homunculus_exp_gain",                &battle_config.homunculus_exp_gain,             10,     0,      100,            },
	{ "rental_item_novalue",                &battle_config.rental_item_novalue,             1,      0,      1,              },
	{ "ping_timer_inverval",                &battle_config.ping_timer_interval,             30,     0,      99999999,       },
	{ "ping_time",                          &battle_config.ping_time,                       20,     0,      99999999,       },
	{ "show_skill_scale",                   &battle_config.show_skill_scale,                1,      0,      1,              },
	{ "achievement_mob_share",              &battle_config.achievement_mob_share,           0,      0,      1,              },
	{ "slave_stick_with_master",            &battle_config.slave_stick_with_master,         0,      0,      1,              },
	{ "at_logout_event",                    &battle_config.at_logout_event,                 1,      0,      1,              },
	{ "homunculus_starving_rate",           &battle_config.homunculus_starving_rate,        10,     0,      100,            },
	{ "homunculus_starving_delay",          &battle_config.homunculus_starving_delay,       20000,  0,      INT_MAX,        },
	{ "drop_connection_on_quit",            &battle_config.drop_connection_on_quit,         0,      0,      1,              },
	{ "mob_spawn_variance",                 &battle_config.mob_spawn_variance,              1,      0,      3,              },
	{ "mercenary_autoloot",                 &battle_config.mercenary_autoloot,              0,      0,      1,              },
	{ "mer_idle_no_share" ,                 &battle_config.mer_idle_no_share,               0,      0,      INT_MAX,        },
	{ "idletime_mer_option",                &battle_config.idletime_mer_option,             0x1F,   0x1,    0xFFF,          },
	{ "feature.refineui",                   &battle_config.feature_refineui,                1,      0,      1,              },
	{ "rndopt_drop_pillar",                 &battle_config.rndopt_drop_pillar,              1,      0,      1,              },
	{ "pet_legacy_formula",                 &battle_config.pet_legacy_formula,              0,      0,      1,              },
	{ "pet_distance_check",                 &battle_config.pet_distance_check,              5,      0,      50,             },
	{ "pet_hide_check",                     &battle_config.pet_hide_check,                  1,      0,      1,              },
	{ "instance_block_leave",               &battle_config.instance_block_leave,            1,      0,      1,              },
	{ "instance_block_leaderchange",        &battle_config.instance_block_leaderchange,     1,      0,      1,              },
	{ "instance_block_invite",              &battle_config.instance_block_invite,           1,      0,      1,              },
	{ "instance_block_expulsion",           &battle_config.instance_block_expulsion,        1,      0,      1,              },

	// 4th Job Stuff
	{ "use_traitpoint_table",               &battle_config.use_traitpoint_table,            1,      0,      1,              },
	{ "trait_points_job_change",            &battle_config.trait_points_job_change,         7,      1,      1000,           },
	{ "max_trait_parameter",                &battle_config.max_trait_parameter,             100,    10,     SHRT_MAX,       },
	{ "max_res_mres_ignored",               &battle_config.max_res_mres_ignored,            50,     0,      100,            },
	{ "max_ap",                             &battle_config.max_ap,                          200,    100,    1000000000,     },
	{ "ap_rate",                            &battle_config.ap_rate,                         100,    1,      INT_MAX,        },
	{ "restart_ap_rate",                    &battle_config.restart_ap_rate,                 0,      0,      100,            },
	{ "loose_ap_on_death",                  &battle_config.loose_ap_on_death,               1,      0,      1,              },
	{ "loose_ap_on_map",                    &battle_config.loose_ap_on_map,                 1,      0,      1,              },
	{ "keep_ap_on_logout",                  &battle_config.keep_ap_on_logout,               1,      0,      1,              },
	{ "attack_machine_level_difference",    &battle_config.attack_machine_level_difference, 15,     0,      INT_MAX,        },

	{ "feature.barter",                     &battle_config.feature_barter,                  1,      0,      1,              },
	{ "feature.barter_extended",            &battle_config.feature_barter_extended,         1,      0,      1,              },
	{ "feature.itemlink",                   &battle_config.feature_itemlink,                1,      0,      1,              },
	{ "feature.mesitemlink",                &battle_config.feature_mesitemlink,             1,      0,      1,              },
	{ "feature.mesitemlink_brackets",       &battle_config.feature_mesitemlink_brackets,    0,      0,      1,              },
	{ "feature.mesitemlink_dbname",         &battle_config.feature_mesitemlink_dbname,      0,      0,      1,              },
	{ "break_mob_equip",                    &battle_config.break_mob_equip,                 0,      0,      1,              },
	{ "macro_detection_retry",              &battle_config.macro_detection_retry,           3,      1,      INT_MAX,        },
	{ "macro_detection_timeout",            &battle_config.macro_detection_timeout,         60000,  0,      INT_MAX,        },
	{ "macro_detection_punishment",         &battle_config.macro_detection_punishment,      0,      0,      1,              },
	{ "macro_detection_punishment_time",    &battle_config.macro_detection_punishment_time, 0,      0,      INT_MAX,        },

	{ "feature.dynamicnpc_timeout",         &battle_config.feature_dynamicnpc_timeout,      1000,   60000,  INT_MAX,        },
	{ "feature.dynamicnpc_rangex",          &battle_config.feature_dynamicnpc_rangex,       2,      0,      INT_MAX,        },
	{ "feature.dynamicnpc_rangey",          &battle_config.feature_dynamicnpc_rangey,       2,      0,      INT_MAX,        },
	{ "feature.dynamicnpc_direction",       &battle_config.feature_dynamicnpc_direction,    0,      0,      1,              },

	{ "mob_respawn_time",                   &battle_config.mob_respawn_time,                1000,   1000,   INT_MAX,        },
	{ "mob_unlock_time",                    &battle_config.mob_unlock_time,                 2000,   0,      INT_MAX,        },
	{ "map_edge_size",                      &battle_config.map_edge_size,                   15,     1,      40,             },
	{ "randomize_center_cell",              &battle_config.randomize_center_cell,           1,      0,      1,              },

	{ "feature.stylist",                    &battle_config.feature_stylist,                 1,      0,      1,              },
	{ "feature.banking_state_enforce",      &battle_config.feature_banking_state_enforce,   0,      0,      1,              },
#ifdef RENEWAL
	{ "feature.instance_allow_reconnect",   &battle_config.instance_allow_reconnect,        1,      0,      1,              },
#else
	{ "feature.instance_allow_reconnect",   &battle_config.instance_allow_reconnect,        0,      0,      1,              },
#endif
	{ "synchronize_damage",                 &battle_config.synchronize_damage,              0,      0,      1,              },
	{ "item_stacking",                      &battle_config.item_stacking,                   1,      0,      1,              },
#ifdef RENEWAL
	{ "hom_delay_reset_vaporize",           &battle_config.hom_delay_reset_vaporize,        0,      0,      1,              },
	{ "hom_delay_reset_warp",               &battle_config.hom_delay_reset_warp,            0,      0,      1,              },
#else
	{ "hom_delay_reset_vaporize",           &battle_config.hom_delay_reset_vaporize,        1,      0,      1,              },
	{ "hom_delay_reset_warp",               &battle_config.hom_delay_reset_warp,            1,      0,      1,              },
#endif
	{ "feature.goldpc_active",              &battle_config.feature_goldpc_active,           1,      0,      1,              },
	{ "feature.goldpc_time",                &battle_config.feature_goldpc_time,          3600,      0,   3600,              },
	{ "feature.goldpc_max_points",          &battle_config.feature_goldpc_max_points,     300,      0,    300,              },
	{ "feature.goldpc_vip",                 &battle_config.feature_goldpc_vip,              1,      0,      1,              },

#include <custom/battle_config_init.inc>
};

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
	battle_config.max_summoner_aspd = 2000 - battle_config.max_summoner_aspd * 10;
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

#if PACKETVER < 20100000
	if( battle_config.feature_mesitemlink ){
		ShowWarning( "conf/battle/feature.conf:mesitemlink is enabled but it requires PACKETVER 2010-01-01 or newer, disabling...\n" );
		battle_config.feature_mesitemlink = 0;
	}
#elif PACKETVER == 20151029 || PACKETVER == 20151104
	// The feature is broken on those two clients or maybe even more. For more details check ItemDatabase::create_item_link_for_mes [Lemongrass]
	if( battle_config.feature_mesitemlink ){
		ShowWarning( "conf/battle/feature.conf:mesitemlink is enabled but it is broken on this specific PACKETVER, disabling...\n" );
		battle_config.feature_mesitemlink = 0;
	}
#endif

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

#if PACKETVER < 20120101
	if (battle_config.feature_bgqueue) {
		ShowWarning("conf/battle/feature.conf:bgqueue is enabled but it requires PACKETVER 2012-01-01 or newer, disabling...\n");
		battle_config.feature_bgqueue = 0;
	}
#endif

#if PACKETVER > 20120000 && PACKETVER < 20130515 /* Exact date (when it started) not known */
	if (battle_config.feature_auction) {
		ShowWarning("conf/battle/feature.conf:feature.auction is enabled but it is not stable on PACKETVER " EXPAND_AND_QUOTE(PACKETVER) ", disabling...\n");
		ShowWarning("conf/battle/feature.conf:feature.auction change value to '2' to silence this warning and maintain it enabled\n");
		battle_config.feature_auction = 0;
	}
#elif PACKETVER >= 20141112
	if (battle_config.feature_auction) {
		ShowWarning("conf/battle/feature.conf:feature.auction is enabled but it is not available for clients from 2014-11-12 on, disabling...\n");
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

#if PACKETVER < 20131223
	if (battle_config.mvp_exp_reward_message) {
		ShowWarning("conf/battle/client.conf MVP EXP reward message is enabled but it requires PACKETVER 2013-12-23 or newer, disabling...\n");
		battle_config.mvp_exp_reward_message = 0;
	}
#endif

#if PACKETVER < 20141022
	if (battle_config.feature_roulette) {
		ShowWarning("conf/battle/feature.conf roulette is enabled but it requires PACKETVER 2014-10-22 or newer, disabling...\n");
		battle_config.feature_roulette = 0;
	}
#endif

#if PACKETVER < 20150513
	if (battle_config.feature_achievement) {
		ShowWarning("conf/battle/feature.conf achievement is enabled but it requires PACKETVER 2015-05-13 or newer, disabling...\n");
		battle_config.feature_achievement = 0;
	}
#endif

#if PACKETVER < 20151104
	if( battle_config.feature_itemlink ){
		ShowWarning( "conf/battle/feature.conf:itemlink is enabled but it requires PACKETVER 2015-11-04 or newer, disabling...\n" );
		battle_config.feature_itemlink = 0;
	}
#endif

#if PACKETVER < 20151104
	if( battle_config.feature_stylist ){
		ShowWarning("conf/battle/feature.conf stylist is enabled but it requires PACKETVER 2015-11-04 or newer, disabling...\n");
		battle_config.feature_stylist = 0;
	}
#endif

#if PACKETVER < 20141008
	if (battle_config.feature_petevolution) {
		ShowWarning("conf/battle/feature.conf petevolution is enabled but it requires PACKETVER 2014-10-08 or newer, disabling...\n");
		battle_config.feature_petevolution = 0;
	}
	if (battle_config.feature_pet_autofeed) {
		ShowWarning("conf/battle/feature.conf pet auto feed is enabled but it requires PACKETVER 2014-10-08 or newer, disabling...\n");
		battle_config.feature_pet_autofeed = 0;
	}
#endif

#if PACKETVER < 20161012
	if (battle_config.feature_refineui) {
		ShowWarning("conf/battle/feature.conf refine UI is enabled but it requires PACKETVER 2016-10-12 or newer, disabling...\n");
		battle_config.feature_refineui = 0;
	}
#endif

#if PACKETVER < 20170208
	if (battle_config.feature_equipswitch) {
		ShowWarning("conf/battle/feature.conf equip switch is enabled but it requires PACKETVER 2017-02-08 or newer, disabling...\n");
		battle_config.feature_equipswitch = 0;
	}
#endif

#if PACKETVER < 20170920
	if( battle_config.feature_homunculus_autofeed ){
		ShowWarning("conf/battle/feature.conf homunculus autofeeding is enabled but it requires PACKETVER 2017-09-20 or newer, disabling...\n");
		battle_config.feature_homunculus_autofeed = 0;
	}
#endif

#if PACKETVER < 20180307
	if( battle_config.feature_attendance ){
		ShowWarning("conf/battle/feature.conf attendance system is enabled but it requires PACKETVER 2018-03-07 or newer, disabling...\n");
		battle_config.feature_attendance = 0;
	}
#endif

#if PACKETVER < 20180321
	if( battle_config.feature_privateairship ){
		ShowWarning("conf/battle/feature.conf private airship system is enabled but it requires PACKETVER 2018-03-21 or newer, disabling...\n");
		battle_config.feature_privateairship = 0;
	}
#endif

#if !( PACKETVER_MAIN_NUM >= 20190116 || PACKETVER_RE_NUM >= 20190116 || PACKETVER_ZERO_NUM >= 20181226 )
	if( battle_config.feature_barter ){
		ShowWarning("conf/battle/feature.conf barter shop system is enabled but it requires PACKETVER 2019-01-16 or newer, disabling...\n");
		battle_config.feature_barter = 0;
	}
#endif

#if !( PACKETVER_MAIN_NUM >= 20191120 || PACKETVER_RE_NUM >= 20191106 || PACKETVER_ZERO_NUM >= 20191127 )
	if( battle_config.feature_barter_extended ){
		ShowWarning("conf/battle/feature.conf extended barter shop system is enabled but it requires PACKETVER 2019-11-06 or newer, disabling...\n");
		battle_config.feature_barter_extended = 0;
	}
#endif

#ifndef CELL_NOSTACK
	if (battle_config.custom_cell_stack_limit != 1)
		ShowWarning("Battle setting 'custom_cell_stack_limit' takes no effect as this server was compiled without Cell Stack Limit support.\n");
#endif

#ifdef MAP_GENERATOR
	battle_config.dynamic_mobs = 1;
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
	if (fp == nullptr)
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
			else if( strcmpi( w1, "atcommand_symbol" ) == 0 ){
				const char* symbol = &w2[0];

				if (ISPRINT(*symbol) && // no control characters
					*symbol != '/' && // symbol of client commands
					*symbol != '%' && // symbol of party chat
					*symbol != '$' && // symbol of guild chat
					*symbol != charcommand_symbol)
					atcommand_symbol = *symbol;
			}else if( strcmpi( w1, "charcommand_symbol" ) == 0 ){
				const char* symbol = &w2[0];

				if (ISPRINT(*symbol) && // no control characters
					*symbol != '/' && // symbol of client commands
					*symbol != '%' && // symbol of party chat
					*symbol != '$' && // symbol of guild chat
					*symbol != atcommand_symbol)
					charcommand_symbol = *symbol;
			}else if( battle_set_value(w1, w2) == 0 )
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
	delay_damage_ers = ers_new(sizeof(struct delay_damage),"battle.cpp::delay_damage_ers",ERS_OPT_CLEAR);
	add_timer_func_list(battle_delay_damage_sub, "battle_delay_damage_sub");
}

/*==================
 * end battle timer
 *------------------*/
void do_final_battle(void)
{
	ers_destroy(delay_damage_ers);
}
