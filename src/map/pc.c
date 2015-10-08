// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/core.h" // get_svn_revision()
#include "../common/malloc.h"
#include "../common/nullpo.h"
#include "../common/random.h"
#include "../common/showmsg.h"
#include "../common/socket.h" // session[]
#include "../common/strlib.h" // safestrncpy()
#include "../common/timer.h"
#include "../common/utils.h"
#include "../common/mmo.h" //NAME_LENGTH

#include "atcommand.h" // get_atcommand_level()
#include "map.h"
#include "battle.h" // battle_config
#include "battleground.h"
#include "channel.h"
#include "chat.h"
#include "chrif.h"
#include "date.h" // is_day_of_*()
#include "duel.h"
#include "intif.h"
#include "homunculus.h"
#include "instance.h"
#include "mercenary.h"
#include "elemental.h"
#include "pet.h" // pet_unlocktarget()
#include "party.h" // party_search()
#include "storage.h"

#include <stdlib.h>
#include <math.h>

int pc_split_atoui(char* str, unsigned int* val, char sep, int max);

#define PVP_CALCRANK_INTERVAL 1000	// PVP calculation interval

static unsigned int statp[MAX_LEVEL+1];
#if defined(RENEWAL_DROP) || defined(RENEWAL_EXP)
static unsigned int level_penalty[3][CLASS_MAX][MAX_LEVEL*2+1];
#endif

// h-files are for declarations, not for implementations... [Shinomori]
struct skill_tree_entry skill_tree[CLASS_COUNT][MAX_SKILL_TREE];
// timer for night.day implementation
int day_timer_tid = INVALID_TIMER;
int night_timer_tid = INVALID_TIMER;

struct eri *pc_sc_display_ers = NULL;
struct eri *pc_itemgrouphealrate_ers = NULL;
int pc_expiration_tid = INVALID_TIMER;

struct fame_list smith_fame_list[MAX_FAME_LIST];
struct fame_list chemist_fame_list[MAX_FAME_LIST];
struct fame_list taekwon_fame_list[MAX_FAME_LIST];

static unsigned int equip_pos[EQI_MAX]={EQP_ACC_L,EQP_ACC_R,EQP_SHOES,EQP_GARMENT,EQP_HEAD_LOW,EQP_HEAD_MID,EQP_HEAD_TOP,EQP_ARMOR,EQP_HAND_L,EQP_HAND_R,EQP_COSTUME_HEAD_TOP,EQP_COSTUME_HEAD_MID,EQP_COSTUME_HEAD_LOW,EQP_COSTUME_GARMENT,EQP_AMMO,EQP_SHADOW_ARMOR,EQP_SHADOW_WEAPON,EQP_SHADOW_SHIELD,EQP_SHADOW_SHOES,EQP_SHADOW_ACC_R,EQP_SHADOW_ACC_L};

#define MOTD_LINE_SIZE 128
static char motd_text[MOTD_LINE_SIZE][CHAT_SIZE_MAX]; // Message of the day buffer [Valaris]

//Links related info to the sd->hate_mob[]/sd->feel_map[] entries
const struct sg_data sg_info[MAX_PC_FEELHATE] = {
		{ SG_SUN_ANGER, SG_SUN_BLESS, SG_SUN_COMFORT, "PC_FEEL_SUN", "PC_HATE_MOB_SUN", is_day_of_sun },
		{ SG_MOON_ANGER, SG_MOON_BLESS, SG_MOON_COMFORT, "PC_FEEL_MOON", "PC_HATE_MOB_MOON", is_day_of_moon },
		{ SG_STAR_ANGER, SG_STAR_BLESS, SG_STAR_COMFORT, "PC_FEEL_STAR", "PC_HATE_MOB_STAR", is_day_of_star }
	};

/**
 * Item Cool Down Delay Saving
 * Struct item_cd is not a member of struct map_session_data
 * to keep cooldowns in memory between player log-ins.
 * All cooldowns are reset when server is restarted.
 **/
DBMap* itemcd_db = NULL; // char_id -> struct item_cd
struct item_cd {
	unsigned int tick[MAX_ITEMDELAYS]; //tick
	unsigned short nameid[MAX_ITEMDELAYS]; //item id
};

/**
* Converts a class to its array index for CLASS_COUNT defined arrays.
* Note that it does not do a validity check for speed purposes, where parsing
* player input make sure to use a pcdb_checkid first!
* @param class_
* @return Class Index
*/
int pc_class2idx(int class_) {
	if (class_ >= JOB_NOVICE_HIGH)
		return class_- JOB_NOVICE_HIGH+JOB_MAX_BASIC;
	return class_;
}

/**
* Get player's group ID
* @param sd
* @return Group ID
*/
inline int pc_get_group_id(struct map_session_data *sd) {
	return sd->group_id;
}

/** Get player's group Level
* @param sd
* @return Group Level
*/
inline int pc_get_group_level(struct map_session_data *sd) {
	return sd->group_level;
}

static int pc_invincible_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd;

	if( (sd=(struct map_session_data *)map_id2sd(id)) == NULL || sd->bl.type!=BL_PC )
		return 1;

	if(sd->invincible_timer != tid){
		ShowError("invincible_timer %d != %d\n",sd->invincible_timer,tid);
		return 0;
	}
	sd->invincible_timer = INVALID_TIMER;
	skill_unit_move(&sd->bl,tick,1);

	return 0;
}

void pc_setinvincibletimer(struct map_session_data* sd, int val) {
	nullpo_retv(sd);

	if( sd->invincible_timer != INVALID_TIMER )
		delete_timer(sd->invincible_timer,pc_invincible_timer);
	sd->invincible_timer = add_timer(gettick()+val,pc_invincible_timer,sd->bl.id,0);
}

void pc_delinvincibletimer(struct map_session_data* sd)
{
	nullpo_retv(sd);

	if( sd->invincible_timer != INVALID_TIMER )
	{
		delete_timer(sd->invincible_timer,pc_invincible_timer);
		sd->invincible_timer = INVALID_TIMER;
		skill_unit_move(&sd->bl,gettick(),1);
	}
}

static int pc_spiritball_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd;
	int i;

	if( (sd=(struct map_session_data *)map_id2sd(id)) == NULL || sd->bl.type!=BL_PC )
		return 1;

	if( sd->spiritball <= 0 )
	{
		ShowError("pc_spiritball_timer: %d spiritball's available. (aid=%d cid=%d tid=%d)\n", sd->spiritball, sd->status.account_id, sd->status.char_id, tid);
		sd->spiritball = 0;
		return 0;
	}

	ARR_FIND(0, sd->spiritball, i, sd->spirit_timer[i] == tid);
	if( i == sd->spiritball )
	{
		ShowError("pc_spiritball_timer: timer not found (aid=%d cid=%d tid=%d)\n", sd->status.account_id, sd->status.char_id, tid);
		return 0;
	}

	sd->spiritball--;
	if( i != sd->spiritball )
		memmove(sd->spirit_timer+i, sd->spirit_timer+i+1, (sd->spiritball-i)*sizeof(int));
	sd->spirit_timer[sd->spiritball] = INVALID_TIMER;

	clif_spiritball(&sd->bl);

	return 0;
}

/**
* Adds a spiritball to player for 'interval' ms
* @param sd
* @param interval
* @param max
*/
void pc_addspiritball(struct map_session_data *sd,int interval,int max)
{
	int tid;
	uint8 i;

	nullpo_retv(sd);

	if(max > MAX_SPIRITBALL)
		max = MAX_SPIRITBALL;
	if(sd->spiritball < 0)
		sd->spiritball = 0;

	if( sd->spiritball && sd->spiritball >= max )
	{
		if(sd->spirit_timer[0] != INVALID_TIMER)
			delete_timer(sd->spirit_timer[0],pc_spiritball_timer);
		sd->spiritball--;
		if( sd->spiritball != 0 )
			memmove(sd->spirit_timer+0, sd->spirit_timer+1, (sd->spiritball)*sizeof(int));
		sd->spirit_timer[sd->spiritball] = INVALID_TIMER;
	}

	tid = add_timer(gettick()+interval, pc_spiritball_timer, sd->bl.id, 0);
	ARR_FIND(0, sd->spiritball, i, sd->spirit_timer[i] == INVALID_TIMER || DIFF_TICK(get_timer(tid)->tick, get_timer(sd->spirit_timer[i])->tick) < 0);
	if( i != sd->spiritball )
		memmove(sd->spirit_timer+i+1, sd->spirit_timer+i, (sd->spiritball-i)*sizeof(int));
	sd->spirit_timer[i] = tid;
	sd->spiritball++;
	if( (sd->class_&MAPID_THIRDMASK) == MAPID_ROYAL_GUARD )
		clif_millenniumshield(&sd->bl,sd->spiritball);
	else
		clif_spiritball(&sd->bl);
}

/**
* Removes number of spiritball from player
* @param sd
* @param count
* @param type 1 = doesn't give client effect
*/
void pc_delspiritball(struct map_session_data *sd,int count,int type)
{
	uint8 i;

	nullpo_retv(sd);

	if(sd->spiritball <= 0) {
		sd->spiritball = 0;
		return;
	}

	if(count == 0)
		return;
	if(count > sd->spiritball)
		count = sd->spiritball;
	sd->spiritball -= count;
	if(count > MAX_SPIRITBALL)
		count = MAX_SPIRITBALL;

	for(i=0;i<count;i++) {
		if(sd->spirit_timer[i] != INVALID_TIMER) {
			delete_timer(sd->spirit_timer[i],pc_spiritball_timer);
			sd->spirit_timer[i] = INVALID_TIMER;
		}
	}
	for(i=count;i<MAX_SPIRITBALL;i++) {
		sd->spirit_timer[i-count] = sd->spirit_timer[i];
		sd->spirit_timer[i] = INVALID_TIMER;
	}

	if(!type) {
		if( (sd->class_&MAPID_THIRDMASK) == MAPID_ROYAL_GUARD )
			clif_millenniumshield(&sd->bl,sd->spiritball);
		else
			clif_spiritball(&sd->bl);
	}
}

static int pc_check_banding( struct block_list *bl, va_list ap ) {
	int *c, *b_sd;
	struct block_list *src;
	struct map_session_data *tsd;
	struct status_change *sc;

	nullpo_ret(bl);
	nullpo_ret(tsd = (struct map_session_data*)bl);
	nullpo_ret(src = va_arg(ap,struct block_list *));
	c = va_arg(ap,int *);
	b_sd = va_arg(ap, int *);

	if(pc_isdead(tsd))
		return 0;

	sc = status_get_sc(bl);

	if( bl == src )
		return 0;

	if( sc && sc->data[SC_BANDING] )
	{
		b_sd[(*c)++] = tsd->bl.id;
		return 1;
	}

	return 0;
}

int pc_banding(struct map_session_data *sd, uint16 skill_lv) {
	int c;
	int b_sd[MAX_PARTY]; // In case of a full Royal Guard party.
	int i, j, hp, extra_hp = 0, tmp_qty = 0;
	int range = skill_get_splash(LG_BANDING,skill_lv);

	nullpo_ret(sd);

	c = 0;
	memset(b_sd, 0, sizeof(b_sd));
	i = party_foreachsamemap(pc_check_banding,sd,range,&sd->bl,&c,&b_sd);

	if( c < 1 ) //just recalc status no need to recalc hp
	{	// No more Royal Guards in Banding found.
		struct status_change *sc;
		if( (sc = status_get_sc(&sd->bl)) != NULL  && sc->data[SC_BANDING] )
		{
			sc->data[SC_BANDING]->val2 = 0; // Reset the counter
			status_calc_bl(&sd->bl, status_sc2scb_flag(SC_BANDING));
		}
		return 0;
	}

	//Add yourself
	hp = status_get_hp(&sd->bl);
	i++;

	// Get total HP of all Royal Guards in party.
	for( j = 0; j < i; j++ ){
		struct map_session_data *bsd = map_id2sd(b_sd[j]);
		if( bsd != NULL )
			hp += status_get_hp(&bsd->bl);
	}

	// Set average HP.
	hp = hp / i;

	// If a Royal Guard have full HP, give more HP to others that haven't full HP.
	for( j = 0; j < i; j++ )
	{
		int tmp_hp=0;
		struct map_session_data *bsd = map_id2sd(b_sd[j]);
		if( bsd != NULL && (tmp_hp = hp - status_get_max_hp(&bsd->bl)) > 0 ){
			extra_hp += tmp_hp;
			tmp_qty++;
		}
	}

	if( extra_hp > 0 && tmp_qty > 0 )
		hp += extra_hp / tmp_qty;

	for( j = 0; j < i; j++ ){
		struct map_session_data *bsd = map_id2sd(b_sd[j]);
		if( bsd != NULL )
		{
			struct status_change *sc;
			status_set_hp(&bsd->bl,hp,0);	// Set hp
			if( (sc = status_get_sc(&bsd->bl)) != NULL  && sc->data[SC_BANDING] )
			{
				sc->data[SC_BANDING]->val2 = c; // Set the counter. It doesn't count your self.
				status_calc_bl(&bsd->bl, status_sc2scb_flag(SC_BANDING));	// Set atk and def.
			}
		}
	}

	return c;
}

/**
* Increases a player's fame points and displays a notice to him
* @param sd Player
* @param count Fame point
*/
void pc_addfame(struct map_session_data *sd,int count)
{
	int ranktype=-1;
	nullpo_retv(sd);
	sd->status.fame += count;
	if(sd->status.fame > MAX_FAME)
		sd->status.fame = MAX_FAME;

	switch(sd->class_&MAPID_UPPERMASK){
		case MAPID_BLACKSMITH:	ranktype = 0; break;
		case MAPID_ALCHEMIST:	ranktype = 1; break;
		case MAPID_TAEKWON:		ranktype = 2; break;
	}

	clif_update_rankingpoint(sd,ranktype,count);
	chrif_updatefamelist(sd);
}

/**
 * Check whether a player ID is in the fame rankers list of its job, returns his/her position if so, 0 else
 * @param sd
 * @param job Job use enum e_mapid
 * @return Rank
 */
unsigned char pc_famerank(uint32 char_id, int job)
{
	uint8 i;

	switch(job){
		case MAPID_BLACKSMITH: // Blacksmith
		    for(i = 0; i < MAX_FAME_LIST; i++){
				if(smith_fame_list[i].id == char_id)
				    return i + 1;
			}
			break;
		case MAPID_ALCHEMIST: // Alchemist
			for(i = 0; i < MAX_FAME_LIST; i++){
				if(chemist_fame_list[i].id == char_id)
					return i + 1;
			}
			break;
		case MAPID_TAEKWON: // Taekwon
			for(i = 0; i < MAX_FAME_LIST; i++){
				if(taekwon_fame_list[i].id == char_id)
					return i + 1;
			}
			break;
	}

	return 0;
}

/**
* Restart player's HP & SP value
* @param sd
* @param type Restart type: 1 - Normal Resurection
*/
void pc_setrestartvalue(struct map_session_data *sd, char type) {
	struct status_data *status, *b_status;
	nullpo_retv(sd);

	b_status = &sd->base_status;
	status = &sd->battle_status;

	if (type&1) {	//Normal resurrection
		status->hp = 1; //Otherwise status_heal may fail if dead.
		status_heal(&sd->bl, b_status->hp, 0, 1);
		if( status->sp < b_status->sp )
			status_set_sp(&sd->bl, b_status->sp, 1);
	} else { //Just for saving on the char-server (with values as if respawned)
		sd->status.hp = b_status->hp;
		sd->status.sp = (status->sp < b_status->sp)?b_status->sp:status->sp;
	}
}

/*==========================================
	Rental System
 *------------------------------------------*/

/** Ends rental */
static int pc_inventory_rental_end(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd = map_id2sd(id);

	if( sd == NULL )
		return 0;
	if( tid != sd->rental_timer ) {
		ShowError("pc_inventory_rental_end: invalid timer id.\n");
		return 0;
	}

	pc_inventory_rentals(sd);
	return 1;
}

void pc_inventory_rental_clear(struct map_session_data *sd)
{
	if( sd->rental_timer != INVALID_TIMER ) {
		delete_timer(sd->rental_timer, pc_inventory_rental_end);
		sd->rental_timer = INVALID_TIMER;
	}
}

/** Assumes I is valid (from default areas where it is called, it is) */
void pc_rental_expire(struct map_session_data *sd, int i)
{
	unsigned short nameid = sd->status.inventory[i].nameid;

	/* Soon to be dropped, we got plans to integrate it with item db */
	switch( nameid ) {
		case ITEMID_REINS_OF_MOUNT:
			if( &sd->sc && sd->sc.data[SC_ALL_RIDING] )
				status_change_end(&sd->bl, SC_ALL_RIDING, INVALID_TIMER);
			break;
		case ITEMID_LOVE_ANGEL:
			if( sd->status.font == 1 ) {
				sd->status.font = 0;
				clif_font(sd);
			}
			break;
		case ITEMID_SQUIRREL:
			if( sd->status.font == 2 ) {
				sd->status.font = 0;
				clif_font(sd);
			}
			break;
		case ITEMID_GOGO:
			if( sd->status.font == 3 ) {
				sd->status.font = 0;
				clif_font(sd);
			}
			break;
		case ITEMID_PICTURE_DIARY:
			if( sd->status.font == 4 ) {
				sd->status.font = 0;
				clif_font(sd);
			}
			break;
		case ITEMID_MINI_HEART:
			if( sd->status.font == 5 ) {
				sd->status.font = 0;
				clif_font(sd);
			}
			break;
		case ITEMID_NEWCOMER:
			if( sd->status.font == 6 ) {
				sd->status.font = 0;
				clif_font(sd);
			}
			break;
		case ITEMID_KID:
			if( sd->status.font == 7 ) {
				sd->status.font = 0;
				clif_font(sd);
			}
			break;
		case ITEMID_MAGIC_CASTLE:
			if( sd->status.font == 8 ) {
				sd->status.font = 0;
				clif_font(sd);
			}
			break;
		case ITEMID_BULGING_HEAD:
			if( sd->status.font == 9 ) {
				sd->status.font = 0;
				clif_font(sd);
			}
			break;
	}
	clif_rental_expired(sd->fd, i, sd->status.inventory[i].nameid);
	pc_delitem(sd, i, sd->status.inventory[i].amount, 0, 0, LOG_TYPE_OTHER);
}

void pc_inventory_rentals(struct map_session_data *sd)
{
	int i, c = 0;
	unsigned int expire_tick, next_tick = UINT_MAX;

	for( i = 0; i < MAX_INVENTORY; i++ ) { // Check for Rentals on Inventory
		if( sd->status.inventory[i].nameid == 0 )
			continue; // Nothing here
		if( sd->status.inventory[i].expire_time == 0 )
			continue;
		if( sd->status.inventory[i].expire_time <= time(NULL) )
			pc_rental_expire(sd, i);
		else {
			expire_tick = (unsigned int)(sd->status.inventory[i].expire_time - time(NULL)) * 1000;
			clif_rental_time(sd->fd, sd->status.inventory[i].nameid, (int)(expire_tick / 1000));
			next_tick = min(expire_tick, next_tick);
			c++;
		}
	}

	if( c > 0 ) // min(next_tick,3600000) 1 hour each timer to keep announcing to the owner, and to avoid a but with rental time > 15 days
		sd->rental_timer = add_timer(gettick() + min(next_tick,3600000), pc_inventory_rental_end, sd->bl.id, 0);
	else
		sd->rental_timer = INVALID_TIMER;
}

void pc_inventory_rental_add(struct map_session_data *sd, int seconds)
{
	int tick = seconds * 1000;

	if( sd == NULL )
		return;

	if( sd->rental_timer != INVALID_TIMER ) {
		const struct TimerData * td;

		td = get_timer(sd->rental_timer);
		if( DIFF_TICK(td->tick, gettick()) > tick ) { // Update Timer as this one ends first than the current one
			pc_inventory_rental_clear(sd);
			sd->rental_timer = add_timer(gettick() + tick, pc_inventory_rental_end, sd->bl.id, 0);
		}
	} else
		sd->rental_timer = add_timer(gettick() + min(tick,3600000), pc_inventory_rental_end, sd->bl.id, 0);
}

/**
 * Determines if player can give / drop / trade / vend items
 */
bool pc_can_give_items(struct map_session_data *sd)
{
	return pc_has_permission(sd, PC_PERM_TRADE);
}

/**
 * Determines if player can give / drop / trade / vend bounded items
 */
bool pc_can_give_bounded_items(struct map_session_data *sd)
{
	return pc_has_permission(sd, PC_PERM_TRADE_BOUNDED);
}

/*==========================================
 * Prepares character for saving.
 * @param sd
 *------------------------------------------*/
void pc_makesavestatus(struct map_session_data *sd)
{
	nullpo_retv(sd);

	if(!battle_config.save_clothcolor)
		sd->status.clothes_color=0;

	//Only copy the Cart/Peco/Falcon options, the rest are handled via
	//status change load/saving. [Skotlex]
#ifdef NEW_CARTS
	sd->status.option = sd->sc.option&(OPTION_INVISIBLE|OPTION_FALCON|OPTION_RIDING|OPTION_DRAGON|OPTION_WUG|OPTION_WUGRIDER|OPTION_MADOGEAR);
#else
	sd->status.option = sd->sc.option&(OPTION_INVISIBLE|OPTION_CART|OPTION_FALCON|OPTION_RIDING|OPTION_DRAGON|OPTION_WUG|OPTION_WUGRIDER|OPTION_MADOGEAR);
#endif
	if (sd->sc.data[SC_JAILED]) { //When Jailed, do not move last point.
		if(pc_isdead(sd)){
			pc_setrestartvalue(sd,0);
		} else {
			sd->status.hp = sd->battle_status.hp;
			sd->status.sp = sd->battle_status.sp;
		}
		sd->status.last_point.map = sd->mapindex;
		sd->status.last_point.x = sd->bl.x;
		sd->status.last_point.y = sd->bl.y;
		return;
	}

	if(pc_isdead(sd)){
		pc_setrestartvalue(sd,0);
		memcpy(&sd->status.last_point,&sd->status.save_point,sizeof(sd->status.last_point));
	} else {
		sd->status.hp = sd->battle_status.hp;
		sd->status.sp = sd->battle_status.sp;
		sd->status.last_point.map = sd->mapindex;
		sd->status.last_point.x = sd->bl.x;
		sd->status.last_point.y = sd->bl.y;
	}

	if(map[sd->bl.m].flag.nosave){
		struct map_data *m=&map[sd->bl.m];
		if(m->save.map)
			memcpy(&sd->status.last_point,&m->save,sizeof(sd->status.last_point));
		else
			memcpy(&sd->status.last_point,&sd->status.save_point,sizeof(sd->status.last_point));
	}
}

/*==========================================
 * Off init ? Connection?
 *------------------------------------------*/
void pc_setnewpc(struct map_session_data *sd, uint32 account_id, uint32 char_id, int login_id1, unsigned int client_tick, int sex, int fd)
{
	nullpo_retv(sd);

	sd->bl.id        = account_id;
	sd->status.account_id   = account_id;
	sd->status.char_id      = char_id;
	sd->status.sex   = sex;
	sd->login_id1    = login_id1;
	sd->login_id2    = 0; // at this point, we can not know the value :(
	sd->client_tick  = client_tick;
	sd->state.active = 0; //to be set to 1 after player is fully authed and loaded.
	sd->bl.type      = BL_PC;
	sd->canlog_tick  = gettick();
	//Required to prevent homunculus copuing a base speed of 0.
	sd->battle_status.speed = sd->base_status.speed = DEFAULT_WALK_SPEED;
}

/**
* Get equip point for an equip
* @param sd
* @param n Equip index in inventory
*/
int pc_equippoint(struct map_session_data *sd,int n){
	int ep = 0;

	nullpo_ret(sd);

	if(!sd->inventory_data[n])
		return 0;

	if (!itemdb_isequip2(sd->inventory_data[n]))
		return 0; //Not equippable by players.

	ep = sd->inventory_data[n]->equip;
	if(sd->inventory_data[n]->look == W_DAGGER	||
		sd->inventory_data[n]->look == W_1HSWORD ||
		sd->inventory_data[n]->look == W_1HAXE) {
		if(ep == EQP_HAND_R && (pc_checkskill(sd,AS_LEFT) > 0 || (sd->class_&MAPID_UPPERMASK) == MAPID_ASSASSIN ||
			(sd->class_&MAPID_UPPERMASK) == MAPID_KAGEROUOBORO))//Kagerou and Oboro can dual wield daggers. [Rytech]
			return EQP_ARMS;
	}
	return ep;
}

/**
 * Fill inventory_data with struct *item_data through inventory (fill with struct *item)
 * @param sd : player session
 * @return 0 sucess, 1:invalid sd
 */
void pc_setinventorydata(struct map_session_data *sd)
{
	uint8 i;
	nullpo_retv(sd);

	for(i = 0; i < MAX_INVENTORY; i++) {
		unsigned short id = sd->status.inventory[i].nameid;
		sd->inventory_data[i] = id?itemdb_search(id):NULL;
	}
}

/**
* 'Calculates' weapon type
* @param sd : player
*/
void pc_calcweapontype(struct map_session_data *sd)
{
	nullpo_retv(sd);

	// single-hand
	if(sd->weapontype2 == W_FIST) {
		sd->status.weapon = sd->weapontype1;
		return;
	}
	if(sd->weapontype1 == W_FIST) {
		sd->status.weapon = sd->weapontype2;
		return;
	}
	// dual-wield
	sd->status.weapon = 0;
	switch (sd->weapontype1){
	case W_DAGGER:
		switch (sd->weapontype2) {
		case W_DAGGER:  sd->status.weapon = W_DOUBLE_DD; break;
		case W_1HSWORD: sd->status.weapon = W_DOUBLE_DS; break;
		case W_1HAXE:   sd->status.weapon = W_DOUBLE_DA; break;
		}
		break;
	case W_1HSWORD:
		switch (sd->weapontype2) {
		case W_DAGGER:  sd->status.weapon = W_DOUBLE_DS; break;
		case W_1HSWORD: sd->status.weapon = W_DOUBLE_SS; break;
		case W_1HAXE:   sd->status.weapon = W_DOUBLE_SA; break;
		}
		break;
	case W_1HAXE:
		switch (sd->weapontype2) {
		case W_DAGGER:  sd->status.weapon = W_DOUBLE_DA; break;
		case W_1HSWORD: sd->status.weapon = W_DOUBLE_SA; break;
		case W_1HAXE:   sd->status.weapon = W_DOUBLE_AA; break;
		}
	}
	// unknown, default to right hand type
	if (!sd->status.weapon)
		sd->status.weapon = sd->weapontype1;
}

/**
* Set equip index
* @param sd : Player
*/
void pc_setequipindex(struct map_session_data *sd)
{
	uint16 i;

	nullpo_retv(sd);

	for (i = 0; i < EQI_MAX; i++)
		sd->equip_index[i] = -1;

	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].nameid <= 0)
			continue;
		if (sd->status.inventory[i].equip) {
			uint8 j;
			for (j = 0; j < EQI_MAX; j++)
				if (sd->status.inventory[i].equip & equip_pos[j])
					sd->equip_index[j] = i;

			if (sd->status.inventory[i].equip & EQP_HAND_R) {
				if (sd->inventory_data[i])
					sd->weapontype1 = sd->inventory_data[i]->look;
				else
					sd->weapontype1 = 0;
			}

			if( sd->status.inventory[i].equip & EQP_HAND_L ) {
				if( sd->inventory_data[i] && sd->inventory_data[i]->type == IT_WEAPON )
					sd->weapontype2 = sd->inventory_data[i]->look;
				else
					sd->weapontype2 = 0;
			}
		}
	}
	pc_calcweapontype(sd);
}

//static int pc_isAllowedCardOn(struct map_session_data *sd,int s,int eqindex,int flag)
//{
//	int i;
//	struct item *item = &sd->status.inventory[eqindex];
//	struct item_data *data;
//
//	//Crafted/made/hatched items.
//	if (itemdb_isspecial(item->card[0]))
//		return 1;
//
//	/* scan for enchant armor gems */
//	if( item->card[MAX_SLOTS - 1] && s < MAX_SLOTS - 1 )
//		s = MAX_SLOTS - 1;
//
//	ARR_FIND( 0, s, i, item->card[i] && (data = itemdb_exists(item->card[i])) != NULL && data->flag.no_equip&flag );
//	return( i < s ) ? 0 : 1;
//}


/**
 * Check if an item is equiped by player
 * (Check if the itemid is equiped then search if that match the index in inventory (should be))
 * @param sd : player session
 * @param nameid : itemid
 * @return 1:yes, 0:no
 */
bool pc_isequipped(struct map_session_data *sd, unsigned short nameid)
{
	uint8 i;

	for( i = 0; i < EQI_MAX; i++ )
	{
		short index = sd->equip_index[i], j;
		if( index < 0 )
			continue;
		if( pc_is_same_equip_index((enum equip_index)i, sd->equip_index, index) )
			continue;
		if( !sd->inventory_data[index] ) 
			continue;
		if( sd->inventory_data[index]->nameid == nameid )
			return true;
		for( j = 0; j < sd->inventory_data[index]->slot; j++ ){
			if( sd->status.inventory[index].card[j] == nameid )
				return true;
		}
	}

	return false;
}

/** Check adopt rule
* @param p1_sd Player 1
* @param p2_sd Player 2
* @param b_sd Player that will be adopted
* @return True - if can be adopted, False otherwise
*/
bool pc_can_Adopt(struct map_session_data *p1_sd, struct map_session_data *p2_sd, struct map_session_data *b_sd )
{
	if( !p1_sd || !p2_sd || !b_sd )
		return false;

	if( b_sd->status.father || b_sd->status.mother || b_sd->adopt_invite )
		return false; // already adopted baby / in adopt request

	if( !p1_sd->status.partner_id || !p1_sd->status.party_id || p1_sd->status.party_id != b_sd->status.party_id )
		return false; // You need to be married and in party with baby to adopt

	if( p1_sd->status.partner_id != p2_sd->status.char_id || p2_sd->status.partner_id != p1_sd->status.char_id )
		return false; // Not married, wrong married

	if( p2_sd->status.party_id != p1_sd->status.party_id )
		return false; // Both parents need to be in the same party

	// Parents need to have their ring equipped
	if( !pc_isequipped(p1_sd, WEDDING_RING_M) && !pc_isequipped(p1_sd, WEDDING_RING_F) )
		return false;

	if( !pc_isequipped(p2_sd, WEDDING_RING_M) && !pc_isequipped(p2_sd, WEDDING_RING_F) )
		return false;

	// Already adopted a baby
	if( p1_sd->status.child || p2_sd->status.child ) {
		clif_Adopt_reply(p1_sd, 0);
		return false;
	}

	// Parents need at least lvl 70 to adopt
	if( p1_sd->status.base_level < 70 || p2_sd->status.base_level < 70 ) {
		clif_Adopt_reply(p1_sd, 1);
		return false;
	}

	if( b_sd->status.partner_id ) {
		clif_Adopt_reply(p1_sd, 2);
		return false;
	}

	if( !( ( b_sd->status.class_ >= JOB_NOVICE && b_sd->status.class_ <= JOB_THIEF ) || b_sd->status.class_ == JOB_SUPER_NOVICE || b_sd->status.class_ == JOB_SUPER_NOVICE_E ) )
		return false;

	return true;
}

/*==========================================
 * Adoption Process
 *------------------------------------------*/
bool pc_adoption(struct map_session_data *p1_sd, struct map_session_data *p2_sd, struct map_session_data *b_sd)
{
	int job, joblevel;
	unsigned int jobexp;

	if( !pc_can_Adopt(p1_sd, p2_sd, b_sd) )
		return false;

	// Preserve current job levels and progress
	joblevel = b_sd->status.job_level;
	jobexp = b_sd->status.job_exp;

	job = pc_mapid2jobid(b_sd->class_|JOBL_BABY, b_sd->status.sex);
	if( job != -1 && pc_jobchange(b_sd, job, 0) )
	{ // Success, proceed to configure parents and baby skills
		p1_sd->status.child = b_sd->status.char_id;
		p2_sd->status.child = b_sd->status.char_id;
		b_sd->status.father = p1_sd->status.char_id;
		b_sd->status.mother = p2_sd->status.char_id;

		// Restore progress
		b_sd->status.job_level = joblevel;
		clif_updatestatus(b_sd, SP_JOBLEVEL);
		b_sd->status.job_exp = jobexp;
		clif_updatestatus(b_sd, SP_JOBEXP);

		// Baby Skills
		pc_skill(b_sd, WE_BABY, 1, ADDSKILL_PERMANENT);
		pc_skill(b_sd, WE_CALLPARENT, 1, ADDSKILL_PERMANENT);

		// Parents Skills
		pc_skill(p1_sd, WE_CALLBABY, 1, ADDSKILL_PERMANENT);
		pc_skill(p2_sd, WE_CALLBABY, 1, ADDSKILL_PERMANENT);

		return true;
	}

	return false; // Job Change Fail
}
 
/*==========================================
 * Check if player can use/equip selected item. Used by pc_isUseitem and pc_isequip
   Returns:
		false : Cannot use/equip
		true  : Can use/equip
 * Credits:
		[Inkfish] for first idea
		[Haru] for third-classes extension
		[Cydh] finishing :D
 *------------------------------------------*/
static bool pc_isItemClass (struct map_session_data *sd, struct item_data* item) {
	while (1) {
		if (item->class_upper&ITEMJ_NORMAL && !(sd->class_&(JOBL_UPPER|JOBL_THIRD|JOBL_BABY)))	//normal classes (no upper, no baby, no third)
			break;
#ifndef RENEWAL
		//allow third classes to use trans. class items
		if (item->class_upper&ITEMJ_UPPER && sd->class_&(JOBL_UPPER|JOBL_THIRD))	//trans. classes
			break;
		//third-baby classes can use same item too
		if (item->class_upper&ITEMJ_BABY && sd->class_&JOBL_BABY)	//baby classes
			break;
		//don't need to decide specific rules for third-classes?
		//items for third classes can be used for all third classes
		if (item->class_upper&(ITEMJ_THIRD|ITEMJ_THIRD_TRANS|ITEMJ_THIRD_BABY) && sd->class_&JOBL_THIRD)
			break;
#else
		//trans. classes (exl. third-trans.)
		if (item->class_upper&ITEMJ_UPPER && sd->class_&JOBL_UPPER && !(sd->class_&JOBL_THIRD))
			break;
		//baby classes (exl. third-baby)
		if (item->class_upper&ITEMJ_BABY && sd->class_&JOBL_BABY && !(sd->class_&JOBL_THIRD))
			break;
		//third classes (exl. third-trans. and baby-third)
		if (item->class_upper&ITEMJ_THIRD && sd->class_&JOBL_THIRD && !(sd->class_&(JOBL_UPPER|JOBL_BABY)))
			break;
		//trans-third classes
		if (item->class_upper&ITEMJ_THIRD_TRANS && sd->class_&JOBL_THIRD && sd->class_&JOBL_UPPER)
			break;
		//third-baby classes
		if (item->class_upper&ITEMJ_THIRD_BABY && sd->class_&JOBL_THIRD && sd->class_&JOBL_BABY)
			break;
#endif
		return false;
	}
	return true;
}

/*=================================================
 * Checks if the player can equip the item at index n in inventory.
 * @param sd
 * @param n Item index in inventory
 * @return ITEM_EQUIP_ACK_OK(0) if can be equipped, or ITEM_EQUIP_ACK_FAIL(1)/ITEM_EQUIP_ACK_FAILLEVEL(2) if can't
 *------------------------------------------------*/
uint8 pc_isequip(struct map_session_data *sd,int n)
{
	struct item_data *item;

	nullpo_retr(ITEM_EQUIP_ACK_FAIL, sd);

	item = sd->inventory_data[n];

	if(pc_has_permission(sd, PC_PERM_USE_ALL_EQUIPMENT))
		return ITEM_EQUIP_ACK_OK;

	if(item == NULL)
		return ITEM_EQUIP_ACK_FAIL;
	if(item->elv && sd->status.base_level < (unsigned int)item->elv)
		return ITEM_EQUIP_ACK_FAILLEVEL;
	if(item->elvmax && sd->status.base_level > (unsigned int)item->elvmax)
		return ITEM_EQUIP_ACK_FAILLEVEL;
	if(item->sex != 2 && sd->status.sex != item->sex)
		return ITEM_EQUIP_ACK_FAIL;

	if (sd->sc.count) {
		if(item->equip & EQP_ARMS && item->type == IT_WEAPON && sd->sc.data[SC_STRIPWEAPON]) // Also works with left-hand weapons [DracoRPG]
			return ITEM_EQUIP_ACK_FAIL;
		if(item->equip & EQP_SHIELD && item->type == IT_ARMOR && sd->sc.data[SC_STRIPSHIELD])
			return ITEM_EQUIP_ACK_FAIL;
		if(item->equip & EQP_ARMOR && sd->sc.data[SC_STRIPARMOR])
			return ITEM_EQUIP_ACK_FAIL;
		if(item->equip & EQP_HEAD_TOP && sd->sc.data[SC_STRIPHELM])
			return ITEM_EQUIP_ACK_FAIL;
		if(item->equip & EQP_ACC && sd->sc.data[SC__STRIPACCESSORY])
			return ITEM_EQUIP_ACK_FAIL;
		if(item->equip && sd->sc.data[SC_KYOUGAKU])
			return ITEM_EQUIP_ACK_FAIL;

		if (sd->sc.data[SC_SPIRIT] && sd->sc.data[SC_SPIRIT]->val2 == SL_SUPERNOVICE) {
			//Spirit of Super Novice equip bonuses. [Skotlex]
			if (sd->status.base_level > 90 && item->equip & EQP_HELM)
				return ITEM_EQUIP_ACK_OK; //Can equip all helms

			if (sd->status.base_level > 96 && item->equip & EQP_ARMS && item->type == IT_WEAPON && item->wlv == 4)
				switch(item->look) { //In weapons, the look determines type of weapon.
					case W_DAGGER: //All level 4 - Daggers
					case W_1HSWORD: //All level 4 - 1H Swords
					case W_1HAXE: //All level 4 - 1H Axes
					case W_MACE: //All level 4 - 1H Maces
					case W_STAFF: //All level 4 - 1H Staves
					case W_2HSTAFF: //All level 4 - 2H Staves
						return ITEM_EQUIP_ACK_OK;
				}
		}
	}

	//fail to equip if item is restricted
	if (!battle_config.allow_equip_restricted_item && itemdb_isNoEquip(item, sd->bl.m))
		return ITEM_EQUIP_ACK_FAIL;

	//Not equipable by class. [Skotlex]
	if (!(1<<(sd->class_&MAPID_BASEMASK)&item->class_base[(sd->class_&JOBL_2_1)?1:((sd->class_&JOBL_2_2)?2:0)]))
		return ITEM_EQUIP_ACK_FAIL;
	
	if (!pc_isItemClass(sd,item))
		return ITEM_EQUIP_ACK_FAIL;

	return ITEM_EQUIP_ACK_OK;
}

/*==========================================
 * No problem with the session id
 * set the status that has been sent from char server
 *------------------------------------------*/
bool pc_authok(struct map_session_data *sd, uint32 login_id2, time_t expiration_time, int group_id, struct mmo_charstatus *st, bool changing_mapservers)
{
	int i;
#ifdef BOUND_ITEMS
	int j;
	int idxlist[MAX_INVENTORY];
#endif
	unsigned long tick = gettick();
	uint32 ip = session[sd->fd]->client_addr;

	sd->login_id2 = login_id2;
	sd->group_id = group_id;

	/* load user permissions */
	pc_group_pc_load(sd);

	memcpy(&sd->status, st, sizeof(*st));

	if (st->sex != sd->status.sex) {
		clif_authfail_fd(sd->fd, 0);
		return false;
	}

	//Set the map-server used job id. [Skotlex]
	i = pc_jobid2mapid(sd->status.class_);
	if (i == -1) { //Invalid class?
		ShowError("pc_authok: Invalid class %d for player %s (%d:%d). Class was changed to novice.\n", sd->status.class_, sd->status.name, sd->status.account_id, sd->status.char_id);
		sd->status.class_ = JOB_NOVICE;
		sd->class_ = MAPID_NOVICE;
	} else
		sd->class_ = i;

	// Checks and fixes to character status data, that are required
	// in case of configuration change or stuff, which cannot be
	// checked on char-server.
	sd->status.hair = cap_value(sd->status.hair,MIN_HAIR_STYLE,MAX_HAIR_STYLE);
	sd->status.hair_color = cap_value(sd->status.hair_color,MIN_HAIR_COLOR,MAX_HAIR_COLOR);
	sd->status.clothes_color = cap_value(sd->status.clothes_color,MIN_CLOTH_COLOR,MAX_CLOTH_COLOR);

	//Initializations to null/0 unneeded since map_session_data was filled with 0 upon allocation.
	if(!sd->status.hp) pc_setdead(sd);
	sd->state.connect_new = 1;

	sd->followtimer = INVALID_TIMER; // [MouseJstr]
	sd->invincible_timer = INVALID_TIMER;
	sd->npc_timer_id = INVALID_TIMER;
	sd->pvp_timer = INVALID_TIMER;
	sd->expiration_tid = INVALID_TIMER;
	sd->autotrade_tid = INVALID_TIMER;

#ifdef SECURE_NPCTIMEOUT
	// Initialize to defaults/expected
	sd->npc_idle_timer = INVALID_TIMER;
	sd->npc_idle_tick = tick;
	sd->npc_idle_type = NPCT_INPUT;
	sd->state.ignoretimeout = false;
#endif

	sd->canuseitem_tick = tick;
	sd->canusecashfood_tick = tick;
	sd->canequip_tick = tick;
	sd->cantalk_tick = tick;
	sd->canskill_tick = tick;
	sd->cansendmail_tick = tick;
	sd->idletime = last_tick;

	for(i = 0; i < MAX_SPIRITBALL; i++)
		sd->spirit_timer[i] = INVALID_TIMER;
	for(i = 0; i < ARRAYLENGTH(sd->autobonus); i++)
		sd->autobonus[i].active = INVALID_TIMER;
	for(i = 0; i < ARRAYLENGTH(sd->autobonus2); i++)
		sd->autobonus2[i].active = INVALID_TIMER;
	for(i = 0; i < ARRAYLENGTH(sd->autobonus3); i++)
		sd->autobonus3[i].active = INVALID_TIMER;

	if (battle_config.item_auto_get)
		sd->state.autoloot = 10000;

	if (battle_config.disp_experience)
		sd->state.showexp = 1;
	if (battle_config.disp_zeny)
		sd->state.showzeny = 1;
#ifdef VIP_ENABLE
	if (!battle_config.vip_disp_rate)
		sd->disableshowrate = 1;
#endif

	if (!(battle_config.display_skill_fail&2))
		sd->state.showdelay = 1;

	pc_setinventorydata(sd);
	pc_setequipindex(sd);

	if( sd->status.option&OPTION_INVISIBLE && !pc_can_use_command( sd, "hide", COMMAND_ATCOMMAND ) ){
		sd->status.option &= ~OPTION_INVISIBLE;
	}

	status_change_init(&sd->bl);

	sd->sc.option = sd->status.option; //This is the actual option used in battle.
	//Set here because we need the inventory data for weapon sprite parsing.
	status_set_viewdata(&sd->bl, sd->status.class_);
	unit_dataset(&sd->bl);

	sd->guild_x = -1;
	sd->guild_y = -1;

	sd->delayed_damage = 0;

	// Event Timers
	for( i = 0; i < MAX_EVENTTIMER; i++ )
		sd->eventtimer[i] = INVALID_TIMER;
	// Rental Timer
	sd->rental_timer = INVALID_TIMER;

	for( i = 0; i < 3; i++ )
		sd->hate_mob[i] = -1;

	sd->quest_log = NULL;
	sd->num_quests = 0;
	sd->avail_quests = 0;
	sd->save_quest = false;
	sd->count_rewarp = 0;

	sd->regs.vars = i64db_alloc(DB_OPT_BASE);
	sd->regs.arrays = NULL;
	sd->vars_dirty = false;
	sd->vars_ok = false;
	sd->vars_received = 0x0;

	//warp player
	if ((i=pc_setpos(sd,sd->status.last_point.map, sd->status.last_point.x, sd->status.last_point.y, CLR_OUTSIGHT)) != 0) {
		ShowError ("Last_point_map %s - id %d not found (error code %d)\n", mapindex_id2name(sd->status.last_point.map), sd->status.last_point.map, i);

		// try warping to a default map instead (church graveyard)
		if (pc_setpos(sd, mapindex_name2id(MAP_PRONTERA), 273, 354, CLR_OUTSIGHT) != 0) {
			// if we fail again
			clif_authfail_fd(sd->fd, 0);
			return false;
		}
	}

	clif_authok(sd);

	//Prevent S. Novices from getting the no-death bonus just yet. [Skotlex]
	sd->die_counter=-1;

	//display login notice
	ShowInfo("'"CL_WHITE"%s"CL_RESET"' logged in."
	         " (AID/CID: '"CL_WHITE"%d/%d"CL_RESET"',"
	         " Packet Ver: '"CL_WHITE"%d"CL_RESET"', IP: '"CL_WHITE"%d.%d.%d.%d"CL_RESET"',"
	         " Group '"CL_WHITE"%d"CL_RESET"').\n",
	         sd->status.name, sd->status.account_id, sd->status.char_id,
	         sd->packet_ver, CONVIP(ip), sd->group_id);
	// Send friends list
	clif_friendslist_send(sd);

	if( !changing_mapservers ) {

		if (battle_config.display_version == 1)
			pc_show_version(sd);

		// Message of the Day [Valaris]
		for(i=0; i < MOTD_LINE_SIZE && motd_text[i][0]; i++) {
			if (battle_config.motd_type)
				clif_disp_onlyself(sd,motd_text[i],strlen(motd_text[i]));
			else
				clif_displaymessage(sd->fd, motd_text[i]);
		}

		if (expiration_time != 0)
			sd->expiration_time = expiration_time;

		/**
		 * Fixes login-without-aura glitch (the screen won't blink at this point, don't worry :P)
		 **/
		clif_changemap(sd,sd->bl.m,sd->bl.x,sd->bl.y);
	}

	/**
	 * Check if player have any item cooldowns on
	 **/
	pc_itemcd_do(sd,true);
	pc_validate_skill(sd);

#ifdef BOUND_ITEMS
	// Party bound item check
	if(sd->status.party_id == 0 && (j = pc_bound_chk(sd,BOUND_PARTY,idxlist))) { // Party was deleted while character offline
		for(i=0;i<j;i++)
			pc_delitem(sd,idxlist[i],sd->status.inventory[idxlist[i]].amount,0,1,LOG_TYPE_OTHER);
	}
#endif

	/* [Ind] */
	sd->sc_display = NULL;
	sd->sc_display_count = 0;

	// Player has not yet received the CashShop list
	sd->status.cashshop_sent = false;

	sd->last_addeditem_index = -1;
	
	sd->bonus_script.head = NULL;
	sd->bonus_script.count = 0;

	// Request all registries (auth is considered completed whence they arrive)
	intif_request_registry(sd,7);
	return true;
}

/*==========================================
 * Closes a connection because it failed to be authenticated from the char server.
 *------------------------------------------*/
void pc_authfail(struct map_session_data *sd)
{
	clif_authfail_fd(sd->fd, 0);
	return;
}

/**
 * Player register a bl as hatred
 * @param sd : player session
 * @param pos : hate position [0;2]
 * @param bl : target bl
 * @return false:failed, true:success
 */
bool pc_set_hate_mob(struct map_session_data *sd, int pos, struct block_list *bl)
{
	int class_;
	if (!sd || !bl || pos < 0 || pos > 2)
		return false;
	if (sd->hate_mob[pos] != -1)
	{	//Can't change hate targets.
		clif_hate_info(sd, pos, sd->hate_mob[pos], 0); //Display current
		return false;
	}

	class_ = status_get_class(bl);
	if (!pcdb_checkid(class_)) {
		unsigned int max_hp = status_get_max_hp(bl);
		if ((pos == 1 && max_hp < 6000) || (pos == 2 && max_hp < 20000))
			return false;
		if (pos != status_get_size(bl))
			return false; //Wrong size
	}
	sd->hate_mob[pos] = class_;
	pc_setglobalreg(sd, add_str(sg_info[pos].hate_var), class_+1);
	clif_hate_info(sd, pos, class_, 1);
	return true;
}

/*==========================================
 * Invoked once after the char/account/account2 registry variables are received. [Skotlex]
 *------------------------------------------*/
void pc_reg_received(struct map_session_data *sd)
{
	uint8 i;

	sd->vars_ok = true;

	sd->change_level_2nd = pc_readglobalreg(sd, add_str("jobchange_level"));
	sd->change_level_3rd = pc_readglobalreg(sd, add_str("jobchange_level_3rd"));
	sd->die_counter = pc_readglobalreg(sd, add_str("PC_DIE_COUNTER"));

	sd->langtype = pc_readaccountreg(sd, add_str("#langtype"));
	if (msg_checklangtype(sd->langtype,true) < 0)
		sd->langtype = 0; //invalid langtype reset to default

	// Cash shop
	sd->cashPoints = pc_readaccountreg(sd, add_str("#CASHPOINTS"));
	sd->kafraPoints = pc_readaccountreg(sd, add_str("#KAFRAPOINTS"));

	// Cooking Exp
	sd->cook_mastery = pc_readglobalreg(sd, add_str("COOK_MASTERY"));

	if( (sd->class_&MAPID_BASEMASK) == MAPID_TAEKWON )
	{ // Better check for class rather than skill to prevent "skill resets" from unsetting this
		sd->mission_mobid = pc_readglobalreg(sd, add_str("TK_MISSION_ID"));
		sd->mission_count = pc_readglobalreg(sd, add_str("TK_MISSION_COUNT"));
	}

	if (battle_config.feature_banking)
		sd->bank_vault = pc_readreg2(sd, BANK_VAULT_VAR);

	if (battle_config.feature_roulette) {
		sd->roulette_point.bronze = pc_readreg2(sd, ROULETTE_BRONZE_VAR);
		sd->roulette_point.silver = pc_readreg2(sd, ROULETTE_SILVER_VAR);
		sd->roulette_point.gold = pc_readreg2(sd, ROULETTE_GOLD_VAR);
	}
	sd->roulette.prizeIdx = -1;

	//SG map and mob read [Komurka]
	for(i=0;i<MAX_PC_FEELHATE;i++) { //for now - someone need to make reading from txt/sql
		uint16 j;

		if ((j = pc_readglobalreg(sd, add_str(sg_info[i].feel_var))) != 0) {
			sd->feel_map[i].index = j;
			sd->feel_map[i].m = map_mapindex2mapid(j);
		} else {
			sd->feel_map[i].index = 0;
			sd->feel_map[i].m = -1;
		}
		sd->hate_mob[i] = pc_readglobalreg(sd, add_str(sg_info[i].hate_var))-1;
	}

	if ((i = pc_checkskill(sd,RG_PLAGIARISM)) > 0) {
		unsigned short skid = pc_readglobalreg(sd, add_str(SKILL_VAR_PLAGIARISM));
		sd->cloneskill_idx = skill_get_index(skid);
		if (sd->cloneskill_idx > 0) {
			sd->status.skill[sd->cloneskill_idx].id = skid;
			sd->status.skill[sd->cloneskill_idx].lv = pc_readglobalreg(sd, add_str(SKILL_VAR_PLAGIARISM_LV));
			if (sd->status.skill[sd->cloneskill_idx].lv > i)
				sd->status.skill[sd->cloneskill_idx].lv = i;
			sd->status.skill[sd->cloneskill_idx].flag = SKILL_FLAG_PLAGIARIZED;
		}
	}
	if ((i = pc_checkskill(sd,SC_REPRODUCE)) > 0) {
		unsigned short skid = pc_readglobalreg(sd, add_str(SKILL_VAR_REPRODUCE));
		sd->reproduceskill_idx = skill_get_index(skid);
		if (sd->reproduceskill_idx > 0) {
			sd->status.skill[sd->reproduceskill_idx].id = skid;
			sd->status.skill[sd->reproduceskill_idx].lv = pc_readglobalreg(sd, add_str(SKILL_VAR_REPRODUCE_LV));
			if (i < sd->status.skill[sd->reproduceskill_idx].lv)
				sd->status.skill[sd->reproduceskill_idx].lv = i;
			sd->status.skill[sd->reproduceskill_idx].flag = SKILL_FLAG_PLAGIARIZED;
		}
	}
	//Weird... maybe registries were reloaded?
	if (sd->state.active)
		return;
	sd->state.active = 1;

	if (sd->status.party_id)
		party_member_joined(sd);
	if (sd->status.guild_id)
		guild_member_joined(sd);

	// pet
	if (sd->status.pet_id > 0)
		intif_request_petdata(sd->status.account_id, sd->status.char_id, sd->status.pet_id);

	// Homunculus [albator]
	if( sd->status.hom_id > 0 )
		intif_homunculus_requestload(sd->status.account_id, sd->status.hom_id);
	if( sd->status.mer_id > 0 )
		intif_mercenary_request(sd->status.mer_id, sd->status.char_id);
	if( sd->status.ele_id > 0 )
		intif_elemental_request(sd->status.ele_id, sd->status.char_id);

	map_addiddb(&sd->bl);
	map_delnickdb(sd->status.char_id, sd->status.name);
	if (!chrif_auth_finished(sd))
		ShowError("pc_reg_received: Failed to properly remove player %d:%d from logging db!\n", sd->status.account_id, sd->status.char_id);

	pc_load_combo(sd);

	status_calc_pc(sd, (enum e_status_calc_opt)(SCO_FIRST|SCO_FORCE));
	chrif_scdata_request(sd->status.account_id, sd->status.char_id);
	chrif_skillcooldown_request(sd->status.account_id, sd->status.char_id);
	chrif_bsdata_request(sd->status.char_id);
	sd->storage_size = MIN_STORAGE; //default to min
#ifdef VIP_ENABLE
	sd->vip.time = 0;
	sd->vip.enabled = 0;
	chrif_req_login_operation(sd->status.account_id, sd->status.name, CHRIF_OP_LOGIN_VIP, 0, 1, 0);  // request VIP informations
#endif
	intif_Mail_requestinbox(sd->status.char_id, 0); // MAIL SYSTEM - Request Mail Inbox
	intif_request_questlog(sd);

	if (sd->state.connect_new == 0 && sd->fd) { //Character already loaded map! Gotta trigger LoadEndAck manually.
		sd->state.connect_new = 1;
		clif_parse_LoadEndAck(sd->fd, sd);
	}

	if( sd->sc.option&OPTION_INVISIBLE ) {
		sd->vd.class_ = INVISIBLE_CLASS;
		clif_displaymessage( sd->fd, msg_txt( sd, 11 ) ); // Invisible: On
		// decrement the number of pvp players on the map
		map[sd->bl.m].users_pvp--;

		if( map[sd->bl.m].flag.pvp && !map[sd->bl.m].flag.pvp_nocalcrank && sd->pvp_timer != INVALID_TIMER ){
			// unregister the player for ranking
			delete_timer( sd->pvp_timer, pc_calc_pvprank_timer );
			sd->pvp_timer = INVALID_TIMER;
		}

		clif_changeoption( &sd->bl );
	}

	pc_check_expiration(sd);

	if( sd->state.autotrade ) {
		clif_parse_LoadEndAck(sd->fd, sd);
		sd->autotrade_tid = add_timer(gettick() + battle_config.feature_autotrade_open_delay, pc_autotrade_timer, sd->bl.id, 0);
	}
}

static int pc_calc_skillpoint(struct map_session_data* sd)
{
	uint16 i, skill_point = 0;

	nullpo_ret(sd);

	for(i = 1; i < MAX_SKILL; i++) {
		if( sd->status.skill[i].id && sd->status.skill[i].lv > 0) {
			uint16 inf2 = skill_get_inf2(sd->status.skill[i].id);
			if ((!(inf2&INF2_QUEST_SKILL) || battle_config.quest_skill_learn) &&
				!(inf2&(INF2_WEDDING_SKILL|INF2_SPIRIT_SKILL)) //Do not count wedding/link skills. [Skotlex]
				)
			{
				if(sd->status.skill[i].flag == SKILL_FLAG_PERMANENT)
					skill_point += sd->status.skill[i].lv;
				else if(sd->status.skill[i].flag >= SKILL_FLAG_REPLACED_LV_0)
					skill_point += (sd->status.skill[i].flag - SKILL_FLAG_REPLACED_LV_0);
			}
		}
	}

	return skill_point;
}

static bool pc_grant_allskills(struct map_session_data *sd, bool addlv) {
	uint16 i = 0;

	if (!sd || !pc_has_permission(sd, PC_PERM_ALL_SKILL) || !SKILL_MAX_DB())
		return false;

	/**
	* Dummy skills must NOT be added here otherwise they'll be displayed in the,
	* skill tree and since they have no icons they'll give resource errors
	* Get ALL skills except npc/guild ones. [Skotlex]
	* Don't add SG_DEVIL [Komurka] and MO_TRIPLEATTACK and RG_SNATCHER [ultramage]
	**/
	for( i = 0; i < MAX_SKILL; i++ ) {
		uint16 skill_id = skill_idx2id(i);
		if (!skill_id || (skill_get_inf2(skill_id)&(INF2_NPC_SKILL|INF2_GUILD_SKILL)))
			continue;
		switch (skill_id) {
			case SM_SELFPROVOKE:
			case AB_DUPLELIGHT_MELEE:
			case AB_DUPLELIGHT_MAGIC:
			case WL_CHAINLIGHTNING_ATK:
			case WL_TETRAVORTEX_FIRE:
			case WL_TETRAVORTEX_WATER:
			case WL_TETRAVORTEX_WIND:
			case WL_TETRAVORTEX_GROUND:
			case WL_SUMMON_ATK_FIRE:
			case WL_SUMMON_ATK_WIND:
			case WL_SUMMON_ATK_WATER:
			case WL_SUMMON_ATK_GROUND:
			case LG_OVERBRAND_BRANDISH:
			case LG_OVERBRAND_PLUSATK:
			case WM_SEVERE_RAINSTORM_MELEE:
			case RL_R_TRIP_PLUSATK:
			case SG_DEVIL:
			case MO_TRIPLEATTACK:
			case RG_SNATCHER:
				continue;
			default:
				{
					uint8 lv = (uint8)skill_get_max(skill_id);
					if (lv > 0) {
						sd->status.skill[i].id = skill_id;
						if (addlv)
							sd->status.skill[i].lv = lv;
					}
				}
				break;
		}
	}
	return true;
}

/*==========================================
 * Calculation of skill level.
 * @param sd
 *------------------------------------------*/
void pc_calc_skilltree(struct map_session_data *sd)
{
	int i, flag;
	int c = 0;

	nullpo_retv(sd);
	i = pc_calc_skilltree_normalize_job(sd);
	c = pc_mapid2jobid(i, sd->status.sex);
	if( c == -1 )
	{ //Unable to normalize job??
		ShowError("pc_calc_skilltree: Unable to normalize job %d for character %s (%d:%d)\n", i, sd->status.name, sd->status.account_id, sd->status.char_id);
		return;
	}
	c = pc_class2idx(c);

	for( i = 0; i < MAX_SKILL; i++ ) {
		if( sd->status.skill[i].flag != SKILL_FLAG_PLAGIARIZED && sd->status.skill[i].flag != SKILL_FLAG_PERM_GRANTED ) //Don't touch these
			sd->status.skill[i].id = 0; //First clear skills.
		/* permanent skills that must be re-checked */
		if( sd->status.skill[i].flag == SKILL_FLAG_PERM_GRANTED ) {
			uint16 sk_id = skill_idx2id(i);
			if (!sk_id) {
				sd->status.skill[i].id = 0;
				sd->status.skill[i].lv = 0;
				sd->status.skill[i].flag = SKILL_FLAG_PERMANENT;
				continue;
			}
			switch (sk_id) {
				case NV_TRICKDEAD:
					if( (sd->class_&MAPID_UPPERMASK) != MAPID_NOVICE ) {
						sd->status.skill[i].id = 0;
						sd->status.skill[i].lv = 0;
						sd->status.skill[i].flag = SKILL_FLAG_PERMANENT;
					}
					break;
			}
		}
	}

	for( i = 0; i < MAX_SKILL; i++ ) {
		uint16 skill_id = 0;

		// Restore original level of skills after deleting earned skills.
		if( sd->status.skill[i].flag != SKILL_FLAG_PERMANENT && sd->status.skill[i].flag != SKILL_FLAG_PERM_GRANTED && sd->status.skill[i].flag != SKILL_FLAG_PLAGIARIZED ) {
			sd->status.skill[i].lv = (sd->status.skill[i].flag == SKILL_FLAG_TEMPORARY) ? 0 : sd->status.skill[i].flag - SKILL_FLAG_REPLACED_LV_0;
			sd->status.skill[i].flag = SKILL_FLAG_PERMANENT;
		}

		//Enable Bard/Dancer spirit linked skills.
		if (!(skill_id = skill_idx2id(i)) || skill_id < DC_HUMMING || skill_id > DC_SERVICEFORYOU)
			continue;

		if( &sd->sc && sd->sc.count && sd->sc.data[SC_SPIRIT] && sd->sc.data[SC_SPIRIT]->val2 == SL_BARDDANCER ) {
			//Link Dancer skills to bard.
			if( sd->status.sex ) {
				if( sd->status.skill[i-8].lv < 10 )
					continue;
				sd->status.skill[i].id = skill_id;
				sd->status.skill[i].lv = sd->status.skill[i-8].lv; // Set the level to the same as the linking skill
				sd->status.skill[i].flag = SKILL_FLAG_TEMPORARY; // Tag it as a non-savable, non-uppable, bonus skill
			}
			//Link Bard skills to dancer.
			else {
				if( sd->status.skill[i].lv < 10 )
					continue;
				sd->status.skill[i-8].id = skill_id - 8;
				sd->status.skill[i-8].lv = sd->status.skill[i].lv; // Set the level to the same as the linking skill
				sd->status.skill[i-8].flag = SKILL_FLAG_TEMPORARY; // Tag it as a non-savable, non-uppable, bonus skill
			}
		}
	}

	// Removes Taekwon Ranker skill bonus
	if ((sd->class_&MAPID_UPPERMASK) != MAPID_TAEKWON) {
		uint16 c_ = pc_class2idx(JOB_TAEKWON);

		for (i = 0; i < MAX_SKILL_TREE; i++) {
			uint16 sk_id = skill_tree[c_][i].id;
			uint16 sk_idx = 0;

			if (!sk_id || !(sk_idx = skill_get_index(skill_tree[c_][i].id)))
				continue;

			if (sd->status.skill[sk_idx].flag != SKILL_FLAG_PLAGIARIZED && sd->status.skill[sk_idx].flag != SKILL_FLAG_PERM_GRANTED) {
				if (sk_id == NV_BASIC || sk_id == NV_FIRSTAID || sk_id == WE_CALLBABY)
					continue;
				sd->status.skill[sk_idx].id = 0;
			}
		}
	}

	// Grant all skills
	pc_grant_allskills(sd, false);

	do {
		uint16 skid = 0;

		flag = 0;
		for (i = 0; i < MAX_SKILL_TREE && (skid = skill_tree[c][i].id) > 0; i++) {
			bool fail = false;
			uint16 sk_idx = skill_get_index(skid);

			if (sd->status.skill[sk_idx].id)
				continue; //Skill already known.

			if (!battle_config.skillfree) {
				uint8 j;

				// Checking required skills
				for(j = 0; j < MAX_PC_SKILL_REQUIRE; j++) {
					uint16 sk_need_id = skill_tree[c][i].need[j].id;
					uint16 sk_need_idx = 0;

					if (sk_need_id && (sk_need_idx = skill_get_index(sk_need_id))) {
						short sk_need = sk_need_id;

						if (sd->status.skill[sk_need_idx].id == 0 || sd->status.skill[sk_need_idx].flag == SKILL_FLAG_TEMPORARY || sd->status.skill[sk_need_idx].flag == SKILL_FLAG_PLAGIARIZED)
							sk_need = 0; //Not learned.
						else if (sd->status.skill[sk_need_idx].flag >= SKILL_FLAG_REPLACED_LV_0) //Real learned level
							sk_need = sd->status.skill[sk_need_idx].flag - SKILL_FLAG_REPLACED_LV_0;
						else
							sk_need = pc_checkskill(sd,sk_need_id);

						if (sk_need < skill_tree[c][i].need[j].lv) {
							fail = true;
							break;
						}
					}
				}

				if (sd->status.job_level < skill_tree[c][i].joblv) { //We need to get the actual class in this case
					int class_ = pc_mapid2jobid(sd->class_, sd->status.sex);
					class_ = pc_class2idx(class_);
					if (class_ == c || (class_ != c && sd->status.job_level < skill_tree[class_][i].joblv))
						fail = true; // job level requirement wasn't satisfied
				}
			}

			if (!fail) {
				int inf2 = skill_get_inf2(skid);

				if (!sd->status.skill[sk_idx].lv && (
					(inf2&INF2_QUEST_SKILL && !battle_config.quest_skill_learn) ||
					inf2&INF2_WEDDING_SKILL ||
					(inf2&INF2_SPIRIT_SKILL && !sd->sc.data[SC_SPIRIT])
				))
					continue; //Cannot be learned via normal means. Note this check DOES allows raising already known skills.

				sd->status.skill[sk_idx].id = skid;

				if(inf2&INF2_SPIRIT_SKILL) { //Spirit skills cannot be learned, they will only show up on your tree when you get buffed.
					sd->status.skill[sk_idx].lv = 1; // need to manually specify a skill level
					sd->status.skill[sk_idx].flag = SKILL_FLAG_TEMPORARY; //So it is not saved, and tagged as a "bonus" skill.
				}
				flag = 1; // skill list has changed, perform another pass
			}
		}
	} while(flag);

	if( c > 0 && sd->status.skill_point == 0 && pc_is_taekwon_ranker(sd) ) {
		unsigned short skid = 0;
		/* Taekwon Ranker Bonus Skill Tree
		============================================
		- Grant All Taekwon Tree, but only as Bonus Skills in case they drop from ranking.
		- (c > 0) to avoid grant Novice Skill Tree in case of Skill Reset (need more logic)
		- (sd->status.skill_point == 0) to wait until all skill points are assigned to avoid problems with Job Change quest. */

		for( i = 0; i < MAX_SKILL_TREE && (skid = skill_tree[c][i].id) > 0; i++ ) {
			uint16 sk_idx = 0;
			if (!(sk_idx = skill_get_index(skid)))
				continue;
			if( (skill_get_inf2(skid)&(INF2_QUEST_SKILL|INF2_WEDDING_SKILL)) )
				continue; //Do not include Quest/Wedding skills.
			if( sd->status.skill[sk_idx].id == 0 ) {
				sd->status.skill[sk_idx].id = skid;
				sd->status.skill[sk_idx].flag = SKILL_FLAG_TEMPORARY; // So it is not saved, and tagged as a "bonus" skill.
			} else if( skid != NV_BASIC )
				sd->status.skill[sk_idx].flag = SKILL_FLAG_REPLACED_LV_0 + sd->status.skill[sk_idx].lv; // Remember original level
			sd->status.skill[sk_idx].lv = skill_tree_get_max(skid, sd->status.class_);
		}
	}
}

//Checks if you can learn a new skill after having leveled up a skill.
static void pc_check_skilltree(struct map_session_data *sd)
{
	int i, flag = 0;
	int c = 0;

	if (battle_config.skillfree)
		return; //Function serves no purpose if this is set

	i = pc_calc_skilltree_normalize_job(sd);
	c = pc_mapid2jobid(i, sd->status.sex);
	if (c == -1) { //Unable to normalize job??
		ShowError("pc_check_skilltree: Unable to normalize job %d for character %s (%d:%d)\n", i, sd->status.name, sd->status.account_id, sd->status.char_id);
		return;
	}
	c = pc_class2idx(c);

	do {
		uint16 skid = 0;

		flag = 0;
		for (i = 0; i < MAX_SKILL_TREE && (skid = skill_tree[c][i].id) > 0; i++ ) {
			uint16 sk_idx = skill_get_index(skid);
			bool fail = false;
			uint8 j = 0;

			if (sd->status.skill[sk_idx].id) //Already learned
				continue;

			// Checking required skills
			for (j = 0; j < MAX_PC_SKILL_REQUIRE; j++) {
				uint16 sk_need_id = skill_tree[c][i].need[j].id;
				uint16 sk_need_idx = 0;

				if (sk_need_id && (sk_need_idx = skill_get_index(sk_need_id))) {
					short sk_need = sk_need_id;

					if (sd->status.skill[sk_need_idx].id == 0 || sd->status.skill[sk_need_idx].flag == SKILL_FLAG_TEMPORARY || sd->status.skill[sk_need_idx].flag == SKILL_FLAG_PLAGIARIZED)
						sk_need = 0; //Not learned.
					else if (sd->status.skill[sk_need_idx].flag >= SKILL_FLAG_REPLACED_LV_0) //Real lerned level
						sk_need = sd->status.skill[sk_need_idx].flag - SKILL_FLAG_REPLACED_LV_0;
					else
						sk_need = pc_checkskill(sd,sk_need_id);

					if (sk_need < skill_tree[c][i].need[j].lv) {
						fail = true;
						break;
					}
				}
			}

			if( fail )
				continue;
			if( sd->status.job_level < skill_tree[c][i].joblv )
				continue;

			j = skill_get_inf2(skid);
			if( !sd->status.skill[sk_idx].lv && (
				(j&INF2_QUEST_SKILL && !battle_config.quest_skill_learn) ||
				j&INF2_WEDDING_SKILL ||
				(j&INF2_SPIRIT_SKILL && !sd->sc.data[SC_SPIRIT])
			) )
				continue; //Cannot be learned via normal means.

			sd->status.skill[sk_idx].id = skid;
			flag = 1;
		}
	} while(flag);
}

// Make sure all the skills are in the correct condition
// before persisting to the backend.. [MouseJstr]
void pc_clean_skilltree(struct map_session_data *sd)
{
	uint16 i;
	for (i = 0; i < MAX_SKILL; i++){
		if (sd->status.skill[i].flag == SKILL_FLAG_TEMPORARY || sd->status.skill[i].flag == SKILL_FLAG_PLAGIARIZED) {
			sd->status.skill[i].id = 0;
			sd->status.skill[i].lv = 0;
			sd->status.skill[i].flag = SKILL_FLAG_PERMANENT;
		}
		else if (sd->status.skill[i].flag >= SKILL_FLAG_REPLACED_LV_0){
			sd->status.skill[i].lv = sd->status.skill[i].flag - SKILL_FLAG_REPLACED_LV_0;
			sd->status.skill[i].flag = SKILL_FLAG_PERMANENT;
		}
	}
}

int pc_calc_skilltree_normalize_job(struct map_session_data *sd)
{
	int skill_point, novice_skills;
	int c = sd->class_;

	if (!battle_config.skillup_limit || pc_has_permission(sd, PC_PERM_ALL_SKILL))
		return c;

	skill_point = pc_calc_skillpoint(sd);

	novice_skills = job_info[pc_class2idx(JOB_NOVICE)].max_level[1] - 1;

	// limit 1st class and above to novice job levels
	if(skill_point < novice_skills)
	{
		c = MAPID_NOVICE;
	}
	// limit 2nd class and above to first class job levels (super novices are exempt)
	else if (sd->class_&JOBL_2 && (sd->class_&MAPID_UPPERMASK) != MAPID_SUPER_NOVICE)
	{
		// regenerate change_level_2nd
		if (!sd->change_level_2nd)
		{
			if (sd->class_&JOBL_THIRD)
			{
				// if neither 2nd nor 3rd jobchange levels are known, we have to assume a default for 2nd
				if (!sd->change_level_3rd)
					sd->change_level_2nd = job_info[pc_class2idx(pc_mapid2jobid(sd->class_&MAPID_UPPERMASK, sd->status.sex))].max_level[1];
				else
					sd->change_level_2nd = 1 + skill_point + sd->status.skill_point
						- (sd->status.job_level - 1)
						- (sd->change_level_3rd - 1)
						- novice_skills;
			}
			else
			{
				sd->change_level_2nd = 1 + skill_point + sd->status.skill_point
						- (sd->status.job_level - 1)
						- novice_skills;

			}

			pc_setglobalreg(sd, add_str("jobchange_level"), sd->change_level_2nd);
		}

		if (skill_point < novice_skills + (sd->change_level_2nd - 1))
		{
			c &= MAPID_BASEMASK;
		}
		// limit 3rd class to 2nd class/trans job levels
		else if(sd->class_&JOBL_THIRD)
		{
			// regenerate change_level_3rd
			if (!sd->change_level_3rd)
			{
					sd->change_level_3rd = 1 + skill_point + sd->status.skill_point
						- (sd->status.job_level - 1)
						- (sd->change_level_2nd - 1)
						- novice_skills;
					pc_setglobalreg(sd, add_str("jobchange_level_3rd"), sd->change_level_3rd);
			}

			if (skill_point < novice_skills + (sd->change_level_2nd - 1) + (sd->change_level_3rd - 1))
				c &= MAPID_UPPERMASK;
		}
	}

	// restore non-limiting flags
	c |= sd->class_&(JOBL_UPPER|JOBL_BABY);

	return c;
}

/*==========================================
 * Updates the weight status
 *------------------------------------------
 * 1: overweight 50%
 * 2: overweight 90%
 * It's assumed that SC_WEIGHT50 and SC_WEIGHT90 are only started/stopped here.
 */
void pc_updateweightstatus(struct map_session_data *sd)
{
	int old_overweight;
	int new_overweight;

	nullpo_retv(sd);

	old_overweight = (sd->sc.data[SC_WEIGHT90]) ? 2 : (sd->sc.data[SC_WEIGHT50]) ? 1 : 0;
	new_overweight = (pc_is90overweight(sd)) ? 2 : (pc_is50overweight(sd)) ? 1 : 0;

	if( old_overweight == new_overweight )
		return; // no change

	// stop old status change
	if( old_overweight == 1 )
		status_change_end(&sd->bl, SC_WEIGHT50, INVALID_TIMER);
	else if( old_overweight == 2 )
		status_change_end(&sd->bl, SC_WEIGHT90, INVALID_TIMER);

	// start new status change
	if( new_overweight == 1 )
		sc_start(&sd->bl,&sd->bl, SC_WEIGHT50, 100, 0, 0);
	else if( new_overweight == 2 )
		sc_start(&sd->bl,&sd->bl, SC_WEIGHT90, 100, 0, 0);

	// update overweight status
	sd->regen.state.overweight = new_overweight;
}

int pc_disguise(struct map_session_data *sd, int class_)
{
	if (!class_ && !sd->disguise)
		return 0;
	if (class_ && sd->disguise == class_)
		return 0;

	if(sd->sc.option&OPTION_INVISIBLE)
  	{	//Character is invisible. Stealth class-change. [Skotlex]
		sd->disguise = class_; //viewdata is set on uncloaking.
		return 2;
	}

	if (sd->bl.prev != NULL) {
		pc_stop_walking(sd, 0);
		clif_clearunit_area(&sd->bl, CLR_OUTSIGHT);
	}

	if (!class_) {
		sd->disguise = 0;
		class_ = sd->status.class_;
	} else
		sd->disguise=class_;

	status_set_viewdata(&sd->bl, class_);
	clif_changeoption(&sd->bl);

	if (sd->bl.prev != NULL) {
		clif_spawn(&sd->bl);
		if (class_ == sd->status.class_ && pc_iscarton(sd))
		{	//It seems the cart info is lost on undisguise.
			clif_cartlist(sd);
			clif_updatestatus(sd,SP_CARTINFO);
		}
		if (sd->chatID) {
			struct chat_data* cd;
			nullpo_retr(1, sd);
			cd = (struct chat_data*)map_id2bl(sd->chatID);
			if( cd != NULL || (struct block_list*)sd == cd->owner )
				clif_dispchat(cd,0);
		}
	}
	return 1;
}

/// Show error message
#define PC_BONUS_SHOW_ERROR(type,type2,val) { ShowError("%s: %s: Invalid %s %d.\n",__FUNCTION__,#type,#type2,(val)); break; }
/// Check for valid Element, break & show error message if invalid Element
#define PC_BONUS_CHK_ELEMENT(ele,bonus) { if (!CHK_ELEMENT((ele))) { PC_BONUS_SHOW_ERROR((bonus),Element,(ele)); }}
/// Check for valid Race, break & show error message if invalid Race
#define PC_BONUS_CHK_RACE(rc,bonus) { if (!CHK_RACE((rc))) { PC_BONUS_SHOW_ERROR((bonus),Race,(rc)); }}
/// Check for valid Race2, break & show error message if invalid Race2
#define PC_BONUS_CHK_RACE2(rc2,bonus) { if (!CHK_RACE2((rc2))) { PC_BONUS_SHOW_ERROR((bonus),Race2,(rc2)); }}
/// Check for valid Class, break & show error message if invalid Class
#define PC_BONUS_CHK_CLASS(cl,bonus) { if (!CHK_CLASS((cl))) { PC_BONUS_SHOW_ERROR((bonus),Class,(cl)); }}
/// Check for valid Size, break & show error message if invalid Size
#define PC_BONUS_CHK_SIZE(sz,bonus) { if (!CHK_MOBSIZE((sz))) { PC_BONUS_SHOW_ERROR((bonus),Size,(sz)); }}
/// Check for valid SC, break & show error message if invalid SC
#define PC_BONUS_CHK_SC(sc,bonus) { if ((sc) <= SC_NONE || (sc) >= SC_MAX) { PC_BONUS_SHOW_ERROR((bonus),Effect,(sc)); }}

static void pc_bonus_autospell(struct s_autospell *spell, int max, short id, short lv, short rate, short flag, unsigned short card_id)
{
	uint8 i;

	if( !rate )
		return;

	for( i = 0; i < max && spell[i].id; i++ )
	{
		if( (spell[i].card_id == card_id || spell[i].rate < 0 || rate < 0) && spell[i].id == id && spell[i].lv == lv )
		{
			if( !battle_config.autospell_stacking && spell[i].rate > 0 && rate > 0 )
				return;
			rate += spell[i].rate;
			break;
		}
	}
	if (i == max) {
		ShowWarning("pc_bonus_autospell: Reached max (%d) number of autospells per character!\n", max);
		return;
	}
	spell[i].id = id;
	spell[i].lv = lv;
	spell[i].rate = rate;
	//Auto-update flag value.
	if (!(flag&BF_RANGEMASK)) flag|=BF_SHORT|BF_LONG; //No range defined? Use both.
	if (!(flag&BF_WEAPONMASK)) flag|=BF_WEAPON; //No attack type defined? Use weapon.
	if (!(flag&BF_SKILLMASK)) {
		if (flag&(BF_MAGIC|BF_MISC)) flag|=BF_SKILL; //These two would never trigger without BF_SKILL
		if (flag&BF_WEAPON) flag|=BF_NORMAL; //By default autospells should only trigger on normal weapon attacks.
	}
	spell[i].flag|= flag;
	spell[i].card_id = card_id;
}

static void pc_bonus_autospell_onskill(struct s_autospell *spell, int max, short src_skill, short id, short lv, short rate, unsigned short card_id)
{
	uint8 i;

	if( !rate )
		return;

	for( i = 0; i < max && spell[i].id; i++ )
	{
		;  // each autospell works independently
	}

	if( i == max )
	{
		ShowWarning("pc_bonus_autospell_onskill: Reached max (%d) number of autospells per character!\n", max);
		return;
	}

	spell[i].flag = src_skill;
	spell[i].id	= id;
	spell[i].lv = lv;
	spell[i].rate = rate;
	spell[i].card_id = card_id;
	return;
}

/**
 * Add inflict effect bonus for player while attacking/atatcked
 * @param effect Effect array
 * @param max Max array
 * @param sc SC/Effect type
 * @param rate Success chance
 * @param arrow_rate success chance if bonus comes from arrow-type item
 * @param flag Target flag
 * @param duration Duration. If 0 use default duration lookup for associated skill with level 7
 **/
static void pc_bonus_addeff(struct s_addeffect* effect, int max, enum sc_type sc, short rate, short arrow_rate, unsigned char flag, unsigned int duration)
{
	uint16 i;

	if (!(flag&(ATF_SHORT|ATF_LONG)))
		flag |= ATF_SHORT|ATF_LONG; //Default range: both
	if (!(flag&(ATF_TARGET|ATF_SELF)))
		flag |= ATF_TARGET; //Default target: enemy.
	if (!(flag&(ATF_WEAPON|ATF_MAGIC|ATF_MISC)))
		flag |= ATF_WEAPON; //Default type: weapon.

	if (!duration)
		duration = skill_get_time2(status_sc2skill(sc),7);

	for (i = 0; i < max && effect[i].flag; i++) {
		if (effect[i].sc == sc && effect[i].flag == flag) {
			effect[i].rate += rate;
			effect[i].arrow_rate += arrow_rate;
			effect[i].duration = max(effect[i].duration, duration);
			return;
		}
	}
	if (i == max) {
		ShowWarning("pc_bonus_addeff: Reached max (%d) number of add effects per character!\n", max);
		return;
	}
	effect[i].sc = sc;
	effect[i].rate = rate;
	effect[i].arrow_rate = arrow_rate;
	effect[i].flag = flag;
	effect[i].duration = duration;
}

/**
 * Add inflict effect bonus for player while attacking using skill
 * @param effect Effect array
 * @param max Max array
 * @param sc SC/Effect type
 * @param rate Success chance
 * @param flag Target flag
 * @param duration Duration. If 0 use default duration lookup for associated skill with level 7
 **/
static void pc_bonus_addeff_onskill(struct s_addeffectonskill* effect, int max, enum sc_type sc, short rate, short skill_id, unsigned char target, unsigned int duration)
{
	uint8 i;

	if (!duration)
		duration = skill_get_time2(status_sc2skill(sc),7);

	for( i = 0; i < max && effect[i].skill_id; i++ ) {
		if( effect[i].sc == sc && effect[i].skill_id == skill_id && effect[i].target == target ) {
			effect[i].rate += rate;
			effect[i].duration = max(effect[i].duration, duration);
			return;
		}
	}
	if( i == max ) {
		ShowWarning("pc_bonus_addeff_onskill: Reached max (%d) number of add effects on skill per character!\n", max);
		return;
	}
	effect[i].sc = sc;
	effect[i].rate = rate;
	effect[i].skill_id = skill_id;
	effect[i].target = target;
	effect[i].duration = duration;
}

/** Adjust/add drop rate modifier for player
* @param drop: Player's sd->add_drop (struct s_add_drop)
* @param max: Max bonus can be received
* @param nameid: item id that will be dropped
* @param group: group id
* @param class_: target class
* @param race: target race. if < 0, means monster_id
* @param rate: rate value: 1 ~ 10000. If < 0, it will be multiplied with mob level/10
*/
static void pc_bonus_item_drop(struct s_add_drop *drop, const short max, unsigned short nameid, uint16 group, int class_, short race, int rate)
{
	uint8 i;

	if (!nameid && !group) {
		ShowWarning("pc_bonus_item_drop: No Item ID nor Item Group ID specified.\n");
		return;
	}
	if (nameid && !itemdb_exists(nameid)) {
		ShowWarning("pc_bonus_item_drop: Invalid item id %hu\n",nameid);
		return;
	}
	if (group && !itemdb_group_exists(group)) {
		ShowWarning("pc_bonus_item_drop: Invalid Item Group %hu\n",group);
		return;
	}

	//Apply config rate adjustment settings.
	if (rate >= 0) { //Absolute drop.
		if (battle_config.item_rate_adddrop != 100)
			rate = rate*battle_config.item_rate_adddrop/100;
		if (rate < battle_config.item_drop_adddrop_min)
			rate = battle_config.item_drop_adddrop_min;
		else if (rate > battle_config.item_drop_adddrop_max)
			rate = battle_config.item_drop_adddrop_max;
	} else { //Relative drop, max/min limits are applied at drop time.
		if (battle_config.item_rate_adddrop != 100)
			rate = rate*battle_config.item_rate_adddrop/100;
		if (rate > -1)
			rate = -1;
	}

	//Find match entry, and adjust the rate only
	for (i = 0; i < max; i++) {
		if (!&drop[i] || (!drop[i].nameid && !drop[i].group))
			continue;
		if (drop[i].nameid == nameid &&
			drop[i].group == group &&
			drop[i].race == race &&
			drop[i].class_ == class_
			)
		{
			//Adjust the rate if it has same classification
			if ((rate < 0  && drop[i].rate < 0) ||
				(rate > 0  && drop[i].rate > 0))
			{
				drop[i].rate += rate;
				return;
			}
		}
	}
	ARR_FIND(0,max,i,!&drop[i] || (drop[i].nameid == 0 && drop[i].group == 0));
	if (i >= max) {
		ShowWarning("pc_bonus_item_drop: Reached max (%d) number of added drops per character! (nameid:%hu group:%d class_:%d race:%d rate:%d)\n",max,nameid,group,class_,race,rate);
		return;
	}
	drop[i].nameid = nameid;
	drop[i].group = group;
	drop[i].race = race;
	drop[i].class_ = class_;
	drop[i].rate = rate;
}

bool pc_addautobonus(struct s_autobonus *bonus,char max,const char *script,short rate,unsigned int dur,short flag,const char *other_script,unsigned int pos,bool onskill)
{
	int i;

	ARR_FIND(0, max, i, bonus[i].rate == 0);
	if( i == max )
	{
		ShowWarning("pc_addautobonus: Reached max (%d) number of autobonus per character!\n", max);
		return false;
	}

	if( !onskill )
	{
		if( !(flag&BF_RANGEMASK) )
			flag|=BF_SHORT|BF_LONG; //No range defined? Use both.
		if( !(flag&BF_WEAPONMASK) )
			flag|=BF_WEAPON; //No attack type defined? Use weapon.
		if( !(flag&BF_SKILLMASK) )
		{
			if( flag&(BF_MAGIC|BF_MISC) )
				flag|=BF_SKILL; //These two would never trigger without BF_SKILL
			if( flag&BF_WEAPON )
				flag|=BF_NORMAL|BF_SKILL;
		}
	}

	bonus[i].rate = rate;
	bonus[i].duration = dur;
	bonus[i].active = INVALID_TIMER;
	bonus[i].atk_type = flag;
	bonus[i].pos = pos;
	bonus[i].bonus_script = aStrdup(script);
	bonus[i].other_script = other_script?aStrdup(other_script):NULL;
	return true;
}

void pc_delautobonus(struct map_session_data* sd, struct s_autobonus *autobonus,char max,bool restore)
{
	int i;
	if (!sd)
		return;

	for( i = 0; i < max; i++ )
	{
		if( autobonus[i].active != INVALID_TIMER )
		{
			if( restore && (sd->state.autobonus&autobonus[i].pos) == autobonus[i].pos)
			{
				if( autobonus[i].bonus_script )
				{
					int j;
					unsigned int equip_pos_idx = 0;
					//Create a list of all equipped positions to see if all items needed for the autobonus are still present [Playtester]
					for(j = 0; j < EQI_MAX; j++) {
						if(sd->equip_index[j] >= 0)
							equip_pos_idx |= sd->status.inventory[sd->equip_index[j]].equip;
					}
					if((equip_pos_idx&autobonus[i].pos) == autobonus[i].pos)
						script_run_autobonus(autobonus[i].bonus_script,sd,autobonus[i].pos);
				}
				continue;
			}
			else
			{ // Logout / Unequipped an item with an activated bonus
				delete_timer(autobonus[i].active,pc_endautobonus);
				autobonus[i].active = INVALID_TIMER;
			}
		}

		if( autobonus[i].bonus_script ) aFree(autobonus[i].bonus_script);
		if( autobonus[i].other_script ) aFree(autobonus[i].other_script);
		autobonus[i].bonus_script = autobonus[i].other_script = NULL;
		autobonus[i].rate = autobonus[i].atk_type = autobonus[i].duration = autobonus[i].pos = 0;
		autobonus[i].active = INVALID_TIMER;
	}
}

void pc_exeautobonus(struct map_session_data *sd,struct s_autobonus *autobonus)
{
	if (!sd || !autobonus)
		return;

	if( autobonus->other_script )
	{
		int j;
		unsigned int equip_pos_idx = 0;
		//Create a list of all equipped positions to see if all items needed for the autobonus are still present [Playtester]
		for(j = 0; j < EQI_MAX; j++) {
			if(sd->equip_index[j] >= 0)
				equip_pos_idx |= sd->status.inventory[sd->equip_index[j]].equip;
		}
		if((equip_pos_idx&autobonus->pos) == autobonus->pos)
			script_run_autobonus(autobonus->other_script,sd,autobonus->pos);
	}

	autobonus->active = add_timer(gettick()+autobonus->duration, pc_endautobonus, sd->bl.id, (intptr_t)autobonus);
	sd->state.autobonus |= autobonus->pos;
	status_calc_pc(sd,SCO_FORCE);
}

int pc_endautobonus(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd = map_id2sd(id);
	struct s_autobonus *autobonus = (struct s_autobonus *)data;

	nullpo_ret(sd);
	nullpo_ret(autobonus);

	autobonus->active = INVALID_TIMER;
	sd->state.autobonus &= ~autobonus->pos;
	status_calc_pc(sd,SCO_FORCE);
	return 0;
}

static void pc_bonus_addele(struct map_session_data* sd, unsigned char ele, short rate, short flag)
{
	uint8 i;
	struct weapon_data* wd;

	wd = (sd->state.lr_flag ? &sd->left_weapon : &sd->right_weapon);

	ARR_FIND(0, MAX_PC_BONUS, i, wd->addele2[i].rate == 0);

	if (i == MAX_PC_BONUS)
	{
		ShowWarning("pc_bonus_addele: Reached max (%d) possible bonuses for this player.\n", MAX_PC_BONUS);
		return;
	}

	if (!(flag&BF_RANGEMASK))
		flag |= BF_SHORT|BF_LONG;
	if (!(flag&BF_WEAPONMASK))
		flag |= BF_WEAPON;
	if (!(flag&BF_SKILLMASK))
	{
		if (flag&(BF_MAGIC|BF_MISC))
			flag |= BF_SKILL;
		if (flag&BF_WEAPON)
			flag |= BF_NORMAL|BF_SKILL;
	}

	wd->addele2[i].ele = ele;
	wd->addele2[i].rate = rate;
	wd->addele2[i].flag = flag;
}

static void pc_bonus_subele(struct map_session_data* sd, unsigned char ele, short rate, short flag)
{
	uint8 i;

	ARR_FIND(0, MAX_PC_BONUS, i, sd->subele2[i].rate == 0);

	if (i == MAX_PC_BONUS)
	{
		ShowWarning("pc_bonus_subele: Reached max (%d) possible bonuses for this player.\n", MAX_PC_BONUS);
		return;
	}

	if (!(flag&BF_RANGEMASK))
		flag |= BF_SHORT|BF_LONG;
	if (!(flag&BF_WEAPONMASK))
		flag |= BF_WEAPON;
	if (!(flag&BF_SKILLMASK))
	{
		if (flag&(BF_MAGIC|BF_MISC))
			flag |= BF_SKILL;
		if (flag&BF_WEAPON)
			flag |= BF_NORMAL|BF_SKILL;
	}

	sd->subele2[i].ele = ele;
	sd->subele2[i].rate = rate;
	sd->subele2[i].flag = flag;
}

/** Add item group heal rate bonus to player
* @param sd Player
* @param group_id Item Group ID
* @param rate
* @author Cydh
*/
void pc_itemgrouphealrate(struct map_session_data *sd, uint16 group_id, short rate) {
	struct s_pc_itemgrouphealrate *entry;
	uint8 i;

	for (i = 0; i < sd->itemgrouphealrate_count; i++) {
		if (sd->itemgrouphealrate[i]->group_id == group_id)
			break;
	}

	if (i != sd->itemgrouphealrate_count) {
		sd->itemgrouphealrate[i]->rate += rate;
		return;
	}

	if (i >= UINT8_MAX) {
		ShowError("pc_itemgrouphealrate_add: Reached max (%d) possible bonuses for this player %d\n", UINT8_MAX);
		return;
	}

	entry = ers_alloc(pc_itemgrouphealrate_ers, struct s_pc_itemgrouphealrate);
	entry->group_id = group_id;
	entry->rate = rate;

	RECREATE(sd->itemgrouphealrate, struct s_pc_itemgrouphealrate *, sd->itemgrouphealrate_count+1);
	sd->itemgrouphealrate[sd->itemgrouphealrate_count++] = entry;
}

/** Clear item group heal rate from player
* @param sd Player
* @author Cydh
*/
void pc_itemgrouphealrate_clear(struct map_session_data *sd) {
	if (!sd || !sd->itemgrouphealrate_count)
		return;
	else {
		uint8 i;
		for( i = 0; i < sd->itemgrouphealrate_count; i++ )
			ers_free(pc_itemgrouphealrate_ers, sd->itemgrouphealrate[i]);
		sd->itemgrouphealrate_count = 0;
		aFree(sd->itemgrouphealrate);
		sd->itemgrouphealrate = NULL;
	}
}

/*==========================================
 * Add a bonus(type) to player sd
 * format: bonus bBonusName,val;
 * @param sd
 * @param type Bonus type used by bBonusName
 * @param val Value that usually for rate or fixed value
 *------------------------------------------*/
void pc_bonus(struct map_session_data *sd,int type,int val)
{
	struct status_data *status;
	int bonus;
	nullpo_retv(sd);

	status = &sd->base_status;

	switch(type){
		case SP_STR:
		case SP_AGI:
		case SP_VIT:
		case SP_INT:
		case SP_DEX:
		case SP_LUK:
			if(sd->state.lr_flag != 2)
				sd->param_bonus[type-SP_STR]+=val;
			break;
		case SP_ATK1:
			if(!sd->state.lr_flag) {
				bonus = status->rhw.atk + val;
				status->rhw.atk = cap_value(bonus, 0, USHRT_MAX);
			}
			else if(sd->state.lr_flag == 1) {
				bonus = status->lhw.atk + val;
				status->lhw.atk =  cap_value(bonus, 0, USHRT_MAX);
			}
			break;
		case SP_ATK2:
			if(!sd->state.lr_flag) {
				bonus = status->rhw.atk2 + val;
				status->rhw.atk2 = cap_value(bonus, 0, USHRT_MAX);
			}
			else if(sd->state.lr_flag == 1) {
				bonus = status->lhw.atk2 + val;
				status->lhw.atk2 =  cap_value(bonus, 0, USHRT_MAX);
			}
			break;
		case SP_BASE_ATK:
			if(sd->state.lr_flag != 2) {
#ifdef RENEWAL
				sd->bonus.eatk += val;
#else
				bonus = status->batk + val;
				status->batk = cap_value(bonus, 0, USHRT_MAX);
#endif
			}
			break;
		case SP_DEF1:
			if(sd->state.lr_flag != 2) {
				bonus = status->def + val;
#ifdef RENEWAL
				status->def = cap_value(bonus, SHRT_MIN, SHRT_MAX);
#else
				status->def = cap_value(bonus, CHAR_MIN, CHAR_MAX);
#endif
			}
			break;
		case SP_DEF2:
			if(sd->state.lr_flag != 2) {
				bonus = status->def2 + val;
				status->def2 = cap_value(bonus, SHRT_MIN, SHRT_MAX);
			}
			break;
		case SP_MDEF1:
			if(sd->state.lr_flag != 2) {
				bonus = status->mdef + val;
#ifdef RENEWAL
				status->mdef = cap_value(bonus, SHRT_MIN, SHRT_MAX);
#else
				status->mdef = cap_value(bonus, CHAR_MIN, CHAR_MAX);
#endif
				if( sd->state.lr_flag == 3 ) {//Shield, used for royal guard
					sd->bonus.shieldmdef += bonus;
				}
			}
			break;
		case SP_MDEF2:
			if(sd->state.lr_flag != 2) {
				bonus = status->mdef2 + val;
				status->mdef2 = cap_value(bonus, SHRT_MIN, SHRT_MAX);
			}
			break;
		case SP_HIT:
			if(sd->state.lr_flag != 2) {
				bonus = status->hit + val;
				status->hit = cap_value(bonus, SHRT_MIN, SHRT_MAX);
			} else
				sd->bonus.arrow_hit+=val;
			break;
		case SP_FLEE1:
			if(sd->state.lr_flag != 2) {
				bonus = status->flee + val;
				status->flee = cap_value(bonus, SHRT_MIN, SHRT_MAX);
			}
			break;
		case SP_FLEE2:
			if(sd->state.lr_flag != 2) {
				bonus = status->flee2 + val*10;
				status->flee2 = cap_value(bonus, SHRT_MIN, SHRT_MAX);
			}
			break;
		case SP_CRITICAL:
			if(sd->state.lr_flag != 2) {
				bonus = status->cri + val*10;
				status->cri = cap_value(bonus, SHRT_MIN, SHRT_MAX);
			} else
				sd->bonus.arrow_cri += val*10;
			break;
		case SP_ATKELE:
			PC_BONUS_CHK_ELEMENT(val,SP_ATKELE);
			switch (sd->state.lr_flag)
			{
			case 2:
				switch (sd->status.weapon) {
					case W_BOW:
					case W_REVOLVER:
					case W_RIFLE:
					case W_GATLING:
					case W_SHOTGUN:
					case W_GRENADE:
						//Become weapon element.
						status->rhw.ele=val;
						break;
					default: //Become arrow element.
						sd->bonus.arrow_ele=val;
						break;
				}
				break;
			case 1:
				status->lhw.ele=val;
				break;
			default:
				status->rhw.ele=val;
				break;
			}
			break;
		case SP_DEFELE:
			PC_BONUS_CHK_ELEMENT(val,SP_DEFELE);
			if(sd->state.lr_flag != 2)
				status->def_ele=val;
			break;
		case SP_MAXHP:
			if(sd->state.lr_flag == 2)
				break;
			sd->bonus.hp += val;
			break;
		case SP_MAXSP:
			if(sd->state.lr_flag == 2)
				break;
			sd->bonus.sp += val;
			break;
		case SP_MAXHPRATE:
			if(sd->state.lr_flag != 2)
				sd->hprate+=val;
			break;
		case SP_MAXSPRATE:
			if(sd->state.lr_flag != 2)
				sd->sprate+=val;
			break;
		case SP_SPRATE:
			if(sd->state.lr_flag != 2)
				sd->dsprate+=val;
			break;
		case SP_ATTACKRANGE:
			switch (sd->state.lr_flag) {
			case 2:
				switch (sd->status.weapon) {
					case W_BOW:
					case W_REVOLVER:
					case W_RIFLE:
					case W_GATLING:
					case W_SHOTGUN:
					case W_GRENADE:
						status->rhw.range += val;
				}
				break;
			case 1:
				status->lhw.range += val;
				break;
			default:
				status->rhw.range += val;
				break;
			}
			break;
		case SP_SPEED_RATE:	//Non stackable increase
			if(sd->state.lr_flag != 2)
				sd->bonus.speed_rate = min(sd->bonus.speed_rate, -val);
			break;
		case SP_SPEED_ADDRATE:	//Stackable increase
			if(sd->state.lr_flag != 2)
				sd->bonus.speed_add_rate -= val;
			break;
		case SP_ASPD:	//Raw increase
			if(sd->state.lr_flag != 2)
				sd->bonus.aspd_add -= 10*val;
			break;
		case SP_ASPD_RATE:	//Stackable increase - Made it linear as per rodatazone
			if(sd->state.lr_flag != 2)
#ifndef RENEWAL_ASPD
				status->aspd_rate -= 10*val;
#else
				status->aspd_rate2 += val;
#endif
			break;
		case SP_HP_RECOV_RATE:
			if(sd->state.lr_flag != 2)
				sd->hprecov_rate += val;
			break;
		case SP_SP_RECOV_RATE:
			if(sd->state.lr_flag != 2)
				sd->sprecov_rate += val;
			break;
		case SP_CRITICAL_DEF:
			if(sd->state.lr_flag != 2)
				sd->bonus.critical_def += val;
			break;
		case SP_NEAR_ATK_DEF:
			if(sd->state.lr_flag != 2)
				sd->bonus.near_attack_def_rate += val;
			break;
		case SP_LONG_ATK_DEF:
			if(sd->state.lr_flag != 2)
				sd->bonus.long_attack_def_rate += val;
			break;
		case SP_DOUBLE_RATE:
			if(sd->state.lr_flag == 0 && sd->bonus.double_rate < val)
				sd->bonus.double_rate = val;
			break;
		case SP_DOUBLE_ADD_RATE:
			if(sd->state.lr_flag == 0)
				sd->bonus.double_add_rate += val;
			break;
		case SP_MATK_RATE:
			if(sd->state.lr_flag != 2)
				sd->matk_rate += val;
			break;
		case SP_IGNORE_DEF_ELE:
			PC_BONUS_CHK_ELEMENT(val,SP_IGNORE_DEF_ELE);
			if(!sd->state.lr_flag)
				sd->right_weapon.ignore_def_ele |= 1<<val;
			else if(sd->state.lr_flag == 1)
				sd->left_weapon.ignore_def_ele |= 1<<val;
			break;
		case SP_IGNORE_DEF_RACE:
			PC_BONUS_CHK_RACE(val,SP_IGNORE_DEF_RACE);
			if(!sd->state.lr_flag)
				sd->right_weapon.ignore_def_race |= 1<<val;
			else if(sd->state.lr_flag == 1)
				sd->left_weapon.ignore_def_race |= 1<<val;
			break;
		case SP_IGNORE_DEF_CLASS:
			PC_BONUS_CHK_CLASS(val,SP_IGNORE_DEF_CLASS);
			if(!sd->state.lr_flag)
				sd->right_weapon.ignore_def_class |= 1<<val;
			else if(sd->state.lr_flag == 1)
				sd->left_weapon.ignore_def_class |= 1<<val;
			break;
		case SP_ATK_RATE:
			if(sd->state.lr_flag != 2)
				sd->bonus.atk_rate += val;
			break;
		case SP_MAGIC_ATK_DEF:
			if(sd->state.lr_flag != 2)
				sd->bonus.magic_def_rate += val;
			break;
		case SP_MISC_ATK_DEF:
			if(sd->state.lr_flag != 2)
				sd->bonus.misc_def_rate += val;
			break;
		case SP_IGNORE_MDEF_ELE:
			PC_BONUS_CHK_ELEMENT(val,SP_IGNORE_MDEF_ELE);
			if(sd->state.lr_flag != 2)
				sd->bonus.ignore_mdef_ele |= 1<<val;
			break;
		case SP_IGNORE_MDEF_RACE:
			PC_BONUS_CHK_RACE(val,SP_IGNORE_MDEF_RACE);
			if(sd->state.lr_flag != 2)
				sd->bonus.ignore_mdef_race |= 1<<val;
			break;
		case SP_PERFECT_HIT_RATE:
			if(sd->state.lr_flag != 2 && sd->bonus.perfect_hit < val)
				sd->bonus.perfect_hit = val;
			break;
		case SP_PERFECT_HIT_ADD_RATE:
			if(sd->state.lr_flag != 2)
				sd->bonus.perfect_hit_add += val;
			break;
		case SP_CRITICAL_RATE:
			if(sd->state.lr_flag != 2)
				sd->critical_rate+=val;
			break;
		case SP_DEF_RATIO_ATK_ELE:
			PC_BONUS_CHK_ELEMENT(val,SP_DEF_RATIO_ATK_ELE);
			if(!sd->state.lr_flag)
				sd->right_weapon.def_ratio_atk_ele |= 1<<val;
			else if(sd->state.lr_flag == 1)
				sd->left_weapon.def_ratio_atk_ele |= 1<<val;
			break;
		case SP_DEF_RATIO_ATK_RACE:
			PC_BONUS_CHK_RACE(val,SP_DEF_RATIO_ATK_RACE);
			if(!sd->state.lr_flag)
				sd->right_weapon.def_ratio_atk_race |= 1<<val;
			else if(sd->state.lr_flag == 1)
				sd->left_weapon.def_ratio_atk_race |= 1<<val;
			break;
		case SP_DEF_RATIO_ATK_CLASS:
			PC_BONUS_CHK_CLASS(val,SP_DEF_RATIO_ATK_CLASS);
			if(!sd->state.lr_flag)
				sd->right_weapon.def_ratio_atk_class |= 1<<val;
			else if(sd->state.lr_flag == 1)
				sd->left_weapon.def_ratio_atk_class |= 1<<val;
			break;
		case SP_HIT_RATE:
			if(sd->state.lr_flag != 2)
				sd->hit_rate += val;
			break;
		case SP_FLEE_RATE:
			if(sd->state.lr_flag != 2)
				sd->flee_rate += val;
			break;
		case SP_FLEE2_RATE:
			if(sd->state.lr_flag != 2)
				sd->flee2_rate += val;
			break;
		case SP_DEF_RATE:
			if(sd->state.lr_flag != 2)
				sd->def_rate += val;
			break;
		case SP_DEF2_RATE:
			if(sd->state.lr_flag != 2)
				sd->def2_rate += val;
			break;
		case SP_MDEF_RATE:
			if(sd->state.lr_flag != 2)
				sd->mdef_rate += val;
			break;
		case SP_MDEF2_RATE:
			if(sd->state.lr_flag != 2)
				sd->mdef2_rate += val;
			break;
		case SP_RESTART_FULL_RECOVER:
			if(sd->state.lr_flag != 2)
				sd->special_state.restart_full_recover = 1;
			break;
		case SP_NO_CASTCANCEL:
			if(sd->state.lr_flag != 2)
				sd->special_state.no_castcancel = 1;
			break;
		case SP_NO_CASTCANCEL2:
			if(sd->state.lr_flag != 2)
				sd->special_state.no_castcancel2 = 1;
			break;
		case SP_NO_SIZEFIX:
			if(sd->state.lr_flag != 2)
				sd->special_state.no_sizefix = 1;
			break;
		case SP_NO_MAGIC_DAMAGE:
			if(sd->state.lr_flag == 2)
				break;
			val+= sd->special_state.no_magic_damage;
			sd->special_state.no_magic_damage = cap_value(val,0,100);
			break;
		case SP_NO_WEAPON_DAMAGE:
			if(sd->state.lr_flag == 2)
				break;
			val+= sd->special_state.no_weapon_damage;
			sd->special_state.no_weapon_damage = cap_value(val,0,100);
			break;
		case SP_NO_MISC_DAMAGE:
			if(sd->state.lr_flag == 2)
				break;
			val+= sd->special_state.no_misc_damage;
			sd->special_state.no_misc_damage = cap_value(val,0,100);
			break;
		case SP_NO_GEMSTONE:
			if(sd->state.lr_flag != 2 && sd->special_state.no_gemstone != 2)
				sd->special_state.no_gemstone = 1;
			break;
		case SP_INTRAVISION: // Maya Purple Card effect allowing to see Hiding/Cloaking people [DracoRPG]
			if(sd->state.lr_flag != 2) {
				sd->special_state.intravision = 1;
				clif_status_load(&sd->bl, SI_INTRAVISION, 1);
			}
			break;
		case SP_NO_KNOCKBACK:
			if(sd->state.lr_flag != 2)
				sd->special_state.no_knockback = 1;
			break;
		case SP_SPLASH_RANGE:
			if(sd->bonus.splash_range < val)
				sd->bonus.splash_range = val;
			break;
		case SP_SPLASH_ADD_RANGE:
			sd->bonus.splash_add_range += val;
			break;
		case SP_SHORT_WEAPON_DAMAGE_RETURN:
			if(sd->state.lr_flag != 2)
				sd->bonus.short_weapon_damage_return += val;
			break;
		case SP_LONG_WEAPON_DAMAGE_RETURN:
			if(sd->state.lr_flag != 2)
				sd->bonus.long_weapon_damage_return += val;
			break;
		case SP_MAGIC_DAMAGE_RETURN: //AppleGirl Was Here
			if(sd->state.lr_flag != 2)
				sd->bonus.magic_damage_return += val;
			break;
		case SP_ALL_STATS:	// [Valaris]
			if(sd->state.lr_flag!=2) {
				sd->param_bonus[SP_STR-SP_STR]+=val;
				sd->param_bonus[SP_AGI-SP_STR]+=val;
				sd->param_bonus[SP_VIT-SP_STR]+=val;
				sd->param_bonus[SP_INT-SP_STR]+=val;
				sd->param_bonus[SP_DEX-SP_STR]+=val;
				sd->param_bonus[SP_LUK-SP_STR]+=val;
			}
			break;
		case SP_AGI_VIT:	// [Valaris]
			if(sd->state.lr_flag!=2) {
				sd->param_bonus[SP_AGI-SP_STR]+=val;
				sd->param_bonus[SP_VIT-SP_STR]+=val;
			}
			break;
		case SP_AGI_DEX_STR:	// [Valaris]
			if(sd->state.lr_flag!=2) {
				sd->param_bonus[SP_AGI-SP_STR]+=val;
				sd->param_bonus[SP_DEX-SP_STR]+=val;
				sd->param_bonus[SP_STR-SP_STR]+=val;
			}
			break;
		case SP_PERFECT_HIDE: // [Valaris]
			if(sd->state.lr_flag!=2)
				sd->special_state.perfect_hiding=1;
			break;
		case SP_UNBREAKABLE:
			if(sd->state.lr_flag!=2)
				sd->bonus.unbreakable += val;
			break;
		case SP_UNBREAKABLE_WEAPON:
			if(sd->state.lr_flag != 2)
				sd->bonus.unbreakable_equip |= EQP_WEAPON;
			break;
		case SP_UNBREAKABLE_ARMOR:
			if(sd->state.lr_flag != 2)
				sd->bonus.unbreakable_equip |= EQP_ARMOR;
			break;
		case SP_UNBREAKABLE_HELM:
			if(sd->state.lr_flag != 2)
				sd->bonus.unbreakable_equip |= EQP_HELM;
			break;
		case SP_UNBREAKABLE_SHIELD:
			if(sd->state.lr_flag != 2)
				sd->bonus.unbreakable_equip |= EQP_SHIELD;
			break;
		case SP_UNBREAKABLE_GARMENT:
			if(sd->state.lr_flag != 2)
				sd->bonus.unbreakable_equip |= EQP_GARMENT;
			break;
		case SP_UNBREAKABLE_SHOES:
			if(sd->state.lr_flag != 2)
				sd->bonus.unbreakable_equip |= EQP_SHOES;
			break;
		case SP_CLASSCHANGE: // [Valaris]
			if(sd->state.lr_flag !=2)
				sd->bonus.classchange=val;
			break;
		case SP_LONG_ATK_RATE:
			if(sd->state.lr_flag != 2)	//[Lupus] it should stack, too. As any other cards rate bonuses
				sd->bonus.long_attack_atk_rate+=val;
			break;
		case SP_BREAK_WEAPON_RATE:
			if(sd->state.lr_flag != 2)
				sd->bonus.break_weapon_rate+=val;
			break;
		case SP_BREAK_ARMOR_RATE:
			if(sd->state.lr_flag != 2)
				sd->bonus.break_armor_rate+=val;
			break;
		case SP_ADD_STEAL_RATE:
			if(sd->state.lr_flag != 2)
				sd->bonus.add_steal_rate+=val;
			break;
		case SP_DELAYRATE:
			if(sd->state.lr_flag != 2)
				sd->delayrate+=val;
			break;
		case SP_CRIT_ATK_RATE:
			if(sd->state.lr_flag != 2)
				sd->bonus.crit_atk_rate += val;
			break;
		case SP_NO_REGEN:
			if(sd->state.lr_flag != 2)
				sd->regen.state.block|=val;
			break;
		case SP_UNSTRIPABLE_WEAPON:
			if(sd->state.lr_flag != 2)
				sd->bonus.unstripable_equip |= EQP_WEAPON;
			break;
		case SP_UNSTRIPABLE:
		case SP_UNSTRIPABLE_ARMOR:
			if(sd->state.lr_flag != 2)
				sd->bonus.unstripable_equip |= EQP_ARMOR;
			break;
		case SP_UNSTRIPABLE_HELM:
			if(sd->state.lr_flag != 2)
				sd->bonus.unstripable_equip |= EQP_HELM;
			break;
		case SP_UNSTRIPABLE_SHIELD:
			if(sd->state.lr_flag != 2)
				sd->bonus.unstripable_equip |= EQP_SHIELD;
			break;
		case SP_HP_DRAIN_VALUE: // bonus bHPDrainValue,n;
			if(!sd->state.lr_flag) {
				sd->right_weapon.hp_drain_class[CLASS_NORMAL] += val;
				sd->right_weapon.hp_drain_class[CLASS_BOSS] += val;
			} else if(sd->state.lr_flag == 1) {
				sd->left_weapon.hp_drain_class[CLASS_NORMAL] += val;
				sd->left_weapon.hp_drain_class[CLASS_BOSS] += val;
			}
			break;
		case SP_SP_DRAIN_VALUE: // bonus bSPDrainValue,n;
			if(!sd->state.lr_flag) {
				sd->right_weapon.sp_drain_class[CLASS_NORMAL] += val;
				sd->right_weapon.sp_drain_class[CLASS_BOSS] += val;
			} else if(sd->state.lr_flag == 1) {
				sd->left_weapon.sp_drain_class[CLASS_NORMAL] += val;
				sd->left_weapon.sp_drain_class[CLASS_BOSS] += val;
			}
			break;
		case SP_SP_GAIN_VALUE:
			if(!sd->state.lr_flag)
				sd->bonus.sp_gain_value += val;
			break;
		case SP_HP_GAIN_VALUE:
			if(!sd->state.lr_flag)
				sd->bonus.hp_gain_value += val;
			break;
		case SP_MAGIC_SP_GAIN_VALUE:
			if(!sd->state.lr_flag)
				sd->bonus.magic_sp_gain_value += val;
			break;
		case SP_MAGIC_HP_GAIN_VALUE:
			if(!sd->state.lr_flag)
				sd->bonus.magic_hp_gain_value += val;
			break;
		case SP_ADD_HEAL_RATE:
			if(sd->state.lr_flag != 2)
				sd->bonus.add_heal_rate += val;
			break;
		case SP_ADD_HEAL2_RATE:
			if(sd->state.lr_flag != 2)
				sd->bonus.add_heal2_rate += val;
			break;
		case SP_ADD_ITEM_HEAL_RATE:
			if(sd->state.lr_flag != 2)
				sd->bonus.itemhealrate2 += val;
			break;
		case SP_EMATK:
			if(sd->state.lr_flag != 2)
				sd->bonus.ematk += val;
			break;
#ifdef RENEWAL_CAST
		case SP_FIXCASTRATE:
			if(sd->state.lr_flag != 2)
				FIXEDCASTRATE(sd->bonus.fixcastrate,val);
			break;
		case SP_ADD_FIXEDCAST:
			if(sd->state.lr_flag != 2)
				sd->bonus.add_fixcast += val;
			break;
		case SP_CASTRATE:
		case SP_VARCASTRATE:
			if(sd->state.lr_flag != 2)
				sd->bonus.varcastrate += val;
			break;
		case SP_ADD_VARIABLECAST:
			if(sd->state.lr_flag != 2)
				sd->bonus.add_varcast += val;
			break;
#else
		case SP_ADD_FIXEDCAST:
		case SP_FIXCASTRATE:
		case SP_ADD_VARIABLECAST:
			//ShowWarning("pc_bonus: non-RENEWAL_CAST doesn't support this bonus %d.\n", type);
			break;
		case SP_VARCASTRATE:
		case SP_CASTRATE:
			if(sd->state.lr_flag != 2)
				sd->castrate += val;
			break;
#endif
		case SP_ADDMAXWEIGHT:
			if (sd->state.lr_flag != 2)
				sd->max_weight += val;
			break;
		case SP_ABSORB_DMG_MAXHP: // bonus bAbsorbDmgMaxHP,n;
			sd->bonus.absorb_dmg_maxhp = max(sd->bonus.absorb_dmg_maxhp, val);
			break;
		default:
			ShowWarning("pc_bonus: unknown type %d %d !\n",type,val);
			break;
	}
}

/*==========================================
 * Player bonus (type) with args type2 and val, called trough bonus2 (npc)
 * format: bonus2 bBonusName,type2,val;
 * @param sd
 * @param type Bonus type used by bBonusName
 * @param type2
 * @param val Value that usually for rate or fixed value
 *------------------------------------------*/
void pc_bonus2(struct map_session_data *sd,int type,int type2,int val)
{
	int i;

	nullpo_retv(sd);

	switch(type){
	case SP_ADDELE: // bonus2 bAddEle,e,x;
		PC_BONUS_CHK_ELEMENT(type2,SP_ADDELE);
		if(!sd->state.lr_flag)
			sd->right_weapon.addele[type2]+=val;
		else if(sd->state.lr_flag == 1)
			sd->left_weapon.addele[type2]+=val;
		else if(sd->state.lr_flag == 2)
			sd->arrow_addele[type2]+=val;
		break;
	case SP_ADDRACE: // bonus2 bAddRace,r,x;
		PC_BONUS_CHK_RACE(type2,SP_ADDRACE);
		if(!sd->state.lr_flag)
			sd->right_weapon.addrace[type2]+=val;
		else if(sd->state.lr_flag == 1)
			sd->left_weapon.addrace[type2]+=val;
		else if(sd->state.lr_flag == 2)
			sd->arrow_addrace[type2]+=val;
		break;
	case SP_ADDCLASS: // bonus2 bAddClass,c,x;
		PC_BONUS_CHK_CLASS(type2,SP_ADDCLASS);
		if(!sd->state.lr_flag)
			sd->right_weapon.addclass[type2]+=val;
		else if(sd->state.lr_flag == 1)
			sd->left_weapon.addclass[type2]+=val;
		else if(sd->state.lr_flag == 2)
			sd->arrow_addclass[type2]+=val;
		break;
	case SP_ADDSIZE: // bonus2 bAddSize,s,x;
		PC_BONUS_CHK_SIZE(type2,SP_ADDSIZE);
		if(!sd->state.lr_flag)
			sd->right_weapon.addsize[type2]+=val;
		else if(sd->state.lr_flag == 1)
			sd->left_weapon.addsize[type2]+=val;
		else if(sd->state.lr_flag == 2)
			sd->arrow_addsize[type2]+=val;
		break;
	case SP_SUBELE: // bonus2 bSubEle,e,x;
		PC_BONUS_CHK_ELEMENT(type2,SP_SUBELE);
		if(sd->state.lr_flag != 2)
			sd->subele[type2]+=val;
		break;
	case SP_SUBRACE: // bonus2 bSubRace,r,x;
		PC_BONUS_CHK_RACE(type2,SP_SUBRACE);
		if(sd->state.lr_flag != 2)
			sd->subrace[type2]+=val;
		break;
	case SP_SUBCLASS: // bonus2 bSubClass,c,x;
		PC_BONUS_CHK_CLASS(type2,SP_SUBCLASS);
		if(sd->state.lr_flag != 2)
			sd->subclass[type2]+=val;
		break;
	case SP_ADDEFF: // bonus2 bAddEff,eff,n;
		PC_BONUS_CHK_SC(type2,SP_ADDEFF);
		pc_bonus_addeff(sd->addeff, ARRAYLENGTH(sd->addeff), (sc_type)type2,
			sd->state.lr_flag != 2 ? val : 0, sd->state.lr_flag == 2 ? val : 0, 0, 0);
		break;
	case SP_ADDEFF2: // bonus2 bAddEff2,eff,n;
		PC_BONUS_CHK_SC(type2,SP_ADDEFF2);
		pc_bonus_addeff(sd->addeff, ARRAYLENGTH(sd->addeff), (sc_type)type2,
			sd->state.lr_flag != 2 ? val : 0, sd->state.lr_flag == 2 ? val : 0, ATF_SELF, 0);
		break;
	case SP_RESEFF: // bonus2 bResEff,eff,n;
		if (type2 < SC_COMMON_MIN || type2 > SC_COMMON_MAX) {
			ShowError("pc_bonus2: SP_RESEFF: %d is invalid effect.\n", type2);
			break;
		}
		if(sd->state.lr_flag == 2)
			break;
		i = sd->reseff[type2]+val;
		sd->reseff[type2]= cap_value(i, -10000, 10000);
		break;
	case SP_MAGIC_ADDELE: // bonus2 bMagicAddEle,e,x;
		PC_BONUS_CHK_ELEMENT(type2,SP_MAGIC_ADDELE);
		if(sd->state.lr_flag != 2)
			sd->magic_addele[type2]+=val;
		break;
	case SP_MAGIC_ADDRACE: // bonus2 bMagicAddRace,r,x;
		PC_BONUS_CHK_RACE(type2,SP_MAGIC_ADDRACE);
		if(sd->state.lr_flag != 2)
			sd->magic_addrace[type2]+=val;
		break;
	case SP_MAGIC_ADDCLASS: // bonus2 bMagicAddClass,c,x;
		PC_BONUS_CHK_CLASS(type2,SP_MAGIC_ADDCLASS);
		if(sd->state.lr_flag != 2)
			sd->magic_addclass[type2]+=val;
		break;
	case SP_MAGIC_ADDSIZE: // bonus2 bMagicAddSize,s,x;
		PC_BONUS_CHK_SIZE(type2,SP_MAGIC_ADDSIZE);
		if(sd->state.lr_flag != 2)
			sd->magic_addsize[type2]+=val;
		break;
	case SP_MAGIC_ATK_ELE: // bonus2 bMagicAtkEle,e,x;
		PC_BONUS_CHK_ELEMENT(type2,SP_MAGIC_ATK_ELE);
		if(sd->state.lr_flag != 2)
			sd->magic_atk_ele[type2]+=val;
		break;
	case SP_ADD_DAMAGE_CLASS: // bonus2 bAddDamageClass,mid,x;
		switch (sd->state.lr_flag) {
			case 0: //Right hand
				ARR_FIND(0, ARRAYLENGTH(sd->right_weapon.add_dmg), i, sd->right_weapon.add_dmg[i].rate == 0 || sd->right_weapon.add_dmg[i].class_ == type2);
				if (i == ARRAYLENGTH(sd->right_weapon.add_dmg))
				{
					ShowError("pc_bonus2: SP_ADD_DAMAGE_CLASS: Reached max (%d) number of add Class dmg bonuses per character!\n", ARRAYLENGTH(sd->right_weapon.add_dmg));
					break;
				}
				sd->right_weapon.add_dmg[i].class_ = type2;
				sd->right_weapon.add_dmg[i].rate += val;
				if (!sd->right_weapon.add_dmg[i].rate) //Shift the rest of elements up.
					memmove(&sd->right_weapon.add_dmg[i], &sd->right_weapon.add_dmg[i+1], sizeof(sd->right_weapon.add_dmg) - (i+1)*sizeof(sd->right_weapon.add_dmg[0]));
				break;
			case 1: //Left hand
				ARR_FIND(0, ARRAYLENGTH(sd->left_weapon.add_dmg), i, sd->left_weapon.add_dmg[i].rate == 0 || sd->left_weapon.add_dmg[i].class_ == type2);
				if (i == ARRAYLENGTH(sd->left_weapon.add_dmg))
				{
					ShowError("pc_bonus2: SP_ADD_DAMAGE_CLASS: Reached max (%d) number of add Class dmg bonuses per character!\n", ARRAYLENGTH(sd->left_weapon.add_dmg));
					break;
				}
				sd->left_weapon.add_dmg[i].class_ = type2;
				sd->left_weapon.add_dmg[i].rate += val;
				if (!sd->left_weapon.add_dmg[i].rate) //Shift the rest of elements up.
					memmove(&sd->left_weapon.add_dmg[i], &sd->left_weapon.add_dmg[i+1], sizeof(sd->left_weapon.add_dmg) - (i+1)*sizeof(sd->left_weapon.add_dmg[0]));
				break;
			}
		break;
	case SP_ADD_MAGIC_DAMAGE_CLASS: // bonus2 bAddMagicDamageClass,mid,x;
		if(sd->state.lr_flag == 2)
			break;
		ARR_FIND(0, ARRAYLENGTH(sd->add_mdmg), i, sd->add_mdmg[i].rate == 0 || sd->add_mdmg[i].class_ == type2);
		if (i == ARRAYLENGTH(sd->add_mdmg))
		{
			ShowError("pc_bonus2: SP_ADD_MAGIC_DAMAGE_CLASS: Reached max (%d) number of add Class magic dmg bonuses per character!\n", ARRAYLENGTH(sd->add_mdmg));
			break;
		}
		sd->add_mdmg[i].class_ = type2;
		sd->add_mdmg[i].rate += val;
		if (!sd->add_mdmg[i].rate) //Shift the rest of elements up.
			memmove(&sd->add_mdmg[i], &sd->add_mdmg[i+1], sizeof(sd->add_mdmg) - (i+1)*sizeof(sd->add_mdmg[0]));
		break;
	case SP_ADD_DEF_MONSTER: // bonus2 bAddDefMonster,mid,x;
		if(sd->state.lr_flag == 2)
			break;
		ARR_FIND(0, ARRAYLENGTH(sd->add_def), i, sd->add_def[i].rate == 0 || sd->add_def[i].class_ == type2);
		if (i == ARRAYLENGTH(sd->add_def))
		{
			ShowError("pc_bonus2: SP_ADD_DEF_MONSTER: Reached max (%d) number of add Class def bonuses per character!\n", ARRAYLENGTH(sd->add_def));
			break;
		}
		sd->add_def[i].class_ = type2;
		sd->add_def[i].rate += val;
		if (!sd->add_def[i].rate) //Shift the rest of elements up.
			memmove(&sd->add_def[i], &sd->add_def[i+1], sizeof(sd->add_def) - (i+1)*sizeof(sd->add_def[0]));
		break;
	case SP_ADD_MDEF_MONSTER: // bonus2 bAddMDefMonster,mid,x;
		if(sd->state.lr_flag == 2)
			break;
		ARR_FIND(0, ARRAYLENGTH(sd->add_mdef), i, sd->add_mdef[i].rate == 0 || sd->add_mdef[i].class_ == type2);
		if (i == ARRAYLENGTH(sd->add_mdef))
		{
			ShowError("pc_bonus2: SP_ADD_MDEF_MONSTER: Reached max (%d) number of add Class mdef bonuses per character!\n", ARRAYLENGTH(sd->add_mdef));
			break;
		}
		sd->add_mdef[i].class_ = type2;
		sd->add_mdef[i].rate += val;
		if (!sd->add_mdef[i].rate) //Shift the rest of elements up.
			memmove(&sd->add_mdef[i], &sd->add_mdef[i+1], sizeof(sd->add_mdef) - (i+1)*sizeof(sd->add_mdef[0]));
		break;
	case SP_HP_DRAIN_RATE: // bonus2 bHPDrainRate,x,n;
		if(!sd->state.lr_flag) {
			sd->right_weapon.hp_drain_rate.rate += type2;
			sd->right_weapon.hp_drain_rate.per += val;
		}
		else if(sd->state.lr_flag == 1) {
			sd->left_weapon.hp_drain_rate.rate += type2;
			sd->left_weapon.hp_drain_rate.per += val;
		}
		break;
	case SP_SP_DRAIN_RATE: // bonus2 bSPDrainRate,x,n;
		if(!sd->state.lr_flag) {
			sd->right_weapon.sp_drain_rate.rate += type2;
			sd->right_weapon.sp_drain_rate.per += val;
		}
		else if(sd->state.lr_flag == 1) {
			sd->left_weapon.sp_drain_rate.rate += type2;
			sd->left_weapon.sp_drain_rate.per += val;
		}
		break;
	case SP_SP_VANISH_RATE: // bonus2 bSPVanishRate,x,n;
		if(sd->state.lr_flag != 2) {
			sd->bonus.sp_vanish_rate += type2;
			sd->bonus.sp_vanish_per += val;
		}
		break;
	case SP_HP_VANISH_RATE: // bonus2 bHPVanishRate,x,n;
		if(sd->state.lr_flag != 2) {
			sd->bonus.hp_vanish_rate += type2;
			sd->bonus.hp_vanish_per += val;
		}
		break;
	case SP_GET_ZENY_NUM: // bonus2 bGetZenyNum,x,n;
		if(sd->state.lr_flag != 2 && sd->bonus.get_zeny_rate < val) {
			sd->bonus.get_zeny_rate = val;
			sd->bonus.get_zeny_num = type2;
		}
		break;
	case SP_ADD_GET_ZENY_NUM: // bonus2 bAddGetZenyNum,x,n;
		if(sd->state.lr_flag != 2) {
			sd->bonus.get_zeny_rate += val;
			sd->bonus.get_zeny_num += type2;
		}
		break;
	case SP_WEAPON_COMA_ELE: // bonus2 bWeaponComaEle,e,n;
		PC_BONUS_CHK_ELEMENT(type2,SP_WEAPON_COMA_ELE);
		if(sd->state.lr_flag == 2)
			break;
		sd->weapon_coma_ele[type2] += val;
		sd->special_state.bonus_coma = 1;
		break;
	case SP_WEAPON_COMA_RACE: // bonus2 bWeaponComaRace,r,n;
		PC_BONUS_CHK_RACE(type2,SP_WEAPON_COMA_RACE);
		if(sd->state.lr_flag == 2)
			break;
		sd->weapon_coma_race[type2] += val;
		sd->special_state.bonus_coma = 1;
		break;
	case SP_WEAPON_COMA_CLASS: // bonus2 bWeaponComaClass,c,n;
		PC_BONUS_CHK_CLASS(type2,SP_WEAPON_COMA_CLASS);
		if(sd->state.lr_flag == 2)
			break;
		sd->weapon_coma_class[type2] += val;
		sd->special_state.bonus_coma = 1;
		break;
	case SP_WEAPON_ATK: // bonus2 bWeaponAtk,w,n;
		if(sd->state.lr_flag != 2)
			sd->weapon_atk[type2]+=val;
		break;
	case SP_WEAPON_ATK_RATE: // bonus2 bWeaponAtkRate,w,n;
		if(sd->state.lr_flag != 2)
			sd->weapon_atk_rate[type2]+=val;
		break;
	case SP_CRITICAL_ADDRACE: // bonus2 bCriticalAddRace,r,n;
		PC_BONUS_CHK_RACE(type2,SP_CRITICAL_ADDRACE);
		if(sd->state.lr_flag != 2)
			sd->critaddrace[type2] += val*10;
		break;
	case SP_ADDEFF_WHENHIT: // bonus2 bAddEffWhenHit,eff,n;
		PC_BONUS_CHK_SC(type2,SP_ADDEFF_WHENHIT);
		if(sd->state.lr_flag != 2)
			pc_bonus_addeff(sd->addeff_atked, ARRAYLENGTH(sd->addeff_atked), (sc_type)type2, val, 0, 0, 0);
		break;
	case SP_SKILL_ATK: // bonus2 bSkillAtk,sk,n;
		if(sd->state.lr_flag == 2)
			break;
		ARR_FIND(0, ARRAYLENGTH(sd->skillatk), i, sd->skillatk[i].id == 0 || sd->skillatk[i].id == type2);
		if (i == ARRAYLENGTH(sd->skillatk))
		{	//Better mention this so the array length can be updated. [Skotlex]
			ShowError("pc_bonus2: SP_SKILL_ATK: Reached max (%d) number of skills per character, bonus skill %d (+%d%%) lost.\n", ARRAYLENGTH(sd->skillatk), type2, val);
			break;
		}
		if (sd->skillatk[i].id == type2)
			sd->skillatk[i].val += val;
		else {
			sd->skillatk[i].id = type2;
			sd->skillatk[i].val = val;
		}
		break;
	case SP_SKILL_HEAL: // bonus2 bSkillHeal,sk,n;
		if(sd->state.lr_flag == 2)
			break;
		ARR_FIND(0, ARRAYLENGTH(sd->skillheal), i, sd->skillheal[i].id == 0 || sd->skillheal[i].id == type2);
		if (i == ARRAYLENGTH(sd->skillheal))
		{ // Better mention this so the array length can be updated. [Skotlex]
			ShowError("pc_bonus2: SP_SKILL_HEAL: Reached max (%d) number of skills per character, bonus skill %d (+%d%%) lost.\n", ARRAYLENGTH(sd->skillheal), type2, val);
			break;
		}
		if (sd->skillheal[i].id == type2)
			sd->skillheal[i].val += val;
		else {
			sd->skillheal[i].id = type2;
			sd->skillheal[i].val = val;
		}
		break;
	case SP_SKILL_HEAL2: // bonus2 bSkillHeal2,sk,n;
		if(sd->state.lr_flag == 2)
			break;
		ARR_FIND(0, ARRAYLENGTH(sd->skillheal2), i, sd->skillheal2[i].id == 0 || sd->skillheal2[i].id == type2);
		if (i == ARRAYLENGTH(sd->skillheal2))
		{ // Better mention this so the array length can be updated. [Skotlex]
			ShowError("pc_bonus2: SP_SKILL_HEAL2: Reached max (%d) number of skills per character, bonus skill %d (+%d%%) lost.\n", ARRAYLENGTH(sd->skillheal2), type2, val);
			break;
		}
		if (sd->skillheal2[i].id == type2)
			sd->skillheal2[i].val += val;
		else {
			sd->skillheal2[i].id = type2;
			sd->skillheal2[i].val = val;
		}
		break;
	case SP_ADD_SKILL_BLOW: // bonus2 bAddSkillBlow,sk,n;
		if(sd->state.lr_flag == 2)
			break;
		ARR_FIND(0, ARRAYLENGTH(sd->skillblown), i, sd->skillblown[i].id == 0 || sd->skillblown[i].id == type2);
		if (i == ARRAYLENGTH(sd->skillblown))
		{	//Better mention this so the array length can be updated. [Skotlex]
			ShowError("pc_bonus2: SP_ADD_SKILL_BLOW: Reached max (%d) number of skills per character, bonus skill %d (%d) lost.\n", ARRAYLENGTH(sd->skillblown), type2, val);
			break;
		}
		if(sd->skillblown[i].id == type2)
			sd->skillblown[i].val += val;
		else {
			sd->skillblown[i].id = type2;
			sd->skillblown[i].val = val;
		}
		break;
	case SP_HP_LOSS_RATE: // bonus2 bHPLossRate,n,t;
		if(sd->state.lr_flag != 2) {
			sd->hp_loss.value = type2;
			sd->hp_loss.rate = val;
		}
		break;
	case SP_HP_REGEN_RATE: // bonus2 bHPRegenRate,n,t;
		if(sd->state.lr_flag != 2) {
			sd->hp_regen.value = type2;
			sd->hp_regen.rate = val;
		}
		break;
	case SP_ADDRACE2: // bonus2 bAddRace2,mr,x;
		PC_BONUS_CHK_RACE2(type2,SP_ADDRACE2);
		if(sd->state.lr_flag != 2)
			sd->right_weapon.addrace2[type2] += val;
		else
			sd->left_weapon.addrace2[type2] += val;
		break;
	case SP_SUBSIZE: // bonus2 bSubSize,s,x;
		PC_BONUS_CHK_SIZE(type2,SP_SUBSIZE);
		if(sd->state.lr_flag != 2)
			sd->subsize[type2]+=val;
		break;
	case SP_SUBRACE2: // bonus2 bSubRace2,mr,x;
		PC_BONUS_CHK_RACE2(type2,SP_SUBRACE2);
		if(sd->state.lr_flag != 2)
			sd->subrace2[type2]+=val;
		break;
	case SP_ADD_ITEM_HEAL_RATE: // bonus2 bAddItemHealRate,iid,n;
		if(sd->state.lr_flag == 2)
			break;
		if (!itemdb_exists(type2)) {
			ShowError("pc_bonus2: SP_ADD_ITEM_HEAL_RATE Invalid item with id %d\n", type2);
			break;
		}
		for(i=0; i < ARRAYLENGTH(sd->itemhealrate) && sd->itemhealrate[i].nameid && sd->itemhealrate[i].nameid != type2; i++);
		if(i == ARRAYLENGTH(sd->itemhealrate)) {
			ShowError("pc_bonus2: SP_ADD_ITEM_HEAL_RATE: Reached max (%d) number of item heal bonuses per character!\n", ARRAYLENGTH(sd->itemhealrate));
			break;
		}
		sd->itemhealrate[i].nameid = type2;
		sd->itemhealrate[i].rate += val;
		break;
	case SP_ADD_ITEMGROUP_HEAL_RATE: // bonus2 bAddItemGroupHealRate,ig,n;
		if (sd->state.lr_flag == 2)
			break;
		if (!type2 || !itemdb_group_exists(type2)) {
			ShowError("pc_bonus2: SP_ADD_ITEMGROUP_HEAL_RATE: Invalid item group with id %d\n", type2);
			break;
		}
		pc_itemgrouphealrate(sd, type2, val);
		break;
	case SP_EXP_ADDRACE: // bonus2 bExpAddRace,r,x;
		PC_BONUS_CHK_RACE(type2,SP_EXP_ADDRACE);
		if(sd->state.lr_flag != 2)
			sd->expaddrace[type2]+=val;
		break;
	case SP_EXP_ADDCLASS: // bonus2 bExpAddClass,c,x;
		PC_BONUS_CHK_CLASS(type2,SP_EXP_ADDCLASS);
		if(sd->state.lr_flag != 2)
			sd->expaddclass[type2]+=val;
		break;
	case SP_SP_GAIN_RACE: // bonus2 bSPGainRace,r,n;
		PC_BONUS_CHK_RACE(type2,SP_SP_GAIN_RACE);
		if(sd->state.lr_flag != 2)
			sd->sp_gain_race[type2]+=val;
		break;
	case SP_ADD_MONSTER_DROP_ITEM: // bonus2 bAddMonsterDropItem,iid,n;
		if (sd->state.lr_flag != 2)
			pc_bonus_item_drop(sd->add_drop, ARRAYLENGTH(sd->add_drop), type2, 0, CLASS_ALL, RC_NONE_, val);
		break;
	case SP_ADD_MONSTER_DROP_ITEMGROUP: // bonus2 bAddMonsterDropItemGroup,ig,n;
		if (sd->state.lr_flag != 2)
			pc_bonus_item_drop(sd->add_drop, ARRAYLENGTH(sd->add_drop), 0, type2, CLASS_ALL, RC_NONE_, val);
		break;
	case SP_SP_LOSS_RATE: // bonus2 bSPLossRate,n,t;
		if(sd->state.lr_flag != 2) {
			sd->sp_loss.value = type2;
			sd->sp_loss.rate = val;
		}
		break;
	case SP_SP_REGEN_RATE: // bonus2 bSPRegenRate,n,t;
		if(sd->state.lr_flag != 2) {
			sd->sp_regen.value = type2;
			sd->sp_regen.rate = val;
		}
		break;
	case SP_HP_DRAIN_VALUE_RACE: // bonus2 bHPDrainValueRace,r,n;
		PC_BONUS_CHK_RACE(type2,SP_HP_DRAIN_VALUE_RACE);
		if(!sd->state.lr_flag) {
			sd->right_weapon.hp_drain_race[type2] += val;
		}
		else if(sd->state.lr_flag == 1) {
			sd->left_weapon.hp_drain_race[type2] += val;
		}
		break;
	case SP_SP_DRAIN_VALUE_RACE: // bonus2 bSPDrainValueRace,r,n;
		PC_BONUS_CHK_RACE(type2,SP_SP_DRAIN_VALUE_RACE);
		if(!sd->state.lr_flag) {
			sd->right_weapon.sp_drain_race[type2] += val;
		}
		else if(sd->state.lr_flag == 1) {
			sd->left_weapon.sp_drain_race[type2] += val;
		}
		break;
	case SP_HP_DRAIN_VALUE_CLASS: // bonus2 bHPDrainValueClass,c,n;
		PC_BONUS_CHK_CLASS(type2,SP_HP_DRAIN_VALUE_CLASS);
		if(!sd->state.lr_flag) {
			sd->right_weapon.hp_drain_class[type2] += val;
		}
		else if(sd->state.lr_flag == 1) {
			sd->left_weapon.hp_drain_class[type2] += val;
		}
		break;
	case SP_SP_DRAIN_VALUE_CLASS: // bonus2 bSPDrainValueClass,c,n;
		PC_BONUS_CHK_CLASS(type2,SP_SP_DRAIN_VALUE_CLASS);
		if(!sd->state.lr_flag) {
			sd->right_weapon.sp_drain_class[type2] += val;
		}
		else if(sd->state.lr_flag == 1) {
			sd->left_weapon.sp_drain_class[type2] += val;
		}
		break;
	case SP_IGNORE_MDEF_RACE_RATE: // bonus2 bIgnoreMdefRaceRate,r,n;
		PC_BONUS_CHK_RACE(type2,SP_IGNORE_MDEF_RACE_RATE);
		if(sd->state.lr_flag != 2)
			sd->ignore_mdef_by_race[type2] += val;
		break;
	case SP_IGNORE_MDEF_CLASS_RATE: // bonus2 bIgnoreMdefClassRate,c,n;
		PC_BONUS_CHK_CLASS(type2,SP_IGNORE_MDEF_CLASS_RATE);
		if(sd->state.lr_flag != 2)
			sd->ignore_mdef_by_class[type2] += val;
		break;
	case SP_IGNORE_DEF_RACE_RATE: // bonus2 bIgnoreDefRaceRate,r,n;
		PC_BONUS_CHK_RACE(type2,SP_IGNORE_DEF_RACE_RATE);
		if(sd->state.lr_flag != 2)
			sd->ignore_def_by_race[type2] += val;
		break;
	case SP_SKILL_USE_SP_RATE: // bonus2 bSkillUseSPrate,sk,n;
		if(sd->state.lr_flag == 2)
			break;
		ARR_FIND(0, ARRAYLENGTH(sd->skillusesprate), i, sd->skillusesprate[i].id == 0 || sd->skillusesprate[i].id == type2);
		if (i == ARRAYLENGTH(sd->skillusesprate)) {
			ShowError("pc_bonus2: SP_SKILL_USE_SP_RATE: Reached max (%d) number of skills per character, bonus skill %d (+%d%%) lost.\n", ARRAYLENGTH(sd->skillusesprate), type2, val);
			break;
		}
		if (sd->skillusesprate[i].id == type2)
			sd->skillusesprate[i].val += val;
		else {
			sd->skillusesprate[i].id = type2;
			sd->skillusesprate[i].val = val;
		}
		break;
	case SP_SKILL_COOLDOWN: // bonus2 bSkillCooldown,sk,t;
		if(sd->state.lr_flag == 2)
			break;
		ARR_FIND(0, ARRAYLENGTH(sd->skillcooldown), i, sd->skillcooldown[i].id == 0 || sd->skillcooldown[i].id == type2);
		if (i == ARRAYLENGTH(sd->skillcooldown))
		{
			ShowError("pc_bonus2: SP_SKILL_COOLDOWN: Reached max (%d) number of skills per character, bonus skill %d (%d) lost.\n", ARRAYLENGTH(sd->skillcooldown), type2, val);
			break;
		}
		if (sd->skillcooldown[i].id == type2)
			sd->skillcooldown[i].val += val;
		else {
			sd->skillcooldown[i].id = type2;
			sd->skillcooldown[i].val = val;
		}
		break;
#ifdef RENEWAL_CAST
	case SP_SKILL_FIXEDCAST: // bonus2 bSkillFixedCast,sk,t;
		if(sd->state.lr_flag == 2)
			break;
		ARR_FIND(0, ARRAYLENGTH(sd->skillfixcast), i, sd->skillfixcast[i].id == 0 || sd->skillfixcast[i].id == type2);
		if (i == ARRAYLENGTH(sd->skillfixcast))
		{
			ShowError("pc_bonus2: SP_SKILL_FIXEDCAST: Reached max (%d) number of skills per character, bonus skill %d (%d) lost.\n", ARRAYLENGTH(sd->skillfixcast), type2, val);
			break;
		}
		if (sd->skillfixcast[i].id == type2)
			sd->skillfixcast[i].val += val;
		else {
			sd->skillfixcast[i].id = type2;
			sd->skillfixcast[i].val = val;
		}
		break;
	case SP_SKILL_VARIABLECAST: // bonus2 bSkillVariableCast,sk,t;
		if(sd->state.lr_flag == 2)
			break;
		ARR_FIND(0, ARRAYLENGTH(sd->skillvarcast), i, sd->skillvarcast[i].id == 0 || sd->skillvarcast[i].id == type2);
		if (i == ARRAYLENGTH(sd->skillvarcast))
		{
			ShowError("pc_bonus2: SP_SKILL_VARIABLECAST: Reached max (%d) number of skills per character, bonus skill %d (%d) lost.\n", ARRAYLENGTH(sd->skillvarcast), type2, val);
			break;
		}
		if (sd->skillvarcast[i].id == type2)
			sd->skillvarcast[i].val += val;
		else {
			sd->skillvarcast[i].id = type2;
			sd->skillvarcast[i].val = val;
		}
		break;
	case SP_CASTRATE: // bonus2 bCastrate,sk,n;
	case SP_VARCASTRATE: // bonus2 bVariableCastrate,sk,n;
		if(sd->state.lr_flag == 2)
			break;
		ARR_FIND(0, ARRAYLENGTH(sd->skillcastrate), i, sd->skillcastrate[i].id == 0 || sd->skillcastrate[i].id == type2);
		if (i == ARRAYLENGTH(sd->skillcastrate))
		{
			ShowError("pc_bonus2: SP_VARCASTRATE: Reached max (%d) number of skills per character, bonus skill %d (%d%%) lost.\n",ARRAYLENGTH(sd->skillcastrate), type2, val);
			break;
		}
		if(sd->skillcastrate[i].id == type2)
			sd->skillcastrate[i].val += val;
		else {
			sd->skillcastrate[i].id = type2;
			sd->skillcastrate[i].val += val;
		}
		break;
	case SP_FIXCASTRATE: // bonus2 bFixedCastrate,sk,n;
		if(sd->state.lr_flag == 2)
			break;
		ARR_FIND(0, ARRAYLENGTH(sd->skillfixcastrate), i, sd->skillfixcastrate[i].id == 0 || sd->skillfixcastrate[i].id == type2);
		if (i == ARRAYLENGTH(sd->skillfixcastrate))
		{
			ShowError("pc_bonus2: SP_FIXCASTRATE: Reached max (%d) number of skills per character, bonus skill %d (%d%%) lost.\n", ARRAYLENGTH(sd->skillfixcastrate), type2, val);
			break;
		}
		if(sd->skillfixcastrate[i].id == type2)
			FIXEDCASTRATE(sd->skillfixcastrate[i].val,val);
		else {
			sd->skillfixcastrate[i].id = type2;
			sd->skillfixcastrate[i].val = val;
		}
		break;
#else
	case SP_SKILL_FIXEDCAST: // bonus2 bSkillFixedCast,sk,t;
	case SP_SKILL_VARIABLECAST: // bonus2 bSkillVariableCast,sk,t;
	case SP_FIXCASTRATE: // bonus2 bFixedCastrate,sk,n;
		//ShowWarning("pc_bonus2: Non-RENEWAL_CAST doesn't support this bonus %d.\n", type);
		break;
	case SP_VARCASTRATE: // bonus2 bVariableCastrate,sk,n;
	case SP_CASTRATE: // bonus2 bCastrate,sk,n;
		if(sd->state.lr_flag == 2)
			break;
		ARR_FIND(0, ARRAYLENGTH(sd->skillcastrate), i, sd->skillcastrate[i].id == 0 || sd->skillcastrate[i].id == type2);
		if (i == ARRAYLENGTH(sd->skillcastrate))
		{	//Better mention this so the array length can be updated. [Skotlex]
			ShowError("pc_bonus2: %s: Reached max (%d) number of skills per character, bonus skill %d (%d%%) lost.\n",
				(type == SP_CASTRATE) ? "SP_CASTRATE" : "SP_VARCASTRATE", ARRAYLENGTH(sd->skillcastrate), type2, val);
			break;
		}
		if(sd->skillcastrate[i].id == type2)
			sd->skillcastrate[i].val += val;
		else {
			sd->skillcastrate[i].id = type2;
			sd->skillcastrate[i].val = val;
		}
		break;
#endif
	case SP_SKILL_USE_SP: // bonus2 bSkillUseSP,sk,n;
		if(sd->state.lr_flag == 2)
			break;
		ARR_FIND(0, ARRAYLENGTH(sd->skillusesp), i, sd->skillusesp[i].id == 0 || sd->skillusesp[i].id == type2);
		if (i == ARRAYLENGTH(sd->skillusesp)) {
			ShowError("pc_bonus2: SP_SKILL_USE_SP: Reached max (%d) number of skills per character, bonus skill %d (%d) lost.\n", ARRAYLENGTH(sd->skillusesp), type2, val);
			break;
		}
		if (sd->skillusesp[i].id == type2)
			sd->skillusesp[i].val += val;
		else {
			sd->skillusesp[i].id = type2;
			sd->skillusesp[i].val = val;
		}
		break;
	case SP_SUB_SKILL: // bonus2 bSubSkill,sk,n;
		ARR_FIND(0, ARRAYLENGTH(sd->subskill), i, sd->subskill[i].id == type2 || sd->subskill[i].id == 0);
		if (i == ARRAYLENGTH(sd->subskill)) {
			ShowError("pc_bonus2: SP_SUB_SKILL: Reached max (%d) number of skills per character, bonus skill %d (%d) lost.\n", ARRAYLENGTH(sd->subskill), type2, val);
			break;
		}
		if (sd->subskill[i].id == type2)
			sd->subskill[i].val += val;
		else {
			sd->subskill[i].id = type2;
			sd->subskill[i].val = val;
		}
		break;
	case SP_SUBDEF_ELE: // bonus2 bSubDefEle,e,x;
		PC_BONUS_CHK_ELEMENT(type2,SP_SUBDEF_ELE);
		sd->subdefele[type2] += val;
		break;
	case SP_COMA_CLASS: // bonus2 bComaClass,c,n;
		PC_BONUS_CHK_CLASS(type2,SP_COMA_CLASS);
		sd->coma_class[type2] += val;
		sd->special_state.bonus_coma = 1;
		break;
	case SP_COMA_RACE: // bonus2 bComaRace,r,n;
		PC_BONUS_CHK_RACE(type2,SP_COMA_RACE);
		sd->coma_race[type2] += val;
		sd->special_state.bonus_coma = 1;
		break;
	default:
		ShowWarning("pc_bonus2: unknown type %d %d %d!\n",type,type2,val);
		break;
	}
}

/**
* Gives item bonus to player for format: bonus3 bBonusName,type2,val;
* @param sd
* @param type Bonus type used by bBonusName
* @param type2
* @param val Value that usually for rate or fixed value
*/
void pc_bonus3(struct map_session_data *sd,int type,int type2,int type3,int val)
{
	nullpo_retv(sd);

	switch(type){
	case SP_ADD_MONSTER_DROP_ITEM: // bonus3 bAddMonsterDropItem,iid,r,n;
		if(sd->state.lr_flag != 2)
			pc_bonus_item_drop(sd->add_drop, ARRAYLENGTH(sd->add_drop), type2, 0, CLASS_NONE, type3, val);
		break;
	case SP_ADD_MONSTER_ID_DROP_ITEM: // bonus3 bAddMonsterIdDropItem,iid,mid,n;
		if(sd->state.lr_flag != 2)
			pc_bonus_item_drop(sd->add_drop, ARRAYLENGTH(sd->add_drop), type2, 0, CLASS_NONE, -type3, val);
		break;
	case SP_ADD_CLASS_DROP_ITEM: // bonus3 bAddClassDropItem,iid,c,n;
		if(sd->state.lr_flag != 2)
			pc_bonus_item_drop(sd->add_drop, ARRAYLENGTH(sd->add_drop), type2, 0, type3, RC_NONE_, val);
		break;
	case SP_AUTOSPELL: // bonus3 bAutoSpell,sk,y,n;
		if(sd->state.lr_flag != 2)
		{
			int target = skill_get_inf(type2); //Support or Self (non-auto-target) skills should pick self.
			target = target&INF_SUPPORT_SKILL || (target&INF_SELF_SKILL && !(skill_get_inf2(type2)&INF2_NO_TARGET_SELF));
			pc_bonus_autospell(sd->autospell, ARRAYLENGTH(sd->autospell),
				target?-type2:type2, type3, val, 0, current_equip_card_id);
		}
		break;
	case SP_AUTOSPELL_WHENHIT: // bonus3 bAutoSpellWhenHit,sk,y,n;
		if(sd->state.lr_flag != 2)
		{
			int target = skill_get_inf(type2); //Support or Self (non-auto-target) skills should pick self.
			target = target&INF_SUPPORT_SKILL || (target&INF_SELF_SKILL && !(skill_get_inf2(type2)&INF2_NO_TARGET_SELF));
			pc_bonus_autospell(sd->autospell2, ARRAYLENGTH(sd->autospell2),
				target?-type2:type2, type3, val, BF_NORMAL|BF_SKILL, current_equip_card_id);
		}
		break;
	case SP_ADD_MONSTER_DROP_ITEMGROUP: // bonus3 bAddMonsterDropItemGroup,ig,r,n;
		if (sd->state.lr_flag != 2)
			pc_bonus_item_drop(sd->add_drop, ARRAYLENGTH(sd->add_drop), 0, type2, CLASS_NONE, type3, val);
		break;
	case SP_ADD_CLASS_DROP_ITEMGROUP: // bonus3 bAddClassDropItemGroup,ig,c,n;
		if (sd->state.lr_flag != 2)
			pc_bonus_item_drop(sd->add_drop, ARRAYLENGTH(sd->add_drop), 0, type2, type3, RC_NONE_, val);
		break;

	case SP_ADDEFF: // bonus3 bAddEff,eff,n,y;
		PC_BONUS_CHK_SC(type2,SP_ADDEFF);
		pc_bonus_addeff(sd->addeff, ARRAYLENGTH(sd->addeff), (sc_type)type2,
			sd->state.lr_flag != 2 ? type3 : 0, sd->state.lr_flag == 2 ? type3 : 0, val, 0);
		break;

	case SP_ADDEFF_WHENHIT: // bonus3 bAddEffWhenHit,eff,n,y;
		PC_BONUS_CHK_SC(type2,SP_ADDEFF_WHENHIT);
		if(sd->state.lr_flag != 2)
			pc_bonus_addeff(sd->addeff_atked, ARRAYLENGTH(sd->addeff_atked), (sc_type)type2, type3, 0, val, 0);
		break;

	case SP_ADDEFF_ONSKILL: // bonus3 bAddEffOnSkill,sk,eff,n;
		PC_BONUS_CHK_SC(type2,SP_ADDEFF_ONSKILL);
		if( sd->state.lr_flag != 2 )
			pc_bonus_addeff_onskill(sd->addeff_onskill, ARRAYLENGTH(sd->addeff_onskill), (sc_type)type3, val, type2, ATF_TARGET, 0);
		break;

	case SP_ADDELE: // bonus3 bAddEle,e,x,bf;
		PC_BONUS_CHK_ELEMENT(type2,SP_ADDELE);
		if (sd->state.lr_flag != 2)
			pc_bonus_addele(sd, (unsigned char)type2, type3, val);
		break;

	case SP_SUBELE: // bonus3 bSubEle,e,x,bf;
		PC_BONUS_CHK_ELEMENT(type2,SP_SUBELE);
		if (sd->state.lr_flag != 2)
			pc_bonus_subele(sd, (unsigned char)type2, type3, val);
		break;
		
	case SP_SP_VANISH_RACE_RATE: // bonus3 bSPVanishRaceRate,r,x,n;
		PC_BONUS_CHK_RACE(type2,SP_SP_VANISH_RACE_RATE);
		if(sd->state.lr_flag != 2) {
			sd->sp_vanish_race[type2].rate += type3;
			sd->sp_vanish_race[type2].per += val;
		}
		break;

	case SP_HP_VANISH_RACE_RATE: // bonus3 bHPVanishRaceRate,r,x,n;
		PC_BONUS_CHK_RACE(type2,SP_HP_VANISH_RACE_RATE);
		if(sd->state.lr_flag != 2) {
			sd->hp_vanish_race[type2].rate += type3;
			sd->hp_vanish_race[type2].per += val;
		}
		break;

	default:
		ShowWarning("pc_bonus3: unknown type %d %d %d %d!\n",type,type2,type3,val);
		break;
	}
}

/**
* Gives item bonus to player for format: bonus4 bBonusName,type2,type3,val;
* @param sd
* @param type Bonus type used by bBonusName
* @param type2
* @param type3
* @param val Value that usually for rate or fixed value
*/
void pc_bonus4(struct map_session_data *sd,int type,int type2,int type3,int type4,int val)
{
	nullpo_retv(sd);

	switch(type){
	case SP_AUTOSPELL: // bonus4 bAutoSpell,sk,y,n,i;
		if(sd->state.lr_flag != 2)
			pc_bonus_autospell(sd->autospell, ARRAYLENGTH(sd->autospell), (val&1?type2:-type2), (val&2?-type3:type3), type4, 0, current_equip_card_id);
		break;

	case SP_AUTOSPELL_WHENHIT: // bonus4 bAutoSpellWhenHit,sk,y,n,i;
		if(sd->state.lr_flag != 2)
			pc_bonus_autospell(sd->autospell2, ARRAYLENGTH(sd->autospell2), (val&1?type2:-type2), (val&2?-type3:type3), type4, BF_NORMAL|BF_SKILL, current_equip_card_id);
		break;

	case SP_AUTOSPELL_ONSKILL: // bonus4 bAutoSpellOnSkill,sk,x,y,n;
		if(sd->state.lr_flag != 2)
		{
			int target = skill_get_inf(type2); //Support or Self (non-auto-target) skills should pick self.
			target = target&INF_SUPPORT_SKILL || (target&INF_SELF_SKILL && !(skill_get_inf2(type2)&INF2_NO_TARGET_SELF));

			pc_bonus_autospell_onskill(sd->autospell3, ARRAYLENGTH(sd->autospell3), type2, target?-type3:type3, type4, val, current_equip_card_id);
		}
		break;

	case SP_ADDEFF: // bonus4 bAddEff,eff,n,y,t;
		PC_BONUS_CHK_SC(type2,SP_ADDEFF);
		pc_bonus_addeff(sd->addeff, ARRAYLENGTH(sd->addeff), (sc_type)type2,
			sd->state.lr_flag != 2 ? type3 : 0, sd->state.lr_flag == 2 ? type3 : 0, type4, val);
		break;

	case SP_ADDEFF_WHENHIT: // bonus4 bAddEffWhenHit,eff,n,y,t;
		PC_BONUS_CHK_SC(type2,SP_ADDEFF_WHENHIT);
		if (sd->state.lr_flag != 2)
			pc_bonus_addeff(sd->addeff_atked, ARRAYLENGTH(sd->addeff_atked), (sc_type)type2, type3, 0, type4, val);
		break;

	case SP_ADDEFF_ONSKILL: // bonus4 bAddEffOnSkill,sk,eff,n,y;
		PC_BONUS_CHK_SC(type3,SP_ADDEFF_ONSKILL);
		if( sd->state.lr_flag != 2 )
			pc_bonus_addeff_onskill(sd->addeff_onskill, ARRAYLENGTH(sd->addeff_onskill), (sc_type)type3, type4, type2, val, 0);
		break;

	case SP_SET_DEF_RACE: // bonus4 bSetDefRace,r,n,t,y;
		PC_BONUS_CHK_RACE(type2,SP_SET_DEF_RACE);
		if(sd->state.lr_flag == 2)
			break;
		sd->def_set_race[type2].rate = type3;
		sd->def_set_race[type2].tick = type4;
		sd->def_set_race[type2].value = val;
		break;

	case SP_SET_MDEF_RACE: // bonus4 bSetMDefRace,r,n,t,y;
		PC_BONUS_CHK_RACE(type2,SP_SET_MDEF_RACE);
		if(sd->state.lr_flag == 2)
			break;
		sd->mdef_set_race[type2].rate = type3;
		sd->mdef_set_race[type2].tick = type4;
		sd->mdef_set_race[type2].value = val;
		break;

	default:
		ShowWarning("pc_bonus4: unknown type %d %d %d %d %d!\n",type,type2,type3,type4,val);
		break;
	}
}

/**
* Gives item bonus to player for format: bonus5 bBonusName,type2,type3,type4,val;
* @param sd
* @param type Bonus type used by bBonusName
* @param type2
* @param type3
* @param type4
* @param val Value that usually for rate or fixed value
*/
void pc_bonus5(struct map_session_data *sd,int type,int type2,int type3,int type4,int type5,int val)
{
	nullpo_retv(sd);

	switch(type){
	case SP_AUTOSPELL: // bonus5 bAutoSpell,sk,y,n,bf,i;
		if(sd->state.lr_flag != 2)
			pc_bonus_autospell(sd->autospell, ARRAYLENGTH(sd->autospell), (val&1?type2:-type2), (val&2?-type3:type3), type4, type5, current_equip_card_id);
		break;

	case SP_AUTOSPELL_WHENHIT: // bonus5 bAutoSpellWhenHit,sk,y,n,bf,i;
		if(sd->state.lr_flag != 2)
			pc_bonus_autospell(sd->autospell2, ARRAYLENGTH(sd->autospell2), (val&1?type2:-type2), (val&2?-type3:type3), type4, type5, current_equip_card_id);
		break;

	case SP_AUTOSPELL_ONSKILL: // bonus5 bAutoSpellOnSkill,sk,x,y,n,i;
		if(sd->state.lr_flag != 2)
			pc_bonus_autospell_onskill(sd->autospell3, ARRAYLENGTH(sd->autospell3), type2, (val&1?-type3:type3), (val&2?-type4:type4), type5, current_equip_card_id);
		break;
 
	case SP_ADDEFF_ONSKILL: // bonus5 bAddEffOnSkill,sk,eff,n,y,t;
		PC_BONUS_CHK_SC(type3,SP_ADDEFF_ONSKILL);
		if( sd->state.lr_flag != 2 )
			pc_bonus_addeff_onskill(sd->addeff_onskill, ARRAYLENGTH(sd->addeff_onskill), (sc_type)type3, type4, type2, type5, val);
		break;

	default:
		ShowWarning("pc_bonus5: unknown type %d %d %d %d %d %d!\n",type,type2,type3,type4,type5,val);
		break;
	}
}

/*==========================================
 *	Grants a player a given skill. Flag values are:
 *	0 - Grant permanent skill to be bound to skill tree
 *	1 - Grant an item skill (temporary)
 *	2 - Like 1, except the level granted can stack with previously learned level.
 *	4 - Like 0, except the skill will ignore skill tree (saves through job changes and resets).
 *------------------------------------------*/
bool pc_skill(TBL_PC* sd, uint16 skill_id, int level, enum e_addskill_type type) {
	uint16 idx = 0;
	nullpo_ret(sd);

	if (!skill_id || !(idx = skill_get_index(skill_id))) {
		ShowError("pc_skill: Skill with id %d does not exist in the skill database\n", skill_id);
		return false;
	}
	if (level > MAX_SKILL_LEVEL) {
		ShowError("pc_skill: Skill level %d too high. Max lv supported is %d\n", level, MAX_SKILL_LEVEL);
		return false;
	}
	if (type == ADDSKILL_TEMP_ADDLEVEL && sd->status.skill[idx].lv + level > MAX_SKILL_LEVEL) {
		ShowWarning("pc_skill: Skill level bonus %d too high. Max lv supported is %d. Curr lv is %d. Set to max level.\n", level, MAX_SKILL_LEVEL, sd->status.skill[idx].lv);
		level = MAX_SKILL_LEVEL - sd->status.skill[idx].lv;
	}

	switch (type) {
		case ADDSKILL_PERMANENT: //Set skill data overwriting whatever was there before.
			sd->status.skill[idx].id   = skill_id;
			sd->status.skill[idx].lv   = level;
			sd->status.skill[idx].flag = SKILL_FLAG_PERMANENT;
			if (level == 0) { //Remove skill.
				sd->status.skill[idx].id = 0;
				clif_deleteskill(sd,skill_id);
			} else
				clif_addskill(sd,skill_id);
			if (!skill_get_inf(skill_id)) //Only recalculate for passive skills.
				status_calc_pc(sd, SCO_NONE);
			break;

		case ADDSKILL_TEMP: //Item bonus skill.
			if (sd->status.skill[idx].id != 0) {
				if (sd->status.skill[idx].lv >= level)
					return true;
				if (sd->status.skill[idx].flag == SKILL_FLAG_PERMANENT) //Non-granted skill, store it's level.
					sd->status.skill[idx].flag = SKILL_FLAG_REPLACED_LV_0 + sd->status.skill[idx].lv;
			} else {
				sd->status.skill[idx].id   = skill_id;
				sd->status.skill[idx].flag = SKILL_FLAG_TEMPORARY;
			}
			sd->status.skill[idx].lv = level;
			break;

		case ADDSKILL_TEMP_ADDLEVEL: //Add skill bonus on top of what you had.
			if (sd->status.skill[idx].id != 0) {
				if (sd->status.skill[idx].flag == SKILL_FLAG_PERMANENT)
					sd->status.skill[idx].flag = SKILL_FLAG_REPLACED_LV_0 + sd->status.skill[idx].lv; // Store previous level.
			} else {
				sd->status.skill[idx].id   = skill_id;
				sd->status.skill[idx].flag = SKILL_FLAG_TEMPORARY; //Set that this is a bonus skill.
			}
			sd->status.skill[idx].lv += level;
			break;

		case ADDSKILL_PERMANENT_GRANTED: //Permanent granted skills ignore the skill tree
			sd->status.skill[idx].id   = skill_id;
			sd->status.skill[idx].lv   = level;
			sd->status.skill[idx].flag = SKILL_FLAG_PERM_GRANTED;
			if (level == 0) { //Remove skill.
				sd->status.skill[idx].id = 0;
				clif_deleteskill(sd,skill_id);
			} else
				clif_addskill(sd,skill_id);
			if (!skill_get_inf(skill_id)) //Only recalculate for passive skills.
				status_calc_pc(sd, SCO_NONE);
			break;

		default:
			return false;
	}
	return true;
}
/*==========================================
 * Append a card to an item ?
 *------------------------------------------*/
int pc_insert_card(struct map_session_data* sd, int idx_card, int idx_equip)
{
	int i;
	unsigned short nameid;

	nullpo_ret(sd);

	if( idx_equip < 0 || idx_equip >= MAX_INVENTORY || sd->inventory_data[idx_equip] == NULL )
		return 0; //Invalid item index.
	if( idx_card < 0 || idx_card >= MAX_INVENTORY || sd->inventory_data[idx_card] == NULL )
		return 0; //Invalid card index.
	if( sd->status.inventory[idx_equip].nameid <= 0 || sd->status.inventory[idx_equip].amount < 1 )
		return 0; // target item missing
	if( sd->status.inventory[idx_card].nameid <= 0 || sd->status.inventory[idx_card].amount < 1 )
		return 0; // target card missing
	if( sd->inventory_data[idx_equip]->type != IT_WEAPON && sd->inventory_data[idx_equip]->type != IT_ARMOR )
		return 0; // only weapons and armor are allowed
	if( sd->inventory_data[idx_card]->type != IT_CARD )
		return 0; // must be a card
	if( sd->status.inventory[idx_equip].identify == 0 )
		return 0; // target must be identified
	if( itemdb_isspecial(sd->status.inventory[idx_equip].card[0]) )
		return 0; // card slots reserved for other purposes
	if( (sd->inventory_data[idx_equip]->equip & sd->inventory_data[idx_card]->equip) == 0 )
		return 0; // card cannot be compounded on this item type
	if( sd->inventory_data[idx_equip]->type == IT_WEAPON && sd->inventory_data[idx_card]->equip == EQP_SHIELD )
		return 0; // attempted to place shield card on left-hand weapon.
	if( sd->status.inventory[idx_equip].equip != 0 )
		return 0; // item must be unequipped

	ARR_FIND( 0, sd->inventory_data[idx_equip]->slot, i, sd->status.inventory[idx_equip].card[i] == 0 );
	if( i == sd->inventory_data[idx_equip]->slot )
		return 0; // no free slots

	// remember the card id to insert
	nameid = sd->status.inventory[idx_card].nameid;

	if( pc_delitem(sd,idx_card,1,1,0,LOG_TYPE_OTHER) == 1 )
	{// failed
		clif_insert_card(sd,idx_equip,idx_card,1);
	}
	else
	{// success
		log_pick_pc(sd, LOG_TYPE_OTHER, -1, &sd->status.inventory[idx_equip]);
		sd->status.inventory[idx_equip].card[i] = nameid;
		log_pick_pc(sd, LOG_TYPE_OTHER,  1, &sd->status.inventory[idx_equip]);
		clif_insert_card(sd,idx_equip,idx_card,0);
	}

	return 0;
}

//
// Items
//

/*==========================================
 * Update buying value by skills
 *------------------------------------------*/
int pc_modifybuyvalue(struct map_session_data *sd,int orig_value)
{
	int skill,val = orig_value,rate1 = 0,rate2 = 0;
	if((skill=pc_checkskill(sd,MC_DISCOUNT))>0)	// merchant discount
		rate1 = 5+skill*2-((skill==10)? 1:0);
	if((skill=pc_checkskill(sd,RG_COMPULSION))>0)	 // rogue discount
		rate2 = 5+skill*4;
	if(rate1 < rate2) rate1 = rate2;
	if(rate1)
		val = (int)((double)orig_value*(double)(100-rate1)/100.);
	if(val < 0) val = 0;
	if(orig_value > 0 && val < 1) val = 1;

	return val;
}

/*==========================================
 * Update selling value by skills
 *------------------------------------------*/
int pc_modifysellvalue(struct map_session_data *sd,int orig_value)
{
	int skill,val = orig_value,rate = 0;
	if((skill=pc_checkskill(sd,MC_OVERCHARGE))>0)	//OverCharge
		rate = 5+skill*2-((skill==10)? 1:0);
	if(rate)
		val = (int)((double)orig_value*(double)(100+rate)/100.);
	if(val < 0) val = 0;
	if(orig_value > 0 && val < 1) val = 1;

	return val;
}

/*==========================================
 * Checking if we have enough place on inventory for new item
 * Make sure to take 30k as limit (for client I guess)
 * @param sd
 * @param nameid
 * @param amount
 * @return e_chkitem_result
 *------------------------------------------*/
char pc_checkadditem(struct map_session_data *sd, unsigned short nameid, int amount)
{
	int i;
	struct item_data* data;

	nullpo_ret(sd);

	if(amount > MAX_AMOUNT)
		return CHKADDITEM_OVERAMOUNT;

	data = itemdb_search(nameid);

	if(!itemdb_isstackable2(data))
		return CHKADDITEM_NEW;

	if( data->stack.inventory && amount > data->stack.amount )
		return CHKADDITEM_OVERAMOUNT;

	for(i=0;i<MAX_INVENTORY;i++){
		// FIXME: This does not consider the checked item's cards, thus could check a wrong slot for stackability.
		if(sd->status.inventory[i].nameid==nameid){
			if( amount > MAX_AMOUNT - sd->status.inventory[i].amount || ( data->stack.inventory && amount > data->stack.amount - sd->status.inventory[i].amount ) )
				return CHKADDITEM_OVERAMOUNT;
			return CHKADDITEM_EXIST;
		}
	}

	return CHKADDITEM_NEW;
}

/*==========================================
 * Return number of available place in inventory
 * Each non stackable item will reduce place by 1
 * @param sd
 * @return Number of empty slots
 *------------------------------------------*/
uint8 pc_inventoryblank(struct map_session_data *sd)
{
	uint8 i, b;

	nullpo_ret(sd);

	for(i = 0, b = 0; i < MAX_INVENTORY; i++){
		if(sd->status.inventory[i].nameid==0)
			b++;
	}

	return b;
}

/*==========================================
 * attempts to remove zeny from player (sd)
 * @param sd
 * @param zeny Zeny removed
 * @param type e_log_pick_type
 * @param tsd (just for log?)
 * @return 0 - Success, 1 - Failed
 *------------------------------------------*/
char pc_payzeny(struct map_session_data *sd,int zeny, enum e_log_pick_type type, struct map_session_data *tsd)
{
	nullpo_retr(2,sd);

	zeny = cap_value(zeny,-MAX_ZENY,MAX_ZENY); //prevent command UB
	if( zeny < 0 )
	{
		ShowError("pc_payzeny: Paying negative Zeny (zeny=%d, account_id=%d, char_id=%d).\n", zeny, sd->status.account_id, sd->status.char_id);
		return 1;
	}

	if( sd->status.zeny < zeny )
		return 1; //Not enough.

	sd->status.zeny -= zeny;
	clif_updatestatus(sd,SP_ZENY);

	if(!tsd) tsd = sd;
	log_zeny(sd, type, tsd, -zeny);
	if( zeny > 0 && sd->state.showzeny ) {
		char output[255];
		sprintf(output, "Removed %dz.", zeny);
		clif_disp_onlyself(sd,output,strlen(output));
	}

	return 0;
}

/** Makes player pays by using cash points
 * @param sd Player who pays
 * @param price How many point player has to pay
 * @param points
 * @param type e_log_pick_type
 * @return -2: Paying negative points, -1: Not enough points, otherwise is succes (cash+points)
 */
int pc_paycash(struct map_session_data *sd, int price, int points, e_log_pick_type type ){
	int cash;
	nullpo_retr(-1,sd);

	points = cap_value(points,-MAX_ZENY,MAX_ZENY); //prevent command UB
	if( price < 0 || points < 0 )
	{
		ShowError("pc_paycash: Paying negative points (price=%d, points=%d, account_id=%d, char_id=%d).\n", price, points, sd->status.account_id, sd->status.char_id);
		return -2;
	}

	if( points > price )
	{
		ShowWarning("pc_paycash: More kafra points provided than needed (price=%d, points=%d, account_id=%d, char_id=%d).\n", price, points, sd->status.account_id, sd->status.char_id);
		points = price;
	}

	cash = price-points;

	if( sd->cashPoints < cash || sd->kafraPoints < points )
	{
		ShowError("pc_paycash: Not enough points (cash=%d, kafra=%d) to cover the price (cash=%d, kafra=%d) (account_id=%d, char_id=%d).\n", sd->cashPoints, sd->kafraPoints, cash, points, sd->status.account_id, sd->status.char_id);
		return -1;
	}

	pc_setaccountreg(sd, add_str("#CASHPOINTS"), sd->cashPoints-cash);
	if( cash ){
		log_cash( sd, type, LOG_CASH_TYPE_CASH, -cash );
	}
	pc_setaccountreg(sd, add_str("#KAFRAPOINTS"), sd->kafraPoints-points);
	if( points ){
		log_cash( sd, type, LOG_CASH_TYPE_KAFRA, -points );
	}

	if( battle_config.cashshop_show_points )
	{
		char output[128];
		sprintf(output, msg_txt(sd,504), points, cash, sd->kafraPoints, sd->cashPoints);
		clif_disp_onlyself(sd, output, strlen(output));
	}
	return cash+points;
}

int pc_getcash( struct map_session_data *sd, int cash, int points, e_log_pick_type type ){
	char output[128];
	nullpo_retr(-1,sd);

	cash = cap_value(cash,-MAX_ZENY,MAX_ZENY); //prevent command UB
	points = cap_value(points,-MAX_ZENY,MAX_ZENY); //prevent command UB
	if( cash > 0 )
	{
		if( cash > MAX_ZENY-sd->cashPoints )
		{
			ShowWarning("pc_getcash: Cash point overflow (cash=%d, have cash=%d, account_id=%d, char_id=%d).\n", cash, sd->cashPoints, sd->status.account_id, sd->status.char_id);
			cash = MAX_ZENY-sd->cashPoints;
		}

		pc_setaccountreg(sd, add_str("#CASHPOINTS"), sd->cashPoints+cash);
		if( cash ){
			log_cash( sd, type, LOG_CASH_TYPE_CASH, cash );
		}

		if( battle_config.cashshop_show_points )
		{
			sprintf(output, msg_txt(sd,505), cash, sd->cashPoints);
			clif_disp_onlyself(sd, output, strlen(output));
		}
		return cash;
	}
	else if( cash < 0 )
	{
		ShowError("pc_getcash: Obtaining negative cash points (cash=%d, account_id=%d, char_id=%d).\n", cash, sd->status.account_id, sd->status.char_id);
		return -1;
	}

	if( points > 0 )
	{
		if( points > MAX_ZENY-sd->kafraPoints )
		{
			ShowWarning("pc_getcash: Kafra point overflow (points=%d, have points=%d, account_id=%d, char_id=%d).\n", points, sd->kafraPoints, sd->status.account_id, sd->status.char_id);
			points = MAX_ZENY-sd->kafraPoints;
		}

		pc_setaccountreg(sd, add_str("#KAFRAPOINTS"), sd->kafraPoints+points);
		if( points ){
			log_cash( sd, type, LOG_CASH_TYPE_KAFRA, points );
		}

		if( battle_config.cashshop_show_points )
		{
			sprintf(output, msg_txt(sd,506), points, sd->kafraPoints);
			clif_disp_onlyself(sd, output, strlen(output));
		}
		return points;
	}
	else if( points < 0 )
	{
		ShowError("pc_getcash: Obtaining negative kafra points (points=%d, account_id=%d, char_id=%d).\n", points, sd->status.account_id, sd->status.char_id);
		return -1;
	}
	return -2; //shouldn't happen but jsut in case
}

/*==========================================
 * Attempts to give zeny to player (sd)
 * tsd (optional) from who for log (if null take sd)
 *------------------------------------------*/
char pc_getzeny(struct map_session_data *sd,int zeny, enum e_log_pick_type type, struct map_session_data *tsd)
{
	nullpo_retr(-1,sd);

	zeny = cap_value(zeny,-MAX_ZENY,MAX_ZENY); //prevent command UB
	if( zeny < 0 )
	{
		ShowError("pc_getzeny: Obtaining negative Zeny (zeny=%d, account_id=%d, char_id=%d).\n", zeny, sd->status.account_id, sd->status.char_id);
		return 1;
	}

	if( zeny > MAX_ZENY - sd->status.zeny )
		zeny = MAX_ZENY - sd->status.zeny;

	sd->status.zeny += zeny;
	clif_updatestatus(sd,SP_ZENY);

	if(!tsd) tsd = sd;
	log_zeny(sd, type, tsd, zeny);
	if( zeny > 0 && sd->state.showzeny ) {
		char output[255];
		sprintf(output, "Gained %dz.", zeny);
		clif_disp_onlyself(sd,output,strlen(output));
	}

	return 0;
}

/**
 * Searching a specified itemid in inventory and return his stored index
 * @param sd Player
 * @param nameid Find this Item!
 * @return Stored index in inventory, or -1 if not found.
 **/
short pc_search_inventory(struct map_session_data *sd, unsigned short nameid) {
	short i;
	nullpo_retr(-1, sd);

	ARR_FIND( 0, MAX_INVENTORY, i, sd->status.inventory[i].nameid == nameid && (sd->status.inventory[i].amount > 0 || nameid == 0) );
	return ( i < MAX_INVENTORY ) ? i : -1;
}

/** Attempt to add a new item to player inventory
 * @param sd
 * @param item_data
 * @param amount
 * @param log_type
 * @return
 *   0 = success
 *   1 = invalid itemid not found or negative amount
 *   2 = overweight
 *   3 = ?
 *   4 = no free place found
 *   5 = max amount reached
 *   6 = ?
 *   7 = stack limitation
 */
char pc_additem(struct map_session_data *sd,struct item *item,int amount,e_log_pick_type log_type) {
	struct item_data *id;
	int16 i;
	unsigned int w;

	nullpo_retr(1, sd);
	nullpo_retr(1, item);

	if( item->nameid == 0 || amount <= 0 )
		return ADDITEM_INVALID;
	if( amount > MAX_AMOUNT )
		return ADDITEM_OVERAMOUNT;

	id = itemdb_search(item->nameid);

	if( id->stack.inventory && amount > id->stack.amount )
	{// item stack limitation
		return ADDITEM_STACKLIMIT;
	}

	w = id->weight*amount;
	if(sd->weight + w > sd->max_weight)
		return ADDITEM_OVERWEIGHT;

	i = MAX_INVENTORY;

	if (id->flag.guid && !item->unique_id)
		item->unique_id = pc_generate_unique_id(sd);

	// Stackable | Non Rental
	if( itemdb_isstackable2(id) && item->expire_time == 0 ) {
		for( i = 0; i < MAX_INVENTORY; i++ ) {
			if( sd->status.inventory[i].nameid == item->nameid &&
				sd->status.inventory[i].bound == item->bound &&
				sd->status.inventory[i].expire_time == 0 &&
				sd->status.inventory[i].unique_id == item->unique_id &&
				memcmp(&sd->status.inventory[i].card, &item->card, sizeof(item->card)) == 0
				)
			{
				if( amount > MAX_AMOUNT - sd->status.inventory[i].amount || ( id->stack.inventory && amount > id->stack.amount - sd->status.inventory[i].amount ) )
					return ADDITEM_OVERAMOUNT;
				sd->status.inventory[i].amount += amount;
				clif_additem(sd,i,amount,0);
				break;
			}
		}
	}

	if (i >= MAX_INVENTORY) {
		i = pc_search_inventory(sd,0);
		if( i < 0 )
			return ADDITEM_OVERITEM;

		memcpy(&sd->status.inventory[i], item, sizeof(sd->status.inventory[0]));
		// clear equip and favorite fields first, just in case
		if( item->equip )
			sd->status.inventory[i].equip = 0;
		if( item->favorite )
			sd->status.inventory[i].favorite = 0;

		sd->status.inventory[i].amount = amount;
		sd->inventory_data[i] = id;
		sd->last_addeditem_index = i;

		if (!itemdb_isstackable2(id) || id->flag.guid)
			sd->status.inventory[i].unique_id = item->unique_id ? item->unique_id : pc_generate_unique_id(sd);

		clif_additem(sd,i,amount,0);
	}

	log_pick_pc(sd, log_type, amount, &sd->status.inventory[i]);

	sd->weight += w;
	clif_updatestatus(sd,SP_WEIGHT);
	//Auto-equip
	if(id->flag.autoequip)
		pc_equipitem(sd, i, id->equip);

	/* rental item check */
	if( item->expire_time ) {
		if( time(NULL) > item->expire_time ) {
			clif_rental_expired(sd->fd, i, sd->status.inventory[i].nameid);
			pc_delitem(sd, i, sd->status.inventory[i].amount, 1, 0, LOG_TYPE_OTHER);
		} else {
			int seconds = (int)( item->expire_time - time(NULL) );
			clif_rental_time(sd->fd, sd->status.inventory[i].nameid, seconds);
			pc_inventory_rental_add(sd, seconds);
		}
	}

	return ADDITEM_SUCCESS;
}

/*==========================================
 * Remove an item at index n from inventory by amount.
 * @param sd
 * @param n Item index in inventory
 * @param amount
 * @param type &1: Don't notify deletion; &2 Don't notify weight change
 * @param reason Delete reason
 * @param log_type e_log_pick_type
 * @return 1 - invalid itemid or negative amount; 0 - Success
 *------------------------------------------*/
char pc_delitem(struct map_session_data *sd,int n,int amount,int type, short reason, e_log_pick_type log_type)
{
	nullpo_retr(1, sd);

	if(n < 0 || sd->status.inventory[n].nameid==0 || amount <= 0 || sd->status.inventory[n].amount<amount || sd->inventory_data[n] == NULL)
		return 1;

	log_pick_pc(sd, log_type, -amount, &sd->status.inventory[n]);

	sd->status.inventory[n].amount -= amount;
	sd->weight -= sd->inventory_data[n]->weight*amount ;
	if( sd->status.inventory[n].amount <= 0 ){
		if(sd->status.inventory[n].equip)
			pc_unequipitem(sd,n,3);
		memset(&sd->status.inventory[n],0,sizeof(sd->status.inventory[0]));
		sd->inventory_data[n] = NULL;
	}
	if(!(type&1))
		clif_delitem(sd,n,amount,reason);
	if(!(type&2))
		clif_updatestatus(sd,SP_WEIGHT);

	return 0;
}

/*==========================================
 * Attempt to drop an item.
 * @param sd
 * @param n Item index in inventory
 * @param amount Amount of item
 * @return False = fail; True = success
 *------------------------------------------*/
bool pc_dropitem(struct map_session_data *sd,int n,int amount)
{
	nullpo_retr(1, sd);

	if(n < 0 || n >= MAX_INVENTORY)
		return false;

	if(amount <= 0)
		return false;

	if(sd->status.inventory[n].nameid <= 0 ||
		sd->status.inventory[n].amount <= 0 ||
		sd->status.inventory[n].amount < amount ||
		sd->state.trading || sd->state.vending ||
		!sd->inventory_data[n] //pc_delitem would fail on this case.
		)
		return false;

	if( map[sd->bl.m].flag.nodrop )
	{
		clif_displaymessage (sd->fd, msg_txt(sd,271));
		return false; //Can't drop items in nodrop mapflag maps.
	}

	if( !pc_candrop(sd,&sd->status.inventory[n]) )
	{
		clif_displaymessage (sd->fd, msg_txt(sd,263));
		return false;
	}

	if (!map_addflooritem(&sd->status.inventory[n], amount, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 2, 0))
		return false;

	pc_delitem(sd, n, amount, 1, 0, LOG_TYPE_PICKDROP_PLAYER);
	clif_dropitem(sd, n, amount);
	return true;
}

/*==========================================
 * Attempt to pick up an item.
 * @param sd
 * @param fitem Item that will be picked
 * @return False = fail; True = success
 *------------------------------------------*/
bool pc_takeitem(struct map_session_data *sd,struct flooritem_data *fitem)
{
	int flag = 0;
	unsigned int tick = gettick();
	struct party_data *p = NULL;

	nullpo_ret(sd);
	nullpo_ret(fitem);

	if (!check_distance_bl(&fitem->bl, &sd->bl, 2) && sd->ud.skill_id!=BS_GREED)
		return false;	// Distance is too far

	if (sd->sc.cant.pickup)
		return false;

	if (sd->status.party_id)
		p = party_search(sd->status.party_id);

	if (fitem->first_get_charid > 0 && fitem->first_get_charid != sd->status.char_id) {
		struct map_session_data *first_sd = map_charid2sd(fitem->first_get_charid);
		if (DIFF_TICK(tick,fitem->first_get_tick) < 0) {
			if (!(p && p->party.item&1 &&
				first_sd && first_sd->status.party_id == sd->status.party_id
				))
				return false;
		}
		else if (fitem->second_get_charid > 0 && fitem->second_get_charid != sd->status.char_id) {
			struct map_session_data *second_sd = map_charid2sd(fitem->second_get_charid);
			if (DIFF_TICK(tick, fitem->second_get_tick) < 0) {
				if (!(p && p->party.item&1 &&
					((first_sd && first_sd->status.party_id == sd->status.party_id) ||
					(second_sd && second_sd->status.party_id == sd->status.party_id))
					))
					return false;
			}
			else if (fitem->third_get_charid > 0 && fitem->third_get_charid != sd->status.char_id){
				struct map_session_data *third_sd = map_charid2sd(fitem->third_get_charid);
				if (DIFF_TICK(tick,fitem->third_get_tick) < 0) {
					if(!(p && p->party.item&1 &&
						((first_sd && first_sd->status.party_id == sd->status.party_id) ||
						(second_sd && second_sd->status.party_id == sd->status.party_id) ||
						(third_sd && third_sd->status.party_id == sd->status.party_id))
						))
						return false;
				}
			}
		}
	}

	//This function takes care of giving the item to whoever should have it, considering party-share options.
	if ((flag = party_share_loot(p,sd,&fitem->item, fitem->first_get_charid))) {
		clif_additem(sd,0,0,flag);
		return true;
	}

	//Display pickup animation.
	pc_stop_attack(sd);
	clif_takeitem(&sd->bl,&fitem->bl);

	if (fitem->mob_id &&
		(itemdb_search(fitem->item.nameid))->flag.broadcast &&
		(!p || !(p->party.item&2)) // Somehow, if party's pickup distribution is 'Even Share', no announcemet
		)
		intif_broadcast_obtain_special_item(sd, fitem->item.nameid, fitem->mob_id, ITEMOBTAIN_TYPE_MONSTER_ITEM);

	map_clearflooritem(&fitem->bl);
	return true;
}

/*==========================================
 * Check if item is usable.
 * Return:
 *	0 = no
 *	1 = yes
 *------------------------------------------*/
bool pc_isUseitem(struct map_session_data *sd,int n)
{
	struct item_data *item;
	unsigned short nameid;

	nullpo_ret(sd);

	item = sd->inventory_data[n];
	nameid = sd->status.inventory[n].nameid;

	if( item == NULL )
		return false;
	//Not consumable item
	if( item->type != IT_HEALING && item->type != IT_USABLE && item->type != IT_CASH )
		return false;
	if( !item->script ) //if it has no script, you can't really consume it!
		return false;
	if (pc_has_permission(sd,PC_PERM_ITEM_UNCONDITIONAL))
		return true;
	if(map[sd->bl.m].flag.noitemconsumption) //consumable but mapflag prevent it
		return false;
	//Prevent mass item usage. [Skotlex]
	if( DIFF_TICK(sd->canuseitem_tick,gettick()) > 0 ||
		(itemdb_iscashfood(nameid) && DIFF_TICK(sd->canusecashfood_tick,gettick()) > 0)
	)
		return false;

	if( (item->item_usage.flag&NOUSE_SITTING) && (pc_issit(sd) == 1) && (pc_get_group_level(sd) < item->item_usage.override) ) {
		clif_msg(sd,ITEM_NOUSE_SITTING);
		return false; // You cannot use this item while sitting.
	}

	if (sd->state.storage_flag && item->type != IT_CASH) {
		clif_colormes(sd->fd, color_table[COLOR_RED], msg_txt(sd,388));
		return false; // You cannot use this item while storage is open.
	}

	if (item->flag.dead_branch && (map[sd->bl.m].flag.nobranch || map_flag_gvg(sd->bl.m)))
		return false;

	switch( nameid ) {
		case ITEMID_ANODYNE:
			if( map_flag_gvg(sd->bl.m) )
				return false;
		case ITEMID_ALOEBERA:
			if( pc_issit(sd) )
				return false;
			break;
		case ITEMID_WING_OF_FLY:
		case ITEMID_GIANT_FLY_WING:
			if( map[sd->bl.m].flag.noteleport || map_flag_gvg(sd->bl.m) ) {
				clif_skill_teleportmessage(sd,0);
				return false;
			}
		case ITEMID_WING_OF_BUTTERFLY:
		case ITEMID_DUN_TELE_SCROLL1:
		case ITEMID_DUN_TELE_SCROLL2:
		case ITEMID_DUN_TELE_SCROLL3:
		case ITEMID_WOB_RUNE:
		case ITEMID_WOB_SCHWALTZ:
		case ITEMID_WOB_RACHEL:
		case ITEMID_WOB_LOCAL:
		case ITEMID_SIEGE_TELEPORT_SCROLL:
			if( sd->duel_group && !battle_config.duel_allow_teleport ) {
				clif_displaymessage(sd->fd, msg_txt(sd,663));
				return false;
			}
			if( nameid != 601 && nameid != 12212 && map[sd->bl.m].flag.noreturn )
				return false;
			break;
		case ITEMID_BUBBLE_GUM:
		case ITEMID_COMP_BUBBLE_GUM:
			if( sd->sc.data[SC_ITEMBOOST] )
				return false;
			break;
		case ITEMID_BATTLE_MANUAL:
		case ITEMID_COMP_BATTLE_MANUAL:
		case ITEMID_THICK_BATTLE_MANUAL:
		case ITEMID_NOBLE_NAMEPLATE:
		case ITEMID_BATTLE_MANUAL25:
		case ITEMID_BATTLE_MANUAL100:
		case ITEMID_BATTLE_MANUAL300:
			if( sd->sc.data[SC_EXPBOOST] )
				return false;
			break;
		case ITEMID_JOB_MANUAL50:
			if( sd->sc.data[SC_JEXPBOOST] )
				return false;
			break;
		case ITEMID_MERCENARY_RED_POTION:
		case ITEMID_MERCENARY_BLUE_POTION:
		case ITEMID_M_CENTER_POTION:
		case ITEMID_M_AWAKENING_POTION:
		case ITEMID_M_BERSERK_POTION:
			if( sd->md == NULL || sd->md->db == NULL )
				return false;
			if( sd->md->sc.data[SC_BERSERK] )
				return false;
			if( nameid == ITEMID_M_AWAKENING_POTION && sd->md->db->lv < 40 )
				return false;
			if( nameid == ITEMID_M_BERSERK_POTION && sd->md->db->lv < 80 )
				return false;
			break;

		case ITEMID_NEURALIZER:
			if( !map[sd->bl.m].flag.reset )
				return false;
			break;
	}

	if( nameid >= ITEMID_BOW_MERCENARY_SCROLL1 && nameid <= ITEMID_SPEARMERCENARY_SCROLL10 && sd->md != NULL )
		return false; // Mercenary Scrolls

	/**
	 * Only Rune Knights may use runes
	 **/
	if( itemdb_is_rune(nameid) && (sd->class_&MAPID_THIRDMASK) != MAPID_RUNE_KNIGHT )
		return false;
	/**
	 * Only GCross may use poisons
	 **/
	else if( itemdb_is_poison(nameid) && (sd->class_&MAPID_THIRDMASK) != MAPID_GUILLOTINE_CROSS )
		return false;

	if( item->flag.group || item->type == IT_CASH) {	//safe check type cash disappear when overweight [Napster]
		if( pc_is90overweight(sd) ) {
			clif_msg(sd, ITEM_CANT_OBTAIN_WEIGHT);
			return false;
		}
		if( !pc_inventoryblank(sd) ) {
			clif_colormes(sd->fd, color_table[COLOR_RED], msg_txt(sd, 732)); //Item cannot be open when inventory is full
			return false;
		}
	}

	//Gender check
	if(item->sex != 2 && sd->status.sex != item->sex)
		return false;
	//Required level check
	if(item->elv && sd->status.base_level < (unsigned int)item->elv)
		return false;
	if(item->elvmax && sd->status.base_level > (unsigned int)item->elvmax)
		return false;

	//Not equipable by class. [Skotlex]
	if (!(
		(1<<(sd->class_&MAPID_BASEMASK)) &
		(item->class_base[sd->class_&JOBL_2_1?1:(sd->class_&JOBL_2_2?2:0)])
	))
		return false;
	
	if (sd->sc.count && (
		sd->sc.data[SC_BERSERK] || sd->sc.data[SC_SATURDAYNIGHTFEVER] ||
		(sd->sc.data[SC_GRAVITATION] && sd->sc.data[SC_GRAVITATION]->val3 == BCT_SELF) ||
		sd->sc.data[SC_TRICKDEAD] ||
		sd->sc.data[SC_HIDING] ||
		sd->sc.data[SC__SHADOWFORM] ||
		sd->sc.data[SC__INVISIBILITY] ||
		sd->sc.data[SC__MANHOLE] ||
		sd->sc.data[SC_KAGEHUMI] ||
		(sd->sc.data[SC_NOCHAT] && sd->sc.data[SC_NOCHAT]->val1&MANNER_NOITEM) ||
		sd->sc.data[SC_HEAT_BARREL_AFTER]))
		return false;
	
	if (!pc_isItemClass(sd,item))
		return false;

	//Dead Branch items
	if( item->flag.dead_branch )
		log_branch(sd);

	return true;
}

/*==========================================
 * Last checks to use an item.
 * Return:
 *	0 = fail
 *	1 = success
 *------------------------------------------*/
int pc_useitem(struct map_session_data *sd,int n)
{
	unsigned int tick = gettick();
	int amount;
	unsigned short nameid;
	struct script_code *script;
	struct item item;
	struct item_data *id;

	nullpo_ret(sd);

	if (sd->npc_id) {
#ifdef RENEWAL
		clif_msg(sd, USAGE_FAIL); // TODO look for the client date that has this message.
		return 0;
#else
		if( !sd->npc_item_flag )
			return 0;
#endif
	}
	item = sd->status.inventory[n];
	id = sd->inventory_data[n];

	if (item.nameid == 0 || item.amount <= 0)
		return 0;

	if( !pc_isUseitem(sd,n) )
		return 0;

	// Store information for later use before it is lost (via pc_delitem) [Paradox924X]
	nameid = id->nameid;

	if (nameid != ITEMID_NAUTHIZ && sd->sc.opt1 > 0 && sd->sc.opt1 != OPT1_STONEWAIT && sd->sc.opt1 != OPT1_BURNING)
		return 0;

	/* Items with delayed consume are not meant to work while in mounts except reins of mount(12622) */
	if( id->flag.delay_consume ) {
		if( nameid != ITEMID_REINS_OF_MOUNT && &sd->sc && sd->sc.data[SC_ALL_RIDING] )
			return 0;
		else if( pc_issit(sd) )
			return 0;
	}
	//Since most delay-consume items involve using a "skill-type" target cursor,
	//perform a skill-use check before going through. [Skotlex]
	//resurrection was picked as testing skill, as a non-offensive, generic skill, it will do.
	//FIXME: Is this really needed here? It'll be checked in unit.c after all and this prevents skill items using when silenced [Inkfish]
	if( id->flag.delay_consume && ( sd->ud.skilltimer != INVALID_TIMER /*|| !status_check_skilluse(&sd->bl, &sd->bl, ALL_RESURRECTION, 0)*/ ) )
		return 0;

	if( id->delay > 0 && !pc_has_permission(sd,PC_PERM_ITEM_UNCONDITIONAL) && pc_itemcd_check(sd, id, tick, n))
		return 0;

	/* on restricted maps the item is consumed but the effect is not used */
	if (!pc_has_permission(sd,PC_PERM_ITEM_UNCONDITIONAL) && itemdb_isNoEquip(id,sd->bl.m)) {
		clif_msg(sd,ITEM_CANT_USE_AREA); // This item cannot be used within this area
		if( battle_config.allow_consume_restricted_item && !id->flag.delay_consume ) { //need confirmation for delayed consumption items
			clif_useitemack(sd,n,item.amount-1,true);
			pc_delitem(sd,n,1,1,0,LOG_TYPE_CONSUME);
		}
		return 0;/* regardless, effect is not run */
	}

	sd->itemid = item.nameid;
	sd->itemindex = n;
	if(sd->catch_target_class != -1) //Abort pet catching.
		sd->catch_target_class = -1;

	amount = item.amount;
	script = id->script;
	//Check if the item is to be consumed immediately [Skotlex]
	if (id->flag.delay_consume)
		clif_useitemack(sd, n, amount, true);
	else
	{
		if( item.expire_time == 0 && nameid != ITEMID_REINS_OF_MOUNT )
		{
			clif_useitemack(sd, n, amount - 1, true);
			pc_delitem(sd, n, 1, 1, 0, LOG_TYPE_CONSUME); // Rental Usable Items are not deleted until expiration
		}
		else
			clif_useitemack(sd, n, 0, false);
	}
	if(item.card[0]==CARD0_CREATE &&
		pc_famerank(MakeDWord(item.card[2],item.card[3]), MAPID_ALCHEMIST))
	{
	    potion_flag = 2; // Famous player's potions have 50% more efficiency
		 if (sd->sc.data[SC_SPIRIT] && sd->sc.data[SC_SPIRIT]->val2 == SL_ROGUE)
			 potion_flag = 3; //Even more effective potions.
	}

	//Update item use time.
	sd->canuseitem_tick = tick + battle_config.item_use_interval;
	if( itemdb_iscashfood(nameid) )
		sd->canusecashfood_tick = tick + battle_config.cashfood_use_interval;

	run_script(script,0,sd->bl.id,fake_nd->bl.id);
	potion_flag = 0;
	return 1;
}

/**
 * Add item on cart for given index.
 * @param sd
 * @param item
 * @param amount
 * @param log_type
 * @return 0 = success; 1 = fail; 2 = no slot
 */
unsigned char pc_cart_additem(struct map_session_data *sd,struct item *item,int amount,e_log_pick_type log_type)
{
	struct item_data *data;
	int i,w;

	nullpo_retr(1, sd);
	nullpo_retr(1, item);

	if(item->nameid == 0 || amount <= 0)
		return 1;
	data = itemdb_search(item->nameid);

	if( data->stack.cart && amount > data->stack.amount )
	{// item stack limitation
		return 1;
	}

	if( !itemdb_cancartstore(item, pc_get_group_level(sd)) || (item->bound > BOUND_ACCOUNT && !pc_can_give_bounded_items(sd)))
	{ // Check item trade restrictions	[Skotlex]
		clif_displaymessage (sd->fd, msg_txt(sd,264));
		return 1;
	}

	if( (w = data->weight*amount) + sd->cart_weight > sd->cart_weight_max )
		return 1;

	// ID no longer points to inventory/kafra ID. While we get a new one we don't want to mess up vending creation.
	item->id = 0;

	i = MAX_CART;
	if( itemdb_isstackable2(data) && !item->expire_time )
	{
		for (i = 0; i < MAX_CART; i++) {
			if (sd->status.cart[i].nameid == item->nameid
				&& sd->status.cart[i].bound == item->bound
				&& sd->status.cart[i].unique_id == item->unique_id
				&& memcmp(sd->status.cart[i].card, item->card, sizeof(item->card)) == 0
				)
				break;
		}
	}

	if( i < MAX_CART )
	{// item already in cart, stack it
		if( amount > MAX_AMOUNT - sd->status.cart[i].amount || ( data->stack.cart && amount > data->stack.amount - sd->status.cart[i].amount ) )
			return 2; // no slot

		sd->status.cart[i].amount+=amount;
		clif_cart_additem(sd,i,amount,0);
	}
	else
	{// item not stackable or not present, add it
		ARR_FIND( 0, MAX_CART, i, sd->status.cart[i].nameid == 0 );
		if( i == MAX_CART )
			return 2; // no slot

		memcpy(&sd->status.cart[i],item,sizeof(sd->status.cart[0]));
		sd->status.cart[i].amount=amount;
		sd->cart_num++;
		clif_cart_additem(sd,i,amount,0);
	}
	sd->status.cart[i].favorite = 0;/* clear */
	log_pick_pc(sd, log_type, amount, &sd->status.cart[i]);

	sd->cart_weight += w;
	clif_updatestatus(sd,SP_CARTINFO);

	return 0;
}

/*==========================================
 * Delete item on cart for given index.
 *------------------------------------------*/
void pc_cart_delitem(struct map_session_data *sd,int n,int amount,int type,e_log_pick_type log_type)
{
	nullpo_retv(sd);

	if(sd->status.cart[n].nameid == 0 ||
	   sd->status.cart[n].amount<amount)
		return;

	log_pick_pc(sd, log_type, -amount, &sd->status.cart[n]);

	sd->status.cart[n].amount -= amount;
	sd->cart_weight -= itemdb_weight(sd->status.cart[n].nameid)*amount ;
	if(sd->status.cart[n].amount <= 0){
		memset(&sd->status.cart[n],0,sizeof(sd->status.cart[0]));
		sd->cart_num--;
	}
	if(!type) {
		clif_cart_delitem(sd,n,amount);
		clif_updatestatus(sd,SP_CARTINFO);
	}
}

/*==========================================
 * Transfer item from inventory to cart.
 *------------------------------------------*/
void pc_putitemtocart(struct map_session_data *sd,int idx,int amount)
{
	struct item *item_data;
	char flag;

	nullpo_retv(sd);

	if (idx < 0 || idx >= MAX_INVENTORY) //Invalid index check [Skotlex]
		return;

	item_data = &sd->status.inventory[idx];

	if( item_data->nameid == 0 || amount < 1 || item_data->amount < amount || sd->state.vending )
		return;

	if( (flag = pc_cart_additem(sd,item_data,amount,LOG_TYPE_NONE)) == 0 )
		pc_delitem(sd,idx,amount,0,5,LOG_TYPE_NONE);
	else {
		clif_dropitem(sd,idx,0);
		clif_cart_additem_ack(sd,(flag==1)?ADDITEM_TO_CART_FAIL_WEIGHT:ADDITEM_TO_CART_FAIL_COUNT);
	}
}

/*==========================================
 * Get number of item in cart.
 * Return:
        -1 = itemid not found or no amount found
        x = remaining itemid on cart after get
 *------------------------------------------*/
int pc_cartitem_amount(struct map_session_data* sd, int idx, int amount)
{
	struct item* item_data;

	nullpo_retr(-1, sd);

	item_data = &sd->status.cart[idx];
	if( item_data->nameid == 0 || item_data->amount == 0 )
		return -1;

	return item_data->amount - amount;
}

/*==========================================
 * Retrieve an item at index idx from cart.
 *------------------------------------------*/
void pc_getitemfromcart(struct map_session_data *sd,int idx,int amount)
{
	struct item *item_data;
	unsigned char flag = 0;

	nullpo_retv(sd);

	if (idx < 0 || idx >= MAX_CART) //Invalid index check [Skotlex]
		return;

	item_data=&sd->status.cart[idx];

	if(item_data->nameid == 0 || amount < 1 || item_data->amount < amount || sd->state.vending )
		return;
	if((flag = pc_additem(sd,item_data,amount,LOG_TYPE_NONE)) == 0)
		pc_cart_delitem(sd,idx,amount,0,LOG_TYPE_NONE);
	else {
		clif_dropitem(sd,idx,0);
		clif_additem(sd,0,0,flag);
	}
}

/*==========================================
 * Bound Item Check
 * Type:
 * 1 Account Bound
 * 2 Guild Bound
 * 3 Party Bound
 * 4 Character Bound
 *------------------------------------------*/
int pc_bound_chk(TBL_PC *sd,enum bound_type type,int *idxlist)
{
	int i=0, j=0;
	for(i=0;i<MAX_INVENTORY;i++){
		if(sd->status.inventory[i].nameid > 0 && sd->status.inventory[i].amount > 0 && sd->status.inventory[i].bound == type) {
			idxlist[j] = i;
			j++;
		}
	}
	return j;
}

/*==========================================
 *  Display item stolen msg to player sd
 *------------------------------------------*/
int pc_show_steal(struct block_list *bl,va_list ap)
{
	struct map_session_data *sd;
	int itemid;

	struct item_data *item=NULL;
	char output[100];

	sd=va_arg(ap,struct map_session_data *);
	itemid=va_arg(ap,int);

	if((item=itemdb_exists(itemid))==NULL)
		sprintf(output,"%s stole an Unknown Item (id: %i).",sd->status.name, itemid);
	else
		sprintf(output,"%s stole %s.",sd->status.name,item->jname);
	clif_displaymessage( ((struct map_session_data *)bl)->fd, output);

	return 0;
}
/*==========================================
 * Steal an item from bl (mob).
 * Return:
 *	0 = fail
 *	1 = succes
 *------------------------------------------*/
int pc_steal_item(struct map_session_data *sd,struct block_list *bl, uint16 skill_lv)
{
	int i,itemid;
	double rate;
	unsigned char flag = 0;
	struct status_data *sd_status, *md_status;
	struct mob_data *md;
	struct item tmp_item;

	if(!sd || !bl || bl->type!=BL_MOB)
		return 0;

	md = (TBL_MOB *)bl;

	if(md->state.steal_flag == UCHAR_MAX || ( md->sc.opt1 && md->sc.opt1 != OPT1_BURNING && md->sc.opt1 != OPT1_CRYSTALIZE ) ) //already stolen from / status change check
		return 0;

	sd_status= status_get_status_data(&sd->bl);
	md_status= status_get_status_data(bl);

	if( md->master_id || md_status->mode&MD_BOSS || mob_is_treasure(md) ||
		map[bl->m].flag.nomobloot || // check noloot map flag [Lorky]
		(battle_config.skill_steal_max_tries && //Reached limit of steal attempts. [Lupus]
			md->state.steal_flag++ >= battle_config.skill_steal_max_tries)
  	) { //Can't steal from
		md->state.steal_flag = UCHAR_MAX;
		return 0;
	}

	// base skill success chance (percentual)
	rate = (sd_status->dex - md_status->dex)/2 + skill_lv*6 + 4;
	rate += sd->bonus.add_steal_rate;

	if( rate < 1 )
		return 0;

	// Try dropping one item, in the order from first to last possible slot.
	// Droprate is affected by the skill success rate.
	for( i = 0; i < MAX_STEAL_DROP; i++ )
		if( md->db->dropitem[i].nameid > 0 && itemdb_exists(md->db->dropitem[i].nameid) && rnd() % 10000 < md->db->dropitem[i].p * rate/100. )
			break;
	if( i == MAX_STEAL_DROP )
		return 0;

	itemid = md->db->dropitem[i].nameid;
	memset(&tmp_item,0,sizeof(tmp_item));
	tmp_item.nameid = itemid;
	tmp_item.amount = 1;
	tmp_item.identify = itemdb_isidentified(itemid);
	flag = pc_additem(sd,&tmp_item,1,LOG_TYPE_PICKDROP_PLAYER);

	//TODO: Should we disable stealing when the item you stole couldn't be added to your inventory? Perhaps players will figure out a way to exploit this behaviour otherwise?
	md->state.steal_flag = UCHAR_MAX; //you can't steal from this mob any more

	if(flag) { //Failed to steal due to overweight
		clif_additem(sd,0,0,flag);
		return 0;
	}

	if(battle_config.show_steal_in_same_party)
		party_foreachsamemap(pc_show_steal,sd,AREA_SIZE,sd,tmp_item.nameid);

	//Logs items, Stolen from mobs [Lupus]
	log_pick_mob(md, LOG_TYPE_STEAL, -1, &tmp_item);

	//A Rare Steal Global Announce by Lupus
	if(md->db->dropitem[i].p<=battle_config.rare_drop_announce) {
		struct item_data *i_data;
		char message[128];
		i_data = itemdb_search(itemid);
		sprintf (message, msg_txt(sd,542), (sd->status.name != NULL)?sd->status.name :"GM", md->db->jname, i_data->jname, (float)md->db->dropitem[i].p/100);
		//MSG: "'%s' stole %s's %s (chance: %0.02f%%)"
		intif_broadcast(message, strlen(message) + 1, BC_DEFAULT);
	}
	return 1;
}

/*==========================================
 * Stole zeny from bl (mob)
 * return
 *	0 = fail
 *	1 = success
 *------------------------------------------*/
int pc_steal_coin(struct map_session_data *sd,struct block_list *target)
{
	int rate,skill;
	struct mob_data *md;
	if(!sd || !target || target->type != BL_MOB)
		return 0;

	md = (TBL_MOB*)target;
	if( md->state.steal_coin_flag || md->sc.data[SC_STONE] || md->sc.data[SC_FREEZE] || md->status.mode&MD_BOSS )
		return 0;

	if( mob_is_treasure(md) )
		return 0;

	// FIXME: This formula is either custom or outdated.
	skill = pc_checkskill(sd,RG_STEALCOIN)*10;
	rate = skill + (sd->status.base_level - md->level)*3 + sd->battle_status.dex*2 + sd->battle_status.luk*2;
	if(rnd()%1000 < rate)
	{
		int amount = md->level*10 + rnd()%100;

		pc_getzeny(sd, amount, LOG_TYPE_STEAL, NULL);
		md->state.steal_coin_flag = 1;
		return 1;
	}
	return 0;
}

/*==========================================
 * Set's a player position.
 * @param sd
 * @param mapindex
 * @param x
 * @param y
 * @param clrtype
 * @return 0 - Success; 1 - Invalid map index; 2 - Map not in this map-server, and failed to locate alternate map-server.
 *------------------------------------------*/
char pc_setpos(struct map_session_data* sd, unsigned short mapindex, int x, int y, clr_type clrtype)
{
	int16 m;

	nullpo_ret(sd);

	if( !mapindex || !mapindex_id2name(mapindex) ) {
		ShowDebug("pc_setpos: Passed mapindex(%d) is invalid!\n", mapindex);
		return 1;
	}

	if ( sd->state.autotrade && (sd->vender_id || sd->buyer_id) ) // Player with autotrade just causes clif glitch! @ FIXME
		return 1;

	if( battle_config.revive_onwarp && pc_isdead(sd) ) { //Revive dead people before warping them
		pc_setstand(sd, true);
		pc_setrestartvalue(sd,1);
	}

	m = map_mapindex2mapid(mapindex);

	sd->state.changemap = (sd->mapindex != mapindex);
	sd->state.warping = 1;

	if(sd->status.party_id && map[sd->bl.m].instance_id && sd->state.changemap && !map[m].instance_id) {
		struct party_data *p;
		if((p = party_search(sd->status.party_id)) != NULL && p->instance_id )
			instance_delusers(p->instance_id);
	}
	if( sd->state.changemap ) { // Misc map-changing settings
		int i;
		sd->state.pmap = sd->bl.m;
		if (sd->sc.count) { // Cancel some map related stuff.
			if (sd->sc.data[SC_JAILED])
				return 1; //You may not get out!
			status_change_end(&sd->bl, SC_BOSSMAPINFO, INVALID_TIMER);
			status_change_end(&sd->bl, SC_WARM, INVALID_TIMER);
			status_change_end(&sd->bl, SC_SUN_COMFORT, INVALID_TIMER);
			status_change_end(&sd->bl, SC_MOON_COMFORT, INVALID_TIMER);
			status_change_end(&sd->bl, SC_STAR_COMFORT, INVALID_TIMER);
			status_change_end(&sd->bl, SC_MIRACLE, INVALID_TIMER);
			if (sd->sc.data[SC_KNOWLEDGE]) {
				struct status_change_entry *sce = sd->sc.data[SC_KNOWLEDGE];
				if (sce->timer != INVALID_TIMER)
					delete_timer(sce->timer, status_change_timer);
				sce->timer = add_timer(gettick() + skill_get_time(SG_KNOWLEDGE, sce->val1), status_change_timer, sd->bl.id, SC_KNOWLEDGE);
			}
			status_change_end(&sd->bl, SC_PROPERTYWALK, INVALID_TIMER);
			status_change_end(&sd->bl, SC_CLOAKING, INVALID_TIMER);
			status_change_end(&sd->bl, SC_CLOAKINGEXCEED, INVALID_TIMER);
		}
		for( i = 0; i < EQI_MAX; i++ ) {
			if( sd->equip_index[i] >= 0 )
				if( pc_isequip(sd,sd->equip_index[i]) )
					pc_unequipitem(sd,sd->equip_index[i],2);
		}
		if (battle_config.clear_unit_onwarp&BL_PC)
			skill_clear_unitgroup(&sd->bl);
		party_send_dot_remove(sd); //minimap dot fix [Kevin]
		guild_send_dot_remove(sd);
		bg_send_dot_remove(sd);
		if (sd->regen.state.gc)
			sd->regen.state.gc = 0;
		// make sure vending is allowed here
		if (sd->state.vending && map[m].flag.novending) {
			clif_displaymessage (sd->fd, msg_txt(sd,276)); // "You can't open a shop on this map"
			vending_closevending(sd);
		}

		channel_pcquit(sd,4); //quit map chan
	}

	if( m < 0 )
	{
		uint32 ip;
		uint16 port;
		//if can't find any map-servers, just abort setting position.
		if(!sd->mapindex || map_mapname2ipport(mapindex,&ip,&port))
			return 2;

		if (sd->npc_id)
			npc_event_dequeue(sd);
		npc_script_event(sd, NPCE_LOGOUT);
		//remove from map, THEN change x/y coordinates
		unit_remove_map_pc(sd,clrtype);
		sd->mapindex = mapindex;
		sd->bl.x=x;
		sd->bl.y=y;
		pc_clean_skilltree(sd);
		chrif_save(sd,2);
		chrif_changemapserver(sd, ip, (short)port);

		//Free session data from this map server [Kevin]
		unit_free_pc(sd);

		return 0;
	}

	if( x < 0 || x >= map[m].xs || y < 0 || y >= map[m].ys )
	{
		ShowError("pc_setpos: attempt to place player '%s' (%d:%d) on invalid coordinates (%s-%d,%d)\n", sd->status.name, sd->status.account_id, sd->status.char_id, mapindex_id2name(mapindex),x,y);
		x = y = 0; // make it random
	}

	if( x == 0 && y == 0 ) { // pick a random walkable cell
		int c=0;
		do {
			x = rnd()%(map[m].xs-2)+1;
			y = rnd()%(map[m].ys-2)+1;
			c++;
			
			if(c > (map[m].xs * map[m].ys)*3){ //force out
				ShowError("pc_setpos: couldn't found a valid coordinates for player '%s' (%d:%d) on (%s), preventing warp\n", sd->status.name, sd->status.account_id, sd->status.char_id, mapindex_id2name(mapindex));
				return 0; //preventing warp
				//break; //allow warp anyway
			}
		} while(map_getcell(m,x,y,CELL_CHKNOPASS) || (!battle_config.teleport_on_portal && npc_check_areanpc(1,m,x,y,1)));
	}

	if (sd->state.vending && map_getcell(m,x,y,CELL_CHKNOVENDING)) {
		clif_displaymessage (sd->fd, msg_txt(sd,204)); // "You can't open a shop on this cell."
		vending_closevending(sd);
	}

	if(sd->bl.prev != NULL){
		unit_remove_map_pc(sd,clrtype);
		clif_changemap(sd,m,x,y); // [MouseJstr]
	} else if(sd->state.active) //Tag player for rewarping after map-loading is done. [Skotlex]
		sd->state.rewarp = 1;

	sd->mapindex = mapindex;
	sd->bl.m = m;
	sd->bl.x = sd->ud.to_x = x;
	sd->bl.y = sd->ud.to_y = y;

	if( sd->status.guild_id > 0 && map[m].flag.gvg_castle )
	{	// Increased guild castle regen [Valaris]
		struct guild_castle *gc = guild_mapindex2gc(sd->mapindex);
		if(gc && gc->guild_id == sd->status.guild_id)
			sd->regen.state.gc = 1;
	}

	if( sd->status.pet_id > 0 && sd->pd && sd->pd->pet.intimate > 0 )
	{
		sd->pd->bl.m = m;
		sd->pd->bl.x = sd->pd->ud.to_x = x;
		sd->pd->bl.y = sd->pd->ud.to_y = y;
		sd->pd->ud.dir = sd->ud.dir;
	}

	if( hom_is_active(sd->hd) )
	{
		sd->hd->bl.m = m;
		sd->hd->bl.x = sd->hd->ud.to_x = x;
		sd->hd->bl.y = sd->hd->ud.to_y = y;
		sd->hd->ud.dir = sd->ud.dir;
	}

	if( sd->md )
	{
		sd->md->bl.m = m;
		sd->md->bl.x = sd->md->ud.to_x = x;
		sd->md->bl.y = sd->md->ud.to_y = y;
		sd->md->ud.dir = sd->ud.dir;
	}

	pc_cell_basilica(sd);
	
	//check if we gonna be rewarped [lighta]
	if(npc_check_areanpc(1,m,x,y,0)){
		sd->count_rewarp++;	
	}
	else 
		sd->count_rewarp = 0;
	
	return 0;
}

/*==========================================
 * Warp player sd to random location on current map.
 * May fail if no walkable cell found (1000 attempts).
 * Return:
 *	0 = Success
 *	1,2,3 = Fail
 *------------------------------------------*/
char pc_randomwarp(struct map_session_data *sd, clr_type type)
{
	int x,y,i=0;
	int16 m;

	nullpo_ret(sd);

	m=sd->bl.m;

	if (map[sd->bl.m].flag.noteleport) //Teleport forbidden
		return 3;

	do {
		x = rnd()%(map[m].xs-2)+1;
		y = rnd()%(map[m].ys-2)+1;
	} while((map_getcell(m,x,y,CELL_CHKNOPASS) || (!battle_config.teleport_on_portal && npc_check_areanpc(1,m,x,y,1))) && (i++) < 1000);

	if (i < 1000)
		return pc_setpos(sd,map[sd->bl.m].index,x,y,type);

	return 3;
}

/*==========================================
 * Records a memo point at sd's current position
 * pos - entry to replace, (-1: shift oldest entry out)
 *------------------------------------------*/
bool pc_memo(struct map_session_data* sd, int pos)
{
	int skill;

	nullpo_ret(sd);

	// check mapflags
	if( sd->bl.m >= 0 && (map[sd->bl.m].flag.nomemo || map[sd->bl.m].flag.nowarpto) && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE) ) {
		clif_skill_teleportmessage(sd, 1); // "Saved point cannot be memorized."
		return false;
	}

	// check inputs
	if( pos < -1 || pos >= MAX_MEMOPOINTS )
		return false; // invalid input

	// check required skill level
	skill = pc_checkskill(sd, AL_WARP);
	if( skill < 1 ) {
		clif_skill_memomessage(sd,2); // "You haven't learned Warp."
		return false;
	}
	if( skill < 2 || skill - 2 < pos ) {
		clif_skill_memomessage(sd,1); // "Skill Level is not high enough."
		return false;
	}

	if( pos == -1 )
	{
		uint8 i;
		// prevent memo-ing the same map multiple times
		ARR_FIND( 0, MAX_MEMOPOINTS, i, sd->status.memo_point[i].map == map_id2index(sd->bl.m) );
		memmove(&sd->status.memo_point[1], &sd->status.memo_point[0], (min(i,MAX_MEMOPOINTS-1))*sizeof(struct point));
		pos = 0;
	}

	if( map[sd->bl.m].instance_id ) {
		clif_displaymessage( sd->fd, msg_txt(sd,384) ); // You cannot create a memo in an instance.
		return false;
	}

	sd->status.memo_point[pos].map = map_id2index(sd->bl.m);
	sd->status.memo_point[pos].x = sd->bl.x;
	sd->status.memo_point[pos].y = sd->bl.y;

	clif_skill_memomessage(sd, 0);

	return true;
}

//
// Skills
//

/**
 * Get the skill current cooldown for player.
 * (get the db base cooldown for skill + player specific cooldown)
 * @param sd : player pointer
 * @param id : skill id
 * @param lv : skill lv
 * @return player skill cooldown
 */
int pc_get_skillcooldown(struct map_session_data *sd, uint16 skill_id, uint16 skill_lv) {
	uint8 i;
	uint16 idx = skill_get_index(skill_id);
	int cooldown = 0, cooldownlen = ARRAYLENGTH(sd->skillcooldown);
	
	if (!idx) return 0;
	if (skill_db[idx]->cooldown[skill_lv - 1])
		cooldown = skill_db[idx]->cooldown[skill_lv - 1];

	ARR_FIND(0, cooldownlen, i, sd->skillcooldown[i].id == skill_id);
	if (i < cooldownlen) {
		cooldown += sd->skillcooldown[i].val;
		cooldown = max(0,cooldown);
	}
	return cooldown;
}

/*==========================================
 * Return player sd skill_lv learned for given skill
 *------------------------------------------*/
uint8 pc_checkskill(struct map_session_data *sd, uint16 skill_id)
{
	uint16 idx = 0;
	if (sd == NULL)
		return 0;
	if ((idx = skill_get_index(skill_id)) == 0) {
		ShowError("pc_checkskill: Invalid skill id %d (char_id=%d).\n", skill_id, sd->status.char_id);
		return 0;
	}
	if (SKILL_CHK_GUILD(skill_id) ) {
		struct guild *g;

		if( sd->status.guild_id>0 && (g=sd->guild)!=NULL)
			return guild_checkskill(g,skill_id);
		return 0;
	}
	return (sd->status.skill[idx].id == skill_id) ? sd->status.skill[idx].lv : 0;
}

/**
 * Check if we still have the correct weapon to continue the skill (actually status)
 * If not ending it
 * @param sd
 * @return 0:error, 1:check done
 */
static void pc_checkallowskill(struct map_session_data *sd)
{
	const enum sc_type scw_list[] = {
		SC_TWOHANDQUICKEN,
		SC_ONEHAND,
		SC_AURABLADE,
		SC_PARRYING,
		SC_SPEARQUICKEN,
		SC_ADRENALINE,
		SC_ADRENALINE2,
		SC_DANCING,
		SC_GATLINGFEVER,
	};
	uint8 i;
	nullpo_retv(sd);

	if(!sd->sc.count)
		return;

	for (i = 0; i < ARRAYLENGTH(scw_list); i++)
	{	// Skills requiring specific weapon types
		if( scw_list[i] == SC_DANCING && !battle_config.dancing_weaponswitch_fix )
			continue;
		if(sd->sc.data[scw_list[i]] &&
			!pc_check_weapontype(sd,skill_get_weapontype(status_sc2skill(scw_list[i]))))
			status_change_end(&sd->bl, scw_list[i], INVALID_TIMER);
	}

	if(sd->sc.data[SC_SPURT] && sd->status.weapon)
		// Spurt requires bare hands (feet, in fact xD)
		status_change_end(&sd->bl, SC_SPURT, INVALID_TIMER);

	if(sd->status.shield <= 0) { // Skills requiring a shield
		const enum sc_type scs_list[] = {
			SC_AUTOGUARD,
			SC_DEFENDER,
			SC_REFLECTSHIELD,
			SC_REFLECTDAMAGE
		};
		for (i = 0; i < ARRAYLENGTH(scs_list); i++)
			if(sd->sc.data[scs_list[i]])
				status_change_end(&sd->bl, scs_list[i], INVALID_TIMER);
	}
}

/*==========================================
 * Return equiped itemid? on player sd at pos
 * Return
 * -1 : mean nothing equiped
 * idx : (this index could be used in inventory to found item_data)
 *------------------------------------------*/
short pc_checkequip(struct map_session_data *sd,int pos)
{
	uint8 i;

	nullpo_retr(-1, sd);

	for(i=0;i<EQI_MAX;i++){
		if(pos & equip_pos[i])
			return sd->equip_index[i];
	}

	return -1;
}

/*==========================================
 * Check if sd as nameid equiped somewhere
 * @sd : the player session
 * @nameid : id of the item to check
 * @min : : see pc.h enum equip_index from ? to @max
 * @max : see pc.h enum equip_index for @min to ?
 * -return true,false
 *------------------------------------------*/
bool pc_checkequip2(struct map_session_data *sd, unsigned short nameid, int min, int max){
	int i;
	for(i=min;i<max;i++){
		if(equip_pos[i]){
			int idx = sd->equip_index[i];
			if (sd->status.inventory[idx].nameid == nameid)
				return true;
		}
	}
	return false;
}

/*==========================================
 * Convert's from the client's lame Job ID system
 * to the map server's 'makes sense' system. [Skotlex]
 *------------------------------------------*/
int pc_jobid2mapid(unsigned short b_class)
{
	switch(b_class)
	{
	//Novice And 1-1 Jobs
		case JOB_NOVICE:                return MAPID_NOVICE;
		case JOB_SWORDMAN:              return MAPID_SWORDMAN;
		case JOB_MAGE:                  return MAPID_MAGE;
		case JOB_ARCHER:                return MAPID_ARCHER;
		case JOB_ACOLYTE:               return MAPID_ACOLYTE;
		case JOB_MERCHANT:              return MAPID_MERCHANT;
		case JOB_THIEF:                 return MAPID_THIEF;
		case JOB_TAEKWON:               return MAPID_TAEKWON;
		case JOB_WEDDING:               return MAPID_WEDDING;
		case JOB_GUNSLINGER:            return MAPID_GUNSLINGER;
		case JOB_NINJA:                 return MAPID_NINJA;
		case JOB_XMAS:                  return MAPID_XMAS;
		case JOB_SUMMER:                return MAPID_SUMMER;
		case JOB_HANBOK:                return MAPID_HANBOK;
		case JOB_GANGSI:                return MAPID_GANGSI;
		case JOB_OKTOBERFEST:           return MAPID_OKTOBERFEST;
	//2-1 Jobs
		case JOB_SUPER_NOVICE:          return MAPID_SUPER_NOVICE;
		case JOB_KNIGHT:                return MAPID_KNIGHT;
		case JOB_WIZARD:                return MAPID_WIZARD;
		case JOB_HUNTER:                return MAPID_HUNTER;
		case JOB_PRIEST:                return MAPID_PRIEST;
		case JOB_BLACKSMITH:            return MAPID_BLACKSMITH;
		case JOB_ASSASSIN:              return MAPID_ASSASSIN;
		case JOB_STAR_GLADIATOR:        return MAPID_STAR_GLADIATOR;
		case JOB_KAGEROU:
		case JOB_OBORO:                 return MAPID_KAGEROUOBORO;
		case JOB_REBELLION:             return MAPID_REBELLION;
		case JOB_DEATH_KNIGHT:          return MAPID_DEATH_KNIGHT;
	//2-2 Jobs
		case JOB_CRUSADER:              return MAPID_CRUSADER;
		case JOB_SAGE:                  return MAPID_SAGE;
		case JOB_BARD:
		case JOB_DANCER:                return MAPID_BARDDANCER;
		case JOB_MONK:                  return MAPID_MONK;
		case JOB_ALCHEMIST:             return MAPID_ALCHEMIST;
		case JOB_ROGUE:                 return MAPID_ROGUE;
		case JOB_SOUL_LINKER:           return MAPID_SOUL_LINKER;
		case JOB_DARK_COLLECTOR:        return MAPID_DARK_COLLECTOR;
	//Trans Novice And Trans 1-1 Jobs
		case JOB_NOVICE_HIGH:           return MAPID_NOVICE_HIGH;
		case JOB_SWORDMAN_HIGH:         return MAPID_SWORDMAN_HIGH;
		case JOB_MAGE_HIGH:             return MAPID_MAGE_HIGH;
		case JOB_ARCHER_HIGH:           return MAPID_ARCHER_HIGH;
		case JOB_ACOLYTE_HIGH:          return MAPID_ACOLYTE_HIGH;
		case JOB_MERCHANT_HIGH:         return MAPID_MERCHANT_HIGH;
		case JOB_THIEF_HIGH:            return MAPID_THIEF_HIGH;
	//Trans 2-1 Jobs
		case JOB_LORD_KNIGHT:           return MAPID_LORD_KNIGHT;
		case JOB_HIGH_WIZARD:           return MAPID_HIGH_WIZARD;
		case JOB_SNIPER:                return MAPID_SNIPER;
		case JOB_HIGH_PRIEST:           return MAPID_HIGH_PRIEST;
		case JOB_WHITESMITH:            return MAPID_WHITESMITH;
		case JOB_ASSASSIN_CROSS:        return MAPID_ASSASSIN_CROSS;
	//Trans 2-2 Jobs
		case JOB_PALADIN:               return MAPID_PALADIN;
		case JOB_PROFESSOR:             return MAPID_PROFESSOR;
		case JOB_CLOWN:
		case JOB_GYPSY:                 return MAPID_CLOWNGYPSY;
		case JOB_CHAMPION:              return MAPID_CHAMPION;
		case JOB_CREATOR:               return MAPID_CREATOR;
		case JOB_STALKER:               return MAPID_STALKER;
	//Baby Novice And Baby 1-1 Jobs
		case JOB_BABY:                  return MAPID_BABY;
		case JOB_BABY_SWORDMAN:         return MAPID_BABY_SWORDMAN;
		case JOB_BABY_MAGE:             return MAPID_BABY_MAGE;
		case JOB_BABY_ARCHER:           return MAPID_BABY_ARCHER;
		case JOB_BABY_ACOLYTE:          return MAPID_BABY_ACOLYTE;
		case JOB_BABY_MERCHANT:         return MAPID_BABY_MERCHANT;
		case JOB_BABY_THIEF:            return MAPID_BABY_THIEF;
	//Baby 2-1 Jobs
		case JOB_SUPER_BABY:            return MAPID_SUPER_BABY;
		case JOB_BABY_KNIGHT:           return MAPID_BABY_KNIGHT;
		case JOB_BABY_WIZARD:           return MAPID_BABY_WIZARD;
		case JOB_BABY_HUNTER:           return MAPID_BABY_HUNTER;
		case JOB_BABY_PRIEST:           return MAPID_BABY_PRIEST;
		case JOB_BABY_BLACKSMITH:       return MAPID_BABY_BLACKSMITH;
		case JOB_BABY_ASSASSIN:         return MAPID_BABY_ASSASSIN;
	//Baby 2-2 Jobs
		case JOB_BABY_CRUSADER:         return MAPID_BABY_CRUSADER;
		case JOB_BABY_SAGE:             return MAPID_BABY_SAGE;
		case JOB_BABY_BARD:
		case JOB_BABY_DANCER:           return MAPID_BABY_BARDDANCER;
		case JOB_BABY_MONK:             return MAPID_BABY_MONK;
		case JOB_BABY_ALCHEMIST:        return MAPID_BABY_ALCHEMIST;
		case JOB_BABY_ROGUE:            return MAPID_BABY_ROGUE;
	//3-1 Jobs
		case JOB_SUPER_NOVICE_E:        return MAPID_SUPER_NOVICE_E;
		case JOB_RUNE_KNIGHT:           return MAPID_RUNE_KNIGHT;
		case JOB_WARLOCK:               return MAPID_WARLOCK;
		case JOB_RANGER:                return MAPID_RANGER;
		case JOB_ARCH_BISHOP:           return MAPID_ARCH_BISHOP;
		case JOB_MECHANIC:              return MAPID_MECHANIC;
		case JOB_GUILLOTINE_CROSS:      return MAPID_GUILLOTINE_CROSS;
	//3-2 Jobs
		case JOB_ROYAL_GUARD:           return MAPID_ROYAL_GUARD;
		case JOB_SORCERER:              return MAPID_SORCERER;
		case JOB_MINSTREL:
		case JOB_WANDERER:              return MAPID_MINSTRELWANDERER;
		case JOB_SURA:                  return MAPID_SURA;
		case JOB_GENETIC:               return MAPID_GENETIC;
		case JOB_SHADOW_CHASER:         return MAPID_SHADOW_CHASER;
	//Trans 3-1 Jobs
		case JOB_RUNE_KNIGHT_T:         return MAPID_RUNE_KNIGHT_T;
		case JOB_WARLOCK_T:             return MAPID_WARLOCK_T;
		case JOB_RANGER_T:              return MAPID_RANGER_T;
		case JOB_ARCH_BISHOP_T:         return MAPID_ARCH_BISHOP_T;
		case JOB_MECHANIC_T:            return MAPID_MECHANIC_T;
		case JOB_GUILLOTINE_CROSS_T:    return MAPID_GUILLOTINE_CROSS_T;
	//Trans 3-2 Jobs
		case JOB_ROYAL_GUARD_T:         return MAPID_ROYAL_GUARD_T;
		case JOB_SORCERER_T:            return MAPID_SORCERER_T;
		case JOB_MINSTREL_T:
		case JOB_WANDERER_T:            return MAPID_MINSTRELWANDERER_T;
		case JOB_SURA_T:                return MAPID_SURA_T;
		case JOB_GENETIC_T:             return MAPID_GENETIC_T;
		case JOB_SHADOW_CHASER_T:       return MAPID_SHADOW_CHASER_T;
	//Baby 3-1 Jobs
		case JOB_SUPER_BABY_E:          return MAPID_SUPER_BABY_E;
		case JOB_BABY_RUNE:             return MAPID_BABY_RUNE;
		case JOB_BABY_WARLOCK:          return MAPID_BABY_WARLOCK;
		case JOB_BABY_RANGER:           return MAPID_BABY_RANGER;
		case JOB_BABY_BISHOP:           return MAPID_BABY_BISHOP;
		case JOB_BABY_MECHANIC:         return MAPID_BABY_MECHANIC;
		case JOB_BABY_CROSS:            return MAPID_BABY_CROSS;
	//Baby 3-2 Jobs
		case JOB_BABY_GUARD:            return MAPID_BABY_GUARD;
		case JOB_BABY_SORCERER:         return MAPID_BABY_SORCERER;
		case JOB_BABY_MINSTREL:
		case JOB_BABY_WANDERER:         return MAPID_BABY_MINSTRELWANDERER;
		case JOB_BABY_SURA:             return MAPID_BABY_SURA;
		case JOB_BABY_GENETIC:          return MAPID_BABY_GENETIC;
		case JOB_BABY_CHASER:           return MAPID_BABY_CHASER;
		default:
			return -1;
	}
}

//Reverts the map-style class id to the client-style one.
int pc_mapid2jobid(unsigned short class_, int sex)
{
	switch(class_) {
	//Novice And 1-1 Jobs
		case MAPID_NOVICE:                return JOB_NOVICE;
		case MAPID_SWORDMAN:              return JOB_SWORDMAN;
		case MAPID_MAGE:                  return JOB_MAGE;
		case MAPID_ARCHER:                return JOB_ARCHER;
		case MAPID_ACOLYTE:               return JOB_ACOLYTE;
		case MAPID_MERCHANT:              return JOB_MERCHANT;
		case MAPID_THIEF:                 return JOB_THIEF;
		case MAPID_TAEKWON:               return JOB_TAEKWON;
		case MAPID_WEDDING:               return JOB_WEDDING;
		case MAPID_GUNSLINGER:            return JOB_GUNSLINGER;
		case MAPID_NINJA:                 return JOB_NINJA;
		case MAPID_XMAS:                  return JOB_XMAS;
		case MAPID_SUMMER:                return JOB_SUMMER;
		case MAPID_HANBOK:                return JOB_HANBOK;
		case MAPID_GANGSI:                return JOB_GANGSI;
		case MAPID_OKTOBERFEST:           return JOB_OKTOBERFEST;
	//2-1 Jobs
		case MAPID_SUPER_NOVICE:          return JOB_SUPER_NOVICE;
		case MAPID_KNIGHT:                return JOB_KNIGHT;
		case MAPID_WIZARD:                return JOB_WIZARD;
		case MAPID_HUNTER:                return JOB_HUNTER;
		case MAPID_PRIEST:                return JOB_PRIEST;
		case MAPID_BLACKSMITH:            return JOB_BLACKSMITH;
		case MAPID_ASSASSIN:              return JOB_ASSASSIN;
		case MAPID_STAR_GLADIATOR:        return JOB_STAR_GLADIATOR;
		case MAPID_KAGEROUOBORO:          return sex?JOB_KAGEROU:JOB_OBORO;
		case MAPID_REBELLION:             return JOB_REBELLION;
		case MAPID_DEATH_KNIGHT:          return JOB_DEATH_KNIGHT;
	//2-2 Jobs
		case MAPID_CRUSADER:              return JOB_CRUSADER;
		case MAPID_SAGE:                  return JOB_SAGE;
		case MAPID_BARDDANCER:            return sex?JOB_BARD:JOB_DANCER;
		case MAPID_MONK:                  return JOB_MONK;
		case MAPID_ALCHEMIST:             return JOB_ALCHEMIST;
		case MAPID_ROGUE:                 return JOB_ROGUE;
		case MAPID_SOUL_LINKER:           return JOB_SOUL_LINKER;
		case MAPID_DARK_COLLECTOR:        return JOB_DARK_COLLECTOR;
	//Trans Novice And Trans 2-1 Jobs
		case MAPID_NOVICE_HIGH:           return JOB_NOVICE_HIGH;
		case MAPID_SWORDMAN_HIGH:         return JOB_SWORDMAN_HIGH;
		case MAPID_MAGE_HIGH:             return JOB_MAGE_HIGH;
		case MAPID_ARCHER_HIGH:           return JOB_ARCHER_HIGH;
		case MAPID_ACOLYTE_HIGH:          return JOB_ACOLYTE_HIGH;
		case MAPID_MERCHANT_HIGH:         return JOB_MERCHANT_HIGH;
		case MAPID_THIEF_HIGH:            return JOB_THIEF_HIGH;
	//Trans 2-1 Jobs
		case MAPID_LORD_KNIGHT:           return JOB_LORD_KNIGHT;
		case MAPID_HIGH_WIZARD:           return JOB_HIGH_WIZARD;
		case MAPID_SNIPER:                return JOB_SNIPER;
		case MAPID_HIGH_PRIEST:           return JOB_HIGH_PRIEST;
		case MAPID_WHITESMITH:            return JOB_WHITESMITH;
		case MAPID_ASSASSIN_CROSS:        return JOB_ASSASSIN_CROSS;
	//Trans 2-2 Jobs
		case MAPID_PALADIN:               return JOB_PALADIN;
		case MAPID_PROFESSOR:             return JOB_PROFESSOR;
		case MAPID_CLOWNGYPSY:            return sex?JOB_CLOWN:JOB_GYPSY;
		case MAPID_CHAMPION:              return JOB_CHAMPION;
		case MAPID_CREATOR:               return JOB_CREATOR;
		case MAPID_STALKER:               return JOB_STALKER;
	//Baby Novice And Baby 1-1 Jobs
		case MAPID_BABY:                  return JOB_BABY;
		case MAPID_BABY_SWORDMAN:         return JOB_BABY_SWORDMAN;
		case MAPID_BABY_MAGE:             return JOB_BABY_MAGE;
		case MAPID_BABY_ARCHER:           return JOB_BABY_ARCHER;
		case MAPID_BABY_ACOLYTE:          return JOB_BABY_ACOLYTE;
		case MAPID_BABY_MERCHANT:         return JOB_BABY_MERCHANT;
		case MAPID_BABY_THIEF:            return JOB_BABY_THIEF;
	//Baby 2-1 Jobs
		case MAPID_SUPER_BABY:            return JOB_SUPER_BABY;
		case MAPID_BABY_KNIGHT:           return JOB_BABY_KNIGHT;
		case MAPID_BABY_WIZARD:           return JOB_BABY_WIZARD;
		case MAPID_BABY_HUNTER:           return JOB_BABY_HUNTER;
		case MAPID_BABY_PRIEST:           return JOB_BABY_PRIEST;
		case MAPID_BABY_BLACKSMITH:       return JOB_BABY_BLACKSMITH;
		case MAPID_BABY_ASSASSIN:         return JOB_BABY_ASSASSIN;
	//Baby 2-2 Jobs
		case MAPID_BABY_CRUSADER:         return JOB_BABY_CRUSADER;
		case MAPID_BABY_SAGE:             return JOB_BABY_SAGE;
		case MAPID_BABY_BARDDANCER:       return sex?JOB_BABY_BARD:JOB_BABY_DANCER;
		case MAPID_BABY_MONK:             return JOB_BABY_MONK;
		case MAPID_BABY_ALCHEMIST:        return JOB_BABY_ALCHEMIST;
		case MAPID_BABY_ROGUE:            return JOB_BABY_ROGUE;
	//3-1 Jobs
		case MAPID_SUPER_NOVICE_E:        return JOB_SUPER_NOVICE_E;
		case MAPID_RUNE_KNIGHT:           return JOB_RUNE_KNIGHT;
		case MAPID_WARLOCK:               return JOB_WARLOCK;
		case MAPID_RANGER:                return JOB_RANGER;
		case MAPID_ARCH_BISHOP:           return JOB_ARCH_BISHOP;
		case MAPID_MECHANIC:              return JOB_MECHANIC;
		case MAPID_GUILLOTINE_CROSS:      return JOB_GUILLOTINE_CROSS;
	//3-2 Jobs
		case MAPID_ROYAL_GUARD:           return JOB_ROYAL_GUARD;
		case MAPID_SORCERER:              return JOB_SORCERER;
		case MAPID_MINSTRELWANDERER:      return sex?JOB_MINSTREL:JOB_WANDERER;
		case MAPID_SURA:                  return JOB_SURA;
		case MAPID_GENETIC:               return JOB_GENETIC;
		case MAPID_SHADOW_CHASER:         return JOB_SHADOW_CHASER;
	//Trans 3-1 Jobs
		case MAPID_RUNE_KNIGHT_T:         return JOB_RUNE_KNIGHT_T;
		case MAPID_WARLOCK_T:             return JOB_WARLOCK_T;
		case MAPID_RANGER_T:              return JOB_RANGER_T;
		case MAPID_ARCH_BISHOP_T:         return JOB_ARCH_BISHOP_T;
		case MAPID_MECHANIC_T:            return JOB_MECHANIC_T;
		case MAPID_GUILLOTINE_CROSS_T:    return JOB_GUILLOTINE_CROSS_T;
	//Trans 3-2 Jobs
		case MAPID_ROYAL_GUARD_T:         return JOB_ROYAL_GUARD_T;
		case MAPID_SORCERER_T:            return JOB_SORCERER_T;
		case MAPID_MINSTRELWANDERER_T:    return sex?JOB_MINSTREL_T:JOB_WANDERER_T;
		case MAPID_SURA_T:                return JOB_SURA_T;
		case MAPID_GENETIC_T:             return JOB_GENETIC_T;
		case MAPID_SHADOW_CHASER_T:       return JOB_SHADOW_CHASER_T;
	//Baby 3-1 Jobs
		case MAPID_SUPER_BABY_E:          return JOB_SUPER_BABY_E;
		case MAPID_BABY_RUNE:             return JOB_BABY_RUNE;
		case MAPID_BABY_WARLOCK:          return JOB_BABY_WARLOCK;
		case MAPID_BABY_RANGER:           return JOB_BABY_RANGER;
		case MAPID_BABY_BISHOP:           return JOB_BABY_BISHOP;
		case MAPID_BABY_MECHANIC:         return JOB_BABY_MECHANIC;
		case MAPID_BABY_CROSS:            return JOB_BABY_CROSS;
	//Baby 3-2 Jobs
		case MAPID_BABY_GUARD:            return JOB_BABY_GUARD;
		case MAPID_BABY_SORCERER:         return JOB_BABY_SORCERER;
		case MAPID_BABY_MINSTRELWANDERER: return sex?JOB_BABY_MINSTREL:JOB_BABY_WANDERER;
		case MAPID_BABY_SURA:             return JOB_BABY_SURA;
		case MAPID_BABY_GENETIC:          return JOB_BABY_GENETIC;
		case MAPID_BABY_CHASER:           return JOB_BABY_CHASER;
		default:
			return -1;
	}
}

/*====================================================
 * This function return the name of the job (by [Yor])
 *----------------------------------------------------*/
const char* job_name(int class_)
{
	switch (class_) {
	case JOB_NOVICE:
	case JOB_SWORDMAN:
	case JOB_MAGE:
	case JOB_ARCHER:
	case JOB_ACOLYTE:
	case JOB_MERCHANT:
	case JOB_THIEF:
		return msg_txt(NULL,550 - JOB_NOVICE+class_);

	case JOB_KNIGHT:
	case JOB_PRIEST:
	case JOB_WIZARD:
	case JOB_BLACKSMITH:
	case JOB_HUNTER:
	case JOB_ASSASSIN:
		return msg_txt(NULL,557 - JOB_KNIGHT+class_);

	case JOB_KNIGHT2:
		return msg_txt(NULL,557);

	case JOB_CRUSADER:
	case JOB_MONK:
	case JOB_SAGE:
	case JOB_ROGUE:
	case JOB_ALCHEMIST:
	case JOB_BARD:
	case JOB_DANCER:
		return msg_txt(NULL,563 - JOB_CRUSADER+class_);

	case JOB_CRUSADER2:
		return msg_txt(NULL,563);

	case JOB_WEDDING:
	case JOB_SUPER_NOVICE:
	case JOB_GUNSLINGER:
	case JOB_NINJA:
	case JOB_XMAS:
		return msg_txt(NULL,570 - JOB_WEDDING+class_);

	case JOB_SUMMER:
		return msg_txt(NULL,621);

	case JOB_HANBOK:
		return msg_txt(NULL,694);

	case JOB_OKTOBERFEST:
		return msg_txt(NULL,696);

	case JOB_NOVICE_HIGH:
	case JOB_SWORDMAN_HIGH:
	case JOB_MAGE_HIGH:
	case JOB_ARCHER_HIGH:
	case JOB_ACOLYTE_HIGH:
	case JOB_MERCHANT_HIGH:
	case JOB_THIEF_HIGH:
		return msg_txt(NULL,575 - JOB_NOVICE_HIGH+class_);

	case JOB_LORD_KNIGHT:
	case JOB_HIGH_PRIEST:
	case JOB_HIGH_WIZARD:
	case JOB_WHITESMITH:
	case JOB_SNIPER:
	case JOB_ASSASSIN_CROSS:
		return msg_txt(NULL,582 - JOB_LORD_KNIGHT+class_);

	case JOB_LORD_KNIGHT2:
		return msg_txt(NULL,582);

	case JOB_PALADIN:
	case JOB_CHAMPION:
	case JOB_PROFESSOR:
	case JOB_STALKER:
	case JOB_CREATOR:
	case JOB_CLOWN:
	case JOB_GYPSY:
		return msg_txt(NULL,588 - JOB_PALADIN + class_);

	case JOB_PALADIN2:
		return msg_txt(NULL,588);

	case JOB_BABY:
	case JOB_BABY_SWORDMAN:
	case JOB_BABY_MAGE:
	case JOB_BABY_ARCHER:
	case JOB_BABY_ACOLYTE:
	case JOB_BABY_MERCHANT:
	case JOB_BABY_THIEF:
		return msg_txt(NULL,595 - JOB_BABY + class_);

	case JOB_BABY_KNIGHT:
	case JOB_BABY_PRIEST:
	case JOB_BABY_WIZARD:
	case JOB_BABY_BLACKSMITH:
	case JOB_BABY_HUNTER:
	case JOB_BABY_ASSASSIN:
		return msg_txt(NULL,602 - JOB_BABY_KNIGHT + class_);

	case JOB_BABY_KNIGHT2:
		return msg_txt(NULL,602);

	case JOB_BABY_CRUSADER:
	case JOB_BABY_MONK:
	case JOB_BABY_SAGE:
	case JOB_BABY_ROGUE:
	case JOB_BABY_ALCHEMIST:
	case JOB_BABY_BARD:
	case JOB_BABY_DANCER:
		return msg_txt(NULL,608 - JOB_BABY_CRUSADER + class_);

	case JOB_BABY_CRUSADER2:
		return msg_txt(NULL,608);

	case JOB_SUPER_BABY:
		return msg_txt(NULL,615);

	case JOB_TAEKWON:
		return msg_txt(NULL,616);
	case JOB_STAR_GLADIATOR:
	case JOB_STAR_GLADIATOR2:
		return msg_txt(NULL,617);
	case JOB_SOUL_LINKER:
		return msg_txt(NULL,618);

	case JOB_GANGSI:
	case JOB_DEATH_KNIGHT:
	case JOB_DARK_COLLECTOR:
		return msg_txt(NULL,622 - JOB_GANGSI+class_);

	case JOB_RUNE_KNIGHT:
	case JOB_WARLOCK:
	case JOB_RANGER:
	case JOB_ARCH_BISHOP:
	case JOB_MECHANIC:
	case JOB_GUILLOTINE_CROSS:
		return msg_txt(NULL,625 - JOB_RUNE_KNIGHT+class_);

	case JOB_RUNE_KNIGHT_T:
	case JOB_WARLOCK_T:
	case JOB_RANGER_T:
	case JOB_ARCH_BISHOP_T:
	case JOB_MECHANIC_T:
	case JOB_GUILLOTINE_CROSS_T:
		return msg_txt(NULL,681 - JOB_RUNE_KNIGHT_T+class_);

	case JOB_ROYAL_GUARD:
	case JOB_SORCERER:
	case JOB_MINSTREL:
	case JOB_WANDERER:
	case JOB_SURA:
	case JOB_GENETIC:
	case JOB_SHADOW_CHASER:
		return msg_txt(NULL,631 - JOB_ROYAL_GUARD+class_);

	case JOB_ROYAL_GUARD_T:
	case JOB_SORCERER_T:
	case JOB_MINSTREL_T:
	case JOB_WANDERER_T:
	case JOB_SURA_T:
	case JOB_GENETIC_T:
	case JOB_SHADOW_CHASER_T:
		return msg_txt(NULL,687 - JOB_ROYAL_GUARD_T+class_);

	case JOB_RUNE_KNIGHT2:
	case JOB_RUNE_KNIGHT_T2:
		return msg_txt(NULL,625);

	case JOB_ROYAL_GUARD2:
	case JOB_ROYAL_GUARD_T2:
		return msg_txt(NULL,631);

	case JOB_RANGER2:
	case JOB_RANGER_T2:
		return msg_txt(NULL,627);

	case JOB_MECHANIC2:
	case JOB_MECHANIC_T2:
		return msg_txt(NULL,629);

	case JOB_BABY_RUNE:
	case JOB_BABY_WARLOCK:
	case JOB_BABY_RANGER:
	case JOB_BABY_BISHOP:
	case JOB_BABY_MECHANIC:
	case JOB_BABY_CROSS:
	case JOB_BABY_GUARD:
	case JOB_BABY_SORCERER:
	case JOB_BABY_MINSTREL:
	case JOB_BABY_WANDERER:
	case JOB_BABY_SURA:
	case JOB_BABY_GENETIC:
	case JOB_BABY_CHASER:
		return msg_txt(NULL,638 - JOB_BABY_RUNE+class_);

	case JOB_BABY_RUNE2:
		return msg_txt(NULL,638);

	case JOB_BABY_GUARD2:
		return msg_txt(NULL,644);

	case JOB_BABY_RANGER2:
		return msg_txt(NULL,640);

	case JOB_BABY_MECHANIC2:
		return msg_txt(NULL,642);

	case JOB_SUPER_NOVICE_E:
	case JOB_SUPER_BABY_E:
		return msg_txt(NULL,651 - JOB_SUPER_NOVICE_E+class_);

	case JOB_KAGEROU:
	case JOB_OBORO:
		return msg_txt(NULL,653 - JOB_KAGEROU+class_);

	case JOB_REBELLION:
		return msg_txt(NULL,695);

	default:
		return msg_txt(NULL,655);
	}
}

/*====================================================
 * Timered function to make id follow a target.
 * @id = bl.id (player only atm)
 * target is define in sd->followtarget (bl.id)
 * used by pc_follow
 *----------------------------------------------------*/
int pc_follow_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd;
	struct block_list *tbl;

	sd = map_id2sd(id);
	nullpo_ret(sd);

	if (sd->followtimer != tid){
		ShowError("pc_follow_timer %d != %d\n",sd->followtimer,tid);
		sd->followtimer = INVALID_TIMER;
		return 0;
	}

	sd->followtimer = INVALID_TIMER;
	tbl = map_id2bl(sd->followtarget);

	if (tbl == NULL || pc_isdead(sd) || status_isdead(tbl))
	{
		pc_stop_following(sd);
		return 0;
	}

	// either player or target is currently detached from map blocks (could be teleporting),
	// but still connected to this map, so we'll just increment the timer and check back later
	if (sd->bl.prev != NULL && tbl->prev != NULL &&
		sd->ud.skilltimer == INVALID_TIMER && sd->ud.attacktimer == INVALID_TIMER && sd->ud.walktimer == INVALID_TIMER)
	{
		if((sd->bl.m == tbl->m) && unit_can_reach_bl(&sd->bl,tbl, AREA_SIZE, 0, NULL, NULL)) {
			if (!check_distance_bl(&sd->bl, tbl, 5))
				unit_walktobl(&sd->bl, tbl, 5, 0);
		} else
			pc_setpos(sd, map_id2index(tbl->m), tbl->x, tbl->y, CLR_TELEPORT);
	}
	sd->followtimer = add_timer(
		tick + 1000,	// increase time a bit to loosen up map's load
		pc_follow_timer, sd->bl.id, 0);
	return 0;
}

int pc_stop_following (struct map_session_data *sd)
{
	nullpo_ret(sd);

	if (sd->followtimer != INVALID_TIMER) {
		delete_timer(sd->followtimer,pc_follow_timer);
		sd->followtimer = INVALID_TIMER;
	}
	sd->followtarget = -1;
	sd->ud.target_to = 0;

	unit_stop_walking(&sd->bl, 1);

	return 0;
}

int pc_follow(struct map_session_data *sd,int target_id)
{
	struct block_list *bl = map_id2bl(target_id);
	if (bl == NULL /*|| bl->type != BL_PC*/)
		return 1;
	if (sd->followtimer != INVALID_TIMER)
		pc_stop_following(sd);

	sd->followtarget = target_id;
	pc_follow_timer(INVALID_TIMER, gettick(), sd->bl.id, 0);

	return 0;
}

int pc_checkbaselevelup(struct map_session_data *sd) {
	unsigned int next = pc_nextbaseexp(sd);

	if (!next || sd->status.base_exp < next)
		return 0;

	do {
		sd->status.base_exp -= next;
		//Kyoki pointed out that the max overcarry exp is the exp needed for the previous level -1. [Skotlex]
		if(!battle_config.multi_level_up && sd->status.base_exp > next-1)
			sd->status.base_exp = next-1;

		next = pc_gets_status_point(sd->status.base_level);
		sd->status.base_level ++;
		sd->status.status_point += next;

	} while ((next=pc_nextbaseexp(sd)) > 0 && sd->status.base_exp >= next);

	if (battle_config.pet_lv_rate && sd->pd)	//<Skotlex> update pet's level
		status_calc_pet(sd->pd,SCO_NONE);

	clif_updatestatus(sd,SP_STATUSPOINT);
	clif_updatestatus(sd,SP_BASELEVEL);
	clif_updatestatus(sd,SP_BASEEXP);
	clif_updatestatus(sd,SP_NEXTBASEEXP);
	status_calc_pc(sd,SCO_FORCE);
	status_percent_heal(&sd->bl,100,100);

	if ((sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE) {
		sc_start(&sd->bl,&sd->bl,status_skill2sc(PR_KYRIE),100,1,skill_get_time(PR_KYRIE,1));
		sc_start(&sd->bl,&sd->bl,status_skill2sc(PR_IMPOSITIO),100,1,skill_get_time(PR_IMPOSITIO,1));
		sc_start(&sd->bl,&sd->bl,status_skill2sc(PR_MAGNIFICAT),100,1,skill_get_time(PR_MAGNIFICAT,1));
		sc_start(&sd->bl,&sd->bl,status_skill2sc(PR_GLORIA),100,1,skill_get_time(PR_GLORIA,1));
		sc_start(&sd->bl,&sd->bl,status_skill2sc(PR_SUFFRAGIUM),100,1,skill_get_time(PR_SUFFRAGIUM,1));
		if (sd->state.snovice_dead_flag)
			sd->state.snovice_dead_flag = 0; //Reenable steelbody resurrection on dead.
	} else if( (sd->class_&MAPID_BASEMASK) == MAPID_TAEKWON ) {
		sc_start(&sd->bl,&sd->bl,status_skill2sc(AL_INCAGI),100,10,600000);
		sc_start(&sd->bl,&sd->bl,status_skill2sc(AL_BLESSING),100,10,600000);
	}
	clif_misceffect(&sd->bl,0);
	npc_script_event(sd, NPCE_BASELVUP); //LORDALFA - LVLUPEVENT

	if(sd->status.party_id)
		party_send_levelup(sd);

	pc_baselevelchanged(sd);
	return 1;
}

void pc_baselevelchanged(struct map_session_data *sd) {
	uint8 i;
	for( i = 0; i < EQI_MAX; i++ ) {
		if( sd->equip_index[i] >= 0 ) {
			if( sd->inventory_data[ sd->equip_index[i] ]->elvmax && sd->status.base_level > (unsigned int)sd->inventory_data[ sd->equip_index[i] ]->elvmax )
				pc_unequipitem(sd, sd->equip_index[i], 3);
		}
	}
}

int pc_checkjoblevelup(struct map_session_data *sd)
{
	unsigned int next = pc_nextjobexp(sd);

	nullpo_ret(sd);
	if(!next || sd->status.job_exp < next)
		return 0;

	do {
		sd->status.job_exp -= next;
		//Kyoki pointed out that the max overcarry exp is the exp needed for the previous level -1. [Skotlex]
		if(!battle_config.multi_level_up && sd->status.job_exp > next-1)
			sd->status.job_exp = next-1;

		sd->status.job_level ++;
		sd->status.skill_point ++;

	} while ((next=pc_nextjobexp(sd)) > 0 && sd->status.job_exp >= next);

	clif_updatestatus(sd,SP_JOBLEVEL);
	clif_updatestatus(sd,SP_JOBEXP);
	clif_updatestatus(sd,SP_NEXTJOBEXP);
	clif_updatestatus(sd,SP_SKILLPOINT);
	status_calc_pc(sd,SCO_FORCE);
	clif_misceffect(&sd->bl,1);
	if (pc_checkskill(sd, SG_DEVIL) && !pc_nextjobexp(sd))
		clif_status_change(&sd->bl,SI_DEVIL, 1, 0, 0, 0, 1); //Permanent blind effect from SG_DEVIL.

	npc_script_event(sd, NPCE_JOBLVUP);
	return 1;
}

/** Alters experiences calculation based on self bonuses that do not get even shared to the party.
* @param sd Player
* @param base_exp Base EXP before peronal bonuses
* @param job_exp Job EXP before peronal bonuses
* @param src Block list that affecting the exp calculation
*/
static void pc_calcexp(struct map_session_data *sd, unsigned int *base_exp, unsigned int *job_exp, struct block_list *src)
{
	int bonus = 0, vip_bonus_base = 0, vip_bonus_job = 0;

	if (src) {
		struct status_data *status = status_get_status_data(src);

		if( sd->expaddrace[status->race] )
			bonus += sd->expaddrace[status->race];
		if( sd->expaddrace[RC_ALL] )
			bonus += sd->expaddrace[RC_ALL];
		if( sd->expaddclass[status->class_] )
			bonus += sd->expaddclass[status->class_];
		if( sd->expaddclass[CLASS_ALL] )
			bonus += sd->expaddclass[CLASS_ALL];

		if (battle_config.pk_mode &&
			(int)(status_get_lv(src) - sd->status.base_level) >= 20)
			bonus += 15; // pk_mode additional exp if monster >20 levels [Valaris]

#ifdef VIP_ENABLE
		//EXP bonus for VIP player
		if (src && src->type == BL_MOB && pc_isvip(sd)) {
			vip_bonus_base = battle_config.vip_base_exp_increase;
			vip_bonus_job = battle_config.vip_job_exp_increase;
		}
#endif
	}

	if (&sd->sc && sd->sc.data[SC_EXPBOOST]) {
		bonus += sd->sc.data[SC_EXPBOOST]->val1;
		if( battle_config.vip_bm_increase && pc_isvip(sd) ) // Increase Battle Manual EXP rate for VIP.
			bonus += ( sd->sc.data[SC_EXPBOOST]->val1 / battle_config.vip_bm_increase );
	}

	*base_exp = (unsigned int) cap_value(*base_exp + (double)*base_exp * (bonus + vip_bonus_base)/100., 1, UINT_MAX);

	if (&sd->sc && sd->sc.data[SC_JEXPBOOST])
		bonus += sd->sc.data[SC_JEXPBOOST]->val1;

	*job_exp = (unsigned int) cap_value(*job_exp + (double)*job_exp * (bonus + vip_bonus_job)/100., 1, UINT_MAX);

	return;
}
/*==========================================
 * Give x exp at sd player and calculate remaining exp for next lvl
 *------------------------------------------*/
int pc_gainexp(struct map_session_data *sd, struct block_list *src, unsigned int base_exp, unsigned int job_exp, bool quest)
{
	float nextbp = 0, nextjp = 0;
	unsigned int nextb = 0, nextj = 0;

	nullpo_ret(sd);

	if(sd->bl.prev == NULL || pc_isdead(sd))
		return 0;

	if(!battle_config.pvp_exp && map[sd->bl.m].flag.pvp)  // [MouseJstr]
		return 0; // no exp on pvp maps
	
	if(sd->status.guild_id>0)
		base_exp-=guild_payexp(sd,base_exp);

	pc_calcexp(sd, &base_exp, &job_exp, src); // Give (J)EXPBOOST for quests even if src is NULL.

	nextb = pc_nextbaseexp(sd);
	nextj = pc_nextjobexp(sd);

	if(sd->state.showexp || battle_config.max_exp_gain_rate){
		if (nextb > 0)
			nextbp = (float) base_exp / (float) nextb;
		if (nextj > 0)
			nextjp = (float) job_exp / (float) nextj;

		if(battle_config.max_exp_gain_rate) {
			if (nextbp > battle_config.max_exp_gain_rate/1000.) {
				//Note that this value should never be greater than the original
				//base_exp, therefore no overflow checks are needed. [Skotlex]
				base_exp = (unsigned int)(battle_config.max_exp_gain_rate/1000.*nextb);
				if (sd->state.showexp)
					nextbp = (float) base_exp / (float) nextb;
			}
			if (nextjp > battle_config.max_exp_gain_rate/1000.) {
				job_exp = (unsigned int)(battle_config.max_exp_gain_rate/1000.*nextj);
				if (sd->state.showexp)
					nextjp = (float) job_exp / (float) nextj;
			}
		}
	}

	//Cap exp to the level up requirement of the previous level when you are at max level, otherwise cap at UINT_MAX (this is required for some S. Novice bonuses). [Skotlex]
	if (base_exp) {
		nextb = nextb?UINT_MAX:pc_thisbaseexp(sd);
		if(sd->status.base_exp > nextb - base_exp)
			sd->status.base_exp = nextb;
		else
			sd->status.base_exp += base_exp;
		pc_checkbaselevelup(sd);
		clif_updatestatus(sd,SP_BASEEXP);
	}

	if (job_exp) {
		nextj = nextj?UINT_MAX:pc_thisjobexp(sd);
		if(sd->status.job_exp > nextj - job_exp)
			sd->status.job_exp = nextj;
		else
			sd->status.job_exp += job_exp;
		pc_checkjoblevelup(sd);
		clif_updatestatus(sd,SP_JOBEXP);
	}

	if(base_exp)
		clif_displayexp(sd, base_exp, SP_BASEEXP, quest);
	if(job_exp)
		clif_displayexp(sd, job_exp,  SP_JOBEXP, quest);
	if(sd->state.showexp) {
		char output[256];
		sprintf(output,
			"Experience Gained Base:%u (%.2f%%) Job:%u (%.2f%%)",base_exp,nextbp*(float)100,job_exp,nextjp*(float)100);
		clif_disp_onlyself(sd,output,strlen(output));
	}

	return 1;
}

/*==========================================
 * Returns max level for this character.
 *------------------------------------------*/
unsigned int pc_maxbaselv(struct map_session_data *sd){
	return job_info[pc_class2idx(sd->status.class_)].max_level[0];
}

unsigned int pc_maxjoblv(struct map_session_data *sd){
	return job_info[pc_class2idx(sd->status.class_)].max_level[1];
}

/*==========================================
 * base level exp lookup.
 *------------------------------------------*/

//Base exp needed for next level.
unsigned int pc_nextbaseexp(struct map_session_data *sd){
	nullpo_ret(sd);
	if(sd->status.base_level>=pc_maxbaselv(sd) || sd->status.base_level==0)
		return 0;
	return job_info[pc_class2idx(sd->status.class_)].exp_table[0][sd->status.base_level-1];
}

//Base exp needed for this level.
unsigned int pc_thisbaseexp(struct map_session_data *sd){
	if(sd->status.base_level>pc_maxbaselv(sd) || sd->status.base_level<=1)
		return 0;
	return job_info[pc_class2idx(sd->status.class_)].exp_table[0][sd->status.base_level-2];
}


/*==========================================
 * job level exp lookup
 * Return:
 *	0 = not found
 *	x = exp for level
 *------------------------------------------*/

//Job exp needed for next level.
unsigned int pc_nextjobexp(struct map_session_data *sd){
	nullpo_ret(sd);
	if(sd->status.job_level>=pc_maxjoblv(sd) || sd->status.job_level==0)
		return 0;
	return job_info[pc_class2idx(sd->status.class_)].exp_table[1][sd->status.job_level-1];
}

//Job exp needed for this level.
unsigned int pc_thisjobexp(struct map_session_data *sd){
	if(sd->status.job_level>pc_maxjoblv(sd) || sd->status.job_level<=1)
		return 0;
	return job_info[pc_class2idx(sd->status.class_)].exp_table[1][sd->status.job_level-2];
}

/// Returns the value of the specified stat.
static int pc_getstat(struct map_session_data* sd, int type)
{
	nullpo_retr(-1, sd);

	switch( type ) {
	case SP_STR: return sd->status.str;
	case SP_AGI: return sd->status.agi;
	case SP_VIT: return sd->status.vit;
	case SP_INT: return sd->status.int_;
	case SP_DEX: return sd->status.dex;
	case SP_LUK: return sd->status.luk;
	default:
		return -1;
	}
}

/// Sets the specified stat to the specified value.
/// Returns the new value.
static int pc_setstat(struct map_session_data* sd, int type, int val)
{
	nullpo_retr(-1, sd);

	switch( type ) {
	case SP_STR: sd->status.str = val; break;
	case SP_AGI: sd->status.agi = val; break;
	case SP_VIT: sd->status.vit = val; break;
	case SP_INT: sd->status.int_ = val; break;
	case SP_DEX: sd->status.dex = val; break;
	case SP_LUK: sd->status.luk = val; break;
	default:
		return -1;
	}

	return val;
}

// Calculates the number of status points PC gets when leveling up (from level to level+1)
int pc_gets_status_point(int level)
{
	if (battle_config.use_statpoint_table) //Use values from "db/statpoint.txt"
		return (statp[level+1] - statp[level]);
	else //Default increase
		return ((level+15) / 5);
}

#ifdef RENEWAL_STAT
/// Renewal status point cost formula
#define PC_STATUS_POINT_COST(low) (((low) < 100) ? (2 + ((low) - 1) / 10) : (16 + 4 * (((low) - 100) / 5)))
#else
/// Pre-Renewal status point cost formula
#define PC_STATUS_POINT_COST(low) (( 1 + ((low) + 9) / 10 ))
#endif

/// Returns the number of stat points needed to change the specified stat by val.
/// If val is negative, returns the number of stat points that would be needed to
/// raise the specified stat from (current value - val) to current value.
int pc_need_status_point(struct map_session_data* sd, int type, int val)
{
	int low, high, sp = 0, max = 0;

	if ( val == 0 )
		return 0;

	low = pc_getstat(sd,type);
	max = pc_maxparameter(sd,(enum e_params)(type-SP_STR));

	if ( low >= max && val > 0 )
		return 0; // Official servers show '0' when max is reached

	high = low + val;

	if ( val < 0 )
		swap(low, high);

	for ( ; low < high; low++ )
		sp += PC_STATUS_POINT_COST(low);

	return sp;
}

/**
 * Returns the value the specified stat can be increased by with the current
 * amount of available status points for the current character's class.
 *
 * @param sd   The target character.
 * @param type Stat to verify.
 * @return Maximum value the stat could grow by.
 */
int pc_maxparameterincrease(struct map_session_data* sd, int type)
{
	int base, final_val, status_points, max_param;

	nullpo_ret(sd);

	base = final_val = pc_getstat(sd, type);
	status_points = sd->status.status_point;
	max_param = pc_maxparameter(sd, (enum e_params)(type-SP_STR));

	while (final_val <= max_param && status_points >= 0) {
		status_points -= PC_STATUS_POINT_COST(final_val);
		final_val++;
	}
	final_val--;

	return (final_val > base ? final_val-base : 0);
}

/**
 * Raises a stat by the specified amount.
 *
 * Obeys max_parameter limits.
 * Subtracts status points according to the cost of the increased stat points.
 *
 * @param sd       The target character.
 * @param type     The stat to change (see enum _sp)
 * @param increase The stat increase (strictly positive) amount.
 * @retval true  if the stat was increased by any amount.
 * @retval false if there were no changes.
 */
bool pc_statusup(struct map_session_data* sd, int type, int increase)
{
	int max_increase = 0, current = 0, needed_points = 0, final_value = 0;

	nullpo_ret(sd);

	// check conditions
	if (type < SP_STR || type > SP_LUK || increase <= 0) {
		clif_statusupack(sd, type, 0, 0);
		return false;
	}

	// check limits
	current = pc_getstat(sd, type);
	max_increase = pc_maxparameterincrease(sd, type);
	increase = cap_value(increase, 0, max_increase); // cap to the maximum status points available
	if (increase <= 0 || current + increase > pc_maxparameter(sd, (enum e_params)(type-SP_STR))) {
		clif_statusupack(sd, type, 0, 0);
		return false;
	}

	// check status points
	needed_points = pc_need_status_point(sd, type, increase);
	if (needed_points < 0 || needed_points > sd->status.status_point) { // Sanity check
		clif_statusupack(sd, type, 0, 0);
		return false;
	}

	// set new values
	final_value = pc_setstat(sd, type, current + increase);
	sd->status.status_point -= needed_points;

	status_calc_pc(sd,SCO_NONE);

	// update increase cost indicator
	clif_updatestatus(sd, SP_USTR + type-SP_STR);

	// update statpoint count
	clif_updatestatus(sd, SP_STATUSPOINT);

	// update stat value
	clif_statusupack(sd, type, 1, final_value); // required
	if( final_value > 255 )
		clif_updatestatus(sd, type); // send after the 'ack' to override the truncated value

	return true;
}

/**
 * Raises a stat by the specified amount.
 *
 * Obeys max_parameter limits.
 * Does not subtract status points for the cost of the modified stat points.
 *
 * @param sd   The target character.
 * @param type The stat to change (see enum _sp)
 * @param val  The stat increase (or decrease) amount.
 * @return the stat increase amount.
 * @retval 0 if no changes were made.
 */
int pc_statusup2(struct map_session_data* sd, int type, int val)
{
	int max, need;
	nullpo_ret(sd);

	if( type < SP_STR || type > SP_LUK )
	{
		clif_statusupack(sd,type,0,0);
		return 0;
	}

	need = pc_need_status_point(sd,type,1);
	max = pc_maxparameter(sd,(enum e_params)(type-SP_STR)); // set new value

	val = pc_setstat(sd, type, cap_value(pc_getstat(sd,type) + val, 1, max));

	status_calc_pc(sd,SCO_NONE);

	// update increase cost indicator
	if( need != pc_need_status_point(sd,type,1) )
		clif_updatestatus(sd, SP_USTR + type-SP_STR);

	// update stat value
	clif_statusupack(sd,type,1,val); // required
	if( val > 255 )
		clif_updatestatus(sd,type); // send after the 'ack' to override the truncated value

	return val;
}

/*==========================================
 * Update skill_lv for player sd
 * Skill point allocation
 *------------------------------------------*/
void pc_skillup(struct map_session_data *sd,uint16 skill_id)
{
	uint16 idx = skill_get_index(skill_id);

	nullpo_retv(sd);

	if (!idx) {
		if (skill_id)
			ShowError("pc_skillup: Player attempts to level up invalid skill '%d'\n", skill_id);
		return;
	}

	// Level up guild skill
	if (SKILL_CHK_GUILD(skill_id)) {
		guild_skillup(sd, skill_id);
		return;
	}
	// Level up homunculus skill
	else if (sd->hd && SKILL_CHK_HOMUN(skill_id)) {
		hom_skillup(sd->hd, skill_id);
		return;
	}
	else {
		if( sd->status.skill_point > 0 &&
			sd->status.skill[idx].id &&
			sd->status.skill[idx].flag == SKILL_FLAG_PERMANENT && //Don't allow raising while you have granted skills. [Skotlex]
			sd->status.skill[idx].lv < skill_tree_get_max(skill_id, sd->status.class_) )
		{
			int lv, range, upgradable;
			sd->status.skill[idx].lv++;
			sd->status.skill_point--;
			if( !skill_get_inf(skill_id) )
				status_calc_pc(sd,SCO_NONE); // Only recalculate for passive skills.
			else if( sd->status.skill_point == 0 && pc_is_taekwon_ranker(sd) )
				pc_calc_skilltree(sd); // Required to grant all TK Ranker skills.
			else
				pc_check_skilltree(sd); // Check if a new skill can Lvlup

			lv = sd->status.skill[idx].lv;
			range = skill_get_range2(&sd->bl, skill_id, lv);
			upgradable = (lv < skill_tree_get_max(sd->status.skill[idx].id, sd->status.class_)) ? 1 : 0;
			clif_skillup(sd,skill_id,lv,range,upgradable);
			clif_updatestatus(sd,SP_SKILLPOINT);
			if( skill_id == GN_REMODELING_CART ) /* cart weight info was updated by status_calc_pc */
				clif_updatestatus(sd,SP_CARTINFO);
			if (!pc_has_permission(sd, PC_PERM_ALL_SKILL)) // may skill everything at any time anyways, and this would cause a huge slowdown
				clif_skillinfoblock(sd);
		}
		//else
		//	ShowDebug("Skill Level up failed. ID:%d idx:%d (CID=%d. AID=%d)\n", skill_id, idx, sd->status.char_id, sd->status.account_id);
	}
}

/*==========================================
 * /allskill
 *------------------------------------------*/
int pc_allskillup(struct map_session_data *sd)
{
	int i;

	nullpo_ret(sd);

	for (i = 0; i < MAX_SKILL; i++) {
		if (sd->status.skill[i].flag != SKILL_FLAG_PERMANENT && sd->status.skill[i].flag != SKILL_FLAG_PERM_GRANTED && sd->status.skill[i].flag != SKILL_FLAG_PLAGIARIZED) {
			sd->status.skill[i].lv = (sd->status.skill[i].flag == SKILL_FLAG_TEMPORARY) ? 0 : sd->status.skill[i].flag - SKILL_FLAG_REPLACED_LV_0;
			sd->status.skill[i].flag = SKILL_FLAG_PERMANENT;
			if (sd->status.skill[i].lv == 0)
				sd->status.skill[i].id = 0;
		}
	}

	if (!pc_grant_allskills(sd, true)) {
		uint16 sk_id;
		for (i = 0; i < MAX_SKILL_TREE && (sk_id = skill_tree[pc_class2idx(sd->status.class_)][i].id) > 0;i++){
			int inf2 = 0;
			uint16 sk_idx = 0;
			if (!sk_id || !(sk_idx = skill_get_index(sk_id)))
				continue;
			inf2 = skill_get_inf2(sk_id);
			if (
				(inf2&INF2_QUEST_SKILL && !battle_config.quest_skill_learn) ||
				(inf2&(INF2_WEDDING_SKILL|INF2_SPIRIT_SKILL)) ||
				sk_id == SG_DEVIL
			)
				continue; //Cannot be learned normally.

			sd->status.skill[sk_idx].id = sk_id;
			sd->status.skill[sk_idx].lv = skill_tree_get_max(sk_id, sd->status.class_);	// celest
		}
	}
	status_calc_pc(sd,SCO_NONE);
	//Required because if you could level up all skills previously,
	//the update will not be sent as only the lv variable changes.
	clif_skillinfoblock(sd);
	return 0;
}

/*==========================================
 * /resetlvl
 *------------------------------------------*/
int pc_resetlvl(struct map_session_data* sd,int type)
{
	int  i;

	nullpo_ret(sd);

	if (type != 3) //Also reset skills
		pc_resetskill(sd, 0);

	if(type == 1){
		sd->status.skill_point=0;
		sd->status.base_level=1;
		sd->status.job_level=1;
		sd->status.base_exp=0;
		sd->status.job_exp=0;
		if(sd->sc.option !=0)
			sd->sc.option = 0;

		sd->status.str=1;
		sd->status.agi=1;
		sd->status.vit=1;
		sd->status.int_=1;
		sd->status.dex=1;
		sd->status.luk=1;
		if(sd->status.class_ == JOB_NOVICE_HIGH) {
			sd->status.status_point=100;	// not 88 [celest]
			// give platinum skills upon changing
			pc_skill(sd,NV_FIRSTAID,1,ADDSKILL_PERMANENT);
			pc_skill(sd,NV_TRICKDEAD,1,ADDSKILL_PERMANENT);
		}
	}

	if(type == 2){
		sd->status.skill_point=0;
		sd->status.base_level=1;
		sd->status.job_level=1;
		sd->status.base_exp=0;
		sd->status.job_exp=0;
	}
	if(type == 3){
		sd->status.base_level=1;
		sd->status.base_exp=0;
	}
	if(type == 4){
		sd->status.job_level=1;
		sd->status.job_exp=0;
	}

	clif_updatestatus(sd,SP_STATUSPOINT);
	clif_updatestatus(sd,SP_STR);
	clif_updatestatus(sd,SP_AGI);
	clif_updatestatus(sd,SP_VIT);
	clif_updatestatus(sd,SP_INT);
	clif_updatestatus(sd,SP_DEX);
	clif_updatestatus(sd,SP_LUK);
	clif_updatestatus(sd,SP_BASELEVEL);
	clif_updatestatus(sd,SP_JOBLEVEL);
	clif_updatestatus(sd,SP_STATUSPOINT);
	clif_updatestatus(sd,SP_BASEEXP);
	clif_updatestatus(sd,SP_JOBEXP);
	clif_updatestatus(sd,SP_NEXTBASEEXP);
	clif_updatestatus(sd,SP_NEXTJOBEXP);
	clif_updatestatus(sd,SP_SKILLPOINT);

	clif_updatestatus(sd,SP_USTR);	// Updates needed stat points - Valaris
	clif_updatestatus(sd,SP_UAGI);
	clif_updatestatus(sd,SP_UVIT);
	clif_updatestatus(sd,SP_UINT);
	clif_updatestatus(sd,SP_UDEX);
	clif_updatestatus(sd,SP_ULUK);	// End Addition

	for(i=0;i<EQI_MAX;i++) { // unequip items that can't be equipped by base 1 [Valaris]
		if(sd->equip_index[i] >= 0)
			if(pc_isequip(sd,sd->equip_index[i]))
				pc_unequipitem(sd,sd->equip_index[i],2);
	}

	if ((type == 1 || type == 2 || type == 3) && sd->status.party_id)
		party_send_levelup(sd);

	status_calc_pc(sd, SCO_FORCE);
	clif_skillinfoblock(sd);

	return 0;
}
/*==========================================
 * /resetstate
 *------------------------------------------*/
int pc_resetstate(struct map_session_data* sd)
{
	nullpo_ret(sd);

	if (battle_config.use_statpoint_table)
	{	// New statpoint table used here - Dexity
		if (sd->status.base_level > MAX_LEVEL)
		{	//statp[] goes out of bounds, can't reset!
			ShowError("pc_resetstate: Can't reset stats of %d:%d, the base level (%d) is greater than the max level supported (%d)\n",
				sd->status.account_id, sd->status.char_id, sd->status.base_level, MAX_LEVEL);
			return 0;
		}

		sd->status.status_point = statp[sd->status.base_level] + ( sd->class_&JOBL_UPPER ? 52 : 0 ); // extra 52+48=100 stat points
	}
	else
	{
		int add=0;
		add += pc_need_status_point(sd, SP_STR, 1-pc_getstat(sd, SP_STR));
		add += pc_need_status_point(sd, SP_AGI, 1-pc_getstat(sd, SP_AGI));
		add += pc_need_status_point(sd, SP_VIT, 1-pc_getstat(sd, SP_VIT));
		add += pc_need_status_point(sd, SP_INT, 1-pc_getstat(sd, SP_INT));
		add += pc_need_status_point(sd, SP_DEX, 1-pc_getstat(sd, SP_DEX));
		add += pc_need_status_point(sd, SP_LUK, 1-pc_getstat(sd, SP_LUK));

		sd->status.status_point+=add;
	}

	pc_setstat(sd, SP_STR, 1);
	pc_setstat(sd, SP_AGI, 1);
	pc_setstat(sd, SP_VIT, 1);
	pc_setstat(sd, SP_INT, 1);
	pc_setstat(sd, SP_DEX, 1);
	pc_setstat(sd, SP_LUK, 1);

	clif_updatestatus(sd,SP_STR);
	clif_updatestatus(sd,SP_AGI);
	clif_updatestatus(sd,SP_VIT);
	clif_updatestatus(sd,SP_INT);
	clif_updatestatus(sd,SP_DEX);
	clif_updatestatus(sd,SP_LUK);

	clif_updatestatus(sd,SP_USTR);	// Updates needed stat points - Valaris
	clif_updatestatus(sd,SP_UAGI);
	clif_updatestatus(sd,SP_UVIT);
	clif_updatestatus(sd,SP_UINT);
	clif_updatestatus(sd,SP_UDEX);
	clif_updatestatus(sd,SP_ULUK);	// End Addition

	clif_updatestatus(sd,SP_STATUSPOINT);

	if( sd->mission_mobid ) { //bugreport:2200
		sd->mission_mobid = 0;
		sd->mission_count = 0;
		pc_setglobalreg(sd, add_str("TK_MISSION_ID"), 0);
	}

	status_calc_pc(sd, SCO_NONE);

	return 1;
}

/*==========================================
 * /resetskill
 * if flag&1, perform block resync and status_calc call.
 * if flag&2, just count total amount of skill points used by player, do not really reset.
 * if flag&4, just reset the skills if the player class is a bard/dancer type (for changesex.)
 *------------------------------------------*/
int pc_resetskill(struct map_session_data* sd, int flag)
{
	int i, skill_point=0;
	nullpo_ret(sd);

	if( flag&4 && (sd->class_&MAPID_UPPERMASK) != MAPID_BARDDANCER )
		return 0;

	if( !(flag&2) ) { //Remove stuff lost when resetting skills.
		/**
		 * It has been confirmed on official servers that when you reset skills with a ranked Taekwon your skills are not reset (because you have all of them anyway)
		 **/
		if( pc_is_taekwon_ranker(sd) )
			return 0;

		if( pc_checkskill(sd, SG_DEVIL) &&  !pc_nextjobexp(sd) )
			clif_status_load(&sd->bl, SI_DEVIL, 0); //Remove perma blindness due to skill-reset. [Skotlex]
		i = sd->sc.option;
		if( i&OPTION_RIDING && pc_checkskill(sd, KN_RIDING) )
			i &= ~OPTION_RIDING;
		if( i&OPTION_FALCON && pc_checkskill(sd, HT_FALCON) )
			i &= ~OPTION_FALCON;
		if( i&OPTION_DRAGON && pc_checkskill(sd, RK_DRAGONTRAINING) )
			i &= ~OPTION_DRAGON;
		if( i&OPTION_WUG && pc_checkskill(sd, RA_WUGMASTERY) )
			i &= ~OPTION_WUG;
		if( i&OPTION_WUGRIDER && pc_checkskill(sd, RA_WUGRIDER) )
			i &= ~OPTION_WUGRIDER;
		if( i&OPTION_MADOGEAR && ( sd->class_&MAPID_THIRDMASK ) == MAPID_MECHANIC )
			i &= ~OPTION_MADOGEAR;
#ifndef NEW_CARTS
		if( i&OPTION_CART && pc_checkskill(sd, MC_PUSHCART) )
			i &= ~OPTION_CART;
#else
		if( sd->sc.data[SC_PUSH_CART] )
			pc_setcart(sd, 0);
#endif
		if( i != sd->sc.option )
			pc_setoption(sd, i);

		if( hom_is_active(sd->hd) && pc_checkskill(sd, AM_CALLHOMUN) )
			hom_vaporize(sd, HOM_ST_ACTIVE);
	}

	for( i = 1; i < MAX_SKILL; i++ )
	{
		uint8 lv = sd->status.skill[i].lv;
		int inf2;
		uint16 skill_id = skill_idx2id(i);
		if (lv == 0 || skill_id == 0)
			continue;

		inf2 = skill_get_inf2(skill_id);

		if( inf2&(INF2_WEDDING_SKILL|INF2_SPIRIT_SKILL) ) //Avoid reseting wedding/linker skills.
			continue;

		// Don't reset trick dead if not a novice/baby
		if( skill_id == NV_TRICKDEAD && (sd->class_&MAPID_UPPERMASK) != MAPID_NOVICE )
		{
			sd->status.skill[i].lv = 0;
			sd->status.skill[i].flag = SKILL_FLAG_PERMANENT;
			continue;
		}

		// do not reset basic skill
		if( skill_id == NV_BASIC && (sd->class_&MAPID_UPPERMASK) != MAPID_NOVICE )
			continue;

		if( sd->status.skill[i].flag == SKILL_FLAG_PERM_GRANTED )
			continue;

		if( flag&4 && !skill_ischangesex(skill_id) )
			continue;

		if( inf2&INF2_QUEST_SKILL && !battle_config.quest_skill_learn )
		{ //Only handle quest skills in a special way when you can't learn them manually
			if( battle_config.quest_skill_reset && !(flag&2) )
			{	//Wipe them
				sd->status.skill[i].lv = 0;
				sd->status.skill[i].flag = SKILL_FLAG_PERMANENT;
			}
			continue;
		}
		if( sd->status.skill[i].flag == SKILL_FLAG_PERMANENT )
			skill_point += lv;
		else
		if( sd->status.skill[i].flag >= SKILL_FLAG_REPLACED_LV_0 )
			skill_point += (sd->status.skill[i].flag - SKILL_FLAG_REPLACED_LV_0);

		if( !(flag&2) )
		{// reset
			sd->status.skill[i].lv = 0;
			sd->status.skill[i].flag = SKILL_FLAG_PERMANENT;
		}
	}

	if( flag&2 || !skill_point ) return skill_point;

	sd->status.skill_point += skill_point;

	if (flag&1) {
		clif_updatestatus(sd,SP_SKILLPOINT);
		clif_skillinfoblock(sd);
		status_calc_pc(sd, SCO_FORCE);
	}

	return skill_point;
}

/*==========================================
 * /resetfeel [Komurka]
 *------------------------------------------*/
int pc_resetfeel(struct map_session_data* sd)
{
	int i;
	nullpo_ret(sd);

	for (i=0; i<MAX_PC_FEELHATE; i++)
	{
		sd->feel_map[i].m = -1;
		sd->feel_map[i].index = 0;
		pc_setglobalreg(sd, add_str(sg_info[i].feel_var), 0);
	}

	return 0;
}

int pc_resethate(struct map_session_data* sd)
{
	int i;
	nullpo_ret(sd);

	for (i=0; i<3; i++)
	{
		sd->hate_mob[i] = -1;
		pc_setglobalreg(sd, add_str(sg_info[i].hate_var), 0);
	}
	return 0;
}

int pc_skillatk_bonus(struct map_session_data *sd, uint16 skill_id)
{
	int i, bonus = 0;
	nullpo_ret(sd);

	skill_id = skill_dummy2skill_id(skill_id);

	ARR_FIND(0, ARRAYLENGTH(sd->skillatk), i, sd->skillatk[i].id == skill_id);
	if( i < ARRAYLENGTH(sd->skillatk) )
		bonus = sd->skillatk[i].val;

	return bonus;
}

int pc_sub_skillatk_bonus(struct map_session_data *sd, uint16 skill_id)
{
	int i, bonus = 0;
	nullpo_ret(sd);

	skill_id = skill_dummy2skill_id(skill_id);

	ARR_FIND(0, ARRAYLENGTH(sd->subskill), i, sd->subskill[i].id == skill_id);
	if( i < ARRAYLENGTH(sd->subskill) )
		bonus = sd->subskill[i].val;

	return bonus;
}

int pc_skillheal_bonus(struct map_session_data *sd, uint16 skill_id) {
	int i, bonus = sd->bonus.add_heal_rate;

	skill_id = skill_dummy2skill_id(skill_id);

	if( bonus ) {
		switch( skill_id ) {
			case AL_HEAL:           if( !(battle_config.skill_add_heal_rate&1) ) bonus = 0; break;
			case PR_SANCTUARY:      if( !(battle_config.skill_add_heal_rate&2) ) bonus = 0; break;
			case AM_POTIONPITCHER:  if( !(battle_config.skill_add_heal_rate&4) ) bonus = 0; break;
			case CR_SLIMPITCHER:    if( !(battle_config.skill_add_heal_rate&8) ) bonus = 0; break;
			case BA_APPLEIDUN:      if( !(battle_config.skill_add_heal_rate&16)) bonus = 0; break;
		}
	}

	ARR_FIND(0, ARRAYLENGTH(sd->skillheal), i, sd->skillheal[i].id == skill_id);

	if( i < ARRAYLENGTH(sd->skillheal) )
		bonus += sd->skillheal[i].val;

	return bonus;
}

int pc_skillheal2_bonus(struct map_session_data *sd, uint16 skill_id) {
	int i, bonus = sd->bonus.add_heal2_rate;

	skill_id = skill_dummy2skill_id(skill_id);

	ARR_FIND(0, ARRAYLENGTH(sd->skillheal2), i, sd->skillheal2[i].id == skill_id);

	if( i < ARRAYLENGTH(sd->skillheal2) )
		bonus += sd->skillheal2[i].val;

	return bonus;
}

void pc_respawn(struct map_session_data* sd, clr_type clrtype)
{
	if( !pc_isdead(sd) )
		return; // not applicable
	if( sd->bg_id && bg_member_respawn(sd) )
		return; // member revived by battleground

	pc_setstand(sd, true);
	pc_setrestartvalue(sd,3);
	if( pc_setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, clrtype) )
		clif_resurrection(&sd->bl, 1); //If warping fails, send a normal stand up packet.
}

static int pc_respawn_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd = map_id2sd(id);
	if( sd != NULL )
	{
		sd->pvp_point=0;
		pc_respawn(sd,CLR_OUTSIGHT);
	}

	return 0;
}

/*==========================================
 * Invoked when a player has received damage
 *------------------------------------------*/
void pc_damage(struct map_session_data *sd,struct block_list *src,unsigned int hp, unsigned int sp)
{
	if (sp) clif_updatestatus(sd,SP_SP);
	if (hp) clif_updatestatus(sd,SP_HP);
	else return;

	if( !src || src == &sd->bl )
		return;

	if( pc_issit(sd) ) {
		pc_setstand(sd, true);
		skill_sit(sd,0);
	}

	if( sd->progressbar.npc_id )
		clif_progressbar_abort(sd);

	if( sd->status.pet_id > 0 && sd->pd && battle_config.pet_damage_support )
		pet_target_check(sd->pd,src,1);

	if( sd->status.ele_id > 0 )
		elemental_set_target(sd,src);

	sd->canlog_tick = gettick();
}

int pc_close_npc_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	TBL_PC *sd = map_id2sd(id);
	if(sd) pc_close_npc(sd,data);
	return 0;
}
/*
 *  Method to properly close npc for player and clear anything related
 * @flag == 1 : produce close button
 * @flag == 2 : directly close it
 */
void pc_close_npc(struct map_session_data *sd,int flag)
{
	nullpo_retv(sd);

	if (sd->npc_id || sd->npc_shopid) {
		if (sd->state.using_fake_npc) {
			clif_clearunit_single(sd->npc_id, CLR_OUTSIGHT, sd->fd);
			sd->state.using_fake_npc = 0;
		}

		if (sd->st) {
			if(sd->st->state == RUN){ //wait ending code execution
				add_timer(gettick()+500,pc_close_npc_timer,sd->bl.id,flag);
				return;
			}
			sd->st->state = ((flag==1 && sd->st->mes_active)?CLOSE:END);
			sd->st->mes_active = 0;
		}
		sd->state.menu_or_input = 0;
		sd->npc_menu = 0;
		sd->npc_shopid = 0;
#ifdef SECURE_NPCTIMEOUT
		sd->npc_idle_timer = INVALID_TIMER;
#endif
		clif_scriptclose(sd,sd->npc_id);
		clif_scriptclear(sd,sd->npc_id); // [Ind/Hercules]
		if(sd->st && sd->st->state == END ) {// free attached scripts that are waiting
			script_free_state(sd->st);
			sd->st = NULL;
			sd->npc_id = 0;
		}
	}
}

/*==========================================
 * Invoked when a player has negative current hp
 *------------------------------------------*/
int pc_dead(struct map_session_data *sd,struct block_list *src)
{
	int i=0,k=0;
	unsigned int tick = gettick();

	// Activate Steel body if a super novice dies at 99+% exp [celest]
	// Super Novices have no kill or die functions attached when saved by their angel
	if (!sd->state.snovice_dead_flag && (sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE) {
		unsigned int next = pc_nextbaseexp(sd);

		if( next == 0 ) next = pc_thisbaseexp(sd);
		if( get_percentage(sd->status.base_exp,next) >= 99 ) {
			sd->state.snovice_dead_flag = 1;
			pc_setrestartvalue(sd,1);
			status_percent_heal(&sd->bl, 100, 100);
			clif_resurrection(&sd->bl, 1);
			if(battle_config.pc_invincible_time)
				pc_setinvincibletimer(sd, battle_config.pc_invincible_time);
			sc_start(&sd->bl,&sd->bl,status_skill2sc(MO_STEELBODY),100,5,skill_get_time(MO_STEELBODY,5));
			if(map_flag_gvg(sd->bl.m))
				pc_respawn_timer(INVALID_TIMER, gettick(), sd->bl.id, 0);
			return 0;
		}
	}

	for(k = 0; k < MAX_DEVOTION; k++) {
		if (sd->devotion[k]){
			struct map_session_data *devsd = map_id2sd(sd->devotion[k]);
			if (devsd)
				status_change_end(&devsd->bl, SC_DEVOTION, INVALID_TIMER);
			sd->devotion[k] = 0;
		}
	}
	if(sd->shadowform_id) { //if we were target of shadowform
		status_change_end(map_id2bl(sd->shadowform_id), SC__SHADOWFORM, INVALID_TIMER);
		sd->shadowform_id = 0; //should be remove on status end anyway
	}

	if(sd->status.pet_id > 0 && sd->pd) {
		struct pet_data *pd = sd->pd;
		if( !map[sd->bl.m].flag.noexppenalty ) {
			pet_set_intimate(pd, pd->pet.intimate - pd->petDB->die);
			if( pd->pet.intimate < 0 )
				pd->pet.intimate = 0;
			clif_send_petdata(sd,sd->pd,1,pd->pet.intimate);
		}
		if( sd->pd->target_id ) // Unlock all targets...
			pet_unlocktarget(sd->pd);
	}

	if (sd->status.hom_id > 0) {
		if(battle_config.homunculus_auto_vapor && sd->hd)
			hom_vaporize(sd, HOM_ST_ACTIVE);
	}

	if( sd->md )
		mercenary_delete(sd->md, 3); // Your mercenary soldier has ran away.

	if( sd->ed )
		elemental_delete(sd->ed, 0);

	// Leave duel if you die [LuzZza]
	if(battle_config.duel_autoleave_when_die) {
		if(sd->duel_group > 0)
			duel_leave(sd->duel_group, sd);
		if(sd->duel_invite > 0)
			duel_reject(sd->duel_invite, sd);
	}

	pc_close_npc(sd,2); //close npc if we were using one

	/* e.g. not killed thru pc_damage */
	if( pc_issit(sd) ) {
		clif_status_load(&sd->bl,SI_SIT,0);
	}

	pc_setdead(sd);

	pc_setglobalreg(sd, add_str("PC_DIE_COUNTER"), sd->die_counter+1);
	pc_setparam(sd, SP_KILLERRID, src?src->id:0);

	//Reset menu skills/item skills
	if ((sd->skillitem) != 0)
		sd->skillitem = sd->skillitemlv = 0;
	if ((sd->menuskill_id) != 0)
		sd->menuskill_id = sd->menuskill_val = 0;
	//Reset ticks.
	sd->hp_loss.tick = sd->sp_loss.tick = sd->hp_regen.tick = sd->sp_regen.tick = 0;

	if ( sd->spiritball !=0 )
		pc_delspiritball(sd,sd->spiritball,0);

	if (sd->spiritcharm_type != CHARM_TYPE_NONE && sd->spiritcharm > 0)
		pc_delspiritcharm(sd,sd->spiritcharm,sd->spiritcharm_type);

	if (src)
	switch (src->type) {
		case BL_MOB:
		{
			struct mob_data *md=(struct mob_data *)src;
			if(md->target_id==sd->bl.id)
				mob_unlocktarget(md,tick);
			if(battle_config.mobs_level_up && md->status.hp &&
				(unsigned int)md->level < pc_maxbaselv(sd) &&
				!md->guardian_data && !md->special_state.ai// Guardians/summons should not level. [Skotlex]
			) { 	// monster level up [Valaris]
				clif_misceffect(&md->bl,0);
				md->level++;
				status_calc_mob(md, SCO_NONE);
				status_percent_heal(src,10,0);

				if( battle_config.show_mob_info&4 )
				{// update name with new level
					clif_charnameack(0, &md->bl);
				}
			}
			src = battle_get_master(src); // Maybe Player Summon
		}
			break;
		case BL_PET: //Pass on to master...
		case BL_HOM:
		case BL_MER:
			src = battle_get_master(src);
			break;
	}

	if (src && src->type == BL_PC) {
		struct map_session_data *ssd = (struct map_session_data *)src;
		pc_setparam(ssd, SP_KILLEDRID, sd->bl.id);
		npc_script_event(ssd, NPCE_KILLPC);

		if (battle_config.pk_mode&2) {
			ssd->status.manner -= 5;
			if(ssd->status.manner < 0)
				sc_start(&sd->bl,src,SC_NOCHAT,100,0,0);
#if 0
			// PK/Karma system code (not enabled yet) [celest]
			// originally from Kade Online, so i don't know if any of these is correct ^^;
			// note: karma is measured REVERSE, so more karma = more 'evil' / less honourable,
			// karma going down = more 'good' / more honourable.
			// The Karma System way...

			if (sd->status.karma > ssd->status.karma) {	// If player killed was more evil
				sd->status.karma--;
				ssd->status.karma--;
			}
			else if (sd->status.karma < ssd->status.karma)	// If player killed was more good
				ssd->status.karma++;


			// or the PK System way...

			if (sd->status.karma > 0)	// player killed is dishonourable?
				ssd->status.karma--; // honour points earned
			sd->status.karma++;	// honour points lost

			// To-do: Receive exp on certain occasions
#endif
		}
	}

	if(battle_config.bone_drop==2
		|| (battle_config.bone_drop==1 && map[sd->bl.m].flag.pvp))
	{
		struct item item_tmp;
		memset(&item_tmp,0,sizeof(item_tmp));
		item_tmp.nameid=ITEMID_SKULL_;
		item_tmp.identify=1;
		item_tmp.card[0]=CARD0_CREATE;
		item_tmp.card[1]=0;
		item_tmp.card[2]=GetWord(sd->status.char_id,0); // CharId
		item_tmp.card[3]=GetWord(sd->status.char_id,1);
		map_addflooritem(&item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0,0);
	}

	//Remove bonus_script when dead
	pc_bonus_script_clear(sd,BSF_REM_ON_DEAD);

	// changed penalty options, added death by player if pk_mode [Valaris]
	if(battle_config.death_penalty_type
		&& (sd->class_&MAPID_UPPERMASK) != MAPID_NOVICE	// only novices will receive no penalty
		&& !map[sd->bl.m].flag.noexppenalty && !map_flag_gvg(sd->bl.m)
		&& !sd->sc.data[SC_BABY] && !sd->sc.data[SC_LIFEINSURANCE])
	{
		uint32 base_penalty = battle_config.death_penalty_base;
		uint32 job_penalty = battle_config.death_penalty_job;
		uint32 zeny_penalty = battle_config.zeny_penalty;

#ifdef VIP_ENABLE
		if(pc_isvip(sd)){
			base_penalty *= battle_config.vip_exp_penalty_base;
			job_penalty *= battle_config.vip_exp_penalty_job;
		}
		else {
			base_penalty *= battle_config.vip_exp_penalty_base_normal;
			job_penalty *= battle_config.vip_exp_penalty_job_normal;
		}
#endif

		if (base_penalty > 0) {
			switch (battle_config.death_penalty_type) {
				case 1: base_penalty = (uint32) ( pc_nextbaseexp(sd) * ( base_penalty / 10000. ) ); break;
				case 2: base_penalty = (uint32) ( sd->status.base_exp * ( base_penalty / 10000. ) ); break;
			}
			if (base_penalty > 0){ //recheck after altering to speedup
				if (battle_config.pk_mode && src && src->type==BL_PC)
					base_penalty*=2;
				sd->status.base_exp -= min(sd->status.base_exp, base_penalty);
				clif_updatestatus(sd,SP_BASEEXP);
			}
		}

		if(job_penalty > 0) {
			switch (battle_config.death_penalty_type) {
				case 1: job_penalty = (uint32) ( pc_nextjobexp(sd) * ( job_penalty / 10000. ) ); break;
				case 2: job_penalty = (uint32) ( sd->status.job_exp * ( job_penalty /10000. ) ); break;
			}
			if(job_penalty) {
				if (battle_config.pk_mode && src && src->type==BL_PC)
					job_penalty*=2;
				sd->status.job_exp -= min(sd->status.job_exp, job_penalty);
				clif_updatestatus(sd,SP_JOBEXP);
			}
		}

		if( zeny_penalty > 0 && !map[sd->bl.m].flag.nozenypenalty) {
			zeny_penalty = (uint32)( sd->status.zeny * ( zeny_penalty / 10000. ) );
			if(zeny_penalty)
				pc_payzeny(sd, zeny_penalty, LOG_TYPE_PICKDROP_PLAYER, NULL);
		}
	}

	if(map[sd->bl.m].flag.pvp_nightmaredrop) { // Moved this outside so it works when PVP isn't enabled and during pk mode [Ancyker]
		int j;
		for(j=0;j<MAX_DROP_PER_MAP;j++){
			int id = map[sd->bl.m].drop_list[j].drop_id;
			int type = map[sd->bl.m].drop_list[j].drop_type;
			int per = map[sd->bl.m].drop_list[j].drop_per;
			if(id == 0)
				continue;
			if(id == -1){
				int eq_num=0,eq_n[MAX_INVENTORY];
				memset(eq_n,0,sizeof(eq_n));
				for(i=0;i<MAX_INVENTORY;i++) {
					if( (type == 1 && !sd->status.inventory[i].equip)
						|| (type == 2 && sd->status.inventory[i].equip)
						||  type == 3)
					{
						int l;
						ARR_FIND( 0, MAX_INVENTORY, l, eq_n[l] <= 0 );
						if( l < MAX_INVENTORY )
							eq_n[l] = i;

						eq_num++;
					}
				}
				if(eq_num > 0){
					int n = eq_n[rnd()%eq_num];
					if(rnd()%10000 < per) {
						if(sd->status.inventory[n].equip)
							pc_unequipitem(sd,n,3);
						pc_dropitem(sd,n,1);
					}
				}
			}
			else if(id > 0) {
				for(i=0;i<MAX_INVENTORY;i++){
					if(sd->status.inventory[i].nameid == id
						&& rnd()%10000 < per
						&& ((type == 1 && !sd->status.inventory[i].equip)
							|| (type == 2 && sd->status.inventory[i].equip)
							|| type == 3) ){
						if(sd->status.inventory[i].equip)
							pc_unequipitem(sd,i,3);
						pc_dropitem(sd,i,1);
						break;
					}
				}
			}
		}
	}
	// pvp
	// disable certain pvp functions on pk_mode [Valaris]
	if( map[sd->bl.m].flag.pvp && !battle_config.pk_mode && !map[sd->bl.m].flag.pvp_nocalcrank ) {
		sd->pvp_point -= 5;
		sd->pvp_lost++;
		if( src && src->type == BL_PC ) {
			struct map_session_data *ssd = (struct map_session_data *)src;
			ssd->pvp_point++;
			ssd->pvp_won++;
		}
		if( sd->pvp_point < 0 ) {
			add_timer(tick+1000, pc_respawn_timer,sd->bl.id,0);
			return 1|8;
		}
	}
	//GvG
	if( map_flag_gvg(sd->bl.m) ) {
		add_timer(tick+1000, pc_respawn_timer, sd->bl.id, 0);
		return 1|8;
	}
	else if( sd->bg_id ) {
		struct battleground_data *bg = bg_team_search(sd->bg_id);
		if( bg && bg->mapindex > 0 ) { // Respawn by BG
			add_timer(tick+1000, pc_respawn_timer, sd->bl.id, 0);
			return 1|8;
		}
	}

	//Reset "can log out" tick.
	if( battle_config.prevent_logout )
		sd->canlog_tick = gettick() - battle_config.prevent_logout;
	return 1;
}

void pc_revive(struct map_session_data *sd,unsigned int hp, unsigned int sp) {
	if(hp) clif_updatestatus(sd,SP_HP);
	if(sp) clif_updatestatus(sd,SP_SP);

	pc_setstand(sd, true);
	if(battle_config.pc_invincible_time > 0)
		pc_setinvincibletimer(sd, battle_config.pc_invincible_time);

	if( sd->state.gmaster_flag ) {
		guild_guildaura_refresh(sd,GD_LEADERSHIP,guild_checkskill(sd->guild,GD_LEADERSHIP));
		guild_guildaura_refresh(sd,GD_GLORYWOUNDS,guild_checkskill(sd->guild,GD_GLORYWOUNDS));
		guild_guildaura_refresh(sd,GD_SOULCOLD,guild_checkskill(sd->guild,GD_SOULCOLD));
		guild_guildaura_refresh(sd,GD_HAWKEYES,guild_checkskill(sd->guild,GD_HAWKEYES));
	}
}
// script
//
/*==========================================
 * script reading pc status registry
 *------------------------------------------*/
int pc_readparam(struct map_session_data* sd,int type)
{
	int val = 0;

	nullpo_ret(sd);

	switch(type) {
		case SP_SKILLPOINT:      val = sd->status.skill_point; break;
		case SP_STATUSPOINT:     val = sd->status.status_point; break;
		case SP_ZENY:            val = sd->status.zeny; break;
		case SP_BASELEVEL:       val = sd->status.base_level; break;
		case SP_JOBLEVEL:        val = sd->status.job_level; break;
		case SP_CLASS:           val = sd->status.class_; break;
		case SP_BASEJOB:         val = pc_mapid2jobid(sd->class_&MAPID_UPPERMASK, sd->status.sex); break; //Base job, extracting upper type.
		case SP_UPPER:           val = sd->class_&JOBL_UPPER?1:(sd->class_&JOBL_BABY?2:0); break;
		case SP_BASECLASS:       val = pc_mapid2jobid(sd->class_&MAPID_BASEMASK, sd->status.sex); break; //Extract base class tree. [Skotlex]
		case SP_SEX:             val = sd->status.sex; break;
		case SP_WEIGHT:          val = sd->weight; break;
		case SP_MAXWEIGHT:       val = sd->max_weight; break;
		case SP_BASEEXP:         val = sd->status.base_exp; break;
		case SP_JOBEXP:          val = sd->status.job_exp; break;
		case SP_NEXTBASEEXP:     val = pc_nextbaseexp(sd); break;
		case SP_NEXTJOBEXP:      val = pc_nextjobexp(sd); break;
		case SP_HP:              val = sd->battle_status.hp; break;
		case SP_MAXHP:           val = sd->battle_status.max_hp; break;
		case SP_SP:              val = sd->battle_status.sp; break;
		case SP_MAXSP:           val = sd->battle_status.max_sp; break;
		case SP_STR:             val = sd->status.str; break;
		case SP_AGI:             val = sd->status.agi; break;
		case SP_VIT:             val = sd->status.vit; break;
		case SP_INT:             val = sd->status.int_; break;
		case SP_DEX:             val = sd->status.dex; break;
		case SP_LUK:             val = sd->status.luk; break;
		case SP_KARMA:           val = sd->status.karma; break;
		case SP_MANNER:          val = sd->status.manner; break;
		case SP_FAME:            val = sd->status.fame; break;
		case SP_KILLERRID:       val = sd->killerrid; break;
		case SP_KILLEDRID:       val = sd->killedrid; break;
		case SP_SITTING:         val = pc_issit(sd)?1:0; break;
		case SP_CHARMOVE:		 val = sd->status.character_moves; break;
		case SP_CHARRENAME:		 val = sd->status.rename; break;
		case SP_CHARFONT:		 val = sd->status.font; break;
		case SP_BANK_VAULT:      val = sd->bank_vault; break;
		case SP_ROULETTE_BRONZE: val = sd->roulette_point.bronze; break;
		case SP_ROULETTE_SILVER: val = sd->roulette_point.silver; break;
		case SP_ROULETTE_GOLD:   val = sd->roulette_point.gold; break;
		case SP_CRITICAL:        val = sd->battle_status.cri/10; break;
		case SP_ASPD:            val = (2000-sd->battle_status.amotion)/10; break;
		case SP_BASE_ATK:	     val = sd->battle_status.batk; break;
		case SP_DEF1:		     val = sd->battle_status.def; break;
		case SP_DEF2:		     val = sd->battle_status.def2; break;
		case SP_MDEF1:		     val = sd->battle_status.mdef; break;
		case SP_MDEF2:		     val = sd->battle_status.mdef2; break;
		case SP_HIT:		     val = sd->battle_status.hit; break;
		case SP_FLEE1:		     val = sd->battle_status.flee; break;
		case SP_FLEE2:		     val = sd->battle_status.flee2; break;
		case SP_DEFELE:		     val = sd->battle_status.def_ele; break;
		case SP_MAXHPRATE:	     val = sd->hprate; break;
		case SP_MAXSPRATE:	     val = sd->sprate; break;
		case SP_SPRATE:		     val = sd->dsprate; break;
		case SP_SPEED_RATE:	     val = sd->bonus.speed_rate; break;
		case SP_SPEED_ADDRATE:   val = sd->bonus.speed_add_rate; break;
		case SP_ASPD_RATE:
#ifndef RENEWAL_ASPD
			val = sd->battle_status.aspd_rate;
#else
			val = sd->battle_status.aspd_rate2;
#endif
			break;
		case SP_HP_RECOV_RATE:   val = sd->hprecov_rate; break;
		case SP_SP_RECOV_RATE:   val = sd->sprecov_rate; break;
		case SP_CRITICAL_DEF:    val = sd->bonus.critical_def; break;
		case SP_NEAR_ATK_DEF:    val = sd->bonus.near_attack_def_rate; break;
		case SP_LONG_ATK_DEF:    val = sd->bonus.long_attack_def_rate; break;
		case SP_DOUBLE_RATE:     val = sd->bonus.double_rate; break;
		case SP_DOUBLE_ADD_RATE: val = sd->bonus.double_add_rate; break;
		case SP_MATK_RATE:       val = sd->matk_rate; break;
		case SP_ATK_RATE:        val = sd->bonus.atk_rate; break;
		case SP_MAGIC_ATK_DEF:   val = sd->bonus.magic_def_rate; break;
		case SP_MISC_ATK_DEF:    val = sd->bonus.misc_def_rate; break;
		case SP_PERFECT_HIT_RATE:val = sd->bonus.perfect_hit; break;
		case SP_PERFECT_HIT_ADD_RATE: val = sd->bonus.perfect_hit_add; break;
		case SP_CRITICAL_RATE:   val = sd->critical_rate; break;
		case SP_HIT_RATE:        val = sd->hit_rate; break;
		case SP_FLEE_RATE:       val = sd->flee_rate; break;
		case SP_FLEE2_RATE:      val = sd->flee2_rate; break;
		case SP_DEF_RATE:        val = sd->def_rate; break;
		case SP_DEF2_RATE:       val = sd->def2_rate; break;
		case SP_MDEF_RATE:       val = sd->mdef_rate; break;
		case SP_MDEF2_RATE:      val = sd->mdef2_rate; break;
		case SP_RESTART_FULL_RECOVER: val = sd->special_state.restart_full_recover?1:0; break;
		case SP_NO_CASTCANCEL:   val = sd->special_state.no_castcancel?1:0; break;
		case SP_NO_CASTCANCEL2:  val = sd->special_state.no_castcancel2?1:0; break;
		case SP_NO_SIZEFIX:      val = sd->special_state.no_sizefix?1:0; break;
		case SP_NO_MAGIC_DAMAGE: val = sd->special_state.no_magic_damage; break;
		case SP_NO_WEAPON_DAMAGE:val = sd->special_state.no_weapon_damage; break;
		case SP_NO_MISC_DAMAGE:  val = sd->special_state.no_misc_damage; break;
		case SP_NO_GEMSTONE:     val = sd->special_state.no_gemstone?1:0; break;
		case SP_INTRAVISION:     val = sd->special_state.intravision?1:0; break;
		case SP_NO_KNOCKBACK:    val = sd->special_state.no_knockback?1:0; break;
		case SP_SPLASH_RANGE:    val = sd->bonus.splash_range; break;
		case SP_SPLASH_ADD_RANGE:val = sd->bonus.splash_add_range; break;
		case SP_SHORT_WEAPON_DAMAGE_RETURN: val = sd->bonus.short_weapon_damage_return; break;
		case SP_LONG_WEAPON_DAMAGE_RETURN: val = sd->bonus.long_weapon_damage_return; break;
		case SP_MAGIC_DAMAGE_RETURN: val = sd->bonus.magic_damage_return; break;
		case SP_PERFECT_HIDE:    val = sd->special_state.perfect_hiding?1:0; break;
		case SP_UNBREAKABLE:     val = sd->bonus.unbreakable; break;
		case SP_UNBREAKABLE_WEAPON: val = (sd->bonus.unbreakable_equip&EQP_WEAPON)?1:0; break;
		case SP_UNBREAKABLE_ARMOR: val = (sd->bonus.unbreakable_equip&EQP_ARMOR)?1:0; break;
		case SP_UNBREAKABLE_HELM: val = (sd->bonus.unbreakable_equip&EQP_HELM)?1:0; break;
		case SP_UNBREAKABLE_SHIELD: val = (sd->bonus.unbreakable_equip&EQP_SHIELD)?1:0; break;
		case SP_UNBREAKABLE_GARMENT: val = (sd->bonus.unbreakable_equip&EQP_GARMENT)?1:0; break;
		case SP_UNBREAKABLE_SHOES: val = (sd->bonus.unbreakable_equip&EQP_SHOES)?1:0; break;
		case SP_CLASSCHANGE:     val = sd->bonus.classchange; break;
		case SP_LONG_ATK_RATE:   val = sd->bonus.long_attack_atk_rate; break;
		case SP_BREAK_WEAPON_RATE: val = sd->bonus.break_weapon_rate; break;
		case SP_BREAK_ARMOR_RATE: val = sd->bonus.break_armor_rate; break;
		case SP_ADD_STEAL_RATE:  val = sd->bonus.add_steal_rate; break;
		case SP_DELAYRATE:       val = sd->delayrate; break;
		case SP_CRIT_ATK_RATE:   val = sd->bonus.crit_atk_rate; break;
		case SP_UNSTRIPABLE_WEAPON: val = (sd->bonus.unstripable_equip&EQP_WEAPON)?1:0; break;
		case SP_UNSTRIPABLE:
		case SP_UNSTRIPABLE_ARMOR:
			val = (sd->bonus.unstripable_equip&EQP_ARMOR)?1:0;
			break;
		case SP_UNSTRIPABLE_HELM: val = (sd->bonus.unstripable_equip&EQP_HELM)?1:0; break;
		case SP_UNSTRIPABLE_SHIELD: val = (sd->bonus.unstripable_equip&EQP_SHIELD)?1:0; break;
		case SP_SP_GAIN_VALUE:   val = sd->bonus.sp_gain_value; break;
		case SP_HP_GAIN_VALUE:   val = sd->bonus.hp_gain_value; break;
		case SP_MAGIC_SP_GAIN_VALUE: val = sd->bonus.magic_sp_gain_value; break;
		case SP_MAGIC_HP_GAIN_VALUE: val = sd->bonus.magic_hp_gain_value; break;
		case SP_ADD_HEAL_RATE:   val = sd->bonus.add_heal_rate; break;
		case SP_ADD_HEAL2_RATE:  val = sd->bonus.add_heal2_rate; break;
		case SP_ADD_ITEM_HEAL_RATE: val = sd->bonus.itemhealrate2; break;
		case SP_EMATK:           val = sd->bonus.ematk; break;
		case SP_FIXCASTRATE:     val = sd->bonus.fixcastrate; break;
		case SP_ADD_FIXEDCAST:   val = sd->bonus.add_fixcast; break;
		case SP_ADD_VARIABLECAST:  val = sd->bonus.add_varcast; break;
		case SP_CASTRATE:
		case SP_VARCASTRATE:
#ifdef RENEWAL_CAST
			val = sd->bonus.varcastrate; break;
#else
			val = sd->castrate; break;
#endif
		default:
			ShowError("pc_readparam: Attempt to read unknown parameter '%d'.\n", type);
			return -1;
	}

	return val;
}

/*==========================================
 * script set pc status registry
 *------------------------------------------*/
bool pc_setparam(struct map_session_data *sd,int type,int val)
{
	nullpo_retr(false,sd);

	switch(type){
	case SP_BASELEVEL:
		if ((unsigned int)val > pc_maxbaselv(sd)) //Capping to max
			val = pc_maxbaselv(sd);
		if ((unsigned int)val > sd->status.base_level) {
			int i = 0;
			int stat=0;
			for (i = 0; i < (int)((unsigned int)val - sd->status.base_level); i++)
				stat += pc_gets_status_point(sd->status.base_level + i);
			sd->status.status_point += stat;
		}
		sd->status.base_level = (unsigned int)val;
		sd->status.base_exp = 0;
		// clif_updatestatus(sd, SP_BASELEVEL);  // Gets updated at the bottom
		clif_updatestatus(sd, SP_NEXTBASEEXP);
		clif_updatestatus(sd, SP_STATUSPOINT);
		clif_updatestatus(sd, SP_BASEEXP);
		status_calc_pc(sd, SCO_FORCE);
		if(sd->status.party_id)
			party_send_levelup(sd);
		break;
	case SP_JOBLEVEL:
		if ((unsigned int)val >= sd->status.job_level) {
			if ((unsigned int)val > pc_maxjoblv(sd)) val = pc_maxjoblv(sd);
			sd->status.skill_point += val - sd->status.job_level;
			clif_updatestatus(sd, SP_SKILLPOINT);
		}
		sd->status.job_level = (unsigned int)val;
		sd->status.job_exp = 0;
		// clif_updatestatus(sd, SP_JOBLEVEL);  // Gets updated at the bottom
		clif_updatestatus(sd, SP_NEXTJOBEXP);
		clif_updatestatus(sd, SP_JOBEXP);
		status_calc_pc(sd, SCO_FORCE);
		break;
	case SP_SKILLPOINT:
		sd->status.skill_point = val;
		break;
	case SP_STATUSPOINT:
		sd->status.status_point = val;
		break;
	case SP_ZENY:
		if( val < 0 )
			return false;// can't set negative zeny
		log_zeny(sd, LOG_TYPE_SCRIPT, sd, -(sd->status.zeny - cap_value(val, 0, MAX_ZENY)));
		sd->status.zeny = cap_value(val, 0, MAX_ZENY);
		break;
	case SP_BASEEXP:
		if(pc_nextbaseexp(sd) > 0) {
			sd->status.base_exp = val;
			pc_checkbaselevelup(sd);
		}
		break;
	case SP_JOBEXP:
		if(pc_nextjobexp(sd) > 0) {
			sd->status.job_exp = val;
			pc_checkjoblevelup(sd);
		}
		break;
	case SP_SEX:
		sd->status.sex = val ? SEX_MALE : SEX_FEMALE;
		break;
	case SP_WEIGHT:
		sd->weight = val;
		break;
	case SP_MAXWEIGHT:
		sd->max_weight = val;
		break;
	case SP_HP:
		sd->battle_status.hp = cap_value(val, 1, (int)sd->battle_status.max_hp);
		break;
	case SP_MAXHP:
		sd->battle_status.max_hp = cap_value(val, 1, battle_config.max_hp);

		if( sd->battle_status.max_hp < sd->battle_status.hp )
		{
			sd->battle_status.hp = sd->battle_status.max_hp;
			clif_updatestatus(sd, SP_HP);
		}
		break;
	case SP_SP:
		sd->battle_status.sp = cap_value(val, 0, (int)sd->battle_status.max_sp);
		break;
	case SP_MAXSP:
		sd->battle_status.max_sp = cap_value(val, 1, battle_config.max_sp);

		if( sd->battle_status.max_sp < sd->battle_status.sp )
		{
			sd->battle_status.sp = sd->battle_status.max_sp;
			clif_updatestatus(sd, SP_SP);
		}
		break;
	case SP_STR:
		sd->status.str = cap_value(val, 1, pc_maxparameter(sd,PARAM_STR));
		break;
	case SP_AGI:
		sd->status.agi = cap_value(val, 1, pc_maxparameter(sd,PARAM_AGI));
		break;
	case SP_VIT:
		sd->status.vit = cap_value(val, 1, pc_maxparameter(sd,PARAM_VIT));
		break;
	case SP_INT:
		sd->status.int_ = cap_value(val, 1, pc_maxparameter(sd,PARAM_INT));
		break;
	case SP_DEX:
		sd->status.dex = cap_value(val, 1, pc_maxparameter(sd,PARAM_DEX));
		break;
	case SP_LUK:
		sd->status.luk = cap_value(val, 1, pc_maxparameter(sd,PARAM_LUK));
		break;
	case SP_KARMA:
		sd->status.karma = val;
		break;
	case SP_MANNER:
		sd->status.manner = val;
		if( val < 0 )
			sc_start(NULL, &sd->bl, SC_NOCHAT, 100, 0, 0);
		else {
			status_change_end(&sd->bl, SC_NOCHAT, INVALID_TIMER);
			clif_manner_message(sd, 5);
		}
		return true; // status_change_start/status_change_end already sends packets warning the client
	case SP_FAME:
		sd->status.fame = val;
		break;
	case SP_KILLERRID:
		sd->killerrid = val;
		return true;
	case SP_KILLEDRID:
		sd->killedrid = val;
		return true;
	case SP_CHARMOVE:
		sd->status.character_moves = val;
		return true;
	case SP_CHARRENAME:	
		sd->status.rename = val;
		return true;
	case SP_CHARFONT:
		sd->status.font = val;
		clif_font(sd);
		return true;
	case SP_BANK_VAULT:
		if (val < 0)
			return false;
		log_zeny(sd, LOG_TYPE_BANK, sd, -(sd->bank_vault - cap_value(val, 0, MAX_BANK_ZENY)));
		sd->bank_vault = cap_value(val, 0, MAX_BANK_ZENY);
		pc_setreg2(sd, BANK_VAULT_VAR, sd->bank_vault);
		return true;
	case SP_ROULETTE_BRONZE:
		sd->roulette_point.bronze = val;
		pc_setreg2(sd, ROULETTE_BRONZE_VAR, sd->roulette_point.bronze);
		return true;
	case SP_ROULETTE_SILVER:
		sd->roulette_point.silver = val;
		pc_setreg2(sd, ROULETTE_SILVER_VAR, sd->roulette_point.silver);
		return true;
	case SP_ROULETTE_GOLD:
		sd->roulette_point.gold = val;
		pc_setreg2(sd, ROULETTE_GOLD_VAR, sd->roulette_point.gold);
		return true;
	default:
		ShowError("pc_setparam: Attempted to set unknown parameter '%d'.\n", type);
		return false;
	}
	clif_updatestatus(sd,type);

	return true;
}

/*==========================================
 * HP/SP Healing. If flag is passed, the heal type is through clif_heal, otherwise update status.
 *------------------------------------------*/
void pc_heal(struct map_session_data *sd,unsigned int hp,unsigned int sp, int type)
{
	if (type) {
		if (hp)
			clif_heal(sd->fd,SP_HP,hp);
		if (sp)
			clif_heal(sd->fd,SP_SP,sp);
	} else {
		if(hp)
			clif_updatestatus(sd,SP_HP);
		if(sp)
			clif_updatestatus(sd,SP_SP);
	}
	return;
}

/*==========================================
 * HP/SP Recovery
 * Heal player hp and/or sp linearly.
 * Calculate bonus by status.
 *------------------------------------------*/
int pc_itemheal(struct map_session_data *sd,int itemid, int hp,int sp)
{
	int bonus, tmp;

	if(hp) {
		int i;
		bonus = 100 + (sd->battle_status.vit<<1)
			+ pc_checkskill(sd,SM_RECOVERY)*10
			+ pc_checkskill(sd,AM_LEARNINGPOTION)*5;
		// A potion produced by an Alchemist in the Fame Top 10 gets +50% effect [DracoRPG]
		if (potion_flag > 1)
			bonus += bonus*(potion_flag-1)*50/100;
		//All item bonuses.
		bonus += sd->bonus.itemhealrate2;
		//Item Group bonuses
		bonus += bonus*pc_get_itemgroup_bonus(sd, itemid)/100;
		//Individual item bonuses.
		for(i = 0; i < ARRAYLENGTH(sd->itemhealrate) && sd->itemhealrate[i].nameid; i++)
		{
			if (sd->itemhealrate[i].nameid == itemid) {
				bonus += bonus*sd->itemhealrate[i].rate/100;
				break;
			}
		}

		tmp = hp * bonus / 100;  // overflow check
		if(bonus != 100 && tmp > hp)
			hp = tmp;

		// Recovery Potion
		if( sd->sc.data[SC_INCHEALRATE] )
			hp += (int)(hp * sd->sc.data[SC_INCHEALRATE]->val1/100.);
		// 2014 Halloween Event : Pumpkin Bonus
		if(sd->sc.data[SC_MTF_PUMPKIN] && itemid == ITEMID_PUMPKIN)
			hp += (int)(hp * sd->sc.data[SC_MTF_PUMPKIN]->val1 / 100.);
	}
	if(sp) {
		bonus = 100 + (sd->battle_status.int_<<1)
			+ pc_checkskill(sd,MG_SRECOVERY)*10
			+ pc_checkskill(sd,AM_LEARNINGPOTION)*5;
		if (potion_flag > 1)
			bonus += bonus*(potion_flag-1)*50/100;

		tmp = sp * bonus / 100;
		if(bonus != 100 && tmp > sp)
			sp = tmp;
	}
	if( sd->sc.count ) {
		if ( sd->sc.data[SC_CRITICALWOUND] ) {
			hp -= hp * sd->sc.data[SC_CRITICALWOUND]->val2 / 100;
			sp -= sp * sd->sc.data[SC_CRITICALWOUND]->val2 / 100;
		}

		if ( sd->sc.data[SC_DEATHHURT] ) {
			hp -= hp * 20 / 100;
			sp -= sp * 20 / 100;
		}

		if( sd->sc.data[SC_VITALITYACTIVATION] ){
			hp += hp / 2; // 1.5 times
			sp -= sp / 2;
		}

		if( sd->sc.data[SC_WATER_INSIGNIA] && sd->sc.data[SC_WATER_INSIGNIA]->val1 == 2 ) {
			hp += hp / 10;
			sp += sp / 10;
		}
#ifdef RENEWAL
		if( sd->sc.data[SC_EXTREMITYFIST2] )
			sp = 0;
#endif
	}

	return status_heal(&sd->bl, hp, sp, 1);
}

/*==========================================
 * HP/SP Recovery
 * Heal player hp nad/or sp by rate
 *------------------------------------------*/
int pc_percentheal(struct map_session_data *sd,int hp,int sp)
{
	nullpo_ret(sd);

	if (hp > 100) hp = 100;
	else if (hp <-100) hp = -100;

	if (sp > 100) sp = 100;
	else if (sp <-100) sp = -100;

	if(hp >= 0 && sp >= 0) //Heal
		return status_percent_heal(&sd->bl, hp, sp);

	if(hp <= 0 && sp <= 0) //Damage (negative rates indicate % of max rather than current), and only kill target IF the specified amount is 100%
		return status_percent_damage(NULL, &sd->bl, hp, sp, hp==-100);

	//Crossed signs
	if(hp) {
		if(hp > 0)
			status_percent_heal(&sd->bl, hp, 0);
		else
			status_percent_damage(NULL, &sd->bl, hp, 0, hp==-100);
	}

	if(sp) {
		if(sp > 0)
			status_percent_heal(&sd->bl, 0, sp);
		else
			status_percent_damage(NULL, &sd->bl, 0, sp, false);
	}
	return 0;
}

static int jobchange_killclone(struct block_list *bl, va_list ap)
{
	struct mob_data *md;
		int flag;
	md = (struct mob_data *)bl;
	nullpo_ret(md);
	flag = va_arg(ap, int);

	if (md->master_id && md->special_state.clone && md->master_id == flag)
		status_kill(&md->bl);
	return 1;
}

/**
 * Called when player changes job
 * Rewrote to make it tidider [Celest]
 * @param sd
 * @param job JOB ID. See enum e_job
 * @param upper 1 - JOBL_UPPER; 2 - JOBL_BABY
 * @return True if success, false if failed
 **/
bool pc_jobchange(struct map_session_data *sd,int job, char upper)
{
	int i, fame_flag = 0;
	int b_class;

	nullpo_retr(false,sd);

	if (job < 0)
		return false;

	//Normalize job.
	b_class = pc_jobid2mapid(job);
	if (b_class == -1)
		return false;
	switch (upper) {
		case 1:
			b_class|= JOBL_UPPER;
			break;
		case 2:
			b_class|= JOBL_BABY;
			break;
	}
	//This will automatically adjust bard/dancer classes to the correct gender
	//That is, if you try to jobchange into dancer, it will turn you to bard.
	job = pc_mapid2jobid(b_class, sd->status.sex);
	if (job == -1)
		return false;

	if ((unsigned short)b_class == sd->class_)
		return false; //Nothing to change.

	// changing from 1st to 2nd job
	if ((b_class&JOBL_2) && !(sd->class_&JOBL_2) && (sd->class_&MAPID_UPPERMASK) != MAPID_SUPER_NOVICE) {
		sd->change_level_2nd = sd->status.job_level;
		pc_setglobalreg(sd, add_str("jobchange_level"), sd->change_level_2nd);
	}
	// changing from 2nd to 3rd job
	else if((b_class&JOBL_THIRD) && !(sd->class_&JOBL_THIRD)) {
		sd->change_level_3rd = sd->status.job_level;
		pc_setglobalreg(sd, add_str("jobchange_level_3rd"), sd->change_level_3rd);
	}

	if(sd->cloneskill_idx > 0) {
		if( sd->status.skill[sd->cloneskill_idx].flag == SKILL_FLAG_PLAGIARIZED ) {
			sd->status.skill[sd->cloneskill_idx].id = 0;
			sd->status.skill[sd->cloneskill_idx].lv = 0;
			sd->status.skill[sd->cloneskill_idx].flag = SKILL_FLAG_PERMANENT;
			clif_deleteskill(sd,pc_readglobalreg(sd, add_str(SKILL_VAR_PLAGIARISM)));
		}
		sd->cloneskill_idx = 0;
		pc_setglobalreg(sd, add_str(SKILL_VAR_PLAGIARISM), 0);
		pc_setglobalreg(sd, add_str(SKILL_VAR_PLAGIARISM_LV), 0);
	}

	if(sd->reproduceskill_idx > 0) {
		if( sd->status.skill[sd->reproduceskill_idx].flag == SKILL_FLAG_PLAGIARIZED ) {
			sd->status.skill[sd->reproduceskill_idx].id = 0;
			sd->status.skill[sd->reproduceskill_idx].lv = 0;
			sd->status.skill[sd->reproduceskill_idx].flag = SKILL_FLAG_PERMANENT;
			clif_deleteskill(sd,pc_readglobalreg(sd, add_str(SKILL_VAR_REPRODUCE)));
		}
		sd->reproduceskill_idx = 0;
		pc_setglobalreg(sd, add_str(SKILL_VAR_REPRODUCE), 0);
		pc_setglobalreg(sd, add_str(SKILL_VAR_REPRODUCE_LV), 0);
	}

	// Give or reduce transcendent status points
	if( (b_class&JOBL_UPPER) && !(sd->class_&JOBL_UPPER) ){ // Change from a non t class to a t class -> give points
		sd->status.status_point += battle_config.transcendent_status_points;
		clif_updatestatus(sd,SP_STATUSPOINT);
	}else if( !(b_class&JOBL_UPPER) && (sd->class_&JOBL_UPPER) ){ // Change from a t class to a non t class -> remove points
		if( sd->status.status_point < battle_config.transcendent_status_points ){
			// The player already used his bonus points, so we have to reset his status points
			pc_resetstate(sd);
		}
		sd->status.status_point -= battle_config.transcendent_status_points;
		clif_updatestatus(sd,SP_STATUSPOINT);
	}

	if ( (b_class&MAPID_UPPERMASK) != (sd->class_&MAPID_UPPERMASK) ) { //Things to remove when changing class tree.
		const int class_ = pc_class2idx(sd->status.class_);
		short id;
		for(i = 0; i < MAX_SKILL_TREE && (id = skill_tree[class_][i].id) > 0; i++) {
			//Remove status specific to your current tree skills.
			enum sc_type sc = status_skill2sc(id);
			if (sc > SC_COMMON_MAX && sd->sc.data[sc])
				status_change_end(&sd->bl, sc, INVALID_TIMER);
		}
	}

	if( (sd->class_&MAPID_UPPERMASK) == MAPID_STAR_GLADIATOR && (b_class&MAPID_UPPERMASK) != MAPID_STAR_GLADIATOR) {
		/* going off star glad lineage, reset feel to not store no-longer-used vars in the database */
		pc_resetfeel(sd);
	}

	sd->status.class_ = job;
	fame_flag = pc_famerank(sd->status.char_id,sd->class_&MAPID_UPPERMASK);
	sd->class_ = (unsigned short)b_class;
	sd->status.job_level=1;
	sd->status.job_exp=0;

	if (sd->status.base_level > pc_maxbaselv(sd)) {
		sd->status.base_level = pc_maxbaselv(sd);
		sd->status.base_exp=0;
		pc_resetstate(sd);
		clif_updatestatus(sd,SP_STATUSPOINT);
		clif_updatestatus(sd,SP_BASELEVEL);
		clif_updatestatus(sd,SP_BASEEXP);
		clif_updatestatus(sd,SP_NEXTBASEEXP);
	}

	clif_updatestatus(sd,SP_JOBLEVEL);
	clif_updatestatus(sd,SP_JOBEXP);
	clif_updatestatus(sd,SP_NEXTJOBEXP);

	for(i=0;i<EQI_MAX;i++) {
		if(sd->equip_index[i] >= 0)
			if(pc_isequip(sd,sd->equip_index[i]))
				pc_unequipitem(sd,sd->equip_index[i],2);	// unequip invalid item for class
	}

	//Change look, if disguised, you need to undisguise
	//to correctly calculate new job sprite without
	if (sd->disguise)
		pc_disguise(sd, 0);

	status_set_viewdata(&sd->bl, job);
	clif_changelook(&sd->bl,LOOK_BASE,sd->vd.class_); // move sprite update to prevent client crashes with incompatible equipment [Valaris]
	if(sd->vd.cloth_color)
		clif_changelook(&sd->bl,LOOK_CLOTHES_COLOR,sd->vd.cloth_color);

	//Update skill tree.
	pc_calc_skilltree(sd);
	clif_skillinfoblock(sd);

	if (sd->ed)
		elemental_delete(sd->ed, 0);
	if (sd->state.vending)
		vending_closevending(sd);
	if (sd->state.buyingstore)
		buyingstore_close(sd);

	map_foreachinmap(jobchange_killclone, sd->bl.m, BL_MOB, sd->bl.id);

	//Remove peco/cart/falcon
	i = sd->sc.option;
	if( i&OPTION_RIDING && !pc_checkskill(sd, KN_RIDING) )
		i&=~OPTION_RIDING;
	if( i&OPTION_FALCON && !pc_checkskill(sd, HT_FALCON) )
		i&=~OPTION_FALCON;
	if( i&OPTION_DRAGON && !pc_checkskill(sd,RK_DRAGONTRAINING) )
		i&=~OPTION_DRAGON;
	if( i&OPTION_WUGRIDER && !pc_checkskill(sd,RA_WUGMASTERY) )
		i&=~OPTION_WUGRIDER;
	if( i&OPTION_WUG && !pc_checkskill(sd,RA_WUGMASTERY) )
		i&=~OPTION_WUG;
	if( i&OPTION_MADOGEAR ) //You do not need a skill for this.
		i&=~OPTION_MADOGEAR;
#ifndef NEW_CARTS
	if( i&OPTION_CART && !pc_checkskill(sd, MC_PUSHCART) )
		i&=~OPTION_CART;
#else
	if( sd->sc.data[SC_PUSH_CART] && !pc_checkskill(sd, MC_PUSHCART) )
		pc_setcart(sd, 0);
#endif
	if(i != sd->sc.option)
		pc_setoption(sd, i);

	if(hom_is_active(sd->hd) && !pc_checkskill(sd, AM_CALLHOMUN))
		hom_vaporize(sd, HOM_ST_ACTIVE);

	if(sd->status.manner < 0)
		clif_changestatus(sd,SP_MANNER,sd->status.manner);

	status_calc_pc(sd,SCO_FORCE);
	pc_checkallowskill(sd);
	pc_equiplookall(sd);

	//if you were previously famous, not anymore.
	if (fame_flag) {
		chrif_save(sd,0);
		chrif_buildfamelist();
	} else if (sd->status.fame > 0) {
		//It may be that now they are famous?
 		switch (sd->class_&MAPID_UPPERMASK) {
			case MAPID_BLACKSMITH:
			case MAPID_ALCHEMIST:
			case MAPID_TAEKWON:
				chrif_save(sd,0);
				chrif_buildfamelist();
			break;
		}
	}

	return true;
}

/*==========================================
 * Tell client player sd has change equipement
 *------------------------------------------*/
void pc_equiplookall(struct map_session_data *sd)
{
	nullpo_retv(sd);

	clif_changelook(&sd->bl,LOOK_WEAPON,0);
	clif_changelook(&sd->bl,LOOK_SHOES,0);
	clif_changelook(&sd->bl,LOOK_HEAD_BOTTOM,sd->status.head_bottom);
	clif_changelook(&sd->bl,LOOK_HEAD_TOP,sd->status.head_top);
	clif_changelook(&sd->bl,LOOK_HEAD_MID,sd->status.head_mid);
	clif_changelook(&sd->bl,LOOK_ROBE, sd->status.robe);
}

/*==========================================
 * Tell client player sd has change look (hair,equip...)
 *------------------------------------------*/
void pc_changelook(struct map_session_data *sd,int type,int val)
{
	nullpo_retv(sd);

	switch(type){
	case LOOK_HAIR:	//Use the battle_config limits! [Skotlex]
		val = cap_value(val, MIN_HAIR_STYLE, MAX_HAIR_STYLE);

		if (sd->status.hair != val)
		{
			sd->status.hair=val;
			if (sd->status.guild_id) //Update Guild Window. [Skotlex]
				intif_guild_change_memberinfo(sd->status.guild_id,sd->status.account_id,sd->status.char_id,
				GMI_HAIR,&sd->status.hair,sizeof(sd->status.hair));
		}
		break;
	case LOOK_WEAPON:
		sd->status.weapon=val;
		break;
	case LOOK_HEAD_BOTTOM:
		sd->status.head_bottom=val;
		break;
	case LOOK_HEAD_TOP:
		sd->status.head_top=val;
		break;
	case LOOK_HEAD_MID:
		sd->status.head_mid=val;
		break;
	case LOOK_HAIR_COLOR:	//Use the battle_config limits! [Skotlex]
		val = cap_value(val, MIN_HAIR_COLOR, MAX_HAIR_COLOR);

		if (sd->status.hair_color != val) {
			sd->status.hair_color=val;
			if (sd->status.guild_id) //Update Guild Window. [Skotlex]
				intif_guild_change_memberinfo(sd->status.guild_id,sd->status.account_id,sd->status.char_id,
				GMI_HAIR_COLOR,&sd->status.hair_color,sizeof(sd->status.hair_color));
		}
		break;
	case LOOK_CLOTHES_COLOR:	//Use the battle_config limits! [Skotlex]
		val = cap_value(val, MIN_CLOTH_COLOR, MAX_CLOTH_COLOR);

		sd->status.clothes_color=val;
		break;
	case LOOK_SHIELD:
		sd->status.shield=val;
		break;
	case LOOK_SHOES:
		break;
	case LOOK_ROBE:
		sd->status.robe = val;
		break;
	}
	clif_changelook(&sd->bl,type,val);
}

/*==========================================
 * Give an option (type) to player (sd) and display it to client
 *------------------------------------------*/
void pc_setoption(struct map_session_data *sd,int type)
{
	int p_type, new_look=0;
	nullpo_retv(sd);
	p_type = sd->sc.option;

	//Option has to be changed client-side before the class sprite or it won't always work (eg: Wedding sprite) [Skotlex]
	sd->sc.option=type;
	clif_changeoption(&sd->bl);

	if( (type&OPTION_RIDING && !(p_type&OPTION_RIDING)) || (type&OPTION_DRAGON && !(p_type&OPTION_DRAGON) && pc_checkskill(sd,RK_DRAGONTRAINING) > 0) )
	{ // Mounting
		clif_status_load(&sd->bl,SI_RIDING,1);
		status_calc_pc(sd,SCO_NONE);
	}
	else if( (!(type&OPTION_RIDING) && p_type&OPTION_RIDING) || (!(type&OPTION_DRAGON) && p_type&OPTION_DRAGON && pc_checkskill(sd,RK_DRAGONTRAINING) > 0) )
	{ // Dismount
		clif_status_load(&sd->bl,SI_RIDING,0);
		status_calc_pc(sd,SCO_NONE);
	}

#ifndef NEW_CARTS
	if( type&OPTION_CART && !( p_type&OPTION_CART ) ) { //Cart On
		clif_cartlist(sd);
		clif_updatestatus(sd, SP_CARTINFO);
		if(pc_checkskill(sd, MC_PUSHCART) < 10)
			status_calc_pc(sd,SCO_NONE); //Apply speed penalty.
	} else if( !( type&OPTION_CART ) && p_type&OPTION_CART ){ //Cart Off
		clif_clearcart(sd->fd);
		if(pc_checkskill(sd, MC_PUSHCART) < 10)
			status_calc_pc(sd,SCO_NONE); //Remove speed penalty.
	}
#endif

	if (type&OPTION_FALCON && !(p_type&OPTION_FALCON)) //Falcon ON
		clif_status_load(&sd->bl,SI_FALCON,1);
	else if (!(type&OPTION_FALCON) && p_type&OPTION_FALCON) //Falcon OFF
		clif_status_load(&sd->bl,SI_FALCON,0);

	if( (sd->class_&MAPID_THIRDMASK) == MAPID_RANGER ) {
		if( type&OPTION_WUGRIDER && !(p_type&OPTION_WUGRIDER) ) { // Mounting
			clif_status_load(&sd->bl,SI_WUGRIDER,1);
			status_calc_pc(sd,SCO_NONE);
		} else if( !(type&OPTION_WUGRIDER) && p_type&OPTION_WUGRIDER ) { // Dismount
			clif_status_load(&sd->bl,SI_WUGRIDER,0);
			status_calc_pc(sd,SCO_NONE);
		}
	}
	if( (sd->class_&MAPID_THIRDMASK) == MAPID_MECHANIC ) {
		if( type&OPTION_MADOGEAR && !(p_type&OPTION_MADOGEAR) ) {
			status_calc_pc(sd,SCO_NONE);
			status_change_end(&sd->bl,SC_MAXIMIZEPOWER,INVALID_TIMER);
			status_change_end(&sd->bl,SC_OVERTHRUST,INVALID_TIMER);
			status_change_end(&sd->bl,SC_WEAPONPERFECTION,INVALID_TIMER);
			status_change_end(&sd->bl,SC_ADRENALINE,INVALID_TIMER);
			status_change_end(&sd->bl,SC_CARTBOOST,INVALID_TIMER);
			status_change_end(&sd->bl,SC_MELTDOWN,INVALID_TIMER);
			status_change_end(&sd->bl,SC_MAXOVERTHRUST,INVALID_TIMER);
			pc_bonus_script_clear(sd,BSF_REM_ON_MADOGEAR);
		} else if( !(type&OPTION_MADOGEAR) && p_type&OPTION_MADOGEAR ) {
			status_calc_pc(sd,SCO_NONE);
			status_change_end(&sd->bl,SC_SHAPESHIFT,INVALID_TIMER);
			status_change_end(&sd->bl,SC_HOVERING,INVALID_TIMER);
			status_change_end(&sd->bl,SC_ACCELERATION,INVALID_TIMER);
			status_change_end(&sd->bl,SC_OVERHEAT_LIMITPOINT,INVALID_TIMER);
			status_change_end(&sd->bl,SC_OVERHEAT,INVALID_TIMER);
			pc_bonus_script_clear(sd,BSF_REM_ON_MADOGEAR);
		}
	}

	if (type&OPTION_FLYING && !(p_type&OPTION_FLYING))
		new_look = JOB_STAR_GLADIATOR2;
	else if (!(type&OPTION_FLYING) && p_type&OPTION_FLYING)
		new_look = -1;

	if (sd->disguise || !new_look)
		return; //Disguises break sprite changes

	if (new_look < 0) { //Restore normal look.
		status_set_viewdata(&sd->bl, sd->status.class_);
		new_look = sd->vd.class_;
	}

	pc_stop_attack(sd); //Stop attacking on new view change (to prevent wedding/santa attacks.
	clif_changelook(&sd->bl,LOOK_BASE,new_look);
	if (sd->vd.cloth_color)
		clif_changelook(&sd->bl,LOOK_CLOTHES_COLOR,sd->vd.cloth_color);
	clif_skillinfoblock(sd); // Skill list needs to be updated after base change.
}

/*==========================================
 * Give player a cart
 *------------------------------------------*/
bool pc_setcart(struct map_session_data *sd,int type) {
#ifndef NEW_CARTS
	int cart[6] = {0x0000,OPTION_CART1,OPTION_CART2,OPTION_CART3,OPTION_CART4,OPTION_CART5};
	int option;
#endif
	nullpo_retr(false,sd);

	if( type < 0 || type > MAX_CARTS )
		return false;// Never trust the values sent by the client! [Skotlex]

	if( pc_checkskill(sd,MC_PUSHCART) <= 0 && type != 0 )
		return false;// Push cart is required

#ifdef NEW_CARTS

	switch( type ) {
		case 0:
			if( !sd->sc.data[SC_PUSH_CART] )
				return 0;
			status_change_end(&sd->bl,SC_PUSH_CART,INVALID_TIMER);
			clif_clearcart(sd->fd);
			break;
		default:/* everything else is an allowed ID so we can move on */
			if( !sd->sc.data[SC_PUSH_CART] ) /* first time, so fill cart data */
				clif_cartlist(sd);
			clif_updatestatus(sd, SP_CARTINFO);
			sc_start(&sd->bl,&sd->bl, SC_PUSH_CART, 100, type, 0);
			clif_status_change2(&sd->bl, sd->bl.id, AREA, SI_ON_PUSH_CART, type, 0, 0);
			if( sd->sc.data[SC_PUSH_CART] )/* forcefully update */
				sd->sc.data[SC_PUSH_CART]->val1 = type;
			break;
	}

	if(pc_checkskill(sd, MC_PUSHCART) < 10)
		status_calc_pc(sd,SCO_NONE); //Recalc speed penalty.
#else
	// Update option
	option = sd->sc.option;
	option &= ~OPTION_CART;// clear cart bits
	option |= cart[type]; // set cart
	pc_setoption(sd, option);
#endif

	return true;
}

/*==========================================
 * Give player a falcon
 *------------------------------------------*/
void pc_setfalcon(TBL_PC* sd, int flag)
{
	if( flag ){
		if( pc_checkskill(sd,HT_FALCON)>0 )	// add falcon if he have the skill
			pc_setoption(sd,sd->sc.option|OPTION_FALCON);
	} else if( pc_isfalcon(sd) ){
		pc_setoption(sd,sd->sc.option&~OPTION_FALCON); // remove falcon
	}
}

/*==========================================
 *  Set player riding
 *------------------------------------------*/
void pc_setriding(TBL_PC* sd, int flag)
{
	if( &sd->sc && sd->sc.data[SC_ALL_RIDING] )
		return;

	if( flag ){
		if( pc_checkskill(sd,KN_RIDING) > 0 ) // add peco
			pc_setoption(sd, sd->sc.option|OPTION_RIDING);
	} else if( pc_isriding(sd) ){
			pc_setoption(sd, sd->sc.option&~OPTION_RIDING);
	}
}

/*==========================================
 * Give player a mado
 *------------------------------------------*/
void pc_setmadogear(TBL_PC* sd, int flag)
{
	if( flag ){
		if( pc_checkskill(sd,NC_MADOLICENCE) > 0 )
			pc_setoption(sd, sd->sc.option|OPTION_MADOGEAR);
	} else if( pc_ismadogear(sd) ){
			pc_setoption(sd, sd->sc.option&~OPTION_MADOGEAR);
	}
}

/*==========================================
 * Check if player can drop an item
 *------------------------------------------*/
bool pc_candrop(struct map_session_data *sd, struct item *item)
{
	if( item && (item->expire_time || (item->bound && !pc_can_give_bounded_items(sd))) )
		return false;
	if( !pc_can_give_items(sd) || sd->sc.cant.drop) //check if this GM level can drop items
		return false;
	return (itemdb_isdropable(item, pc_get_group_level(sd)));
}

/**
 * Determines whether a player can attack based on status changes
 *  Why not use status_check_skilluse?
 *  "src MAY be null to indicate we shouldn't check it, this is a ground-based skill attack."
 *  Even ground-based attacks should be blocked by these statuses
 * Called from unit_attack and unit_attack_timer_sub
 * @retval true Can attack
 **/
bool pc_can_attack( struct map_session_data *sd, int target_id ) {
	nullpo_retr(false, sd);

	if (!&sd->sc)
		return true;

	if( sd->sc.data[SC_BASILICA] ||
		sd->sc.data[SC__SHADOWFORM] ||
		sd->sc.data[SC__MANHOLE] ||
		sd->sc.data[SC_CURSEDCIRCLE_ATKER] ||
		sd->sc.data[SC_CURSEDCIRCLE_TARGET] ||
		sd->sc.data[SC_CRYSTALIZE] ||
		sd->sc.data[SC_ALL_RIDING] || // The client doesn't let you, this is to make cheat-safe
		sd->sc.data[SC_TRICKDEAD] ||
		(sd->sc.data[SC_VOICEOFSIREN] && sd->sc.data[SC_VOICEOFSIREN]->val2 == target_id) ||
		sd->sc.data[SC_BLADESTOP] ||
		sd->sc.data[SC_DEEPSLEEP] ||
		(sd->sc.data[SC_GRAVITATION] && sd->sc.data[SC_GRAVITATION]->val3 == BCT_SELF) )
			return false;

	return true;
}

/*==========================================
 * Read '@type' variables (temporary numeric char reg)
 *------------------------------------------*/
int pc_readreg(struct map_session_data* sd, int64 reg)
{
	return i64db_iget(sd->regs.vars, reg);
}

/*==========================================
 * Set '@type' variables (temporary numeric char reg)
 *------------------------------------------*/
bool pc_setreg(struct map_session_data* sd, int64 reg, int val)
{
	unsigned int index = script_getvaridx(reg);

	nullpo_retr(false, sd);

	if( val ) {
		i64db_iput(sd->regs.vars, reg, val);
		if( index )
			script_array_update(&sd->regs, reg, false);
	} else {
		i64db_remove(sd->regs.vars, reg);
		if( index )
			script_array_update(&sd->regs, reg, true);
	}

	return true;
}

/*==========================================
 * Read '@type$' variables (temporary string char reg)
 *------------------------------------------*/
char* pc_readregstr(struct map_session_data* sd, int64 reg)
{
	struct script_reg_str *p = NULL;

	p = (struct script_reg_str *)i64db_get(sd->regs.vars, reg);

	return p ? p->value : NULL;
}

/*==========================================
 * Set '@type$' variables (temporary string char reg)
 *------------------------------------------*/
bool pc_setregstr(struct map_session_data* sd, int64 reg, const char* str)
{
	struct script_reg_str *p = NULL;
	unsigned int index = script_getvaridx(reg);
	DBData prev;

	nullpo_retr(false, sd);

	if( str[0] ) {
		p = ers_alloc(str_reg_ers, struct script_reg_str);

		p->value = aStrdup(str);
		p->flag.type = 1;

		if (sd->regs.vars->put(sd->regs.vars, db_i642key(reg), db_ptr2data(p), &prev)) {
			p = (struct script_reg_str *)db_data2ptr(&prev);
			if( p->value )
				aFree(p->value);
			ers_free(str_reg_ers, p);
		} else {
			if( index )
				script_array_update(&sd->regs, reg, false);
		}
	} else {
		if (sd->regs.vars->remove(sd->regs.vars, db_i642key(reg), &prev)) {
			p = (struct script_reg_str *)db_data2ptr(&prev);
			if( p->value )
				aFree(p->value);
			ers_free(str_reg_ers, p);
			if( index )
				script_array_update(&sd->regs, reg, true);
		}
	}

	return true;
}

/**
 * Serves the following variable types:
 * - 'type' (permanent numeric char reg)
 * - '#type' (permanent numeric account reg)
 * - '##type' (permanent numeric account reg2)
 **/
int pc_readregistry(struct map_session_data *sd, int64 reg)
{
	struct script_reg_num *p = NULL;

	if (!sd->vars_ok) {
		ShowError("pc_readregistry: Trying to read reg %s before it's been loaded!\n", get_str(script_getvarid(reg)));
		//This really shouldn't happen, so it's possible the data was lost somewhere, we should request it again.
		//intif->request_registry(sd,type==3?4:type);
		set_eof(sd->fd);
		return 0;
	}

	p = (struct script_reg_num *)i64db_get(sd->regs.vars, reg);

	return p ? p->value : 0;
}

/**
 * Serves the following variable types:
 * - 'type$' (permanent str char reg)
 * - '#type$' (permanent str account reg)
 * - '##type$' (permanent str account reg2)
 **/
char* pc_readregistry_str(struct map_session_data *sd, int64 reg)
{
	struct script_reg_str *p = NULL;

	if (!sd->vars_ok) {
		ShowError("pc_readregistry_str: Trying to read reg %s before it's been loaded!\n", get_str(script_getvarid(reg)));
		//This really shouldn't happen, so it's possible the data was lost somewhere, we should request it again.
		//intif->request_registry(sd,type==3?4:type);
		set_eof(sd->fd);
		return NULL;
	}

	p = (struct script_reg_str *)i64db_get(sd->regs.vars, reg);

	return p ? p->value : NULL;
}

/**
 * Serves the following variable types:
 * - 'type' (permanent numeric char reg)
 * - '#type' (permanent numeric account reg)
 * - '##type' (permanent numeric account reg2)
 **/
int pc_setregistry(struct map_session_data *sd, int64 reg, int val)
{
	struct script_reg_num *p = NULL;
	const char *regname = get_str(script_getvarid(reg));
	unsigned int index = script_getvaridx(reg);

	// These should be stored elsewhere e.g. char ones in char table, the cash ones in account_data table!
	switch( regname[0] ) {
		default: //Char reg
			if( !strcmp(regname,"PC_DIE_COUNTER") && sd->die_counter != val ) {
				int i = (!sd->die_counter && (sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE);
				sd->die_counter = val;
				if( i )
					status_calc_pc(sd,SCO_NONE); // Lost the bonus.
			} else if( !strcmp(regname,"COOK_MASTERY") && sd->cook_mastery != val ) {
				val = cap_value(val, 0, 1999);
				sd->cook_mastery = val;
			}
			break;
		case '#':
			if( !strcmp(regname,"#CASHPOINTS") && sd->cashPoints != val ) {
				val = cap_value(val, 0, MAX_ZENY);
				sd->cashPoints = val;
			} else if( !strcmp(regname,"#KAFRAPOINTS") && sd->kafraPoints != val ) {
				val = cap_value(val, 0, MAX_ZENY);
				sd->kafraPoints = val;
			}
			break;
	}
	
	if ( !reg_load && !sd->vars_ok ) {
		ShowError("pc_setregistry : refusing to set %s until vars are received.\n", regname);
		return 0;
	}

	if ((p = (struct script_reg_num *)i64db_get(sd->regs.vars, reg))) {
		if( val ) {
			if( !p->value && index ) /* its a entry that was deleted, so we reset array */
				script_array_update(&sd->regs, reg, false);
			p->value = val;
		} else {
			p->value = 0;
			if( index )
				script_array_update(&sd->regs, reg, true);
		}
		if (!reg_load)
			p->flag.update = 1;/* either way, it will require either delete or replace */
	} else if( val ) {
		DBData prev;

		if( index )
			script_array_update(&sd->regs, reg, false);

		p = ers_alloc(num_reg_ers, struct script_reg_num);

		p->value = val;
		if (!reg_load)
			p->flag.update = 1;

		if (sd->regs.vars->put(sd->regs.vars, db_i642key(reg), db_ptr2data(p), &prev)) {
			p = (struct script_reg_num *)db_data2ptr(&prev);
			ers_free(num_reg_ers, p);
		}
	}

	if (!reg_load && p)
		sd->vars_dirty = true;

	return 1;
}

/**
 * Serves the following variable types:
 * - 'type$' (permanent str char reg)
 * - '#type$' (permanent str account reg)
 * - '##type$' (permanent str account reg2)
 **/
int pc_setregistry_str(struct map_session_data *sd, int64 reg, const char *val)
{
	struct script_reg_str *p = NULL;
	const char *regname = get_str(script_getvarid(reg));
	unsigned int index = script_getvaridx(reg);

	if (!reg_load && !sd->vars_ok) {
		ShowError("pc_setregistry_str : refusing to set %s until vars are received.\n", regname);
		return 0;
	}

	if( (p = (struct script_reg_str *)i64db_get(sd->regs.vars, reg) ) ) {
		if( val[0] ) {
			if( p->value )
				aFree(p->value);
			else if ( index ) // an entry that was deleted, so we reset
				script_array_update(&sd->regs, reg, false);
			p->value = aStrdup(val);
		} else {
			p->value = NULL;
			if( index )
				script_array_update(&sd->regs, reg, true);
		}
		if( !reg_load )
			p->flag.update = 1; // either way, it will require either delete or replace
	} else if( val[0] ) {
		DBData prev;

		if( index )
			script_array_update(&sd->regs, reg, false);

		p = ers_alloc(str_reg_ers, struct script_reg_str);

		p->value = aStrdup(val);
		if( !reg_load )
			p->flag.update = 1;
		p->flag.type = 1;

		if( sd->regs.vars->put(sd->regs.vars, db_i642key(reg), db_ptr2data(p), &prev) ) {
			p = (struct script_reg_str *)db_data2ptr(&prev);
			if( p->value )
				aFree(p->value);
			ers_free(str_reg_ers, p);
		}
	}

	if( !reg_load && p )
		sd->vars_dirty = true;

	return 1;
}

/**
 * Set value of player variable
 * @param sd Player
 * @param reg Variable name
 * @param value
 * @return True if success, false if failed.
 **/
bool pc_setreg2(struct map_session_data *sd, const char *reg, int val) {
	char prefix = reg[0];

	nullpo_retr(false, sd);

	if (reg[strlen(reg)-1] == '$') {
		ShowError("pc_setreg2: Invalid variable scope '%s'\n", reg);
		return false;
	}

	val = cap_value(val, 0, INT_MAX);

	switch (prefix) {
		case '.':
		case '\'':
		case '$':
			ShowError("pc_setreg2: Invalid variable scope '%s'\n", reg);
			return false;
		case '@':
			return pc_setreg(sd, add_str(reg), val);
		case '#':
			return (reg[1] == '#') ? pc_setaccountreg2(sd, add_str(reg), val) : pc_setaccountreg(sd, add_str(reg), val);
		default:
			return pc_setglobalreg(sd, add_str(reg), val);
	}

	return false;
}

/**
 * Get value of player variable
 * @param sd Player
 * @param reg Variable name
 * @return Variable value or 0 if failed.
 **/
int pc_readreg2(struct map_session_data *sd, const char *reg) {
	char prefix = reg[0];

	nullpo_ret(sd);

	if (reg[strlen(reg)-1] == '$') {
		ShowError("pc_readreg2: Invalid variable scope '%s'\n", reg);
		return 0;
	}

	switch (prefix) {
		case '.':
		case '\'':
		case '$':
			ShowError("pc_readreg2: Invalid variable scope '%s'\n", reg);
			return 0;
		case '@':
			return pc_readreg(sd, add_str(reg));
		case '#':
			return (reg[1] == '#') ? pc_readaccountreg2(sd, add_str(reg)) : pc_readaccountreg(sd, add_str(reg));
		default:
			return pc_readglobalreg(sd, add_str(reg));
	}

	return 0;
}

/*==========================================
 * Exec eventtimer for player sd (retrieved from map_session (id))
 *------------------------------------------*/
static int pc_eventtimer(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd=map_id2sd(id);
	char *p = (char *)data;
	int i;
	if(sd==NULL)
		return 0;

	ARR_FIND( 0, MAX_EVENTTIMER, i, sd->eventtimer[i] == tid );
	if( i < MAX_EVENTTIMER )
	{
		sd->eventtimer[i] = INVALID_TIMER;
		sd->eventcount--;
		npc_event(sd,p,0);
	}
	else
		ShowError("pc_eventtimer: no such event timer\n");

	if (p) aFree(p);
	return 0;
}

/*==========================================
 * Add eventtimer for player sd ?
 *------------------------------------------*/
bool pc_addeventtimer(struct map_session_data *sd,int tick,const char *name)
{
	int i;
	nullpo_retr(false,sd);

	ARR_FIND( 0, MAX_EVENTTIMER, i, sd->eventtimer[i] == INVALID_TIMER );
	if( i == MAX_EVENTTIMER )
		return false;

	sd->eventtimer[i] = add_timer(gettick()+tick, pc_eventtimer, sd->bl.id, (intptr_t)aStrdup(name));
	sd->eventcount++;

	return true;
}

/*==========================================
 * Del eventtimer for player sd ?
 *------------------------------------------*/
bool pc_deleventtimer(struct map_session_data *sd,const char *name)
{
	char* p = NULL;
	int i;

	nullpo_retr(false,sd);
	if (sd->eventcount == 0)
		return false;

	// find the named event timer
	ARR_FIND( 0, MAX_EVENTTIMER, i,
		sd->eventtimer[i] != INVALID_TIMER &&
		(p = (char *)(get_timer(sd->eventtimer[i])->data)) != NULL &&
		strcmp(p, name) == 0
	);
	if( i == MAX_EVENTTIMER )
		return false; // not found

	delete_timer(sd->eventtimer[i],pc_eventtimer);
	sd->eventtimer[i] = INVALID_TIMER;
	if(sd->eventcount > 0)
		sd->eventcount--;
	aFree(p);

	return true;
}

/*==========================================
 * Update eventtimer count for player sd
 *------------------------------------------*/
void pc_addeventtimercount(struct map_session_data *sd,const char *name,int tick)
{
	int i;

	nullpo_retv(sd);

	for(i=0;i<MAX_EVENTTIMER;i++)
		if( sd->eventtimer[i] != INVALID_TIMER && strcmp(
			(char *)(get_timer(sd->eventtimer[i])->data), name)==0 ){
				addtick_timer(sd->eventtimer[i],tick);
				break;
		}
}

/*==========================================
 * Remove all eventtimer for player sd
 *------------------------------------------*/
void pc_cleareventtimer(struct map_session_data *sd)
{
	int i;

	nullpo_retv(sd);

	if (sd->eventcount == 0)
		return;

	for(i=0;i<MAX_EVENTTIMER;i++){
		if( sd->eventtimer[i] != INVALID_TIMER ){
			char *p = (char *)(get_timer(sd->eventtimer[i])->data);
			delete_timer(sd->eventtimer[i],pc_eventtimer);
			sd->eventtimer[i] = INVALID_TIMER;
			if(sd->eventcount > 0) //avoid looping to max val
				sd->eventcount--;
			if (p) aFree(p);
		}
	}
}

/**
* Called when an item with combo is worn
* @param *sd
* @param *data struct item_data
* @return success numbers of succeed combo
*/
static int pc_checkcombo(struct map_session_data *sd, struct item_data *data) {
	uint16 i;
	int success = 0;
	for( i = 0; i < data->combos_count; i++ ) {
		struct itemchk {
			int idx;
			short card[MAX_SLOTS];
		} *combo_idx;
		int idx, j;
		int nb_itemCombo;
		unsigned int pos = 0;
		/* ensure this isn't a duplicate combo */
		if( sd->combos.bonus != NULL ) {
			int x;
			ARR_FIND( 0, sd->combos.count, x, sd->combos.id[x] == data->combos[i]->id );
			/* found a match, skip this combo */
			if( x < sd->combos.count )
				continue;
		}

		nb_itemCombo = data->combos[i]->count;
		if(nb_itemCombo<2) //a combo with less then 2 item ?? how that possible
			continue;
		CREATE(combo_idx,struct itemchk,nb_itemCombo);
		for(j=0; j < nb_itemCombo; j++){
			combo_idx[j].idx=-1;
			memset(combo_idx[j].card,-1,MAX_SLOTS);
		}
			
		for( j = 0; j < nb_itemCombo; j++ ) {
			uint16 id = data->combos[i]->nameid[j], k;
			bool found = false;
			
			for( k = 0; k < EQI_MAX; k++ ) {
				short index = sd->equip_index[k];
				if( index < 0 )
					continue;
				if( pc_is_same_equip_index((enum equip_index)k, sd->equip_index, index) )
					continue;
				if (!sd->inventory_data[index] )
					continue;
				
				if ( itemdb_type(id) != IT_CARD ) {
					if ( sd->inventory_data[index]->nameid != id )
						continue;
					if(j>0){ //check if this item not already used
						bool do_continue = false; //used to continue that specific loop with some check that also use some loop
						uint8 z;
						for (z = 0; z < nb_itemCombo-1; z++)
							if(combo_idx[z].idx == index) //we already have that index recorded
								do_continue=true;
						if(do_continue)
							continue;
					}
					combo_idx[j].idx = index;
					pos |= sd->status.inventory[index].equip;
					found = true;
					break;
				} else { //Cards
					uint16 z;
					if ( sd->inventory_data[index]->slot == 0 || itemdb_isspecial(sd->status.inventory[index].card[0]) )
						continue;
					for (z = 0; z < sd->inventory_data[index]->slot; z++) {
						bool do_continue=false;			
						if (sd->status.inventory[index].card[z] != id)
							continue;
						if(j>0){
							int c1, c2;
							for (c1 = 0; c1 < nb_itemCombo-1; c1++){
								if(combo_idx[c1].idx == index){
									for (c2 = 0; c2 < sd->inventory_data[index]->slot; c2++){
										if(combo_idx[c1].card[c2] == id){ //we already have that card recorded (at this same idx)
											do_continue = true;
											break;
										}
									}
								}
							}
						}
						if(do_continue)
							continue;
						combo_idx[j].idx = index;
						combo_idx[j].card[z] = id;
						pos |= sd->status.inventory[index].equip;
						found = true;
 						break;
 					}
				}
			}
			if( !found )
				break;/* we haven't found all the ids for this combo, so we can return */
		}
		aFree(combo_idx);
		/* means we broke out of the count loop w/o finding all ids, we can move to the next combo */
		if( j < nb_itemCombo )
			continue;
		/* we got here, means all items in the combo are matching */
		idx = sd->combos.count;
		if( sd->combos.bonus == NULL ) {
			CREATE(sd->combos.bonus, struct script_code *, 1);
			CREATE(sd->combos.id, unsigned short, 1);
			CREATE(sd->combos.pos, unsigned int, 1);
			sd->combos.count = 1;
		} else {
			RECREATE(sd->combos.bonus, struct script_code *, ++sd->combos.count);
			RECREATE(sd->combos.id, unsigned short, sd->combos.count);
			RECREATE(sd->combos.pos, unsigned int, sd->combos.count);
		}
		/* we simply copy the pointer */
		sd->combos.bonus[idx] = data->combos[i]->script;
		/* save this combo's id */
		sd->combos.id[idx] = data->combos[i]->id;
		/* save pos of combo*/
		sd->combos.pos[idx] = pos;
		success++;
	}
	return success;
}

/**
* Called when an item with combo is removed
* @param *sd
* @param *data struct item_data
* @return retval numbers of removed combo
*/
static int pc_removecombo(struct map_session_data *sd, struct item_data *data ) {
	int i, retval = 0;

	if( sd->combos.bonus == NULL )
		return 0;/* nothing to do here, player has no combos */
	for( i = 0; i < data->combos_count; i++ ) {
		/* check if this combo exists in this user */
		int x = 0, cursor = 0, j;
		ARR_FIND( 0, sd->combos.count, x, sd->combos.id[x] == data->combos[i]->id );
		/* no match, skip this combo */
		if(x >= sd->combos.count)
			continue;

		sd->combos.bonus[x] = NULL;
		sd->combos.id[x] = 0;
		sd->combos.pos[x] = 0;
		retval++;

		/* check if combo requirements still fit */
		if( pc_checkcombo( sd, data ) )
			continue;

		/* move next value to empty slot */
		for( j = 0, cursor = 0; j < sd->combos.count; j++ ) {
			if( sd->combos.bonus[j] == NULL )
				continue;

			if( cursor != j ) {
				sd->combos.bonus[cursor] = sd->combos.bonus[j];
				sd->combos.id[cursor]    = sd->combos.id[j];
				sd->combos.pos[cursor]   = sd->combos.pos[j];
			}
			cursor++;
		}

		/* it's empty, we can clear all the memory */
		if( (sd->combos.count = cursor) == 0 ) {
			aFree(sd->combos.bonus);
			aFree(sd->combos.id);
			aFree(sd->combos.pos);
			sd->combos.bonus = NULL;
			sd->combos.id = NULL;
			sd->combos.pos = NULL;
			return retval; /* we also can return at this point for we have no more combos to check */
		}
	}

	return retval;
}

/**
* Load combo data(s) of player
* @param *sd
* @return ret numbers of succeed combo
*/
int pc_load_combo(struct map_session_data *sd) {
	int i, ret = 0;
	for( i = 0; i < EQI_MAX; i++ ) {
		struct item_data *id = NULL;
		short idx = sd->equip_index[i];
		if( idx < 0 || !(id = sd->inventory_data[idx] ) )
			continue;
		if( id->combos_count )
			ret += pc_checkcombo(sd,id);
		if(!itemdb_isspecial(sd->status.inventory[idx].card[0])) {
			struct item_data *data;
			int j;
			for( j = 0; j < id->slot; j++ ) {
				if (!sd->status.inventory[idx].card[j])
					continue;
				if ( ( data = itemdb_exists(sd->status.inventory[idx].card[j]) ) != NULL ) {
					if( data->combos_count )
						ret += pc_checkcombo(sd,data);
				}
			}
		}
	}
	return ret;
}
/*==========================================
 * Equip item on player sd at req_pos from inventory index n
 * return: false - fail; true - success
 *------------------------------------------*/
bool pc_equipitem(struct map_session_data *sd,short n,int req_pos)
{
	int i, pos, flag = 0, iflag;
	struct item_data *id;
	uint8 res = ITEM_EQUIP_ACK_OK;

	nullpo_retr(false,sd);

	if( n < 0 || n >= MAX_INVENTORY ) {
		clif_equipitemack(sd,0,0,ITEM_EQUIP_ACK_FAIL);
		return false;
	}
	if( DIFF_TICK(sd->canequip_tick,gettick()) > 0 ) {
		clif_equipitemack(sd,n,0,ITEM_EQUIP_ACK_FAIL);
		return false;
	}

	if (!(id = sd->inventory_data[n]))
		return false;
	pos = pc_equippoint(sd,n); //With a few exceptions, item should go in all specified slots.

	if(battle_config.battle_log)
		ShowInfo("equip %hu(%d) %x:%x\n",sd->status.inventory[n].nameid,n,id?id->equip:0,req_pos);

	if((res = pc_isequip(sd,n))) {
		clif_equipitemack(sd,n,0,res);	// fail
		return false;
	}

	if (!(pos&req_pos) || sd->status.inventory[n].equip != 0 || sd->status.inventory[n].attribute==1 ) { // [Valaris]
		clif_equipitemack(sd,n,0,ITEM_EQUIP_ACK_FAIL);	// fail
		return false;
	}
	if( sd->sc.count && (sd->sc.data[SC_BERSERK] || sd->sc.data[SC_SATURDAYNIGHTFEVER] ||
		sd->sc.data[SC_KYOUGAKU] || (sd->sc.data[SC_PYROCLASTIC] && sd->inventory_data[n]->type == IT_WEAPON)) ) {
		clif_equipitemack(sd,n,0,ITEM_EQUIP_ACK_FAIL); //Fail
		return false;
	}

	if (id->flag.bindOnEquip && !sd->status.inventory[n].bound) {
		sd->status.inventory[n].bound = (char)battle_config.default_bind_on_equip;
		clif_notify_bindOnEquip(sd,n);
	}

	if(pos == EQP_ACC) { //Accesories should only go in one of the two,
		pos = req_pos&EQP_ACC;
		if (pos == EQP_ACC) //User specified both slots..
			pos = sd->equip_index[EQI_ACC_R] >= 0 ? EQP_ACC_L : EQP_ACC_R;
	}

	if(pos == EQP_SHADOW_ACC) { // Shadow System
		pos = req_pos&EQP_SHADOW_ACC;
		if (pos == EQP_SHADOW_ACC)
			pos = sd->equip_index[EQI_SHADOW_ACC_L] >= 0 ? EQP_SHADOW_ACC_R : EQP_SHADOW_ACC_L;
	}

	if(pos == EQP_ARMS && id->equip == EQP_HAND_R) { //Dual wield capable weapon.
		pos = (req_pos&EQP_ARMS);
		if (pos == EQP_ARMS) //User specified both slots, pick one for them.
			pos = sd->equip_index[EQI_HAND_R] >= 0 ? EQP_HAND_L : EQP_HAND_R;
	}

	if (pos&EQP_HAND_R && battle_config.use_weapon_skill_range&BL_PC) {
		//Update skill-block range database when weapon range changes. [Skotlex]
		i = sd->equip_index[EQI_HAND_R];
		if (i < 0 || !sd->inventory_data[i]) //No data, or no weapon equipped
			flag = 1;
		else
			flag = id->range != sd->inventory_data[i]->range;
	}

	for(i=0;i<EQI_MAX;i++) {
		if(pos & equip_pos[i]) {
			if(sd->equip_index[i] >= 0) //Slot taken, remove item from there.
				pc_unequipitem(sd,sd->equip_index[i],2);

			sd->equip_index[i] = n;
		}
	}

	if(pos==EQP_AMMO) {
		clif_arrowequip(sd,n);
		clif_arrow_fail(sd,3);
	}
	else
		clif_equipitemack(sd,n,pos,ITEM_EQUIP_ACK_OK);

	sd->status.inventory[n].equip=pos;

	if(pos & EQP_HAND_R) {
		if(id)
			sd->weapontype1 = id->look;
		else
			sd->weapontype1 = 0;
		pc_calcweapontype(sd);
		clif_changelook(&sd->bl,LOOK_WEAPON,sd->status.weapon);
	}
	if(pos & EQP_HAND_L) {
		if(id) {
			if(id->type == IT_WEAPON) {
				sd->status.shield = 0;
				sd->weapontype2 = id->look;
			}
			else
			if(id->type == IT_ARMOR) {
				sd->status.shield = id->look;
				sd->weapontype2 = 0;
			}
		}
		else
			sd->status.shield = sd->weapontype2 = 0;
		pc_calcweapontype(sd);
		clif_changelook(&sd->bl,LOOK_SHIELD,sd->status.shield);
	}
	//Added check to prevent sending the same look on multiple slots ->
	//causes client to redraw item on top of itself. (suggested by Lupus)
	if(pos & EQP_HEAD_LOW && pc_checkequip(sd,EQP_COSTUME_HEAD_LOW) == -1) {
		if(id && !(pos&(EQP_HEAD_TOP|EQP_HEAD_MID)))
			sd->status.head_bottom = id->look;
		else
			sd->status.head_bottom = 0;
		clif_changelook(&sd->bl,LOOK_HEAD_BOTTOM,sd->status.head_bottom);
	}
	if(pos & EQP_HEAD_TOP && pc_checkequip(sd,EQP_COSTUME_HEAD_TOP) == -1) {
		if(id)
			sd->status.head_top = id->look;
		else
			sd->status.head_top = 0;
		clif_changelook(&sd->bl,LOOK_HEAD_TOP,sd->status.head_top);
	}
	if(pos & EQP_HEAD_MID && pc_checkequip(sd,EQP_COSTUME_HEAD_MID) == -1) {
		if(id && !(pos&EQP_HEAD_TOP))
			sd->status.head_mid = id->look;
		else
			sd->status.head_mid = 0;
		clif_changelook(&sd->bl,LOOK_HEAD_MID,sd->status.head_mid);
	}
	if(pos & EQP_COSTUME_HEAD_TOP) {
		if(id){
			sd->status.head_top = id->look;
		} else
			sd->status.head_top = 0;
		clif_changelook(&sd->bl,LOOK_HEAD_TOP,sd->status.head_top);
	}
	if(pos & EQP_COSTUME_HEAD_MID) {
		if(id && !(pos&EQP_HEAD_TOP)){
			sd->status.head_mid = id->look;
		} else
			sd->status.head_mid = 0;
		clif_changelook(&sd->bl,LOOK_HEAD_MID,sd->status.head_mid);
	}
	if(pos & EQP_COSTUME_HEAD_LOW) {
		if(id && !(pos&(EQP_HEAD_TOP|EQP_HEAD_MID))){
			sd->status.head_bottom = id->look;
		} else
			sd->status.head_bottom = 0;
		clif_changelook(&sd->bl,LOOK_HEAD_BOTTOM,sd->status.head_bottom);
	}
	if(pos & EQP_SHOES)
		clif_changelook(&sd->bl,LOOK_SHOES,0);
	if(pos&EQP_GARMENT && pc_checkequip(sd,EQP_COSTUME_GARMENT) == -1) {
		sd->status.robe = id ? id->look : 0;
		clif_changelook(&sd->bl, LOOK_ROBE, sd->status.robe);
	}
	if(pos & EQP_COSTUME_GARMENT) {
		sd->status.robe = id ? id->look : 0;
		clif_changelook(&sd->bl,LOOK_ROBE,sd->status.robe);
	}

	pc_checkallowskill(sd); //Check if status changes should be halted.
	iflag = sd->npc_item_flag;

	/* check for combos (MUST be before status_calc_pc) */
	if( id->combos_count )
		pc_checkcombo(sd,id);
	if(itemdb_isspecial(sd->status.inventory[n].card[0]))
		; //No cards
	else {
		for( i = 0; i < id->slot; i++ ) {
			struct item_data *data;
			if (!sd->status.inventory[n].card[i])
				continue;
			if ( ( data = itemdb_exists(sd->status.inventory[n].card[i]) ) != NULL ) {
				if( data->combos_count )
					pc_checkcombo(sd,data);
			}
		}
	}

	status_calc_pc(sd,SCO_NONE);
	if (flag) //Update skill data
		clif_skillinfoblock(sd);

	//OnEquip script [Skotlex]
	if (id) {
		//only run the script if item isn't restricted
		if (id->equip_script && (pc_has_permission(sd,PC_PERM_USE_ALL_EQUIPMENT) || !itemdb_isNoEquip(id,sd->bl.m)))
			run_script(id->equip_script,0,sd->bl.id,fake_nd->bl.id);
		if(itemdb_isspecial(sd->status.inventory[n].card[0]))
			; //No cards
		else {
			for( i = 0; i < id->slot; i++ ) {
				struct item_data *data;
				if (!sd->status.inventory[n].card[i])
					continue;
				if ( ( data = itemdb_exists(sd->status.inventory[n].card[i]) ) != NULL ) {
					if (data->equip_script && (pc_has_permission(sd,PC_PERM_USE_ALL_EQUIPMENT) || !itemdb_isNoEquip(data,sd->bl.m)))
						run_script(data->equip_script,0,sd->bl.id,fake_nd->bl.id);
				}
			}
		}
	}
	sd->npc_item_flag = iflag;

	return true;
}

/*==========================================
 * Called when attemting to unequip an item from player
 * type:
 * 0 - only unequip
 * 1 - calculate status after unequipping
 * 2 - force unequip
 * return: false - fail; true - success
 *------------------------------------------*/
bool pc_unequipitem(struct map_session_data *sd, int n, int flag) {
	int i, iflag;
	bool status_cacl = false;

	nullpo_retr(false,sd);

	if (n < 0 || n >= MAX_INVENTORY) {
		clif_unequipitemack(sd,0,0,0);
		return false;
	}
	if (!sd->status.inventory[n].equip) {
		clif_unequipitemack(sd,n,0,0);
		return false; //Nothing to unequip
	}
	// status change that makes player cannot unequip equipment
	if (!(flag&2) && sd->sc.count &&
		(sd->sc.data[SC_BERSERK] ||
		sd->sc.data[SC_SATURDAYNIGHTFEVER] ||
		sd->sc.data[SC__BLOODYLUST] ||
		sd->sc.data[SC_KYOUGAKU] ||
		(sd->sc.data[SC_PYROCLASTIC] &&
		sd->inventory_data[n]->type == IT_WEAPON)))	// can't switch weapon
	{
		clif_unequipitemack(sd,n,0,0);
		return false;
	}

	if (battle_config.battle_log)
		ShowInfo("unequip %d %x:%x\n",n,pc_equippoint(sd,n),sd->status.inventory[n].equip);

	for(i = 0; i < EQI_MAX; i++) {
		if (sd->status.inventory[n].equip & equip_pos[i])
			sd->equip_index[i] = -1;
	}

	if(sd->status.inventory[n].equip & EQP_HAND_R) {
		sd->weapontype1 = 0;
		sd->status.weapon = sd->weapontype2;
		pc_calcweapontype(sd);
		clif_changelook(&sd->bl,LOOK_WEAPON,sd->status.weapon);
		if( !battle_config.dancing_weaponswitch_fix )
			status_change_end(&sd->bl, SC_DANCING, INVALID_TIMER); // Unequipping => stop dancing.
	}
	if(sd->status.inventory[n].equip & EQP_HAND_L) {
		sd->status.shield = sd->weapontype2 = 0;
		pc_calcweapontype(sd);
		clif_changelook(&sd->bl,LOOK_SHIELD,sd->status.shield);
	}
	if(sd->status.inventory[n].equip & EQP_HEAD_LOW && pc_checkequip(sd,EQP_COSTUME_HEAD_LOW) == -1 ) {
		sd->status.head_bottom = 0;
		clif_changelook(&sd->bl,LOOK_HEAD_BOTTOM,sd->status.head_bottom);
	}
	if(sd->status.inventory[n].equip & EQP_HEAD_TOP && pc_checkequip(sd,EQP_COSTUME_HEAD_TOP) == -1 ) {
		sd->status.head_top = 0;
		clif_changelook(&sd->bl,LOOK_HEAD_TOP,sd->status.head_top);
	}
	if(sd->status.inventory[n].equip & EQP_HEAD_MID && pc_checkequip(sd,EQP_COSTUME_HEAD_MID) == -1 ) {
		sd->status.head_mid = 0;
		clif_changelook(&sd->bl,LOOK_HEAD_MID,sd->status.head_mid);
	}

	if(sd->status.inventory[n].equip & EQP_COSTUME_HEAD_TOP) {
		sd->status.head_top = ( pc_checkequip(sd,EQP_HEAD_TOP) >= 0 ) ? sd->inventory_data[pc_checkequip(sd,EQP_HEAD_TOP)]->look : 0;
		clif_changelook(&sd->bl,LOOK_HEAD_TOP,sd->status.head_top);
	}

	if(sd->status.inventory[n].equip & EQP_COSTUME_HEAD_MID) {
		sd->status.head_mid = ( pc_checkequip(sd,EQP_HEAD_MID) >= 0 ) ? sd->inventory_data[pc_checkequip(sd,EQP_HEAD_MID)]->look : 0;
		clif_changelook(&sd->bl,LOOK_HEAD_MID,sd->status.head_mid);
	}

	if(sd->status.inventory[n].equip & EQP_COSTUME_HEAD_LOW) {
		sd->status.head_bottom = ( pc_checkequip(sd,EQP_HEAD_LOW) >= 0 ) ? sd->inventory_data[pc_checkequip(sd,EQP_HEAD_LOW)]->look : 0;
		clif_changelook(&sd->bl,LOOK_HEAD_BOTTOM,sd->status.head_bottom);
	}

	if(sd->status.inventory[n].equip & EQP_SHOES)
		clif_changelook(&sd->bl,LOOK_SHOES,0);

	if(sd->status.inventory[n].equip&EQP_GARMENT && pc_checkequip(sd,EQP_COSTUME_GARMENT) == -1) {
		sd->status.robe = 0;
		clif_changelook(&sd->bl, LOOK_ROBE, 0);
	}

	if(sd->status.inventory[n].equip & EQP_COSTUME_GARMENT) {
		sd->status.robe = ( pc_checkequip(sd,EQP_GARMENT) >= 0 ) ? sd->inventory_data[pc_checkequip(sd,EQP_GARMENT)]->look : 0;
		clif_changelook(&sd->bl,LOOK_ROBE,sd->status.robe);
	}

	clif_unequipitemack(sd,n,sd->status.inventory[n].equip,1);

	status_change_end(&sd->bl,SC_HEAT_BARREL,INVALID_TIMER);
	// On weapon change (right and left hand)
	if ((sd->status.inventory[n].equip & EQP_ARMS) && sd->inventory_data[n]->type == IT_WEAPON) {
		if (!sd->sc.data[SC_SEVENWIND] || sd->sc.data[SC_ASPERSIO]) //Check for seven wind (but not level seven!)
			skill_enchant_elemental_end(&sd->bl, SC_NONE);
		status_change_end(&sd->bl, SC_FEARBREEZE, INVALID_TIMER);
		status_change_end(&sd->bl, SC_EXEEDBREAK, INVALID_TIMER);
		status_change_end(&sd->bl, SC_P_ALTER, INVALID_TIMER);
	}

	// On armor change
	if (sd->status.inventory[n].equip & EQP_ARMOR) {
		if (sd->sc.data[SC_HOVERING] && sd->inventory_data[n]->nameid == ITEMID_HOVERING_BOOSTER)
			status_change_end(&sd->bl, SC_HOVERING, INVALID_TIMER);
		//status_change_end(&sd->bl, SC_BENEDICTIO, INVALID_TIMER); // No longer is removed? Need confirmation
		status_change_end(&sd->bl, SC_ARMOR_RESIST, INVALID_TIMER);
	}

	// On ammo change
	if (sd->inventory_data[n]->type == IT_AMMO)
		status_change_end(&sd->bl, SC_P_ALTER, INVALID_TIMER);

	if( sd->state.autobonus&sd->status.inventory[n].equip )
		sd->state.autobonus &= ~sd->status.inventory[n].equip; //Check for activated autobonus [Inkfish]

	sd->status.inventory[n].equip = 0;
	iflag = sd->npc_item_flag;

	/* check for combos (MUST be before status_calc_pc) */
	if ( sd->inventory_data[n] ) {
		if( sd->inventory_data[n]->combos_count ) {
			if( pc_removecombo(sd,sd->inventory_data[n]) )
				status_cacl = true;
		} if(itemdb_isspecial(sd->status.inventory[n].card[0]))
			; //No cards
		else {
			for( i = 0; i < sd->inventory_data[n]->slot; i++ ) {
				struct item_data *data;

				if (!sd->status.inventory[n].card[i])
					continue;
				if ( ( data = itemdb_exists(sd->status.inventory[n].card[i]) ) != NULL ) {
					if( data->combos_count ) {
						if( pc_removecombo(sd,data) )
							status_cacl = true;
					}
				}
			}
		}
	}

	if(flag&1 || status_cacl) {
		pc_checkallowskill(sd);
		status_calc_pc(sd,SCO_NONE);
	}

	if(sd->sc.data[SC_SIGNUMCRUCIS] && !battle_check_undead(sd->battle_status.race,sd->battle_status.def_ele))
		status_change_end(&sd->bl, SC_SIGNUMCRUCIS, INVALID_TIMER);

	//OnUnEquip script [Skotlex]
	if (sd->inventory_data[n]) {
		if (sd->inventory_data[n]->unequip_script)
			run_script(sd->inventory_data[n]->unequip_script,0,sd->bl.id,fake_nd->bl.id);
		if(itemdb_isspecial(sd->status.inventory[n].card[0]))
			; //No cards
		else {
			for( i = 0; i < sd->inventory_data[n]->slot; i++ ) {
				struct item_data *data;
				if (!sd->status.inventory[n].card[i])
					continue;

				if ( ( data = itemdb_exists(sd->status.inventory[n].card[i]) ) != NULL ) {
					if( data->unequip_script )
						run_script(data->unequip_script,0,sd->bl.id,fake_nd->bl.id);
				}

			}
		}
	}
	sd->npc_item_flag = iflag;

	return true;
}

/*==========================================
 * Checking if player (sd) has an invalid item
 * and is unequiped on map load (item_noequip)
 *------------------------------------------*/
void pc_checkitem(struct map_session_data *sd) {
	int i, calc_flag = 0;
	struct item it;

	nullpo_retv(sd);

	if( sd->state.vending ) //Avoid reorganizing items when we are vending, as that leads to exploits (pointed out by End of Exam)
		return;

	pc_check_available_item(sd); // Check for invalid(ated) items.

	for( i = 0; i < MAX_INVENTORY; i++ ) {
		it = sd->status.inventory[i];

		if( it.nameid == 0 )
			continue;
		if( !it.equip )
			continue;
		if( it.equip&~pc_equippoint(sd,i) ) {
			pc_unequipitem(sd, i, 2);
			calc_flag = 1;
			continue;
		}

		if( !pc_has_permission(sd, PC_PERM_USE_ALL_EQUIPMENT) && !battle_config.allow_equip_restricted_item && itemdb_isNoEquip(sd->inventory_data[i], sd->bl.m) ) {
			pc_unequipitem(sd, i, 2);
			calc_flag = 1;
			continue;
		}
	}

	if( calc_flag && sd->state.active ) {
		pc_checkallowskill(sd);
		status_calc_pc(sd,SCO_NONE);
	}
}

/*==========================================
 * Checks for unavailable items and removes them.
 *------------------------------------------*/
void pc_check_available_item(struct map_session_data *sd)
{
	int i;
	unsigned short nameid;
	char output[256];

	nullpo_retv(sd);

	if (battle_config.item_check&0x1) { // Check for invalid(ated) items in inventory.
		for(i = 0; i < MAX_INVENTORY; i++) {
			nameid = sd->status.inventory[i].nameid;

			if (!nameid)
				continue;
			if (!itemdb_available(nameid)) {
				sprintf(output, msg_txt(sd, 709), nameid); // Item %hu has been removed from your inventory.
				clif_displaymessage(sd->fd, output);
				ShowWarning("Removed invalid/disabled item id %hu from inventory (amount=%d, char_id=%d).\n", nameid, sd->status.inventory[i].amount, sd->status.char_id);
				pc_delitem(sd, i, sd->status.inventory[i].amount, 0, 0, LOG_TYPE_OTHER);
				continue;
			}
			if (!sd->status.inventory[i].unique_id && !itemdb_isstackable(nameid))
				sd->status.inventory[i].unique_id = pc_generate_unique_id(sd);
		}
	}

	if (battle_config.item_check&0x2) { // Check for invalid(ated) items in cart.
		for(i = 0; i < MAX_CART; i++) {
			nameid = sd->status.cart[i].nameid;

			if (!nameid)
				continue;
			if (!itemdb_available(nameid)) {
				sprintf(output, msg_txt(sd, 710), nameid); // Item %hu has been removed from your cart.
				clif_displaymessage(sd->fd, output);
				ShowWarning("Removed invalid/disabled item id %hu from cart (amount=%d, char_id=%d).\n", nameid, sd->status.cart[i].amount, sd->status.char_id);
				pc_cart_delitem(sd, i, sd->status.cart[i].amount, 0, LOG_TYPE_OTHER);
				continue;
			}
			if (!sd->status.cart[i].unique_id && !itemdb_isstackable(nameid))
				sd->status.cart[i].unique_id = pc_generate_unique_id(sd);
		}
	}

	if (battle_config.item_check&0x4) { // Check for invalid(ated) items in storage.
		for(i = 0; i < sd->storage_size; i++) {
			nameid = sd->status.storage.items[i].nameid;

			if (!nameid)
				continue;
			if (!itemdb_available(nameid)) {
				sprintf(output, msg_txt(sd, 711), nameid); // Item %hu has been removed from your storage.
				clif_displaymessage(sd->fd, output);
				ShowWarning("Removed invalid/disabled item id %hu from storage (amount=%d, char_id=%d).\n", nameid, sd->status.storage.items[i].amount, sd->status.char_id);
				storage_delitem(sd, i, sd->status.storage.items[i].amount);
				continue;
			}
			if (!sd->status.storage.items[i].unique_id && !itemdb_isstackable(nameid))
				sd->status.storage.items[i].unique_id = pc_generate_unique_id(sd);
 		}
	}
}

/*==========================================
 * Update PVP rank for sd1 in cmp to sd2
 *------------------------------------------*/
static int pc_calc_pvprank_sub(struct block_list *bl,va_list ap)
{
	struct map_session_data *sd1,*sd2;

	sd1=(struct map_session_data *)bl;
	sd2=va_arg(ap,struct map_session_data *);

	if( sd1->sc.option&OPTION_INVISIBLE || sd2->sc.option&OPTION_INVISIBLE )
	{// cannot register pvp rank for hidden GMs
		return 0;
	}

	if( sd1->pvp_point > sd2->pvp_point )
		sd2->pvp_rank++;
	return 0;
}
/*==========================================
 * Calculate new rank beetween all present players (map_foreachinarea)
 * and display result
 *------------------------------------------*/
int pc_calc_pvprank(struct map_session_data *sd)
{
	int old = sd->pvp_rank;
	struct map_data *m = &map[sd->bl.m];

	sd->pvp_rank=1;
	map_foreachinmap(pc_calc_pvprank_sub,sd->bl.m,BL_PC,sd);
	if(old!=sd->pvp_rank || sd->pvp_lastusers!=m->users_pvp)
		clif_pvpset(sd,sd->pvp_rank,sd->pvp_lastusers=m->users_pvp,0);
	return sd->pvp_rank;
}
/*==========================================
 * Calculate next sd ranking calculation from config
 *------------------------------------------*/
int pc_calc_pvprank_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd;

	sd=map_id2sd(id);
	if(sd==NULL)
		return 0;
	sd->pvp_timer = INVALID_TIMER;

	if( sd->sc.option&OPTION_INVISIBLE )
	{// do not calculate the pvp rank for a hidden GM
		return 0;
	}

	if( pc_calc_pvprank(sd) > 0 )
		sd->pvp_timer = add_timer(gettick()+PVP_CALCRANK_INTERVAL,pc_calc_pvprank_timer,id,data);
	return 0;
}

/*==========================================
 * Checking if sd is married
 * Return:
 *	partner_id = yes
 *	0 = no
 *------------------------------------------*/
int pc_ismarried(struct map_session_data *sd)
{
	if(sd == NULL)
		return -1;
	if(sd->status.partner_id > 0)
		return sd->status.partner_id;
	else
		return 0;
}
/*==========================================
 * Marry player sd to player dstsd
 * Return:
 *	false = fail
 *	true = success
 *------------------------------------------*/
bool pc_marriage(struct map_session_data *sd,struct map_session_data *dstsd)
{
	if(sd == NULL || dstsd == NULL ||
		sd->status.partner_id > 0 || dstsd->status.partner_id > 0 ||
		(sd->class_&JOBL_BABY) || (dstsd->class_&JOBL_BABY))
		return false;
	sd->status.partner_id = dstsd->status.char_id;
	dstsd->status.partner_id = sd->status.char_id;
	return true;
}

/*==========================================
 * Divorce sd from its partner
 * Return:
 *	false = fail
 *	true  = success
 *------------------------------------------*/
bool pc_divorce(struct map_session_data *sd)
{
	struct map_session_data *p_sd;
	int i;

	if( sd == NULL || !pc_ismarried(sd) )
		return false;

	if( !sd->status.partner_id )
		return false; // Char is not married

	if( (p_sd = map_charid2sd(sd->status.partner_id)) == NULL )
	{ // Lets char server do the divorce
		if( chrif_divorce(sd->status.char_id, sd->status.partner_id) )
			return false; // No char server connected

		return true;
	}

	// Both players online, lets do the divorce manually
	sd->status.partner_id = 0;
	p_sd->status.partner_id = 0;
	for( i = 0; i < MAX_INVENTORY; i++ )
	{
		if( sd->status.inventory[i].nameid == WEDDING_RING_M || sd->status.inventory[i].nameid == WEDDING_RING_F )
			pc_delitem(sd, i, 1, 0, 0, LOG_TYPE_OTHER);
		if( p_sd->status.inventory[i].nameid == WEDDING_RING_M || p_sd->status.inventory[i].nameid == WEDDING_RING_F )
			pc_delitem(p_sd, i, 1, 0, 0, LOG_TYPE_OTHER);
	}

	clif_divorced(sd, p_sd->status.name);
	clif_divorced(p_sd, sd->status.name);

	return true;
}

/**
 * Get the partner map_session_data of a player
 * @param sd : the husband|wife session
 * @return partner session or NULL
 */
struct map_session_data *pc_get_partner(struct map_session_data *sd){
	if (!sd || !pc_ismarried(sd))
		return NULL;
	return map_charid2sd(sd->status.partner_id);
}

/**
 * Get the father map_session_data of a player
 * @param sd : the baby session
 * @return father session or NULL
 */
struct map_session_data *pc_get_father (struct map_session_data *sd){
	if (!sd || !(sd->class_&JOBL_BABY) || !sd->status.father)
		return NULL;
	return map_charid2sd(sd->status.father);
}

/**
 * Get the mother map_session_data of a player
 * @param sd : the baby session
 * @return mother session or NULL
 */
struct map_session_data *pc_get_mother (struct map_session_data *sd){
	if (!sd || !(sd->class_&JOBL_BABY) || !sd->status.mother)
		return NULL;
	return map_charid2sd(sd->status.mother);
}

/*==========================================
 * Get sd children charid. (Need to be married)
 *------------------------------------------*/
struct map_session_data *pc_get_child (struct map_session_data *sd)
{
	if (!sd || !pc_ismarried(sd) || !sd->status.child)
		// charid2sd returns NULL if not found
		return NULL;
	return map_charid2sd(sd->status.child);
}

/*==========================================
 * Set player sd to bleed. (losing hp and/or sp each diff_tick)
 *------------------------------------------*/
void pc_bleeding (struct map_session_data *sd, unsigned int diff_tick)
{
	int hp = 0, sp = 0;

	if( pc_isdead(sd) )
		return;

	if (sd->hp_loss.value) {
		sd->hp_loss.tick += diff_tick;
		while (sd->hp_loss.tick >= sd->hp_loss.rate) {
			hp += sd->hp_loss.value;
			sd->hp_loss.tick -= sd->hp_loss.rate;
		}
		if(hp >= sd->battle_status.hp)
			hp = sd->battle_status.hp-1; //Script drains cannot kill you.
	}

	if (sd->sp_loss.value) {
		sd->sp_loss.tick += diff_tick;
		while (sd->sp_loss.tick >= sd->sp_loss.rate) {
			sp += sd->sp_loss.value;
			sd->sp_loss.tick -= sd->sp_loss.rate;
		}
	}

	if (hp > 0 || sp > 0)
		status_zap(&sd->bl, hp, sp);
}

//Character regen. Flag is used to know which types of regen can take place.
//&1: HP regen
//&2: SP regen
void pc_regen (struct map_session_data *sd, unsigned int diff_tick)
{
	int hp = 0, sp = 0;

	if (sd->hp_regen.value) {
		sd->hp_regen.tick += diff_tick;
		while (sd->hp_regen.tick >= sd->hp_regen.rate) {
			hp += sd->hp_regen.value;
			sd->hp_regen.tick -= sd->hp_regen.rate;
		}
	}

	if (sd->sp_regen.value) {
		sd->sp_regen.tick += diff_tick;
		while (sd->sp_regen.tick >= sd->sp_regen.rate) {
			sp += sd->sp_regen.value;
			sd->sp_regen.tick -= sd->sp_regen.rate;
		}
	}

	if (hp > 0 || sp > 0)
		status_heal(&sd->bl, hp, sp, 0);
}

/*==========================================
 * Memo player sd savepoint. (map,x,y)
 *------------------------------------------*/
void pc_setsavepoint(struct map_session_data *sd, short mapindex,int x,int y)
{
	nullpo_retv(sd);

	sd->status.save_point.map = mapindex;
	sd->status.save_point.x = x;
	sd->status.save_point.y = y;
}

/*==========================================
 * Save 1 player data at autosave interval
 *------------------------------------------*/
static int pc_autosave(int tid, unsigned int tick, int id, intptr_t data)
{
	int interval;
	struct s_mapiterator* iter;
	struct map_session_data* sd;
	static int last_save_id = 0, save_flag = 0;

	if(save_flag == 2) //Someone was saved on last call, normal cycle
		save_flag = 0;
	else
		save_flag = 1; //Noone was saved, so save first found char.

	iter = mapit_getallusers();
	for( sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter) )
	{
		if(sd->bl.id == last_save_id && save_flag != 1) {
			save_flag = 1;
			continue;
		}

		if(save_flag != 1) //Not our turn to save yet.
			continue;

		//Save char.
		last_save_id = sd->bl.id;
		save_flag = 2;
#ifdef VIP_ENABLE
		if(sd->vip.enabled) //check if we're still vip
			chrif_req_login_operation(1, sd->status.name, 6, 0, 1, 0);
#endif
		chrif_save(sd,0);
		break;
	}
	mapit_free(iter);

	interval = autosave_interval/(map_usercount()+1);
	if(interval < minsave_interval)
		interval = minsave_interval;
	add_timer(gettick()+interval,pc_autosave,0,0);

	return 0;
}

static int pc_daynight_timer_sub(struct map_session_data *sd,va_list ap)
{
	if (sd->state.night != night_flag && map[sd->bl.m].flag.nightenabled)
	{	//Night/day state does not match.
		clif_status_load(&sd->bl, SI_NIGHT, night_flag); //New night effect by dynamix [Skotlex]
		sd->state.night = night_flag;
		return 1;
	}
	return 0;
}
/*================================================
 * timer to do the day [Yor]
 * data: 0 = called by timer, 1 = gmcommand/script
 *------------------------------------------------*/
int map_day_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	char tmp_soutput[1024];

	if (data == 0 && battle_config.day_duration <= 0)	// if we want a day
		return 0;

	if (!night_flag)
		return 0; //Already day.

	night_flag = 0; // 0=day, 1=night [Yor]
	map_foreachpc(pc_daynight_timer_sub);
	strcpy(tmp_soutput, (data == 0) ? msg_txt(NULL,502) : msg_txt(NULL,60)); // The day has arrived!
	intif_broadcast(tmp_soutput, strlen(tmp_soutput) + 1, BC_DEFAULT);
	return 0;
}

/*================================================
 * timer to do the night [Yor]
 * data: 0 = called by timer, 1 = gmcommand/script
 *------------------------------------------------*/
int map_night_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	char tmp_soutput[1024];

	if (data == 0 && battle_config.night_duration <= 0)	// if we want a night
		return 0;

	if (night_flag)
		return 0; //Already nigth.

	night_flag = 1; // 0=day, 1=night [Yor]
	map_foreachpc(pc_daynight_timer_sub);
	strcpy(tmp_soutput, (data == 0) ? msg_txt(NULL,503) : msg_txt(NULL,59)); // The night has fallen...
	intif_broadcast(tmp_soutput, strlen(tmp_soutput) + 1, BC_DEFAULT);
	return 0;
}

/**
* Attempt to stand up a player
* @param sd
* @param force Ignore the check, ask player to stand up. Used in some cases like pc_damage(), pc_revive(), etc
* @return True if success, Fals if failed
*/
bool pc_setstand(struct map_session_data *sd, bool force){
	nullpo_ret(sd);

	// Cannot stand yet
	// TODO: Move to SCS_NOSTAND [Cydh]
	if (!force && &sd->sc && (sd->sc.data[SC_SITDOWN_FORCE] || sd->sc.data[SC_BANANA_BOMB_SITDOWN]))
		return false;

	status_change_end(&sd->bl, SC_TENSIONRELAX, INVALID_TIMER);
	clif_status_load(&sd->bl,SI_SIT,0);
	clif_standing(&sd->bl); //Inform area PC is standing
	//Reset sitting tick.
	sd->ssregen.tick.hp = sd->ssregen.tick.sp = 0;
	sd->state.dead_sit = sd->vd.dead_sit = 0;
	return true;
}

/**
 * Mechanic (MADO GEAR)
 **/
void pc_overheat(struct map_session_data *sd, int val) {
	int heat = val, skill,
		limit[] = { 10, 20, 28, 46, 66 };

	if( !pc_ismadogear(sd) || sd->sc.data[SC_OVERHEAT] )
		return; // already burning

	skill = cap_value(pc_checkskill(sd,NC_MAINFRAME),0,4);
	if( sd->sc.data[SC_OVERHEAT_LIMITPOINT] ) {
		heat += sd->sc.data[SC_OVERHEAT_LIMITPOINT]->val1;
		status_change_end(&sd->bl,SC_OVERHEAT_LIMITPOINT,INVALID_TIMER);
	}

	heat = max(0,heat); // Avoid negative HEAT
	if( heat >= limit[skill] )
		sc_start(&sd->bl,&sd->bl,SC_OVERHEAT,100,0,1000);
	else
		sc_start(&sd->bl,&sd->bl,SC_OVERHEAT_LIMITPOINT,100,heat,30000);

	return;
}

/**
 * Check if player is autolooting given itemID.
 */
bool pc_isautolooting(struct map_session_data *sd, unsigned short nameid)
{
	uint8 i = 0;

	if (sd->state.autoloottype && sd->state.autoloottype&(1<<itemdb_type(nameid)))
		return true;

	if (!sd->state.autolooting)
		return false;

	if (sd->state.autolooting)
		ARR_FIND(0, AUTOLOOTITEM_SIZE, i, sd->state.autolootid[i] == nameid);

	return (i != AUTOLOOTITEM_SIZE);
}

/**
 * Checks if player can use @/#command
 * @param sd Player map session data
 * @param command Command name without @/# and params
 * @param type is it atcommand or charcommand
 */
bool pc_can_use_command(struct map_session_data *sd, const char *command, AtCommandType type)
{
	return pc_group_can_use_command(pc_get_group_id(sd), command, type);
}

/**
 * Checks if commands used by a player should be logged
 * according to their group setting.
 * @param sd Player map session data
 */
bool pc_should_log_commands(struct map_session_data *sd)
{
	return pc_group_should_log_commands(pc_get_group_id(sd));
}

/**
 * Spirit Charm expiration timer.
 * @see TimerFunc
 */
static int pc_spiritcharm_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd;
	int i;

	if ((sd = (struct map_session_data *)map_id2sd(id)) == NULL || sd->bl.type != BL_PC)
		return 1;

	if (sd->spiritcharm <= 0) {
		ShowError("pc_spiritcharm_timer: %d spiritcharm's available. (aid=%d cid=%d tid=%d)\n", sd->spiritcharm, sd->status.account_id, sd->status.char_id, tid);
		sd->spiritcharm = 0;
		sd->spiritcharm_type = CHARM_TYPE_NONE;
		return 0;
	}

	ARR_FIND(0, sd->spiritcharm, i, sd->spiritcharm_timer[i] == tid);

	if (i == sd->spiritcharm) {
		ShowError("pc_spiritcharm_timer: timer not found (aid=%d cid=%d tid=%d)\n", sd->status.account_id, sd->status.char_id, tid);
		return 0;
	}

	sd->spiritcharm--;

	if (i != sd->spiritcharm)
		memmove(sd->spiritcharm_timer + i, sd->spiritcharm_timer + i + 1, (sd->spiritcharm - i) * sizeof(int));

	sd->spiritcharm_timer[sd->spiritcharm] = INVALID_TIMER;

	if (sd->spiritcharm <= 0)
		sd->spiritcharm_type = CHARM_TYPE_NONE;

	clif_spiritcharm(sd);

	return 0;
}

/**
 * Adds a spirit charm.
 * @param sd: Target character
 * @param interval: Duration
 * @param max: Maximum amount of charms to add
 * @param type: Charm type (@see spirit_charm_types)
 */
void pc_addspiritcharm(struct map_session_data *sd, int interval, int max, int type)
{
	int tid, i;

	nullpo_retv(sd);

	if (sd->spiritcharm_type != CHARM_TYPE_NONE && type != sd->spiritcharm_type)
		pc_delspiritcharm(sd, sd->spiritcharm, sd->spiritcharm_type);

	if (max > MAX_SPIRITCHARM)
		max = MAX_SPIRITCHARM;

	if (sd->spiritcharm < 0)
		sd->spiritcharm = 0;

	if (sd->spiritcharm && sd->spiritcharm >= max) {
		if (sd->spiritcharm_timer[0] != INVALID_TIMER)
			delete_timer(sd->spiritcharm_timer[0], pc_spiritcharm_timer);
		sd->spiritcharm--;
		if (sd->spiritcharm != 0)
			memmove(sd->spiritcharm_timer + 0, sd->spiritcharm_timer + 1, (sd->spiritcharm) * sizeof(int));
		sd->spiritcharm_timer[sd->spiritcharm] = INVALID_TIMER;
	}

	tid = add_timer(gettick() + interval, pc_spiritcharm_timer, sd->bl.id, 0);
	ARR_FIND(0, sd->spiritcharm, i, sd->spiritcharm_timer[i] == INVALID_TIMER || DIFF_TICK(get_timer(tid)->tick, get_timer(sd->spiritcharm_timer[i])->tick) < 0);

	if (i != sd->spiritcharm)
		memmove(sd->spiritcharm_timer + i + 1, sd->spiritcharm_timer + i, (sd->spiritcharm - i) * sizeof(int));

	sd->spiritcharm_timer[i] = tid;
	sd->spiritcharm++;
	sd->spiritcharm_type = type;

	clif_spiritcharm(sd);
}

/**
 * Removes one or more spirit charms.
 * @param sd: The target character
 * @param count: Amount of charms to remove
 * @param type: Type of charm to remove
 */
void pc_delspiritcharm(struct map_session_data *sd, int count, int type)
{
	int i;

	nullpo_retv(sd);

	if (sd->spiritcharm_type != type)
		return;

	if (sd->spiritcharm <= 0) {
		sd->spiritcharm = 0;
		return;
	}

	if (count <= 0)
		return;

	if (count > sd->spiritcharm)
		count = sd->spiritcharm;

	sd->spiritcharm -= count;

	if (count > MAX_SPIRITCHARM)
		count = MAX_SPIRITCHARM;

	for (i = 0; i < count; i++) {
		if (sd->spiritcharm_timer[i] != INVALID_TIMER) {
			delete_timer(sd->spiritcharm_timer[i], pc_spiritcharm_timer);
			sd->spiritcharm_timer[i] = INVALID_TIMER;
		}
	}

	for (i = count; i < MAX_SPIRITCHARM; i++) {
		sd->spiritcharm_timer[i - count] = sd->spiritcharm_timer[i];
		sd->spiritcharm_timer[i] = INVALID_TIMER;
	}

	if (sd->spiritcharm <= 0)
		sd->spiritcharm_type = CHARM_TYPE_NONE;

	clif_spiritcharm(sd);
}

#if defined(RENEWAL_DROP) || defined(RENEWAL_EXP)
/*==========================================
 * Renewal EXP/Itemdrop rate modifier base on level penalty
 * 1=exp 2=itemdrop
 *------------------------------------------*/
int pc_level_penalty_mod(struct map_session_data *sd, int mob_level, uint32 mob_class, int type)
{
	int diff, rate = 100, i;
	int tmp;

	nullpo_ret(sd);

	diff = mob_level - sd->status.base_level;

	if( diff < 0 )
		diff = MAX_LEVEL + ( ~diff + 1 );

	if((tmp = level_penalty[type][mob_class][diff] ) > 0 ) //use mobclass directly
		return tmp;
	
	//wtf is that for ? if penalty not found use the 1st one we found ?? ̂[lighta]
	for( i = 0; i < CLASS_ALL; i++ ) {
		if( ( tmp = level_penalty[type][i][diff] ) > 0 ) {
			rate = tmp;
			break;
		}
	}

	return rate;
}
#endif

int pc_split_str(char *str,char **val,int num)
{
	int i;

	for (i=0; i<num && str; i++){
		val[i] = str;
		str = strchr(str,',');
		if (str && i<num-1) //Do not remove a trailing comma.
			*str++=0;
	}
	return i;
}

int pc_split_atoi(char* str, int* val, char sep, int max)
{
	int i,j;
	for (i=0; i<max; i++) {
		if (!str) break;
		val[i] = atoi(str);
		str = strchr(str,sep);
		if (str)
			*str++=0;
	}
	//Zero up the remaining.
	for(j=i; j < max; j++)
		val[j] = 0;
	return i;
}

int pc_split_atoui(char* str, unsigned int* val, char sep, int max)
{
	static int warning=0;
	int i,j;
	for (i=0; i<max; i++) {
		double f;
		if (!str) break;
		f = atof(str);
		if (f < 0)
			val[i] = 0;
		else if (f > UINT_MAX) {
			val[i] = UINT_MAX;
			if (!warning) {
				warning = 1;
				ShowWarning("pc_readdb (exp.txt): Required exp per level is capped to %u\n", UINT_MAX);
			}
		} else
			val[i] = (unsigned int)f;
		str = strchr(str,sep);
		if (str)
			*str++=0;
	}
	//Zero up the remaining.
	for(j=i; j < max; j++)
		val[j] = 0;
	return i;
}

/*==========================================
 * sub DB reading.
 * Function used to read skill_tree.txt
 *------------------------------------------*/
static bool pc_readdb_skilltree(char* fields[], int columns, int current)
{
	unsigned char joblv = 0, skill_lv;
	uint16 skill_id;
	int idx, class_;
	unsigned int i, offset = 3, skill_idx;

	class_  = atoi(fields[0]);
	skill_id = (uint16)atoi(fields[1]);
	skill_lv = (unsigned char)atoi(fields[2]);

	if(columns==4+MAX_PC_SKILL_REQUIRE*2)
	{// job level requirement extra column
		joblv = (unsigned char)atoi(fields[3]);
		offset++;
	}

	if(!pcdb_checkid(class_))
	{
		ShowWarning("pc_readdb_skilltree: Invalid job class %d specified.\n", class_);
		return false;
	}
	idx = pc_class2idx(class_);

	//This is to avoid adding two lines for the same skill. [Skotlex]
	ARR_FIND( 0, MAX_SKILL_TREE, skill_idx, skill_tree[idx][skill_idx].id == 0 || skill_tree[idx][skill_idx].id == skill_id );
	if( skill_idx == MAX_SKILL_TREE )
	{
		ShowWarning("pc_readdb_skilltree: Unable to load skill %hu into job %d's tree. Maximum number of skills per class has been reached.\n", skill_id, class_);
		return false;
	}
	else if(skill_tree[idx][skill_idx].id)
	{
		ShowNotice("pc_readdb_skilltree: Overwriting skill %hu for job class %d.\n", skill_id, class_);
	}

	skill_tree[idx][skill_idx].id    = skill_id;
	skill_tree[idx][skill_idx].max   = skill_lv;
	skill_tree[idx][skill_idx].joblv = joblv;

	for(i = 0; i < MAX_PC_SKILL_REQUIRE; i++)
	{
		skill_tree[idx][skill_idx].need[i].id = atoi(fields[i*2+offset]);
		skill_tree[idx][skill_idx].need[i].lv = atoi(fields[i*2+offset+1]);
	}
	return true;
}
#if defined(RENEWAL_DROP) || defined(RENEWAL_EXP)
static bool pc_readdb_levelpenalty(char* fields[], int columns, int current)
{
	int type, class_, diff;

	type = atoi(fields[0]); //1=experience, 2=item drop
	class_ = atoi(fields[1]);
	diff = atoi(fields[2]);

	if( type != 1 && type != 2 ){
		ShowWarning("pc_readdb_levelpenalty: Invalid type %d specified.\n", type);
		return false;
	}

	if( !CHK_CLASS(class_) ){
		ShowWarning("pc_readdb_levelpenalty: Invalid class %d specified.\n", class_);
		return false;
	}

	diff = min(diff, MAX_LEVEL);

	if( diff < 0 )
		diff = min(MAX_LEVEL + ( ~(diff) + 1 ), MAX_LEVEL*2);

	level_penalty[type][class_][diff] = atoi(fields[3]);

	return true;
}
#endif

/** [Cydh]
* Calculates base hp of player. Reference: http://irowiki.org/wiki/Max_HP
* @param level Base level of player
* @param class_ Job ID @see enum e_job
* @return base_hp
*/
static unsigned int pc_calc_basehp(uint16 level, uint16 class_) {
	double base_hp;
	uint16 i, idx = pc_class2idx(class_);

	base_hp = 35 + level * (job_info[idx].hp_multiplicator/100.);
#ifndef RENEWAL
	if(level >= 10 && (class_ == JOB_NINJA || class_ == JOB_GUNSLINGER)) base_hp += 90;
#endif
	for (i = 2; i <= level; i++)
		base_hp += floor(((job_info[idx].hp_factor/100.) * i) + 0.5); //Don't have round()
	return (unsigned int)base_hp;
}

/** [Playtester]
* Calculates base sp of player.
* @param level Base level of player
* @param class_ Job ID @see enum e_job
* @return base_sp
*/
static unsigned int pc_calc_basesp(uint16 level, uint16 class_) {
	double base_sp;
	uint16 idx = pc_class2idx(class_);

	base_sp = 10 + floor(level * (job_info[idx].sp_factor / 100.));

	switch (class_) {
		case JOB_NINJA:
			if (level >= 10)
				base_sp -= 22;
			else
				base_sp = 11 + 3*level;
			break;
		case JOB_GUNSLINGER:
			if (level > 10)
				base_sp -= 18;
			else
				base_sp = 9 + 3*level;
			break;
	}

	return (unsigned int)base_sp;
}

//Reading job_db1.txt line, (class,weight,HPFactor,HPMultiplicator,SPFactor,aspd/lvl...)
static bool pc_readdb_job1(char* fields[], int columns, int current){
	int idx, class_;
	unsigned int i;

	class_ = atoi(fields[0]);

	if (!pcdb_checkid(class_)) {
		ShowWarning("status_readdb_job1: Invalid job class %d specified.\n", class_);
		return false;
	}
	idx = pc_class2idx(class_);

	job_info[idx].max_weight_base = atoi(fields[1]);
	job_info[idx].hp_factor  = atoi(fields[2]);
	job_info[idx].hp_multiplicator = atoi(fields[3]);
	job_info[idx].sp_factor  = atoi(fields[4]);

#ifdef RENEWAL_ASPD
	for(i = 0; i <= MAX_WEAPON_TYPE; i++)
#else
	for(i = 0; i < MAX_WEAPON_TYPE; i++)
#endif
	{
		job_info[idx].aspd_base[i] = atoi(fields[i+5]);
	}

	return true;
}

//Reading job_db2.txt line (class,JobLv1,JobLv2,JobLv3,...)
static bool pc_readdb_job2(char* fields[], int columns, int current)
{
	int idx, class_, i;

	class_ = atoi(fields[0]);

	if(!pcdb_checkid(class_))
	{
		ShowWarning("status_readdb_job2: Invalid job class %d specified.\n", class_);
		return false;
	}
	idx = pc_class2idx(class_);

	for(i = 1; i < columns; i++)
	{
		job_info[idx].job_bonus[i-1] = atoi(fields[i]);
	}
	return true;
}

//Reading job_exp.txt line
//Max Level,Class list,Type (0 - Base Exp; 1 - Job Exp),Exp/lvl...
static bool pc_readdb_job_exp(char* fields[], int columns, int current)
{
	int idx, i, type;
	int job_id,job_count,jobs[CLASS_COUNT];
	unsigned int ui, maxlvl;

	maxlvl = atoi(fields[0]);
	if(maxlvl > MAX_LEVEL || maxlvl<1){
		ShowError("pc_readdb_job_exp: Invalid maxlevel %d specified.\n", maxlvl);
		return false;
	}
	if((maxlvl+3) > columns){ //nb values = (maxlvl-startlvl)+1-index1stvalue
		ShowError("pc_readdb_job_exp: Number of columns %d defined is too low for max level %d.\n",columns,maxlvl);
		return false;
	}
	type = atoi(fields[2]);
	if(type < 0 || type > 1){
		ShowError("pc_readdb_job_exp: Invalid type %d specified.\n", type);
		return false;
	}
	job_count = pc_split_atoi(fields[1],jobs,':',CLASS_COUNT);
	if (job_count < 1)
		return false;
	job_id = jobs[0];
	if(!pcdb_checkid(job_id)){
		ShowError("pc_readdb_job_exp: Invalid job class %d specified.\n", job_id);
		return false;
	}
	idx = pc_class2idx(job_id);

	job_info[idx].max_level[type] = maxlvl;
	for(i=0; i<maxlvl; i++)
		job_info[idx].exp_table[type][i] = ((uint32) atoi(fields[3+i]));
	//Reverse check in case the array has a bunch of trailing zeros... [Skotlex]
	//The reasoning behind the -2 is this... if the max level is 5, then the array
	//should look like this:
	//0: x, 1: x, 2: x: 3: x 4: 0 <- last valid value is at 3.
	while ((ui = job_info[idx].max_level[type]) >= 2 && job_info[idx].exp_table[type][ui-2] <= 0)
		job_info[idx].max_level[type]--;
	if (job_info[idx].max_level[type] < maxlvl) {
		ShowWarning("pc_readdb_job_exp: Specified max %u for job %d, but that job's exp table only goes up to level %u.\n", maxlvl, job_id, job_info[idx].max_level[type]);
		ShowInfo("Filling the missing values with the last exp entry.\n");
		//Fill the requested values with the last entry.
		ui = (job_info[idx].max_level[type] <= 2? 0: job_info[idx].max_level[type]-2);
		for (; ui+2 < maxlvl; ui++)
			job_info[idx].exp_table[type][ui] = job_info[idx].exp_table[type][ui-1];
		job_info[idx].max_level[type] = maxlvl;
	}
//	ShowInfo("%s - Class %d: %d\n", type?"Job":"Base", job_id, job_info[idx].max_level[type]);
	for (i = 1; i < job_count; i++) {
		job_id = jobs[i];
		if (!pcdb_checkid(job_id)) {
			ShowError("pc_readdb_job_exp: Invalid job ID %d.\n", job_id);
			continue;
		}
		idx = pc_class2idx(job_id);
		memcpy(job_info[idx].exp_table[type], job_info[pc_class2idx(jobs[0])].exp_table[type], sizeof(job_info[pc_class2idx(jobs[0])].exp_table[type]));
		job_info[idx].max_level[type] = maxlvl;
//		ShowInfo("%s - Class %d: %u\n", type?"Job":"Base", job_id, job_info[idx].max_level[type]);
	}
	return true;
}

/**
* #ifdef HP_SP_TABLES, reads 'job_basehpsp_db.txt to replace hp/sp results from formula
* startlvl,endlvl,class,type,values...
*/
#ifdef HP_SP_TABLES
static bool pc_readdb_job_basehpsp(char* fields[], int columns, int current)
{
	int i, startlvl, endlvl;
	int job_count,jobs[CLASS_COUNT];
	short type;

	startlvl = atoi(fields[0]);
	if(startlvl<1){
		ShowError("pc_readdb_job_basehpsp: Invalid start level %d specified.\n", startlvl);
		return false;
	}
	endlvl = atoi(fields[1]);
	if(endlvl<1 || endlvl<startlvl){
		ShowError("pc_readdb_job_basehpsp: Invalid end level %d specified.\n", endlvl);
		return false;
	}
	if((endlvl-startlvl+1+4) > columns){ //nb values = (maxlvl-startlvl)+1-index1stvalue
		ShowError("pc_readdb_job_basehpsp: Number of columns %d (needs %d) defined is too low for start level %d, max level %d.\n",columns,(endlvl-startlvl+1+4),startlvl,endlvl);
		return false;
	}
	type = atoi(fields[3]);
	if(type < 0 || type > 1){
		ShowError("pc_readdb_job_basehpsp: Invalid type %d specified.\n", type);
		return false;
	}

	job_count = pc_split_atoi(fields[2],jobs,':',CLASS_COUNT);
	if (job_count < 1)
		return false;

	for (i = 0; i < job_count; i++) {
		int idx, job_id = jobs[i], use_endlvl;
		if (!pcdb_checkid(job_id)) {
			ShowError("pc_readdb_job_basehpsp: Invalid job class %d specified.\n", job_id);
			return false;
		}
		idx = pc_class2idx(job_id);
		if (startlvl > job_info[idx].max_level[0]) {
			ShowError("pc_readdb_job_basehpsp: Invalid start level %d specified.\n", startlvl);
			return false;
		}
		//Just read until available max level for this job, don't use MAX_LEVEL!
		use_endlvl = endlvl;
		if (use_endlvl > job_info[idx].max_level[0])
			use_endlvl = job_info[idx].max_level[0];
		
		if(type == 0) {	//hp type
			uint16 j;
			for(j = 0; j < use_endlvl; j++) {
				if (atoi(fields[j+4])) {
					uint16 lvl_idx = startlvl-1+j;
					job_info[idx].base_hp[lvl_idx] = atoi(fields[j+4]);
					//Tells if this HP is lower than previous level (but not for 99->100)
					if (lvl_idx-1 >= 0 && lvl_idx != 99 && job_info[idx].base_hp[lvl_idx] < job_info[idx].base_hp[lvl_idx-1])
						ShowInfo("pc_readdb_job_basehpsp: HP value at entry %d col %d is lower than previous level (job=%d,lvl=%d,oldval=%d,val=%d).\n",
							current,j+4,job_id,lvl_idx+1,job_info[idx].base_hp[lvl_idx-1],job_info[idx].base_hp[lvl_idx]);
				}
			}
		}
		else { //sp type
			uint16 j;
			for(j = 0; j < use_endlvl; j++) {
				if (atoi(fields[j+4])) {
					uint16 lvl_idx = startlvl-1+j;
					job_info[idx].base_sp[lvl_idx] = atoi(fields[j+4]);
					//Tells if this SP is lower than previous level (but not for 99->100)
					if (lvl_idx-1 >= 0 && lvl_idx != 99 && job_info[idx].base_sp[lvl_idx] < job_info[idx].base_sp[lvl_idx-1])
						ShowInfo("pc_readdb_job_basehpsp: SP value at entry %d col %d is lower than previous level (job=%d,lvl=%d,oldval=%d,val=%d).\n",
							current,j+4,job_id,lvl_idx+1,job_info[idx].base_sp[lvl_idx-1],job_info[idx].base_sp[lvl_idx]);
				}
			}
		}
	}
	return true;
}
#endif

/** [Cydh]
* Reads 'job_param_db.txt' to check max. param each job and store them to job_info[].max_param.*
*/
static bool pc_readdb_job_param(char* fields[], int columns, int current)
{
	int idx, class_;
	uint16 str, agi, vit, int_, dex, luk;

	script_get_constant(trim(fields[0]),&class_);

	if ((idx = pc_class2idx(class_)) < 0) {
		ShowError("pc_readdb_job_param: Invalid job '%s'. Skipping!",fields[0]);
		return false;
	}
	str = cap_value(atoi(fields[1]),10,SHRT_MAX);
	agi = atoi(fields[2]) ? cap_value(atoi(fields[2]),10,SHRT_MAX) : str;
	vit = atoi(fields[3]) ? cap_value(atoi(fields[3]),10,SHRT_MAX) : str;
	int_ = atoi(fields[4]) ? cap_value(atoi(fields[4]),10,SHRT_MAX) : str;
	dex = atoi(fields[5]) ? cap_value(atoi(fields[5]),10,SHRT_MAX) : str;
	luk = atoi(fields[6]) ? cap_value(atoi(fields[6]),10,SHRT_MAX) : str;

	job_info[idx].max_param.str = str;
	job_info[idx].max_param.agi = agi;
	job_info[idx].max_param.vit = vit;
	job_info[idx].max_param.int_ = int_;
	job_info[idx].max_param.dex = dex;
	job_info[idx].max_param.luk = luk;
	
	return true;
}

static int pc_read_statsdb(const char *basedir, int last_s, bool silent){
	int i=1;
	char line[24000]; //FIXME this seem too big
	FILE *fp;
	
	sprintf(line, "%s/statpoint.txt", basedir);
	fp=fopen(line,"r");
	if(fp == NULL){
		if(silent==0) ShowWarning("Can't read '"CL_WHITE"%s"CL_RESET"'... Generating DB.\n",line);
		return max(last_s,i);
	} else {
		int entries=0;
		while(fgets(line, sizeof(line), fp))
		{
			int stat;
			trim(line);
			if(line[0] == '\0' || (line[0]=='/' && line[1]=='/'))
				continue;
			if ((stat=strtoul(line,NULL,10))<0)
				stat=0;
			if (i > MAX_LEVEL)
				break;
			statp[i]=stat;
			i++;
			entries++;
		}
		fclose(fp);
		ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s/%s"CL_RESET"'.\n", entries, basedir,"statpoint.txt");
	}
	return max(last_s,i);
}

/*==========================================
 * pc DB reading.
 * job_exp.txt		- required experience values
 * skill_tree.txt	- skill tree for every class
 * attr_fix.txt		- elemental adjustment table
 * job_db1.txt		- job,weight,hp_factor,hp_multiplicator,sp_factor,aspds/lvl
 * job_db2.txt		- job,stats bonuses/lvl
 * job_maxhpsp_db.txt	- strtlvl,maxlvl,job,type,values/lvl (values=hp|sp)
 *------------------------------------------*/
void pc_readdb(void) {
	int i, k, s = 1;
	const char* dbsubpath[] = {
		"",
		"/"DBIMPORT,
		//add other path here
	};
		
	//reset
	memset(job_info,0,sizeof(job_info)); // job_info table

	// Reset and read skilltree
	memset(skill_tree,0,sizeof(skill_tree));
	sv_readdb(db_path, DBPATH"skill_tree.txt", ',', 3+MAX_PC_SKILL_REQUIRE*2, 4+MAX_PC_SKILL_REQUIRE*2, -1, &pc_readdb_skilltree, 0);
	sv_readdb(db_path, DBIMPORT"/skill_tree.txt", ',', 3+MAX_PC_SKILL_REQUIRE*2, 4+MAX_PC_SKILL_REQUIRE*2, -1, &pc_readdb_skilltree, 1);

#if defined(RENEWAL_DROP) || defined(RENEWAL_EXP)
	sv_readdb(db_path, "re/level_penalty.txt", ',', 4, 4, -1, &pc_readdb_levelpenalty, 0);
	sv_readdb(db_path, DBIMPORT"/level_penalty.txt", ',', 4, 4, -1, &pc_readdb_levelpenalty, 1);
	for( k=1; k < 3; k++ ){ // fill in the blanks
		int j;
		for( j = 0; j < CLASS_ALL; j++ ){
			int tmp = 0;
			for( i = 0; i < MAX_LEVEL*2; i++ ){
				if( i == MAX_LEVEL+1 )
					tmp = level_penalty[k][j][0];// reset
				if( level_penalty[k][j][i] > 0 )
					tmp = level_penalty[k][j][i];
				else
					level_penalty[k][j][i] = tmp;
			}
		}
	}
#endif

	 // reset then read statspoint
	memset(statp,0,sizeof(statp));
	for(i=0; i<ARRAYLENGTH(dbsubpath); i++){
		uint8 n1 = strlen(db_path)+strlen(dbsubpath[i])+1;
		uint8 n2 = strlen(db_path)+strlen(DBPATH)+strlen(dbsubpath[i])+1;
		char* dbsubpath1 = (char*)aMalloc(n1+1);
		char* dbsubpath2 = (char*)aMalloc(n2+1);

		if(i==0) {
			safesnprintf(dbsubpath1,n1,"%s%s",db_path,dbsubpath[i]);
			safesnprintf(dbsubpath2,n2,"%s/%s%s",db_path,DBPATH,dbsubpath[i]);
		}
		else {
			safesnprintf(dbsubpath1,n1,"%s%s",db_path,dbsubpath[i]);
			safesnprintf(dbsubpath2,n1,"%s%s",db_path,dbsubpath[i]);
		}

		s = pc_read_statsdb(dbsubpath2,s,i);
		if (i == 0)
#ifdef RENEWAL_ASPD
			sv_readdb(dbsubpath1, "re/job_db1.txt",',',6+MAX_WEAPON_TYPE,6+MAX_WEAPON_TYPE,CLASS_COUNT,&pc_readdb_job1, i);
#else
			sv_readdb(dbsubpath1, "pre-re/job_db1.txt",',',5+MAX_WEAPON_TYPE,5+MAX_WEAPON_TYPE,CLASS_COUNT,&pc_readdb_job1, i);
#endif
		else
			sv_readdb(dbsubpath1, "job_db1.txt",',',5+MAX_WEAPON_TYPE,6+MAX_WEAPON_TYPE,CLASS_COUNT,&pc_readdb_job1, i);
		sv_readdb(dbsubpath1, "job_db2.txt",',',1,1+MAX_LEVEL,CLASS_COUNT,&pc_readdb_job2, i);
		sv_readdb(dbsubpath2, "job_exp.txt",',',4,1000+3,CLASS_COUNT*2,&pc_readdb_job_exp, i); //support till 1000lvl
#ifdef HP_SP_TABLES
		sv_readdb(dbsubpath2, "job_basehpsp_db.txt", ',', 4, 4+500, CLASS_COUNT*2, &pc_readdb_job_basehpsp, i); //Make it support until lvl 500!
#endif
		sv_readdb(dbsubpath2, "job_param_db.txt", ',', 2, PARAM_MAX+1, CLASS_COUNT, &pc_readdb_job_param, i);
		aFree(dbsubpath1);
		aFree(dbsubpath2);
	}
	
	// generate the remaining parts of the db if necessary
	k = battle_config.use_statpoint_table; //save setting
	battle_config.use_statpoint_table = 0; //temporarily disable to force pc_gets_status_point use default values
	statp[0] = 45; // seed value
	for (; s <= MAX_LEVEL; s++)
		statp[s] = statp[s-1] + pc_gets_status_point(s-1);
	battle_config.use_statpoint_table = k; //restore setting
	
	//Checking if all class have their data
	for (i = 0; i < JOB_MAX; i++) {
		int idx;
		uint16 j;
		if (!pcdb_checkid(i))
			continue;
		if (i == JOB_WEDDING || i == JOB_XMAS || i == JOB_SUMMER || i == JOB_HANBOK || i == JOB_OKTOBERFEST)
			continue; //Classes that do not need exp tables.
		idx = pc_class2idx(i);
		if (!job_info[idx].max_level[0])
			ShowWarning("Class %s (%d) does not have a base exp table.\n", job_name(i), i);
		if (!job_info[idx].max_level[1])
			ShowWarning("Class %s (%d) does not have a job exp table.\n", job_name(i), i);
		
		//Init and checking the empty value of Base HP/SP [Cydh]
		for (j = 0; j < (job_info[idx].max_level[0] ? job_info[idx].max_level[0] : MAX_LEVEL); j++) {
			if (job_info[idx].base_hp[j] == 0)
				job_info[idx].base_hp[j] = pc_calc_basehp(j+1,i);
			if (job_info[idx].base_sp[j] == 0)
				job_info[idx].base_sp[j] = pc_calc_basesp(j+1,i);
		}
	}
}

// Read MOTD on startup. [Valaris]
int pc_read_motd(void)
{
	FILE* fp;
	// clear old MOTD
	memset(motd_text, 0, sizeof(motd_text));

	// read current MOTD
	if( ( fp = fopen(motd_txt, "r") ) != NULL )
	{
		unsigned int entries = 0;

		while( entries < MOTD_LINE_SIZE && fgets(motd_text[entries], sizeof(motd_text[entries]), fp) )
		{
			char* buf = motd_text[entries];
			unsigned int lines = 0;
			size_t len;
			lines++;
			if( buf[0] == '/' && buf[1] == '/' )
				continue;
			len = strlen(buf);
			while( len && ( buf[len-1] == '\r' || buf[len-1] == '\n' ) ) // strip trailing EOL characters
				len--;
			if( len ) {
				char * ptr;
				buf[len] = 0;
				if( ( ptr = strstr(buf, " :") ) != NULL && ptr-buf >= NAME_LENGTH ) // crashes newer clients
					ShowWarning("Found sequence '"CL_WHITE" :"CL_RESET"' on line '"CL_WHITE"%u"CL_RESET"' in '"CL_WHITE"%s"CL_RESET"'. This can cause newer clients to crash.\n", lines, motd_txt);
			}
			else {// empty line
				buf[0] = ' ';
				buf[1] = 0;
			}
			entries++;
		}
		fclose(fp);
		ShowStatus("Done reading '"CL_WHITE"%u"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", entries, motd_txt);
	}
	else
		ShowWarning("File '"CL_WHITE"%s"CL_RESET"' not found.\n", motd_txt);

	return 0;
}

void pc_itemcd_do(struct map_session_data *sd, bool load) {
	int i,cursor = 0;
	struct item_cd* cd = NULL;

	if( load ) {
		if( !(cd = (struct item_cd*)idb_get(itemcd_db, sd->status.char_id)) ) {
			// no item cooldown is associated with this character
			return;
		}
		for(i = 0; i < MAX_ITEMDELAYS; i++) {
			if( cd->nameid[i] && DIFF_TICK(gettick(),cd->tick[i]) < 0 ) {
				sd->item_delay[cursor].tick = cd->tick[i];
				sd->item_delay[cursor].nameid = cd->nameid[i];
				cursor++;
			}
		}
		idb_remove(itemcd_db,sd->status.char_id);
	} else {
		if( !(cd = (struct item_cd*)idb_get(itemcd_db,sd->status.char_id)) ) {
			// create a new skill cooldown object for map storage
			CREATE( cd, struct item_cd, 1 );
			idb_put( itemcd_db, sd->status.char_id, cd );
		}
		for(i = 0; i < MAX_ITEMDELAYS; i++) {
			if( sd->item_delay[i].nameid && DIFF_TICK(gettick(),sd->item_delay[i].tick) < 0 ) {
				cd->tick[cursor] = sd->item_delay[i].tick;
				cd->nameid[cursor] = sd->item_delay[i].nameid;
				cursor++;
			}
		}
	}
	return;
}

/**
 * Add item delay to player's item delay data
 * @param sd Player
 * @param id Item data
 * @param tick Current tick
 * @param n Item index in inventory
 * @return 0: No delay, can consume item.
 *         1: Has delay, cancel consumption.
 **/
uint8 pc_itemcd_add(struct map_session_data *sd, struct item_data *id, unsigned int tick, unsigned short n) {
	int i;
	ARR_FIND(0, MAX_ITEMDELAYS, i, sd->item_delay[i].nameid == id->nameid );
	if( i == MAX_ITEMDELAYS ) /* item not found. try first empty now */
		ARR_FIND(0, MAX_ITEMDELAYS, i, !sd->item_delay[i].nameid );
	if( i < MAX_ITEMDELAYS ) {
		if( sd->item_delay[i].nameid ) {// found
			if( DIFF_TICK(sd->item_delay[i].tick, tick) > 0 ) {
				int e_tick = DIFF_TICK(sd->item_delay[i].tick, tick)/1000;
				char e_msg[CHAT_SIZE_MAX];
				if( e_tick > 99 )
					sprintf(e_msg,msg_txt(sd,379), // Item Failed. [%s] is cooling down. Wait %.1f minutes.
									itemdb_jname(sd->item_delay[i].nameid), (double)e_tick / 60);
				else
					sprintf(e_msg,msg_txt(sd,380), // Item Failed. [%s] is cooling down. Wait %d seconds.
									itemdb_jname(sd->item_delay[i].nameid), e_tick+1);
				clif_colormes(sd->fd,color_table[COLOR_YELLOW],e_msg);
				return 1; // Delay has not expired yet
			}
		} else {// not yet used item (all slots are initially empty)
			sd->item_delay[i].nameid = id->nameid;
		}
		if( !(id->nameid == ITEMID_REINS_OF_MOUNT && sd->sc.option&(OPTION_WUGRIDER|OPTION_RIDING|OPTION_DRAGON|OPTION_MADOGEAR)) )
			sd->item_delay[i].tick = tick + sd->inventory_data[n]->delay;
	} else {// should not happen
		ShowError("pc_itemcd_add: Exceeded item delay array capacity! (nameid=%hu, char_id=%d)\n", id->nameid, sd->status.char_id);
	}
	//clean up used delays so we can give room for more
	for(i = 0; i < MAX_ITEMDELAYS; i++) {
		if( DIFF_TICK(sd->item_delay[i].tick, tick) <= 0 ) {
			sd->item_delay[i].tick = 0;
			sd->item_delay[i].nameid = 0;
		}
	}
	return 0;
}

/**
 * Check if player has delay to reuse item
 * @param sd Player
 * @param id Item data
 * @param tick Current tick
 * @param n Item index in inventory
 * @return 0: No delay, can consume item.
 *         1: Has delay, cancel consumption.
 **/
uint8 pc_itemcd_check(struct map_session_data *sd, struct item_data *id, unsigned int tick, unsigned short n) {
	struct status_change *sc = NULL;

	nullpo_retr(0, sd);
	nullpo_retr(0, id);

	// Do normal delay assignment
	if (id->delay_sc <= SC_NONE || id->delay_sc >= SC_MAX || !(sc = &sd->sc))
		return pc_itemcd_add(sd, id, tick, n);

	// Send reply of delay remains
	if (sc->data[id->delay_sc]) {
		const struct TimerData *timer = get_timer(sc->data[id->delay_sc]->timer);
		clif_msg_value(sd, ITEM_REUSE_LIMIT, timer ? DIFF_TICK(timer->tick, tick) / 1000 : 99);
		return 1;
	}

	sc_start(&sd->bl, &sd->bl, (sc_type)id->delay_sc, 100, id->nameid, id->delay);
	return 0;
}

/**
* Clear the dmglog data from player
* @param sd
* @param md
**/
static void pc_clear_log_damage_sub(uint32 char_id, struct mob_data *md)
{
	uint8 i;
	ARR_FIND(0,DAMAGELOG_SIZE,i,md->dmglog[i].id == char_id);
	if (i < DAMAGELOG_SIZE) {
		md->dmglog[i].id=0;
		md->dmglog[i].dmg=0;
		md->dmglog[i].flag=0;
	}
}

/**
* Add log to player's dmglog
* @param sd
* @param id Monster's GID
**/
void pc_damage_log_add(struct map_session_data *sd, int id)
{
	uint8 i = 0;

	if (!sd)
		return;

	//Only store new data, don't need to renew the old one with same id
	for (i = 0; i < DAMAGELOG_SIZE_PC; i++) {
		if (sd->dmglog[i] == id)
			return;
		sd->dmglog[i] = id;
		return;
	}
}

/**
* Clear dmglog data from player
* @param sd
* @param id Monster's id
**/
void pc_damage_log_clear(struct map_session_data *sd, int id)
{
	uint8 i;
	struct mob_data *md = NULL;

	if (!sd)
		return;

	if (!id) {
		for (i = 0; i < DAMAGELOG_SIZE_PC; i++) {
			if( !sd->dmglog[i] )	//skip the empty value
				continue;

			if ((md = map_id2md(sd->dmglog[i])))
				pc_clear_log_damage_sub(sd->status.char_id,md);
			sd->dmglog[i] = 0;
		}
	}
	else {
		if ((md = map_id2md(id)))
			pc_clear_log_damage_sub(sd->status.char_id,md);

		ARR_FIND(0,DAMAGELOG_SIZE_PC,i,sd->dmglog[i] == id);	// find the id position
		if (i < DAMAGELOG_SIZE_PC)
			sd->dmglog[i] = 0;
	}
}

/* Status change data arrived from char-server */
void pc_scdata_received(struct map_session_data *sd) {
	// Nothing todo yet
	return;
}

/** Check expiration time and rental items
* @param sd
*/
void pc_check_expiration(struct map_session_data *sd) {
	pc_inventory_rentals(sd);

	if (sd->expiration_time != 0) { //Don't display if it's unlimited or unknow value
		time_t exp_time = sd->expiration_time;
		char tmpstr[1024];

		strftime(tmpstr,sizeof(tmpstr) - 1,msg_txt(sd,501),localtime(&exp_time)); // "Your account time limit is: %d-%m-%Y %H:%M:%S."
		clif_wis_message(sd->fd,wisp_server_name,tmpstr,strlen(tmpstr) + 1);

		pc_expire_check(sd);
	}
}

int pc_expiration_timer(int tid, unsigned int tick, int id, intptr_t data) {
	struct map_session_data *sd = map_id2sd(id);

	if( !sd ) return 0;

	sd->expiration_tid = INVALID_TIMER;

	if( sd->fd )
		clif_authfail_fd(sd->fd,10);

	map_quit(sd);

	return 0;
}

int pc_autotrade_timer(int tid, unsigned int tick, int id, intptr_t data) {
	struct map_session_data *sd = map_id2sd(id);

	if (!sd)
		return 0;

	sd->autotrade_tid = INVALID_TIMER;

	if (sd->state.autotrade&2)
		vending_reopen(sd);
	if (sd->state.autotrade&4)
		buyingstore_reopen(sd);

	if (!sd->vender_id && !sd->buyer_id) {
		sd->state.autotrade = 0;
		map_quit(sd);
	}

	return 0;
}

/* this timer exists only when a character with a expire timer > 24h is online */
/* it loops thru online players once an hour to check whether a new < 24h is available */
int pc_global_expiration_timer(int tid, unsigned int tick, int id, intptr_t data) {
	struct s_mapiterator* iter;
	struct map_session_data* sd;

	iter = mapit_getallusers();

	for( sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter) )
		if( sd->expiration_time )
			pc_expire_check(sd);

	mapit_free(iter);

  return 0;
}

void pc_expire_check(struct map_session_data *sd) {  
	/* ongoing timer */
	if( sd->expiration_tid != INVALID_TIMER )
		return;

	/* not within the next 24h, enable the global check */
	if( sd->expiration_time > (time(NULL) + ((60 * 60) * 24)) ) {

		/* global check not running, enable */
		if( pc_expiration_tid == INVALID_TIMER ) /* Starts in 1h, repeats every hour */
			pc_expiration_tid = add_timer_interval(gettick() + ((1000 * 60) * 60), pc_global_expiration_timer, 0, 0, ((1000 * 60) * 60));

		return;
	}

	sd->expiration_tid = add_timer(gettick() + (unsigned int)(sd->expiration_time - time(NULL)) * 1000, pc_expiration_timer, sd->bl.id, 0);
}

/**
* Deposit some money to bank
* @param sd
* @param money Amount of money to deposit
**/
enum e_BANKING_DEPOSIT_ACK pc_bank_deposit(struct map_session_data *sd, int money) {
	unsigned int limit_check = money + sd->bank_vault;

	if( money <= 0 || limit_check > MAX_BANK_ZENY ) {
		return BDA_OVERFLOW;
	} else if ( money > sd->status.zeny ) {
		return BDA_NO_MONEY;
	}

	if( pc_payzeny(sd,money, LOG_TYPE_BANK, NULL) )
		return BDA_NO_MONEY;

	sd->bank_vault += money;
	pc_setreg2(sd, BANK_VAULT_VAR, sd->bank_vault);
	if( save_settings&CHARSAVE_BANK )
		chrif_save(sd,0);
	return BDA_SUCCESS;
}

/**
* Withdraw money from bank
* @param sd
* @param money Amount of money that will be withdrawn
**/
enum e_BANKING_WITHDRAW_ACK pc_bank_withdraw(struct map_session_data *sd, int money) {
	unsigned int limit_check = money + sd->status.zeny;
	
	if( money <= 0 ) {
		return BWA_UNKNOWN_ERROR;
	} else if ( money > sd->bank_vault ) {
		return BWA_NO_MONEY;
	} else if ( limit_check > MAX_ZENY ) {
		/* no official response for this scenario exists. */
		clif_colormes(sd->fd,color_table[COLOR_RED],msg_txt(sd,1495)); //You can't withdraw that much money
		return BWA_UNKNOWN_ERROR;
	}
	
	if( pc_getzeny(sd,money, LOG_TYPE_BANK, NULL) )
		return BWA_NO_MONEY;
	
	sd->bank_vault -= money;
	pc_setreg2(sd, BANK_VAULT_VAR, sd->bank_vault);
	if( save_settings&CHARSAVE_BANK )
		chrif_save(sd,0);
	return BWA_SUCCESS;
}

/**
* Clear Crimson Marker data from caster
* @param sd: Player
**/
void pc_crimson_marker_clear(struct map_session_data *sd) {
	uint8 i;

	if (!sd)
		return;

	for (i = 0; i < MAX_SKILL_CRIMSON_MARKER; i++) {
		struct block_list *bl = NULL;
		if (sd->c_marker[i] && (bl = map_id2bl(sd->c_marker[i])))
			status_change_end(bl,SC_C_MARKER,INVALID_TIMER);
		sd->c_marker[i] = 0;
	}
}

/**
* Show version to player
* @param sd: Player
**/
void pc_show_version(struct map_session_data *sd) {
	const char* svn = get_svn_revision();
	char buf[CHAT_SIZE_MAX];

	if( svn[0] != UNKNOWN_VERSION )
		sprintf(buf,msg_txt(sd,1295),"SVN: r",svn); //rAthena Version SVN: r%s
	else {
		const char* git = get_git_hash();
		if( git[0] != UNKNOWN_VERSION )
			sprintf(buf,msg_txt(sd,1295),"Git Hash: ",git); //rAthena Version Git Hash: %s
		else
			sprintf(buf,"%s",msg_txt(sd,1296)); //Cannot determine SVN/Git version.
	}
	clif_displaymessage(sd->fd,buf);
}

/**
 * Run bonus_script on player
 * @param sd
 * @author [Cydh]
 **/
void pc_bonus_script(struct map_session_data *sd) {
	int now = gettick();
	struct linkdb_node *node = NULL, *next = NULL;

	if (!sd || !(node = sd->bonus_script.head))
		return;

	while (node) {
		struct s_bonus_script_entry *entry = NULL;
		next = node->next;

		if ((entry = (struct s_bonus_script_entry *)node->data)) {
			// Only start timer for new bonus_script
			if (entry->tid == INVALID_TIMER) {
				if (entry->icon != SI_BLANK) // Gives status icon if exist
					clif_status_change(&sd->bl, entry->icon, 1, entry->tick, 1, 0, 0);

				entry->tick += now;
				entry->tid = add_timer(entry->tick, pc_bonus_script_timer, sd->bl.id, (intptr_t)entry);
			}

			if (entry->script)
				run_script(entry->script, 0, sd->bl.id, 0);
			else
				ShowError("pc_bonus_script: The script has been removed somewhere. \"%s\"\n", StringBuf_Value(entry->script_buf));
		}

		node = next;
	}
}

/**
 * Add bonus_script to player
 * @param sd Player
 * @param script_str Script string
 * @param dur Duration in ms
 * @param icon SI
 * @param flag Flags @see enum e_bonus_script_flags
 * @param type 0 - None, 1 - Buff, 2 - Debuff
 * @return New created entry pointer or NULL if failed or NULL if duplicate fail
 * @author [Cydh]
 **/
struct s_bonus_script_entry *pc_bonus_script_add(struct map_session_data *sd, const char *script_str, uint32 dur, enum si_type icon, uint16 flag, uint8 type) {
	struct script_code *script = NULL;
	struct linkdb_node *node = NULL;
	struct s_bonus_script_entry *entry = NULL;

	if (!sd)
		return NULL;
	
	if (!(script = parse_script(script_str, "bonus_script", 0, SCRIPT_IGNORE_EXTERNAL_BRACKETS))) {
		ShowError("pc_bonus_script_add: Failed to parse script '%s' (CID:%d).\n", script_str, sd->status.char_id);
		return NULL;
	}

	// Duplication checks
	if ((node = sd->bonus_script.head)) {
		while (node) {
			entry = (struct s_bonus_script_entry *)node->data;
			if (strcmpi(script_str, StringBuf_Value(entry->script_buf)) == 0) {
				int newdur = gettick() + dur;
				if (flag&BSF_FORCE_REPLACE && entry->tick < newdur) { // Change duration
					settick_timer(entry->tid, newdur);
					script_free_code(script);
					return NULL;
				}
				else if (flag&BSF_FORCE_DUPLICATE) // Allow duplicate
					break;
				else { // No duplicate bonus
					script_free_code(script);
					return NULL;
				}
			}
			node = node->next;
		}
	}

	CREATE(entry, struct s_bonus_script_entry, 1);

	entry->script_buf = StringBuf_Malloc();
	StringBuf_AppendStr(entry->script_buf, script_str);
	entry->tid = INVALID_TIMER;
	entry->flag = flag;
	entry->icon = icon;
	entry->tick = dur; // Use duration first, on run change to expire time
	entry->type = type;
	entry->script = script;
	sd->bonus_script.count++;
	return entry;
}

/**
* Remove bonus_script data from player
* @param sd: Target player
* @param list: Bonus script entry from player
* @author [Cydh]
**/
void pc_bonus_script_free_entry(struct map_session_data *sd, struct s_bonus_script_entry *entry) {
	if (entry->tid != INVALID_TIMER)
		delete_timer(entry->tid, pc_bonus_script_timer);

	if (entry->script)
		script_free_code(entry->script);

	if (entry->script_buf)
		StringBuf_Free(entry->script_buf);

	if (sd) {
		if (entry->icon != SI_BLANK)
			clif_status_load(&sd->bl, entry->icon, 0);
		if (sd->bonus_script.count > 0)
			sd->bonus_script.count--;
	}
	aFree(entry);
}

/**
 * Do final process if no entry left
 * @param sd
 **/
static void inline pc_bonus_script_check_final(struct map_session_data *sd) {
	if (sd->bonus_script.count == 0) {
		if (sd->bonus_script.head && sd->bonus_script.head->data)
			pc_bonus_script_free_entry(sd, (struct s_bonus_script_entry *)sd->bonus_script.head->data);
		linkdb_final(&sd->bonus_script.head);
	}
}

/**
* Timer for bonus_script
* @param tid
* @param tick
* @param id
* @param data
* @author [Cydh]
**/
int pc_bonus_script_timer(int tid, unsigned int tick, int id, intptr_t data) {
	struct map_session_data *sd;
	struct s_bonus_script_entry *entry = (struct s_bonus_script_entry *)data;

	sd = map_id2sd(id);
	if (!sd) {
		ShowError("pc_bonus_script_timer: Null pointer id: %d tid: %d\n", id, tid);
		return 0;
	}

	if (tid == INVALID_TIMER)
		return 0;

	if (!sd->bonus_script.head || entry == NULL) {
		ShowError("pc_bonus_script_timer: Invalid entry pointer 0x%08X!\n", entry);
		return 0;
	}

	linkdb_erase(&sd->bonus_script.head, (void *)((intptr_t)entry));
	pc_bonus_script_free_entry(sd, entry);
	pc_bonus_script_check_final(sd);
	status_calc_pc(sd,SCO_NONE);
	return 0;
}

/**
* Check then clear all active timer(s) of bonus_script data from player based on reason
* @param sd: Target player
* @param flag: Reason to remove the bonus_script. e_bonus_script_flags or e_bonus_script_types
* @author [Cydh]
**/
void pc_bonus_script_clear(struct map_session_data *sd, uint16 flag) {
	struct linkdb_node *node = NULL;
	uint16 count = 0;

	if (!sd || !(node = sd->bonus_script.head))
		return;

	while (node) {
		struct linkdb_node *next = node->next;
		struct s_bonus_script_entry *entry = (struct s_bonus_script_entry *)node->data;

		if (entry && (
				(flag == BSF_PERMANENT) ||					 // Remove all with permanent bonus
				(!flag && !(entry->flag&BSF_PERMANENT)) ||	 // Remove all WITHOUT permanent bonus
				(flag&entry->flag) ||						 // Matched flag
				(flag&BSF_REM_BUFF   && entry->type == 1) || // Remove buff
				(flag&BSF_REM_DEBUFF && entry->type == 2)	 // Remove debuff
				)
			)
		{
			linkdb_erase(&sd->bonus_script.head, (void *)((intptr_t)entry));
			pc_bonus_script_free_entry(sd, entry);
			count++;
		}

		node = next;
	}

	pc_bonus_script_check_final(sd);

	if (count && !(flag&BSF_REM_ON_LOGOUT)) //Don't need to do this if log out
		status_calc_pc(sd,SCO_NONE);
}

/** [Cydh]
 * Gives/removes SC_BASILICA when player steps in/out the cell with 'cell_basilica'
 * @param sd: Target player
 */
void pc_cell_basilica(struct map_session_data *sd) {
	nullpo_retv(sd);
	
	if (!map_getcell(sd->bl.m,sd->bl.x,sd->bl.y,CELL_CHKBASILICA)) {
		if (&sd->sc && sd->sc.data[SC_BASILICA])
			status_change_end(&sd->bl,SC_BASILICA,INVALID_TIMER);
	}
	else if (!(&sd->sc) || !sd->sc.data[SC_BASILICA])
		sc_start(&sd->bl,&sd->bl,SC_BASILICA,100,0,-1);
}

/** [Cydh]
 * Get maximum specified parameter for specified class
 * @param class_: sd->class
 * @param sex: sd->status.sex
 * @param flag: parameter will be checked
 * @return max_param
 */
short pc_maxparameter(struct map_session_data *sd, enum e_params param) {
	int idx = -1, class_ = sd->class_;

	if ((idx = pc_class2idx(pc_mapid2jobid(class_,sd->status.sex))) >= 0) {
		short max_param = 0;
		switch (param) {
			case PARAM_STR: max_param = job_info[idx].max_param.str; break;
			case PARAM_AGI: max_param = job_info[idx].max_param.agi; break;
			case PARAM_VIT: max_param = job_info[idx].max_param.vit; break;
			case PARAM_INT: max_param = job_info[idx].max_param.int_; break;
			case PARAM_DEX: max_param = job_info[idx].max_param.dex; break;
			case PARAM_LUK: max_param = job_info[idx].max_param.luk; break;
		}
		if (max_param > 0)
			return max_param;
	}

	return ((class_&MAPID_UPPERMASK) == MAPID_KAGEROUOBORO || (class_&MAPID_UPPERMASK) == MAPID_REBELLION) ? battle_config.max_extended_parameter :
		((class_&JOBL_THIRD) ? ((class_&JOBL_UPPER) ? battle_config.max_third_trans_parameter : ((class_&JOBL_BABY) ? battle_config.max_baby_third_parameter : battle_config.max_third_parameter)) : 
		((class_&JOBL_BABY) ? battle_config.max_baby_parameter :
		((class_&JOBL_UPPER) ? battle_config.max_trans_parameter : battle_config.max_parameter)));
}

/**
* Get max ASPD for player based on Class
* @param sd Player
* @return ASPD
*/
short pc_maxaspd(struct map_session_data *sd) {
	nullpo_ret(sd);

	return (( sd->class_&JOBL_THIRD) ? battle_config.max_third_aspd : (
			((sd->class_&MAPID_UPPERMASK) == MAPID_KAGEROUOBORO || (sd->class_&MAPID_UPPERMASK) == MAPID_REBELLION) ? battle_config.max_extended_aspd :
			battle_config.max_aspd ));
}

/**
* Calculates total item-group related bonuses for the given item
* @param sd Player
* @param nameid Item ID
* @return Heal rate
**/
short pc_get_itemgroup_bonus(struct map_session_data* sd, unsigned short nameid) {
	short bonus = 0;
	uint8 i;

	if (!sd->itemgrouphealrate_count)
		return bonus;
	for (i = 0; i < sd->itemgrouphealrate_count; i++) {
		uint16 group_id = sd->itemgrouphealrate[i]->group_id, j;
		struct s_item_group_db *group = NULL;
		if (!group_id || !(group = itemdb_group_exists(group_id)))
			continue;
		
		for (j = 0; j < group->random[0].data_qty; j++) {
			if (group->random[0].data[j].nameid == nameid) {
				bonus += sd->itemgrouphealrate[i]->rate;
				break;
			}
		}
	}
	return bonus;
}

/**
* Calculates total item-group related bonuses for the given item group
* @param sd Player
* @param group_id Item Group ID
* @return Heal rate
**/
short pc_get_itemgroup_bonus_group(struct map_session_data* sd, uint16 group_id) {
	short bonus = 0;
	uint8 i;

	if (!sd->itemgrouphealrate_count)
		return bonus;
	for (i = 0; i < sd->itemgrouphealrate_count; i++) {
		if (sd->itemgrouphealrate[i]->group_id == group_id)
			return sd->itemgrouphealrate[i]->rate;
	}
	return bonus;
}

/**
* Check if player's equip index in same specified position, like for 2-Handed weapon & Heagdear (inc. costume)
* @param eqi Item EQI of enum equip_index
* @param *equip_index Player's equip_index[]
* @param index Known index item in inventory from sd->equip_index[] to compare with specified EQI in *equip_index
* @return True if item in same inventory index, False if doesn't
*/
bool pc_is_same_equip_index(enum equip_index eqi, short *equip_index, short index) {
	if (index < 0 || index >= MAX_INVENTORY)
		return true;
	// Dual weapon checks
	if (eqi == EQI_HAND_R && equip_index[EQI_HAND_L] == index)
		return true;
	// Headgear with Mid & Low location
	if (eqi == EQI_HEAD_MID && equip_index[EQI_HEAD_LOW] == index)
		return true;
	// Headgear with Top & Mid or Low location
	if (eqi == EQI_HEAD_TOP && (equip_index[EQI_HEAD_MID] == index || equip_index[EQI_HEAD_LOW] == index))
		return true;
	// Headgear with Mid & Low location
	if (eqi == EQI_COSTUME_MID && equip_index[EQI_COSTUME_LOW] == index)
		return true;
	// Headgear with Top & Mid or Low location
	if (eqi == EQI_COSTUME_TOP && (equip_index[EQI_COSTUME_MID] == index || equip_index[EQI_COSTUME_LOW] == index))
		return true;
	return false;
}

/**
 * Generate Unique item ID for player
 * @param sd : Player
 * @return A generated Unique item ID
 */
uint64 pc_generate_unique_id(struct map_session_data *sd) {
	nullpo_ret(sd);
	return ((uint64)sd->status.char_id << 32) | sd->status.uniqueitem_counter++;
}

/**
 * Validating skill from player after logged on
 * @param sd
 **/
void pc_validate_skill(struct map_session_data *sd) {
	if (sd) {
		uint16 i = 0, count = 0;
		struct s_skill tmp_skills[MAX_SKILL] = {{ 0 }};

		memcpy(tmp_skills, sd->status.skill, sizeof(sd->status.skill));
		memset(sd->status.skill, 0, sizeof(sd->status.skill));

		for (i = 0; i < MAX_SKILL; i++) {
			uint16 idx = 0;
			if (tmp_skills[i].id == 0 || tmp_skills[i].lv == 0)
				continue;
			if ((idx = skill_get_index(tmp_skills[i].id))) {
				memcpy(&sd->status.skill[idx], &tmp_skills[i], sizeof(tmp_skills[i]));
				count++;
			}
			else
				ShowWarning("pc_validate_skill: Removing invalid skill '%d' from player (AID=%d CID=%d).\n", tmp_skills[i].id, sd->status.account_id, sd->status.char_id);
		}
	}
}

/*==========================================
 * pc Init/Terminate
 *------------------------------------------*/
void do_final_pc(void) {
	db_destroy(itemcd_db);
	do_final_pc_groups();

	ers_destroy(pc_sc_display_ers);
	ers_destroy(pc_itemgrouphealrate_ers);
	ers_destroy(num_reg_ers);
	ers_destroy(str_reg_ers);
}

void do_init_pc(void) {

	itemcd_db = idb_alloc(DB_OPT_RELEASE_DATA);

	pc_readdb();
	pc_read_motd(); // Read MOTD [Valaris]

	add_timer_func_list(pc_invincible_timer, "pc_invincible_timer");
	add_timer_func_list(pc_eventtimer, "pc_eventtimer");
	add_timer_func_list(pc_inventory_rental_end, "pc_inventory_rental_end");
	add_timer_func_list(pc_calc_pvprank_timer, "pc_calc_pvprank_timer");
	add_timer_func_list(pc_autosave, "pc_autosave");
	add_timer_func_list(pc_spiritball_timer, "pc_spiritball_timer");
	add_timer_func_list(pc_follow_timer, "pc_follow_timer");
	add_timer_func_list(pc_endautobonus, "pc_endautobonus");
	add_timer_func_list(pc_spiritcharm_timer, "pc_spiritcharm_timer");
	add_timer_func_list(pc_global_expiration_timer, "pc_global_expiration_timer");
	add_timer_func_list(pc_expiration_timer, "pc_expiration_timer");
	add_timer_func_list(pc_autotrade_timer, "pc_autotrade_timer");

	add_timer(gettick() + autosave_interval, pc_autosave, 0, 0);

	// 0=day, 1=night [Yor]
	night_flag = battle_config.night_at_start ? 1 : 0;

	if (battle_config.day_duration > 0 && battle_config.night_duration > 0) {
		int day_duration = battle_config.day_duration;
		int night_duration = battle_config.night_duration;
		// add night/day timer [Yor]
		add_timer_func_list(map_day_timer, "map_day_timer");
		add_timer_func_list(map_night_timer, "map_night_timer");

		day_timer_tid   = add_timer_interval(gettick() + (night_flag ? 0 : day_duration) + night_duration, map_day_timer,   0, 0, day_duration + night_duration);
		night_timer_tid = add_timer_interval(gettick() + day_duration + (night_flag ? night_duration : 0), map_night_timer, 0, 0, day_duration + night_duration);
	}

	do_init_pc_groups();

	pc_sc_display_ers = ers_new(sizeof(struct sc_display_entry), "pc.c:pc_sc_display_ers", ERS_OPT_FLEX_CHUNK);
	pc_itemgrouphealrate_ers = ers_new(sizeof(struct s_pc_itemgrouphealrate), "pc.c:pc_itemgrouphealrate_ers", ERS_OPT_NONE);
	num_reg_ers = ers_new(sizeof(struct script_reg_num), "pc.c:num_reg_ers", ERS_OPT_CLEAN|ERS_OPT_FLEX_CHUNK);
	str_reg_ers = ers_new(sizeof(struct script_reg_str), "pc.c:str_reg_ers", ERS_OPT_CLEAN|ERS_OPT_FLEX_CHUNK);

	ers_chunk_size(pc_sc_display_ers, 150);
	ers_chunk_size(num_reg_ers, 300);
	ers_chunk_size(str_reg_ers, 50);
}
