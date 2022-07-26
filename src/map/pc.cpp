// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "pc.hpp"

#include <map>

#include <math.h>
#include <stdlib.h>

#include "../common/cbasetypes.hpp"
#include "../common/core.hpp" // get_svn_revision()
#include "../common/database.hpp"
#include "../common/ers.hpp"  // ers_destroy
#include "../common/malloc.hpp"
#include "../common/mmo.hpp" //NAME_LENGTH
#include "../common/nullpo.hpp"
#include "../common/random.hpp"
#include "../common/showmsg.hpp"
#include "../common/socket.hpp" // session[]
#include "../common/strlib.hpp" // safestrncpy()
#include "../common/timer.hpp"
#include "../common/utilities.hpp"
#include "../common/utils.hpp"

#include "achievement.hpp"
#include "atcommand.hpp" // get_atcommand_level()
#include "battle.hpp" // battle_config
#include "battleground.hpp"
#include "buyingstore.hpp"  // struct s_buyingstore
#include "channel.hpp"
#include "chat.hpp"
#include "chrif.hpp"
#include "clan.hpp"
#include "clif.hpp"
#include "date.hpp" // is_day_of_*()
#include "duel.hpp"
#include "elemental.hpp"
#include "guild.hpp"
#include "homunculus.hpp"
#include "instance.hpp"
#include "intif.hpp"
#include "itemdb.hpp" // MAX_ITEMGROUP
#include "log.hpp"
#include "map.hpp"
#include "mercenary.hpp"
#include "mob.hpp"
#include "npc.hpp"
#include "party.hpp" // party_search()
#include "pc_groups.hpp"
#include "pet.hpp" // pet_unlocktarget()
#include "quest.hpp"
#include "script.hpp" // struct script_reg, struct script_regstr
#include "searchstore.hpp"  // struct s_search_store_info
#include "status.hpp" // OPTION_*, struct weapon_atk
#include "storage.hpp"
#include "unit.hpp" // unit_stop_attack(), unit_stop_walking()
#include "vending.hpp" // struct s_vending

using namespace rathena;

JobDatabase job_db;

int pc_split_atoui(char* str, unsigned int* val, char sep, int max);
static inline bool pc_attendance_rewarded_today( struct map_session_data* sd );

#define PVP_CALCRANK_INTERVAL 1000	// PVP calculation interval

PlayerStatPointDatabase statpoint_db;

SkillTreeDatabase skill_tree_db;

// timer for night.day implementation
int day_timer_tid = INVALID_TIMER;
int night_timer_tid = INVALID_TIMER;

struct eri *pc_sc_display_ers = NULL;
struct eri *num_reg_ers;
struct eri *str_reg_ers;
int pc_expiration_tid = INVALID_TIMER;

struct fame_list smith_fame_list[MAX_FAME_LIST];
struct fame_list chemist_fame_list[MAX_FAME_LIST];
struct fame_list taekwon_fame_list[MAX_FAME_LIST];

const std::string AttendanceDatabase::getDefaultLocation(){
	return std::string(db_path) + "/attendance.yml";
}

/**
 * Reads and parses an entry from the attendance_db.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 AttendanceDatabase::parseBodyNode(const ryml::NodeRef& node){
	uint32 start;

	if( !this->asUInt32( node, "Start", start ) ){
		return 0;
	}

	std::shared_ptr<s_attendance_period> attendance_period = this->find( start );
	bool exists = attendance_period != nullptr;

	if( !exists ){
		if( !this->nodeExists( node, "End" ) ){
			this->invalidWarning( node, "Node \"End\" is missing.\n" );
			return 0;
		}

		if( !this->nodeExists( node, "Rewards" ) ){
			this->invalidWarning( node, "Node \"Rewards\" is missing.\n" );
			return 0;
		}

		attendance_period = std::make_shared<s_attendance_period>();

		attendance_period->start = start;
	}

	// If it does not exist yet, we need to check it for sure
	bool requiresCollisionDetection = !exists;

	if( this->nodeExists( node, "End" ) ){
		uint32 end;

		if( !this->asUInt32( node, "End", end ) ){
			return 0;
		}

		// If the period is outdated already, we do not even bother parsing
		if( end < date_get( DT_YYYYMMDD ) ){
			this->invalidWarning( node, "Node \"End\" date %u has already passed, skipping.\n", end );
			return 0;
		}

		if( !exists || attendance_period->end != end ){
			requiresCollisionDetection = true;
			attendance_period->end = end;
		}
	}

	// Collision detection
	if( requiresCollisionDetection ){
		bool collision = false;

		for( std::pair<const uint32,std::shared_ptr<s_attendance_period>>& pair : *this ){
			std::shared_ptr<s_attendance_period> period = pair.second;

			if( exists && period->start == attendance_period->start ){
				// Dont compare to yourself
				continue;
			}

			// Check if start is inside another period
			if( period->start <= attendance_period->start && start <= period->end ){
				this->invalidWarning( node, "Node \"Start\" period %u intersects with period %u-%u, skipping.\n", attendance_period->start, period->start, period->end );
				collision = true;
				break;
			}

			// Check if end is inside another period
			if( period->start <= attendance_period->end && attendance_period->end <= period->end ){
				this->invalidWarning( node, "Node \"End\" period %u intersects with period %u-%u.\n", attendance_period->start, period->start, period->end );
				collision = true;
				break;
			}
		}

		if( collision ){
			return 0;
		}
	}

	if( this->nodeExists( node, "Rewards" ) ){
		const auto& rewardsNode = node["Rewards"];

		for( const auto& rewardNode : rewardsNode ){
			uint32 day;

			if( !this->asUInt32( rewardNode, "Day", day ) ){
				continue;
			}

			day -= 1;

			std::shared_ptr<s_attendance_reward> reward = util::map_find( attendance_period->rewards, day );
			bool reward_exists = reward != nullptr;

			if( !reward_exists ){
				if( !this->nodeExists( rewardNode, "ItemId" ) ){
					this->invalidWarning( rewardNode, "Node \"ItemId\" is missing.\n" );
					return 0;
				}

				reward = std::make_shared<s_attendance_reward>();
			}

			if( this->nodeExists( rewardNode, "ItemId" ) ){
				t_itemid item_id;

				if( !this->asUInt32( rewardNode, "ItemId", item_id ) ){
					continue;
				}

				if( item_id == 0 || !itemdb_exists( item_id ) ){
					ShowError( "pc_attendance_load: Unknown item ID %u for day %d.\n", item_id, day + 1 );
					continue;
				}

				reward->item_id = item_id;
			}

			if( this->nodeExists( rewardNode, "Amount" ) ){
				uint16 amount;

				if( !this->asUInt16( rewardNode, "Amount", amount ) ){
					continue;
				}

				if( amount == 0 ){
					ShowWarning( "pc_attendance_load: Invalid reward count %hu for day %d. Defaulting to 1...\n", amount, day + 1 );
					amount = 1;
				}else if( amount > MAX_AMOUNT ){
					ShowError( "pc_attendance_load: Reward count %hu above maximum %hu for day %d. Defaulting to %hu...\n", amount, MAX_AMOUNT, day + 1, MAX_AMOUNT );
					amount = MAX_AMOUNT;
				}

				reward->amount = amount;
			}else{
				if( !reward_exists ){
					reward->amount = 1;
				}
			}

			if( !reward_exists ){
				attendance_period->rewards[day] = reward;
			}
		}

		bool missing_day = false;

		for( int day = 0; day < attendance_period->rewards.size(); day++ ){
			if( attendance_period->rewards.find( day ) == attendance_period->rewards.end() ){
				ShowError( "pc_attendance_load: Reward for day %d is missing.\n", day + 1 );
				missing_day = true;
				break;
			}
		}

		if( missing_day ){
			return 0;
		}
	}

	if( !exists ){
		this->put( start, attendance_period );
	}

	return 1;
}

AttendanceDatabase attendance_db;

const std::string PenaltyDatabase::getDefaultLocation(){
	return std::string( db_path ) + "/level_penalty.yml";
}

uint64 PenaltyDatabase::parseBodyNode(const ryml::NodeRef& node){
	std::string type_constant;

	if( !this->asString( node, "Type", type_constant ) ){
		return 0;
	}

	int64 constant_value;

	if( !script_get_constant( ( "PENALTY_" + type_constant ).c_str(), &constant_value ) ){
		this->invalidWarning( node["Type"], "Unknown penalty type \"%s\".\n", type_constant.c_str() );
		return 0;
	}

	if( constant_value < PENALTY_NONE || constant_value > PENALTY_MAX ){
		this->invalidWarning( node["Type"], "Invalid penalty type \"%s\".\n", type_constant.c_str() );
		return 0;
	}

	e_penalty_type type = static_cast<e_penalty_type>( constant_value );

	std::shared_ptr<s_penalty> penalty = this->find( type );
	bool exists = penalty != nullptr;

	if( !exists ){
		penalty = std::make_shared<s_penalty>();
		penalty->type = type;

		for( int i = 0, max = ARRAYLENGTH( penalty->rate ); i < max; i++ ){
			penalty->rate[i] = UINT16_MAX;
		}
	}

	if( this->nodeExists( node, "LevelDifferences" ) ){
		for( const auto& levelNode : node["LevelDifferences"] ){
			if( !this->nodesExist( levelNode, { "Difference", "Rate" } ) ){
				return 0;
			}

			int32 difference;

			if( !this->asInt32( levelNode, "Difference", difference ) ){
				return 0;
			}

			if( std::abs( difference ) > MAX_LEVEL ){
				this->invalidWarning( levelNode["Difference"], "Level difference %d is bigger than maximum level %d.\n", difference, MAX_LEVEL );
				return 0;
			}

			uint16 rate;

			if( !this->asUInt16Rate( levelNode, "Rate", rate ) ){
				return 0;
			}

			penalty->rate[difference + MAX_LEVEL - 1] = rate;
		}
	}

	if( !exists ){
		this->put( type, penalty );
	}

	return 1;
}

void PenaltyDatabase::loadingFinished(){
	for( const auto& pair : *this ){
		for( int i = MAX_LEVEL - 1, max = ARRAYLENGTH( pair.second->rate ), last_rate = 100; i < max; i++ ){
			uint16 rate = pair.second->rate[i];

			// Check if it has been defined
			if( rate == UINT16_MAX ){
				pair.second->rate[i] = last_rate;
			}else{
				last_rate = rate;
			}
		}

		for( int i = MAX_LEVEL - 1, last_rate = 100; i >= 0; i-- ){
			uint16 rate = pair.second->rate[i];

			// Check if it has been defined
			if( rate == UINT16_MAX ){
				pair.second->rate[i] = last_rate;
			}else{
				last_rate = rate;
			}
		}
	}

	TypesafeYamlDatabase::loadingFinished();
}

PenaltyDatabase penalty_db;

#define MOTD_LINE_SIZE 128
static char motd_text[MOTD_LINE_SIZE][CHAT_SIZE_MAX]; // Message of the day buffer [Valaris]

bool reg_load;

/**
 * Translation table from athena equip index to aegis bitmask
*/
unsigned int equip_bitmask[EQI_MAX] = {
	EQP_ACC_L,				// EQI_ACC_L
	EQP_ACC_R,				// EQI_ACC_R
	EQP_SHOES,				// EQI_SHOES
	EQP_GARMENT,			// EQI_GARMENT
	EQP_HEAD_LOW,			// EQI_HEAD_LOW
	EQP_HEAD_MID,			// EQI_HEAD_MID
	EQP_HEAD_TOP,			// EQI_HEAD_TOP
	EQP_ARMOR,				// EQI_ARMOR
	EQP_HAND_L,				// EQI_HAND_L
	EQP_HAND_R,				// EQI_HAND_R
	EQP_COSTUME_HEAD_TOP,	// EQI_COSTUME_HEAD_TOP
	EQP_COSTUME_HEAD_MID,	// EQI_COSTUME_HEAD_MID
	EQP_COSTUME_HEAD_LOW,	// EQI_COSTUME_HEAD_LOW
	EQP_COSTUME_GARMENT,	// EQI_COSTUME_GARMENT
	EQP_AMMO,				// EQI_AMMO
	EQP_SHADOW_ARMOR,		// EQI_SHADOW_ARMOR
	EQP_SHADOW_WEAPON,		// EQI_SHADOW_WEAPON
	EQP_SHADOW_SHIELD,		// EQI_SHADOW_SHIELD
	EQP_SHADOW_SHOES,		// EQI_SHADOW_SHOES
	EQP_SHADOW_ACC_R,		// EQI_SHADOW_ACC_R
	EQP_SHADOW_ACC_L		// EQI_SHADOW_ACC_L
};

//Links related info to the sd->hate_mob[]/sd->feel_map[] entries
const struct sg_data sg_info[MAX_PC_FEELHATE] = {
		{ SG_SUN_ANGER, SG_SUN_BLESS, SG_SUN_COMFORT, "PC_FEEL_SUN", "PC_HATE_MOB_SUN", is_day_of_sun },
		{ SG_MOON_ANGER, SG_MOON_BLESS, SG_MOON_COMFORT, "PC_FEEL_MOON", "PC_HATE_MOB_MOON", is_day_of_moon },
		{ SG_STAR_ANGER, SG_STAR_BLESS, SG_STAR_COMFORT, "PC_FEEL_STAR", "PC_HATE_MOB_STAR", is_day_of_star }
	};

void pc_set_reg_load( bool val ){
	reg_load = val;
}

/**
 * Item Cool Down Delay Saving
 * Struct item_cd is not a member of struct map_session_data
 * to keep cooldowns in memory between player log-ins.
 * All cooldowns are reset when server is restarted.
 **/
DBMap* itemcd_db = NULL; // char_id -> struct item_cd
struct item_cd {
	t_tick tick[MAX_ITEMDELAYS]; //tick
	t_itemid nameid[MAX_ITEMDELAYS]; //item id
};

/**
* Converts a class to its array index for CLASS_COUNT defined arrays.
* Note that it does not do a validity check for speed purposes, where parsing
* player input make sure to use a pcdb_checkid first!
* @param class_ Job ID see enum e_job
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
int pc_get_group_id(struct map_session_data *sd) {
	return sd->group_id;
}

/** Get player's group Level
* @param sd
* @return Group Level
*/
int pc_get_group_level(struct map_session_data *sd) {
	return sd->group->level;
}

/**
 * Remove a player from queue after timeout
 * @param tid: Timer ID
 * @param tick: Timer
 * @param id: ID
 * @return 0 on success or 1 otherwise
 */
static TIMER_FUNC(pc_on_expire_active)
{
	map_session_data *sd = (map_session_data *)data;

	nullpo_retr(1, sd);

	sd->tid_queue_active = INVALID_TIMER;

	bg_queue_leave(sd);
	clif_bg_queue_entry_init(sd);
	return 0;
}

/**
 * Function used to set timer externally
 * @param sd: Player data
 */
void pc_set_bg_queue_timer(map_session_data *sd) {
	nullpo_retv(sd);

	if (sd->tid_queue_active != INVALID_TIMER) {
		delete_timer(sd->tid_queue_active, pc_on_expire_active);
		sd->tid_queue_active = INVALID_TIMER;
	}

	sd->tid_queue_active = add_timer(gettick() + 20000, pc_on_expire_active, 0, (intptr_t)sd);
}

/**
 * Function used to delete timer externally
 * @param sd: Player data
 */
void pc_delete_bg_queue_timer(map_session_data *sd) {
	nullpo_retv(sd);

	if (sd->tid_queue_active != INVALID_TIMER) {
		delete_timer(sd->tid_queue_active, pc_on_expire_active);
		sd->tid_queue_active = INVALID_TIMER;
	}
}

static TIMER_FUNC(pc_invincible_timer){
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

static TIMER_FUNC(pc_spiritball_timer){
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

/**
 * Adds a soulball to player
 * @param sd: Player data
 * @param max: Max amount of soulballs
 */
int pc_addsoulball(map_session_data *sd, int max)
{
	nullpo_ret(sd);

	status_change *sc = status_get_sc(&sd->bl);

	if (sc == nullptr || sc->data[SC_SOULENERGY] == nullptr) {
		sc_start(&sd->bl, &sd->bl, SC_SOULENERGY, 100, 0, skill_get_time2(SP_SOULCOLLECT, 1));
		sd->soulball = 0;
	}

	max = min(max, MAX_SOUL_BALL);
	sd->soulball = cap_value(sd->soulball, 0, MAX_SOUL_BALL);

	if (sd->soulball && sd->soulball >= max)
		sd->soulball--;

	sd->soulball++;
	sc_start(&sd->bl, &sd->bl, SC_SOULENERGY, 100, sd->soulball, skill_get_time2(SP_SOULCOLLECT, 1));
	clif_soulball(sd);

	return 0;
}

/**
 * Removes number of soulball from player
 * @param sd: Player data
 * @param count: Amount to remove
 * @param type: true = doesn't give client effect
 */
int pc_delsoulball(map_session_data *sd, int count, bool type)
{
	nullpo_ret(sd);

	if (count <= 0)
		return 0;

	status_change *sc = status_get_sc(&sd->bl);

	if (sd->soulball <= 0 || sc == nullptr || sc->data[SC_SOULENERGY] == nullptr) {
		sd->soulball = 0;
	}else{
		sd->soulball -= cap_value(count, 0, sd->soulball);
		if (sd->soulball == 0)
			status_change_end(&sd->bl, SC_SOULENERGY, INVALID_TIMER);
		else
			sc->data[SC_SOULENERGY]->val1 = sd->soulball;
	}

	if (!type)
		clif_soulball(sd);

	return 0;
}

/**
* Adds servantballs to a player
* @param sd: Player data
* @param amount: Amount to add
*/
void pc_addservantball( struct map_session_data& sd, int count ){
	sd.servantball = cap_value( sd.servantball + count, 0, MAX_SERVANTBALL );

	clif_servantball( sd );
}

/**
* Removes number of servantballs from player
* @param sd: Player data
* @param count: Amount to remove
*/
void pc_delservantball( struct map_session_data& sd, int count ){
	sd.servantball = cap_value( sd.servantball - count, 0, MAX_SERVANTBALL );

	clif_servantball( sd );
}

/**
* Adds abyssballs to a player
* @param sd: Player data
* @param amount: Amount to add
*/
void pc_addabyssball( struct map_session_data& sd, int count ){
	sd.abyssball = cap_value( sd.abyssball + count, 0, MAX_ABYSSBALL );

	clif_abyssball( sd );
}

/**
* Removes number of abyssballs from player
* @param sd: Player data
* @param count: Amount to remove
*/
void pc_delabyssball( struct map_session_data& sd, int count ){
	sd.abyssball = cap_value( sd.abyssball - count, 0, MAX_ABYSSBALL );

	clif_abyssball( sd );
}

/**
* Increases a player's fame points and displays a notice to him
* @param sd Player
* @param count Fame point
* @return true: on success, false: on error
*/
bool pc_addfame(map_session_data &sd, int count)
{
	enum e_rank ranktype;

	sd.status.fame += count;
	if (sd.status.fame > MAX_FAME)
		sd.status.fame = MAX_FAME;

	switch(sd.class_&MAPID_UPPERMASK){
		case MAPID_BLACKSMITH:	ranktype = RANK_BLACKSMITH; break;
		case MAPID_ALCHEMIST:	ranktype = RANK_ALCHEMIST; break;
		case MAPID_TAEKWON:		ranktype = RANK_TAEKWON; break;
		default:
			ShowWarning( "pc_addfame: Trying to add fame to class '%s'(%d).\n", job_name(sd.status.class_), sd.status.class_ );
			return false;
	}

	clif_update_rankingpoint(sd, ranktype, count);
	chrif_updatefamelist(sd, ranktype);
	return true;
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
		sd->status.ap = (status->ap < b_status->ap)?b_status->ap:status->ap;
	}
}

/*==========================================
	Rental System
 *------------------------------------------*/

/**
 * Ends a rental and removes the item/effect
 * @param tid: Tick ID
 * @param tick: Timer
 * @param id: Timer ID
 * @param data: Data
 * @return false - failure, true - success
 */
TIMER_FUNC(pc_inventory_rental_end){
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

/**
 * Removes the rental timer from the player
 * @param sd: Player data
 */
void pc_inventory_rental_clear(struct map_session_data *sd)
{
	if( sd->rental_timer != INVALID_TIMER ) {
		delete_timer(sd->rental_timer, pc_inventory_rental_end);
		sd->rental_timer = INVALID_TIMER;
	}
}

/**
 * Check for items in the player's inventory that are rental type
 * @param sd: Player data
 */
void pc_inventory_rentals(struct map_session_data *sd)
{
	int i, c = 0;
	unsigned int next_tick = UINT_MAX;

	for( i = 0; i < MAX_INVENTORY; i++ ) { // Check for Rentals on Inventory
		if( sd->inventory.u.items_inventory[i].nameid == 0 )
			continue; // Nothing here
		if( sd->inventory.u.items_inventory[i].expire_time == 0 )
			continue;
		if( sd->inventory.u.items_inventory[i].expire_time <= time(NULL) ) {
			if (sd->inventory_data[i]->unequip_script)
				run_script(sd->inventory_data[i]->unequip_script, 0, sd->bl.id, fake_nd->bl.id);
			clif_rental_expired(sd, i, sd->inventory.u.items_inventory[i].nameid);
			pc_delitem(sd, i, sd->inventory.u.items_inventory[i].amount, 0, 0, LOG_TYPE_OTHER);
		} else {
			unsigned int expire_tick = (unsigned int)(sd->inventory.u.items_inventory[i].expire_time - time(NULL));

			clif_rental_time(sd, sd->inventory.u.items_inventory[i].nameid, (int)expire_tick);
			next_tick = umin(expire_tick * 1000U, next_tick);
			c++;
		}
	}

	if( c > 0 ) // min(next_tick,3600000) 1 hour each timer to keep announcing to the owner, and to avoid a but with rental time > 15 days
		sd->rental_timer = add_timer(gettick() + umin(next_tick,3600000), pc_inventory_rental_end, sd->bl.id, 0);
	else
		sd->rental_timer = INVALID_TIMER;
}

/**
 * Add a rental item to the player and adjusts the rental timer appropriately
 * @param sd: Player data
 * @param seconds: Rental time
 */
void pc_inventory_rental_add(struct map_session_data *sd, unsigned int seconds)
{
	t_tick tick = seconds * 1000;

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
		sd->rental_timer = add_timer(gettick() + i64min(tick,3600000), pc_inventory_rental_end, sd->bl.id, 0);
}

/**
 * Check if the player can sell the current item
 * @param sd: map_session_data of the player
 * @param item: struct of the checking item
 * @param shoptype: NPC's sub type see enum npc_subtype
 * @return bool 'true' is sellable, 'false' otherwise
 */
bool pc_can_sell_item(struct map_session_data *sd, struct item *item, enum npc_subtype shoptype) {
	if (sd == NULL || item == NULL)
		return false;

	if (!pc_can_give_items(sd))
		return false;

	if (item->equip > 0 || item->amount < 0)
		return false;

	if (battle_config.hide_fav_sell && item->favorite)
		return false; //Cannot sell favs (optional config)

	if (!battle_config.rental_transaction && item->expire_time)
		return false; // Cannot Sell Rental Items

	if( item->equipSwitch ){
		return false;
	}

	if (itemdb_ishatched_egg(item))
		return false;

	switch (shoptype) {
		case NPCTYPE_SHOP:
			if (item->bound && battle_config.allow_bound_sell&ISR_BOUND_SELLABLE && (
				item->bound != BOUND_GUILD ||
				(sd->guild && sd->status.char_id == sd->guild->member[0].char_id) ||
				(item->bound == BOUND_GUILD && !(battle_config.allow_bound_sell&ISR_BOUND_GUILDLEADER_ONLY))
				))
				return true;
			break;
		case NPCTYPE_ITEMSHOP:
			if (item->bound && battle_config.allow_bound_sell&ISR_BOUND && (
				item->bound != BOUND_GUILD ||
				(sd->guild && sd->status.char_id == sd->guild->member[0].char_id) ||
				(item->bound == BOUND_GUILD && !(battle_config.allow_bound_sell&ISR_BOUND_GUILDLEADER_ONLY))
				))
				return true;
			else if (!item->bound) {
				struct item_data *itd = itemdb_search(item->nameid);
				if (itd && itd->flag.trade_restriction.sell && battle_config.allow_bound_sell&ISR_SELLABLE)
					return true;
			}
			break;
	}

	if (!itemdb_cansell(item, pc_get_group_level(sd)))
		return false;

	if (item->bound && !pc_can_give_bounded_items(sd))
		return false; // Don't allow sale of bound items
	return true;
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

/**
 * Determine if an item in a player's inventory is tradeable based on several merits.
 * Checks for item_trade, bound, and rental restrictions.
 * @param sd: Player data
 * @param index: Item inventory index
 * @return True if the item can be traded or false otherwise
 */
bool pc_can_trade_item(map_session_data *sd, int index) {
	if (sd && index >= 0) {
		return (sd->inventory.u.items_inventory[index].expire_time == 0 &&
			(sd->inventory.u.items_inventory[index].bound == 0 || pc_can_give_bounded_items(sd)) &&
			itemdb_cantrade(&sd->inventory.u.items_inventory[index], pc_get_group_level(sd), pc_get_group_level(sd))
			);
	}

	return false;
}

/*==========================================
 * Prepares character for saving.
 * @param sd
 *------------------------------------------*/
void pc_makesavestatus(struct map_session_data *sd) {
	nullpo_retv(sd);

	if(!battle_config.save_clothcolor)
		sd->status.clothes_color = 0;

	if(!battle_config.save_body_style)
		sd->status.body = 0;

	//Only copy the Cart/Peco/Falcon options, the rest are handled via
	//status change load/saving. [Skotlex]
#ifdef NEW_CARTS
	sd->status.option = sd->sc.option&(OPTION_INVISIBLE|OPTION_FALCON|OPTION_RIDING|OPTION_DRAGON|OPTION_WUG|OPTION_WUGRIDER|OPTION_MADOGEAR);
#else
	sd->status.option = sd->sc.option&(OPTION_INVISIBLE|OPTION_CART|OPTION_FALCON|OPTION_RIDING|OPTION_DRAGON|OPTION_WUG|OPTION_WUGRIDER|OPTION_MADOGEAR);
#endif
	if (sd->sc.data[SC_JAILED]) { //When Jailed, do not move last point.
		if(pc_isdead(sd)){
			pc_setrestartvalue(sd, 0);
		} else {
			sd->status.hp = sd->battle_status.hp;
			sd->status.sp = sd->battle_status.sp;
			sd->status.ap = sd->battle_status.ap;
		}
		sd->status.last_point.map = sd->mapindex;
		sd->status.last_point.x = sd->bl.x;
		sd->status.last_point.y = sd->bl.y;
		return;
	}

	if(pc_isdead(sd)) {
		pc_setrestartvalue(sd, 0);
		memcpy(&sd->status.last_point,&sd->status.save_point,sizeof(sd->status.last_point));
	} else {
		sd->status.hp = sd->battle_status.hp;
		sd->status.sp = sd->battle_status.sp;
		sd->status.ap = sd->battle_status.ap;
		sd->status.last_point.map = sd->mapindex;
		sd->status.last_point.x = sd->bl.x;
		sd->status.last_point.y = sd->bl.y;
	}

	if(map_getmapflag(sd->bl.m, MF_NOSAVE)) {
		struct map_data *mapdata = map_getmapdata(sd->bl.m);

		if(mapdata->save.map)
			memcpy(&sd->status.last_point,&mapdata->save,sizeof(sd->status.last_point));
		else
			memcpy(&sd->status.last_point,&sd->status.save_point,sizeof(sd->status.last_point));
	}
}

/*==========================================
 * Off init ? Connection?
 *------------------------------------------*/
void pc_setnewpc(struct map_session_data *sd, uint32 account_id, uint32 char_id, int login_id1, t_tick client_tick, int sex, int fd) {
	nullpo_retv(sd);

	sd->bl.id = account_id;
	sd->status.account_id = account_id;
	sd->status.char_id = char_id;
	sd->status.sex = sex;
	sd->login_id1 = login_id1;
	sd->login_id2 = 0; // at this point, we can not know the value :(
	sd->client_tick = client_tick;
	sd->state.active = 0; //to be set to 1 after player is fully authed and loaded.
	sd->bl.type = BL_PC;
	if(battle_config.prevent_logout_trigger&PLT_LOGIN)
		sd->canlog_tick = gettick();
	//Required to prevent homunculus copuing a base speed of 0.
	sd->battle_status.speed = sd->base_status.speed = DEFAULT_WALK_SPEED;
}

/**
* Get equip point for an equip
* @param sd
* @param id
*/
int pc_equippoint_sub(struct map_session_data *sd,struct item_data* id){
	int ep = 0;

	nullpo_ret(sd);
	nullpo_ret(id);

	if (!itemdb_isequip2(id))
		return 0; //Not equippable by players.

	ep = id->equip;
	if(id->subtype == W_DAGGER	|| id->subtype == W_1HSWORD || id->subtype == W_1HAXE) {
		if(pc_checkskill(sd,AS_LEFT) > 0 || (sd->class_&MAPID_UPPERMASK) == MAPID_ASSASSIN || (sd->class_&MAPID_UPPERMASK) == MAPID_KAGEROUOBORO) { //Kagerou and Oboro can dual wield daggers. [Rytech]
			if (ep == EQP_WEAPON)
				return EQP_ARMS;
			if (ep == EQP_SHADOW_WEAPON)
				return EQP_SHADOW_ARMS;
		}
	}
	return ep;
}

/**
* Get equip point for an equip
* @param sd
* @param n Equip index in inventory
*/
int pc_equippoint(struct map_session_data *sd,int n){
	nullpo_ret(sd);

	return pc_equippoint_sub(sd,sd->inventory_data[n]);
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
		t_itemid id = sd->inventory.u.items_inventory[i].nameid;
		sd->inventory_data[i] = id?itemdb_search(id):NULL;
	}
}

/**
* 'Calculates' weapon type
* @param sd : Player
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
	sd->status.weapon = W_FIST;
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

	for (i = 0; i < EQI_MAX; i++){
		sd->equip_index[i] = -1;
		sd->equip_switch_index[i] = -1;
	}

	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->inventory.u.items_inventory[i].nameid == 0)
			continue;
		if (sd->inventory.u.items_inventory[i].equip) {
			uint8 j;
			for (j = 0; j < EQI_MAX; j++)
				if (sd->inventory.u.items_inventory[i].equip & equip_bitmask[j])
					sd->equip_index[j] = i;

			if (sd->inventory.u.items_inventory[i].equip & EQP_HAND_R) {
				if (sd->inventory_data[i])
					sd->weapontype1 = sd->inventory_data[i]->subtype;
				else
					sd->weapontype1 = W_FIST;
			}

			if( sd->inventory.u.items_inventory[i].equip & EQP_HAND_L ) {
				if( sd->inventory_data[i] && sd->inventory_data[i]->type == IT_WEAPON )
					sd->weapontype2 = sd->inventory_data[i]->subtype;
				else
					sd->weapontype2 = W_FIST;
			}
		}
		if (sd->inventory.u.items_inventory[i].equipSwitch) {
			for (uint8 j = 0; j < EQI_MAX; j++) {
				if (sd->inventory.u.items_inventory[i].equipSwitch & equip_bitmask[j]) {
					sd->equip_switch_index[j] = i;
				}
			}
		}
	}
	pc_calcweapontype(sd);
}

//static int pc_isAllowedCardOn(struct map_session_data *sd,int s,int eqindex,int flag)
//{
//	int i;
//	struct item *item = &sd->inventory.u.items_inventory[eqindex];
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
bool pc_isequipped(struct map_session_data *sd, t_itemid nameid)
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
		for( j = 0; j < MAX_SLOTS; j++ ){
			if( sd->inventory.u.items_inventory[index].card[j] == nameid )
				return true;
		}
	}

	return false;
}

/**
 * Check adoption rules
 * @param p1_sd: Player 1
 * @param p2_sd: Player 2
 * @param b_sd: Player that will be adopted
 * @return ADOPT_ALLOWED - Sent message to Baby to accept or deny
 *         ADOPT_ALREADY_ADOPTED - Already adopted
 *         ADOPT_MARRIED_AND_PARTY - Need to be married and in the same party
 *         ADOPT_EQUIP_RINGS - Need wedding rings equipped
 *         ADOPT_NOT_NOVICE - Adoptee is not a Novice
 *         ADOPT_CHARACTER_NOT_FOUND - Parent or Baby not found
 *         ADOPT_MORE_CHILDREN - Cannot adopt more than 1 Baby (client message)
 *         ADOPT_LEVEL_70 - Parents need to be level 70+ (client message)
 *         ADOPT_MARRIED - Cannot adopt a married person (client message)
 */
enum adopt_responses pc_try_adopt(struct map_session_data *p1_sd, struct map_session_data *p2_sd, struct map_session_data *b_sd)
{
	if( !p1_sd || !p2_sd || !b_sd )
		return ADOPT_CHARACTER_NOT_FOUND;

	if( b_sd->status.father || b_sd->status.mother || b_sd->adopt_invite )
		return ADOPT_ALREADY_ADOPTED; // already adopted baby / in adopt request

	if( !p1_sd->status.partner_id || !p1_sd->status.party_id || p1_sd->status.party_id != b_sd->status.party_id )
		return ADOPT_MARRIED_AND_PARTY; // You need to be married and in party with baby to adopt

	if( p1_sd->status.partner_id != p2_sd->status.char_id || p2_sd->status.partner_id != p1_sd->status.char_id )
		return ADOPT_MARRIED_AND_PARTY; // Not married, wrong married

	if( p2_sd->status.party_id != p1_sd->status.party_id )
		return ADOPT_MARRIED_AND_PARTY; // Both parents need to be in the same party

	// Parents need to have their ring equipped
	if( !pc_isequipped(p1_sd, WEDDING_RING_M) && !pc_isequipped(p1_sd, WEDDING_RING_F) )
		return ADOPT_EQUIP_RINGS;

	if( !pc_isequipped(p2_sd, WEDDING_RING_M) && !pc_isequipped(p2_sd, WEDDING_RING_F) )
		return ADOPT_EQUIP_RINGS;

	// Already adopted a baby
	if( p1_sd->status.child || p2_sd->status.child ) {
		clif_Adopt_reply(p1_sd, ADOPT_REPLY_MORE_CHILDREN);
		return ADOPT_MORE_CHILDREN;
	}

	// Parents need at least lvl 70 to adopt
	if( p1_sd->status.base_level < 70 || p2_sd->status.base_level < 70 ) {
		clif_Adopt_reply(p1_sd, ADOPT_REPLY_LEVEL_70);
		return ADOPT_LEVEL_70;
	}

	if( b_sd->status.partner_id ) {
		clif_Adopt_reply(p1_sd, ADOPT_REPLY_MARRIED);
		return ADOPT_MARRIED;
	}

	if( !( ( b_sd->status.class_ >= JOB_NOVICE && b_sd->status.class_ <= JOB_THIEF ) || b_sd->status.class_ == JOB_SUPER_NOVICE || b_sd->status.class_ == JOB_SUPER_NOVICE_E ) )
		return ADOPT_NOT_NOVICE;

	return ADOPT_ALLOWED;
}

/*==========================================
 * Adoption Process
 *------------------------------------------*/
bool pc_adoption(struct map_session_data *p1_sd, struct map_session_data *p2_sd, struct map_session_data *b_sd)
{
	int job, joblevel;
	t_exp jobexp;

	if( pc_try_adopt(p1_sd, p2_sd, b_sd) != ADOPT_ALLOWED )
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
		pc_skill(b_sd, WE_CHEERUP, 1, ADDSKILL_PERMANENT);

		// Parents Skills
		pc_skill(p1_sd, WE_CALLBABY, 1, ADDSKILL_PERMANENT);
		pc_skill(p2_sd, WE_CALLBABY, 1, ADDSKILL_PERMANENT);

		chrif_save(p1_sd, CSAVE_NORMAL);
		chrif_save(p2_sd, CSAVE_NORMAL);
		chrif_save(b_sd, CSAVE_NORMAL);

		achievement_update_objective(b_sd, AG_BABY, 1, 1);
		achievement_update_objective(p1_sd, AG_BABY, 1, 2);
		achievement_update_objective(p2_sd, AG_BABY, 1, 2);

		return true;
	}

	return false; // Job Change Fail
}

static bool pc_job_can_use_item( struct map_session_data* sd, struct item_data* item ){
	nullpo_retr( false, sd );
	nullpo_retr( false, item );

	// Calculate the required bit to check
	uint64 job = 1ULL << ( sd->class_ & MAPID_BASEMASK );

	size_t index;

	// 2-1
	if( ( sd->class_ & JOBL_2_1 ) != 0 ){
		index = 1;
	// 2-2
	}else if( ( sd->class_ & JOBL_2_2 ) != 0 ){
		index = 2;
	// Basejob
	}else{
		index = 0;
	}

	return ( item->class_base[index] & job ) != 0;
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
		if (item->class_upper&ITEMJ_NORMAL && !(sd->class_&(JOBL_UPPER|JOBL_BABY|JOBL_THIRD|JOBL_FOURTH)))	//normal classes (no upper, no baby, no third, no fourth)
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
		if (item->class_upper&(ITEMJ_THIRD|ITEMJ_THIRD_UPPER|ITEMJ_THIRD_BABY) && sd->class_&JOBL_THIRD)
			break;
#else
		//trans. classes (exl. third-trans.)
		if (item->class_upper&ITEMJ_UPPER && sd->class_&JOBL_UPPER && !(sd->class_&JOBL_THIRD))
			break;
		//baby classes (exl. third-baby)
		if (item->class_upper&ITEMJ_BABY && sd->class_&JOBL_BABY && !(sd->class_&JOBL_THIRD))
			break;
		//third classes (exl. third-trans. and baby-third and fourth)
		if (item->class_upper&ITEMJ_THIRD && sd->class_&JOBL_THIRD && !(sd->class_&(JOBL_UPPER|JOBL_BABY)) && !(sd->class_&JOBL_FOURTH))
			break;
		//trans-third classes (exl. fourth)
		if (item->class_upper&ITEMJ_THIRD_UPPER && sd->class_&JOBL_THIRD && sd->class_&JOBL_UPPER && !(sd->class_&JOBL_FOURTH))
			break;
		//third-baby classes (exl. fourth)
		if (item->class_upper&ITEMJ_THIRD_BABY && sd->class_&JOBL_THIRD && sd->class_&JOBL_BABY && !(sd->class_&JOBL_FOURTH))
			break;
		//fourth classes
		if (item->class_upper&ITEMJ_FOURTH && sd->class_&JOBL_FOURTH)
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
	if(item->sex != SEX_BOTH && sd->status.sex != item->sex)
		return ITEM_EQUIP_ACK_FAIL;

	//fail to equip if item is restricted
	if (!battle_config.allow_equip_restricted_item && itemdb_isNoEquip(item, sd->bl.m))
		return ITEM_EQUIP_ACK_FAIL;

	if (item->equip&EQP_AMMO) {
		switch (item->subtype) {
			case AMMO_ARROW:
				if (battle_config.ammo_check_weapon && sd->status.weapon != W_BOW && sd->status.weapon != W_MUSICAL && sd->status.weapon != W_WHIP) {
					clif_msg(sd, ITEM_NEED_BOW);
					return ITEM_EQUIP_ACK_FAIL;
				}
				break;
			case AMMO_DAGGER:
				if (!pc_checkskill(sd, AS_VENOMKNIFE))
					return ITEM_EQUIP_ACK_FAIL;
				break;
			case AMMO_BULLET:
			case AMMO_SHELL:
				if (battle_config.ammo_check_weapon && sd->status.weapon != W_REVOLVER && sd->status.weapon != W_RIFLE && sd->status.weapon != W_GATLING && sd->status.weapon != W_SHOTGUN
#ifdef RENEWAL
					&& sd->status.weapon != W_GRENADE
#endif
					) {
					clif_msg(sd, ITEM_BULLET_EQUIP_FAIL);
					return ITEM_EQUIP_ACK_FAIL;
				}
				break;
#ifndef RENEWAL
			case AMMO_GRENADE:
				if (battle_config.ammo_check_weapon && sd->status.weapon != W_GRENADE) {
					clif_msg(sd, ITEM_BULLET_EQUIP_FAIL);
					return ITEM_EQUIP_ACK_FAIL;
				}
				break;
#endif
			case AMMO_CANNONBALL:
				if (!pc_ismadogear(sd) && (sd->status.class_ == JOB_MECHANIC_T || sd->status.class_ == JOB_MECHANIC)) {
					clif_msg(sd, ITEM_NEED_MADOGEAR); // Item can only be used when Mado Gear is mounted.
					return ITEM_EQUIP_ACK_FAIL;
				}
				if (sd->state.active && !pc_iscarton(sd) && //Check if sc data is already loaded
					(sd->status.class_ == JOB_GENETIC_T || sd->status.class_ == JOB_GENETIC)) {
					clif_msg(sd, ITEM_NEED_CART); // Only available when cart is mounted.
					return ITEM_EQUIP_ACK_FAIL;
				}
				break;
		}
	}

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
		if (item->equip & EQP_ARMS && sd->sc.data[SC__WEAKNESS])
			return ITEM_EQUIP_ACK_FAIL;
		if (item->equip & EQP_SHADOW_GEAR && sd->sc.data[SC_SHADOW_STRIP])
			return ITEM_EQUIP_ACK_FAIL;
		if(item->equip && (sd->sc.data[SC_KYOUGAKU] || sd->sc.data[SC_SUHIDE]))
			return ITEM_EQUIP_ACK_FAIL;

		if (sd->sc.data[SC_SPIRIT] && sd->sc.data[SC_SPIRIT]->val2 == SL_SUPERNOVICE) {
			//Spirit of Super Novice equip bonuses. [Skotlex]
			if (sd->status.base_level > 90 && item->equip & EQP_HELM)
				return ITEM_EQUIP_ACK_OK; //Can equip all helms

			if (sd->status.base_level > 96 && item->equip & EQP_ARMS && item->type == IT_WEAPON && item->weapon_level == 4)
				switch(item->subtype) { //In weapons, the look determines type of weapon.
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

	//Not equipable by class. [Skotlex]
	if (!pc_job_can_use_item(sd,item))
		return ITEM_EQUIP_ACK_FAIL;

	if (!pc_isItemClass(sd, item))
		return ITEM_EQUIP_ACK_FAIL;

	return ITEM_EQUIP_ACK_OK;
}

/*==========================================
 * No problem with the session id
 * set the status that has been sent from char server
 *------------------------------------------*/
bool pc_authok(struct map_session_data *sd, uint32 login_id2, time_t expiration_time, int group_id, struct mmo_charstatus *st, bool changing_mapservers)
{
	t_tick tick = gettick();
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
	uint64 class_ = pc_jobid2mapid(sd->status.class_);
	if (class_ == -1 || !job_db.exists(sd->status.class_)) { //Invalid class?
		ShowError("pc_authok: Invalid class %d for player %s (%d:%d). Class was changed to novice.\n", sd->status.class_, sd->status.name, sd->status.account_id, sd->status.char_id);
		sd->status.class_ = JOB_NOVICE;
		sd->class_ = MAPID_NOVICE;
	} else
		sd->class_ = class_;

	// Checks and fixes to character status data, that are required
	// in case of configuration change or stuff, which cannot be
	// checked on char-server.
	sd->status.hair = cap_value(sd->status.hair,MIN_HAIR_STYLE,MAX_HAIR_STYLE);
	sd->status.hair_color = cap_value(sd->status.hair_color,MIN_HAIR_COLOR,MAX_HAIR_COLOR);
	sd->status.clothes_color = cap_value(sd->status.clothes_color,MIN_CLOTH_COLOR,MAX_CLOTH_COLOR);
	sd->status.body = cap_value(sd->status.body,MIN_BODY_STYLE,MAX_BODY_STYLE);

	//Initializations to null/0 unneeded since map_session_data was filled with 0 upon allocation.
	sd->state.connect_new = 1;

	sd->followtimer = INVALID_TIMER; // [MouseJstr]
	sd->invincible_timer = INVALID_TIMER;
	sd->npc_timer_id = INVALID_TIMER;
	sd->pvp_timer = INVALID_TIMER;
	sd->expiration_tid = INVALID_TIMER;
	sd->autotrade_tid = INVALID_TIMER;
	sd->respawn_tid = INVALID_TIMER;
	sd->tid_queue_active = INVALID_TIMER;

	sd->skill_keep_using.tid = INVALID_TIMER;
	sd->skill_keep_using.skill_id = 0;
	sd->skill_keep_using.level = 0;
	sd->skill_keep_using.target = 0;

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

	sd->regen.tick.hp = tick;
	sd->regen.tick.sp = tick;

	for(int i = 0; i < MAX_SPIRITBALL; i++)
		sd->spirit_timer[i] = INVALID_TIMER;

	if (battle_config.item_auto_get)
		sd->state.autoloot = 10000;

	if (battle_config.disp_experience)
		sd->state.showexp = 1;
	if (battle_config.disp_zeny)
		sd->state.showzeny = 1;
#ifdef VIP_ENABLE
	if (!battle_config.vip_disp_rate)
		sd->vip.disableshowrate = 1;
#endif

	if (!(battle_config.display_skill_fail&2))
		sd->state.showdelay = 1;

	memset(&sd->inventory, 0, sizeof(struct s_storage));
	memset(&sd->cart, 0, sizeof(struct s_storage));
	memset(&sd->storage, 0, sizeof(struct s_storage));
	memset(&sd->premiumStorage, 0, sizeof(struct s_storage));
	memset(&sd->equip_index, -1, sizeof(sd->equip_index));
	memset(&sd->equip_switch_index, -1, sizeof(sd->equip_switch_index));

	if( pc_isinvisible(sd) && !pc_can_use_command( sd, "hide", COMMAND_ATCOMMAND ) ){
		sd->status.option &= ~OPTION_INVISIBLE;
	}

	status_change_init(&sd->bl);

	sd->sc.option = sd->status.option; //This is the actual option used in battle.

	unit_dataset(&sd->bl);

	sd->guild_x = -1;
	sd->guild_y = -1;

	sd->delayed_damage = 0;

	// Event Timers
	for( int i = 0; i < MAX_EVENTTIMER; i++ )
		sd->eventtimer[i] = INVALID_TIMER;
	// Rental Timer
	sd->rental_timer = INVALID_TIMER;

	for( int i = 0; i < 3; i++ )
		sd->hate_mob[i] = -1;

	sd->quest_log = NULL;
	sd->num_quests = 0;
	sd->avail_quests = 0;
	sd->save_quest = false;
	sd->count_rewarp = 0;
	sd->mail.pending_weight = 0;
	sd->mail.pending_zeny = 0;
	sd->mail.pending_slots = 0;

	sd->regs.vars = i64db_alloc(DB_OPT_BASE);
	sd->regs.arrays = NULL;
	sd->vars_dirty = false;
	sd->vars_ok = false;
	sd->vars_received = 0x0;

	//warp player
	enum e_setpos setpos_result = pc_setpos( sd, sd->status.last_point.map, sd->status.last_point.x, sd->status.last_point.y, CLR_OUTSIGHT );
	if( setpos_result != SETPOS_OK ){
		ShowError( "Last_point_map %s - id %d not found (error code %d)\n", mapindex_id2name(sd->status.last_point.map), sd->status.last_point.map, setpos_result );

		// try warping to a default map instead (church graveyard)
		if (pc_setpos(sd, mapindex_name2id(MAP_PRONTERA), 273, 354, CLR_OUTSIGHT) != SETPOS_OK) {
			// if we fail again
			clif_authfail_fd(sd->fd, 0);
			return false;
		}
	}

	clif_inventory_expansion_info( sd );
	clif_authok(sd);

	//Prevent S. Novices from getting the no-death bonus just yet. [Skotlex]
	sd->die_counter=-1;

	//display login notice
	ShowInfo("'" CL_WHITE "%s" CL_RESET "' logged in."
	         " (AID/CID: '" CL_WHITE "%d/%d" CL_RESET "',"
	         " IP: '" CL_WHITE "%d.%d.%d.%d" CL_RESET "',"
	         " Group '" CL_WHITE "%d" CL_RESET "').\n",
	         sd->status.name, sd->status.account_id, sd->status.char_id,
	         CONVIP(ip), sd->group_id);
	// Send friends list
	clif_friendslist_send(sd);

	if( !changing_mapservers ) {

		if (battle_config.display_version == 1)
			pc_show_version(sd);

		// Message of the Day [Valaris]
		for(int i=0; i < MOTD_LINE_SIZE && motd_text[i][0]; i++) {
			if (battle_config.motd_type)
				clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], motd_text[i], false, SELF);
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

	pc_validate_skill(sd);

	/* [Ind] */
	sd->sc_display = NULL;
	sd->sc_display_count = 0;

	// Player has not yet received the CashShop list
	sd->status.cashshop_sent = false;

	sd->last_addeditem_index = -1;
	
	sd->bonus_script.head = NULL;
	sd->bonus_script.count = 0;

	// Initialize BG queue
	sd->bg_queue_id = 0;

#if PACKETVER_MAIN_NUM >= 20150507 || PACKETVER_RE_NUM >= 20150429 || defined(PACKETVER_ZERO)
	sd->hatEffects = {};
#endif

	sd->catch_target_class = PET_CATCH_FAIL;

	// Check EXP overflow, since in previous revision EXP on Max Level can be more than 'official' Max EXP
	if (pc_is_maxbaselv(sd) && sd->status.base_exp > MAX_LEVEL_BASE_EXP) {
		sd->status.base_exp = MAX_LEVEL_BASE_EXP;
		clif_updatestatus(sd, SP_BASEEXP);
	}
	if (pc_is_maxjoblv(sd) && sd->status.job_exp > MAX_LEVEL_JOB_EXP) {
		sd->status.job_exp = MAX_LEVEL_JOB_EXP;
		clif_updatestatus(sd, SP_JOBEXP);
	}

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
 * We didn't receive item information at this point so DO NOT attempt to do item operations here.
 * See intif_parse_StorageReceived() for item operations [lighta]
 *------------------------------------------*/
void pc_reg_received(struct map_session_data *sd)
{
	uint8 i;

	sd->vars_ok = true;

	sd->change_level_2nd = static_cast<unsigned char>(pc_readglobalreg(sd, add_str(JOBCHANGE2ND_VAR)));
	sd->change_level_3rd = static_cast<unsigned char>(pc_readglobalreg(sd, add_str(JOBCHANGE3RD_VAR)));
	sd->change_level_4th = static_cast<unsigned char>(pc_readglobalreg(sd, add_str(JOBCHANGE4TH_VAR)));
	sd->die_counter = static_cast<int>(pc_readglobalreg(sd, add_str(PCDIECOUNTER_VAR)));

	sd->langtype = static_cast<int>(pc_readaccountreg(sd, add_str(LANGTYPE_VAR)));
	if (msg_checklangtype(sd->langtype,true) < 0)
		sd->langtype = 0; //invalid langtype reset to default

	// Cash shop
	sd->cashPoints = static_cast<int>(pc_readaccountreg(sd, add_str(CASHPOINT_VAR)));
	sd->kafraPoints = static_cast<int>(pc_readaccountreg(sd, add_str(KAFRAPOINT_VAR)));

	// Cooking Exp
	sd->cook_mastery = static_cast<short>(pc_readglobalreg(sd, add_str(COOKMASTERY_VAR)));

	if( (sd->class_&MAPID_BASEMASK) == MAPID_TAEKWON )
	{ // Better check for class rather than skill to prevent "skill resets" from unsetting this
		sd->mission_mobid = static_cast<short>(pc_readglobalreg(sd, add_str(TKMISSIONID_VAR)));
		sd->mission_count = static_cast<unsigned char>(pc_readglobalreg(sd, add_str(TKMISSIONCOUNT_VAR)));
	}

	if (battle_config.feature_banking)
		sd->bank_vault = static_cast<int>(pc_readreg2(sd, BANK_VAULT_VAR));

	if (battle_config.feature_roulette) {
		sd->roulette_point.bronze = static_cast<int>(pc_readreg2(sd, ROULETTE_BRONZE_VAR));
		sd->roulette_point.silver = static_cast<int>(pc_readreg2(sd, ROULETTE_SILVER_VAR));
		sd->roulette_point.gold = static_cast<int>(pc_readreg2(sd, ROULETTE_GOLD_VAR));
	}
	sd->roulette.prizeIdx = -1;

	//SG map and mob read [Komurka]
	for(i=0;i<MAX_PC_FEELHATE;i++) { //for now - someone need to make reading from txt/sql
		uint16 j;

		if ((j = static_cast<uint16>(pc_readglobalreg(sd, add_str(sg_info[i].feel_var)))) != 0) {
			sd->feel_map[i].index = j;
			sd->feel_map[i].m = map_mapindex2mapid(j);
		} else {
			sd->feel_map[i].index = 0;
			sd->feel_map[i].m = -1;
		}
		sd->hate_mob[i] = static_cast<short>(pc_readglobalreg(sd, add_str(sg_info[i].hate_var)))-1;
	}

	if ((i = pc_checkskill(sd,RG_PLAGIARISM)) > 0) {
		unsigned short skid = static_cast<unsigned short>(pc_readglobalreg(sd, add_str(SKILL_VAR_PLAGIARISM)));
		sd->cloneskill_idx = skill_get_index(skid);
		if (sd->cloneskill_idx > 0) {
			sd->status.skill[sd->cloneskill_idx].id = skid;
			sd->status.skill[sd->cloneskill_idx].lv = static_cast<uint8>(pc_readglobalreg(sd, add_str(SKILL_VAR_PLAGIARISM_LV)));
			if (sd->status.skill[sd->cloneskill_idx].lv > i)
				sd->status.skill[sd->cloneskill_idx].lv = i;
			sd->status.skill[sd->cloneskill_idx].flag = SKILL_FLAG_PLAGIARIZED;
		}
	}
	if ((i = pc_checkskill(sd,SC_REPRODUCE)) > 0) {
		unsigned short skid = static_cast<unsigned short>(pc_readglobalreg(sd, add_str(SKILL_VAR_REPRODUCE)));
		sd->reproduceskill_idx = skill_get_index(skid);
		if (sd->reproduceskill_idx > 0) {
			sd->status.skill[sd->reproduceskill_idx].id = skid;
			sd->status.skill[sd->reproduceskill_idx].lv = static_cast<uint8>(pc_readglobalreg(sd, add_str(SKILL_VAR_REPRODUCE_LV)));
			if (i < sd->status.skill[sd->reproduceskill_idx].lv)
				sd->status.skill[sd->reproduceskill_idx].lv = i;
			sd->status.skill[sd->reproduceskill_idx].flag = SKILL_FLAG_PLAGIARIZED;
		}
	}
	//Weird... maybe registries were reloaded?
	if (sd->state.active)
		return;
	sd->state.active = 1;
	sd->state.pc_loaded = false; // Ensure inventory data and status data is loaded before we calculate player stats

	intif_storage_request(sd,TABLE_STORAGE, 0, STOR_MODE_ALL); // Request storage data
	intif_storage_request(sd,TABLE_CART, 0, STOR_MODE_ALL); // Request cart data
	intif_storage_request(sd,TABLE_INVENTORY, 0, STOR_MODE_ALL); // Request inventory data

	// Restore IM_CHAR instance to the player
	for (const auto &instance : instances) {
		if (instance.second->mode == IM_CHAR && instance.second->owner_id == sd->status.char_id) {
			sd->instance_id = instance.first;
			break;
		}
	}

#if PACKETVER_MAIN_NUM < 20190403 || PACKETVER_RE_NUM < 20190320 || PACKETVER_ZERO_NUM < 20190410
	if (sd->instance_id > 0)
		instance_reqinfo(sd, sd->instance_id);
	if (sd->status.party_id > 0)
		party_member_joined(sd);
	if (sd->status.guild_id > 0)
		guild_member_joined(sd);
	if (sd->status.clan_id > 0)
		clan_member_joined(sd);
#endif

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

	chrif_skillcooldown_request(sd->status.account_id, sd->status.char_id);
	chrif_bsdata_request(sd->status.char_id);
#ifdef VIP_ENABLE
	sd->vip.time = 0;
	sd->vip.enabled = 0;
	chrif_req_login_operation(sd->status.account_id, sd->status.name, CHRIF_OP_LOGIN_VIP, 0, 1|8, 0);  // request VIP information
#endif
	intif_Mail_requestinbox(sd->status.char_id, 0, MAIL_INBOX_NORMAL); // MAIL SYSTEM - Request Mail Inbox
	intif_request_questlog(sd);

	if (battle_config.feature_achievement) {
		sd->achievement_data.total_score = 0;
		sd->achievement_data.level = 0;
		sd->achievement_data.save = false;
		sd->achievement_data.count = 0;
		sd->achievement_data.incompleteCount = 0;
		sd->achievement_data.achievements = NULL;
		intif_request_achievements(sd->status.char_id);
	}

	if (sd->state.connect_new == 0 && sd->fd) { //Character already loaded map! Gotta trigger LoadEndAck manually.
		sd->state.connect_new = 1;
		clif_parse_LoadEndAck(sd->fd, sd);

		if( pc_isinvisible( sd ) ){
			sd->vd.class_ = JT_INVISIBLE;
			clif_displaymessage( sd->fd, msg_txt( sd, 11 ) ); // Invisible: On
			// decrement the number of pvp players on the map
			map_getmapdata( sd->bl.m )->users_pvp--;

			if( map_getmapflag( sd->bl.m, MF_PVP ) && !map_getmapflag( sd->bl.m, MF_PVP_NOCALCRANK ) && sd->pvp_timer != INVALID_TIMER ){
				// unregister the player for ranking
				delete_timer( sd->pvp_timer, pc_calc_pvprank_timer );
				sd->pvp_timer = INVALID_TIMER;
			}

			clif_changeoption( &sd->bl );
		}
	}

	channel_autojoin(sd);
}

static int pc_calc_skillpoint(struct map_session_data* sd)
{
	uint16 i, skill_point = 0;

	nullpo_ret(sd);

	for(i = 1; i < MAX_SKILL; i++) {
		if( sd->status.skill[i].id && sd->status.skill[i].lv > 0) {
			std::shared_ptr<s_skill_db> skill = skill_db.find(sd->status.skill[i].id);

			if ((!skill->inf2[INF2_ISQUEST] || battle_config.quest_skill_learn) &&
				(!skill->inf2[INF2_ISWEDDING] || skill->inf2[INF2_ISSPIRIT]) //Do not count wedding/link skills. [Skotlex]
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
	if (!sd || !pc_has_permission(sd, PC_PERM_ALL_SKILL))
		return false;

	/**
	* Dummy skills must NOT be added here otherwise they'll be displayed in the,
	* skill tree and since they have no icons they'll give resource errors
	* Get ALL skills except npc/guild ones. [Skotlex]
	* Don't add SG_DEVIL [Komurka] and MO_TRIPLEATTACK and RG_SNATCHER [ultramage]
	**/
	for (const auto &skill : skill_db) {
		uint16 skill_id = skill.second->nameid;

		if (skill_id == 0 || skill.second->inf2[INF2_ISNPC]|| skill.second->inf2[INF2_ISGUILD])
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
						uint16 idx = skill_get_index(skill_id);

						sd->status.skill[idx].id = skill_id;
						if (addlv)
							sd->status.skill[idx].lv = lv;
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
	nullpo_retv(sd);

	uint64 job = pc_calc_skilltree_normalize_job(sd);
	int class_ = pc_mapid2jobid(job, sd->status.sex);

	if( class_ == -1 )
	{ //Unable to normalize job??
		ShowError( "pc_calc_skilltree: Unable to normalize job %s(%d) for character %s (%d:%d)\n", job_name( sd->status.class_ ), sd->status.class_, sd->status.name, sd->status.account_id, sd->status.char_id );
		return;
	}
	uint16 job_id = class_;
	class_ = pc_class2idx(class_);

	for (const auto &skill : skill_db) {
		uint16 skill_id = skill.second->nameid;
		uint16 idx = skill_get_index(skill_id);

		if( sd->status.skill[idx].flag != SKILL_FLAG_PLAGIARIZED && sd->status.skill[idx].flag != SKILL_FLAG_PERM_GRANTED ) //Don't touch these
			sd->status.skill[idx].id = 0; //First clear skills.
		/* permanent skills that must be re-checked */
		if( sd->status.skill[idx].flag == SKILL_FLAG_PERM_GRANTED ) {
			if (skill_id == 0) {
				sd->status.skill[idx].id = 0;
				sd->status.skill[idx].lv = 0;
				sd->status.skill[idx].flag = SKILL_FLAG_PERMANENT;
				continue;
			}
			switch (skill_id) {
				case NV_TRICKDEAD:
					if( (sd->class_&MAPID_UPPERMASK) != MAPID_NOVICE ) {
						sd->status.skill[idx].id = 0;
						sd->status.skill[idx].lv = 0;
						sd->status.skill[idx].flag = SKILL_FLAG_PERMANENT;
					}
					break;
			}
		}
	}

	for (const auto &skill : skill_db) {
		uint16 idx = skill_get_index(skill.second->nameid);

		// Restore original level of skills after deleting earned skills.
		if( sd->status.skill[idx].flag != SKILL_FLAG_PERMANENT && sd->status.skill[idx].flag != SKILL_FLAG_PERM_GRANTED && sd->status.skill[idx].flag != SKILL_FLAG_PLAGIARIZED ) {
			sd->status.skill[idx].lv = (sd->status.skill[idx].flag == SKILL_FLAG_TEMPORARY) ? 0 : sd->status.skill[idx].flag - SKILL_FLAG_REPLACED_LV_0;
			sd->status.skill[idx].flag = SKILL_FLAG_PERMANENT;
		}
	}

	// Removes Taekwon Ranker skill bonus
	if ((sd->class_&MAPID_UPPERMASK) != MAPID_TAEKWON) {
		std::shared_ptr<s_skill_tree> tree = skill_tree_db.find(JOB_TAEKWON);
	
		if (tree != nullptr && !tree->skills.empty()) {
			for (const auto &it : tree->skills) {
				uint16 sk_idx = skill_get_index(it.first);
				if (sk_idx == 0)
					continue;
				if (sd->status.skill[sk_idx].flag != SKILL_FLAG_PLAGIARIZED && sd->status.skill[sk_idx].flag != SKILL_FLAG_PERM_GRANTED) {
					if (it.first == NV_BASIC || it.first == NV_FIRSTAID || it.first == WE_CALLBABY)
						continue;
					sd->status.skill[sk_idx].id = 0;
				}
			}
		}
	}

	// Grant all skills
	pc_grant_allskills(sd, false);

	int flag;
	std::shared_ptr<s_skill_tree> tree = skill_tree_db.find(job_id);

	do {
		flag = 0;
		if (tree == nullptr || tree->skills.empty())
			break;

		for (const auto &skillsit : tree->skills) {
			bool fail = false;
			uint16 skid = skillsit.first;
			uint16 sk_idx = skill_get_index(skid);

			if (sd->status.skill[sk_idx].id)
				continue; //Skill already known.

			if (!battle_config.skillfree) {
				// Checking required skills
				std::shared_ptr<s_skill_tree_entry> entry = skillsit.second;

				if (entry != nullptr && !entry->need.empty()) {
					for (const auto &it : entry->need) {
						uint16 sk_need_id = it.first;
						uint16 sk_need_idx = skill_get_index(sk_need_id);

						if (sk_need_idx > 0) {
							uint16 sk_need = sk_need_id;

							if (sd->status.skill[sk_need_idx].id == 0 || sd->status.skill[sk_need_idx].flag == SKILL_FLAG_TEMPORARY || sd->status.skill[sk_need_idx].flag == SKILL_FLAG_PLAGIARIZED)
								sk_need = 0; //Not learned.
							else if (sd->status.skill[sk_need_idx].flag >= SKILL_FLAG_REPLACED_LV_0) //Real learned level
								sk_need = sd->status.skill[sk_need_idx].flag - SKILL_FLAG_REPLACED_LV_0;
							else
								sk_need = pc_checkskill(sd,sk_need_id);

							if (sk_need < it.second) {
								fail = true;
								break;
							}
						}
					}
				}

				if (sd->status.base_level < entry->baselv) { //We need to get the actual class in this case
					int c_ = pc_mapid2jobid(sd->class_, sd->status.sex);

					c_ = pc_class2idx(c_);
					if (class_ == c_ || (class_ != c_ && sd->status.base_level < entry->baselv))
						fail = true; // base level requirement wasn't satisfied
				}
				if (sd->status.job_level < entry->joblv) { //We need to get the actual class in this case
					int c_ = pc_mapid2jobid(sd->class_, sd->status.sex);

					c_ = pc_class2idx(c_);
					if (class_ == c_ || (class_ != c_ && sd->status.job_level < entry->joblv))
						fail = true; // job level requirement wasn't satisfied
				}
			}

			if (!fail) {
				std::shared_ptr<s_skill_db> skill = skill_db.find(skid);

				if (!sd->status.skill[sk_idx].lv && (
					(skill->inf2[INF2_ISQUEST] && !battle_config.quest_skill_learn) ||
					skill->inf2[INF2_ISWEDDING] ||
					(skill->inf2[INF2_ISSPIRIT] && !sd->sc.data[SC_SPIRIT])
				))
					continue; //Cannot be learned via normal means. Note this check DOES allows raising already known skills.

				sd->status.skill[sk_idx].id = skid;

				if(skill->inf2[INF2_ISSPIRIT]) { //Spirit skills cannot be learned, they will only show up on your tree when you get buffed.
					sd->status.skill[sk_idx].lv = 1; // need to manually specify a skill level
					sd->status.skill[sk_idx].flag = SKILL_FLAG_TEMPORARY; //So it is not saved, and tagged as a "bonus" skill.
				}
				flag = 1; // skill list has changed, perform another pass
			}
		}
	} while(flag);

	if( class_ > 0 && sd->status.skill_point == 0 && pc_is_taekwon_ranker(sd) ) {
		uint16 skid = 0;

		/* Taekwon Ranker Bonus Skill Tree
		============================================
		- Grant All Taekwon Tree, but only as Bonus Skills in case they drop from ranking.
		- (c > 0) to avoid grant Novice Skill Tree in case of Skill Reset (need more logic)
		- (sd->status.skill_point == 0) to wait until all skill points are assigned to avoid problems with Job Change quest. */

		std::shared_ptr<s_skill_tree> tree = skill_tree_db.find(job_id);

		if (tree != nullptr && !tree->skills.empty()) {
			for (const auto &it : tree->skills) {
				skid = it.first;
				uint16 sk_idx = skill_get_index(skid);

				if (sk_idx == 0)
					continue;

				if( skill_get_inf2_(skid, { INF2_ISQUEST, INF2_ISWEDDING }) )
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

	// Enable Bard/Dancer spirit linked skills.
	if (sd->sc.count && sd->sc.data[SC_SPIRIT] && sd->sc.data[SC_SPIRIT]->val2 == SL_BARDDANCER) {
		std::vector<std::vector<uint16>> linked_skills = { { BA_WHISTLE, DC_HUMMING },
														   { BA_ASSASSINCROSS, DC_DONTFORGETME },
														   { BA_POEMBRAGI, DC_FORTUNEKISS },
														   { BA_APPLEIDUN, DC_SERVICEFORYOU } };

		for (const auto &skill : linked_skills) {
			if (pc_checkskill(sd, skill[!sd->status.sex]) < 10)
				continue;

			// Tag it as a non-savable, non-uppable, bonus skill
			pc_skill(sd, skill[sd->status.sex], 10, ADDSKILL_TEMP);
		}
	}
}

//Checks if you can learn a new skill after having leveled up a skill.
static void pc_check_skilltree(struct map_session_data *sd)
{
	int flag = 0;

	if (battle_config.skillfree)
		return; //Function serves no purpose if this is set

	int c = pc_mapid2jobid( pc_calc_skilltree_normalize_job( sd ), sd->status.sex );
	if (c == -1) { //Unable to normalize job??
		ShowError("pc_check_skilltree: Unable to normalize job %d for character %s (%d:%d)\n", sd->status.class_, sd->status.name, sd->status.account_id, sd->status.char_id);
		return;
	}
	std::shared_ptr<s_skill_tree> tree = skill_tree_db.find(c);
	if (tree == nullptr || tree->skills.empty())
		return;

	do {
		flag = 0;

		for (const auto &skillsit : tree->skills) {
			uint16 skid = skillsit.first;
			uint16 sk_idx = skill_get_index(skid);
			bool fail = false;

			if (sd->status.skill[sk_idx].id) //Already learned
				continue;

			// Checking required skills
			std::shared_ptr<s_skill_tree_entry> entry = skillsit.second;

			if (entry != nullptr && !entry->need.empty()) {
				for (const auto &it : entry->need) {
					uint16 sk_need_id = it.first;
					uint16 sk_need_idx = skill_get_index(sk_need_id);

					if (sk_need_id > 0) {
						short sk_need = sk_need_id;

						if (sd->status.skill[sk_need_idx].id == 0 || sd->status.skill[sk_need_idx].flag == SKILL_FLAG_TEMPORARY || sd->status.skill[sk_need_idx].flag == SKILL_FLAG_PLAGIARIZED)
							sk_need = 0; //Not learned.
						else if (sd->status.skill[sk_need_idx].flag >= SKILL_FLAG_REPLACED_LV_0) //Real lerned level
							sk_need = sd->status.skill[sk_need_idx].flag - SKILL_FLAG_REPLACED_LV_0;
						else
							sk_need = pc_checkskill(sd,sk_need_id);

						if (sk_need < it.second) {
							fail = true;
							break;
						}
					}
				}
			}

			if( fail )
				continue;
			if (sd->status.base_level < entry->baselv || sd->status.job_level < entry->joblv)
				continue;

			std::shared_ptr<s_skill_db> skill = skill_db.find(skid);

			if( !sd->status.skill[sk_idx].lv && (
				(skill->inf2[INF2_ISQUEST] && !battle_config.quest_skill_learn) ||
				skill->inf2[INF2_ISWEDDING] ||
				(skill->inf2[INF2_ISSPIRIT] && !sd->sc.data[SC_SPIRIT])
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

uint64 pc_calc_skilltree_normalize_job_sub( struct map_session_data *sd ){
	int skill_point = pc_calc_skillpoint( sd );

	if( sd->class_ & MAPID_SUMMONER ){
		// Novice's skill points for basic skill.
		std::shared_ptr<s_job_info> summoner_job = job_db.find( JOB_SUMMONER );

		int summoner_skills = summoner_job->max_job_level - 1;

		if( skill_point < summoner_skills ){
			return MAPID_SUMMONER;
		}

		skill_point -= summoner_skills;
	}else{
		// Novice's skill points for basic skill.
		std::shared_ptr<s_job_info> novice_job = job_db.find( JOB_NOVICE );

		int novice_skills = novice_job->max_job_level - 1;

		if( skill_point < novice_skills ){
			return MAPID_NOVICE;
		}

		skill_point -= novice_skills;
	}

	// 1st Class Job LV Check
	if( sd->class_ & JOBL_2 && ( sd->class_ & MAPID_UPPERMASK ) != MAPID_SUPER_NOVICE ){
		uint64 mapid_1st = sd->class_ & MAPID_BASEMASK;
		int class_1st = pc_mapid2jobid( mapid_1st, sd->status.sex );

		if( !sd->change_level_2nd ){
			if( class_1st >= JOB_NOVICE ){
				sd->change_level_2nd = job_db.find( class_1st )->max_job_level;
			}else{
				sd->change_level_2nd = 0;
			}

			pc_setglobalreg( sd, add_str( JOBCHANGE2ND_VAR ), sd->change_level_2nd );
		}

		if( class_1st > 0 && skill_point < ( sd->change_level_2nd - 1 ) ){
			return mapid_1st;
		}

		skill_point -= ( sd->change_level_2nd - 1 );
	}

	// 2nd Class Job LV Check
	if( sd->class_ & JOBL_THIRD && (sd->class_ & MAPID_THIRDMASK) != MAPID_SUPER_NOVICE_E ){
		uint64 mapid_2nd = sd->class_ & MAPID_UPPERMASK;
		int class_2nd = pc_mapid2jobid( mapid_2nd, sd->status.sex );

		if( !sd->change_level_3rd ){
			if( class_2nd >= JOB_NOVICE ){
				sd->change_level_3rd = job_db.find( class_2nd )->max_job_level;
			}else{
				sd->change_level_3rd = 0;
			}

			pc_setglobalreg( sd, add_str( JOBCHANGE3RD_VAR ), sd->change_level_3rd );
		}

		if( class_2nd > 0 && skill_point < ( sd->change_level_3rd - 1 ) ){
			return mapid_2nd;
		}

		skill_point -= ( sd->change_level_3rd - 1 );
	}

	// 3rd Class Job LV Check
	if( sd->class_ & JOBL_FOURTH ){
		uint64 mapid_3rd = sd->class_ & MAPID_THIRDMASK | JOBL_THIRD;
		int class_3rd = pc_mapid2jobid( mapid_3rd, sd->status.sex );

		if( !sd->change_level_4th ){
			if( class_3rd >= JOB_NOVICE ){
				sd->change_level_4th = job_db.find( class_3rd )->max_job_level;
			}else{
				sd->change_level_4th = 0;
			}

			pc_setglobalreg( sd, add_str( JOBCHANGE4TH_VAR ), sd->change_level_4th );
		}

		if( class_3rd > 0 && skill_point < ( sd->change_level_4th - 1 ) ){
			return mapid_3rd;
		}

		skill_point -= ( sd->change_level_4th - 1 );
	}

	return sd->class_;
}

uint64 pc_calc_skilltree_normalize_job( struct map_session_data *sd ){
	if( !battle_config.skillup_limit || pc_has_permission( sd, PC_PERM_ALL_SKILL ) ){
		return sd->class_;
	}

	uint64 c = pc_calc_skilltree_normalize_job_sub( sd );

	// Special Masks
	if( sd->class_&JOBL_UPPER ){
		if( pc_mapid2jobid( c | JOBL_UPPER, sd->status.sex ) >= JOB_NOVICE ){
			c |= JOBL_UPPER;// Rebirth Job
		}
	}

	if( sd->class_&JOBL_BABY ){
		if( pc_mapid2jobid( c | JOBL_BABY, sd->status.sex ) >= JOB_NOVICE ){
			c |= JOBL_BABY;// Baby Job
		}
	}

	return c;
}

/*==========================================
 * Updates the weight status
 *------------------------------------------
 * 1: overweight 50% for pre-renewal and 70% for renewal
 * 2: overweight 90%
 * It's assumed that SC_WEIGHT50 and SC_WEIGHT90 are only started/stopped here.
 */
void pc_updateweightstatus(struct map_session_data *sd)
{
	int old_overweight;
	int new_overweight;

	nullpo_retv(sd);

	old_overweight = (sd->sc.data[SC_WEIGHT90]) ? 2 : (sd->sc.data[SC_WEIGHT50]) ? 1 : 0;
#ifdef RENEWAL
	new_overweight = (pc_is90overweight(sd)) ? 2 : (pc_is70overweight(sd)) ? 1 : 0;
#else
	new_overweight = (pc_is90overweight(sd)) ? 2 : (pc_is50overweight(sd)) ? 1 : 0;
#endif

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

	if(pc_isinvisible(sd))
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
			if ((cd = (struct chat_data*)map_id2bl(sd->chatID)) != NULL)
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

/**
 * Add auto spell bonus for player while attacking/attacked
 * @param spell: Spell array
 * @param id: Skill to cast
 * @param lv: Skill level
 * @param rate: Success chance
 * @param battle_flag: Battle flag
 * @param card_id: Used to prevent card stacking
 * @param flag: Flags used for extra arguments
 *              &0: forces the skill to be casted on self, rather than on the target
 *              &1: forces the skill to be casted on target, rather than self
 *              &2: random skill level in [1..lv] is chosen
 */
static void pc_bonus_autospell(std::vector<s_autospell> &spell, uint16 id, uint16 lv, short rate, short battle_flag, t_itemid card_id, uint8 flag)
{
	if (spell.size() == MAX_PC_BONUS) {
		ShowWarning("pc_bonus_autospell: Reached max (%d) number of autospells per character!\n", MAX_PC_BONUS);
		return;
	}

	if (!rate)
		return;

	if (!(battle_flag&BF_RANGEMASK))
		battle_flag |= BF_SHORT | BF_LONG; //No range defined? Use both.
	if (!(battle_flag&BF_WEAPONMASK))
		battle_flag |= BF_WEAPON; //No attack type defined? Use weapon.
	if (!(battle_flag&BF_SKILLMASK)) {
		if (battle_flag&(BF_MAGIC | BF_MISC))
			battle_flag |= BF_SKILL; //These two would never trigger without BF_SKILL
		if (battle_flag&BF_WEAPON)
			battle_flag |= BF_NORMAL; //By default autospells should only trigger on normal weapon attacks.
	}

	for (auto &it : spell) {
		if ((it.card_id == card_id || it.rate < 0 || rate < 0) && it.id == id && it.lv == lv && it.battle_flag == battle_flag && it.flag == flag) {
			if (!battle_config.autospell_stacking && it.rate > 0 && rate > 0) // Stacking disabled
				return;
			it.rate = util::safe_addition_cap(it.rate, rate, (short)1000);
			return;
		}
	}

	struct s_autospell entry = {};

	if (rate < -1000 || rate > 1000)
		ShowWarning("pc_bonus_autospell: Item bonus rate %d exceeds -1000~1000 range, capping.\n", rate);

	entry.id = id;
	entry.lv = lv;
	entry.rate = cap_value(rate, -1000, 1000);
	entry.battle_flag = battle_flag;
	entry.card_id = card_id;
	entry.flag = flag;

	spell.push_back(entry);
}

/**
 * Add auto spell bonus for player while using skills
 * @param spell: Spell array
 * @param src_skill: Trigger skill
 * @param id: Support or target type
 * @param lv: Skill level
 * @param rate: Success chance
 * @param card_id: Used to prevent card stacking
 * @param flag: Flags used for extra arguments
 *              &1: forces the skill to be casted on self, rather than on the target of skill
 *              &2: random skill level in [1..lv] is chosen
 */
static void pc_bonus_autospell_onskill(std::vector<s_autospell> &spell, uint16 src_skill, uint16 id, uint16 lv, short rate, t_itemid card_id, uint8 flag)
{
	if (spell.size() == MAX_PC_BONUS) {
		ShowWarning("pc_bonus_autospell_onskill: Reached max (%d) number of autospells per character!\n", MAX_PC_BONUS);
		return;
	}

	if (!rate)
		return;

	struct s_autospell entry = {};

	if (rate < -1000 || rate > 1000)
		ShowWarning("pc_bonus_onskill: Item bonus rate %d exceeds -1000~1000 range, capping.\n", rate);

	entry.trigger_skill = src_skill;
	entry.id = id;
	entry.lv = lv;
	entry.rate = cap_value(rate, -1000, 1000);
	entry.card_id = card_id;
	entry.flag = flag;

	spell.push_back(entry);
}

/**
 * Add inflict effect bonus for player while attacking/attacked
 * @param effect: Effect array
 * @param sc: SC/Effect type
 * @param rate: Success chance
 * @param arrow_rate: success chance if bonus comes from arrow-type item
 * @param flag: Target flag
 * @param duration: Duration. If 0 use default duration lookup for associated skill with level 7
 */
static void pc_bonus_addeff(std::vector<s_addeffect> &effect, enum sc_type sc, int rate, short arrow_rate, unsigned char flag, unsigned int duration)
{
	if (effect.size() == MAX_PC_BONUS) {
		ShowWarning("pc_bonus_addeff: Reached max (%d) number of add effects per character!\n", MAX_PC_BONUS);
		return;
	}

	if (!(flag&(ATF_SHORT | ATF_LONG)))
		flag |= ATF_SHORT | ATF_LONG; //Default range: both
	if (!(flag&(ATF_TARGET | ATF_SELF)))
		flag |= ATF_TARGET; //Default target: enemy.
	if (!(flag&(ATF_WEAPON | ATF_MAGIC | ATF_MISC)))
		flag |= ATF_WEAPON; //Default type: weapon.

	if (!duration)
		duration = (unsigned int)skill_get_time2(status_db.getSkill(sc), 7);

	for (auto &it : effect) {
		if (it.sc == sc && it.flag == flag) {
			it.rate = util::safe_addition_cap(it.rate, rate, INT_MAX);
			it.arrow_rate = util::safe_addition_cap(it.arrow_rate, arrow_rate, (short)SHRT_MAX);
			it.duration = umax(it.duration, duration);
			return;
		}
	}

	struct s_addeffect entry = {};

	entry.sc = sc;
	entry.rate = rate;
	entry.arrow_rate = arrow_rate;
	entry.flag = flag;
	entry.duration = duration;

	effect.push_back(entry);
}

/**
 * Add inflict effect bonus for player while attacking using skill
 * @param effect: Effect array
 * @param sc: SC/Effect type
 * @param rate: Success chance
 * @param skill_id: Skill to cast
 * @param target: Target type
 * @param duration: Duration. If 0 use default duration lookup for associated skill with level 7
 */
static void pc_bonus_addeff_onskill(std::vector<s_addeffectonskill> &effect, enum sc_type sc, int rate, short skill_id, unsigned char target, unsigned int duration)
{
	if (effect.size() == MAX_PC_BONUS) {
		ShowWarning("pc_bonus_addeff_onskill: Reached max (%d) number of add effects per character!\n", MAX_PC_BONUS);
		return;
	}

	if (!duration)
		duration = (unsigned int)skill_get_time2(status_db.getSkill(sc), 7);

	for (auto &it : effect) {
		if (it.sc == sc && it.skill_id == skill_id && it.target == target) {
			it.rate = util::safe_addition_cap(it.rate, rate, INT_MAX);
			it.duration = umax(it.duration, duration);
			return;
		}
	}

	struct s_addeffectonskill entry = {};

	entry.sc = sc;
	entry.rate = rate;
	entry.skill_id = skill_id;
	entry.target = target;
	entry.duration = duration;

	effect.push_back(entry);
}

/**
 * Adjust/add drop rate modifier for player
 * @param drop: Player's sd->add_drop (struct s_add_drop)
 * @param nameid: item id that will be dropped
 * @param group: group id
 * @param class_: target class
 * @param race: target race. if < 0, means monster_id
 * @param rate: rate value: 1 ~ 10000. If < 0, it will be multiplied with mob level/10
 */
static void pc_bonus_item_drop(std::vector<s_add_drop> &drop, t_itemid nameid, uint16 group, int class_, short race, int rate)
{
	if (!nameid && !group) {
		ShowWarning("pc_bonus_item_drop: No Item ID nor Item Group ID specified.\n");
		return;
	}
	if (nameid && !itemdb_exists(nameid)) {
		ShowWarning("pc_bonus_item_drop: Invalid item id %u\n",nameid);
		return;
	}
	if (group && !itemdb_group.exists(group)) {
		ShowWarning("pc_bonus_item_drop: Invalid Item Group %hu\n",group);
		return;
	}

	if (drop.size() == MAX_PC_BONUS) {
		ShowWarning("pc_bonus_item_drop: Reached max (%d) number of added drops per character! (nameid: %u group: %d class_: %d race: %d rate: %d)\n", MAX_PC_BONUS, nameid, group, class_, race, rate);
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

	for (auto &it : drop) {
		if (it.nameid == nameid && it.group == group && it.race == race && it.class_ == class_) {
			if ((rate < 0 && it.rate < 0) || (rate > 0 && it.rate > 0)) //Adjust the rate if it has same classification
				it.rate = util::safe_addition_cap(it.rate, rate, 10000);
			return;
		}
	}

	struct s_add_drop entry = {};

	if (rate < -10000 || rate > 10000)
		ShowWarning("pc_bonus_item_drop: Item bonus rate %d exceeds -10000~10000 range, capping.\n", rate);

	entry.nameid = nameid;
	entry.group = group;
	entry.race = race;
	entry.class_ = class_;
	entry.rate = cap_value(rate, -10000, 10000);

	drop.push_back(entry);
}

s_autobonus::~s_autobonus(){
	if( this->active != INVALID_TIMER ){
		delete_timer( this->active, pc_endautobonus );
		this->active = INVALID_TIMER;
	}

	if( this->bonus_script != nullptr ){
		aFree( this->bonus_script );
		this->bonus_script = nullptr;
	}

	if( this->other_script != nullptr ){
		aFree( this->other_script );
		this->other_script = nullptr;
	}
}

/**
 * Add autobonus to player when attacking/attacked
 * @param bonus: Bonus array
 * @param script: Script to execute
 * @param rate: Success chance
 * @param dur: Duration
 * @param flag: Battle flag/skill
 * @param other_script: Secondary script to execute
 * @param pos: Item equip position
 * @param onskill: Skill used to trigger autobonus
 * @return True on success or false otherwise
 */
bool pc_addautobonus(std::vector<std::shared_ptr<s_autobonus>> &bonus, const char *script, short rate, unsigned int dur, uint16 flag, const char *other_script, unsigned int pos, bool onskill){
	// Check if the same bonus already exists
	for( std::shared_ptr<s_autobonus> autobonus : bonus ){
		// Compare based on position and bonus script
		if( autobonus->pos == pos && strcmp( script, autobonus->bonus_script ) == 0 ){
			return false;
		}
	}

	if (bonus.size() == MAX_PC_BONUS) {
		ShowWarning("pc_addautobonus: Reached max (%d) number of autobonus per character!\n", MAX_PC_BONUS);
		return false;
	}

	if (!onskill) {
		if (!(flag&BF_RANGEMASK))
			flag |= BF_SHORT | BF_LONG; //No range defined? Use both.
		if (!(flag&BF_WEAPONMASK))
			flag |= BF_WEAPON; //No attack type defined? Use weapon.
		if (!(flag&BF_SKILLMASK)) {
			if (flag&(BF_MAGIC | BF_MISC))
				flag |= BF_SKILL; //These two would never trigger without BF_SKILL
			if (flag&BF_WEAPON)
				flag |= BF_NORMAL | BF_SKILL;
		}
	}

	std::shared_ptr<s_autobonus> entry = std::make_shared<s_autobonus>();

	if (rate < -10000 || rate > 10000)
		ShowWarning("pc_addautobonus: Item bonus rate %d exceeds -10000~10000 range, capping.\n", rate);

	entry->rate = cap_value(rate, -10000, 10000);
	entry->duration = dur;
	entry->active = INVALID_TIMER;
	entry->atk_type = flag;
	entry->pos = pos;
	entry->bonus_script = aStrdup(script);
	entry->other_script = (other_script ? aStrdup(other_script) : NULL);

	bonus.push_back(entry);

	return true;
}

/**
 * Remove an autobonus from player
 * @param sd: Player data
 * @param bonus: Autobonus array
 * @param restore: Run script on clearing or not
 */
void pc_delautobonus(struct map_session_data &sd, std::vector<std::shared_ptr<s_autobonus>> &bonus, bool restore){
	std::vector<std::shared_ptr<s_autobonus>>::iterator it = bonus.begin();

	while( it != bonus.end() ){
		std::shared_ptr<s_autobonus> b = *it;

		if( b->active != INVALID_TIMER && restore && b->bonus_script != nullptr ){
			unsigned int equip_pos_idx = 0;

			// Create a list of all equipped positions to see if all items needed for the autobonus are still present [Playtester]
			for (uint8 j = 0; j < EQI_MAX; j++) {
				if (sd.equip_index[j] >= 0)
					equip_pos_idx |= sd.inventory.u.items_inventory[sd.equip_index[j]].equip;
			}

			if( ( equip_pos_idx&b->pos ) == b->pos ){
				script_run_autobonus(b->bonus_script, &sd, b->pos);
			}else{
				// Not all required items equipped anymore
				restore = false;
			}
		}

		if( restore ){
			it++;
			continue;
		}

		it = bonus.erase(it);
	}
}

/**
 * Execute autobonus on player
 * @param sd: Player data
 * @param autobonus: Autobonus to run
 */
void pc_exeautobonus(struct map_session_data &sd, std::vector<std::shared_ptr<s_autobonus>> *bonus, std::shared_ptr<s_autobonus> autobonus)
{
	if (autobonus->active != INVALID_TIMER)
		delete_timer(autobonus->active, pc_endautobonus);

	if( autobonus->other_script )
	{
		int j;
		unsigned int equip_pos_idx = 0;
		//Create a list of all equipped positions to see if all items needed for the autobonus are still present [Playtester]
		for(j = 0; j < EQI_MAX; j++) {
			if(sd.equip_index[j] >= 0)
				equip_pos_idx |= sd.inventory.u.items_inventory[sd.equip_index[j]].equip;
		}
		if((equip_pos_idx&autobonus->pos) == autobonus->pos)
			script_run_autobonus(autobonus->other_script,&sd,autobonus->pos);
	}

	autobonus->active = add_timer(gettick()+autobonus->duration, pc_endautobonus, sd.bl.id, (intptr_t)bonus);
	status_calc_pc(&sd,SCO_FORCE);
}

/**
 * Remove autobonus timer from player
 */
TIMER_FUNC(pc_endautobonus){
	struct map_session_data *sd = map_id2sd(id);
	std::vector<std::shared_ptr<s_autobonus>> *bonus = (std::vector<std::shared_ptr<s_autobonus>> *)data;

	nullpo_ret(sd);
	nullpo_ret(bonus);

	for( std::shared_ptr<s_autobonus> autobonus : *bonus ){
		if( autobonus->active == tid ){
			autobonus->active = INVALID_TIMER;
			break;
		}
	}
	
	status_calc_pc(sd,SCO_FORCE);
	return 0;
}

/**
 * Add element bonus to player when attacking
 * @param sd: Player data
 * @param ele: Element to adjust
 * @param rate: Success chance
 * @param flag: Battle flag
 */
static void pc_bonus_addele(struct map_session_data* sd, unsigned char ele, short rate, short flag)
{
	nullpo_retv(sd);

	struct weapon_data *wd = (sd->state.lr_flag ? &sd->left_weapon : &sd->right_weapon);

	if (wd->addele2.size() == MAX_PC_BONUS) {
		ShowWarning("pc_bonus_addele: Reached max (%d) number of add element damage bonuses per character!\n", MAX_PC_BONUS);
		return;
	}

	if (!(flag&BF_RANGEMASK))
		flag |= BF_SHORT | BF_LONG;
	if (!(flag&BF_WEAPONMASK))
		flag |= BF_WEAPON;
	if (!(flag&BF_SKILLMASK)) {
		if (flag&(BF_MAGIC | BF_MISC))
			flag |= BF_SKILL;
		if (flag&BF_WEAPON)
			flag |= BF_NORMAL | BF_SKILL;
	}

	for (auto &it : wd->addele2) {
		if (it.ele == ele && it.flag == flag) {
			it.rate = util::safe_addition_cap(it.rate, rate, (short)10000);
			return;
		}
	}

	struct s_addele2 entry = {};

	if (rate < -10000 || rate > 10000)
		ShowWarning("pc_bonus_addele: Item bonus rate %d exceeds -10000~10000 range, capping.\n", rate);

	entry.ele = ele;
	entry.rate = cap_value(rate, -10000, 10000);
	entry.flag = flag;

	wd->addele2.push_back(entry);
}

/**
 * Reduce element bonus to player when attacking
 * @param sd: Player data
 * @param ele: Element to adjust
 * @param rate: Success chance
 * @param flag: Battle flag
 */
static void pc_bonus_subele(struct map_session_data* sd, unsigned char ele, short rate, short flag)
{
	nullpo_retv(sd);

	if (sd->subele2.size() == MAX_PC_BONUS) {
		ShowWarning("pc_bonus_subele: Reached max (%d) number of resist element damage bonuses per character!\n", MAX_PC_BONUS);
		return;
	}

	if (!(flag&BF_RANGEMASK))
		flag |= BF_SHORT | BF_LONG;
	if (!(flag&BF_WEAPONMASK))
		flag |= BF_WEAPON;
	if (!(flag&BF_SKILLMASK)) {
		if (flag&(BF_MAGIC | BF_MISC))
			flag |= BF_SKILL;
		if (flag&BF_WEAPON)
			flag |= BF_NORMAL | BF_SKILL;
	}

	for (auto &it : sd->subele2) {
		if (it.ele == ele && it.flag == flag) {
			it.rate = util::safe_addition_cap(it.rate, rate, (short)10000);
			return;
		}
	}

	struct s_addele2 entry = {};

	if (rate < -10000 || rate > 10000)
		ShowWarning("pc_bonus_subele: Item bonus rate %d exceeds -10000~10000 range, capping.\n", rate);

	entry.ele = ele;
	entry.rate = cap_value(rate, -10000, 10000);
	entry.flag = flag;

	sd->subele2.push_back(entry);
}

/**
 * Adjust race damage to target when attacking
 * @param sd: Player data
 * @param race: Race to adjust
 * @param rate: Success chance
 * @param flag: Battle flag
*/
static void pc_bonus_subrace(struct map_session_data* sd, unsigned char race, short rate, short flag)
{
	if (sd->subrace3.size() == MAX_PC_BONUS) {
		ShowWarning("pc_bonus_subrace: Reached max (%d) number of resist race damage bonuses per character!\n", MAX_PC_BONUS);
		return;
	}

	if (!(flag&BF_RANGEMASK))
		flag |= BF_SHORT | BF_LONG;
	if (!(flag&BF_WEAPONMASK))
		flag |= BF_WEAPON;
	if (!(flag&BF_SKILLMASK)) {
		if (flag&(BF_MAGIC | BF_MISC))
			flag |= BF_SKILL;
		if (flag&BF_WEAPON)
			flag |= BF_NORMAL | BF_SKILL;
	}

	for (auto &it : sd->subrace3) {
		if (it.race == race && it.flag == flag) {
			it.rate = util::safe_addition_cap(it.rate, rate, (short)10000);
			return;
		}
	}

	struct s_addrace2 entry = {};

	if (rate < -10000 || rate > 10000)
		ShowWarning("pc_bonus_subrace: Item bonus rate %d exceeds -10000~10000 range, capping.\n", rate);

	entry.race = race;
	entry.rate = cap_value(rate, -10000, 10000);
	entry.flag = flag;

	sd->subrace3.push_back(entry);
}
/**
 * General item bonus for player
 * @param bonus: Bonus array
 * @param id: Key
 * @param val: Value
 * @param cap_rate: If Value is a rate value that needs to be capped
 */
static void pc_bonus_itembonus(std::vector<s_item_bonus> &bonus, uint16 id, int val, bool cap_rate)
{
	for (auto &it : bonus) {
		if (it.id == id) {
			if (cap_rate)
				it.val = util::safe_addition_cap(it.val, val, 10000);
			else
				it.val += val;
			return;
		}
	}

	struct s_item_bonus entry = {};

	if (cap_rate && (val < -10000 || val > 10000)) {
		ShowWarning("pc_bonus_itembonus: Item bonus val %d exceeds -10000~10000 range, capping.\n", val);
		val = cap_value(val, -10000, 10000);
	}

	entry.id = id;
	entry.val = val;

	bonus.push_back(entry);
}

/**
 * Remove HP/SP to player when attacking
 * @param bonus: Bonus array
 * @param rate: Success chance
 * @param per: Percentage of HP/SP to vanish
 * @param flag: Battle flag
 */
static void pc_bonus_addvanish(std::vector<s_vanish_bonus> &bonus, int16 rate, int16 per, int flag) {
	if (bonus.size() == MAX_PC_BONUS) {
		ShowWarning("pc_bonus_addvanish: Reached max (%d) number of vanish damage bonuses per character!\n", MAX_PC_BONUS);
		return;
	}

	if (!(flag&BF_RANGEMASK))
		flag |= BF_SHORT | BF_LONG;
	if (!(flag&BF_WEAPONMASK))
		flag |= BF_WEAPON;
	if (!(flag&BF_SKILLMASK)) {
		if (flag&(BF_MAGIC | BF_MISC))
			flag |= BF_SKILL;
		if (flag&BF_WEAPON)
			flag |= BF_NORMAL | BF_SKILL;
	}

	for (auto &it : bonus) {
		if (it.flag == flag) {
			it.rate = util::safe_addition_cap(it.rate, rate, (int16)10000);
			it.per += per;
			return;
		}
	}

	struct s_vanish_bonus entry = {};

	if (rate < -10000 || rate > 10000)
		ShowWarning("pc_bonus_addvanish: Item bonus rate %d exceeds -10000~10000 range, capping.\n", rate);

	entry.rate = cap_value(rate, -10000, 10000);
	entry.per = per;
	entry.flag = flag;

	bonus.push_back(entry);
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
				sd->indexed_bonus.param_bonus[type-SP_STR]+=val;
			break;
		case SP_POW:
		case SP_STA:
		case SP_WIS:
		case SP_SPL:
		case SP_CON:
		case SP_CRT:
			if (sd->state.lr_flag != 2)
				sd->indexed_bonus.param_bonus[type - SP_POW + PARAM_POW] += val;
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
				bonus = sd->bonus.eatk + val;
				sd->bonus.eatk = cap_value(bonus, SHRT_MIN, SHRT_MAX);
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
		case SP_PATK:
			if (sd->state.lr_flag != 2) {
				bonus = status->patk + val;
				status->patk = cap_value(bonus, SHRT_MIN, SHRT_MAX);
			}
			break;
		case SP_SMATK:
			if (sd->state.lr_flag != 2) {
				bonus = status->smatk + val;
				status->smatk = cap_value(bonus, SHRT_MIN, SHRT_MAX);
			}
			break;
		case SP_RES:
			if (sd->state.lr_flag != 2) {
				bonus = status->res + val;
				status->res = cap_value(bonus, SHRT_MIN, SHRT_MAX);
			}
			break;
		case SP_MRES:
			if (sd->state.lr_flag != 2) {
				bonus = status->mres + val;
				status->mres = cap_value(bonus, SHRT_MIN, SHRT_MAX);
			}
			break;
		case SP_HPLUS:
			if (sd->state.lr_flag != 2) {
				bonus = status->hplus + val;
				status->hplus = cap_value(bonus, SHRT_MIN, SHRT_MAX);
			}
			break;
		case SP_CRATE:
			if (sd->state.lr_flag != 2) {
				bonus = status->crate + val;
				status->crate = cap_value(bonus, SHRT_MIN, SHRT_MAX);
			}
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
		case SP_MAXAP:
			if (sd->state.lr_flag == 2)
				break;
			sd->bonus.ap += val;
			break;
		case SP_MAXHPRATE:
			if(sd->state.lr_flag != 2)
				sd->hprate+=val;
			break;
		case SP_MAXSPRATE:
			if(sd->state.lr_flag != 2)
				sd->sprate+=val;
			break;
		case SP_MAXAPRATE:
			if (sd->state.lr_flag != 2)
				sd->aprate += val;
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
		case SP_PATK_RATE:
			if (sd->state.lr_flag != 2)
				sd->patk_rate += val;
			break;
		case SP_SMATK_RATE:
			if (sd->state.lr_flag != 2)
				sd->smatk_rate += val;
			break;
		case SP_RES_RATE:
			if (sd->state.lr_flag != 2)
				sd->res_rate += val;
			break;
		case SP_MRES_RATE:
			if (sd->state.lr_flag != 2)
				sd->mres_rate += val;
			break;
		case SP_HPLUS_RATE:
			if (sd->state.lr_flag != 2)
				sd->hplus_rate += val;
			break;
		case SP_CRATE_RATE:
			if (sd->state.lr_flag != 2)
				sd->crate_rate += val;
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
				clif_status_load(&sd->bl, EFST_CLAIRVOYANCE, 1);
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
		case SP_REDUCE_DAMAGE_RETURN:
			if (sd->state.lr_flag != 2)
				sd->bonus.reduce_damage_return += val;
			break;
		case SP_ALL_STATS:	// [Valaris]
			if(sd->state.lr_flag!=2) {
				sd->indexed_bonus.param_bonus[SP_STR-SP_STR]+=val;
				sd->indexed_bonus.param_bonus[SP_AGI-SP_STR]+=val;
				sd->indexed_bonus.param_bonus[SP_VIT-SP_STR]+=val;
				sd->indexed_bonus.param_bonus[SP_INT-SP_STR]+=val;
				sd->indexed_bonus.param_bonus[SP_DEX-SP_STR]+=val;
				sd->indexed_bonus.param_bonus[SP_LUK-SP_STR]+=val;
			}
			break;
		case SP_ALL_TRAIT_STATS:
			if (sd->state.lr_flag != 2) {
				sd->indexed_bonus.param_bonus[PARAM_POW] += val;
				sd->indexed_bonus.param_bonus[PARAM_STA] += val;
				sd->indexed_bonus.param_bonus[PARAM_WIS] += val;
				sd->indexed_bonus.param_bonus[PARAM_SPL] += val;
				sd->indexed_bonus.param_bonus[PARAM_CON] += val;
				sd->indexed_bonus.param_bonus[PARAM_CRT] += val;
			}
			break;
		case SP_AGI_VIT:	// [Valaris]
			if(sd->state.lr_flag!=2) {
				sd->indexed_bonus.param_bonus[SP_AGI-SP_STR]+=val;
				sd->indexed_bonus.param_bonus[SP_VIT-SP_STR]+=val;
			}
			break;
		case SP_AGI_DEX_STR:	// [Valaris]
			if(sd->state.lr_flag!=2) {
				sd->indexed_bonus.param_bonus[SP_AGI-SP_STR]+=val;
				sd->indexed_bonus.param_bonus[SP_DEX-SP_STR]+=val;
				sd->indexed_bonus.param_bonus[SP_STR-SP_STR]+=val;
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
		case SP_SHORT_ATK_RATE:
			if(sd->state.lr_flag != 2)	//[Lupus] it should stack, too. As any other cards rate bonuses
				sd->bonus.short_attack_atk_rate+=val;
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
				sd->bonus.delayrate -= val;
			break;
		case SP_CRIT_ATK_RATE:
			if(sd->state.lr_flag != 2)
				sd->bonus.crit_atk_rate += val;
			break;
		case SP_CRIT_DEF_RATE:
			if(sd->state.lr_flag != 2)
				sd->bonus.crit_def_rate += val;
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
		case SP_LONG_SP_GAIN_VALUE:
			if(!sd->state.lr_flag)
				sd->bonus.long_sp_gain_value += val;
		case SP_LONG_HP_GAIN_VALUE:
			if(!sd->state.lr_flag)
				sd->bonus.long_hp_gain_value += val;
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
		case SP_ADD_VARIABLECAST:
			if (sd->state.lr_flag != 2)
				sd->bonus.add_varcast += val;
			break;
#ifdef RENEWAL_CAST
		case SP_FIXCASTRATE:
			if(sd->state.lr_flag != 2)
				sd->bonus.fixcastrate = min(sd->bonus.fixcastrate,val);
			break;
		case SP_ADD_FIXEDCAST:
			if(sd->state.lr_flag != 2)
				sd->bonus.add_fixcast += val;
			break;
		case SP_CASTRATE:
		case SP_VARCASTRATE:
			if(sd->state.lr_flag != 2)
				sd->bonus.varcastrate -= val;
			break;
#else
		case SP_ADD_FIXEDCAST:
		case SP_FIXCASTRATE:
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
				sd->add_max_weight += val;
			break;
		case SP_ABSORB_DMG_MAXHP: // bonus bAbsorbDmgMaxHP,n;
			sd->bonus.absorb_dmg_maxhp = max(sd->bonus.absorb_dmg_maxhp, val);
			break;
		case SP_ABSORB_DMG_MAXHP2:
			sd->bonus.absorb_dmg_maxhp2 = max(sd->bonus.absorb_dmg_maxhp2, val);
			break;
		case SP_CRITICAL_RANGEATK: // bonus bCriticalLong,n;
			if (sd->state.lr_flag != 2)
				sd->bonus.critical_rangeatk += val*10;
			else
				sd->bonus.arrow_cri += val*10;
			break;
		case SP_WEAPON_ATK_RATE:
			if (sd->state.lr_flag != 2)
				sd->bonus.weapon_atk_rate += val;
			break;
		case SP_WEAPON_MATK_RATE:
			if (sd->state.lr_flag != 2)
				sd->bonus.weapon_matk_rate += val;
			break;
		case SP_NO_MADO_FUEL:
			if (sd->state.lr_flag != 2)
				sd->special_state.no_mado_fuel = 1;
			break;
		case SP_NO_WALK_DELAY:
			if (sd->state.lr_flag != 2)
				sd->special_state.no_walk_delay = 1;
			break;
		case SP_ADD_ITEM_SPHEAL_RATE:
			if(sd->state.lr_flag != 2)
				sd->bonus.itemsphealrate2 += val;
			break;
		default:
			if (current_equip_combo_pos > 0) {
				ShowWarning("pc_bonus: unknown bonus type %d %d in a combo with item #%u\n", type, val, sd->inventory_data[pc_checkequip( sd, current_equip_combo_pos )]->nameid);
			}
			else if (current_equip_card_id > 0 || current_equip_item_index > 0) {
				ShowWarning("pc_bonus: unknown bonus type %d %d in item #%u\n", type, val, current_equip_card_id ? current_equip_card_id : sd->inventory_data[current_equip_item_index]->nameid);
			}
			else {
				ShowWarning("pc_bonus: unknown bonus type %d %d in unknown usage. Report this!\n", type, val);
			}
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
	nullpo_retv(sd);

	switch(type){
	case SP_ADDELE: // bonus2 bAddEle,e,x;
		PC_BONUS_CHK_ELEMENT(type2,SP_ADDELE);
		if(!sd->state.lr_flag || sd->state.lr_flag == 3)
			sd->right_weapon.addele[type2]+=val;
		else if(sd->state.lr_flag == 1)
			sd->left_weapon.addele[type2]+=val;
		else if(sd->state.lr_flag == 2)
			sd->indexed_bonus.arrow_addele[type2]+=val;
		break;
	case SP_ADDRACE: // bonus2 bAddRace,r,x;
		PC_BONUS_CHK_RACE(type2,SP_ADDRACE);
		if(!sd->state.lr_flag || sd->state.lr_flag == 3)
			sd->right_weapon.addrace[type2]+=val;
		else if(sd->state.lr_flag == 1)
			sd->left_weapon.addrace[type2]+=val;
		else if(sd->state.lr_flag == 2)
			sd->indexed_bonus.arrow_addrace[type2]+=val;
		break;
	case SP_ADDCLASS: // bonus2 bAddClass,c,x;
		PC_BONUS_CHK_CLASS(type2,SP_ADDCLASS);
		if(!sd->state.lr_flag || sd->state.lr_flag == 3)
			sd->right_weapon.addclass[type2]+=val;
		else if(sd->state.lr_flag == 1)
			sd->left_weapon.addclass[type2]+=val;
		else if(sd->state.lr_flag == 2)
			sd->indexed_bonus.arrow_addclass[type2]+=val;
		break;
	case SP_ADDSIZE: // bonus2 bAddSize,s,x;
		PC_BONUS_CHK_SIZE(type2,SP_ADDSIZE);
		if(!sd->state.lr_flag || sd->state.lr_flag == 3)
			sd->right_weapon.addsize[type2]+=val;
		else if(sd->state.lr_flag == 1)
			sd->left_weapon.addsize[type2]+=val;
		else if(sd->state.lr_flag == 2)
			sd->indexed_bonus.arrow_addsize[type2]+=val;
		break;
	case SP_SUBELE: // bonus2 bSubEle,e,x;
		PC_BONUS_CHK_ELEMENT(type2,SP_SUBELE);
		if(sd->state.lr_flag != 2)
			sd->indexed_bonus.subele_script[type2] += val;
		break;
	case SP_SUBRACE: // bonus2 bSubRace,r,x;
		PC_BONUS_CHK_RACE(type2,SP_SUBRACE);
		if(sd->state.lr_flag != 2)
			sd->indexed_bonus.subrace[type2]+=val;
		break;
	case SP_SUBCLASS: // bonus2 bSubClass,c,x;
		PC_BONUS_CHK_CLASS(type2,SP_SUBCLASS);
		if(sd->state.lr_flag != 2)
			sd->indexed_bonus.subclass[type2]+=val;
		break;
	case SP_ADDEFF: // bonus2 bAddEff,eff,n;
		PC_BONUS_CHK_SC(type2,SP_ADDEFF);
		pc_bonus_addeff(sd->addeff, (sc_type)type2, sd->state.lr_flag != 2 ? val : 0, sd->state.lr_flag == 2 ? val : 0, 0, 0);
		break;
	case SP_ADDEFF2: // bonus2 bAddEff2,eff,n;
		PC_BONUS_CHK_SC(type2,SP_ADDEFF2);
		pc_bonus_addeff(sd->addeff, (sc_type)type2, sd->state.lr_flag != 2 ? val : 0, sd->state.lr_flag == 2 ? val : 0, ATF_SELF, 0);
		break;
	case SP_RESEFF: // bonus2 bResEff,eff,n;
		if(sd->state.lr_flag == 2)
			break;
		if (sd->reseff.size() == MAX_PC_BONUS) {
			ShowWarning("pc_bonus2: SP_RESEFF: Reached max (%d) number of resistance bonuses per character!\n", MAX_PC_BONUS);
			break;
		}

		pc_bonus_itembonus(sd->reseff, type2, val, true);
		break;
	case SP_MAGIC_ADDELE: // bonus2 bMagicAddEle,e,x;
		PC_BONUS_CHK_ELEMENT(type2,SP_MAGIC_ADDELE);
		if(sd->state.lr_flag != 2)
			sd->indexed_bonus.magic_addele_script[type2] += val;
		break;
	case SP_MAGIC_ADDRACE: // bonus2 bMagicAddRace,r,x;
		PC_BONUS_CHK_RACE(type2,SP_MAGIC_ADDRACE);
		if(sd->state.lr_flag != 2)
			sd->indexed_bonus.magic_addrace[type2]+=val;
		break;
	case SP_MAGIC_ADDCLASS: // bonus2 bMagicAddClass,c,x;
		PC_BONUS_CHK_CLASS(type2,SP_MAGIC_ADDCLASS);
		if(sd->state.lr_flag != 2)
			sd->indexed_bonus.magic_addclass[type2]+=val;
		break;
	case SP_MAGIC_ADDSIZE: // bonus2 bMagicAddSize,s,x;
		PC_BONUS_CHK_SIZE(type2,SP_MAGIC_ADDSIZE);
		if(sd->state.lr_flag != 2)
			sd->indexed_bonus.magic_addsize[type2]+=val;
		break;
	case SP_MAGIC_ATK_ELE: // bonus2 bMagicAtkEle,e,x;
		PC_BONUS_CHK_ELEMENT(type2,SP_MAGIC_ATK_ELE);
		if(sd->state.lr_flag != 2)
			sd->indexed_bonus.magic_atk_ele[type2]+=val;
		break;
	case SP_ADD_DAMAGE_CLASS: { // bonus2 bAddDamageClass,mid,x;
			struct weapon_data *wd = (sd->state.lr_flag ? &sd->left_weapon : &sd->right_weapon);

			if (wd->add_dmg.size() == MAX_PC_BONUS) {
				ShowWarning("pc_bonus2: SP_ADD_DAMAGE_CLASS: Reached max (%d) number of add class damage bonuses per character!\n", MAX_PC_BONUS);
				break;
			}

			pc_bonus_itembonus(wd->add_dmg, type2, val, false);
		}
		break;
	case SP_ADD_MAGIC_DAMAGE_CLASS: // bonus2 bAddMagicDamageClass,mid,x;
		if(sd->state.lr_flag == 2)
			break;
		if (sd->add_mdmg.size() == MAX_PC_BONUS) {
			ShowWarning("pc_bonus2: SP_ADD_MAGIC_DAMAGE_CLASS: Reached max (%d) number of add class magic dmg bonuses per character!\n", MAX_PC_BONUS);
			break;
		}

		pc_bonus_itembonus(sd->add_mdmg, type2, val, false);
		break;
	case SP_ADD_DEF_MONSTER: // bonus2 bAddDefMonster,mid,x;
		if(sd->state.lr_flag == 2)
			break;
		if (sd->add_def.size() == MAX_PC_BONUS) {
			ShowWarning("pc_bonus2: SP_ADD_DEF_MONSTER: Reached max (%d) number of add class def bonuses per character!\n", MAX_PC_BONUS);
			break;
		}

		pc_bonus_itembonus(sd->add_def, type2, val, false);
		break;
	case SP_ADD_MDEF_MONSTER: // bonus2 bAddMDefMonster,mid,x;
		if(sd->state.lr_flag == 2)
			break;
		if (sd->add_mdef.size() == MAX_PC_BONUS) {
			ShowWarning("pc_bonus2: SP_ADD_MDEF_MONSTER: Reached max (%d) number of add class mdef bonuses per character!\n", MAX_PC_BONUS);
			break;
		}

		pc_bonus_itembonus(sd->add_mdef, type2, val, false);
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
			pc_bonus_addvanish(sd->sp_vanish, type2, val, BF_NORMAL);
		}
		break;
	case SP_HP_VANISH_RATE: // bonus2 bHPVanishRate,x,n;
		if(sd->state.lr_flag != 2) {
			pc_bonus_addvanish(sd->hp_vanish, type2, val, BF_NORMAL);
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
		sd->indexed_bonus.weapon_coma_ele[type2] += val;
		sd->special_state.bonus_coma = 1;
		break;
	case SP_WEAPON_COMA_RACE: // bonus2 bWeaponComaRace,r,n;
		PC_BONUS_CHK_RACE(type2,SP_WEAPON_COMA_RACE);
		if(sd->state.lr_flag == 2)
			break;
		sd->indexed_bonus.weapon_coma_race[type2] += val;
		sd->special_state.bonus_coma = 1;
		break;
	case SP_WEAPON_COMA_CLASS: // bonus2 bWeaponComaClass,c,n;
		PC_BONUS_CHK_CLASS(type2,SP_WEAPON_COMA_CLASS);
		if(sd->state.lr_flag == 2)
			break;
		sd->indexed_bonus.weapon_coma_class[type2] += val;
		sd->special_state.bonus_coma = 1;
		break;
	case SP_WEAPON_ATK: // bonus2 bWeaponAtk,w,n;
		if(sd->state.lr_flag != 2)
			sd->indexed_bonus.weapon_atk[type2]+=val;
		break;
	case SP_WEAPON_DAMAGE_RATE: // bonus2 bWeaponDamageRate,w,n;
		if(sd->state.lr_flag != 2)
			sd->indexed_bonus.weapon_damage_rate[type2]+=val;
		break;
	case SP_CRITICAL_ADDRACE: // bonus2 bCriticalAddRace,r,n;
		PC_BONUS_CHK_RACE(type2,SP_CRITICAL_ADDRACE);
		if(sd->state.lr_flag != 2)
			sd->indexed_bonus.critaddrace[type2] += val*10;
		break;
	case SP_ADDEFF_WHENHIT: // bonus2 bAddEffWhenHit,eff,n;
		PC_BONUS_CHK_SC(type2,SP_ADDEFF_WHENHIT);
		if(sd->state.lr_flag != 2)
			pc_bonus_addeff(sd->addeff_atked, (sc_type)type2, val, 0, 0, 0);
		break;
	case SP_SKILL_ATK: // bonus2 bSkillAtk,sk,n;
		if(sd->state.lr_flag == 2)
			break;
		if (sd->skillatk.size() == MAX_PC_BONUS) {
			ShowWarning("pc_bonus2: SP_SKILL_ATK: Reached max (%d) number of skills per character, bonus skill %d (+%d%%) lost.\n", MAX_PC_BONUS, type2, val);
			break;
		}

		pc_bonus_itembonus(sd->skillatk, type2, val, false);
		break;
	case SP_SKILL_HEAL: // bonus2 bSkillHeal,sk,n;
		if(sd->state.lr_flag == 2)
			break;
		if (sd->skillheal.size() == MAX_PC_BONUS) { // Better mention this so the array length can be updated. [Skotlex]
			ShowWarning("pc_bonus2: SP_SKILL_HEAL: Reached max (%d) number of skills per character, bonus skill %d (+%d%%) lost.\n", MAX_PC_BONUS, type2, val);
			break;
		}
		
		pc_bonus_itembonus(sd->skillheal, type2, val, false);
		break;
	case SP_SKILL_HEAL2: // bonus2 bSkillHeal2,sk,n;
		if(sd->state.lr_flag == 2)
			break;
		if (sd->skillheal2.size() == MAX_PC_BONUS) { // Better mention this so the array length can be updated. [Skotlex]
			ShowWarning("pc_bonus2: SP_SKILL_HEAL2: Reached max (%d) number of skills per character, bonus skill %d (+%d%%) lost.\n", MAX_PC_BONUS, type2, val);
			break;
		}
		
		pc_bonus_itembonus(sd->skillheal2, type2, val, false);
		break;
	case SP_ADD_SKILL_BLOW: // bonus2 bAddSkillBlow,sk,n;
		if(sd->state.lr_flag == 2)
			break;
		if (sd->skillblown.size() == MAX_PC_BONUS) { //Better mention this so the array length can be updated. [Skotlex]
			ShowWarning("pc_bonus2: SP_ADD_SKILL_BLOW: Reached max (%d) number of skills per character, bonus skill %d (%d) lost.\n", MAX_PC_BONUS, type2, val);
			break;
		}
		
		pc_bonus_itembonus(sd->skillblown, type2, val, false);
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
	case SP_REGEN_PERCENT_HP: // bonus2 bRegenPercentHP,n,t;
		if (sd->state.lr_flag != 2) {
			sd->percent_hp_regen.value = type2;
			sd->percent_hp_regen.rate = val;
		}
		break;
	case SP_REGEN_PERCENT_SP: // bonus2 bRegenPercentSP,n,t;
		if (sd->state.lr_flag != 2) {
			sd->percent_sp_regen.value = type2;
			sd->percent_sp_regen.rate = val;
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
			sd->indexed_bonus.subsize[type2]+=val;
		break;
	case SP_WEAPON_SUBSIZE: // bonus2 bWeaponSubSize,s,x;
		PC_BONUS_CHK_SIZE(type2, SP_WEAPON_SUBSIZE);
		if (sd->state.lr_flag != 2)
			sd->indexed_bonus.weapon_subsize[type2] += val;
		break;
	case SP_MAGIC_SUBSIZE: // bonus2 bMagicSubSize,s,x;
		PC_BONUS_CHK_SIZE(type2,SP_MAGIC_SUBSIZE);
		if(sd->state.lr_flag != 2)
			sd->indexed_bonus.magic_subsize[type2]+=val;
		break;
	case SP_SUBRACE2: // bonus2 bSubRace2,mr,x;
		PC_BONUS_CHK_RACE2(type2,SP_SUBRACE2);
		if(sd->state.lr_flag != 2)
			sd->indexed_bonus.subrace2[type2]+=val;
		break;
	case SP_ADD_ITEM_HEAL_RATE: // bonus2 bAddItemHealRate,iid,n;
		if(sd->state.lr_flag == 2)
			break;
		if( !item_db.exists( type2 ) ){
			ShowWarning("pc_bonus2: SP_ADD_ITEM_HEAL_RATE Invalid item with id %d\n", type2);
			break;
		}
		if (sd->itemhealrate.size() == MAX_PC_BONUS) {
			ShowWarning("pc_bonus2: SP_ADD_ITEM_HEAL_RATE: Reached max (%d) number of item heal bonuses per character!\n", MAX_PC_BONUS);
			break;
		}

		pc_bonus_itembonus(sd->itemhealrate, type2, val, false);
		break;
	case SP_ADD_ITEMGROUP_HEAL_RATE: // bonus2 bAddItemGroupHealRate,ig,n;
		if (sd->state.lr_flag == 2)
			break;
		if (!type2 || !itemdb_group.exists(type2)) {
			ShowWarning("pc_bonus2: SP_ADD_ITEMGROUP_HEAL_RATE: Invalid item group with id %d\n", type2);
			break;
		}
		if (sd->itemgrouphealrate.size() == MAX_PC_BONUS) {
			ShowWarning("pc_bonus2: SP_ADD_ITEMGROUP_HEAL_RATE: Reached max (%d) number of item heal bonuses per character!\n", MAX_PC_BONUS);
			break;
		}

		pc_bonus_itembonus(sd->itemgrouphealrate, type2, val, false);
		break;
	case SP_EXP_ADDRACE: // bonus2 bExpAddRace,r,x;
		PC_BONUS_CHK_RACE(type2,SP_EXP_ADDRACE);
		if(sd->state.lr_flag != 2)
			sd->indexed_bonus.expaddrace[type2]+=val;
		break;
	case SP_EXP_ADDCLASS: // bonus2 bExpAddClass,c,x;
		PC_BONUS_CHK_CLASS(type2,SP_EXP_ADDCLASS);
		if(sd->state.lr_flag != 2)
			sd->indexed_bonus.expaddclass[type2]+=val;
		break;
	case SP_SP_GAIN_RACE: // bonus2 bSPGainRace,r,n;
		PC_BONUS_CHK_RACE(type2,SP_SP_GAIN_RACE);
		if(sd->state.lr_flag != 2)
			sd->indexed_bonus.sp_gain_race[type2]+=val;
		break;
	case SP_ADD_MONSTER_DROP_ITEM: // bonus2 bAddMonsterDropItem,iid,n;
		if (sd->state.lr_flag != 2)
			pc_bonus_item_drop(sd->add_drop, type2, 0, CLASS_ALL, RC_NONE_, val);
		break;
	case SP_ADD_MONSTER_DROP_ITEMGROUP: // bonus2 bAddMonsterDropItemGroup,ig,n;
		if (sd->state.lr_flag != 2)
			pc_bonus_item_drop(sd->add_drop, 0, type2, CLASS_ALL, RC_NONE_, val);
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
			sd->indexed_bonus.ignore_mdef_by_race[type2] += val;
		break;
	case SP_IGNORE_MDEF_CLASS_RATE: // bonus2 bIgnoreMdefClassRate,c,n;
		PC_BONUS_CHK_CLASS(type2,SP_IGNORE_MDEF_CLASS_RATE);
		if(sd->state.lr_flag != 2)
			sd->indexed_bonus.ignore_mdef_by_class[type2] += val;
		break;
	case SP_IGNORE_DEF_RACE_RATE: // bonus2 bIgnoreDefRaceRate,r,n;
		PC_BONUS_CHK_RACE(type2,SP_IGNORE_DEF_RACE_RATE);
		if(sd->state.lr_flag != 2)
			sd->indexed_bonus.ignore_def_by_race[type2] += val;
		break;
	case SP_IGNORE_DEF_CLASS_RATE: // bonus2 bIgnoreDefClassRate,r,n;
		PC_BONUS_CHK_CLASS(type2, SP_IGNORE_DEF_CLASS_RATE);
		if (sd->state.lr_flag != 2)
			sd->indexed_bonus.ignore_def_by_class[type2] += val;
		break;
	case SP_SKILL_USE_SP_RATE: // bonus2 bSkillUseSPrate,sk,n;
		if(sd->state.lr_flag == 2)
			break;
		if (sd->skillusesprate.size() == MAX_PC_BONUS) {
			ShowWarning("pc_bonus2: SP_SKILL_USE_SP_RATE: Reached max (%d) number of skills per character, bonus skill %d (+%d%%) lost.\n", MAX_PC_BONUS, type2, val);
			break;
		}
		
		pc_bonus_itembonus(sd->skillusesprate, type2, val, true);
		break;
	case SP_SKILL_DELAY:
		if(sd->state.lr_flag == 2)
			break;
		if (sd->skilldelay.size() == MAX_PC_BONUS) {
			ShowWarning("pc_bonus2: SP_SKILL_DELAY: Reached max (%d) number of skills per character, bonus skill %d (%d) lost.\n", MAX_PC_BONUS, type2, val);
			break;
		}

		pc_bonus_itembonus(sd->skilldelay, type2, val, false);
		break;
	case SP_SKILL_COOLDOWN: // bonus2 bSkillCooldown,sk,t;
		if(sd->state.lr_flag == 2)
			break;
		if (sd->skillcooldown.size() == MAX_PC_BONUS) {
			ShowWarning("pc_bonus2: SP_SKILL_COOLDOWN: Reached max (%d) number of skills per character, bonus skill %d (%d) lost.\n", MAX_PC_BONUS, type2, val);
			break;
		}

		pc_bonus_itembonus(sd->skillcooldown, type2, val, false);
		break;
	case SP_SKILL_VARIABLECAST: // bonus2 bSkillVariableCast,sk,t;
		if (sd->state.lr_flag == 2)
			break;
		if (sd->skillvarcast.size() == MAX_PC_BONUS) {
			ShowWarning("pc_bonus2: SP_SKILL_VARIABLECAST: Reached max (%d) number of skills per character, bonus skill %d (%d) lost.\n", MAX_PC_BONUS, type2, val);
			break;
		}

		pc_bonus_itembonus(sd->skillvarcast, type2, val, false);
		break;
#ifdef RENEWAL_CAST
	case SP_SKILL_FIXEDCAST: // bonus2 bSkillFixedCast,sk,t;
		if(sd->state.lr_flag == 2)
			break;
		if (sd->skillfixcast.size() == MAX_PC_BONUS) {
			ShowWarning("pc_bonus2: SP_SKILL_FIXEDCAST: Reached max (%d) number of skills per character, bonus skill %d (%d) lost.\n", MAX_PC_BONUS, type2, val);
			break;
		}

		pc_bonus_itembonus(sd->skillfixcast, type2, val, false);
		break;
	case SP_CASTRATE: // bonus2 bCastrate,sk,n;
	case SP_VARCASTRATE: // bonus2 bVariableCastrate,sk,n;
		if(sd->state.lr_flag == 2)
			break;
		if (sd->skillcastrate.size() == MAX_PC_BONUS) {
			ShowWarning("pc_bonus2: SP_VARCASTRATE: Reached max (%d) number of skills per character, bonus skill %d (%d%%) lost.\n", MAX_PC_BONUS, type2, val);
			break;
		}

		pc_bonus_itembonus(sd->skillcastrate, type2, -val, true); // Send inversed value here
		break;
	case SP_FIXCASTRATE: // bonus2 bFixedCastrate,sk,n;
		if(sd->state.lr_flag == 2)
			break;
		if (sd->skillfixcastrate.size() == MAX_PC_BONUS) {
			ShowWarning("pc_bonus2: SP_FIXCASTRATE: Reached max (%d) number of skills per character, bonus skill %d (%d%%) lost.\n", MAX_PC_BONUS, type2, val);
			break;
		}

		pc_bonus_itembonus(sd->skillfixcastrate, type2, -val, true); // Send inversed value here
		break;
#else
	case SP_SKILL_FIXEDCAST: // bonus2 bSkillFixedCast,sk,t;
	case SP_FIXCASTRATE: // bonus2 bFixedCastrate,sk,n;
		//ShowWarning("pc_bonus2: Non-RENEWAL_CAST doesn't support this bonus %d.\n", type);
		break;
	case SP_VARCASTRATE: // bonus2 bVariableCastrate,sk,n;
	case SP_CASTRATE: // bonus2 bCastrate,sk,n;
		if(sd->state.lr_flag == 2)
			break;
		if (sd->skillcastrate.size() == MAX_PC_BONUS) { //Better mention this so the array length can be updated. [Skotlex]
			ShowWarning("pc_bonus2: %s: Reached max (%d) number of skills per character, bonus skill %d (%d%%) lost.\n", (type == SP_CASTRATE) ? "SP_CASTRATE" : "SP_VARCASTRATE", MAX_PC_BONUS, type2, val);
			break;
		}

		pc_bonus_itembonus(sd->skillcastrate, type2, val, true);
		break;
#endif
	case SP_SKILL_USE_SP: // bonus2 bSkillUseSP,sk,n;
		if(sd->state.lr_flag == 2)
			break;
		if (sd->skillusesp.size() == MAX_PC_BONUS) {
			ShowWarning("pc_bonus2: SP_SKILL_USE_SP: Reached max (%d) number of skills per character, bonus skill %d (%d) lost.\n", MAX_PC_BONUS, type2, val);
			break;
		}

		pc_bonus_itembonus(sd->skillusesp, type2, val, false);
		break;
	case SP_SUB_SKILL: // bonus2 bSubSkill,sk,n;
		if (sd->subskill.size() == MAX_PC_BONUS) {
			ShowWarning("pc_bonus2: SP_SUB_SKILL: Reached max (%d) number of skills per character, bonus skill %d (%d) lost.\n", MAX_PC_BONUS, type2, val);
			break;
		}

		pc_bonus_itembonus(sd->subskill, type2, val, false);
		break;
	case SP_SUBDEF_ELE: // bonus2 bSubDefEle,e,x;
		PC_BONUS_CHK_ELEMENT(type2,SP_SUBDEF_ELE);
		sd->indexed_bonus.subdefele[type2] += val;
		break;
	case SP_COMA_CLASS: // bonus2 bComaClass,c,n;
		PC_BONUS_CHK_CLASS(type2,SP_COMA_CLASS);
		sd->indexed_bonus.coma_class[type2] += val;
		sd->special_state.bonus_coma = 1;
		break;
	case SP_COMA_RACE: // bonus2 bComaRace,r,n;
		PC_BONUS_CHK_RACE(type2,SP_COMA_RACE);
		sd->indexed_bonus.coma_race[type2] += val;
		sd->special_state.bonus_coma = 1;
		break;
	case SP_MAGIC_ADDRACE2: // bonus2 bMagicAddRace2,mr,n;
		PC_BONUS_CHK_RACE2(type2, SP_MAGIC_ADDRACE2);
		if(sd->state.lr_flag != 2)
			sd->indexed_bonus.magic_addrace2[type2] += val;
		break;
	case SP_IGNORE_MDEF_RACE2_RATE: //bonus2 bIgnoreMdefRace2Rate,mr,n;
		PC_BONUS_CHK_RACE2(type2, SP_IGNORE_MDEF_RACE2);
		if (sd->state.lr_flag != 2)
			sd->indexed_bonus.ignore_mdef_by_race2[type2] += val;
		break;
	case SP_DROP_ADDRACE: // bonus2 bDropAddRace,r,x;
		PC_BONUS_CHK_RACE(type2, SP_DROP_ADDRACE);
		if (sd->state.lr_flag != 2)
			sd->indexed_bonus.dropaddrace[type2] += val;
		break;
	case SP_DROP_ADDCLASS: // bonus2 bDropAddClass,c,x;
		PC_BONUS_CHK_CLASS(type2, SP_DROP_ADDCLASS);
		if (sd->state.lr_flag != 2)
			sd->indexed_bonus.dropaddclass[type2] += val;
		break;
	case SP_MAGIC_SUBDEF_ELE: // bonus2 bMagicSubDefEle,e,x;
		PC_BONUS_CHK_ELEMENT(type2, SP_MAGIC_SUBDEF_ELE);
		sd->indexed_bonus.magic_subdefele[type2] += val;
		break;
	case SP_ADD_ITEM_SPHEAL_RATE: // bonus2 bAddItemSPHealRate,iid,n;
		if( sd->state.lr_flag == 2 ){
			break;
		}

		if( !item_db.exists( type2 ) ){
			ShowWarning( "pc_bonus2: SP_ADD_ITEM_SPHEAL_RATE Invalid item with id %d\n", type2 );
			break;
		}

		if( sd->itemsphealrate.size() == MAX_PC_BONUS ){
			ShowWarning( "pc_bonus2: SP_ADD_ITEM_SPHEAL_RATE: Reached max (%d) number of item SP heal bonuses per character!\n", MAX_PC_BONUS );
			break;
		}

		pc_bonus_itembonus( sd->itemsphealrate, type2, val, false );
		break;
	case SP_ADD_ITEMGROUP_SPHEAL_RATE: // bonus2 bAddItemGroupSPHealRate,ig,n;
		if( sd->state.lr_flag == 2 ){
			break;
		}

		if( !type2 || !itemdb_group.exists( type2 ) ){
			ShowWarning( "pc_bonus2: SP_ADD_ITEMGROUP_SPHEAL_RATE: Invalid item group with id %d\n", type2 );
			break;
		}

		if( sd->itemgroupsphealrate.size() == MAX_PC_BONUS ){
			ShowWarning( "pc_bonus2: SP_ADD_ITEMGROUP_SPHEAL_RATE: Reached max (%d) number of item SP heal bonuses per character!\n", MAX_PC_BONUS );
			break;
		}

		pc_bonus_itembonus( sd->itemgroupsphealrate, type2, val, false );
		break;
	default:
		if (current_equip_combo_pos > 0) {
			ShowWarning("pc_bonus2: unknown bonus type %d %d %d in a combo with item #%u\n", type, type2, val, sd->inventory_data[pc_checkequip( sd, current_equip_combo_pos )]->nameid);
		} 
		else if (current_equip_card_id > 0 || current_equip_item_index > 0) {
			ShowWarning("pc_bonus2: unknown bonus type %d %d %d in item #%u\n", type, type2, val, current_equip_card_id ? current_equip_card_id : sd->inventory_data[current_equip_item_index]->nameid);
		}
		else {
			ShowWarning("pc_bonus2: unknown bonus type %d %d %d in unknown usage. Report this!\n", type, type2, val);
		}
		break;
	}
}

/**
 * Gives item bonus to player for format: bonus3 bBonusName,type2,type3,val;
 * @param sd
 * @param type Bonus type used by bBonusName
 * @param type2
 * @param type3
 * @param val Value that usually for rate or fixed value
 */
void pc_bonus3(struct map_session_data *sd,int type,int type2,int type3,int val)
{
	nullpo_retv(sd);

	switch(type){
	case SP_ADD_MONSTER_DROP_ITEM: // bonus3 bAddMonsterDropItem,iid,r,n;
		if(sd->state.lr_flag != 2)
			pc_bonus_item_drop(sd->add_drop, type2, 0, CLASS_NONE, type3, val);
		break;
	case SP_ADD_MONSTER_ID_DROP_ITEM: // bonus3 bAddMonsterIdDropItem,iid,mid,n;
		if(sd->state.lr_flag != 2)
			pc_bonus_item_drop(sd->add_drop, type2, 0, CLASS_NONE, -type3, val);
		break;
	case SP_ADD_CLASS_DROP_ITEM: // bonus3 bAddClassDropItem,iid,c,n;
		if(sd->state.lr_flag != 2)
			pc_bonus_item_drop(sd->add_drop, type2, 0, type3, RC_NONE_, val);
		break;
	case SP_AUTOSPELL: // bonus3 bAutoSpell,sk,y,n;
		if(sd->state.lr_flag != 2)
		{
			int target = skill_get_inf(type2); //Support or Self (non-auto-target) skills should pick self.
			target = target&INF_SUPPORT_SKILL || (target&INF_SELF_SKILL && !skill_get_inf2(type2, INF2_NOTARGETSELF));
			pc_bonus_autospell(sd->autospell, type2, type3, val, 0, current_equip_card_id, target ? AUTOSPELL_FORCE_SELF : AUTOSPELL_FORCE_TARGET);
		}
		break;
	case SP_AUTOSPELL_WHENHIT: // bonus3 bAutoSpellWhenHit,sk,y,n;
		if(sd->state.lr_flag != 2)
		{
			int target = skill_get_inf(type2); //Support or Self (non-auto-target) skills should pick self.
			target = target&INF_SUPPORT_SKILL || (target&INF_SELF_SKILL && !skill_get_inf2(type2, INF2_NOTARGETSELF));
			pc_bonus_autospell(sd->autospell2, type2, type3, val, BF_NORMAL|BF_SKILL, current_equip_card_id, target ? AUTOSPELL_FORCE_SELF : AUTOSPELL_FORCE_TARGET);
		}
		break;
	case SP_ADD_MONSTER_DROP_ITEMGROUP: // bonus3 bAddMonsterDropItemGroup,ig,r,n;
		if (sd->state.lr_flag != 2)
			pc_bonus_item_drop(sd->add_drop, 0, type2, CLASS_NONE, type3, val);
		break;
	case SP_ADD_CLASS_DROP_ITEMGROUP: // bonus3 bAddClassDropItemGroup,ig,c,n;
		if (sd->state.lr_flag != 2)
			pc_bonus_item_drop(sd->add_drop, 0, type2, type3, RC_NONE_, val);
		break;

	case SP_ADDEFF: // bonus3 bAddEff,eff,n,y;
		PC_BONUS_CHK_SC(type2,SP_ADDEFF);
		pc_bonus_addeff(sd->addeff, (sc_type)type2, sd->state.lr_flag != 2 ? type3 : 0, sd->state.lr_flag == 2 ? type3 : 0, val, 0);
		break;

	case SP_ADDEFF_WHENHIT: // bonus3 bAddEffWhenHit,eff,n,y;
		PC_BONUS_CHK_SC(type2,SP_ADDEFF_WHENHIT);
		if(sd->state.lr_flag != 2)
			pc_bonus_addeff(sd->addeff_atked, (sc_type)type2, type3, 0, val, 0);
		break;

	case SP_ADDEFF_ONSKILL: // bonus3 bAddEffOnSkill,sk,eff,n;
		PC_BONUS_CHK_SC(type2,SP_ADDEFF_ONSKILL);
		if( sd->state.lr_flag != 2 )
			pc_bonus_addeff_onskill(sd->addeff_onskill, (sc_type)type3, val, type2, ATF_TARGET, 0);
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

	case SP_SUBRACE: // bonus3 bSubRace,r,x,bf;
		PC_BONUS_CHK_RACE(type2, SP_SUBRACE);
		if (sd->state.lr_flag != 2)
			pc_bonus_subrace(sd, (unsigned char)type2, type3, val);
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

	case SP_SP_VANISH_RATE: // bonus3 bSPVanishRate,x,n,bf;
		if(sd->state.lr_flag != 2) {
			pc_bonus_addvanish(sd->sp_vanish, type2, type3, val);
		}
		break;

	case SP_HP_VANISH_RATE: // bonus3 bHPVanishRate,x,n,bf;
		if(sd->state.lr_flag != 2) {
			pc_bonus_addvanish(sd->hp_vanish, type2, type3, val);
		}
		break;

	case SP_STATE_NORECOVER_RACE: // bonus3 bStateNoRecoverRace,r,x,t;
		PC_BONUS_CHK_RACE(type2, SP_STATE_NORECOVER_RACE);
		if (sd->state.lr_flag == 2)
			break;
		//! CONFIRM: Is it not stackable? Does not check max or min value?
		//if (type3 > sd->norecover_state_race[type2].rate) {
		//	sd->norecover_state_race[type2].rate = type3;
		//	sd->norecover_state_race[type2].tick = val;
		//	break;
		//}
		sd->norecover_state_race[type2].rate = type3;
		sd->norecover_state_race[type2].tick = val;
		break;
	default:
		if (current_equip_combo_pos > 0) {
			ShowWarning("pc_bonus3: unknown bonus type %d %d %d %d in a combo with item #%u\n", type, type2, type3, val, sd->inventory_data[pc_checkequip( sd, current_equip_combo_pos )]->nameid);
		}
		else if (current_equip_card_id > 0 || current_equip_item_index > 0) {
			ShowWarning("pc_bonus3: unknown bonus type %d %d %d %d in item #%u\n", type, type2, type3, val, current_equip_card_id ? current_equip_card_id : sd->inventory_data[current_equip_item_index]->nameid);
		}
		else {
			ShowWarning("pc_bonus3: unknown bonus type %d %d %d %d in unknown usage. Report this!\n", type, type2, type3, val);
		}
		break;
	}
}

/**
 * Gives item bonus to player for format: bonus4 bBonusName,type2,type3,type4,val;
 * @param sd
 * @param type Bonus type used by bBonusName
 * @param type2
 * @param type3
 * @param type4
 * @param val Value that usually for rate or fixed value
 */
void pc_bonus4(struct map_session_data *sd,int type,int type2,int type3,int type4,int val)
{
	nullpo_retv(sd);

	switch(type){
	case SP_AUTOSPELL: // bonus4 bAutoSpell,sk,y,n,i;
		if(sd->state.lr_flag != 2)
			pc_bonus_autospell(sd->autospell, type2, type3, type4, 0, current_equip_card_id, val & AUTOSPELL_FORCE_ALL);
		break;

	case SP_AUTOSPELL_WHENHIT: // bonus4 bAutoSpellWhenHit,sk,y,n,i;
		if(sd->state.lr_flag != 2)
			pc_bonus_autospell(sd->autospell2, type2, type3, type4, BF_NORMAL|BF_SKILL, current_equip_card_id, val & AUTOSPELL_FORCE_ALL);
		break;

	case SP_AUTOSPELL_ONSKILL: // bonus4 bAutoSpellOnSkill,sk,x,y,n;
		if(sd->state.lr_flag != 2)
		{
			int target = skill_get_inf(type3); //Support or Self (non-auto-target) skills should pick self.
			target = target&INF_SUPPORT_SKILL || (target&INF_SELF_SKILL && !skill_get_inf2(type3, INF2_NOTARGETSELF));

			pc_bonus_autospell_onskill(sd->autospell3, type2, type3, type4, val, current_equip_card_id, target ? AUTOSPELL_FORCE_TARGET : AUTOSPELL_FORCE_SELF);
		}
		break;

	case SP_ADDEFF: // bonus4 bAddEff,eff,n,y,t;
		PC_BONUS_CHK_SC(type2,SP_ADDEFF);
		pc_bonus_addeff(sd->addeff, (sc_type)type2, sd->state.lr_flag != 2 ? type3 : 0, sd->state.lr_flag == 2 ? type3 : 0, type4, val);
		break;

	case SP_ADDEFF_WHENHIT: // bonus4 bAddEffWhenHit,eff,n,y,t;
		PC_BONUS_CHK_SC(type2,SP_ADDEFF_WHENHIT);
		if (sd->state.lr_flag != 2)
			pc_bonus_addeff(sd->addeff_atked, (sc_type)type2, type3, 0, type4, val);
		break;

	case SP_ADDEFF_ONSKILL: // bonus4 bAddEffOnSkill,sk,eff,n,y;
		PC_BONUS_CHK_SC(type3,SP_ADDEFF_ONSKILL);
		if( sd->state.lr_flag != 2 )
			pc_bonus_addeff_onskill(sd->addeff_onskill, (sc_type)type3, type4, type2, val, 0);
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
		if (current_equip_combo_pos > 0) {
			ShowWarning("pc_bonus4: unknown bonus type %d %d %d %d %d in a combo with item #%u\n", type, type2, type3, type4, val, sd->inventory_data[pc_checkequip( sd, current_equip_combo_pos )]->nameid);
		}
		else if (current_equip_card_id > 0 || current_equip_item_index > 0) {
			ShowWarning("pc_bonus4: unknown bonus type %d %d %d %d %d in item #%u\n", type, type2, type3, type4, val, current_equip_card_id ? current_equip_card_id : sd->inventory_data[current_equip_item_index]->nameid);
		}
		else {
			ShowWarning("pc_bonus4: unknown bonus type %d %d %d %d %d in unknown usage. Report this!\n", type, type2, type3, type4, val);
		}
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
			pc_bonus_autospell(sd->autospell, type2, type3, type4, type5, current_equip_card_id, val & AUTOSPELL_FORCE_ALL);
		break;

	case SP_AUTOSPELL_WHENHIT: // bonus5 bAutoSpellWhenHit,sk,y,n,bf,i;
		if(sd->state.lr_flag != 2)
			pc_bonus_autospell(sd->autospell2, type2, type3, type4, type5, current_equip_card_id, val & AUTOSPELL_FORCE_ALL);
		break;

	case SP_AUTOSPELL_ONSKILL: // bonus5 bAutoSpellOnSkill,sk,x,y,n,i;
		if(sd->state.lr_flag != 2)
			pc_bonus_autospell_onskill(sd->autospell3, type2, type3, type4, type5, current_equip_card_id, val & AUTOSPELL_FORCE_ALL);
		break;
 
	case SP_ADDEFF_ONSKILL: // bonus5 bAddEffOnSkill,sk,eff,n,y,t;
		PC_BONUS_CHK_SC(type3,SP_ADDEFF_ONSKILL);
		if( sd->state.lr_flag != 2 )
			pc_bonus_addeff_onskill(sd->addeff_onskill, (sc_type)type3, type4, type2, type5, val);
		break;

	default:
		if (current_equip_combo_pos > 0) {
			ShowWarning("pc_bonus5: unknown bonus type %d %d %d %d %d %d in a combo with item #%u\n", type, type2, type3, type4, type5, val, sd->inventory_data[pc_checkequip( sd, current_equip_combo_pos )]->nameid);
		}
		else if (current_equip_card_id > 0 || current_equip_item_index > 0) {
			ShowWarning("pc_bonus5: unknown bonus type %d %d %d %d %d %d in item #%u\n", type, type2, type3, type4, type5, val, current_equip_card_id ? current_equip_card_id : sd->inventory_data[current_equip_item_index]->nameid);
		}
		else {
			ShowWarning("pc_bonus5: unknown bonus type %d %d %d %d %d %d in unknown usage. Report this!\n", type, type2, type3, type4, type5, val);
		}
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
bool pc_skill(struct map_session_data* sd, uint16 skill_id, int level, enum e_addskill_type type) {
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
			if (!skill_get_inf(skill_id) || pc_checkskill_summoner(sd, SUMMONER_POWER_LAND) >= 20 || pc_checkskill_summoner(sd, SUMMONER_POWER_SEA) >= 20) //Only recalculate for passive skills.
				status_calc_pc(sd, SCO_NONE);
			break;

		case ADDSKILL_TEMP: //Item bonus skill.
			if (sd->status.skill[idx].id != 0) {
				if (sd->status.skill[idx].flag == SKILL_FLAG_PERMANENT) //Non-granted skill, store it's level.
					sd->status.skill[idx].flag = SKILL_FLAG_REPLACED_LV_0 + sd->status.skill[idx].lv;
			} else {
				sd->status.skill[idx].id   = skill_id;
				sd->status.skill[idx].flag = SKILL_FLAG_TEMPORARY;
			}
			sd->status.skill[idx].lv = max(sd->status.skill[idx].lv, level);
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
			if (!skill_get_inf(skill_id) || pc_checkskill_summoner(sd, SUMMONER_POWER_LAND) >= 20 || pc_checkskill_summoner(sd, SUMMONER_POWER_SEA) >= 20) //Only recalculate for passive skills.
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
	nullpo_ret(sd);

	if (idx_equip < 0 || idx_equip >= MAX_INVENTORY) {
		return 0;
	}
	if (idx_card < 0 || idx_card >= MAX_INVENTORY) {
		return 0;
	}

	int i;
	t_itemid nameid;
	struct item_data* item_eq = sd->inventory_data[idx_equip];
	struct item_data* item_card = sd->inventory_data[idx_card];

	if(item_eq == nullptr)
		return 0; //Invalid item index.
	if(item_card == nullptr)
		return 0; //Invalid card index.
	if( sd->inventory.u.items_inventory[idx_equip].nameid == 0 || sd->inventory.u.items_inventory[idx_equip].amount < 1 )
		return 0; // target item missing
	if( sd->inventory.u.items_inventory[idx_card].nameid == 0 || sd->inventory.u.items_inventory[idx_card].amount < 1 )
		return 0; // target card missing
	if( item_eq->type != IT_WEAPON && item_eq->type != IT_ARMOR )
		return 0; // only weapons and armor are allowed
	if( item_card->type != IT_CARD )
		return 0; // must be a card
	if( sd->inventory.u.items_inventory[idx_equip].identify == 0 )
		return 0; // target must be identified
	if( itemdb_isspecial(sd->inventory.u.items_inventory[idx_equip].card[0]) )
		return 0; // card slots reserved for other purposes
	if( (item_eq->equip & item_card->equip) == 0 )
		return 0; // card cannot be compounded on this item type
	if( item_eq->type == IT_WEAPON && item_card->equip == EQP_SHIELD )
		return 0; // attempted to place shield card on left-hand weapon.
	if( item_eq->type == IT_ARMOR && (item_card->equip & EQP_ACC) && ((item_card->equip & EQP_ACC) != EQP_ACC) && ((item_eq->equip & EQP_ACC) != (item_card->equip & EQP_ACC)) )
		return 0; // specific accessory-card can only be inserted to specific accessory.
	if( sd->inventory.u.items_inventory[idx_equip].equip != 0 )
		return 0; // item must be unequipped

	ARR_FIND( 0, item_eq->slots, i, sd->inventory.u.items_inventory[idx_equip].card[i] == 0 );
	if( i == item_eq->slots )
		return 0; // no free slots

	// remember the card id to insert
	nameid = sd->inventory.u.items_inventory[idx_card].nameid;

	if( pc_delitem(sd,idx_card,1,1,0,LOG_TYPE_OTHER) == 1 )
	{// failed
		clif_insert_card(sd,idx_equip,idx_card,1);
	}
	else
	{// success
		log_pick_pc(sd, LOG_TYPE_OTHER, -1, &sd->inventory.u.items_inventory[idx_equip]);
		sd->inventory.u.items_inventory[idx_equip].card[i] = nameid;
		log_pick_pc(sd, LOG_TYPE_OTHER,  1, &sd->inventory.u.items_inventory[idx_equip]);
		clif_insert_card(sd,idx_equip,idx_card,0);
	}

	return 0;
}

/**
 * Returns the count of unidentified items with the option to identify too.
 * @param sd: Player data
 * @param identify_item: Whether or not to identify any unidentified items
 * @return Unidentified items count
 */
int pc_identifyall(struct map_session_data *sd, bool identify_item)
{
	int unidentified_count = 0;

	for (int i = 0; i < MAX_INVENTORY; i++) {
		if (sd->inventory.u.items_inventory[i].nameid > 0 && sd->inventory.u.items_inventory[i].identify != 1) {
			if (identify_item == true) {
				sd->inventory.u.items_inventory[i].identify = 1;
				clif_item_identified(sd,i,0);
			}
			unidentified_count++;
		}
	}

	return unidentified_count;
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
	if(val < battle_config.min_shop_buy)
		val = battle_config.min_shop_buy;

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
	if (val < battle_config.min_shop_sell)
		val = battle_config.min_shop_sell;

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
char pc_checkadditem(struct map_session_data *sd, t_itemid nameid, int amount)
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

	if (data->flag.guid)
		return CHKADDITEM_NEW;

	for(i=0;i<MAX_INVENTORY;i++){
		// FIXME: This does not consider the checked item's cards, thus could check a wrong slot for stackability.
		if(sd->inventory.u.items_inventory[i].nameid == nameid){
			if( amount > MAX_AMOUNT - sd->inventory.u.items_inventory[i].amount || ( data->stack.inventory && amount > data->stack.amount - sd->inventory.u.items_inventory[i].amount ) )
				return CHKADDITEM_OVERAMOUNT;
			// If the item is in the inventory already, but the player is not allowed to use that many slots anymore
			if( i >= sd->status.inventory_slots ){
				return CHKADDITEM_OVERAMOUNT;
			}
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
	uint16 i;
	uint8 b;

	nullpo_ret(sd);

	for(i = 0, b = 0; i < sd->status.inventory_slots; i++){
		if(sd->inventory.u.items_inventory[i].nameid == 0)
			b++;
	}

	return b;
}

/**
 * Attempts to remove zeny from player
 * @param sd: Player
 * @param zeny: Zeny removed
 * @param type: Log type
 * @param tsd: (optional) From who to log (if null take sd)
 * @return 0: Success, 1: Failed (Removing negative Zeny or not enough Zeny), 2: Player not found
 */
char pc_payzeny(struct map_session_data *sd, int zeny, enum e_log_pick_type type, struct map_session_data *tsd)
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
		clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], output, false, SELF);
	}

	return 0;
}

/**
 * Attempts to give zeny to player
 * @param sd: Player
 * @param type: Log type
 * @param tsd: (optional) From who to log (if null take sd)
 * @return -1: Player not found, 0: Success, 1: Giving negative Zeny
 */
char pc_getzeny(struct map_session_data *sd, int zeny, enum e_log_pick_type type, struct map_session_data *tsd)
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
		clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], output, false, SELF);
	}

	achievement_update_objective(sd, AG_GET_ZENY, 1, sd->status.zeny);

	return 0;
}

/**
 * Attempts to remove Cash Points from player
 * @param sd: Player
 * @param price: Total points (cash + kafra) the player has to pay
 * @param points: Kafra points the player has to pay
 * @param type: Log type
 * @return -1: Not enough points, otherwise success (cash+points)
 */
int pc_paycash(struct map_session_data *sd, int price, int points, e_log_pick_type type)
{
	int cash;
	nullpo_retr(-1,sd);

	points = cap_value(points, 0, MAX_ZENY); //prevent command UB

	cash = price-points;

	if( sd->cashPoints < cash || sd->kafraPoints < points )
	{
		ShowError("pc_paycash: Not enough points (cash=%d, kafra=%d) to cover the price (cash=%d, kafra=%d) (account_id=%d, char_id=%d).\n", sd->cashPoints, sd->kafraPoints, cash, points, sd->status.account_id, sd->status.char_id);
		return -1;
	}

	if( cash ){
		pc_setaccountreg(sd, add_str(CASHPOINT_VAR), sd->cashPoints - cash);
		sd->cashPoints -= cash;
		log_cash( sd, type, LOG_CASH_TYPE_CASH, -cash );
	}

	if( points ){
		pc_setaccountreg(sd, add_str(KAFRAPOINT_VAR), sd->kafraPoints - points);
		sd->kafraPoints -= points;
		log_cash( sd, type, LOG_CASH_TYPE_KAFRA, -points );
	}

	if( battle_config.cashshop_show_points )
	{
		char output[CHAT_SIZE_MAX];

		sprintf(output, msg_txt(sd,504), points, cash, sd->kafraPoints, sd->cashPoints); // Used %d kafra points and %d cash points. %d kafra and %d cash points remaining.
		clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], output, false, SELF);
	}
	return cash+points;
}

/**
 * Attempts to give Cash Points to player
 * @param sd: Player
 * @param cash: Cash points the player gets
 * @param points: Kafra points the player gets
 * @param type: Log type
 * @return -1: Error, otherwise success (cash or points)
 */
int pc_getcash(struct map_session_data *sd, int cash, int points, e_log_pick_type type)
{
	char output[CHAT_SIZE_MAX];

	nullpo_retr(-1,sd);

	cash = cap_value(cash, 0, MAX_ZENY); //prevent command UB
	points = cap_value(points, 0, MAX_ZENY); //prevent command UB
	if( cash > 0 )
	{
		if( cash > MAX_ZENY-sd->cashPoints )
		{
			ShowWarning("pc_getcash: Cash point overflow (cash=%d, have cash=%d, account_id=%d, char_id=%d).\n", cash, sd->cashPoints, sd->status.account_id, sd->status.char_id);
			cash = MAX_ZENY-sd->cashPoints;
		}

		pc_setaccountreg(sd, add_str(CASHPOINT_VAR), sd->cashPoints+cash);
		sd->cashPoints += cash;
		log_cash( sd, type, LOG_CASH_TYPE_CASH, cash );

		if( battle_config.cashshop_show_points )
		{
			sprintf(output, msg_txt(sd,505), cash, sd->cashPoints);
			clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], output, false, SELF);
		}
		return cash;
	}

	if( points > 0 )
	{
		if( points > MAX_ZENY-sd->kafraPoints )
		{
			ShowWarning("pc_getcash: Kafra point overflow (points=%d, have points=%d, account_id=%d, char_id=%d).\n", points, sd->kafraPoints, sd->status.account_id, sd->status.char_id);
			points = MAX_ZENY-sd->kafraPoints;
		}

		pc_setaccountreg(sd, add_str(KAFRAPOINT_VAR), sd->kafraPoints+points);
		sd->kafraPoints += points;
		log_cash( sd, type, LOG_CASH_TYPE_KAFRA, points );

		if( battle_config.cashshop_show_points )
		{
			sprintf(output, msg_txt(sd,506), points, sd->kafraPoints);
			clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], output, false, SELF);
		}
		return points;
	}

	return -1; //shouldn't happen but just in case
}

/**
 * Searching a specified itemid in inventory and return his stored index
 * @param sd Player
 * @param nameid Find this Item!
 * @return Stored index in inventory, or -1 if not found.
 **/
short pc_search_inventory(struct map_session_data *sd, t_itemid nameid) {
	short i;
	nullpo_retr(-1, sd);

	ARR_FIND( 0, MAX_INVENTORY, i, sd->inventory.u.items_inventory[i].nameid == nameid && (sd->inventory.u.items_inventory[i].amount > 0 || nameid == 0) );
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
enum e_additem_result pc_additem(struct map_session_data *sd,struct item *item,int amount,e_log_pick_type log_type) {
	struct item_data *id;
	int16 i;
	unsigned int w;

	nullpo_retr(ADDITEM_INVALID, sd);
	nullpo_retr(ADDITEM_INVALID, item);

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

	if (id->flag.guid && !item->unique_id)
		item->unique_id = pc_generate_unique_id(sd);

	// Stackable | Non Rental
	if( itemdb_isstackable2(id) && item->expire_time == 0 ) {
		for( i = 0; i < MAX_INVENTORY; i++ ) {
			if( sd->inventory.u.items_inventory[i].nameid == item->nameid &&
				sd->inventory.u.items_inventory[i].bound == item->bound &&
				sd->inventory.u.items_inventory[i].expire_time == 0 &&
				sd->inventory.u.items_inventory[i].unique_id == item->unique_id &&
				memcmp(&sd->inventory.u.items_inventory[i].card, &item->card, sizeof(item->card)) == 0 ) {
				if( amount > MAX_AMOUNT - sd->inventory.u.items_inventory[i].amount || ( id->stack.inventory && amount > id->stack.amount - sd->inventory.u.items_inventory[i].amount ) )
					return ADDITEM_OVERAMOUNT;
				// If the item is in the inventory already, but the player is not allowed to use that many slots anymore
				if( i >= sd->status.inventory_slots ){
					return ADDITEM_OVERAMOUNT;
				}
				sd->inventory.u.items_inventory[i].amount += amount;
				clif_additem(sd,i,amount,0);
				break;
			}
		}
	}else{
		i = MAX_INVENTORY;
 	}

	if (i >= MAX_INVENTORY) {
		i = pc_search_inventory(sd,0);
		if( i < 0 )
			return ADDITEM_OVERITEM;
		if( i >= sd->status.inventory_slots ){
			return ADDITEM_OVERITEM;
		}

		memcpy(&sd->inventory.u.items_inventory[i], item, sizeof(sd->inventory.u.items_inventory[0]));
		// clear equip and favorite fields first, just in case
		if( item->equip )
			sd->inventory.u.items_inventory[i].equip = 0;
		if( item->favorite )
			sd->inventory.u.items_inventory[i].favorite = 0;
		if( item->equipSwitch )
			sd->inventory.u.items_inventory[i].equipSwitch = 0;

		sd->inventory.u.items_inventory[i].amount = amount;
		sd->inventory_data[i] = id;
		sd->last_addeditem_index = i;

		if (!itemdb_isstackable2(id) || id->flag.guid)
			sd->inventory.u.items_inventory[i].unique_id = item->unique_id ? item->unique_id : pc_generate_unique_id(sd);

		clif_additem(sd,i,amount,0);
	}

	log_pick_pc(sd, log_type, amount, &sd->inventory.u.items_inventory[i]);

	sd->weight += w;
	clif_updatestatus(sd,SP_WEIGHT);
	//Auto-equip
	if(id->flag.autoequip)
		pc_equipitem(sd, i, id->equip);

	/* rental item check */
	if( item->expire_time ) {
		if( time(NULL) > item->expire_time ) {
			clif_rental_expired(sd, i, sd->inventory.u.items_inventory[i].nameid);
			pc_delitem(sd, i, sd->inventory.u.items_inventory[i].amount, 1, 0, LOG_TYPE_OTHER);
		} else {
			unsigned int seconds = (unsigned int)( item->expire_time - time(NULL) );
			clif_rental_time(sd, sd->inventory.u.items_inventory[i].nameid, seconds);
			pc_inventory_rental_add(sd, seconds);
		}
	}

	achievement_update_objective(sd, AG_GET_ITEM, 1, id->value_sell);
	pc_show_questinfo(sd);

	return ADDITEM_SUCCESS;
}

/*==========================================
 * Remove an item at index n from inventory by amount.
 * @param sd
 * @param n Item index in inventory
 * @param amount
 * @param type &1: Don't notify deletion; &2 Don't notify weight change; &4 Don't calculate status
 * @param reason Delete reason
 * @param log_type e_log_pick_type
 * @return 1 - invalid itemid or negative amount; 0 - Success
 *------------------------------------------*/
char pc_delitem(struct map_session_data *sd,int n,int amount,int type, short reason, e_log_pick_type log_type)
{
	nullpo_retr(1, sd);

	if(n < 0 || sd->inventory.u.items_inventory[n].nameid == 0 || amount <= 0 || sd->inventory.u.items_inventory[n].amount<amount || sd->inventory_data[n] == NULL)
		return 1;

	log_pick_pc(sd, log_type, -amount, &sd->inventory.u.items_inventory[n]);

	sd->inventory.u.items_inventory[n].amount -= amount;
	sd->weight -= sd->inventory_data[n]->weight*amount ;
	if( sd->inventory.u.items_inventory[n].amount <= 0 ){
		if(sd->inventory.u.items_inventory[n].equip)
			pc_unequipitem(sd,n,2|(!(type&4) ? 1 : 0));
		memset(&sd->inventory.u.items_inventory[n],0,sizeof(sd->inventory.u.items_inventory[0]));
		sd->inventory_data[n] = NULL;
	}
	if(!(type&1))
		clif_delitem(sd,n,amount,reason);
	if(!(type&2))
		clif_updatestatus(sd,SP_WEIGHT);

	pc_show_questinfo(sd);

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

	if(sd->inventory.u.items_inventory[n].nameid == 0 ||
		sd->inventory.u.items_inventory[n].amount <= 0 ||
		sd->inventory.u.items_inventory[n].amount < amount ||
		sd->state.trading || sd->state.vending ||
		!sd->inventory_data[n] //pc_delitem would fail on this case.
		)
		return false;

	if( sd->inventory.u.items_inventory[n].equipSwitch )
		return false;

	if( map_getmapflag(sd->bl.m, MF_NODROP) )
	{
		clif_displaymessage (sd->fd, msg_txt(sd,271));
		return false; //Can't drop items in nodrop mapflag maps.
	}

	if( !pc_candrop(sd,&sd->inventory.u.items_inventory[n]) )
	{
		clif_displaymessage (sd->fd, msg_txt(sd,263));
		return false;
	}

	if (!map_addflooritem(&sd->inventory.u.items_inventory[n], amount, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 2, 0))
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
	t_tick tick = gettick();
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
 *	false = no
 *	true = yes
 *------------------------------------------*/
bool pc_isUseitem(struct map_session_data *sd,int n)
{
	struct item_data *item;
	t_itemid nameid;

	nullpo_ret(sd);

	item = sd->inventory_data[n];
	nameid = sd->inventory.u.items_inventory[n].nameid;

	if( item == NULL )
		return false;
	//Not consumable item
	if( item->type != IT_HEALING && item->type != IT_USABLE && item->type != IT_CASH )
		return false;
	if (pc_has_permission(sd,PC_PERM_ITEM_UNCONDITIONAL))
		return true;

	struct map_data *mapdata = map_getmapdata(sd->bl.m);

	if(mapdata->flag[MF_NOITEMCONSUMPTION]) //consumable but mapflag prevent it
		return false;
	//Prevent mass item usage. [Skotlex]
	if( DIFF_TICK(sd->canuseitem_tick,gettick()) > 0 ||
		(itemdb_group.item_exists(IG_CASH_FOOD, nameid) && DIFF_TICK(sd->canusecashfood_tick,gettick()) > 0)
	)
		return false;

	if( (item->item_usage.sitting) && (pc_issit(sd) == 1) && (pc_get_group_level(sd) < item->item_usage.override) ) {
		clif_msg(sd,ITEM_NOUSE_SITTING);
		return false; // You cannot use this item while sitting.
	}

	if (sd->state.storage_flag && item->type != IT_CASH) {
		clif_messagecolor(&sd->bl, color_table[COLOR_RED], msg_txt(sd,388), false, SELF);
		return false; // You cannot use this item while storage is open.
	}

	if (item->flag.dead_branch && (mapdata->flag[MF_NOBRANCH] || mapdata_flag_gvg2(mapdata)))
		return false;

	switch( nameid ) {
		case ITEMID_WING_OF_FLY:
		case ITEMID_GIANT_FLY_WING:
		case ITEMID_N_FLY_WING:
			if( mapdata->flag[MF_NOTELEPORT] || mapdata_flag_gvg2(mapdata) ) {
				clif_skill_teleportmessage(sd,0);
				return false;
			}
			if (nameid == ITEMID_GIANT_FLY_WING) {
				struct party_data *pd = party_search(sd->status.party_id);

				if (pd) {
					int i;

					ARR_FIND(0, MAX_PARTY, i, pd->data[i].sd == sd && pd->party.member[i].leader);
					if (i == MAX_PARTY) { // User is not party leader
						clif_msg(sd, ITEM_PARTY_MEMBER_NOT_SUMMONED);
						break;
					}

					ARR_FIND(0, MAX_PARTY, i, pd->data[i].sd && pd->data[i].sd != sd && pd->data[i].sd->bl.m == sd->bl.m && !pc_isdead(pd->data[i].sd));
					if (i == MAX_PARTY) { // No party members found on same map
						clif_msg(sd, ITEM_PARTY_NO_MEMBER_IN_MAP);
						break;
					}
				} else {
					clif_msg(sd, ITEM_PARTY_MEMBER_NOT_SUMMONED);
					break;
				}
			}
		// Fall through
		case ITEMID_WING_OF_BUTTERFLY:
		case ITEMID_N_BUTTERFLY_WING:
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
			if( mapdata->flag[MF_NORETURN] && nameid != ITEMID_WING_OF_FLY && nameid != ITEMID_GIANT_FLY_WING && nameid != ITEMID_N_FLY_WING )
				return false;
			break;
		case ITEMID_MERCENARY_RED_POTION:
		case ITEMID_MERCENARY_BLUE_POTION:
		case ITEMID_M_CENTER_POTION:
		case ITEMID_M_AWAKENING_POTION:
		case ITEMID_M_BERSERK_POTION:
			if( sd->md == NULL || sd->md->db == NULL )
				return false;
			if( sd->md->sc.cant.consume )
				return false;
			if( nameid == ITEMID_M_AWAKENING_POTION && sd->md->db->lv < 40 )
				return false;
			if( nameid == ITEMID_M_BERSERK_POTION && sd->md->db->lv < 80 )
				return false;
			break;

		case ITEMID_NEURALIZER:
			if( !mapdata->flag[MF_RESET] )
				return false;
			break;
	}

	if( itemdb_group.item_exists(IG_MERCENARY, nameid) && sd->md != NULL )
		return false; // Mercenary Scrolls

	if( item->flag.group || item->type == IT_CASH) {	//safe check type cash disappear when overweight [Napster]
		if( pc_is90overweight(sd) ) {
			clif_msg(sd, ITEM_CANT_OBTAIN_WEIGHT);
			return false;
		}
		if( !pc_inventoryblank(sd) ) {
			clif_messagecolor(&sd->bl, color_table[COLOR_RED], msg_txt(sd, 732), false, SELF); //Item cannot be open when inventory is full
			return false;
		}
	}

	//Gender check
	if(item->sex != SEX_BOTH && sd->status.sex != item->sex)
		return false;
	//Required level check
	if(item->elv && sd->status.base_level < (unsigned int)item->elv)
		return false;
	if(item->elvmax && sd->status.base_level > (unsigned int)item->elvmax)
		return false;

	//Not equipable by class. [Skotlex]
	if (!pc_job_can_use_item(sd,item))
		return false;
	
	if (sd->sc.cant.consume)
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
	t_tick tick = gettick();
	int amount;
	t_itemid nameid;
	struct script_code *script;
	struct item item;
	struct item_data *id;

	nullpo_ret(sd);

	if (sd->state.mail_writing)
		return 0;

	if (sd->npc_id) {
		if (sd->progressbar.npc_id) {
			clif_progressbar_abort(sd);
			return 0; // First item use attempt cancels the progress bar
		}

		if( pc_hasprogress( sd, WIP_DISABLE_SKILLITEM ) || !sd->npc_item_flag ){
#ifdef RENEWAL
			clif_msg( sd, WORK_IN_PROGRESS );
#endif
			return 0;
		}
	}
	item = sd->inventory.u.items_inventory[n];
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
	if( id->flag.delay_consume > 0 ) {
		if( nameid != ITEMID_REINS_OF_MOUNT && sd->sc.data[SC_ALL_RIDING] )
			return 0;
		else if( pc_issit(sd) )
			return 0;
	}
	//Since most delay-consume items involve using a "skill-type" target cursor,
	//perform a skill-use check before going through. [Skotlex]
	//resurrection was picked as testing skill, as a non-offensive, generic skill, it will do.
	//FIXME: Is this really needed here? It'll be checked in unit.cpp after all and this prevents skill items using when silenced [Inkfish]
	if( id->flag.delay_consume > 0 && ( sd->ud.skilltimer != INVALID_TIMER /*|| !status_check_skilluse(&sd->bl, &sd->bl, ALL_RESURRECTION, 0)*/ ) )
		return 0;

	if( id->delay.duration > 0 && !pc_has_permission(sd,PC_PERM_ITEM_UNCONDITIONAL) && pc_itemcd_check(sd, id, tick, n))
		return 0;

	/* on restricted maps the item is consumed but the effect is not used */
	if (!pc_has_permission(sd,PC_PERM_ITEM_UNCONDITIONAL) && itemdb_isNoEquip(id,sd->bl.m)) {
		clif_msg(sd,ITEM_CANT_USE_AREA); // This item cannot be used within this area
		if( battle_config.allow_consume_restricted_item && id->flag.delay_consume > 0 ) { //need confirmation for delayed consumption items
			clif_useitemack(sd,n,item.amount-1,true);
			pc_delitem(sd,n,1,1,0,LOG_TYPE_CONSUME);
		}
		return 0;/* regardless, effect is not run */
	}

	if (pet_db_search(id->nameid, PET_CATCH) != nullptr && map_getmapflag(sd->bl.m, MF_NOPETCAPTURE)) {
		clif_displaymessage(sd->fd, msg_txt(sd, 669)); // You can't catch any pet on this map.
		return 0;
	}

	sd->itemid = item.nameid;
	sd->itemindex = n;
	if(sd->catch_target_class != PET_CATCH_FAIL) //Abort pet catching.
		sd->catch_target_class = PET_CATCH_FAIL;

	amount = item.amount;
	script = id->script;
	//Check if the item is to be consumed immediately [Skotlex]
	if (id->flag.delay_consume > 0)
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
	if (item.card[0]==CARD0_CREATE && pc_famerank(MakeDWord(item.card[2],item.card[3]), MAPID_ALCHEMIST))
	    potion_flag = 2; // Famous player's potions have 50% more efficiency

	//Update item use time.
	sd->canuseitem_tick = tick + battle_config.item_use_interval;
	if( itemdb_group.item_exists(IG_CASH_FOOD, nameid) )
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
 * @return See pc.hpp::e_additem_result
 */
enum e_additem_result pc_cart_additem(struct map_session_data *sd,struct item *item,int amount,e_log_pick_type log_type)
{
	struct item_data *data;
	int i,w;

	nullpo_retr(ADDITEM_INVALID, sd);
	nullpo_retr(ADDITEM_INVALID, item);

	if(item->nameid == 0 || amount <= 0)
		return ADDITEM_INVALID;

	if (itemdb_ishatched_egg(item))
		return ADDITEM_INVALID;

	data = itemdb_search(item->nameid);

	if( data->stack.cart && amount > data->stack.amount )
	{// item stack limitation
		return ADDITEM_STACKLIMIT;
	}

	if( !itemdb_cancartstore(item, pc_get_group_level(sd)) || (item->bound > BOUND_ACCOUNT && !pc_can_give_bounded_items(sd)))
	{ // Check item trade restrictions	[Skotlex]
		clif_displaymessage (sd->fd, msg_txt(sd,264));
		return ADDITEM_INVALID;
	}

	if( (w = data->weight*amount) + sd->cart_weight > sd->cart_weight_max )
		return ADDITEM_OVERWEIGHT;

	i = MAX_CART;
	if( itemdb_isstackable2(data) && !item->expire_time )
	{
		for (i = 0; i < MAX_CART; i++) {
			if (sd->cart.u.items_cart[i].nameid == item->nameid
				&& sd->cart.u.items_cart[i].bound == item->bound
				&& sd->cart.u.items_cart[i].unique_id == item->unique_id
				&& memcmp(sd->cart.u.items_cart[i].card, item->card, sizeof(item->card)) == 0
				)
				break;
		}
	}

	if( i < MAX_CART )
	{// item already in cart, stack it
		if( amount > MAX_AMOUNT - sd->cart.u.items_cart[i].amount || ( data->stack.cart && amount > data->stack.amount - sd->cart.u.items_cart[i].amount ) )
			return ADDITEM_OVERAMOUNT; // no slot

		sd->cart.u.items_cart[i].amount += amount;
		clif_cart_additem(sd,i,amount);
	}
	else
	{// item not stackable or not present, add it
		ARR_FIND( 0, MAX_CART, i, sd->cart.u.items_cart[i].nameid == 0 );
		if( i == MAX_CART )
			return ADDITEM_OVERAMOUNT; // no slot

		memcpy(&sd->cart.u.items_cart[i],item,sizeof(sd->cart.u.items_cart[0]));
		sd->cart.u.items_cart[i].id = 0;
		sd->cart.u.items_cart[i].amount = amount;
		sd->cart_num++;
		clif_cart_additem(sd,i,amount);
	}
	sd->cart.u.items_cart[i].favorite = 0; // clear
	sd->cart.u.items_cart[i].equipSwitch = 0;
	log_pick_pc(sd, log_type, amount, &sd->cart.u.items_cart[i]);

	sd->cart_weight += w;
	clif_updatestatus(sd,SP_CARTINFO);

	return ADDITEM_SUCCESS;
}

/*==========================================
 * Delete item on cart for given index.
 *------------------------------------------*/
void pc_cart_delitem(struct map_session_data *sd,int n,int amount,int type,e_log_pick_type log_type)
{
	nullpo_retv(sd);

	if(sd->cart.u.items_cart[n].nameid == 0 ||
		sd->cart.u.items_cart[n].amount < amount)
		return;

	log_pick_pc(sd, log_type, -amount, &sd->cart.u.items_cart[n]);

	sd->cart.u.items_cart[n].amount -= amount;
	sd->cart_weight -= itemdb_weight(sd->cart.u.items_cart[n].nameid) * amount;
	if(sd->cart.u.items_cart[n].amount <= 0) {
		memset(&sd->cart.u.items_cart[n],0,sizeof(sd->cart.u.items_cart[0]));
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
	nullpo_retv(sd);

	if (idx < 0 || idx >= MAX_INVENTORY) //Invalid index check [Skotlex]
		return;

	struct item *item_data = &sd->inventory.u.items_inventory[idx];

	if( item_data->nameid == 0 || amount < 1 || item_data->amount < amount || sd->state.vending || sd->state.prevend )
		return;

	if( item_data->equipSwitch ){
		clif_msg( sd, C_ITEM_EQUIP_SWITCH );
		return;
	}

	enum e_additem_result flag = pc_cart_additem(sd,item_data,amount,LOG_TYPE_NONE);

	if (flag == ADDITEM_SUCCESS)
		pc_delitem(sd,idx,amount,0,5,LOG_TYPE_NONE);
	else {
		clif_cart_additem_ack(sd, (flag == ADDITEM_OVERAMOUNT) ? ADDITEM_TO_CART_FAIL_COUNT : ADDITEM_TO_CART_FAIL_WEIGHT);
		clif_additem(sd, idx, amount, 0);
		clif_delitem(sd, idx, amount, 0);
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

	item_data = &sd->cart.u.items_cart[idx];
	if( item_data->nameid == 0 || item_data->amount == 0 )
		return -1;

	return item_data->amount - amount;
}

/*==========================================
 * Retrieve an item at index idx from cart.
 *------------------------------------------*/
void pc_getitemfromcart(struct map_session_data *sd,int idx,int amount)
{
	nullpo_retv(sd);

	if (idx < 0 || idx >= MAX_CART) //Invalid index check [Skotlex]
		return;

	struct item *item_data=&sd->cart.u.items_cart[idx];

	if (item_data->nameid == 0 || amount < 1 || item_data->amount < amount || sd->state.vending || sd->state.prevend)
		return;

	enum e_additem_result flag = pc_additem(sd, item_data, amount, LOG_TYPE_NONE);

	if (flag == ADDITEM_SUCCESS)
		pc_cart_delitem(sd, idx, amount, 0, LOG_TYPE_NONE);
	else {
		clif_cart_delitem(sd, idx, amount);
		clif_additem(sd, idx, amount, flag);
		clif_cart_additem(sd, idx, amount);
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
	int i = 0, j = 0;
	for(i = 0; i < MAX_INVENTORY; i++) {
		if(sd->inventory.u.items_inventory[i].nameid > 0 && sd->inventory.u.items_inventory[i].amount > 0 && sd->inventory.u.items_inventory[i].bound == type) {
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
	t_itemid itemid;
	char output[100];

	sd=va_arg(ap,struct map_session_data *);
	itemid=va_arg(ap,int);

	std::shared_ptr<item_data> id = item_db.find(itemid);

	if(id == nullptr)
		sprintf(output,"%s stole an Unknown Item (id: %u).",sd->status.name, itemid);
	else
		sprintf(output,"%s stole %s.",sd->status.name,id->ename.c_str());
	clif_displaymessage( ((struct map_session_data *)bl)->fd, output);

	return 0;
}

/**
 * Steal an item from bl (mob).
 * @param sd: Player data
 * @param bl: Object to steal from
 * @param skill_lv: Level of skill used
 * @return True on success or false otherwise
 */
bool pc_steal_item(struct map_session_data *sd,struct block_list *bl, uint16 skill_lv)
{
	int i;
	t_itemid itemid;
	double rate;
	unsigned char flag = 0;
	struct status_data *sd_status, *md_status;
	struct mob_data *md;

	if(!sd || !bl || bl->type!=BL_MOB)
		return false;

	md = (TBL_MOB *)bl;

	if(md->state.steal_flag == UCHAR_MAX || ( md->sc.opt1 && md->sc.opt1 != OPT1_BURNING ) ) //already stolen from / status change check
		return false;

	sd_status= status_get_status_data(&sd->bl);
	md_status= status_get_status_data(bl);

	if (md->master_id || status_has_mode(md_status, MD_STATUSIMMUNE) || util::vector_exists(status_get_race2(&md->bl), RC2_TREASURE) ||
		map_getmapflag(bl->m, MF_NOMOBLOOT) || // check noloot map flag [Lorky]
		(battle_config.skill_steal_max_tries && //Reached limit of steal attempts. [Lupus]
			md->state.steal_flag++ >= battle_config.skill_steal_max_tries)
  	) { //Can't steal from
		md->state.steal_flag = UCHAR_MAX;
		return false;
	}

	// base skill success chance (percentual)
	rate = (sd_status->dex - md_status->dex)/2 + skill_lv*6 + 4;
	rate += sd->bonus.add_steal_rate;

	if( rate < 1
#ifdef RENEWAL
		|| rnd()%100 >= rate
#endif
	)
		return false;

	// Try dropping one item, in the order from first to last possible slot.
	// Droprate is affected by the skill success rate.
	for( i = 0; i < MAX_MOB_DROP; i++ )
		if( md->db->dropitem[i].nameid > 0 && !md->db->dropitem[i].steal_protected && itemdb_exists(md->db->dropitem[i].nameid) && rnd() % 10000 < md->db->dropitem[i].rate
#ifndef RENEWAL
		* rate/100.
#endif
		)
			break;
	if( i == MAX_MOB_DROP )
		return false;

	itemid = md->db->dropitem[i].nameid;
	struct item tmp_item = {};
	tmp_item.nameid = itemid;
	tmp_item.amount = 1;
	tmp_item.identify = itemdb_isidentified(itemid);
	if( battle_config.skill_steal_random_options ){
		mob_setdropitem_option( &tmp_item, &md->db->dropitem[i] );
	}
	flag = pc_additem(sd,&tmp_item,1,LOG_TYPE_PICKDROP_PLAYER);

	//TODO: Should we disable stealing when the item you stole couldn't be added to your inventory? Perhaps players will figure out a way to exploit this behaviour otherwise?
	md->state.steal_flag = UCHAR_MAX; //you can't steal from this mob any more

	if(flag) { //Failed to steal due to overweight
		clif_additem(sd,0,0,flag);
		return false;
	}

	if(battle_config.show_steal_in_same_party)
		party_foreachsamemap(pc_show_steal,sd,AREA_SIZE,sd,tmp_item.nameid);

	//Logs items, Stolen from mobs [Lupus]
	log_pick_mob(md, LOG_TYPE_STEAL, -1, &tmp_item);

	//A Rare Steal Global Announce by Lupus
	if(md->db->dropitem[i].rate <= battle_config.rare_drop_announce) {
		struct item_data *i_data;
		char message[128];
		i_data = itemdb_search(itemid);
		sprintf (message, msg_txt(sd,542), (sd->status.name[0])?sd->status.name :"GM", md->db->jname.c_str(), i_data->ename.c_str(), (float)md->db->dropitem[i].rate / 100);
		//MSG: "'%s' stole %s's %s (chance: %0.02f%%)"
		intif_broadcast(message, strlen(message) + 1, BC_DEFAULT);
	}
	return true;
}

/*==========================================
 * Stole zeny from bl (mob)
 * return
 *	0 = fail
 *	1 = success
 *------------------------------------------*/
int pc_steal_coin(struct map_session_data *sd,struct block_list *target)
{
	int rate, target_lv;
	struct mob_data *md;

	if(!sd || !target || target->type != BL_MOB)
		return 0;

	md = (TBL_MOB*)target;
	target_lv = status_get_lv(target);

	if (md->state.steal_coin_flag || md->sc.data[SC_STONE] || md->sc.data[SC_FREEZE] || md->sc.data[SC_HANDICAPSTATE_FROSTBITE] || 
		md->sc.data[SC_HANDICAPSTATE_SWOONING] || md->sc.data[SC_HANDICAPSTATE_LIGHTNINGSTRIKE] || md->sc.data[SC_HANDICAPSTATE_CRYSTALLIZATION] || 
		status_bl_has_mode(target,MD_STATUSIMMUNE) || util::vector_exists(status_get_race2(&md->bl), RC2_TREASURE))
		return 0;

	rate = sd->battle_status.dex / 2 + 2 * (sd->status.base_level - target_lv) + (10 * pc_checkskill(sd, RG_STEALCOIN)) + sd->battle_status.luk / 2;
	if(rnd()%1000 < rate)
	{
		// Zeny Steal Amount: (rnd() % (10 * target_lv + 1 - 8 * target_lv)) + 8 * target_lv
		int amount = (rnd() % (2 * target_lv + 1)) + 8 * target_lv; // Reduced formula

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
 * @return	SETPOS_OK			Success
 *			SETPOS_MAPINDEX		Invalid map index
 *			SETPOS_NO_MAPSERVER	Map not in this map-server, and failed to locate alternate map-server.
 *			SETPOS_AUTOTRADE	Player is in autotrade state
 *------------------------------------------*/
enum e_setpos pc_setpos(struct map_session_data* sd, unsigned short mapindex, int x, int y, clr_type clrtype)
{
	nullpo_retr(SETPOS_OK,sd);

	if( !mapindex || !mapindex_id2name(mapindex) ) {
		ShowDebug("pc_setpos: Passed mapindex(%d) is invalid!\n", mapindex);
		return SETPOS_MAPINDEX;
	}

	if ( sd->state.autotrade && (sd->vender_id || sd->buyer_id) ) // Player with autotrade just causes clif glitch! @ FIXME
		return SETPOS_AUTOTRADE;

	if( battle_config.revive_onwarp && pc_isdead(sd) ) { //Revive dead people before warping them
		pc_setstand(sd, true);
		pc_setrestartvalue(sd,1);
	}

	int16 m = map_mapindex2mapid(mapindex);
	struct map_data *mapdata = map_getmapdata(m);
	status_change *sc = status_get_sc(&sd->bl);

	sd->state.changemap = (sd->mapindex != mapindex);
	sd->state.warping = 1;
	sd->state.workinprogress = WIP_DISABLE_NONE;
	sd->state.mail_writing = false;
	sd->state.refineui_open = false;

	if( sd->state.changemap ) { // Misc map-changing settings
		int curr_map_instance_id = map_getmapdata(sd->bl.m)->instance_id, new_map_instance_id = (mapdata ? mapdata->instance_id : 0);

		if (curr_map_instance_id != new_map_instance_id) {
			if (curr_map_instance_id > 0) { // Update instance timer for the map on leave
				instance_delusers(curr_map_instance_id);
				sd->instance_mode = util::umap_find(instances, curr_map_instance_id)->mode; // Store mode for instance destruction button checks
			}

			if (new_map_instance_id > 0) // Update instance timer for the map on enter
				instance_addusers(new_map_instance_id);
		}

		if (sd->bg_id && mapdata && !mapdata->flag[MF_BATTLEGROUND]) // Moving to a map that isn't a Battlegrounds
			bg_team_leave(sd, false, true);

		sd->state.pmap = sd->bl.m;
		if (sc && sc->count) { // Cancel some map related stuff.
			if (sc->cant.warp)
				return SETPOS_MAPINDEX; // You may not get out!

			for (const auto &it : status_db) {
				if (sc->data[it.first]) {
					if (it.second->flag[SCF_REMOVEONMAPWARP])
						status_change_end(&sd->bl, static_cast<sc_type>(it.first), INVALID_TIMER);

					if (it.second->flag[SCF_RESTARTONMAPWARP] && it.second->skill_id > 0) {
						status_change_entry *sce = sd->sc.data[it.first];

						if (sce->timer != INVALID_TIMER)
							delete_timer(sce->timer, status_change_timer);
						sce->timer = add_timer(gettick() + skill_get_time(it.second->skill_id, sce->val1), status_change_timer, sd->bl.id, it.first);
					}
				}
			}
		}
		for(int i = 0; i < EQI_MAX; i++ ) {
			if( sd->equip_index[i] >= 0 )
				if( pc_isequip(sd,sd->equip_index[i]) )
					pc_unequipitem(sd,sd->equip_index[i],2);
		}
		if (battle_config.clear_unit_onwarp&BL_PC)
			skill_clear_unitgroup(&sd->bl);
		if( battle_config.loose_ap_on_map && mapdata_flag_vs( mapdata ) ){
			status_percent_damage( nullptr, &sd->bl, 0, 0, 100, 0 );
		}
		party_send_dot_remove(sd); //minimap dot fix [Kevin]
		guild_send_dot_remove(sd);
		bg_send_dot_remove(sd);
		if (sd->regen.state.gc)
			sd->regen.state.gc = 0;

		if (mapdata) {
			// make sure vending is allowed here
			if (sd->state.vending && mapdata->flag[MF_NOVENDING]) {
				clif_displaymessage(sd->fd, msg_txt(sd, 276)); // "You can't open a shop on this map"
				vending_closevending(sd);
			}
			// make sure buyingstore is allowed here
			if (sd->state.buyingstore && mapdata->flag[MF_NOBUYINGSTORE]) {
				clif_displaymessage(sd->fd, msg_txt(sd, 276)); // "You can't open a shop on this map"
				buyingstore_close(sd);
			}
		}

		channel_pcquit(sd,4); //quit map chan

		// Remove Cloaked NPC if changing to another map
		for (auto it = sd->cloaked_npc.begin(); it != sd->cloaked_npc.end(); ++it) {
			block_list *npc_bl = map_id2bl(*it);

			if (npc_bl && npc_bl->m != m) {
				sd->cloaked_npc.erase(it);
				break;
			}
		}
	}

	if( m < 0 )
	{
		uint32 ip;
		uint16 port;
		struct script_state *st;

		//if can't find any map-servers, just abort setting position.
		if(!sd->mapindex || map_mapname2ipport(mapindex,&ip,&port))
			return SETPOS_NO_MAPSERVER;

		if (sd->npc_id){
			npc_event_dequeue(sd,false);
			st = sd->st;
		}else{
			st = nullptr;
		}

		if (sd->bg_id) // Switching map servers, remove from bg
			bg_team_leave(sd, false, true);

		if (sd->state.vending) // Stop vending
			vending_closevending(sd);
		if (sd->state.buyingstore) // Stop buyingstore
			buyingstore_close(sd);

		npc_script_event(sd, NPCE_LOGOUT);
		//remove from map, THEN change x/y coordinates
		unit_remove_map_pc(sd,clrtype);
		sd->mapindex = mapindex;
		sd->bl.x=x;
		sd->bl.y=y;
		pc_clean_skilltree(sd);
		chrif_save(sd, CSAVE_CHANGE_MAPSERV|CSAVE_INVENTORY|CSAVE_CART);
		chrif_changemapserver(sd, ip, (short)port);

		//Free session data from this map server [Kevin]
		unit_free_pc(sd);

		if( st ){
			// Has to be done here, because otherwise unit_free_pc will free the stack already
			st->state = END;
		}

		return SETPOS_OK;
	}

	if( x < 0 || x >= mapdata->xs || y < 0 || y >= mapdata->ys )
	{
		ShowError("pc_setpos: attempt to place player '%s' (%d:%d) on invalid coordinates (%s-%d,%d)\n", sd->status.name, sd->status.account_id, sd->status.char_id, mapindex_id2name(mapindex),x,y);
		x = y = 0; // make it random
	}

	if( x == 0 && y == 0 ) { // pick a random walkable cell
		int c=0;
		do {
			x = rnd()%(mapdata->xs-2)+1;
			y = rnd()%(mapdata->ys-2)+1;
			c++;
			
			if(c > (mapdata->xs * mapdata->ys)*3){ //force out
				ShowError("pc_setpos: couldn't found a valid coordinates for player '%s' (%d:%d) on (%s), preventing warp\n", sd->status.name, sd->status.account_id, sd->status.char_id, mapindex_id2name(mapindex));
				return SETPOS_OK; //preventing warp
				//break; //allow warp anyway
			}
		} while(map_getcell(m,x,y,CELL_CHKNOPASS) || (!battle_config.teleport_on_portal && npc_check_areanpc(1,m,x,y,1)));
	}

	if (sd->state.vending && map_getcell(m,x,y,CELL_CHKNOVENDING)) {
		clif_displaymessage (sd->fd, msg_txt(sd,204)); // "You can't open a shop on this cell."
		vending_closevending(sd);
	}
	if (sd->state.buyingstore && map_getcell(m, x, y, CELL_CHKNOBUYINGSTORE)) {
		clif_displaymessage(sd->fd, msg_txt(sd, 204)); // "You can't open a shop on this cell."
		buyingstore_close(sd);
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

	if( sd->status.guild_id > 0 && mapdata->flag[MF_GVG_CASTLE] )
	{	// Increased guild castle regen [Valaris]
		std::shared_ptr<guild_castle> gc = castle_db.mapindex2gc(sd->mapindex);
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

	if( sd->ed ) {
		sd->ed->bl.m = m;
		sd->ed->bl.x = sd->ed->ud.to_x = x;
		sd->ed->bl.y = sd->ed->ud.to_y = y;
		sd->ed->ud.dir = sd->ud.dir;
	}

	pc_cell_basilica(sd);

	//check if we gonna be rewarped [lighta]
	if(npc_check_areanpc(1,m,x,y,0)){
		sd->count_rewarp++;
	}
	else 
		sd->count_rewarp = 0;

	if (sd->state.vending)
		vending_update(*sd);
	if (sd->state.buyingstore)
		buyingstore_update(*sd);
	
	return SETPOS_OK;
}

/*==========================================
 * Warp player sd to random location on current map.
 * May fail if no walkable cell found (1000 attempts).
 * Return:
 *	0 = Success
 *	1,2,3 = Fail
 *------------------------------------------*/
char pc_randomwarp(struct map_session_data *sd, clr_type type, bool ignore_mapflag)
{
	int x,y,i=0;

	nullpo_ret(sd);

	struct map_data *mapdata = map_getmapdata(sd->bl.m);

	if (mapdata->flag[MF_NOTELEPORT] && !ignore_mapflag) //Teleport forbidden
		return 3;

	do {
		x = rnd()%(mapdata->xs-2)+1;
		y = rnd()%(mapdata->ys-2)+1;
	} while((map_getcell(sd->bl.m,x,y,CELL_CHKNOPASS) || (!battle_config.teleport_on_portal && npc_check_areanpc(1,sd->bl.m,x,y,1))) && (i++) < 1000);

	if (i < 1000)
		return pc_setpos(sd,mapdata->index,x,y,type);

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
	if( sd->bl.m >= 0 && (map_getmapflag(sd->bl.m, MF_NOMEMO) || map_getmapflag(sd->bl.m, MF_NOWARPTO)) && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE) ) {
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
		memmove(&sd->status.memo_point[1], &sd->status.memo_point[0], (u8min(i,MAX_MEMOPOINTS-1))*sizeof(struct point));
		pos = 0;
	}

	if( map_getmapdata(sd->bl.m)->instance_id ) {
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
	if (skill_id == SJ_NOVAEXPLOSING) {
		struct status_change *sc = status_get_sc(&sd->bl);

		if (sc && sc->data[SC_DIMENSION])
			return 0;
	}

	int cooldown = skill_get_cooldown(skill_id, skill_lv);

	if (skill_id == SU_TUNABELLY && pc_checkskill(sd, SU_SPIRITOFSEA) > 0)
		cooldown -= skill_get_time(SU_TUNABELLY, skill_lv);

	for (auto &it : sd->skillcooldown) {
		if (it.id == skill_id) {
			cooldown += it.val;
			break;
		}
	}

	return max(0, cooldown);
}

/*==========================================
 * Return player sd skill_lv learned for given skill
 *------------------------------------------*/
uint8 pc_checkskill(struct map_session_data *sd, uint16 skill_id)
{
	uint16 idx = 0;
	if (sd == NULL)
		return 0;

#ifdef RENEWAL
	if ((idx = skill_get_index(skill_id)) == 0) {
#else
	if( ( idx = skill_db.get_index( skill_id, skill_id >= RK_ENCHANTBLADE, __FUNCTION__, __FILE__, __LINE__ ) ) == 0 ){
		if( skill_id >= RK_ENCHANTBLADE ){
			// Silently fail for now -> future update planned
			return 0;
		}
#endif
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
 * Returns the flag of the given skill (when learned).
 * @param sd: Player data
 * @param skill_id: Skill to lookup
 * @return Skill flag type
 */
e_skill_flag pc_checkskill_flag(map_session_data &sd, uint16 skill_id) {
	uint16 idx;

#ifdef RENEWAL
	if ((idx = skill_get_index(skill_id)) == 0) {
#else
	if ((idx = skill_db.get_index(skill_id, skill_id >= RK_ENCHANTBLADE, __FUNCTION__, __FILE__, __LINE__)) == 0) {
		if (skill_id >= RK_ENCHANTBLADE) {
			// Silently fail for now -> future update planned
			return SKILL_FLAG_NONE;
		}
#endif
		ShowError("pc_checkskill_flag: Invalid skill id %d (char_id=%d).\n", skill_id, sd.status.char_id);
		return SKILL_FLAG_NONE;
	}

	return (sd.status.skill[idx].id == skill_id && sd.status.skill[idx].lv > 0) ? static_cast<e_skill_flag>(sd.status.skill[idx].flag) : SKILL_FLAG_NONE;
}

/**
 * Returns the amount of skill points invested in a Summoner's Power of Sea/Land/Life
 * @param sd: Player data
 * @param type: Summoner Power Type
 * @return Skill points invested
 */
uint8 pc_checkskill_summoner(map_session_data *sd, e_summoner_power_type type) {
	if (sd == nullptr)
		return 0;

	uint8 count = 0;

	switch (type) {
		case SUMMONER_POWER_SEA:
			count = pc_checkskill(sd, SU_TUNABELLY) + pc_checkskill(sd, SU_TUNAPARTY) + pc_checkskill(sd, SU_BUNCHOFSHRIMP) + pc_checkskill(sd, SU_FRESHSHRIMP) +
				pc_checkskill(sd, SU_GROOMING) + pc_checkskill(sd, SU_PURRING) + pc_checkskill(sd, SU_SHRIMPARTY);
			break;
		case SUMMONER_POWER_LAND:
			count = pc_checkskill(sd, SU_SV_STEMSPEAR) + pc_checkskill(sd, SU_CN_POWDERING) + pc_checkskill(sd, SU_CN_METEOR) + pc_checkskill(sd, SU_SV_ROOTTWIST) +
				pc_checkskill(sd, SU_CHATTERING) + pc_checkskill(sd, SU_MEOWMEOW) + pc_checkskill(sd, SU_NYANGGRASS);
			break;
		case SUMMONER_POWER_LIFE:
			count = pc_checkskill(sd, SU_SCAROFTAROU) + pc_checkskill(sd, SU_PICKYPECK) + pc_checkskill(sd, SU_ARCLOUSEDASH) + pc_checkskill(sd, SU_LUNATICCARROTBEAT) +
				pc_checkskill(sd, SU_HISS) + pc_checkskill(sd, SU_POWEROFFLOCK) + pc_checkskill(sd, SU_SVG_SPIRIT);
			break;
	}

	return count;
}

/**
 * Checks for Imperial Guard's passive skills.
 * @param sd: Player data
 * @param flag:
 *		Flag&1 = IG_SHIELD_MASTERY
 *		Flag&2 = IG_SPEAR_SWORD_M
 */
uint8 pc_checkskill_imperial_guard(struct map_session_data *sd, short flag)
{
	nullpo_retr(0, sd);

	uint8 count = 0;

	if (flag&1 && sd->status.shield > 0)
		count += pc_checkskill(sd, IG_SHIELD_MASTERY);

	if (flag&2 && (sd->status.weapon == W_1HSWORD || sd->status.weapon == W_1HSPEAR || sd->status.weapon == W_2HSPEAR))
		count += pc_checkskill(sd, IG_SPEAR_SWORD_M);

	return count;
}

/**
 * Check if we still have the correct weapon to continue the skill (actually status)
 * If not ending it
 * @param sd
 * @return 0:error, 1:check done
 */
static void pc_checkallowskill(struct map_session_data *sd)
{
	nullpo_retv(sd);

	if(!sd->sc.count)
		return;

	for (const auto &it : status_db) {
		sc_type status = it.second->type;
		std::bitset<SCF_MAX> flag = it.second->flag;

		if (flag[SCF_REQUIREWEAPON]) { // Skills requiring specific weapon types
			if (status == SC_DANCING && !battle_config.dancing_weaponswitch_fix)
				continue;
			if (sd->sc.data[status] && !pc_check_weapontype(sd, skill_get_weapontype(it.second->skill_id)))
				status_change_end(&sd->bl, status, INVALID_TIMER);
		}

		if (flag[SCF_REQUIRESHIELD]) { // Skills requiring a shield
			if (sd->sc.data[status] && sd->status.shield <= 0)
				status_change_end(&sd->bl, status, INVALID_TIMER);
		}
	}
}

/*==========================================
 * Return equipped index of item on player sd at pos
 * Return
 * -1 : Nothing equipped
 * idx : (this index could be used in inventory to found item_data)
 *------------------------------------------*/
short pc_checkequip(struct map_session_data *sd,int pos, bool checkall)
{
	uint8 i;

	nullpo_retr(-1, sd);

	for(i=0;i<EQI_MAX;i++){
		if(pos & equip_bitmask[i]){
			if( checkall && ( pos&~equip_bitmask[i] ) != 0 && sd->equip_index[i] == -1 ){
				// Check all if any match is found
				continue;
			}

			return sd->equip_index[i];
		}
	}

	return -1;
}

/*==========================================
 * Check if sd has nameid equipped somewhere
 * @sd : the player session
 * @nameid : id of the item to check
 * @min : : see pc.hpp enum equip_index from ? to @max
 * @max : see pc.hpp enum equip_index for @min to ?
 * -return true,false
 *------------------------------------------*/
bool pc_checkequip2(struct map_session_data *sd, t_itemid nameid, int min, int max)
{
	int i;

	for(i = min; i < max; i++) {
		if(equip_bitmask[i]) {
			int idx = sd->equip_index[i];

			if (sd->inventory.u.items_inventory[idx].nameid == nameid)
				return true;
		}
	}
	return false;
}

/*==========================================
 * Convert's from the client's lame Job ID system
 * to the map server's 'makes sense' system. [Skotlex]
 *------------------------------------------*/
uint64 pc_jobid2mapid(unsigned short b_class)
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
		case JOB_SUMMER2:               return MAPID_SUMMER2;
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
		case JOB_BABY_TAEKWON:          return MAPID_BABY_TAEKWON;
		case JOB_BABY_GUNSLINGER:       return MAPID_BABY_GUNSLINGER;
		case JOB_BABY_NINJA:            return MAPID_BABY_NINJA;
		case JOB_BABY_SUMMONER:         return MAPID_BABY_SUMMONER;
	//Baby 2-1 Jobs
		case JOB_SUPER_BABY:            return MAPID_SUPER_BABY;
		case JOB_BABY_KNIGHT:           return MAPID_BABY_KNIGHT;
		case JOB_BABY_WIZARD:           return MAPID_BABY_WIZARD;
		case JOB_BABY_HUNTER:           return MAPID_BABY_HUNTER;
		case JOB_BABY_PRIEST:           return MAPID_BABY_PRIEST;
		case JOB_BABY_BLACKSMITH:       return MAPID_BABY_BLACKSMITH;
		case JOB_BABY_ASSASSIN:         return MAPID_BABY_ASSASSIN;
		case JOB_BABY_STAR_GLADIATOR:   return MAPID_BABY_STAR_GLADIATOR;
		case JOB_BABY_REBELLION:        return MAPID_BABY_REBELLION;
		case JOB_BABY_KAGEROU:
		case JOB_BABY_OBORO:            return MAPID_BABY_KAGEROUOBORO;
	//Baby 2-2 Jobs
		case JOB_BABY_CRUSADER:         return MAPID_BABY_CRUSADER;
		case JOB_BABY_SAGE:             return MAPID_BABY_SAGE;
		case JOB_BABY_BARD:
		case JOB_BABY_DANCER:           return MAPID_BABY_BARDDANCER;
		case JOB_BABY_MONK:             return MAPID_BABY_MONK;
		case JOB_BABY_ALCHEMIST:        return MAPID_BABY_ALCHEMIST;
		case JOB_BABY_ROGUE:            return MAPID_BABY_ROGUE;
		case JOB_BABY_SOUL_LINKER:      return MAPID_BABY_SOUL_LINKER;
	//3-1 Jobs
		case JOB_SUPER_NOVICE_E:        return MAPID_SUPER_NOVICE_E;
		case JOB_RUNE_KNIGHT:           return MAPID_RUNE_KNIGHT;
		case JOB_WARLOCK:               return MAPID_WARLOCK;
		case JOB_RANGER:                return MAPID_RANGER;
		case JOB_ARCH_BISHOP:           return MAPID_ARCH_BISHOP;
		case JOB_MECHANIC:              return MAPID_MECHANIC;
		case JOB_GUILLOTINE_CROSS:      return MAPID_GUILLOTINE_CROSS;
		case JOB_STAR_EMPEROR:          return MAPID_STAR_EMPEROR;
	//3-2 Jobs
		case JOB_ROYAL_GUARD:           return MAPID_ROYAL_GUARD;
		case JOB_SORCERER:              return MAPID_SORCERER;
		case JOB_MINSTREL:
		case JOB_WANDERER:              return MAPID_MINSTRELWANDERER;
		case JOB_SURA:                  return MAPID_SURA;
		case JOB_GENETIC:               return MAPID_GENETIC;
		case JOB_SHADOW_CHASER:         return MAPID_SHADOW_CHASER;
		case JOB_SOUL_REAPER:           return MAPID_SOUL_REAPER;
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
		case JOB_BABY_RUNE_KNIGHT:      return MAPID_BABY_RUNE_KNIGHT;
		case JOB_BABY_WARLOCK:          return MAPID_BABY_WARLOCK;
		case JOB_BABY_RANGER:           return MAPID_BABY_RANGER;
		case JOB_BABY_ARCH_BISHOP:      return MAPID_BABY_ARCH_BISHOP;
		case JOB_BABY_MECHANIC:         return MAPID_BABY_MECHANIC;
		case JOB_BABY_GUILLOTINE_CROSS: return MAPID_BABY_GUILLOTINE_CROSS;
		case JOB_BABY_STAR_EMPEROR:     return MAPID_BABY_STAR_EMPEROR;
	//Baby 3-2 Jobs
		case JOB_BABY_ROYAL_GUARD:      return MAPID_BABY_ROYAL_GUARD;
		case JOB_BABY_SORCERER:         return MAPID_BABY_SORCERER;
		case JOB_BABY_MINSTREL:
		case JOB_BABY_WANDERER:         return MAPID_BABY_MINSTRELWANDERER;
		case JOB_BABY_SURA:             return MAPID_BABY_SURA;
		case JOB_BABY_GENETIC:          return MAPID_BABY_GENETIC;
		case JOB_BABY_SHADOW_CHASER:    return MAPID_BABY_SHADOW_CHASER;
		case JOB_BABY_SOUL_REAPER:      return MAPID_BABY_SOUL_REAPER;
	//Doram Jobs
		case JOB_SUMMONER:              return MAPID_SUMMONER;
		case JOB_SPIRIT_HANDLER:        return MAPID_SPIRIT_HANDLER;
	//4-1 Jobs
		case JOB_HYPER_NOVICE:          return MAPID_HYPER_NOVICE;
		case JOB_DRAGON_KNIGHT:         return MAPID_DRAGON_KNIGHT;
		case JOB_ARCH_MAGE:             return MAPID_ARCH_MAGE;
		case JOB_WINDHAWK:              return MAPID_WINDHAWK;
		case JOB_CARDINAL:              return MAPID_CARDINAL;
		case JOB_MEISTER:               return MAPID_MEISTER;
		case JOB_SHADOW_CROSS:          return MAPID_SHADOW_CROSS;
		case JOB_SKY_EMPEROR:           return MAPID_SKY_EMPEROR;
		case JOB_NIGHT_WATCH:           return MAPID_NIGHT_WATCH;
		case JOB_SHINKIRO:
		case JOB_SHIRANUI:              return MAPID_SHINKIRO_SHIRANUI;
	//4-2 Jobs
		case JOB_IMPERIAL_GUARD:        return MAPID_IMPERIAL_GUARD;
		case JOB_ELEMENTAL_MASTER:      return MAPID_ELEMENTAL_MASTER;
		case JOB_INQUISITOR:            return MAPID_INQUISITOR;
		case JOB_TROUBADOUR:
		case JOB_TROUVERE:              return MAPID_TROUBADOURTROUVERE;
		case JOB_BIOLO:                 return MAPID_BIOLO;
		case JOB_ABYSS_CHASER:          return MAPID_ABYSS_CHASER;
		case JOB_SOUL_ASCETIC:          return MAPID_SOUL_ASCETIC;
	//Unknown
		default:
			return -1;
	}
}

//Reverts the map-style class id to the client-style one.
int pc_mapid2jobid(uint64 class_, int sex)
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
		case MAPID_SUMMER2:               return JOB_SUMMER2;
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
		case MAPID_BABY_TAEKWON:          return JOB_BABY_TAEKWON;
		case MAPID_BABY_GUNSLINGER:       return JOB_BABY_GUNSLINGER;
		case MAPID_BABY_NINJA:            return JOB_BABY_NINJA;
		case MAPID_BABY_SUMMONER:         return JOB_BABY_SUMMONER;
	//Baby 2-1 Jobs
		case MAPID_SUPER_BABY:            return JOB_SUPER_BABY;
		case MAPID_BABY_KNIGHT:           return JOB_BABY_KNIGHT;
		case MAPID_BABY_WIZARD:           return JOB_BABY_WIZARD;
		case MAPID_BABY_HUNTER:           return JOB_BABY_HUNTER;
		case MAPID_BABY_PRIEST:           return JOB_BABY_PRIEST;
		case MAPID_BABY_BLACKSMITH:       return JOB_BABY_BLACKSMITH;
		case MAPID_BABY_ASSASSIN:         return JOB_BABY_ASSASSIN;
		case MAPID_BABY_STAR_GLADIATOR:   return JOB_BABY_STAR_GLADIATOR;
		case MAPID_BABY_REBELLION:        return JOB_BABY_REBELLION;
		case MAPID_BABY_KAGEROUOBORO:     return sex?JOB_BABY_KAGEROU:JOB_BABY_OBORO;
	//Baby 2-2 Jobs
		case MAPID_BABY_CRUSADER:         return JOB_BABY_CRUSADER;
		case MAPID_BABY_SAGE:             return JOB_BABY_SAGE;
		case MAPID_BABY_BARDDANCER:       return sex?JOB_BABY_BARD:JOB_BABY_DANCER;
		case MAPID_BABY_MONK:             return JOB_BABY_MONK;
		case MAPID_BABY_ALCHEMIST:        return JOB_BABY_ALCHEMIST;
		case MAPID_BABY_ROGUE:            return JOB_BABY_ROGUE;
		case MAPID_BABY_SOUL_LINKER:      return JOB_BABY_SOUL_LINKER;
	//3-1 Jobs
		case MAPID_SUPER_NOVICE_E:        return JOB_SUPER_NOVICE_E;
		case MAPID_RUNE_KNIGHT:           return JOB_RUNE_KNIGHT;
		case MAPID_WARLOCK:               return JOB_WARLOCK;
		case MAPID_RANGER:                return JOB_RANGER;
		case MAPID_ARCH_BISHOP:           return JOB_ARCH_BISHOP;
		case MAPID_MECHANIC:              return JOB_MECHANIC;
		case MAPID_GUILLOTINE_CROSS:      return JOB_GUILLOTINE_CROSS;
		case MAPID_STAR_EMPEROR:          return JOB_STAR_EMPEROR;
	//3-2 Jobs
		case MAPID_ROYAL_GUARD:           return JOB_ROYAL_GUARD;
		case MAPID_SORCERER:              return JOB_SORCERER;
		case MAPID_MINSTRELWANDERER:      return sex?JOB_MINSTREL:JOB_WANDERER;
		case MAPID_SURA:                  return JOB_SURA;
		case MAPID_GENETIC:               return JOB_GENETIC;
		case MAPID_SHADOW_CHASER:         return JOB_SHADOW_CHASER;
		case MAPID_SOUL_REAPER:           return JOB_SOUL_REAPER;
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
		case MAPID_BABY_RUNE_KNIGHT:      return JOB_BABY_RUNE_KNIGHT;
		case MAPID_BABY_WARLOCK:          return JOB_BABY_WARLOCK;
		case MAPID_BABY_RANGER:           return JOB_BABY_RANGER;
		case MAPID_BABY_ARCH_BISHOP:      return JOB_BABY_ARCH_BISHOP;
		case MAPID_BABY_MECHANIC:         return JOB_BABY_MECHANIC;
		case MAPID_BABY_GUILLOTINE_CROSS: return JOB_BABY_GUILLOTINE_CROSS;
		case MAPID_BABY_STAR_EMPEROR:     return JOB_BABY_STAR_EMPEROR;
	//Baby 3-2 Jobs
		case MAPID_BABY_ROYAL_GUARD:      return JOB_BABY_ROYAL_GUARD;
		case MAPID_BABY_SORCERER:         return JOB_BABY_SORCERER;
		case MAPID_BABY_MINSTRELWANDERER: return sex?JOB_BABY_MINSTREL:JOB_BABY_WANDERER;
		case MAPID_BABY_SURA:             return JOB_BABY_SURA;
		case MAPID_BABY_GENETIC:          return JOB_BABY_GENETIC;
		case MAPID_BABY_SHADOW_CHASER:    return JOB_BABY_SHADOW_CHASER;
		case MAPID_BABY_SOUL_REAPER:      return JOB_BABY_SOUL_REAPER;
	//Doram Jobs
		case MAPID_SUMMONER:              return JOB_SUMMONER;
		case MAPID_SPIRIT_HANDLER:        return JOB_SPIRIT_HANDLER;
	//4-1 Jobs
		case MAPID_HYPER_NOVICE:          return JOB_HYPER_NOVICE;
		case MAPID_DRAGON_KNIGHT:         return JOB_DRAGON_KNIGHT;
		case MAPID_ARCH_MAGE:             return JOB_ARCH_MAGE;
		case MAPID_WINDHAWK:              return JOB_WINDHAWK;
		case MAPID_CARDINAL:              return JOB_CARDINAL;
		case MAPID_MEISTER:               return JOB_MEISTER;
		case MAPID_SHADOW_CROSS:          return JOB_SHADOW_CROSS;
		case MAPID_SKY_EMPEROR:           return JOB_SKY_EMPEROR;
		case MAPID_NIGHT_WATCH:           return JOB_NIGHT_WATCH;
		case MAPID_SHINKIRO_SHIRANUI:     return sex?JOB_SHINKIRO:JOB_SHIRANUI;
	//4-2 Jobs
		case MAPID_IMPERIAL_GUARD:        return JOB_IMPERIAL_GUARD;
		case MAPID_ELEMENTAL_MASTER:      return JOB_ELEMENTAL_MASTER;
		case MAPID_INQUISITOR:            return JOB_INQUISITOR;
		case MAPID_TROUBADOURTROUVERE:    return sex?JOB_TROUBADOUR:JOB_TROUVERE;
		case MAPID_BIOLO:                 return JOB_BIOLO;
		case MAPID_ABYSS_CHASER:          return JOB_ABYSS_CHASER;
		case MAPID_SOUL_ASCETIC:          return JOB_SOUL_ASCETIC;
	//Unknown
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
	case JOB_SUMMER2:
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

	case JOB_BABY_RUNE_KNIGHT:
	case JOB_BABY_WARLOCK:
	case JOB_BABY_RANGER:
	case JOB_BABY_ARCH_BISHOP:
	case JOB_BABY_MECHANIC:
	case JOB_BABY_GUILLOTINE_CROSS:
	case JOB_BABY_ROYAL_GUARD:
	case JOB_BABY_SORCERER:
	case JOB_BABY_MINSTREL:
	case JOB_BABY_WANDERER:
	case JOB_BABY_SURA:
	case JOB_BABY_GENETIC:
	case JOB_BABY_SHADOW_CHASER:
		return msg_txt(NULL,638 - JOB_BABY_RUNE_KNIGHT+class_);

	case JOB_BABY_RUNE_KNIGHT2:
		return msg_txt(NULL,638);

	case JOB_BABY_ROYAL_GUARD2:
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

	case JOB_SUMMONER:
		return msg_txt(NULL,697);
	case JOB_BABY_SUMMONER:
		return msg_txt(NULL,698);
	case JOB_BABY_NINJA:
		return msg_txt(NULL,699);

	case JOB_BABY_KAGEROU:
	case JOB_BABY_OBORO:
	case JOB_BABY_TAEKWON:
	case JOB_BABY_STAR_GLADIATOR:
	case JOB_BABY_SOUL_LINKER:
	case JOB_BABY_GUNSLINGER:
	case JOB_BABY_REBELLION:
		return msg_txt(NULL,753 - JOB_BABY_KAGEROU+class_);

	case JOB_BABY_STAR_GLADIATOR2:
		return msg_txt(NULL,756);

	case JOB_STAR_EMPEROR:
	case JOB_SOUL_REAPER:
	case JOB_BABY_STAR_EMPEROR:
	case JOB_BABY_SOUL_REAPER:
		return msg_txt(NULL,782 - JOB_STAR_EMPEROR + class_);

	case JOB_STAR_EMPEROR2:
		return msg_txt(NULL,782);

	case JOB_BABY_STAR_EMPEROR2:
		return msg_txt(NULL,784);

	case JOB_DRAGON_KNIGHT:
	case JOB_MEISTER:
	case JOB_SHADOW_CROSS:
	case JOB_ARCH_MAGE:
	case JOB_CARDINAL:
	case JOB_WINDHAWK:
	case JOB_IMPERIAL_GUARD:
	case JOB_BIOLO:
	case JOB_ABYSS_CHASER:
	case JOB_ELEMENTAL_MASTER:
	case JOB_INQUISITOR:
	case JOB_TROUBADOUR:
	case JOB_TROUVERE:
		return msg_txt( nullptr, 800 - JOB_DRAGON_KNIGHT + class_ );

	case JOB_WINDHAWK2:
		return msg_txt( nullptr, 805);

	case JOB_MEISTER2:
		return msg_txt( nullptr, 801 );

	case JOB_DRAGON_KNIGHT2:
		return msg_txt( nullptr, 800 );

	case JOB_IMPERIAL_GUARD2:
		return msg_txt( nullptr, 806 );

	case JOB_SKY_EMPEROR:
	case JOB_SOUL_ASCETIC:
	case JOB_SHINKIRO:
	case JOB_SHIRANUI:
	case JOB_NIGHT_WATCH:
	case JOB_HYPER_NOVICE:
	case JOB_SPIRIT_HANDLER:
		return msg_txt( nullptr, 813 - JOB_SKY_EMPEROR + class_ );

	case JOB_SKY_EMPEROR2:
		return msg_txt( nullptr, 813 );

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
TIMER_FUNC(pc_follow_timer){
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

	if (tbl == NULL || pc_isdead(sd))
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
	t_exp next = pc_nextbaseexp(sd);

	if (!next || sd->status.base_exp < next || pc_is_maxbaselv(sd))
		return 0;

	uint32 base_level = sd->status.base_level;

	do {
		sd->status.base_exp -= next;
		//Kyoki pointed out that the max overcarry exp is the exp needed for the previous level -1. [Skotlex]
		if( ( !battle_config.multi_level_up || ( battle_config.multi_level_up_base > 0 && sd->status.base_level >= battle_config.multi_level_up_base ) ) && sd->status.base_exp > next-1 )
			sd->status.base_exp = next-1;

		sd->status.status_point += statpoint_db.pc_gets_status_point(sd->status.base_level);
		sd->status.trait_point += statpoint_db.pc_gets_trait_point(sd->status.base_level);
		sd->status.base_level++;

		if( pc_is_maxbaselv(sd) ){
			sd->status.base_exp = u64min(sd->status.base_exp,MAX_LEVEL_BASE_EXP);
			break;
		}
	} while ((next=pc_nextbaseexp(sd)) > 0 && sd->status.base_exp >= next);

	if (battle_config.pet_lv_rate && sd->pd)	//<Skotlex> update pet's level
		status_calc_pet(sd->pd,SCO_NONE);

	clif_updatestatus(sd,SP_STATUSPOINT);
	clif_updatestatus(sd,SP_TRAITPOINT);
	clif_updatestatus(sd,SP_BASELEVEL);
	clif_updatestatus(sd,SP_BASEEXP);
	clif_updatestatus(sd,SP_NEXTBASEEXP);
	status_calc_pc(sd,SCO_FORCE);
	status_percent_heal(&sd->bl,100,100);

	if ((sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE) {
		for (const auto &status : status_db) {
			if (status.second->flag[SCF_SUPERNOVICEANGEL])
				sc_start(&sd->bl, &sd->bl, status.second->type, 100, 1, skill_get_time(status.second->skill_id, 1));
		}
		if (sd->state.snovice_dead_flag)
			sd->state.snovice_dead_flag = 0; //Reenable steelbody resurrection on dead.
	} else if( (sd->class_&MAPID_BASEMASK) == MAPID_TAEKWON ) {
		for (const auto &status : status_db) {
			if (status.second->flag[SCF_TAEKWONANGEL])
				sc_start(&sd->bl, &sd->bl, status.second->type, 100, 10, 600000);
		}
	}
	clif_misceffect(&sd->bl,0);
	npc_script_event(sd, NPCE_BASELVUP); //LORDALFA - LVLUPEVENT

	if(sd->status.party_id)
		party_send_levelup(sd);

	pc_baselevelchanged(sd);
	for (; base_level <= sd->status.base_level; base_level++) {
		achievement_update_objective(sd, AG_GOAL_LEVEL, 1, base_level);
		achievement_update_objective(sd, AG_GOAL_STATUS, 2, base_level, sd->status.class_);
	}
	return 1;
}

void pc_baselevelchanged(struct map_session_data *sd) {
	uint8 i;
	for( i = 0; i < EQI_MAX; i++ ) {
		if( sd->equip_index[i] >= 0 && sd->inventory_data[sd->equip_index[i]] ) {
			if( sd->inventory_data[ sd->equip_index[i] ]->elvmax && sd->status.base_level > (unsigned int)sd->inventory_data[ sd->equip_index[i] ]->elvmax )
				pc_unequipitem(sd, sd->equip_index[i], 3);
		}
	}
	pc_show_questinfo(sd);
}

int pc_checkjoblevelup(struct map_session_data *sd)
{
	t_exp next = pc_nextjobexp(sd);

	nullpo_ret(sd);
	if(!next || sd->status.job_exp < next || pc_is_maxjoblv(sd))
		return 0;

	uint32 job_level = sd->status.job_level;

	do {
		sd->status.job_exp -= next;
		//Kyoki pointed out that the max overcarry exp is the exp needed for the previous level -1. [Skotlex]
		if( ( !battle_config.multi_level_up || ( battle_config.multi_level_up_job > 0 && sd->status.job_level >= battle_config.multi_level_up_job ) ) && sd->status.job_exp > next-1 )
			sd->status.job_exp = next-1;

		sd->status.job_level ++;
		sd->status.skill_point ++;

		if( pc_is_maxjoblv(sd) ){
			sd->status.job_exp = u64min(sd->status.job_exp,MAX_LEVEL_JOB_EXP);
			break;
		}
	} while ((next=pc_nextjobexp(sd)) > 0 && sd->status.job_exp >= next);

	clif_updatestatus(sd,SP_JOBLEVEL);
	clif_updatestatus(sd,SP_JOBEXP);
	clif_updatestatus(sd,SP_NEXTJOBEXP);
	clif_updatestatus(sd,SP_SKILLPOINT);
	status_calc_pc(sd,SCO_FORCE);
	clif_misceffect(&sd->bl,1);
	if (pc_checkskill(sd, SG_DEVIL) && ((sd->class_&MAPID_THIRDMASK) == MAPID_STAR_EMPEROR || pc_is_maxjoblv(sd)) )
		clif_status_change(&sd->bl, EFST_DEVIL1, 1, 0, 0, 0, 1); //Permanent blind effect from SG_DEVIL.

	npc_script_event(sd, NPCE_JOBLVUP);
	for (; job_level <= sd->status.job_level; job_level++)
		achievement_update_objective(sd, AG_GOAL_LEVEL, 1, job_level);

	pc_show_questinfo(sd);
	return 1;
}

/** Alters experiences calculation based on self bonuses that do not get even shared to the party.
* @param sd Player
* @param base_exp Base EXP before peronal bonuses
* @param job_exp Job EXP before peronal bonuses
* @param src Block list that affecting the exp calculation
*/
static void pc_calcexp(struct map_session_data *sd, t_exp *base_exp, t_exp *job_exp, struct block_list *src)
{
	int bonus = 0, vip_bonus_base = 0, vip_bonus_job = 0;

	if (src) {
		struct status_data *status = status_get_status_data(src);

		if( sd->indexed_bonus.expaddrace[status->race] )
			bonus += sd->indexed_bonus.expaddrace[status->race];
		if( sd->indexed_bonus.expaddrace[RC_ALL] )
			bonus += sd->indexed_bonus.expaddrace[RC_ALL];
		if( sd->indexed_bonus.expaddclass[status->class_] )
			bonus += sd->indexed_bonus.expaddclass[status->class_];
		if( sd->indexed_bonus.expaddclass[CLASS_ALL] )
			bonus += sd->indexed_bonus.expaddclass[CLASS_ALL];

		if (battle_config.pk_mode &&
			(int)(status_get_lv(src) - sd->status.base_level) >= 20)
			bonus += 15; // pk_mode additional exp if monster >20 levels [Valaris]

		if (src && src->type == BL_MOB && pc_isvip(sd)) { // EXP bonus for VIP player
			vip_bonus_base = battle_config.vip_base_exp_increase;
			vip_bonus_job = battle_config.vip_job_exp_increase;
		}
	}

	// Give EXPBOOST for quests even if src is NULL.
	if (sd->sc.data[SC_EXPBOOST]) {
		bonus += sd->sc.data[SC_EXPBOOST]->val1;
		if (battle_config.vip_bm_increase && pc_isvip(sd)) // Increase Battle Manual EXP rate for VIP
			bonus += (sd->sc.data[SC_EXPBOOST]->val1 / battle_config.vip_bm_increase);
	}

	if (*base_exp) {
		t_exp exp = (t_exp)(*base_exp + ((double)*base_exp * ((bonus + vip_bonus_base) / 100.)));
		*base_exp = cap_value(exp, 1, MAX_EXP);
	}

	// Give JEXPBOOST for quests even if src is NULL.
	if (sd->sc.data[SC_JEXPBOOST])
		bonus += sd->sc.data[SC_JEXPBOOST]->val1;

	if (*job_exp) {
		t_exp exp = (t_exp)(*job_exp + ((double)*job_exp * ((bonus + vip_bonus_job) / 100.)));
		*job_exp = cap_value(exp, 1, MAX_EXP);
	}

	return;
}

/**
 * Show EXP gained by player in percentage by @showexp
 * @param sd Player
 * @param base_exp Base EXP gained/loss
 * @param next_base_exp Base EXP needed for next base level
 * @param job_exp Job EXP gained/loss
 * @param next_job_exp Job EXP needed for next job level
 * @param lost True:EXP penalty, lose EXP
 **/
void pc_gainexp_disp(struct map_session_data *sd, t_exp base_exp, t_exp next_base_exp, t_exp job_exp, t_exp next_job_exp, bool lost) {
	char output[CHAT_SIZE_MAX];

	nullpo_retv(sd);

	sprintf(output, msg_txt(sd,743), // Experience %s Base:%ld (%0.2f%%) Job:%ld (%0.2f%%)
		(lost) ? msg_txt(sd,742) : msg_txt(sd,741),
		(long)base_exp * (lost ? -1 : 1), (base_exp / (float)next_base_exp * 100 * (lost ? -1 : 1)),
		(long)job_exp * (lost ? -1 : 1), (job_exp / (float)next_job_exp * 100 * (lost ? -1 : 1)));
	clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], output, false, SELF);
}

/**
 * Give Base or Job EXP to player, then calculate remaining exp for next lvl
 * @param sd Player
 * @param src EXP source
 * @param base_exp Base EXP gained
 * @param base_exp Job EXP gained
 * @param exp_flag 1: Quest EXP; 2: Param Exp (Ignore Guild EXP tax, EXP adjustments)
 * @return
 **/
void pc_gainexp(struct map_session_data *sd, struct block_list *src, t_exp base_exp, t_exp job_exp, uint8 exp_flag)
{
	t_exp nextb = 0, nextj = 0;
	uint8 flag = 0; ///< 1: Base EXP given, 2: Job EXP given, 4: Max Base level, 8: Max Job Level

	nullpo_retv(sd);

	if(sd->bl.prev == NULL || pc_isdead(sd))
		return;

	if (!(exp_flag&2)) {

		if (!battle_config.pvp_exp && map_getmapflag(sd->bl.m, MF_PVP))  // [MouseJstr]
			return; // no exp on pvp maps
	
		if (sd->status.guild_id>0)
			base_exp -= guild_payexp(sd,base_exp);
	}

	flag = ((base_exp) ? 1 : 0) |
		((job_exp) ? 2 : 0) |
		((pc_is_maxbaselv(sd)) ? 4 : 0) |
		((pc_is_maxjoblv(sd)) ? 8 : 0);

	if (!(exp_flag&2))
		pc_calcexp(sd, &base_exp, &job_exp, src);

	nextb = pc_nextbaseexp(sd);
	nextj = pc_nextjobexp(sd);

	if (flag&4){
		if( sd->status.base_exp >= MAX_LEVEL_BASE_EXP )
			base_exp = 0;
		else if( sd->status.base_exp + base_exp >= MAX_LEVEL_BASE_EXP )
			base_exp = MAX_LEVEL_BASE_EXP - sd->status.base_exp;
	}
	if (flag&8){
		if( sd->status.job_exp >= MAX_LEVEL_JOB_EXP )
			job_exp = 0;
		else if( sd->status.job_exp + job_exp >= MAX_LEVEL_JOB_EXP )
			job_exp = MAX_LEVEL_JOB_EXP - sd->status.job_exp;
	}

	if (!(exp_flag&2) && battle_config.max_exp_gain_rate && (base_exp || job_exp)) {
		//Note that this value should never be greater than the original
		//therefore no overflow checks are needed. [Skotlex]
		if (nextb > 0) {
			float nextbp = (float) base_exp / (float) nextb;
			if (nextbp > battle_config.max_exp_gain_rate/1000.)
				base_exp = (unsigned int)(battle_config.max_exp_gain_rate/1000.*nextb);
		}
		if (nextj > 0) {
			float nextjp = (float) job_exp / (float) nextj;
			if (nextjp > battle_config.max_exp_gain_rate/1000.)
				job_exp = (unsigned int)(battle_config.max_exp_gain_rate/1000.*nextj);
		}
	}

	// Give EXP for Base Level
	if (base_exp) {
		sd->status.base_exp = util::safe_addition_cap(sd->status.base_exp, base_exp, MAX_EXP);

		if (!pc_checkbaselevelup(sd))
			clif_updatestatus(sd,SP_BASEEXP);
	}

	// Give EXP for Job Level
	if (job_exp) {
		sd->status.job_exp = util::safe_addition_cap(sd->status.job_exp, job_exp, MAX_EXP);

		if (!pc_checkjoblevelup(sd))
			clif_updatestatus(sd,SP_JOBEXP);
	}

	if (flag&1)
		clif_displayexp(sd, (flag&4) ? 0 : base_exp, SP_BASEEXP, exp_flag&1, false);
	if (flag&2)
		clif_displayexp(sd, (flag&8) ? 0 : job_exp,  SP_JOBEXP, exp_flag&1, false);

	if (sd->state.showexp && (base_exp || job_exp))
		pc_gainexp_disp(sd, base_exp, nextb, job_exp, nextj, false);
}

/**
 * Lost Base/Job EXP from a player
 * @param sd Player
 * @param base_exp Base EXP lost
 * @param job_exp Job EXP lost
 **/
void pc_lostexp(struct map_session_data *sd, t_exp base_exp, t_exp job_exp) {

	nullpo_retv(sd);

	if (base_exp) {
		base_exp = u64min(sd->status.base_exp, base_exp);
		sd->status.base_exp -= base_exp;
		clif_displayexp(sd, base_exp, SP_BASEEXP, false, true);
		clif_updatestatus(sd, SP_BASEEXP);
	}

	if (job_exp) {
		job_exp = u64min(sd->status.job_exp, job_exp);
		sd->status.job_exp -= job_exp;
		clif_displayexp(sd, job_exp, SP_JOBEXP, false, true);
		clif_updatestatus(sd, SP_JOBEXP);
	}

	if (sd->state.showexp && (base_exp || job_exp))
		pc_gainexp_disp(sd, base_exp, pc_nextbaseexp(sd), job_exp, pc_nextjobexp(sd), true);
}

/**
 * Returns max base level for this character's class.
 * @param job_id: Player's class
 * @return Max Base Level
 */
uint32 JobDatabase::get_maxBaseLv(uint16 job_id) {
	std::shared_ptr<s_job_info> job = job_db.find(job_id);

	return job ? job->max_base_level : 0;
}

/**
 * Returns max base level for this character.
 * @param sd Player
 * @return Max Base Level
 **/
unsigned int pc_maxbaselv(struct map_session_data *sd){
	return job_db.get_maxBaseLv(sd->status.class_);
}

/**
 * Returns max job level for this character's class.
 * @param job_id: Player's class
 * @return Max Job Level
 */
uint32 JobDatabase::get_maxJobLv(uint16 job_id) {
	std::shared_ptr<s_job_info> job = job_db.find(job_id);

	return job ? job->max_job_level : 0;
}

/**
 * Returns max job level for this character.
 * @param sd Player
 * @return Max Job Level
 **/
unsigned int pc_maxjoblv(struct map_session_data *sd){
	return job_db.get_maxJobLv(sd->status.class_);
}

/**
 * Check if player is reached max base level
 * @param sd
 * @return True if reached max level
 **/
bool pc_is_maxbaselv(struct map_session_data *sd) {
	nullpo_retr(false, sd);
	return (sd->status.base_level >= pc_maxbaselv(sd));
}

/**
 * Check if player is reached max base level
 * @param sd
 * @return True if reached max level
 **/
bool pc_is_maxjoblv(struct map_session_data *sd) {
	nullpo_retr(false, sd);
	return (sd->status.job_level >= pc_maxjoblv(sd));
}

/**
 * Returns base experience for this character's class.
 * @param job_id: Player's class
 * @param level: Player's level
 * @return Base EXP
 */
t_exp JobDatabase::get_baseExp(uint16 job_id, uint32 level) {
	std::shared_ptr<s_job_info> job = job_db.find(job_id);

	return job ? job->base_exp[level - 1] : 0;
}

/**
 * Base exp needed for player to level up.
 * @param sd
 * @return Base EXP needed for next base level
 **/
t_exp pc_nextbaseexp(struct map_session_data *sd){
	nullpo_ret(sd);
	if (sd->status.base_level == 0) // Is this something that possible?
		return 0;
	if (pc_is_maxbaselv(sd))
		return MAX_LEVEL_BASE_EXP; // On max level, player's base EXP limit is 99,999,999
	return static_cast<t_exp>(job_db.get_baseExp(sd->status.class_, sd->status.base_level));
}

/**
 * Returns job experience for this character's class.
 * @param job_id: Player's class
 * @param level: Player's level
 * @return Job EXP
 */
t_exp JobDatabase::get_jobExp(uint16 job_id, uint32 level) {
	std::shared_ptr<s_job_info> job = job_db.find(job_id);

	return job ? job->job_exp[level - 1] : 0;
}

/**
 * Job exp needed for player to level up.
 * @param sd
 * @return Job EXP needed for next job level
 **/
t_exp pc_nextjobexp(struct map_session_data *sd){
	nullpo_ret(sd);
	if (sd->status.job_level == 0) // Is this something that possible?
		return 0;
	if (pc_is_maxjoblv(sd))
		return MAX_LEVEL_JOB_EXP; // On max level, player's job EXP limit is 999,999,999
	return static_cast<t_exp>(job_db.get_jobExp(sd->status.class_, sd->status.job_level));
}

/**
 * Returns max weight base for this character's class.
 * @param job_id: Player's class
 * @return Max weight base
 */
int32 JobDatabase::get_maxWeight(uint16 job_id) {
	std::shared_ptr<s_job_info> job = job_db.find(job_id);

	return job ? job->max_weight_base : 0;
}

/// Returns the value of the specified stat.
int pc_getstat(map_session_data *sd, int type)
{
	nullpo_retr(-1, sd);

	switch( type ) {
	case SP_STR: return sd->status.str;
	case SP_AGI: return sd->status.agi;
	case SP_VIT: return sd->status.vit;
	case SP_INT: return sd->status.int_;
	case SP_DEX: return sd->status.dex;
	case SP_LUK: return sd->status.luk;
	case SP_POW: return sd->status.pow;
	case SP_STA: return sd->status.sta;
	case SP_WIS: return sd->status.wis;
	case SP_SPL: return sd->status.spl;
	case SP_CON: return sd->status.con;
	case SP_CRT: return sd->status.crt;
	default:
		return -1;
	}
}

/// Sets the specified stat to the specified value.
/// Returns the new value.
int pc_setstat(struct map_session_data* sd, int type, int val)
{
	nullpo_retr(-1, sd);

	switch( type ) {
	case SP_STR: sd->status.str = val; break;
	case SP_AGI: sd->status.agi = val; break;
	case SP_VIT: sd->status.vit = val; break;
	case SP_INT: sd->status.int_ = val; break;
	case SP_DEX: sd->status.dex = val; break;
	case SP_LUK: sd->status.luk = val; break;
	case SP_POW: sd->status.pow = val; break;
	case SP_STA: sd->status.sta = val; break;
	case SP_WIS: sd->status.wis = val; break;
	case SP_SPL: sd->status.spl = val; break;
	case SP_CON: sd->status.con = val; break;
	case SP_CRT: sd->status.crt = val; break;
	default:
		return -1;
	}

	return val;
}
/**
 * Gets the total number of status points at the provided level.
 * @param level: Player base level.
 * @return Total number of status points at specific base level.
 */
uint32 PlayerStatPointDatabase::get_table_point(uint16 level) {
	std::shared_ptr<s_statpoint_entry> entry = this->find( level );

	if( entry != nullptr ){
		return entry->statpoints;
	}else{
		return 0;
	}
}

/**
 * Calculates the number of status points PC gets when leveling up (from level to level+1)
 * @param level: Player base level.
 * @param table: Use table value or formula.
 * @return Status points at specific base level.
 */
uint32 PlayerStatPointDatabase::pc_gets_status_point(uint16 level) {
	uint32 next_level = this->get_table_point( level + 1 );
	uint32 current_level = this->get_table_point( level );

	if( next_level > current_level ){
		return next_level - current_level;
	}else{
		return 0;
	}
}

/**
* Gets the total number of trait points at the provided level.
* @param level: Player base level.
* @return Total number of trait points at specific base level.
*/
uint32 PlayerStatPointDatabase::get_trait_table_point(uint16 level) {
	std::shared_ptr<s_statpoint_entry> entry = this->find( level );

	if( entry != nullptr ){
		return entry->traitpoints;
	}else{
		return 0;
	}
}

/**
* Calculates the number of trait points PC gets when leveling up (from level to level+1)
* @param level: Player base level.
* @param table: Use table value or formula.
* @return Trait points at specific base level.
*/
uint32 PlayerStatPointDatabase::pc_gets_trait_point(uint16 level) {
	uint32 next_level = this->get_trait_table_point( level + 1 );
	uint32 current_level = this->get_trait_table_point( level );

	if( next_level > current_level ){
		return next_level - current_level;
	}else{
		return 0;
	}
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
		SWAP(low, high);

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

	achievement_update_objective(sd, AG_GOAL_STATUS, 1, final_value);

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

/// Returns the number of trait stat points needed to change the specified trait stat by val.
/// If val is negative, returns the number of trait stat points that would be needed to
/// raise the specified trait stat from (current value - val) to current value.
int pc_need_trait_point(struct map_session_data* sd, int type, int val)
{
	nullpo_retr(0, sd);

	if (val == 0 || type < SP_POW || type > SP_CRT)
		return 0;

	int low = pc_getstat(sd, type);
	int max = pc_maxparameter(sd, (e_params)(PARAM_POW + type - SP_POW));

	if (low >= max && val > 0)
		return 0; // Official servers show '0' when max is reached

	int high = low + val, sp = 0;

	if (val < 0)
		SWAP(low, high);

	for (; low < high; low++)
		sp += 1;

	return sp;
}

/**
* Returns the value the specified trait stat can be increased by with the current
* amount of available trait status points for the current character's class.
*
* @param sd   The target character.
* @param type Trait stat to verify.
* @return Maximum value the stat could grow by.
*/
int pc_maxtraitparameterincrease(struct map_session_data* sd, int type)
{
	nullpo_ret(sd);

	if( type < SP_POW || type > SP_CRT ){
		return 0;
	}

	int base, final_val = pc_getstat(sd, type);
	int trait_points = sd->status.trait_point;
	int max_param = pc_maxparameter(sd, (enum e_params)(PARAM_POW + type - SP_POW));

	base = final_val;

	while (final_val <= max_param && trait_points >= 0) {
		trait_points -= 1;
		final_val++;
	}
	final_val--;

	return (final_val > base ? final_val - base : 0);
}

/**
* Raises a trait stat by the specified amount.
*
* Obeys max_traitparameter limits.
* Subtracts trait status points according to the cost of the increased trait stat points.
*
* @param sd       The target character.
* @param type     The stat to change (see enum _sp)
* @param increase The stat increase (strictly positive) amount.
* @retval true  if the trait stat was increased by any amount.
* @retval false if there were no changes.
*/
bool pc_traitstatusup(struct map_session_data* sd, int type, int increase)
{
	nullpo_ret(sd);

	// check conditions
	if (type < SP_POW || type > SP_CRT || increase <= 0) {
		clif_statusupack(sd, type, 0, 0);
		return false;
	}

	// check limits
	int current = pc_getstat(sd, type);
	int max_increase = pc_maxtraitparameterincrease(sd, type);

	increase = cap_value(increase, 0, max_increase); // cap to the maximum status points available
	if (increase <= 0 || current + increase > pc_maxparameter(sd, (enum e_params)(PARAM_POW + type - SP_POW))) {
		clif_statusupack(sd, type, 0, 0);
		return false;
	}

	// check status points
	int needed_points = pc_need_trait_point(sd, type, increase);

	if (needed_points < 0 || needed_points > sd->status.trait_point) { // Sanity check
		clif_statusupack(sd, type, 0, 0);
		return false;
	}

	// set new values
	int final_value = pc_setstat(sd, type, current + increase);

	sd->status.trait_point -= needed_points;

	status_calc_pc(sd, SCO_NONE);

	// update increase cost indicator
	clif_updatestatus(sd, SP_UPOW + type - SP_POW);

	// update statpoint count
	clif_updatestatus(sd, SP_TRAITPOINT);

	// update stat value
	clif_statusupack(sd, type, 1, final_value); // required
	if (final_value > 255)
		clif_updatestatus(sd, type); // send after the 'ack' to override the truncated value

	//achievement_update_objective(sd, AG_GOAL_STATUS, 1, final_value);

	return true;
}

/**
* Raises a trait stat by the specified amount.
*
* Obeys max_trait_parameter limits.
* Does not subtract status points for the cost of the modified stat points.
*
* @param sd   The target character.
* @param type The stat to change (see enum _sp)
* @param val  The stat increase (or decrease) amount.
* @return the stat increase amount.
* @retval 0 if no changes were made.
*/
int pc_traitstatusup2(struct map_session_data* sd, int type, int val)
{
	nullpo_ret(sd);

	if (type < SP_POW || type > SP_CRT) {
		clif_statusupack(sd, type, 0, 0);
		return 0;
	}

	int need = pc_need_trait_point(sd, type, 1);
	int max = pc_maxparameter(sd, (enum e_params)(PARAM_POW + type - SP_POW)); // set new value

	val = pc_setstat(sd, type, cap_value(pc_getstat(sd, type) + val, 0, max));

	status_calc_pc(sd, SCO_NONE);

	// update increase cost indicator
	if (need != pc_need_trait_point(sd, type, 1))
		clif_updatestatus(sd, SP_UPOW + type - SP_POW);

	// update stat value
	clif_statusupack(sd, type, 1, val); // required
	if (val > 255)
		clif_updatestatus(sd, type); // send after the 'ack' to override the truncated value

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
			if( !skill_get_inf(skill_id) || pc_checkskill_summoner(sd, SUMMONER_POWER_LAND) >= 20 || pc_checkskill_summoner(sd, SUMMONER_POWER_SEA) >= 20 )
				status_calc_pc(sd,SCO_NONE); // Only recalculate for passive skills.
			else if( sd->status.skill_point == 0 && pc_is_taekwon_ranker(sd) )
				pc_calc_skilltree(sd); // Required to grant all TK Ranker skills.
			else
				pc_check_skilltree(sd); // Check if a new skill can Lvlup

			lv = sd->status.skill[idx].lv;
			range = skill_get_range2(&sd->bl, skill_id, lv, false);
			upgradable = (lv < skill_tree_get_max(sd->status.skill[idx].id, sd->status.class_)) ? 1 : 0;
			clif_skillup(sd,skill_id,lv,range,upgradable);
			clif_updatestatus(sd,SP_SKILLPOINT);
			if( skill_id == GN_REMODELING_CART ) /* cart weight info was updated by status_calc_pc */
				clif_updatestatus(sd,SP_CARTINFO);
			if (pc_checkskill(sd, SG_DEVIL) && ((sd->class_&MAPID_THIRDMASK) == MAPID_STAR_EMPEROR || pc_is_maxjoblv(sd)))
				clif_status_change(&sd->bl, EFST_DEVIL1, 1, 0, 0, 0, 1); //Permanent blind effect from SG_DEVIL.
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
		std::shared_ptr<s_skill_tree> tree = skill_tree_db.find(sd->status.class_);

		if (tree != nullptr && !tree->skills.empty()) {
			for (const auto &skillsit : tree->skills) {
				sk_id = skillsit.first;
				uint16 sk_idx = skill_get_index(sk_id);

				if (sk_idx == 0)
					continue;

				std::shared_ptr<s_skill_db> skill = skill_db.find(sk_id);

				if (
					(skill->inf2[INF2_ISQUEST] && !battle_config.quest_skill_learn) ||
					((skill->inf2[INF2_ISWEDDING] || skill->inf2[INF2_ISSPIRIT])) ||
					sk_id == SG_DEVIL
				)
					continue; //Cannot be learned normally.

				sd->status.skill[sk_idx].id = sk_id;
				sd->status.skill[sk_idx].lv = skill_tree_get_max(sk_id, sd->status.class_);	// celest
			}
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
		sd->status.trait_point = 0;
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
		sd->status.pow=0;
		sd->status.sta=0;
		sd->status.wis=0;
		sd->status.spl=0;
		sd->status.con=0;
		sd->status.crt=0;

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
	clif_updatestatus(sd,SP_TRAITPOINT);
	clif_updatestatus(sd,SP_STR);
	clif_updatestatus(sd,SP_AGI);
	clif_updatestatus(sd,SP_VIT);
	clif_updatestatus(sd,SP_INT);
	clif_updatestatus(sd,SP_DEX);
	clif_updatestatus(sd,SP_LUK);
	clif_updatestatus(sd,SP_POW);
	clif_updatestatus(sd,SP_STA);
	clif_updatestatus(sd,SP_WIS);
	clif_updatestatus(sd,SP_SPL);
	clif_updatestatus(sd,SP_CON);
	clif_updatestatus(sd,SP_CRT);
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
	clif_updatestatus(sd,SP_UPOW);
	clif_updatestatus(sd,SP_USTA);
	clif_updatestatus(sd,SP_UWIS);
	clif_updatestatus(sd,SP_USPL);
	clif_updatestatus(sd,SP_UCON);
	clif_updatestatus(sd,SP_UCRT);

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

	if( sd->status.base_level > pc_maxbaselv( sd ) ){
		ShowError( "pc_resetstate: Capping the Level to %d to reset the stats of %d:%d, the base level (%d) is greater than the max level supported.\n",
			pc_maxbaselv( sd ), sd->status.account_id, sd->status.char_id, sd->status.base_level );
		sd->status.base_level = pc_maxbaselv( sd );
		clif_updatestatus( sd, SP_BASELEVEL );
	}

	sd->status.status_point = statpoint_db.get_table_point( sd->status.base_level );
	sd->status.trait_point = statpoint_db.get_trait_table_point(sd->status.base_level);

	if( ( sd->class_&JOBL_UPPER ) != 0 ){
		sd->status.status_point += battle_config.transcendent_status_points;
	}

	if ((sd->class_&JOBL_FOURTH) != 0) {
		sd->status.trait_point += battle_config.trait_points_job_change;
	}

	pc_setstat(sd, SP_STR, 1);
	pc_setstat(sd, SP_AGI, 1);
	pc_setstat(sd, SP_VIT, 1);
	pc_setstat(sd, SP_INT, 1);
	pc_setstat(sd, SP_DEX, 1);
	pc_setstat(sd, SP_LUK, 1);
	pc_setstat(sd, SP_POW, 0);
	pc_setstat(sd, SP_STA, 0);
	pc_setstat(sd, SP_WIS, 0);
	pc_setstat(sd, SP_SPL, 0);
	pc_setstat(sd, SP_CON, 0);
	pc_setstat(sd, SP_CRT, 0);

	clif_updatestatus(sd,SP_STR);
	clif_updatestatus(sd,SP_AGI);
	clif_updatestatus(sd,SP_VIT);
	clif_updatestatus(sd,SP_INT);
	clif_updatestatus(sd,SP_DEX);
	clif_updatestatus(sd,SP_LUK);
	clif_updatestatus(sd,SP_POW);
	clif_updatestatus(sd,SP_STA);
	clif_updatestatus(sd,SP_WIS);
	clif_updatestatus(sd,SP_SPL);
	clif_updatestatus(sd,SP_CON);
	clif_updatestatus(sd,SP_CRT);

	clif_updatestatus(sd,SP_USTR);	// Updates needed stat points - Valaris
	clif_updatestatus(sd,SP_UAGI);
	clif_updatestatus(sd,SP_UVIT);
	clif_updatestatus(sd,SP_UINT);
	clif_updatestatus(sd,SP_UDEX);
	clif_updatestatus(sd,SP_ULUK);	// End Addition
	clif_updatestatus(sd,SP_UPOW);
	clif_updatestatus(sd,SP_USTA);
	clif_updatestatus(sd,SP_UWIS);
	clif_updatestatus(sd,SP_USPL);
	clif_updatestatus(sd,SP_UCON);
	clif_updatestatus(sd,SP_UCRT);

	clif_updatestatus(sd,SP_STATUSPOINT);
	clif_updatestatus(sd,SP_TRAITPOINT);

	if( sd->mission_mobid ) { //bugreport:2200
		sd->mission_mobid = 0;
		sd->mission_count = 0;
		pc_setglobalreg(sd, add_str(TKMISSIONID_VAR), 0);
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

		if( pc_checkskill(sd, SG_DEVIL) && ((sd->class_&MAPID_THIRDMASK) == MAPID_STAR_EMPEROR || pc_is_maxjoblv(sd)) )
			clif_status_load(&sd->bl, EFST_DEVIL1, 0); //Remove perma blindness due to skill-reset. [Skotlex]
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

		if (sd->sc.data[SC_SPRITEMABLE] && pc_checkskill(sd, SU_SPRITEMABLE))
			status_change_end(&sd->bl, SC_SPRITEMABLE, INVALID_TIMER);
		if (sd->sc.data[SC_SOULATTACK] && pc_checkskill(sd, SU_SOULATTACK))
			status_change_end(&sd->bl, SC_SOULATTACK, INVALID_TIMER);
	}

	for (const auto &skill : skill_db) {
		uint16 skill_id = skill.second->nameid, idx = skill_get_index(skill_id);
		uint8 lv = sd->status.skill[idx].lv;

		if (lv == 0 || skill_id == 0)
			continue;

		if( skill.second->inf2[INF2_ISWEDDING] || skill.second->inf2[INF2_ISSPIRIT] ) //Avoid reseting wedding/linker skills.
			continue;

		// Don't reset trick dead if not a novice/baby
		if( skill_id == NV_TRICKDEAD && (sd->class_&MAPID_UPPERMASK) != MAPID_NOVICE )
		{
			sd->status.skill[idx].lv = 0;
			sd->status.skill[idx].flag = SKILL_FLAG_PERMANENT;
			continue;
		}

		// do not reset basic skill
		if (skill_id == NV_BASIC && (sd->class_&MAPID_UPPERMASK) != MAPID_NOVICE )
			continue;

		if( sd->status.skill[idx].flag == SKILL_FLAG_PERM_GRANTED )
			continue;

		if( flag&4 && !skill_ischangesex(skill_id) )
			continue;

		if( skill.second->inf2[INF2_ISQUEST] && !battle_config.quest_skill_learn )
		{ //Only handle quest skills in a special way when you can't learn them manually
			if( battle_config.quest_skill_reset && !(flag&2) )
			{	//Wipe them
				sd->status.skill[idx].lv = 0;
				sd->status.skill[idx].flag = SKILL_FLAG_PERMANENT;
			}
			continue;
		}
		if( sd->status.skill[idx].flag == SKILL_FLAG_PERMANENT )
			skill_point += lv;
		else
		if( sd->status.skill[idx].flag >= SKILL_FLAG_REPLACED_LV_0 )
			skill_point += (sd->status.skill[idx].flag - SKILL_FLAG_REPLACED_LV_0);

		if( !(flag&2) )
		{// reset
			sd->status.skill[idx].lv = 0;
			sd->status.skill[idx].flag = SKILL_FLAG_PERMANENT;
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

	for (i=0; i<MAX_PC_FEELHATE; i++)
	{
		sd->hate_mob[i] = -1;
		pc_setglobalreg(sd, add_str(sg_info[i].hate_var), 0);
	}
	return 0;
}

int pc_skillatk_bonus(struct map_session_data *sd, uint16 skill_id)
{
	int bonus = 0;

	nullpo_ret(sd);

	skill_id = skill_dummy2skill_id(skill_id);

	for (auto &it : sd->skillatk) {
		if (it.id == skill_id) {
			bonus += it.val;
			break;
		}
	}

	return bonus;
}

int pc_sub_skillatk_bonus(struct map_session_data *sd, uint16 skill_id)
{
	int bonus = 0;

	nullpo_ret(sd);

	skill_id = skill_dummy2skill_id(skill_id);

	for (auto &it : sd->subskill) {
		if (it.id == skill_id) {
			bonus += it.val;
			break;
		}
	}

	return bonus;
}

int pc_skillheal_bonus(struct map_session_data *sd, uint16 skill_id) {
	int bonus = sd->bonus.add_heal_rate;

	nullpo_ret(sd);

	skill_id = skill_dummy2skill_id(skill_id);

	if( bonus ) {
		switch( skill_id ) {
			case AL_HEAL:           if( !(battle_config.skill_add_heal_rate&1) ) bonus = 0; break;
			case PR_SANCTUARY:      if( !(battle_config.skill_add_heal_rate&2) ) bonus = 0; break;
			case AM_POTIONPITCHER:  if( !(battle_config.skill_add_heal_rate&4) ) bonus = 0; break;
			case CR_SLIMPITCHER:    if( !(battle_config.skill_add_heal_rate&8) ) bonus = 0; break;
			case BA_APPLEIDUN:      if( !(battle_config.skill_add_heal_rate&16)) bonus = 0; break;
			case AB_CHEAL:          if (!(battle_config.skill_add_heal_rate & 32)) bonus = 0; break;
			case AB_HIGHNESSHEAL:   if (!(battle_config.skill_add_heal_rate & 64)) bonus = 0; break;
			case CD_MEDIALE_VOTUM:  if (!(battle_config.skill_add_heal_rate & 128)) bonus = 0; break;
			case CD_DILECTIO_HEAL:  if (!(battle_config.skill_add_heal_rate & 256)) bonus = 0; break;
		}
	}

	for (auto &it : sd->skillheal) {
		if (it.id == skill_id) {
			bonus += it.val;
			break;
		}
	}

	return bonus;
}

int pc_skillheal2_bonus(struct map_session_data *sd, uint16 skill_id) {
	int bonus = sd->bonus.add_heal2_rate;

	skill_id = skill_dummy2skill_id(skill_id);

	for (auto &it : sd->skillheal2) {
		if (it.id == skill_id) {
			bonus += it.val;
			break;
		}
	}

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
	if( pc_setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, clrtype) != SETPOS_OK )
		clif_resurrection(&sd->bl, 1); //If warping fails, send a normal stand up packet.
}

static TIMER_FUNC(pc_respawn_timer){
	struct map_session_data *sd = map_id2sd(id);
	if( sd != NULL )
	{
		sd->pvp_point=0;
		sd->respawn_tid = INVALID_TIMER;
		pc_respawn(sd,CLR_OUTSIGHT);
	}

	return 0;
}

/*==========================================
 * Invoked when a player has received damage
 *------------------------------------------*/
void pc_damage(struct map_session_data *sd,struct block_list *src,unsigned int hp, unsigned int sp, unsigned int ap)
{
	if (ap) clif_updatestatus(sd,SP_AP);
	if (sp) clif_updatestatus(sd,SP_SP);
	if (hp) clif_updatestatus(sd,SP_HP);
	else return;

	if (!src)
		return;

	if( pc_issit(sd) ) {
		pc_setstand(sd, true);
		skill_sit(sd,0);
	}

	if (sd->progressbar.npc_id)
		clif_progressbar_abort(sd);

	if( sd->status.pet_id > 0 && sd->pd && battle_config.pet_damage_support )
		pet_target_check(sd->pd,src,1);

	if( sd->status.ele_id > 0 )
		elemental_set_target(sd,src);

	if(battle_config.prevent_logout_trigger&PLT_DAMAGE)
		sd->canlog_tick = gettick();
}

TIMER_FUNC(pc_close_npc_timer){
	TBL_PC *sd = map_id2sd(id);
	if(sd) pc_close_npc(sd,data);
	return 0;
}
/**
 * Method to properly close a NPC for player and clear anything related.
 * @param sd: Player attached
 * @param flag: Method of closure
 *   1: Produce a close button and end the NPC
 *   2: End the NPC (best for no dialog windows)
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
		if( sd->npc_idle_timer != INVALID_TIMER ){
			delete_timer( sd->npc_idle_timer, npc_secure_timeout_timer );
			sd->npc_idle_timer = INVALID_TIMER;
		}
#endif
		if (sd->st) {
			if (sd->st->state == CLOSE) {
				clif_scriptclose(sd, sd->npc_id);
				clif_scriptclear(sd, sd->npc_id); // [Ind/Hercules]
				sd->st->state = END; // Force to end now
			}
			if (sd->st->state == END) { // free attached scripts that are waiting
				script_free_state(sd->st);
				sd->st = NULL;
				sd->npc_id = 0;
			}
		}
	}
}

/*==========================================
 * Invoked when a player has negative current hp
 *------------------------------------------*/
int pc_dead(struct map_session_data *sd,struct block_list *src)
{
	int i=0,k=0;
	t_tick tick = gettick();
	struct map_data *mapdata = map_getmapdata(sd->bl.m);

	// Activate Steel body if a super novice dies at 99+% exp [celest]
	// Super Novices have no kill or die functions attached when saved by their angel
	if (!sd->state.snovice_dead_flag && (sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE) {
		t_exp exp = pc_nextbaseexp(sd);

		if( exp && get_percentage_exp(sd->status.base_exp, exp) >= 99 ) {
			sd->state.snovice_dead_flag = 1;
			pc_setrestartvalue(sd,1);
			status_percent_heal(&sd->bl, 100, 100);
			clif_resurrection(&sd->bl, 1);
			if(battle_config.pc_invincible_time)
				pc_setinvincibletimer(sd, battle_config.pc_invincible_time);
			sc_start(&sd->bl,&sd->bl,SC_STEELBODY,100,5,skill_get_time(MO_STEELBODY,5));
			if(mapdata_flag_gvg2(mapdata))
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

	for (k = 0; k < MAX_STELLAR_MARKS; k++) {
		if (sd->stellar_mark[k]) {
			struct map_session_data *smarksd = map_id2sd(sd->stellar_mark[k]);

			if (smarksd)
				status_change_end(&smarksd->bl, SC_FLASHKICK, INVALID_TIMER);
			sd->stellar_mark[k] = 0;
		}
	}

	for (k = 0; k < MAX_UNITED_SOULS; k++) {
		if (sd->united_soul[k]) {
			struct map_session_data *usoulsd = map_id2sd(sd->united_soul[k]);

			if (usoulsd)
				status_change_end(&usoulsd->bl, SC_SOULUNITY, INVALID_TIMER);
			sd->united_soul[k] = 0;
		}
	}

	for (k = 0; k < MAX_SERVANT_SIGN; k++) {
		if (sd->servant_sign[k]) {
			struct map_session_data *ssignsd = map_id2sd(sd->servant_sign[k]);

			if (ssignsd)
				status_change_end(&ssignsd->bl, SC_SERVANT_SIGN, INVALID_TIMER);
			sd->servant_sign[k] = 0;
		}
	}

	if(sd->shadowform_id) { //if we were target of shadowform
		status_change_end(map_id2bl(sd->shadowform_id), SC__SHADOWFORM, INVALID_TIMER);
		sd->shadowform_id = 0; //should be remove on status end anyway
	}

	if(sd->status.pet_id > 0 && sd->pd) {
		struct pet_data *pd = sd->pd;
		if( !mapdata->flag[MF_NOEXPPENALTY] ) {
			pet_set_intimate(pd, pd->pet.intimate + pd->get_pet_db()->die);
			if( pd->pet.intimate <= PET_INTIMATE_NONE )
				pet_set_intimate(pd, PET_INTIMATE_NONE);
			clif_send_petdata(sd,sd->pd,1,pd->pet.intimate);
		}
		if( sd->pd->target_id ) // Unlock all targets...
			pet_unlocktarget(sd->pd);
	}

	if (hom_is_active(sd->hd) && battle_config.homunculus_auto_vapor && get_percentage(sd->hd->battle_status.hp, sd->hd->battle_status.max_hp) >= battle_config.homunculus_auto_vapor)
		hom_vaporize(sd, HOM_ST_ACTIVE);

	if( sd->md )
		mercenary_delete(sd->md, 3); // Your mercenary soldier has ran away.

	if( sd->ed )
		elemental_delete(sd->ed);

	// Leave duel if you die [LuzZza]
	if(battle_config.duel_autoleave_when_die) {
		if(sd->duel_group > 0)
			duel_leave(sd->duel_group, sd);
		if(sd->duel_invite > 0)
			duel_reject(sd->duel_invite, sd);
	}

	if( sd->skill_keep_using.tid != INVALID_TIMER ){
		delete_timer( sd->skill_keep_using.tid, skill_keep_using );
		sd->skill_keep_using.tid = INVALID_TIMER;
	}

	pc_close_npc(sd,2); //close npc if we were using one

	/* e.g. not killed thru pc_damage */
	if( pc_issit(sd) ) {
		clif_status_load(&sd->bl,EFST_SIT,0);
	}

	pc_setdead(sd);

	clif_party_dead( sd );

	pc_setparam(sd, SP_PCDIECOUNTER, sd->die_counter+1);
	pc_setparam(sd, SP_KILLERRID, src?src->id:0);

	if (battle_config.loose_ap_on_death == 1)
		status_percent_damage( nullptr, &sd->bl, 0, 0, 100, 0 );

	//Reset menu skills/item skills
	if ((sd->skillitem) != 0)
		sd->skillitem = sd->skillitemlv = 0;
	if ((sd->menuskill_id) != 0)
		sd->menuskill_id = sd->menuskill_val = 0;
	//Reset ticks.
	sd->hp_loss.tick = sd->sp_loss.tick = sd->hp_regen.tick = sd->sp_regen.tick = 0;

	if ( sd->spiritball !=0 )
		pc_delspiritball(sd,sd->spiritball,0);
	if (sd->soulball != 0)
		pc_delsoulball(sd, sd->soulball, false);
	if (sd->servantball != 0)
		pc_delservantball( *sd, sd->servantball );
	if (sd->abyssball != 0)
		pc_delabyssball( *sd, sd->abyssball );

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
					clif_name_area(&md->bl);
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
		|| (battle_config.bone_drop==1 && mapdata->flag[MF_PVP]))
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
	    && !sd->sc.cant.deathpenalty
		&& !mapdata->flag[MF_NOEXPPENALTY] && !mapdata_flag_gvg2(mapdata))
	{
		t_exp base_penalty = 0;
		t_exp job_penalty = 0;
		uint32 zeny_penalty = 0;

		if (pc_isvip(sd)) { // EXP penalty for VIP
			base_penalty = battle_config.vip_exp_penalty_base;
			job_penalty = battle_config.vip_exp_penalty_job;
			zeny_penalty = battle_config.vip_zeny_penalty;
		} else {
			base_penalty = battle_config.death_penalty_base;
			job_penalty = battle_config.death_penalty_job;
			zeny_penalty = battle_config.zeny_penalty;
		}

		if ((battle_config.death_penalty_maxlv&1 || !pc_is_maxbaselv(sd)) && base_penalty > 0) {
			switch (battle_config.death_penalty_type) {
				case 1: base_penalty = (t_exp) ( pc_nextbaseexp(sd) * ( base_penalty / 10000. ) ); break;
				case 2: base_penalty = (t_exp) ( sd->status.base_exp * ( base_penalty / 10000. ) ); break;
			}
			if (base_penalty){ //recheck after altering to speedup
				if (battle_config.pk_mode && src && src->type==BL_PC)
					base_penalty *= 2;
				base_penalty = u64min(sd->status.base_exp, base_penalty);
			}
		}
		else 
			base_penalty = 0;

		if ((battle_config.death_penalty_maxlv&2 || !pc_is_maxjoblv(sd)) && job_penalty > 0) {
			switch (battle_config.death_penalty_type) {
				case 1: job_penalty = (uint32) ( pc_nextjobexp(sd) * ( job_penalty / 10000. ) ); break;
				case 2: job_penalty = (uint32) ( sd->status.job_exp * ( job_penalty /10000. ) ); break;
			}
			if (job_penalty) {
				if (battle_config.pk_mode && src && src->type==BL_PC)
					job_penalty *= 2;
				job_penalty = u64min(sd->status.job_exp, job_penalty);
			}
		}
		else
			job_penalty = 0;

		if (base_penalty || job_penalty) {
			short insurance_idx = pc_search_inventory(sd, ITEMID_NEW_INSURANCE);
			if (insurance_idx < 0 || pc_delitem(sd, insurance_idx, 1, 0, 1, LOG_TYPE_CONSUME) != 0)
				pc_lostexp(sd, base_penalty, job_penalty);
		}

		if( zeny_penalty > 0 && !mapdata->flag[MF_NOZENYPENALTY]) {
			zeny_penalty = (uint32)( sd->status.zeny * ( zeny_penalty / 10000. ) );
			if(zeny_penalty)
				pc_payzeny(sd, zeny_penalty, LOG_TYPE_PICKDROP_PLAYER, NULL);
		}
	}

	if( mapdata->flag[MF_PVP_NIGHTMAREDROP] ) { // Moved this outside so it works when PVP isn't enabled and during pk mode [Ancyker]
		for (const auto &it : mapdata->drop_list) {
			int id = it.drop_id, per = it.drop_per;
			enum e_nightmare_drop_type type = it.drop_type;

			if(id == 0)
				continue;
			if(id == -1){
				int eq_num=0,eq_n[MAX_INVENTORY];
				memset(eq_n,0,sizeof(eq_n));
				for(i=0;i<MAX_INVENTORY;i++) {
					if( (type&NMDT_INVENTORY && !sd->inventory.u.items_inventory[i].equip)
						|| (type&NMDT_EQUIP && sd->inventory.u.items_inventory[i].equip)
						||  type&NMDT_ALL)
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
						if(sd->inventory.u.items_inventory[n].equip)
							pc_unequipitem(sd,n,3);
						pc_dropitem(sd,n,1);
					}
				}
			}
			else if(id > 0) {
				for(i=0;i<MAX_INVENTORY;i++){
					if(sd->inventory.u.items_inventory[i].nameid == id
						&& rnd()%10000 < per
						&& ((type&NMDT_INVENTORY && !sd->inventory.u.items_inventory[i].equip)
							|| (type&NMDT_EQUIP && sd->inventory.u.items_inventory[i].equip)
							|| type&NMDT_ALL) ){
						if(sd->inventory.u.items_inventory[i].equip)
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
	if( !battle_config.pk_mode && mapdata->flag[MF_PVP] && !mapdata->flag[MF_PVP_NOCALCRANK] ) {
		sd->pvp_point -= 5;
		sd->pvp_lost++;
		if( src && src->type == BL_PC ) {
			struct map_session_data *ssd = (struct map_session_data *)src;
			ssd->pvp_point++;
			ssd->pvp_won++;
		}
		if( sd->pvp_point < 0 ) {
			sd->respawn_tid = add_timer(tick+1000, pc_respawn_timer,sd->bl.id,0);
			return 1|8;
		}
	}
	//GvG
	if( mapdata_flag_gvg2(mapdata) ) {
		sd->respawn_tid = add_timer(tick+1000, pc_respawn_timer, sd->bl.id, 0);
		return 1|8;
	}
	else if( sd->bg_id ) {
		std::shared_ptr<s_battleground_data> bg = util::umap_find(bg_team_db, sd->bg_id);

		if (bg) {
			if (bg->cemetery.map > 0) { // Respawn by BG
				sd->respawn_tid = add_timer(tick + 1000, pc_respawn_timer, sd->bl.id, 0);
				return 1|8;
			}
		}
	}

	//Reset "can log out" tick.
	if( battle_config.prevent_logout )
		sd->canlog_tick = gettick() - battle_config.prevent_logout;
	return 1;
}

void pc_revive(struct map_session_data *sd,unsigned int hp, unsigned int sp, unsigned int ap) {
	if(hp) clif_updatestatus(sd,SP_HP);
	if(sp) clif_updatestatus(sd,SP_SP);
	if(ap) clif_updatestatus(sd,SP_AP);

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

bool pc_revive_item(struct map_session_data *sd) {
	nullpo_retr(false, sd);

	if (!pc_isdead(sd) || sd->respawn_tid != INVALID_TIMER)
		return false;

	if (sd->sc.data[SC_HELLPOWER]) // Cannot resurrect while under the effect of SC_HELLPOWER.
		return false;

	int16 item_position = itemdb_group.item_exists_pc(sd, IG_TOKEN_OF_SIEGFRIED);
	uint8 hp = 100, sp = 100;

	if (item_position < 0) {
		if (sd->sc.data[SC_LIGHT_OF_REGENE]) {
			hp = sd->sc.data[SC_LIGHT_OF_REGENE]->val2;
			sp = 0;
		}
		else
			return false;
	}

	if (!status_revive(&sd->bl, hp, sp))
		return false;

	if (item_position < 0)
		status_change_end(&sd->bl, SC_LIGHT_OF_REGENE, INVALID_TIMER);
	else
		pc_delitem(sd, item_position, 1, 0, 1, LOG_TYPE_CONSUME);

	clif_skill_nodamage(&sd->bl, &sd->bl, ALL_RESURRECTION, 4, 1);

	return true;
}

// script
//
/*==========================================
 * script reading pc status registry
 *------------------------------------------*/
int64 pc_readparam(struct map_session_data* sd,int64 type)
{
	int64 val = 0;

	nullpo_ret(sd);

	switch(type) {
		case SP_SKILLPOINT:      val = sd->status.skill_point; break;
		case SP_STATUSPOINT:     val = sd->status.status_point; break;
		case SP_TRAITPOINT:      val = sd->status.trait_point; break;
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
		case SP_BASEEXP:         val = u64min(sd->status.base_exp, MAX_EXP); break;
		case SP_JOBEXP:          val = u64min(sd->status.job_exp, MAX_EXP); break;
		case SP_NEXTBASEEXP:     val = u64min(pc_nextbaseexp(sd), MAX_EXP); break;
		case SP_NEXTJOBEXP:      val = u64min(pc_nextjobexp(sd), MAX_EXP); break;
		case SP_HP:              val = sd->battle_status.hp; break;
		case SP_MAXHP:           val = sd->battle_status.max_hp; break;
		case SP_SP:              val = sd->battle_status.sp; break;
		case SP_MAXSP:           val = sd->battle_status.max_sp; break;
		case SP_AP:              val = sd->battle_status.ap; break;
		case SP_MAXAP:           val = sd->battle_status.max_ap; break;
		case SP_STR:             val = sd->status.str; break;
		case SP_AGI:             val = sd->status.agi; break;
		case SP_VIT:             val = sd->status.vit; break;
		case SP_INT:             val = sd->status.int_; break;
		case SP_DEX:             val = sd->status.dex; break;
		case SP_LUK:             val = sd->status.luk; break;
		case SP_POW:             val = sd->status.pow; break;
		case SP_STA:             val = sd->status.sta; break;
		case SP_WIS:             val = sd->status.wis; break;
		case SP_SPL:             val = sd->status.spl; break;
		case SP_CON:             val = sd->status.con; break;
		case SP_CRT:             val = sd->status.crt; break;
		case SP_KARMA:           val = sd->status.karma; break;
		case SP_MANNER:          val = sd->status.manner; break;
		case SP_FAME:            val = sd->status.fame; break;
		case SP_KILLERRID:       val = sd->killerrid; break;
		case SP_KILLEDRID:       val = sd->killedrid; break;
		case SP_KILLEDGID:       val = sd->killedgid; break;
		case SP_SITTING:         val = pc_issit(sd)?1:0; break;
		case SP_CHARMOVE:		 val = sd->status.character_moves; break;
		case SP_CHARRENAME:		 val = sd->status.rename; break;
		case SP_CHARFONT:		 val = sd->status.font; break;
		case SP_BANK_VAULT:      val = sd->bank_vault; break;
		case SP_CASHPOINTS:      val = sd->cashPoints; break;
		case SP_KAFRAPOINTS:     val = sd->kafraPoints; break;
		case SP_ROULETTE_BRONZE: val = sd->roulette_point.bronze; break;
		case SP_ROULETTE_SILVER: val = sd->roulette_point.silver; break;
		case SP_ROULETTE_GOLD:   val = sd->roulette_point.gold; break;
		case SP_PCDIECOUNTER:    val = sd->die_counter; break;
		case SP_COOKMASTERY:     val = sd->cook_mastery; break;
		case SP_ACHIEVEMENT_LEVEL: val = sd->achievement_data.level; break;
		case SP_CRITICAL:        val = sd->battle_status.cri/10; break;
		case SP_ASPD:            val = (2000-sd->battle_status.amotion)/10; break;
		case SP_BASE_ATK:
#ifdef RENEWAL
			val = sd->bonus.eatk;
#else
			val = sd->battle_status.batk;
#endif
			break;
		case SP_DEF1:		     val = sd->battle_status.def; break;
		case SP_DEF2:		     val = sd->battle_status.def2; break;
		case SP_MDEF1:		     val = sd->battle_status.mdef; break;
		case SP_MDEF2:		     val = sd->battle_status.mdef2; break;
		case SP_HIT:		     val = sd->battle_status.hit; break;
		case SP_FLEE1:		     val = sd->battle_status.flee; break;
		case SP_FLEE2:		     val = sd->battle_status.flee2; break;
		case SP_PATK:		     val = sd->battle_status.patk; break;
		case SP_SMATK:		     val = sd->battle_status.smatk; break;
		case SP_RES:		     val = sd->battle_status.res; break;
		case SP_MRES:		     val = sd->battle_status.mres; break;
		case SP_HPLUS:		     val = sd->battle_status.hplus; break;
		case SP_CRATE:		     val = sd->battle_status.crate; break;
		case SP_DEFELE:		     val = sd->battle_status.def_ele; break;
		case SP_MAXHPRATE:	     val = sd->hprate; break;
		case SP_MAXSPRATE:	     val = sd->sprate; break;
		case SP_MAXAPRATE:	     val = sd->aprate; break;
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
		case SP_PATK_RATE:       val = sd->patk_rate; break;
		case SP_SMATK_RATE:      val = sd->smatk_rate; break;
		case SP_RES_RATE:        val = sd->res_rate; break;
		case SP_MRES_RATE:       val = sd->mres_rate; break;
		case SP_HPLUS_RATE:      val = sd->hplus_rate; break;
		case SP_CRATE_RATE:      val = sd->crate_rate; break;
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
		case SP_NO_MADO_FUEL:    val = sd->special_state.no_mado_fuel?1:0; break;
		case SP_NO_WALK_DELAY:   val = sd->special_state.no_walk_delay?1:0; break;
		case SP_SPLASH_RANGE:    val = sd->bonus.splash_range; break;
		case SP_SPLASH_ADD_RANGE:val = sd->bonus.splash_add_range; break;
		case SP_SHORT_WEAPON_DAMAGE_RETURN: val = sd->bonus.short_weapon_damage_return; break;
		case SP_LONG_WEAPON_DAMAGE_RETURN: val = sd->bonus.long_weapon_damage_return; break;
		case SP_MAGIC_DAMAGE_RETURN: val = sd->bonus.magic_damage_return; break;
		case SP_REDUCE_DAMAGE_RETURN: val = sd->bonus.reduce_damage_return; break;
		case SP_PERFECT_HIDE:    val = sd->special_state.perfect_hiding?1:0; break;
		case SP_UNBREAKABLE:     val = sd->bonus.unbreakable; break;
		case SP_UNBREAKABLE_WEAPON: val = (sd->bonus.unbreakable_equip&EQP_WEAPON)?1:0; break;
		case SP_UNBREAKABLE_ARMOR: val = (sd->bonus.unbreakable_equip&EQP_ARMOR)?1:0; break;
		case SP_UNBREAKABLE_HELM: val = (sd->bonus.unbreakable_equip&EQP_HELM)?1:0; break;
		case SP_UNBREAKABLE_SHIELD: val = (sd->bonus.unbreakable_equip&EQP_SHIELD)?1:0; break;
		case SP_UNBREAKABLE_GARMENT: val = (sd->bonus.unbreakable_equip&EQP_GARMENT)?1:0; break;
		case SP_UNBREAKABLE_SHOES: val = (sd->bonus.unbreakable_equip&EQP_SHOES)?1:0; break;
		case SP_CLASSCHANGE:     val = sd->bonus.classchange; break;
		case SP_SHORT_ATK_RATE:  val = sd->bonus.short_attack_atk_rate; break;
		case SP_LONG_ATK_RATE:   val = sd->bonus.long_attack_atk_rate; break;
		case SP_BREAK_WEAPON_RATE: val = sd->bonus.break_weapon_rate; break;
		case SP_BREAK_ARMOR_RATE: val = sd->bonus.break_armor_rate; break;
		case SP_ADD_STEAL_RATE:  val = sd->bonus.add_steal_rate; break;
		case SP_DELAYRATE:       val = sd->bonus.delayrate; break;
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
		case SP_LONG_SP_GAIN_VALUE:   val = sd->bonus.long_sp_gain_value; break;
		case SP_LONG_HP_GAIN_VALUE:   val = sd->bonus.long_hp_gain_value; break;
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
		case SP_CRIT_DEF_RATE: val = sd->bonus.crit_def_rate; break;
		case SP_ADD_ITEM_SPHEAL_RATE: val = sd->bonus.itemsphealrate2; break;
		default:
			ShowError("pc_readparam: Attempt to read unknown parameter '%lld'.\n", type);
			return -1;
	}

	return val;
}

/*==========================================
 * script set pc status registry
 *------------------------------------------*/
bool pc_setparam(struct map_session_data *sd,int64 type,int64 val_tmp)
{
	nullpo_retr(false,sd);

	int val = static_cast<unsigned int>(val_tmp);

	switch(type){
	case SP_BASELEVEL:
		if (val > pc_maxbaselv(sd)) //Capping to max
			val = pc_maxbaselv(sd);
		if (val > sd->status.base_level) {
			for( int i = 0; i < (int)( val - sd->status.base_level ); i++ ){
				sd->status.status_point += statpoint_db.pc_gets_status_point( sd->status.base_level + i );
				sd->status.trait_point += statpoint_db.pc_gets_trait_point( sd->status.base_level + i );
			}
		}
		sd->status.base_level = val;
		sd->status.base_exp = 0;
		// clif_updatestatus(sd, SP_BASELEVEL);  // Gets updated at the bottom
		clif_updatestatus(sd, SP_NEXTBASEEXP);
		clif_updatestatus(sd, SP_STATUSPOINT);
		clif_updatestatus(sd, SP_TRAITPOINT);
		clif_updatestatus(sd, SP_BASEEXP);
		status_calc_pc(sd, SCO_FORCE);
		if(sd->status.party_id)
			party_send_levelup(sd);
		break;
	case SP_JOBLEVEL:
		if (val >= sd->status.job_level) {
			if (val > pc_maxjoblv(sd)) val = pc_maxjoblv(sd);
			sd->status.skill_point += val - sd->status.job_level;
			clif_updatestatus(sd, SP_SKILLPOINT);
		}
		sd->status.job_level = val;
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
	case SP_TRAITPOINT:
		sd->status.trait_point = val;
		break;
	case SP_ZENY:
		if( val < 0 )
			return false;// can't set negative zeny
		log_zeny(sd, LOG_TYPE_SCRIPT, sd, -(sd->status.zeny - cap_value(val, 0, MAX_ZENY)));
		sd->status.zeny = cap_value(val, 0, MAX_ZENY);
		break;
	case SP_BASEEXP:
		val_tmp = cap_value(val_tmp, 0, pc_is_maxbaselv(sd) ? MAX_LEVEL_BASE_EXP : MAX_EXP);
		if (val_tmp < sd->status.base_exp) // Lost
			pc_lostexp(sd, sd->status.base_exp - val_tmp, 0);
		else // Gained
			pc_gainexp(sd, NULL, val_tmp - sd->status.base_exp, 0, 2);
		return true;
	case SP_JOBEXP:
		val_tmp = cap_value(val_tmp, 0, pc_is_maxjoblv(sd) ? MAX_LEVEL_JOB_EXP : MAX_EXP);
		if (val_tmp < sd->status.job_exp) // Lost
			pc_lostexp(sd, 0, sd->status.job_exp - val_tmp);
		else // Gained
			pc_gainexp(sd, NULL, 0, val_tmp - sd->status.job_exp, 2);
		return true;
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
		if (sd->status.base_level < 100)
			sd->battle_status.max_hp = cap_value(val, 1, battle_config.max_hp_lv99);
		else if (sd->status.base_level < 151)
			sd->battle_status.max_hp = cap_value(val, 1, battle_config.max_hp_lv150);
		else
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
	case SP_AP:
		sd->battle_status.ap = cap_value(val, 0, (int)sd->battle_status.max_ap);
		break;
	case SP_MAXAP:
		sd->battle_status.max_ap = cap_value(val, 1, battle_config.max_ap);

		if (sd->battle_status.max_ap < sd->battle_status.ap) {
			sd->battle_status.ap = sd->battle_status.max_ap;
			clif_updatestatus(sd, SP_AP);
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
	case SP_POW:
		sd->status.pow = cap_value(val, 0, pc_maxparameter(sd,PARAM_POW));
		break;
	case SP_STA:
		sd->status.sta = cap_value(val, 0, pc_maxparameter(sd,PARAM_STA));
		break;
	case SP_WIS:
		sd->status.wis = cap_value(val, 0, pc_maxparameter(sd,PARAM_WIS));
		break;
	case SP_SPL:
		sd->status.spl = cap_value(val, 0, pc_maxparameter(sd,PARAM_SPL));
		break;
	case SP_CON:
		sd->status.con = cap_value(val, 0, pc_maxparameter(sd,PARAM_CON));
		break;
	case SP_CRT:
		sd->status.crt = cap_value(val, 0, pc_maxparameter(sd,PARAM_CRT));
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
	case SP_KILLEDGID:
		sd->killedgid = val;
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
	case SP_CASHPOINTS:
		if (val < 0)
			return false;
		if (!sd->state.connect_new)
			log_cash(sd, LOG_TYPE_SCRIPT, LOG_CASH_TYPE_CASH, -(sd->cashPoints - cap_value(val, 0, MAX_ZENY)));
		sd->cashPoints = cap_value(val, 0, MAX_ZENY);
		pc_setaccountreg(sd, add_str(CASHPOINT_VAR), sd->cashPoints);
		return true;
	case SP_KAFRAPOINTS:
		if (val < 0)
			return false;
		if (!sd->state.connect_new)
			log_cash(sd, LOG_TYPE_SCRIPT, LOG_CASH_TYPE_KAFRA, -(sd->kafraPoints - cap_value(val, 0, MAX_ZENY)));
		sd->kafraPoints = cap_value(val, 0, MAX_ZENY);
		pc_setaccountreg(sd, add_str(KAFRAPOINT_VAR), sd->kafraPoints);
		return true;
	case SP_PCDIECOUNTER:
		if (val < 0)
			return false;
		if (sd->die_counter == val)
			return true;
		sd->die_counter = val;
		if (!sd->state.connect_new && sd->die_counter == 1 && (sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE)
			status_calc_pc(sd, SCO_NONE); // Lost the bonus.
		pc_setglobalreg(sd, add_str(PCDIECOUNTER_VAR), sd->die_counter);
		return true;
	case SP_COOKMASTERY:
		if (sd->cook_mastery == val)
			return true;
		val = cap_value(val, 0, 1999);
		sd->cook_mastery = val;
		pc_setglobalreg(sd, add_str(COOKMASTERY_VAR), sd->cook_mastery);
		return true;
	default:
		ShowError("pc_setparam: Attempted to set unknown parameter '%lld'.\n", type);
		return false;
	}
	clif_updatestatus(sd,static_cast<int>(type));

	return true;
}

/*==========================================
 * HP/SP/AP Healing. If flag is passed, the heal type is through clif_heal, otherwise update status.
 *------------------------------------------*/
void pc_heal(struct map_session_data *sd,unsigned int hp,unsigned int sp, unsigned int ap, int type)
{// Is there going to be a effect for gaining AP soon??? [Rytech]
	nullpo_retv(sd);

	if (type&2) {
		if (hp || type&4) {
			clif_heal(sd->fd,SP_HP,hp);
			clif_update_hp(*sd);
		}
		if (sp)
			clif_heal(sd->fd,SP_SP,sp);
		if (ap)
			clif_heal(sd->fd,SP_AP,ap);
	} else {
		if(hp)
			clif_updatestatus(sd,SP_HP);
		if(sp)
			clif_updatestatus(sd,SP_SP);
		if (ap)
			clif_updatestatus(sd,SP_AP);
	}
	return;
}

/**
 * Heal player HP and/or SP linearly. Calculate any bonus based on active statuses.
 * @param sd: Player data
 * @param itemid: Item ID
 * @param hp: HP to heal
 * @param sp: SP to heal
 * @return Amount healed to an object
 */
int pc_itemheal(struct map_session_data *sd, t_itemid itemid, int hp, int sp)
{
	int bonus, tmp, penalty = 0;

	if (hp) {
		bonus = 100 + (sd->battle_status.vit << 1) + pc_checkskill(sd, SM_RECOVERY) * 10 + pc_checkskill(sd, AM_LEARNINGPOTION) * 5;
		// A potion produced by an Alchemist in the Fame Top 10 gets +50% effect [DracoRPG]
		if (potion_flag == 2) {
			bonus += bonus * 50 / 100;
			if (sd->sc.data[SC_SPIRIT] && sd->sc.data[SC_SPIRIT]->val2 == SL_ROGUE)
				bonus += bonus; // Receive an additional +100% effect from ranked potions to HP only
		}
		//All item bonuses.
		bonus += sd->bonus.itemhealrate2;
		//Item Group bonuses
		bonus += bonus * pc_get_itemgroup_bonus(sd, itemid, sd->itemgrouphealrate) / 100;
		//Individual item bonuses.
		for(const auto &it : sd->itemhealrate) {
			if (it.id == itemid) {
				bonus += bonus * it.val / 100;
				break;
			}
		}
		// Recovery Potion
		if (sd->sc.data[SC_INCHEALRATE])
			bonus += bonus * sd->sc.data[SC_INCHEALRATE]->val1 / 100;
		// 2014 Halloween Event : Pumpkin Bonus
		if (sd->sc.data[SC_MTF_PUMPKIN]) {
			if (itemid == ITEMID_PUMPKIN)
				bonus += bonus * sd->sc.data[SC_MTF_PUMPKIN]->val1 / 100;
			else if (itemid == ITEMID_COOKIE_BAT)
				bonus += sd->sc.data[SC_MTF_PUMPKIN]->val2;
		}

		tmp = hp * bonus / 100; // Overflow check
		if (bonus != 100 && tmp > hp)
			hp = tmp;
	}
	if (sp) {
		bonus = 100 + (sd->battle_status.int_ << 1) + pc_checkskill(sd, MG_SRECOVERY) * 10 + pc_checkskill(sd, AM_LEARNINGPOTION) * 5;
		// A potion produced by an Alchemist in the Fame Top 10 gets +50% effect [DracoRPG]
		if (potion_flag == 2)
			bonus += bonus * 50 / 100;

		// All item bonuses.
		bonus += sd->bonus.itemsphealrate2;
		// Item Group bonuses
		bonus += bonus * pc_get_itemgroup_bonus( sd, itemid, sd->itemgroupsphealrate ) / 100;
		// Individual item bonuses.
		for( const auto &it : sd->itemsphealrate ){
			if( it.id == itemid ){
				bonus += bonus * it.val / 100;
				break;
			}
		}

		tmp = sp * bonus / 100; // Overflow check
		if (bonus != 100 && tmp > sp)
			sp = tmp;
	}
	if (sd->sc.count) {
		// Critical Wound and Death Hurt stack
		if (sd->sc.data[SC_CRITICALWOUND])
			penalty += sd->sc.data[SC_CRITICALWOUND]->val2;

		if (sd->sc.data[SC_DEATHHURT] && sd->sc.data[SC_DEATHHURT]->val3 == 1)
			penalty += 20;

		if (sd->sc.data[SC_NORECOVER_STATE])
			penalty = 100;

		if (sd->sc.data[SC_VITALITYACTIVATION])
			hp += hp / 2; // 1.5 times

		if (sd->sc.data[SC_WATER_INSIGNIA] && sd->sc.data[SC_WATER_INSIGNIA]->val1 == 2) {
			hp += hp / 10;
			sp += sp / 10;
		}

#ifdef RENEWAL
		if (sd->sc.data[SC_APPLEIDUN])
			hp += sd->sc.data[SC_APPLEIDUN]->val3 / 100;
#endif

		if (penalty > 0) {
			hp -= hp * penalty / 100;
			sp -= sp * penalty / 100;
		}

#ifdef RENEWAL
		if (sd->sc.data[SC_EXTREMITYFIST2])
			sp = 0;
#endif
		if (sd->sc.data[SC_BITESCAR])
			hp = 0;
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

	nullpo_retr(false,sd);

	if (job < 0)
		return false;

	//Normalize job.
	uint64 b_class = pc_jobid2mapid(job);
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

	// If the job does not exist in the job db, dont allow changing to it
	if( !job_db.exists( job ) ){
		return false;
	}

	if( ( b_class&JOBL_FOURTH ) && !( sd->class_&JOBL_FOURTH ) ){
		// Changing to 4th job
		sd->change_level_4th = sd->status.job_level;
		pc_setglobalreg( sd, add_str( JOBCHANGE4TH_VAR ) , sd->change_level_4th );
	}else if( ( b_class&JOBL_THIRD ) && !( sd->class_&JOBL_THIRD ) ){
		// changing from 2nd to 3rd job
		sd->change_level_3rd = sd->status.job_level;
		pc_setglobalreg( sd, add_str( JOBCHANGE3RD_VAR ), sd->change_level_3rd );
	}else if( ( b_class&JOBL_2 ) && !( sd->class_&JOBL_2 ) ){
		// changing from 1st to 2nd job
		sd->change_level_2nd = sd->status.job_level;
		pc_setglobalreg( sd, add_str( JOBCHANGE2ND_VAR ), sd->change_level_2nd );
	}

	if(sd->cloneskill_idx > 0) {
		if( sd->status.skill[sd->cloneskill_idx].flag == SKILL_FLAG_PLAGIARIZED ) {
			sd->status.skill[sd->cloneskill_idx].id = 0;
			sd->status.skill[sd->cloneskill_idx].lv = 0;
			sd->status.skill[sd->cloneskill_idx].flag = SKILL_FLAG_PERMANENT;
			clif_deleteskill(sd, static_cast<int>(pc_readglobalreg(sd, add_str(SKILL_VAR_PLAGIARISM))));
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
			clif_deleteskill(sd, static_cast<int>(pc_readglobalreg(sd, add_str(SKILL_VAR_REPRODUCE))));
		}
		sd->reproduceskill_idx = 0;
		pc_setglobalreg(sd, add_str(SKILL_VAR_REPRODUCE), 0);
		pc_setglobalreg(sd, add_str(SKILL_VAR_REPRODUCE_LV), 0);
	}

	if ( (b_class&MAPID_UPPERMASK) != (sd->class_&MAPID_UPPERMASK) ) { //Things to remove when changing class tree.
		status_db.changeSkillTree(sd);
	}

	if( (sd->class_&MAPID_UPPERMASK) == MAPID_STAR_GLADIATOR && (b_class&MAPID_UPPERMASK) != MAPID_STAR_GLADIATOR) {
		/* going off star glad lineage, reset feel and hate to not store no-longer-used vars in the database */
		pc_resetfeel(sd);
		pc_resethate(sd);
	}

	// Reset body style to 0 before changing job to avoid
	// errors since not every job has a alternate outfit.
	sd->status.body = 0;
	clif_changelook(&sd->bl,LOOK_BODY2,0);

	sd->status.class_ = job;
	fame_flag = pc_famerank(sd->status.char_id,sd->class_&MAPID_UPPERMASK);
	uint64 previous_class = sd->class_;
	sd->class_ = (unsigned short)b_class;
	sd->status.job_level=1;
	sd->status.job_exp=0;

	if (sd->status.base_level > pc_maxbaselv(sd)) {
		sd->status.base_level = pc_maxbaselv(sd);
		sd->status.base_exp=0;
		pc_resetstate(sd);
		clif_updatestatus(sd,SP_STATUSPOINT);
		clif_updatestatus(sd,SP_TRAITPOINT);
		clif_updatestatus(sd,SP_BASELEVEL);
		clif_updatestatus(sd,SP_BASEEXP);
		clif_updatestatus(sd,SP_NEXTBASEEXP);
	}

	// Give or reduce transcendent status points
	if( (b_class&JOBL_UPPER) && !(previous_class&JOBL_UPPER) ){ // Change from a non t class to a t class -> give points
		sd->status.status_point += battle_config.transcendent_status_points;
		clif_updatestatus(sd,SP_STATUSPOINT);
	}else if( !(b_class&JOBL_UPPER) && (previous_class&JOBL_UPPER) ){ // Change from a t class to a non t class -> remove points
		if( sd->status.status_point < battle_config.transcendent_status_points ){
			// The player already used his bonus points, so we have to reset his status points
			pc_resetstate(sd);
		}else{
			sd->status.status_point -= battle_config.transcendent_status_points;
			clif_updatestatus(sd,SP_STATUSPOINT);
		}
	}

	// Give or reduce trait status points
	if ((b_class & JOBL_FOURTH) && !(previous_class & JOBL_FOURTH)) {// Change to a 4th job.
		sd->status.trait_point += battle_config.trait_points_job_change;
		clif_updatestatus(sd, SP_TRAITPOINT);
		clif_updatestatus(sd, SP_UPOW);
		clif_updatestatus(sd, SP_USTA);
		clif_updatestatus(sd, SP_UWIS);
		clif_updatestatus(sd, SP_USPL);
		clif_updatestatus(sd, SP_UCON);
		clif_updatestatus(sd, SP_UCRT);
	} else if (!(b_class & JOBL_FOURTH) && (previous_class & JOBL_FOURTH)) {// Change to a non 4th job.
		if (sd->status.trait_point < battle_config.trait_points_job_change) {
			// Player may have already used the trait status points. Force a reset.
			pc_resetstate(sd);
		} else {
			sd->status.trait_point = 0;
			clif_updatestatus(sd, SP_TRAITPOINT);
			clif_updatestatus(sd, SP_UPOW);
			clif_updatestatus(sd, SP_USTA);
			clif_updatestatus(sd, SP_UWIS);
			clif_updatestatus(sd, SP_USPL);
			clif_updatestatus(sd, SP_UCON);
			clif_updatestatus(sd, SP_UCRT);
		}
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
#if PACKETVER >= 20151001
	clif_changelook(&sd->bl, LOOK_HAIR, sd->vd.hair_style); // Update player's head (only matters when switching to or from Doram)
#endif
	if(sd->vd.cloth_color)
		clif_changelook(&sd->bl,LOOK_CLOTHES_COLOR,sd->vd.cloth_color);
	/*
	if(sd->vd.body_style)
		clif_changelook(&sd->bl,LOOK_BODY2,sd->vd.body_style);
	*/
	//Update skill tree.
	pc_calc_skilltree(sd);
	clif_skillinfoblock(sd);

	if (sd->ed)
		elemental_delete(sd->ed);
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

	if (sd->sc.data[SC_SPRITEMABLE] && !pc_checkskill(sd, SU_SPRITEMABLE))
		status_change_end(&sd->bl, SC_SPRITEMABLE, INVALID_TIMER);
	if (sd->sc.data[SC_SOULATTACK] && !pc_checkskill(sd, SU_SOULATTACK))
		status_change_end(&sd->bl, SC_SOULATTACK, INVALID_TIMER);
	if( sd->sc.data[SC_SPIRIT] ){
		status_change_end( &sd->bl, SC_SPIRIT, INVALID_TIMER );
	}

	if(sd->status.manner < 0)
		clif_changestatus(sd,SP_MANNER,sd->status.manner);

	status_calc_pc(sd,SCO_FORCE);
	pc_checkallowskill(sd);
	pc_equiplookall(sd);
	pc_show_questinfo(sd);
	achievement_update_objective(sd, AG_JOB_CHANGE, 2, sd->status.base_level, job);
	if( sd->status.party_id ){
		struct party_data* p;
		
		if( ( p = party_search( sd->status.party_id ) ) != NULL ){
			ARR_FIND(0, MAX_PARTY, i, p->party.member[i].char_id == sd->status.char_id);

			if( i < MAX_PARTY ){
				p->party.member[i].class_ = sd->status.class_;
				clif_party_job_and_level(sd);
			}
		}
	}

	chrif_save(sd, CSAVE_NORMAL);
	//if you were previously famous, not anymore.
	if (fame_flag)
		chrif_buildfamelist();
	else if (sd->status.fame > 0) {
		//It may be that now they are famous?
 		switch (sd->class_&MAPID_UPPERMASK) {
			case MAPID_BLACKSMITH:
			case MAPID_ALCHEMIST:
			case MAPID_TAEKWON:
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
void pc_changelook(struct map_session_data *sd,int type,int val) {
	nullpo_retv(sd);

	switch(type) {
	case LOOK_HAIR:	//Use the battle_config limits! [Skotlex]
		val = cap_value(val, MIN_HAIR_STYLE, MAX_HAIR_STYLE);

		if (sd->status.hair != val) {
			sd->status.hair = val;
			if (sd->status.guild_id) //Update Guild Window. [Skotlex]
				intif_guild_change_memberinfo(sd->status.guild_id, sd->status.account_id, sd->status.char_id,
				GMI_HAIR, &sd->status.hair, sizeof(sd->status.hair));
		}
		break;
	case LOOK_WEAPON:
		sd->status.weapon = val;
		break;
	case LOOK_HEAD_BOTTOM:
		sd->status.head_bottom = val;
		sd->setlook_head_bottom = val;
		break;
	case LOOK_HEAD_TOP:
		sd->status.head_top = val;
		sd->setlook_head_top = val;
		break;
	case LOOK_HEAD_MID:
		sd->status.head_mid = val;
		sd->setlook_head_mid = val;
		break;
	case LOOK_HAIR_COLOR:	//Use the battle_config limits! [Skotlex]
		val = cap_value(val, MIN_HAIR_COLOR, MAX_HAIR_COLOR);

		if (sd->status.hair_color != val) {
			sd->status.hair_color = val;
			if (sd->status.guild_id) //Update Guild Window. [Skotlex]
				intif_guild_change_memberinfo(sd->status.guild_id, sd->status.account_id, sd->status.char_id,
				GMI_HAIR_COLOR, &sd->status.hair_color, sizeof(sd->status.hair_color));
		}
		break;
	case LOOK_CLOTHES_COLOR:	//Use the battle_config limits! [Skotlex]
		val = cap_value(val, MIN_CLOTH_COLOR, MAX_CLOTH_COLOR);

		sd->status.clothes_color = val;
		break;
	case LOOK_SHIELD:
		sd->status.shield = val;
		break;
	case LOOK_SHOES:
		break;
	case LOOK_ROBE:
		sd->status.robe = val;
		sd->setlook_robe = val;
		break;
	case LOOK_BODY2:
		val = cap_value(val, MIN_BODY_STYLE, MAX_BODY_STYLE);

		sd->status.body = val;
		break;
	}
	clif_changelook(&sd->bl, type, val);
}

/*==========================================
 * Give an option (type) to player (sd) and display it to client
 *------------------------------------------*/
void pc_setoption(struct map_session_data *sd,int type, int subtype)
{
	int p_type, new_look=0;
	nullpo_retv(sd);
	p_type = sd->sc.option;

	//Option has to be changed client-side before the class sprite or it won't always work (eg: Wedding sprite) [Skotlex]
	sd->sc.option=type;
	clif_changeoption(&sd->bl);

	if( (type&OPTION_RIDING && !(p_type&OPTION_RIDING)) || (type&OPTION_DRAGON && !(p_type&OPTION_DRAGON) && pc_checkskill(sd,RK_DRAGONTRAINING) > 0) )
	{ // Mounting
		clif_status_load(&sd->bl,EFST_RIDING,1);
		status_calc_pc(sd,SCO_NONE);
	}
	else if( (!(type&OPTION_RIDING) && p_type&OPTION_RIDING) || (!(type&OPTION_DRAGON) && p_type&OPTION_DRAGON && pc_checkskill(sd,RK_DRAGONTRAINING) > 0) )
	{ // Dismount
		clif_status_load(&sd->bl,EFST_RIDING,0);
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
		clif_status_load(&sd->bl,EFST_FALCON,1);
	else if (!(type&OPTION_FALCON) && p_type&OPTION_FALCON) //Falcon OFF
		clif_status_load(&sd->bl,EFST_FALCON,0);

	if( type&OPTION_WUGRIDER && !(p_type&OPTION_WUGRIDER) ) { // Mounting
		clif_status_load(&sd->bl,EFST_WUGRIDER,1);
		status_calc_pc(sd,SCO_NONE);
	} else if( !(type&OPTION_WUGRIDER) && p_type&OPTION_WUGRIDER ) { // Dismount
		clif_status_load(&sd->bl,EFST_WUGRIDER,0);
		status_calc_pc(sd,SCO_NONE);
	}

	if( type&OPTION_MADOGEAR && !(p_type&OPTION_MADOGEAR) ) {
		sc_start(&sd->bl, &sd->bl, SC_MADOGEAR, 100, subtype, INFINITE_TICK);
	} else if( !(type&OPTION_MADOGEAR) && p_type&OPTION_MADOGEAR ) {
		status_change_end(&sd->bl, SC_MADOGEAR, INVALID_TIMER);
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
	if( sd->vd.body_style )
		clif_changelook(&sd->bl,LOOK_BODY2,sd->vd.body_style);
	clif_skillinfoblock(sd); // Skill list needs to be updated after base change.
}

/**
 * Give player a cart
 * @param sd Player
 * @param type 0:Remove cart, 1 ~ MAX_CARTS: Cart type
 **/
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
			if( !sd->sc.data[SC_PUSH_CART] ) { /* first time, so fill cart data */
				clif_cartlist(sd);
				status_calc_cart_weight(sd, (e_status_calc_weight_opt)(CALCWT_ITEM|CALCWT_MAXBONUS|CALCWT_CARTSTATE));
			}
			clif_updatestatus(sd, SP_CARTINFO);
			sc_start(&sd->bl, &sd->bl, SC_PUSH_CART, 100, type, 0);
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
void pc_setfalcon(struct map_session_data* sd, int flag)
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
void pc_setriding(struct map_session_data* sd, int flag)
{
	if( sd->sc.data[SC_ALL_RIDING] )
		return;

	if( flag ){
		if( pc_checkskill(sd,KN_RIDING) > 0 ) // add peco
			pc_setoption(sd, sd->sc.option|OPTION_RIDING);
	} else if( pc_isriding(sd) ){
			pc_setoption(sd, sd->sc.option&~OPTION_RIDING);
	}
}

/**
 * Give player a mado
 * @param sd: Player
 * @param flag: Enable or disable mado
 * @param type: See pc.hpp::e_mado_type (Default is MADO_ROBOT)
 */
void pc_setmadogear(struct map_session_data *sd, bool flag, e_mado_type type)
{
	if ((sd->class_ & MAPID_THIRDMASK) != MAPID_MECHANIC)
		return;

	if (flag) {
		if (pc_checkskill(sd, NC_MADOLICENCE) > 0)
			pc_setoption(sd, sd->sc.option | OPTION_MADOGEAR, type);
	} else if (pc_ismadogear(sd)) {
		pc_setoption(sd, sd->sc.option & ~OPTION_MADOGEAR);
	}
}

/*==========================================
 * Check if player can drop an item
 *------------------------------------------*/
bool pc_candrop(struct map_session_data *sd, struct item *item)
{
	if( item && ((item->expire_time || (item->bound && !pc_can_give_bounded_items(sd))) || (itemdb_ishatched_egg(item))) )
		return false;
	if( !pc_can_give_items(sd) || sd->sc.cant.drop) //check if this GM level can drop items
		return false;
	return (itemdb_isdropable(item, pc_get_group_level(sd)));
}

/*==========================================
 * Read '@type' variables (temporary numeric char reg)
 *------------------------------------------*/
int64 pc_readreg(struct map_session_data* sd, int64 reg)
{
	return i64db_i64get(sd->regs.vars, reg);
}

/*==========================================
 * Set '@type' variables (temporary numeric char reg)
 *------------------------------------------*/
bool pc_setreg(struct map_session_data* sd, int64 reg, int64 val)
{
	uint32 index = script_getvaridx(reg);

	nullpo_retr(false, sd);

	if( val ) {
		i64db_i64put(sd->regs.vars, reg, val);
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
int64 pc_readregistry(struct map_session_data *sd, int64 reg)
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
bool pc_setregistry(struct map_session_data *sd, int64 reg, int64 val)
{
	struct script_reg_num *p = NULL;
	const char *regname = get_str(script_getvarid(reg));
	uint32 index = script_getvaridx(reg);

	if ( !reg_load && !sd->vars_ok ) {
		ShowError("pc_setregistry : refusing to set %s until vars are received.\n", regname);
		return false;
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

	return true;
}

/**
 * Serves the following variable types:
 * - 'type$' (permanent str char reg)
 * - '#type$' (permanent str account reg)
 * - '##type$' (permanent str account reg2)
 **/
bool pc_setregistry_str(struct map_session_data *sd, int64 reg, const char *val)
{
	struct script_reg_str *p = NULL;
	const char *regname = get_str(script_getvarid(reg));
	unsigned int index = script_getvaridx(reg);
	size_t vlen = 0;

	if (!reg_load && !sd->vars_ok) {
		ShowError("pc_setregistry_str : refusing to set %s until vars are received.\n", regname);
		return false;
	}

	if ( !script_check_RegistryVariableLength(1, val, &vlen ) )
	{
		ShowError("pc_check_RegistryVariableLength: Variable value length is too long (aid: %d, cid: %d): '%s' sz=%zu\n", sd->status.account_id, sd->status.char_id, val, vlen);
		return false;
	}

	if( (p = (struct script_reg_str *)i64db_get(sd->regs.vars, reg) ) ) {
		if( val[0] ) {
			if( p->value )
				aFree(p->value);
			else if ( index ) // an entry that was deleted, so we reset
				script_array_update(&sd->regs, reg, false);
			p->value = aStrdup(val);
		} else {
			if (p->value)
				aFree(p->value);
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

	return true;
}

/**
 * Set value of player variable
 * @param sd Player
 * @param reg Variable name
 * @param value
 * @return True if success, false if failed.
 **/
bool pc_setreg2(struct map_session_data *sd, const char *reg, int64 val) {
	char prefix = reg[0];

	nullpo_retr(false, sd);

	if (reg[strlen(reg)-1] == '$') {
		ShowError("pc_setreg2: Invalid variable scope '%s'\n", reg);
		return false;
	}

	val = cap_value(val, INT_MIN, INT_MAX);

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
int64 pc_readreg2(struct map_session_data *sd, const char *reg) {
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
static TIMER_FUNC(pc_eventtimer){
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
				addt_tickimer(sd->eventtimer[i],tick);
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
 * @param sd: Player data
 * @param data: Item data
 * @return Number of succeeded combo(s)
 */
static int pc_checkcombo(struct map_session_data *sd, item_data *data) {
	int success = 0;

	for (const auto &item_combo : data->combos) {
		bool do_continue = false;

		// Ensure this isn't a duplicate combo
		for (const auto player_combo : sd->combos) {
			if (player_combo->id == item_combo->id) {
				do_continue = true;
				break;
			}
		}

		// Combo already equipped
		if (do_continue)
			continue;

		size_t nb_itemCombo = item_combo->nameid.size();

		if (nb_itemCombo < 2) // A combo with less then 2 item?
			continue;

		struct s_itemchk {
			int idx;
			t_itemid nameid, card[MAX_SLOTS];

			s_itemchk() : idx(0), nameid(0), card() {};
		};
		std::vector<s_itemchk> combo_idx(nb_itemCombo);
		size_t j;
		unsigned int pos = 0;

		for (j = 0; j < nb_itemCombo; j++) {
			t_itemid id = item_combo->nameid[j];
			bool found = false;

			for (int16 k = 0; k < EQI_MAX; k++) {
				short index = sd->equip_index[k];

				if (index < 0)
					continue;
				if (pc_is_same_equip_index((equip_index)k, sd->equip_index, index))
					continue;
				if (!sd->inventory_data[index])
					continue;

				if (itemdb_type(id) != IT_CARD) {
					if (sd->inventory_data[index]->nameid != id)
						continue;

					if (j > 0) { // Check if this item not already used
						do_continue = false;

						for (size_t z = 0; z < nb_itemCombo - 1; z++) {
							if (combo_idx[z].idx == index && combo_idx[z].nameid == id) { // Index already recorded
								do_continue = true;
								break;
							}
						}

						if (do_continue)
							continue;
					}

					combo_idx[j].nameid = id;
					combo_idx[j].idx = index;
					pos |= sd->inventory.u.items_inventory[index].equip;
					found = true;
					break;
				} else { // Cards and enchants
					if (itemdb_isspecial(sd->inventory.u.items_inventory[index].card[0]))
						continue;
					for (uint8 z = 0; z < MAX_SLOTS; z++) {
						do_continue = false;

						if (sd->inventory.u.items_inventory[index].card[z] != id)
							continue;

						if (j > 0) {
							for (size_t c1 = 0; c1 < nb_itemCombo - 1; c1++) {
								if (combo_idx[c1].idx == index && combo_idx[c1].nameid == id) {
									for (uint8 c2 = 0; c2 < MAX_SLOTS; c2++) {
										if (combo_idx[c1].card[c2] == id) { // Card already recorded (at this same idx)
											do_continue = true;
											break;
										}
									}
								}
							}
						}

						if (do_continue)
							continue;

						combo_idx[j].nameid = id;
						combo_idx[j].idx = index;
						combo_idx[j].card[z] = id;
						pos |= sd->inventory.u.items_inventory[index].equip;
						found = true;
						break;
					}
				}
			}

			if (!found)
				break; // Unable to found all the IDs for this combo, return
		}

		// Broke out of the count loop without finding all IDs, move to the next combo
		if (j < nb_itemCombo)
			continue;

		// All items in the combo are matching
		auto entry = std::make_shared<s_combos>();

		entry->bonus = item_combo->script;
		entry->id = item_combo->id;
		entry->pos = pos;
		sd->combos.push_back(entry);
		combo_idx.clear();
		success++;
	}

	return success;
}

/**
 * Called when an item with combo is removed
 * @param sd: Player data
 * @param data: Item data
 * @return Number of removed combo(s)
 */
static int pc_removecombo(struct map_session_data *sd, item_data *data ) {

	if (sd->combos.empty())
		return 0; // Nothing to do here, player has no combos

	int retval = 0;

	for (const auto &item_combo : data->combos) {
		std::shared_ptr<s_combos> del_combo = nullptr;

		// Check if this combo exists on this player
		for (const auto &player_combo : sd->combos) {
			if (player_combo->id == item_combo->id) {
				del_combo = player_combo;
				break;
			}
		}

		// No match, skip this combo
		if (del_combo == nullptr)
			continue;

		util::vector_erase_if_exists(sd->combos, del_combo);
		retval++;

		// Check if combo requirements still fit
		if (pc_checkcombo(sd, data))
			continue;

		// It's empty, clear all the memory
		if (sd->combos.empty()) {
			sd->combos.clear();
			return retval; // Return at this point as there are no more combos to check
		}
	}

	return retval;
}

/**
 * Load combo data(s) of player
 * @param sd: Player data
 * @return ret numbers of succeed combo
 */
int pc_load_combo(struct map_session_data *sd) {
	int ret = 0;

	for (int16 i = 0; i < EQI_MAX; i++) {
		item_data *id;
		short idx = sd->equip_index[i];

		if (idx < 0 || !(id = sd->inventory_data[idx]))
			continue;

		if (!id->combos.empty())
			ret += pc_checkcombo(sd, id);

		if (!itemdb_isspecial(sd->inventory.u.items_inventory[idx].card[0])) {
			for (uint8 j = 0; j < MAX_SLOTS; j++) {
				if (!sd->inventory.u.items_inventory[idx].card[j])
					continue;

				std::shared_ptr<item_data> data = item_db.find(sd->inventory.u.items_inventory[idx].card[j]);

				if (data != nullptr) {
					if (!data->combos.empty())
						ret += pc_checkcombo(sd, data.get());
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
bool pc_equipitem(struct map_session_data *sd,short n,int req_pos,bool equipswitch)
{
	int i, pos, flag = 0, iflag;
	struct item_data *id;
	uint8 res = ITEM_EQUIP_ACK_OK;
	short* equip_index;

	nullpo_retr(false,sd);

	if( n < 0 || n >= MAX_INVENTORY ) {
		if( equipswitch ){
			clif_equipswitch_add( sd, n, req_pos, ITEM_EQUIP_ACK_FAIL );
		}else{
			clif_equipitemack(sd,0,0,ITEM_EQUIP_ACK_FAIL);
		}
		return false;
	}
	if( DIFF_TICK(sd->canequip_tick,gettick()) > 0 ) {
		if( equipswitch ){
			clif_equipswitch_add( sd, n, req_pos, ITEM_EQUIP_ACK_FAIL );
		}else{
			clif_equipitemack(sd,n,0,ITEM_EQUIP_ACK_FAIL);
		}
		return false;
	}

	if (!(id = sd->inventory_data[n]))
		return false;
	pos = pc_equippoint(sd,n); //With a few exceptions, item should go in all specified slots.

	if(battle_config.battle_log && !equipswitch)
		ShowInfo("equip %u (%d) %x:%x\n",sd->inventory.u.items_inventory[n].nameid,n,id?id->equip:0,req_pos);

	if((res = pc_isequip(sd,n))) {
		if( equipswitch ){
			clif_equipswitch_add( sd, n, req_pos, res );
		}else{
			clif_equipitemack(sd,n,0,res);	// fail
		}
		return false;
	}

	if( equipswitch && id->type == IT_AMMO ){
		clif_equipswitch_add( sd, n, req_pos, ITEM_EQUIP_ACK_FAIL );
		return false;
	}

	if (!(pos&req_pos) || sd->inventory.u.items_inventory[n].equip != 0 || sd->inventory.u.items_inventory[n].attribute==1 ) { // [Valaris]
		if( equipswitch ){
			clif_equipswitch_add( sd, n, req_pos, ITEM_EQUIP_ACK_FAIL );
		}else{
			clif_equipitemack(sd,n,0,ITEM_EQUIP_ACK_FAIL);	// fail
		}
		return false;
	}
	if( sd->sc.count && (sd->sc.cant.equip || (sd->sc.data[SC_PYROCLASTIC] && sd->inventory_data[n]->type == IT_WEAPON)) ) {
		if( equipswitch ){
			clif_equipswitch_add( sd, n, req_pos, ITEM_EQUIP_ACK_FAIL );
		}else{
			clif_equipitemack(sd,n,0,ITEM_EQUIP_ACK_FAIL); //Fail
		}
		return false;
	}

	equip_index = equipswitch ? sd->equip_switch_index : sd->equip_index;

	if ( !equipswitch && id->flag.bindOnEquip && !sd->inventory.u.items_inventory[n].bound) {
		sd->inventory.u.items_inventory[n].bound = (char)battle_config.default_bind_on_equip;
		clif_notify_bindOnEquip(sd,n);
	}

	if(pos == EQP_ACC) { //Accessories should only go in one of the two.
		pos = req_pos&EQP_ACC;
		if (pos == EQP_ACC) //User specified both slots.
			pos = (equip_index[EQI_ACC_R] >= 0 && equip_index[EQI_ACC_L] < 0) ? EQP_ACC_L : EQP_ACC_R;

		for (i = 0; i < sd->inventory_data[n]->slots; i++) { // Accessories that have cards that force equip location
			if (!sd->inventory.u.items_inventory[n].card[i])
				continue;

			std::shared_ptr<item_data> card_data = item_db.find(sd->inventory.u.items_inventory[n].card[i]);

			if (card_data) {
				int card_pos = card_data->equip;

				if (card_pos == EQP_ACC_L || card_pos == EQP_ACC_R) {
					pos = card_pos; // Use the card's equip position
					break;
				}
			}
		}
	} else if(pos == EQP_ARMS && id->equip == EQP_HAND_R) { //Dual wield capable weapon.
		pos = (req_pos&EQP_ARMS);
		if (pos == EQP_ARMS) //User specified both slots, pick one for them.
#ifdef RENEWAL
			pos = (equip_index[EQI_HAND_R] >= 0 && equip_index[EQI_HAND_L] < 0) ? EQP_HAND_L : EQP_HAND_R;
#else
			pos = equip_index[EQI_HAND_R] >= 0 ? EQP_HAND_L : EQP_HAND_R;
#endif
	} else if(pos == EQP_SHADOW_ACC) { // Shadow System
		pos = req_pos&EQP_SHADOW_ACC;
		if (pos == EQP_SHADOW_ACC)
			pos = (equip_index[EQI_SHADOW_ACC_L] >= 0 && equip_index[EQI_SHADOW_ACC_R] < 0) ? EQP_SHADOW_ACC_R : EQP_SHADOW_ACC_L;
	} else if(pos == EQP_SHADOW_ARMS && id->equip == EQP_SHADOW_WEAPON) {
		pos = (req_pos&EQP_SHADOW_ARMS);
		if( pos == EQP_SHADOW_ARMS )
			pos = (equip_index[EQI_SHADOW_WEAPON] >= 0 ? EQP_SHADOW_SHIELD : EQP_SHADOW_WEAPON);
	}

	if (pos&EQP_HAND_R && battle_config.use_weapon_skill_range&BL_PC) {
		//Update skill-block range database when weapon range changes. [Skotlex]
		i = equip_index[EQI_HAND_R];
		if (i < 0 || !sd->inventory_data[i]) //No data, or no weapon equipped
			flag = 1;
		else
			flag = id->range != sd->inventory_data[i]->range;
	}

	if( equipswitch ){
		for( i = 0; i < EQI_MAX; i++ ){
			if( pos&equip_bitmask[i] ){
				// If there was already an item assigned to this slot
				if( sd->equip_switch_index[i] >= 0 ){
					pc_equipswitch_remove( sd, sd->equip_switch_index[i] );
				}

				// Assign the new index to it
				sd->equip_switch_index[i] = n;
			}
		}

		sd->inventory.u.items_inventory[n].equipSwitch = pos;
		clif_equipswitch_add( sd, n, pos, ITEM_EQUIP_ACK_OK );
		return true;
	}else{
		for(i=0;i<EQI_MAX;i++) {
			if(pos & equip_bitmask[i]) {
				if(sd->equip_index[i] >= 0) //Slot taken, remove item from there.
					pc_unequipitem(sd,sd->equip_index[i], 1 | 2 | 4);

				sd->equip_index[i] = n;
			}
		}

		pc_equipswitch_remove(sd, n);

		if(pos==EQP_AMMO) {
			clif_arrowequip(sd,n);
			clif_arrow_fail(sd,3);
		}
		else
			clif_equipitemack(sd,n,pos,ITEM_EQUIP_ACK_OK);

		sd->inventory.u.items_inventory[n].equip = pos;
	}

	if(pos & EQP_HAND_R) {
		if(id)
			sd->weapontype1 = id->subtype;
		else
			sd->weapontype1 = W_FIST;
		pc_calcweapontype(sd);
		clif_changelook(&sd->bl,LOOK_WEAPON,sd->status.weapon);
	}
	if(pos & EQP_HAND_L) {
		if(id) {
			if(id->type == IT_WEAPON) {
				sd->status.shield = W_FIST;
				sd->weapontype2 = id->subtype;
			}
			else
			if(id->type == IT_ARMOR) {
				sd->status.shield = id->look;
				sd->weapontype2 = W_FIST;
			}
		}
		else
			sd->status.shield = sd->weapontype2 = W_FIST;
		pc_calcweapontype(sd);
		clif_changelook(&sd->bl,LOOK_SHIELD,sd->status.shield);
	}
	if(pos & EQP_SHOES)
		clif_changelook(&sd->bl,LOOK_SHOES,0);

	if (battle_config.ammo_unequip && (pos&EQP_ARMS) && id->type == IT_WEAPON) {
		short idx = sd->equip_index[EQI_AMMO];

		if (idx >= 0) {
			switch (sd->inventory_data[idx]->subtype) {
				case AMMO_ARROW:
					if (id->subtype != W_BOW && id->subtype != W_MUSICAL && id->subtype != W_WHIP)
						pc_unequipitem(sd, idx, 2 | 4);
					break;
				case AMMO_BULLET:
				case AMMO_SHELL:
					if (id->subtype != W_REVOLVER && id->subtype != W_RIFLE && id->subtype != W_GATLING && id->subtype != W_SHOTGUN
#ifdef RENEWAL
						&& id->subtype != W_GRENADE
#endif
						)
						pc_unequipitem(sd, idx, 2 | 4);
					break;
#ifndef RENEWAL
				case AMMO_GRENADE:
					if (id->subtype != W_GRENADE)
						pc_unequipitem(sd, idx, 2 | 4);
					break;
#endif
			}
		}
	}

	pc_set_costume_view(sd);

	pc_checkallowskill(sd); //Check if status changes should be halted.
	iflag = sd->npc_item_flag;

	// Check for combos (MUST be before status_calc_pc)
	if (!id->combos.empty())
		pc_checkcombo(sd, id);

	if (itemdb_isspecial(sd->inventory.u.items_inventory[n].card[0]))
		; // No cards
	else {
		for (i = 0; i < MAX_SLOTS; i++) {
			if (!sd->inventory.u.items_inventory[n].card[i])
				continue;

			std::shared_ptr<item_data> data = item_db.find(sd->inventory.u.items_inventory[n].card[i]);

			if (data != nullptr) {
				if (!data->combos.empty())
					pc_checkcombo(sd, data.get());
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
		if(itemdb_isspecial(sd->inventory.u.items_inventory[n].card[0]))
			; //No cards
		else {
			for( i = 0; i < MAX_SLOTS; i++ ) {
				if (!sd->inventory.u.items_inventory[n].card[i])
					continue;

				std::shared_ptr<item_data> data = item_db.find(sd->inventory.u.items_inventory[n].card[i]);

				if ( data != nullptr ) {
					if (data->equip_script && (pc_has_permission(sd,PC_PERM_USE_ALL_EQUIPMENT) || !itemdb_isNoEquip(data.get(), sd->bl.m)))
						run_script(data->equip_script,0,sd->bl.id,fake_nd->bl.id);
				}
			}
		}
	}
	sd->npc_item_flag = iflag;

	return true;
}

static void pc_deleteautobonus( std::vector<std::shared_ptr<s_autobonus>>& bonus, int position ){
	std::vector<std::shared_ptr<s_autobonus>>::iterator it = bonus.begin();

	while( it != bonus.end() ){
		std::shared_ptr<s_autobonus> b = *it;

		if( ( b->pos & position ) != b->pos ){
			it++;
			continue;
		}

		it = bonus.erase( it );
	}
}

/**
 * Recalculate player status on unequip
 * @param sd: Player data
 * @param n: Item inventory index
 * @param flag: Whether to recalculate a player's status or not
 * @return True on success or false on failure
 */
static void pc_unequipitem_sub(struct map_session_data *sd, int n, int flag) {
	int i, iflag;
	bool status_calc = false;

	pc_deleteautobonus( sd->autobonus, sd->inventory.u.items_inventory[n].equip );
	pc_deleteautobonus( sd->autobonus2, sd->inventory.u.items_inventory[n].equip );
	pc_deleteautobonus( sd->autobonus3, sd->inventory.u.items_inventory[n].equip );

	sd->inventory.u.items_inventory[n].equip = 0;
	if (!(flag & 4))
		pc_checkallowskill(sd);
	iflag = sd->npc_item_flag;

	// Check for combos (MUST be before status_calc_pc)
	if (sd->inventory_data[n]) {
		if (!sd->inventory_data[n]->combos.empty()) {
			if (pc_removecombo(sd, sd->inventory_data[n]))
				status_calc = true;
		}

		if (itemdb_isspecial(sd->inventory.u.items_inventory[n].card[0]))
			; // No cards
		else {
			for (i = 0; i < MAX_SLOTS; i++) {
				if (!sd->inventory.u.items_inventory[n].card[i])
					continue;

				std::shared_ptr<item_data> data = item_db.find(sd->inventory.u.items_inventory[n].card[i]);

				if (data != nullptr) {
					if (!data->combos.empty()) {
						if (pc_removecombo(sd, data.get()))
							status_calc = true;
					}
				}
			}
		}
	}

	if (flag & 1 || status_calc) {
		pc_checkallowskill(sd);
		status_calc_pc(sd, SCO_FORCE);
	}

	if (sd->sc.data[SC_SIGNUMCRUCIS] && !battle_check_undead(sd->battle_status.race, sd->battle_status.def_ele))
		status_change_end(&sd->bl, SC_SIGNUMCRUCIS, INVALID_TIMER);

	//OnUnEquip script [Skotlex]
	if (sd->inventory_data[n]) {
		if (sd->inventory_data[n]->unequip_script)
			run_script(sd->inventory_data[n]->unequip_script, 0, sd->bl.id, fake_nd->bl.id);
		if (itemdb_isspecial(sd->inventory.u.items_inventory[n].card[0]))
			; //No cards
		else {
			for (i = 0; i < MAX_SLOTS; i++) {
				if (!sd->inventory.u.items_inventory[n].card[i])
					continue;

				std::shared_ptr<item_data> data = item_db.find(sd->inventory.u.items_inventory[n].card[i]);

				if (data != nullptr) {
					if (data->unequip_script)
						run_script(data->unequip_script, 0, sd->bl.id, fake_nd->bl.id);
				}

			}
		}
	}

	sd->npc_item_flag = iflag;
}

/**
 * Called when attempting to unequip an item from a player
 * @param sd: Player data
 * @param n: Item inventory index
 * @param flag: Type of unequip
 *  0 - only unequip
 *  1 - calculate status after unequipping
 *  2 - force unequip
 *  4 - unequip by switching equipment
 * @return True on success or false on failure
 */
bool pc_unequipitem(struct map_session_data *sd, int n, int flag) {
	int i, pos;

	nullpo_retr(false,sd);

	if (n < 0 || n >= MAX_INVENTORY) {
		clif_unequipitemack(sd,0,0,0);
		return false;
	}
	if (!(pos = sd->inventory.u.items_inventory[n].equip)) {
		clif_unequipitemack(sd,n,0,0);
		return false; //Nothing to unequip
	}
	// status change that makes player cannot unequip equipment
	if (!(flag&2) && sd->sc.count &&( sd->sc.cant.unequip ||
		(sd->sc.data[SC_PYROCLASTIC] &&	sd->inventory_data[n]->type == IT_WEAPON)))	// can't switch weapon
	{
		clif_unequipitemack(sd,n,0,0);
		return false;
	}

	if (battle_config.battle_log)
		ShowInfo("unequip %d %x:%x\n",n,pc_equippoint(sd,n),pos);

	for(i = 0; i < EQI_MAX; i++) {
		if (pos & equip_bitmask[i])
			sd->equip_index[i] = -1;
	}

	if(pos & EQP_HAND_R) {
		sd->weapontype1 = 0;
		sd->status.weapon = sd->weapontype2;
		pc_calcweapontype(sd);
		clif_changelook(&sd->bl,LOOK_WEAPON,sd->status.weapon);
		if( !battle_config.dancing_weaponswitch_fix )
			status_change_end(&sd->bl, SC_DANCING, INVALID_TIMER); // Unequipping => stop dancing.
#ifdef RENEWAL
		if (battle_config.switch_remove_edp&2) {
#else
		if (battle_config.switch_remove_edp&1) {
#endif
			status_change_end(&sd->bl, SC_EDP, INVALID_TIMER);
		}
	}
	if(pos & EQP_HAND_L) {
		if (sd->status.shield && battle_getcurrentskill(&sd->bl) == LG_SHIELDSPELL)
			unit_skillcastcancel(&sd->bl, 0); // Cancel Shield Spell if player swaps shields.

		sd->status.shield = sd->weapontype2 = W_FIST;
		pc_calcweapontype(sd);
		clif_changelook(&sd->bl,LOOK_SHIELD,sd->status.shield);
	}

	if(pos & EQP_SHOES)
		clif_changelook(&sd->bl,LOOK_SHOES,0);

	clif_unequipitemack(sd,n,pos,1);
	pc_set_costume_view(sd);

	status_db.removeByStatusFlag(&sd->bl, { SCF_REMOVEONUNEQUIP });

	// On weapon change (right and left hand)
	if ((pos & EQP_ARMS) && sd->inventory_data[n]->type == IT_WEAPON) {
		if (battle_config.ammo_unequip && !(flag & 4)) {
			switch (sd->inventory_data[n]->subtype) {
				case W_BOW:
				case W_MUSICAL:
				case W_WHIP:
				case W_REVOLVER:
				case W_RIFLE:
				case W_GATLING:
				case W_SHOTGUN:
				case W_GRENADE: {
					short idx = sd->equip_index[EQI_AMMO];

					if (idx >= 0) {
						sd->equip_index[EQI_AMMO] = -1;
						clif_unequipitemack(sd, idx, sd->inventory.u.items_inventory[idx].equip, 1);
						pc_unequipitem_sub(sd, idx, 0);
					}
				}
				break;
			}
		}

		status_db.removeByStatusFlag(&sd->bl, { SCF_REMOVEONUNEQUIPWEAPON });
	}

	// On armor change
	if (pos & EQP_ARMOR) {
		status_db.removeByStatusFlag(&sd->bl, { SCF_REMOVEONUNEQUIPARMOR });
	}

	// On equipment change
#ifndef RENEWAL
	if (!(flag&4))
		status_change_end(&sd->bl, SC_CONCENTRATION, INVALID_TIMER);
#endif

	// On ammo change
	if (sd->inventory_data[n]->type == IT_AMMO && (sd->inventory_data[n]->nameid != ITEMID_SILVER_BULLET || sd->inventory_data[n]->nameid != ITEMID_PURIFICATION_BULLET || sd->inventory_data[n]->nameid != ITEMID_SILVER_BULLET_))
		status_change_end(&sd->bl, SC_P_ALTER, INVALID_TIMER);

	pc_unequipitem_sub(sd, n, flag);

	return true;
}

int pc_equipswitch( struct map_session_data* sd, int index ){
	// Get the target equip mask
	int position = sd->inventory.u.items_inventory[index].equipSwitch;

	// Get the currently equipped item
	short equippedItem = pc_checkequip( sd, position, true );

	// No item equipped at the target
	if( equippedItem == -1 ){
		// Remove it from the equip switch
		pc_equipswitch_remove( sd, index );

		pc_equipitem( sd, index, position );

		return position;
	}else{
		std::map<int, int> unequipped;
		int unequipped_position = 0;

		// Unequip all items that interfere
		for( int i = 0; i < EQI_MAX; i++ ){
			int unequip_index = sd->equip_index[i];

			if( unequip_index >= 0 && position & equip_bitmask[i] ){
				struct item* unequip_item = &sd->inventory.u.items_inventory[unequip_index];

				// Store the unequipped index and position mask for later
				unequipped[unequip_index] = unequip_item->equip;

				// Keep the position for later
				unequipped_position |= unequip_item->equip;

				// Unequip the item
				pc_unequipitem( sd, unequip_index, 0 );
			}
		}

		int all_position = position | unequipped_position;

		// Equip everything that is hit by the mask
		for( int i = 0; i < EQI_MAX; i++ ){
			int exchange_index = sd->equip_switch_index[i];

			if( exchange_index >= 0 && all_position & equip_bitmask[i] ){
				struct item* exchange_item = &sd->inventory.u.items_inventory[exchange_index];

				// Store the target position
				int exchange_position = exchange_item->equipSwitch;

				// Remove the item from equip switch
				pc_equipswitch_remove( sd, exchange_index );

				// Equip the item at the destinated position
				pc_equipitem( sd, exchange_index, exchange_position );
			}
		}

		// Place all unequipped items into the equip switch window
		for( std::pair<int, int> pair : unequipped ){
			int unequipped_index = pair.first;
			int unequipped_position = pair.second;

			// Rebuild the index cache
			for( int i = 0; i < EQI_MAX; i++ ){
				if( unequipped_position & equip_bitmask[i] ){
					sd->equip_switch_index[i] = unequipped_index;
				}
			}

			// Set the correct position mask
			sd->inventory.u.items_inventory[unequipped_index].equipSwitch = unequipped_position;

			// Notify the client
			clif_equipswitch_add( sd, unequipped_index, unequipped_position, ITEM_EQUIP_ACK_OK );
		}

		return all_position;
	}
}

void pc_equipswitch_remove( struct map_session_data* sd, int index ){
	struct item* item = &sd->inventory.u.items_inventory[index];

	if( !item->equipSwitch ){
		return;
	}

	for( int i = 0; i < EQI_MAX; i++ ){
		// If a match is found
		if( sd->equip_switch_index[i] == index ){
			// Remove it from the slot
			sd->equip_switch_index[i] = -1;
		}
	}

	// Send out one packet for all slots using the current item's mask
	clif_equipswitch_remove( sd, index, item->equipSwitch, false );

	item->equipSwitch = 0;
}

/*==========================================
 * Checking if player (sd) has an invalid item
 * and is unequiped on map load (item_noequip)
 *------------------------------------------*/
void pc_checkitem(struct map_session_data *sd) {
	int i, calc_flag = 0;
	struct item* it;

	nullpo_retv(sd);

	if( sd->state.vending ) //Avoid reorganizing items when we are vending, as that leads to exploits (pointed out by End of Exam)
		return;

	pc_check_available_item(sd, ITMCHK_NONE); // Check for invalid(ated) items.

	for( i = 0; i < MAX_INVENTORY; i++ ) {
		it = &sd->inventory.u.items_inventory[i];

		if( it->nameid == 0 )
			continue;
		if( !it->equip )
			continue;
		if( it->equip&~pc_equippoint(sd,i) ) {
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

	for( i = 0; i < MAX_INVENTORY; i++ ) {
		it = &sd->inventory.u.items_inventory[i];

		if( it->nameid == 0 )
			continue;
		if( !it->equipSwitch )
			continue;
		if( it->equipSwitch&~pc_equippoint(sd,i) ||
			( !pc_has_permission(sd, PC_PERM_USE_ALL_EQUIPMENT) && !battle_config.allow_equip_restricted_item && itemdb_isNoEquip(sd->inventory_data[i], sd->bl.m) ) ){
			
			for( int j = 0; j < EQI_MAX; j++ ){
				if( sd->equip_switch_index[j] == i ){
					sd->equip_switch_index[j] = -1;
				}
			}

			sd->inventory.u.items_inventory[i].equipSwitch = 0;

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
 * @param sd: Player data
 * @param type Forced check:
 *   1 - Inventory
 *   2 - Cart
 *   4 - Storage
 *------------------------------------------*/
void pc_check_available_item(struct map_session_data *sd, uint8 type)
{
	int i;
	t_itemid nameid;
	char output[256];

	nullpo_retv(sd);

	if (battle_config.item_check&ITMCHK_INVENTORY && type&ITMCHK_INVENTORY) { // Check for invalid(ated) items in inventory.
		for(i = 0; i < MAX_INVENTORY; i++) {
			nameid = sd->inventory.u.items_inventory[i].nameid;

			if (!nameid)
				continue;
			if (!itemdb_available(nameid)) {
				sprintf(output, msg_txt(sd, 709), nameid); // Item %u has been removed from your inventory.
				clif_displaymessage(sd->fd, output);
				ShowWarning("Removed invalid/disabled item (ID: %u, amount: %d) from inventory (char_id: %d).\n", nameid, sd->inventory.u.items_inventory[i].amount, sd->status.char_id);
				pc_delitem(sd, i, sd->inventory.u.items_inventory[i].amount, 4, 0, LOG_TYPE_OTHER);
				continue;
			}
			if (!sd->inventory.u.items_inventory[i].unique_id && !itemdb_isstackable(nameid))
				sd->inventory.u.items_inventory[i].unique_id = pc_generate_unique_id(sd);
		}
	}

	if (battle_config.item_check&ITMCHK_CART && type&ITMCHK_CART) { // Check for invalid(ated) items in cart.
		for(i = 0; i < MAX_CART; i++) {
			nameid = sd->cart.u.items_cart[i].nameid;

			if (!nameid)
				continue;
			if (!itemdb_available(nameid)) {
				sprintf(output, msg_txt(sd, 710), nameid); // Item %u has been removed from your cart.
				clif_displaymessage(sd->fd, output);
				ShowWarning("Removed invalid/disabled item (ID: %u, amount: %d) from cart (char_id: %d).\n", nameid, sd->cart.u.items_cart[i].amount, sd->status.char_id);
				pc_cart_delitem(sd, i, sd->cart.u.items_cart[i].amount, 0, LOG_TYPE_OTHER);
				continue;
			}
			if (!sd->cart.u.items_cart[i].unique_id && !itemdb_isstackable(nameid))
				sd->cart.u.items_cart[i].unique_id = pc_generate_unique_id(sd);
		}
	}

	if (battle_config.item_check&ITMCHK_STORAGE && type&ITMCHK_STORAGE) { // Check for invalid(ated) items in storage.
		for(i = 0; i < sd->storage.max_amount; i++) {
			nameid = sd->storage.u.items_storage[i].nameid;

			if (!nameid)
				continue;
			if (!itemdb_available(nameid)) {
				sprintf(output, msg_txt(sd, 711), nameid); // Item %u has been removed from your storage.
				clif_displaymessage(sd->fd, output);
				ShowWarning("Removed invalid/disabled item (ID: %u, amount: %d) from storage (char_id: %d).\n", nameid, sd->storage.u.items_storage[i].amount, sd->status.char_id);
				storage_delitem(sd, &sd->storage, i, sd->storage.u.items_storage[i].amount);
				continue;
			}
			if (!sd->storage.u.items_storage[i].unique_id && !itemdb_isstackable(nameid))
				sd->storage.u.items_storage[i].unique_id = pc_generate_unique_id(sd);
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

	if( pc_isinvisible(sd1) || pc_isinvisible(sd2) )
	{// cannot register pvp rank for hidden GMs
		return 0;
	}

	if( sd1->pvp_point > sd2->pvp_point )
		sd2->pvp_rank++;
	return 0;
}
/*==========================================
 * Calculate new rank beetween all present players (map_foreachinallarea)
 * and display result
 *------------------------------------------*/
int pc_calc_pvprank(struct map_session_data *sd)
{
	int old = sd->pvp_rank;
	struct map_data *mapdata = map_getmapdata(sd->bl.m);

	sd->pvp_rank=1;
	map_foreachinmap(pc_calc_pvprank_sub,sd->bl.m,BL_PC,sd);
	if(old!=sd->pvp_rank || sd->pvp_lastusers!=mapdata->users_pvp)
		clif_pvpset(sd,sd->pvp_rank,sd->pvp_lastusers=mapdata->users_pvp,0);
	return sd->pvp_rank;
}
/*==========================================
 * Calculate next sd ranking calculation from config
 *------------------------------------------*/
TIMER_FUNC(pc_calc_pvprank_timer){
	struct map_session_data *sd;

	sd=map_id2sd(id);
	if(sd==NULL)
		return 0;
	sd->pvp_timer = INVALID_TIMER;

	if( pc_isinvisible(sd) )
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

	achievement_update_objective(sd, AG_MARRY, 1, 1);
	achievement_update_objective(dstsd, AG_MARRY, 1, 1);

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
		if( sd->inventory.u.items_inventory[i].nameid == WEDDING_RING_M || sd->inventory.u.items_inventory[i].nameid == WEDDING_RING_F )
			pc_delitem(sd, i, 1, 0, 0, LOG_TYPE_OTHER);
		if( p_sd->inventory.u.items_inventory[i].nameid == WEDDING_RING_M || p_sd->inventory.u.items_inventory[i].nameid == WEDDING_RING_F )
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
void pc_bleeding (struct map_session_data *sd, t_tick diff_tick)
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
void pc_regen (struct map_session_data *sd, t_tick diff_tick)
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

	if (sd->percent_hp_regen.value) {
		sd->percent_hp_regen.tick += diff_tick;
		while (sd->percent_hp_regen.tick >= sd->percent_hp_regen.rate) {
			hp += sd->status.max_hp * sd->percent_hp_regen.value / 100;
			sd->percent_hp_regen.tick -= sd->percent_hp_regen.rate;
		}
	}

	if (sd->percent_sp_regen.value) {
		sd->percent_sp_regen.tick += diff_tick;
		while (sd->percent_sp_regen.tick >= sd->percent_sp_regen.rate) {
			sp += sd->status.max_sp * sd->percent_sp_regen.value / 100;
			sd->percent_sp_regen.tick -= sd->percent_sp_regen.rate;
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
static TIMER_FUNC(pc_autosave){
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
		if (!sd->state.pc_loaded) // Player data hasn't fully loaded
			continue;
		if(sd->bl.id == last_save_id && save_flag != 1) {
			save_flag = 1;
			continue;
		}

		if(save_flag != 1) //Not our turn to save yet.
			continue;

		//Save char.
		last_save_id = sd->bl.id;
		save_flag = 2;
		if (pc_isvip(sd)) // Check if we're still VIP
			chrif_req_login_operation(1, sd->status.name, CHRIF_OP_LOGIN_VIP, 0, 1, 0);
		chrif_save(sd, CSAVE_INVENTORY|CSAVE_CART);
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
	if (sd->state.night != night_flag && map_getmapflag(sd->bl.m, MF_NIGHTENABLED))
	{	//Night/day state does not match.
		clif_status_load(&sd->bl, EFST_SKE, night_flag); //New night effect by dynamix [Skotlex]
		sd->state.night = night_flag;
		return 1;
	}
	return 0;
}
/*================================================
 * timer to do the day [Yor]
 * data: 0 = called by timer, 1 = gmcommand/script
 *------------------------------------------------*/
TIMER_FUNC(map_day_timer){
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
TIMER_FUNC(map_night_timer){
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
	if (!force && (sd->sc.data[SC_SITDOWN_FORCE] || sd->sc.data[SC_BANANA_BOMB_SITDOWN]))
		return false;

	status_change_end(&sd->bl, SC_TENSIONRELAX, INVALID_TIMER);
	clif_status_load(&sd->bl,EFST_SIT,0);
	clif_standing(&sd->bl); //Inform area PC is standing
	//Reset sitting tick.
	sd->ssregen.tick.hp = sd->ssregen.tick.sp = 0;
	if( pc_isdead( sd ) ){
		sd->state.dead_sit = sd->vd.dead_sit = 0;
		clif_party_dead( sd );
	}else{
		sd->state.dead_sit = sd->vd.dead_sit = 0;
	}
	return true;
}

/**
 * Calculate Overheat value
 * @param sd: Player data
 * @param heat: Amount of Heat to adjust
 **/
void pc_overheat(struct map_session_data *sd, int16 heat) {
	nullpo_retv(sd);

	status_change_entry *sce = sd->sc.data[SC_OVERHEAT_LIMITPOINT];

	if (sce) {
		static std::vector<int16> limit = { 150, 200, 280, 360, 450 };
		uint16 skill_lv = cap_value(pc_checkskill(sd, NC_MAINFRAME), 0, (uint16)(limit.size()-1));

		sce->val1 += heat;
		sce->val1 = cap_value(sce->val1, 0, 1000);
		if (sd->sc.data[SC_OVERHEAT])
			status_change_end(&sd->bl, SC_OVERHEAT, INVALID_TIMER);
		if (sce->val1 > limit[skill_lv])
			sc_start(&sd->bl, &sd->bl, SC_OVERHEAT, 100, sce->val1, 1000);
	} else if (heat > 0)
		sc_start(&sd->bl, &sd->bl, SC_OVERHEAT_LIMITPOINT, 100, heat, 1000);
}

/**
 * Check if player is autolooting given itemID.
 */
bool pc_isautolooting(struct map_session_data *sd, t_itemid nameid)
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
bool pc_can_use_command( struct map_session_data *sd, const char *command, AtCommandType type ){
	return sd->group->can_use_command( command, type );
}

bool pc_has_permission( struct map_session_data* sd, e_pc_permission permission ){
	return sd->permissions.test( permission );
}

/**
 * Checks if commands used by a player should be logged
 * according to their group setting.
 * @param sd Player map session data
 */
bool pc_should_log_commands( struct map_session_data *sd ){
	return sd->group->log_commands;
}

/**
 * Spirit Charm expiration timer.
 * @see TimerFunc
 */
static TIMER_FUNC(pc_spiritcharm_timer){
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
/**
 * Renewal EXP/Item Drop rate modifier based on level penalty
 * @param level_diff: Monster and Player level difference
 * @param mob_class: Monster class
 * @param mode: Monster mode
 * @param type: 1 - EXP, 2 - Item Drop
 * @return Penalty rate
 */
uint16 pc_level_penalty_mod( struct map_session_data* sd, e_penalty_type type, std::shared_ptr<s_mob_db> mob, mob_data* md ){
	// No player was attached, we don't use any modifier (100 = rates are not touched)
	if( sd == nullptr ){
		return 100;
	}

	if ((type == PENALTY_DROP && map_getmapflag(sd->bl.m, MF_NORENEWALDROPPENALTY)) || (type == PENALTY_EXP && map_getmapflag(sd->bl.m, MF_NORENEWALEXPPENALTY))) {
		return 100;
	}

	int monster_level;

	if( md != nullptr ){
		monster_level = md->level;
		mob = md->db;
	}else if( mob != nullptr ){
		monster_level = mob->lv;
	}else{
		return 100;
	}

	if( ( type == PENALTY_DROP || type == PENALTY_MVP_DROP ) && status_has_mode( &mob->status, MD_FIXEDITEMDROP )  ){
		return 100;
	}

	int level_difference = monster_level - sd->status.base_level;

	std::shared_ptr<s_penalty> penalty = penalty_db.find( type );

	if( penalty != nullptr ){
		return penalty->rate[ level_difference + MAX_LEVEL - 1 ];
	}else{
		return 100;
	}
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


std::shared_ptr<s_skill_tree_entry> SkillTreeDatabase::get_skill_data(int class_, uint16 skill_id) {
	std::shared_ptr<s_skill_tree> tree = this->find(class_);

	if (tree != nullptr)
		return util::umap_find(tree->skills, skill_id);

	return nullptr;
}

const std::string SkillTreeDatabase::getDefaultLocation() {
	return std::string(db_path) + "/skill_tree.yml";
}

/**
 * Reads and parses an entry from the skill_tree.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 SkillTreeDatabase::parseBodyNode(const ryml::NodeRef& node) {
	std::string job_name;

	if (!this->asString(node, "Job", job_name))
		return 0;

	int64 constant;
	std::string job_name_constant = "JOB_" + job_name;

	if (!script_get_constant(job_name_constant.c_str(), &constant) || !pcdb_checkid(constant)) {
		this->invalidWarning(node["Job"], "Invalid job %s.\n", job_name.c_str());
		return 0;
	}
	uint16 job_id = static_cast<uint16>(constant);

	std::shared_ptr<s_skill_tree> tree = this->find(job_id);
	bool exists = tree != nullptr;

	if (!exists)
		tree = std::make_shared<s_skill_tree>();

	if (this->nodeExists(node, "Inherit")) {
		const ryml::NodeRef& InheritNode = node["Inherit"];

		for (const auto &Inheritit : InheritNode) {
			std::string inheritname;
			c4::from_chars(Inheritit.key(), &inheritname);
			std::string inheritname_constant = "JOB_" + inheritname;

			if (!script_get_constant(inheritname_constant.c_str(), &constant) || !pcdb_checkid(constant)) {
				this->invalidWarning(InheritNode[Inheritit.key()], "Invalid job %s.\n", inheritname.c_str());
				return 0;
			}

			bool active;

			if (!this->asBool(InheritNode, inheritname, active))
				return 0;

			uint16 inherit_job = static_cast<uint16>(constant);

			if (!active) {
				if (exists)
					util::vector_erase_if_exists(tree->inherit_job, inherit_job);
			}
			else {
				if (!util::vector_exists(tree->inherit_job, inherit_job))
					tree->inherit_job.push_back(inherit_job);
			}
		}
	}

	if (this->nodeExists(node, "Tree")) {
		for (const auto &it : node["Tree"]) {
			std::string skill_name;

			if (!this->asString(it, "Name", skill_name))
				return 0;

			uint16 skill_id = skill_name2id(skill_name.c_str());

			if (skill_id == 0) {
				this->invalidWarning(it["Name"], "Invalid skill name \"%s\".\n", skill_name.c_str());
				return 0;
			}
			if (!skill_get_index(skill_id)) {
				this->invalidWarning(it["Name"], "Unable to load skill %s into job %hu's tree.\n", skill_name.c_str(), job_id);
				return 0;
			}

			std::shared_ptr<s_skill_tree_entry> entry;
			bool skill_exists = tree->skills.count(skill_id) > 0;
				
			if (skill_exists)
				entry = tree->skills[skill_id];
			else
				entry = std::make_shared<s_skill_tree_entry>();

			entry->skill_id = skill_id;

			uint16 max_lv;

			if (!this->asUInt16(it, "MaxLevel", max_lv))
				return 0;

			if (max_lv > MAX_SKILL_LEVEL) {
				this->invalidWarning(it["MaxLevel"], "MaxLevel exceeds the maximum skill level of %d, skipping.\n", MAX_SKILL_LEVEL);
				return 0;
			}

			uint16 skill_lv_max = skill_get_max(skill_id);

			if (max_lv > skill_lv_max) {
				this->invalidWarning(it["MaxLevel"], "Skill %s's level %hu exceeds the skill's max level %hu. Capping skill level.\n", skill_name.c_str(), max_lv, skill_lv_max);
				max_lv = skill_lv_max;
			}

			// if (max_lv == 0) {	// skill lvl 0 removed on loadingFinished (because of inherit)
				// if (!skill_exists || entry->skill_id.erase(skill_id) == 0)
					// this->invalidWarning(it["Name"], "Failed to erase %s, the skill doesn't exist in for job %s, skipping.\n", skill_name.c_str(), job_name.c_str());
				// continue;
			// }

			entry->max_lv = max_lv;

			if (this->nodeExists(it, "BaseLevel")) {
				uint32 baselv;

				if (!this->asUInt32(it, "BaseLevel", baselv))
					return 0;

				uint32 baselv_max = job_db.get_maxBaseLv(job_id);

				if (baselv > baselv_max) {
					this->invalidWarning(it["BaseLevel"], "Skill %hu's base level requirement %hu exceeds job %s's max base level %d. Capping skill base level.\n",
						skill_id, baselv, job_name.c_str(), baselv_max);
					baselv = baselv_max;
				}
				entry->baselv = baselv;
			} else {
				if (!skill_exists)
					entry->baselv = 0;
			}

			if (this->nodeExists(it, "JobLevel")) {
				uint32 joblv;

				if (!this->asUInt32(it, "JobLevel", joblv))
					return 0;

				uint32 joblv_max = job_db.get_maxJobLv(job_id);

				if (joblv > joblv_max) {
					this->invalidWarning(it["JobLevel"], "Skill %hu's job level requirement %hu exceeds job %s's max job level %d. Capping skill job level.\n",
						skill_id, joblv, job_name.c_str(), joblv_max);
					joblv = joblv_max;
				}
				entry->joblv = joblv;
			} else {
				if (!skill_exists)
					entry->joblv = 0;
			}

			if (this->nodeExists(it, "Requires")) {
				for (const auto &Requiresit : it["Requires"]) {
					if (!this->nodesExist(Requiresit, { "Name" }))
						return 0;

					std::string skill_name_req;

					if (!this->asString(Requiresit, "Name", skill_name_req))
						return 0;

					uint16 skill_id_req = skill_name2id(skill_name_req.c_str());

					if (skill_id_req == 0) {
						this->invalidWarning(Requiresit["Name"], "Invalid skill name \"%s\".\n", skill_name_req.c_str());
						return 0;
					}

					uint16 lv_req;

					if (!this->asUInt16(Requiresit, "Level", lv_req))
						return 0;

					if (lv_req > MAX_SKILL_LEVEL) {
						this->invalidWarning(Requiresit["Level"], "Level exceeds the maximum skill level of %d, skipping.\n", MAX_SKILL_LEVEL);
						return 0;
					}

					uint16 lv_req_max = skill_get_max(skill_id_req);

					if (lv_req > lv_req_max) {
						this->invalidWarning(it["MaxLevel"], "Required skill %s's level %hu exceeds the skill's max level %hu. Capping skill level.\n", skill_name.c_str(), lv_req, lv_req_max);
						lv_req = lv_req_max;
					}
					
					if (lv_req == 0) {
						if (entry->need.erase(skill_id_req) == 0)
							this->invalidWarning(Requiresit["Name"], "Failed to erase %s, the skill doesn't exist in for job %s, skipping.\n", skill_name_req.c_str(), job_name.c_str());
						continue;
					}

					entry->need[skill_id_req] = lv_req;
				}
			}

			if (this->nodeExists(it, "Exclude")) {
				bool exclude;

				if (!this->asBool(it, "Exclude", exclude))
					return 0;

				entry->exclude_inherit = exclude;
			} else {
				if (!skill_exists)
					entry->exclude_inherit = false;
			}

			if (!skill_exists)
				tree->skills.insert({ skill_id, entry });
		}
	}

	if (!exists)
		this->put(job_id, tree);

	return true;
}

void SkillTreeDatabase::loadingFinished() {
	std::unordered_map<uint16, std::shared_ptr<s_skill_tree>> job_tree;	// get the data from skill_tree_db before populate it

	for (auto &data : *this) {
		if (data.second->inherit_job.empty())
			continue;

		std::shared_ptr<s_skill_tree> skill_tree = std::make_shared<s_skill_tree>();

		uint32 baselv_max = job_db.get_maxBaseLv(data.first);
		uint32 joblv_max = job_db.get_maxJobLv(data.first);

		for (const auto &inherit_job : data.second->inherit_job) {
			std::shared_ptr<s_skill_tree> tree = this->find(inherit_job);
			if (tree == nullptr || tree->skills.empty())
				continue;

			for (const auto &it : tree->skills) {
				if (it.second->exclude_inherit)
					continue;
				if (data.second->skills.count(it.first) > 0)	// skill already in the skill tree
					continue;

				if (skill_tree->skills.count(it.first) > 0)	// replaced by the last inheritance
					skill_tree->skills[it.first] = it.second;
				else
					skill_tree->skills.insert({ it.first, it.second });
				std::shared_ptr<s_skill_tree_entry> skill = skill_tree->skills[it.first];

				if (skill->baselv > baselv_max) {
					ShowWarning("SkillTreeDatabase: Skill %s (%hu)'s base level requirement %hu exceeds job %s's max base level %d. Capping skill base level.\n",
						skill_get_name(skill->skill_id), skill->skill_id, skill->baselv, job_name(data.first), baselv_max);
					skill->baselv = baselv_max;
				}
				if (skill->joblv > joblv_max) {
					ShowWarning("SkillTreeDatabase: Skill %s (%hu)'s job level requirement %hu exceeds job %s's max job level %d. Capping skill job level.\n",
						skill_get_name(skill->skill_id), skill->skill_id, skill->joblv, job_name(data.first), joblv_max);
					skill->joblv = joblv_max;
				}
			}
		}
		if (skill_tree != nullptr && !skill_tree->skills.empty())
			job_tree.insert({ data.first, skill_tree });
	}

	if (!job_tree.empty()) {
		for (auto &data : *this) {
			if (job_tree.count(data.first) == 0)
				continue;
			data.second->skills.insert(job_tree[data.first]->skills.begin(), job_tree[data.first]->skills.end());
		}
	}

	// remove skills with max_lv = 0
	for (const auto &job : *this) {
		if (job.second->skills.empty())
			continue;

		auto it = job.second->skills.begin();

		while( it != job.second->skills.end() ){
			if( it->second->max_lv == 0 ){
				it = job.second->skills.erase( it );
			}else{
				it++;
			}
		}
	}

	TypesafeYamlDatabase::loadingFinished();
}

/**
 * Calculates base hp of player. Reference: http://irowiki.org/wiki/Max_HP
 * @param level: Base level of player
 * @param job_id: Job ID @see enum e_job
 * @return base_hp
 * @author [Cydh]
 */
static unsigned int pc_calc_basehp(uint16 level, uint16 job_id) {
	std::shared_ptr<s_job_info> job = job_db.find(job_id);
	double base_hp = 35 + level * (job->hp_increase / 100.);

#ifndef RENEWAL
	if (level >= 10 && (job_id == JOB_NINJA || job_id == JOB_GUNSLINGER))
		base_hp += 90;
#endif
	for (uint16 i = 2; i <= level; i++)
		base_hp += floor(((job->hp_factor / 100.) * i) + 0.5); //Don't have round()
	if (job_id == JOB_SUMMONER || job_id == JOB_SPIRIT_HANDLER)
		base_hp += floor((base_hp / 2) + 0.5);
	return (unsigned int)base_hp;
}

/**
 * Calculates base sp of player.
 * @param level: Base level of player
 * @param job_id: Job ID @see enum e_job
 * @return base_sp
 * @author [Playtester]
 */
static unsigned int pc_calc_basesp(uint16 level, uint16 job_id) {
	std::shared_ptr<s_job_info> job = job_db.find(job_id);
	double base_sp = 10 + floor(level * (job->sp_increase / 100.));

	switch (job_id) {
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
		case JOB_SUMMONER:
		case JOB_SPIRIT_HANDLER:
			base_sp -= floor(base_sp / 2);
			break;
	}

	return (unsigned int)base_sp;
}

const std::string JobDatabase::getDefaultLocation() {
	return std::string(db_path) + "/job_stats.yml";
}

/**
 * Reads and parses an entry from the job_db.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 JobDatabase::parseBodyNode(const ryml::NodeRef& node) {
	if (this->nodeExists(node, "Jobs")) {
		const ryml::NodeRef& jobsNode = node["Jobs"];

		for (const auto &jobit : jobsNode) {
			std::string job_name;
			c4::from_chars(jobit.key(), &job_name);
			std::string job_name_constant = "JOB_" + job_name;
			int64 job_id;

			if (!script_get_constant(job_name_constant.c_str(), &job_id)) {
				this->invalidWarning(node["Job"], "Job %s does not exist.\n", job_name.c_str());
				return 0;
			}

			std::shared_ptr<s_job_info> job = job_db.find(static_cast<uint16>(job_id));
			bool exists = job != nullptr;

			if (!exists) {
				job = std::make_shared<s_job_info>();

				job->job_bonus.resize(MAX_LEVEL);
				std::fill(job->job_bonus.begin(), job->job_bonus.end(), std::array<uint16, PARAM_MAX> { 0 });

				job->base_hp.resize(MAX_LEVEL);
				std::fill(job->base_hp.begin(), job->base_hp.end(), 0);

				job->base_sp.resize(MAX_LEVEL);
				std::fill(job->base_sp.begin(), job->base_sp.end(), 0);

				job->base_ap.resize(MAX_LEVEL);
				std::fill(job->base_ap.begin(), job->base_ap.end(), 0);
			}

			if (this->nodeExists(node, "MaxWeight")) {
				uint32 weight;

				if (!this->asUInt32(node, "MaxWeight", weight))
					return 0;

				job->max_weight_base = weight;
			} else {
				if (!exists)
					job->max_weight_base = 20000;
			}

			if (this->nodeExists(node, "HpFactor")) {
				uint32 hp;

				if (!this->asUInt32(node, "HpFactor", hp))
					return 0;

				job->hp_factor = hp;
			} else {
				if (!exists)
					job->hp_factor = 0;
			}

			if (this->nodeExists(node, "HpIncrease")) {
				uint32 hp;

				if (!this->asUInt32(node, "HpIncrease", hp))
					return 0;

				job->hp_increase = hp;
			} else {
				if (!exists)
					job->hp_increase = 500;
			}

			if (this->nodeExists(node, "SpIncrease")) {
				uint32 sp;

				if (!this->asUInt32(node, "SpIncrease", sp))
					return 0;

				job->sp_increase = sp;
			} else {
				if (!exists)
					job->sp_increase = 100;
			}

			if (this->nodeExists(node, "BaseASPD")) {
				const ryml::NodeRef& aspdNode = node["BaseASPD"];
				uint8 max = MAX_WEAPON_TYPE;

#ifdef RENEWAL // Renewal adds an extra column for shields
				max += 1;
#endif

				if (!exists) {
					job->aspd_base.resize(max);
					std::fill(job->aspd_base.begin(), job->aspd_base.end(), 2000);
				}

				for (const auto &aspdit : aspdNode) {
					std::string weapon;
					c4::from_chars(aspdit.key(), &weapon);
					std::string weapon_constant = "W_" + weapon;
					int64 constant;

					if (!script_get_constant(weapon_constant.c_str(), &constant)) {
						this->invalidWarning(aspdNode["BaseASPD"], "Unknown weapon type %s specified for %s, skipping.\n", weapon.c_str(), job_name.c_str());
						continue;
					}

					if (constant < W_FIST || constant > max) {
						this->invalidWarning(aspdNode["BaseASPD"], "Invalid weapon type %s specified for %s, skipping.\n", weapon.c_str(), job_name.c_str());
						continue;
					}

					int16 aspd;

					if (!this->asInt16(aspdNode, weapon.c_str(), aspd))
						return 0;

					job->aspd_base[static_cast<int16>(constant)] = aspd;
				}
			} else {
				if (!exists) {
					uint8 max = MAX_WEAPON_TYPE;

#ifdef RENEWAL // Renewal adds an extra column for shields
					max += 1;
#endif
					job->aspd_base.resize(max);
					std::fill(job->aspd_base.begin(), job->aspd_base.end(), 2000);
				}
			}

			if (this->nodeExists(node, "MaxStats")) {
				const ryml::NodeRef& statNode = node["MaxStats"];

				for (const auto &statit : statNode) {
					std::string stat;
					c4::from_chars(statit.key(), &stat);
					std::string stat_constant = "PARAM_" + stat;
					int64 constant;

					if (!script_get_constant(stat_constant.c_str(), &constant)) {
						this->invalidWarning(statNode["Bonus"], "Unknown max stat %s specified for %s, skipping.\n", stat.c_str(), job_name.c_str());
						continue;
					}

					if (constant < PARAM_STR || constant >= PARAM_MAX) {
						this->invalidWarning(statNode["Bonus"], "Invalid max stat %s specified for %s, skipping.\n", stat.c_str(), job_name.c_str());
						continue;
					}

					uint16 max;

					if (!this->asUInt16(statNode, stat.c_str(), max))
						return 0;

					job->max_param[constant] = max;
				}
			}

			if (this->nodeExists(node, "MaxBaseLevel")) {
				uint16 level;

				if (!this->asUInt16(node, "MaxBaseLevel", level))
					return 0;

				if (level == 0 || level > MAX_LEVEL) {
					this->invalidWarning(node["MaxBaseLevel"], "MaxBaseLevel must be between 1~MAX_LEVEL for %s, capping to MAX_LEVEL.\n", job_name.c_str());
					level = MAX_LEVEL;
				}

				job->max_base_level = level;
			} else {
				if (!exists)
					job->max_base_level = MAX_LEVEL;
			}

			if (this->nodeExists(node, "BaseExp")) {
				for (const ryml::NodeRef& bexpNode : node["BaseExp"]) {
					uint16 level;

					if (!this->asUInt16(bexpNode, "Level", level))
						return 0;

					if (level > job->max_base_level)
						continue;

					if (level == 0 || level > MAX_LEVEL) {
						this->invalidWarning(bexpNode["Level"], "Level must be between 1~MAX_LEVEL for %s.\n", job_name.c_str());
						return 0;
					}

					if (this->nodeExists(bexpNode, "Exp")) {
						t_exp exp;

						if (!this->asUInt64(bexpNode, "Exp", exp))
							return 0;

						job->base_exp[level - 1] = exp;
					}
				}
			}

			if (this->nodeExists(node, "MaxJobLevel")) {
				uint16 level;

				if (!this->asUInt16(node, "MaxJobLevel", level))
					return 0;

				if (level == 0 || level > MAX_LEVEL) {
					this->invalidWarning(node["MaxJobLevel"], "MaxJobLevel must be between 1~MAX_LEVEL for %s, capping to MAX_LEVEL.\n", job_name.c_str());
					level = MAX_LEVEL;
				}

				job->max_job_level = level;
			} else {
				if (!exists)
					job->max_job_level = MAX_LEVEL;
			}

			if (this->nodeExists(node, "JobExp")) {
				for (const ryml::NodeRef& jexpNode : node["JobExp"]) {
					uint16 level;

					if (!this->asUInt16(jexpNode, "Level", level))
						return 0;

					if (level > job->max_job_level)
						continue;

					if (level == 0 || level > MAX_LEVEL) {
						this->invalidWarning(jexpNode["Level"], "Level must be between 1~MAX_LEVEL for %s.\n", job_name.c_str());
						return 0;
					}

					if (this->nodeExists(jexpNode, "Exp")) {
						t_exp exp;

						if (!this->asUInt64(jexpNode, "Exp", exp))
							return 0;

						job->job_exp[level - 1] = exp;
					}
				}
			}

			if (this->nodeExists(node, "BonusStats")) {
				const ryml::NodeRef& bonusNode = node["BonusStats"];

				for (const ryml::NodeRef& levelNode : bonusNode) {
					uint16 level;

					if (!this->asUInt16(levelNode, "Level", level))
						return 0;

					if (level == 0 || level > MAX_LEVEL) {
						this->invalidWarning(levelNode["Level"], "Level must be between 1~MAX_LEVEL for %s.\n", job_name.c_str());
						return 0;
					}

					for (uint8 idx = PARAM_STR; idx < PARAM_MAX; idx++) {
						if (this->nodeExists(levelNode, parameter_names[idx])) {
							int16 change;

							if (!this->asInt16(levelNode, parameter_names[idx], change))
								return 0;

							job->job_bonus[level - 1][idx] = change;
						}
					}
				}
			}

#ifdef HP_SP_TABLES
			if (this->nodeExists(node, "BaseHp")) {
				for (const ryml::NodeRef& bhpNode : node["BaseHp"]) {
					uint16 level;

					if (!this->asUInt16(bhpNode, "Level", level))
						return 0;

					if (level > job->max_base_level)
						continue;

					if (level == 0 || level > MAX_LEVEL) {
						this->invalidWarning(bhpNode["Level"], "Level must be between 1~MAX_LEVEL for %s.\n", job_name.c_str());
						return 0;
					}

					if (this->nodeExists(bhpNode, "Hp")) {
						uint32 points;

						if (!this->asUInt32(bhpNode, "Hp", points))
							return 0;

						job->base_hp[level - 1] = points;
					}
				}
			}

			if (this->nodeExists(node, "BaseSp")) {
				for (const ryml::NodeRef& bspNode : node["BaseSp"]) {
					uint16 level;

					if (!this->asUInt16(bspNode, "Level", level))
						return 0;

					if (level > job->max_base_level)
						continue;

					if (level == 0 || level > MAX_LEVEL) {
						this->invalidWarning(bspNode["Level"], "Level must be between 1~MAX_LEVEL for %s.\n", job_name.c_str());
						return 0;
					}

					if (this->nodeExists(bspNode, "Sp")) {
						uint32 points;

						if (!this->asUInt32(bspNode, "Sp", points))
							return 0;

						job->base_sp[level - 1] = points;
					}
				}
			}

			if (this->nodeExists(node, "BaseAp")) {
				for (const ryml::NodeRef& bapNode : node["BaseAp"]) {
					uint16 level;

					if (!this->asUInt16(bapNode, "Level", level))
						return 0;

					if (level > job->max_base_level)
						continue;

					if (level == 0 || level > MAX_LEVEL) {
						this->invalidWarning(bapNode["Level"], "Level must be between 1~MAX_LEVEL for %s.\n", job_name.c_str());
						return 0;
					}

					if (this->nodeExists(bapNode, "Ap")) {
						uint32 points;

						if (!this->asUInt32(bapNode, "Ap", points))
							return 0;

						job->base_ap[level - 1] = points;
					}
				}
			}
#endif

			if (!exists)
				this->put(static_cast<uint16>(job_id), job);
		}
	}

	return 1;
}

void JobDatabase::loadingFinished() {
	// Checking if all class have their data
	for (auto &jobIt : *this) {
		uint16 job_id = jobIt.first;

		if (!pcdb_checkid(job_id))
			continue;
		if (job_id == JOB_WEDDING || job_id == JOB_XMAS || job_id == JOB_SUMMER || job_id == JOB_HANBOK || job_id == JOB_OKTOBERFEST || job_id == JOB_SUMMER2)
			continue; // Classes that do not need exp tables.

		std::shared_ptr<s_job_info> job = jobIt.second;
		uint16 maxBaseLv = job->max_base_level, maxJobLv = job->max_job_level;

		if (!maxBaseLv)
			ShowWarning("Class %s (%d) does not have a base exp table.\n", job_name(job_id), job_id);
		if (!maxJobLv)
			ShowWarning("Class %s (%d) does not have a job exp table.\n", job_name(job_id), job_id);

		// Init and checking the empty value of Base HP/SP [Cydh]
		if (job->base_hp.empty())
			job->base_hp.resize(maxBaseLv);
		for (uint16 j = 0; j < maxBaseLv; j++) {
			if (job->base_hp[j] == 0)
				job->base_hp[j] = pc_calc_basehp(j + 1, job_id);
		}
		if (job->base_sp.empty())
			job->base_sp.resize(maxBaseLv);
		for (uint16 j = 0; j < maxBaseLv; j++) {
			if (job->base_sp[j] == 0)
				job->base_sp[j] = pc_calc_basesp(j + 1, job_id);
		}

		// Resize to the maximum base level
		if (job->base_hp.capacity() > maxBaseLv)
			job->base_hp.erase(job->base_hp.begin() + maxBaseLv, job->base_hp.end());
		if (job->base_sp.capacity() > maxBaseLv)
			job->base_sp.erase(job->base_sp.begin() + maxBaseLv, job->base_sp.end());
		if (job->base_ap.capacity() > maxBaseLv)
			job->base_ap.erase(job->base_ap.begin() + maxBaseLv, job->base_ap.end());

		// Resize to the maximum job level
		if (job->job_bonus.capacity() > maxJobLv)
			job->job_bonus.erase(job->job_bonus.begin() + maxJobLv, job->job_bonus.end());

		for (uint16 parameter = PARAM_STR; parameter < PARAM_MAX; parameter++) {
			// Store total
			int16 current = 0;

			for (uint16 job_level = 0; job_level < maxJobLv; job_level++) {
				// Add the bonus from this job level
				current += job->job_bonus[job_level][parameter];

				// Set the new total on this job level
				job->job_bonus[job_level][parameter] = current;
			}
		}

		uint64 class_ = pc_jobid2mapid( job_id );

		// Set normal status limits
		uint16 max = battle_config.max_parameter;

		do{
			// Always check babies first
			if( class_ & JOBL_BABY ){
				if( class_ & JOBL_THIRD ){
					max = battle_config.max_baby_third_parameter;
					break;
				}else{
					max = battle_config.max_baby_parameter;
					break;
				}
			}

			// Summoner
			if( ( class_ & MAPID_BASEMASK ) == MAPID_SUMMONER ){
				max = battle_config.max_summoner_parameter;
				break;
			}

			// Extended classes
			if( ( class_ & MAPID_UPPERMASK ) == MAPID_KAGEROUOBORO || ( class_ & MAPID_UPPERMASK ) == MAPID_REBELLION ){
				max = battle_config.max_extended_parameter;
				break;
			}

			if( class_ & JOBL_FOURTH ){
				max = battle_config.max_fourth_parameter;
				break;
			}

			// 3rd class
			if( class_ & JOBL_THIRD ){
				// Transcendent
				if( class_ & JOBL_UPPER ){
					max = battle_config.max_third_trans_parameter;
					break;
				}else{
					max = battle_config.max_third_parameter;
					break;
				}
			}

			// Transcendent
			if( class_ & JOBL_UPPER ){
				max = battle_config.max_trans_parameter;
				break;
			}
		}while( false );

		for( uint16 parameter = PARAM_STR; parameter < PARAM_POW; parameter++ ){
			// If it is not explicitly set in the database file
			if( job->max_param[parameter] == 0 ){
				job->max_param[parameter] = max;
			}
		}

		// Set trait status limit
		if( class_ & JOBL_FOURTH ){
			max = battle_config.max_trait_parameter;
		}else{
			max = 0;
		}

		for( uint16 parameter = PARAM_POW; parameter < PARAM_MAX; parameter++ ){
			// If it is not explicitly set in the database file
			if( job->max_param[parameter] == 0 ){
				job->max_param[parameter] = max;
			}
		}
	}

	TypesafeCachedYamlDatabase::loadingFinished();
}

/**
 * Read job_noenter_map.txt
 **/
static bool pc_readdb_job_noenter_map(char *str[], int columns, int current) {
	int class_ = -1;
	int64 class_tmp;

	if (ISDIGIT(str[0][0])) {
		class_ = atoi(str[0]);
	} else {
		if (!script_get_constant(str[0], &class_tmp)) {
			ShowError("pc_readdb_job_noenter_map: Invalid job %s specified.\n", str[0]);
			return false;
		}
		class_ = static_cast<int>(class_tmp);
	}

	if (!pcdb_checkid(class_)) {
		ShowError("pc_readdb_job_noenter_map: Invalid job %d specified.\n", class_);
		return false;
	}

	std::shared_ptr<s_job_info> job = job_db.find(class_);

	if (job == nullptr) {
		ShowError("pc_readdb_job_noenter_map: Job %d data not initialized.\n", class_);
		return false;
	}

	job->noenter_map.zone = atoi(str[1]);
	job->noenter_map.group_lv = atoi(str[2]);
	return true;
}

const std::string PlayerStatPointDatabase::getDefaultLocation() {
	return std::string(db_path) + "/statpoint.yml";
}

uint64 PlayerStatPointDatabase::parseBodyNode(const ryml::NodeRef& node) {
	if (!this->nodesExist(node, { "Level", "Points" })) {
		return 0;
	}

	uint16 level;

	if (!this->asUInt16(node, "Level", level))
		return 0;

	uint32 point;

	if (!this->asUInt32(node, "Points", point))
		return 0;

	if (level == 0) {
		this->invalidWarning(node["Level"], "The minimum level is 1.\n");
		return 0;
	}

	if (level > MAX_LEVEL) {
		this->invalidWarning(node["Level"], "Level %d exceeds maximum BaseLevel %d, skipping.\n", level, MAX_LEVEL);
		return 0;
	}

	std::shared_ptr<s_statpoint_entry> entry = this->find( level );
	bool exists = entry != nullptr;

	if( !exists ){
		entry = std::make_shared<s_statpoint_entry>();
		entry->level = level;
		entry->statpoints = point;
	}

	if( this->nodeExists( node, "TraitPoints" ) ){
		uint32 traitpoints;

		if( !this->asUInt32( node, "TraitPoints", traitpoints ) ){
			return 0;
		}

		entry->traitpoints = traitpoints;
	}else{
		if( !exists ){
			entry->traitpoints = 0;
		}
	}

	if( !exists ){
		this->put( level, entry );
	}

	return 1;
}

/**
 * Generate the remaining parts of the db if necessary.
 */
void PlayerStatPointDatabase::loadingFinished(){
	const uint16 trait_start_level = 200;
	std::shared_ptr<s_statpoint_entry> level_one = this->find( 1 );

	if( level_one == nullptr ){
		if( battle_config.use_statpoint_table ){
			ShowError( "Missing status points for Level 1\n" );
		}

		level_one = std::make_shared<s_statpoint_entry>();

		level_one->level = 1;
		level_one->statpoints = start_status_points;
		level_one->traitpoints = 0;

		this->put( 1, level_one );
	}else if( battle_config.use_statpoint_table ){
		if( level_one->statpoints != start_status_points ){
			ShowError( "Status points for Level 1 (=%u) do not match inter_athena.conf value (=%u).\n", level_one->statpoints, start_status_points );
			level_one->statpoints = start_status_points;
		}
	}else{
		level_one->statpoints = start_status_points;
		level_one->traitpoints = 0;
	}

	std::shared_ptr<s_statpoint_entry> last_level = level_one;
	for( uint16 level = 2; level <= MAX_LEVEL; level++ ){
		std::shared_ptr<s_statpoint_entry> entry = this->find( level );
		bool exists = entry != nullptr;

		if( !exists ){
			entry = std::make_shared<s_statpoint_entry>();
			entry->level = level;
			this->put( level, entry );
		}

		if( !battle_config.use_statpoint_table || !exists ){
			if( battle_config.use_statpoint_table ){
				ShowError("Missing status points for Level %hu\n", level);
			}

			if( level <= trait_start_level ){
				entry->statpoints = last_level->statpoints + ( ( level - 1 + 15 ) / 5 );
			}else{
				entry->statpoints = last_level->statpoints;
			}
		}

		if( !battle_config.use_traitpoint_table || !exists ){
			if( battle_config.use_traitpoint_table && level > trait_start_level ){
				ShowError( "Missing trait points for Level %hu\n", level );
			}

			if( level > trait_start_level ){
				entry->traitpoints = ( level - trait_start_level ) * 3 + ( level - trait_start_level ) / 5 * 4;
			}else{
				entry->traitpoints = 0;
			}
		}

		// Store it for next iteration
		last_level = entry;
	}

	TypesafeCachedYamlDatabase::loadingFinished();
}

/*==========================================
 * pc DB reading.
 * job_stats.yml	- Job values
 * skill_tree.txt	- skill tree for every class
 * attr_fix.yml		- elemental adjustment table
 *------------------------------------------*/
void pc_readdb(void) {
	int i, s = 1;
	const char* dbsubpath[] = {
		"",
		"/" DBIMPORT,
		//add other path here
	};
		
	//reset
	job_db.clear(); // job_info table

#if defined(RENEWAL_DROP) || defined(RENEWAL_EXP)
	penalty_db.load();
#endif

	statpoint_db.clear();
	job_db.load();

	for(i=0; i<ARRAYLENGTH(dbsubpath); i++){
		uint8 n1 = (uint8)(strlen(db_path)+strlen(dbsubpath[i])+1);
		uint8 n2 = (uint8)(strlen(db_path)+strlen(DBPATH)+strlen(dbsubpath[i])+1);
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

		sv_readdb(dbsubpath2, "job_noenter_map.txt", ',', 3, 3, CLASS_COUNT, &pc_readdb_job_noenter_map, i > 0);
		aFree(dbsubpath1);
		aFree(dbsubpath2);
	}

	// Reset and read skilltree - needs to be read after pc_readdb_job_exp to get max base and job levels
	skill_tree_db.reload();

	statpoint_db.load();
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
		char buf[CHAT_SIZE_MAX];

		while( entries < MOTD_LINE_SIZE && fgets(buf, CHAT_SIZE_MAX, fp) )
		{
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
					ShowWarning("Found sequence '" CL_WHITE " :" CL_RESET "' on line '" CL_WHITE "%u" CL_RESET "' in '" CL_WHITE "%s" CL_RESET "'. This can cause newer clients to crash.\n", lines, motd_txt);
			}
			else {// empty line
				buf[0] = ' ';
				buf[1] = 0;
			}
			safestrncpy(motd_text[entries], buf, CHAT_SIZE_MAX);
			entries++;
		}
		fclose(fp);
		ShowStatus("Done reading '" CL_WHITE "%u" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", entries, motd_txt);
	}
	else
		ShowWarning("File '" CL_WHITE "%s" CL_RESET "' not found.\n", motd_txt);

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
uint8 pc_itemcd_add(struct map_session_data *sd, struct item_data *id, t_tick tick, unsigned short n) {
	int i;
	ARR_FIND(0, MAX_ITEMDELAYS, i, sd->item_delay[i].nameid == id->nameid );
	if( i == MAX_ITEMDELAYS ) /* item not found. try first empty now */
		ARR_FIND(0, MAX_ITEMDELAYS, i, !sd->item_delay[i].nameid );
	if( i < MAX_ITEMDELAYS ) {
		if( sd->item_delay[i].nameid ) {// found
			if( DIFF_TICK(sd->item_delay[i].tick, tick) > 0 ) {
				t_tick e_tick = DIFF_TICK(sd->item_delay[i].tick, tick)/1000;
				char e_msg[CHAT_SIZE_MAX];
				if( e_tick > 99 )
					sprintf(e_msg,msg_txt(sd,379), // Item Failed. [%s] is cooling down. Wait %.1f minutes.
									itemdb_ename(sd->item_delay[i].nameid), (double)e_tick / 60);
				else
					sprintf(e_msg,msg_txt(sd,380), // Item Failed. [%s] is cooling down. Wait %d seconds.
									itemdb_ename(sd->item_delay[i].nameid), e_tick+1);
				clif_messagecolor(&sd->bl,color_table[COLOR_YELLOW],e_msg,false,SELF);
				return 1; // Delay has not expired yet
			}
		} else {// not yet used item (all slots are initially empty)
			sd->item_delay[i].nameid = id->nameid;
		}
		if( !(id->nameid == ITEMID_REINS_OF_MOUNT && sd->sc.option&(OPTION_WUGRIDER|OPTION_RIDING|OPTION_DRAGON|OPTION_MADOGEAR)) )
			sd->item_delay[i].tick = tick + sd->inventory_data[n]->delay.duration;
	} else {// should not happen
		ShowError("pc_itemcd_add: Exceeded item delay array capacity! (nameid=%u, char_id=%d)\n", id->nameid, sd->status.char_id);
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
uint8 pc_itemcd_check(struct map_session_data *sd, struct item_data *id, t_tick tick, unsigned short n) {
	struct status_change *sc = NULL;

	nullpo_retr(0, sd);
	nullpo_retr(0, id);

	// Do normal delay assignment
	if (id->delay.sc <= SC_NONE || id->delay.sc >= SC_MAX || !(sc = &sd->sc))
		return pc_itemcd_add(sd, id, tick, n);

	// Send reply of delay remains
	if (sc->data[id->delay.sc]) {
		const struct TimerData *timer = get_timer(sc->data[id->delay.sc]->timer);
		clif_msg_value(sd, ITEM_REUSE_LIMIT, (int)(timer ? DIFF_TICK(timer->tick, tick) / 1000 : 99));
		return 1;
	}

	sc_start(&sd->bl, &sd->bl, id->delay.sc, 100, id->nameid, id->delay.duration);
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
		md->dmglog[i].id = 0;
		md->dmglog[i].dmg = 0;
		md->dmglog[i].flag = 0;
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

	if (!sd || !id)
		return;

	//Only store new data, don't need to renew the old one with same id
	ARR_FIND(0, DAMAGELOG_SIZE_PC, i, sd->dmglog[i] == id);
	if (i < DAMAGELOG_SIZE_PC)
		return;

	for (i = 0; i < DAMAGELOG_SIZE_PC; i++) {
		if (sd->dmglog[i] == 0) {
			sd->dmglog[i] = id;
			return;
		}
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

/**
 * Status change data arrived from char-server
 * @param sd: Player data
 */
void pc_scdata_received(struct map_session_data *sd) {
	pc_inventory_rentals(sd); // Needed here to remove rentals that have Status Changes after chrif_load_scdata has finished

	clif_weight_limit( sd );

	if( pc_has_permission( sd, PC_PERM_ATTENDANCE ) && pc_attendance_enabled() && !pc_attendance_rewarded_today( sd ) ){
		clif_ui_open( *sd, OUT_UI_ATTENDANCE, pc_attendance_counter( sd ) );
	}

	sd->state.pc_loaded = true;

	if (sd->state.connect_new == 0 && sd->fd) { // Character already loaded map! Gotta trigger LoadEndAck manually.
		sd->state.connect_new = 1;
		clif_parse_LoadEndAck(sd->fd, sd);
	}

	if (pc_iscarton(sd)) {
		sd->cart_weight_max = 0; // Force a client refesh
		status_calc_cart_weight(sd, (e_status_calc_weight_opt)(CALCWT_ITEM|CALCWT_MAXBONUS|CALCWT_CARTSTATE));
	}

	if (sd->sc.data[SC_SOULENERGY])
		sd->soulball = sd->sc.data[SC_SOULENERGY]->val1;
}

/**
 * Check player account expiration time and rental item expirations
 * @param sd: Player data
 */
void pc_check_expiration(struct map_session_data *sd) {
#ifndef ENABLE_SC_SAVING
	pc_inventory_rentals(sd); // Check here if Status Change saving is disabled
#endif

	if (sd->expiration_time != 0) { //Don't display if it's unlimited or unknow value
		time_t exp_time = sd->expiration_time;
		char tmpstr[1024];

		strftime(tmpstr,sizeof(tmpstr) - 1,msg_txt(sd,501),localtime(&exp_time)); // "Your account time limit is: %d-%m-%Y %H:%M:%S."
		clif_wis_message(sd,wisp_server_name,tmpstr,strlen(tmpstr) + 1,0);

		pc_expire_check(sd);
	}
}

TIMER_FUNC(pc_expiration_timer){
	struct map_session_data *sd = map_id2sd(id);

	if( !sd ) return 0;

	sd->expiration_tid = INVALID_TIMER;

	if( sd->fd )
		clif_authfail_fd(sd->fd,10);

	map_quit(sd);

	return 0;
}

TIMER_FUNC(pc_autotrade_timer){
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
TIMER_FUNC(pc_global_expiration_timer){
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
		chrif_save(sd, CSAVE_NORMAL);
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
		clif_messagecolor(&sd->bl,color_table[COLOR_RED],msg_txt(sd,1495),false,SELF); //You can't withdraw that much money
		return BWA_UNKNOWN_ERROR;
	}
	
	if( pc_getzeny(sd,money, LOG_TYPE_BANK, NULL) )
		return BWA_NO_MONEY;
	
	sd->bank_vault -= money;
	pc_setreg2(sd, BANK_VAULT_VAR, sd->bank_vault);
	if( save_settings&CHARSAVE_BANK )
		chrif_save(sd, CSAVE_NORMAL);
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
	t_tick now = gettick();
	struct linkdb_node *node = NULL, *next = NULL;

	if (!sd || !(node = sd->bonus_script.head))
		return;

	while (node) {
		struct s_bonus_script_entry *entry = NULL;
		next = node->next;

		if ((entry = (struct s_bonus_script_entry *)node->data)) {
			// Only start timer for new bonus_script
			if (entry->tid == INVALID_TIMER) {
				if (entry->icon != EFST_BLANK) // Gives status icon if exist
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
 * @param icon EFST
 * @param flag Flags @see enum e_bonus_script_flags
 * @param type 0 - None, 1 - Buff, 2 - Debuff
 * @return New created entry pointer or NULL if failed or NULL if duplicate fail
 * @author [Cydh]
 **/
struct s_bonus_script_entry *pc_bonus_script_add(struct map_session_data *sd, const char *script_str, t_tick dur, enum efst_type icon, uint16 flag, uint8 type) {
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
				t_tick newdur = gettick() + dur;
				if (flag&BSF_FORCE_REPLACE && entry->tick < newdur) { // Change duration
					sett_tickimer(entry->tid, newdur);
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
		if (entry->icon != EFST_BLANK)
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
TIMER_FUNC(pc_bonus_script_timer){
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
		ShowError("pc_bonus_script_timer: Invalid entry pointer %p!\n", entry);
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
void pc_bonus_script_clear(struct map_session_data *sd, uint32 flag) {
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

#ifdef RENEWAL
	enum sc_type type = SC_BASILICA_CELL;
#else
	enum sc_type type = SC_BASILICA;
#endif

	if (!map_getcell(sd->bl.m,sd->bl.x,sd->bl.y,CELL_CHKBASILICA)) {
		if (sd->sc.data[type])
			status_change_end(&sd->bl, type,INVALID_TIMER);
	}
	else if (!sd->sc.data[type])
		sc_start(&sd->bl,&sd->bl, type,100,0,INFINITE_TICK);
}

/** [Cydh]
 * Get maximum specified parameter for specified class
 * @param sd: Player data
 * @param param: Max parameter to check
 * @return max_param
 */
uint16 pc_maxparameter(struct map_session_data *sd, e_params param) {
	nullpo_retr(0, sd);

	std::shared_ptr<s_job_info> job = job_db.find(pc_mapid2jobid(sd->class_,sd->status.sex));

	if( job == nullptr || param == PARAM_MAX ){
		return 0;
	}

	return job->max_param[param];
}

/**
* Get max ASPD for player based on Class
* @param sd Player
* @return ASPD
*/
short pc_maxaspd(struct map_session_data *sd) {
	nullpo_ret(sd);

	return (( sd->class_&JOBL_THIRD) ? battle_config.max_third_aspd : (
			((sd->class_&MAPID_UPPERMASK) == MAPID_KAGEROUOBORO || (sd->class_&MAPID_UPPERMASK) == MAPID_REBELLION) ? battle_config.max_extended_aspd : (
			(sd->class_&MAPID_BASEMASK) == MAPID_SUMMONER) ? battle_config.max_summoner_aspd : 
			battle_config.max_aspd ));
}

/**
* Calculates total item-group related bonuses for the given item
* @param sd Player
* @param nameid Item ID
* @return Heal rate
**/
short pc_get_itemgroup_bonus(struct map_session_data* sd, t_itemid nameid, std::vector<s_item_bonus>& bonuses) {
	if (bonuses.empty())
		return 0;

	short bonus = 0;

	for (const auto &it : bonuses) {
		uint16 group_id = it.id;
		if (group_id == 0)
			continue;

		if (itemdb_group.item_exists(group_id, nameid))
			bonus += it.val;
	}
	return bonus;
}

/**
* Calculates total item-group related bonuses for the given item group
* @param sd Player
* @param group_id Item Group ID
* @return Heal rate
**/
short pc_get_itemgroup_bonus_group(struct map_session_data* sd, uint16 group_id, std::vector<s_item_bonus>& bonuses) {
	if (bonuses.empty())
		return 0;

	for (const auto &it : bonuses) {
		if (it.id == group_id)
			return it.val;
	}

	return 0;
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
	if (eqi == EQI_COSTUME_HEAD_MID && equip_index[EQI_COSTUME_HEAD_LOW] == index)
		return true;
	// Headgear with Top & Mid or Low location
	if (eqi == EQI_COSTUME_HEAD_TOP && (equip_index[EQI_COSTUME_HEAD_MID] == index || equip_index[EQI_COSTUME_HEAD_LOW] == index))
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

/**
 * Show available NPC Quest / Event Icon Check [Kisuka]
 * @param sd Player
 **/
void pc_show_questinfo(struct map_session_data *sd) {
#if PACKETVER >= 20090218
	nullpo_retv(sd);

	if (sd->bl.m < 0 || sd->bl.m >= MAX_MAPINDEX)
		return;

	struct map_data *mapdata = map_getmapdata(sd->bl.m);
	nullpo_retv(mapdata);

	if (mapdata->qi_npc.empty())
		return;
	if (mapdata->qi_npc.size() != sd->qi_display.size())
		return; // init was not called yet

	for (int i = 0; i < mapdata->qi_npc.size(); i++) {
		struct npc_data *nd = map_id2nd(mapdata->qi_npc[i]);

		if (!nd || nd->qi_data.empty())
			continue;

		bool show = false;

		for (auto &qi : nd->qi_data) {
			if (!qi->condition || achievement_check_condition(qi->condition, sd)) {
				show = true;
				// Check if need to be displayed
				if (!sd->qi_display[i].is_active || qi->icon != sd->qi_display[i].icon || qi->color != sd->qi_display[i].color) {
					sd->qi_display[i].is_active = true;
					sd->qi_display[i].icon = qi->icon;
					sd->qi_display[i].color = qi->color;
					clif_quest_show_event(sd, &nd->bl, qi->icon, qi->color);
				}
				break;
			}
		}
		if (show == false) {
			// Check if need to be hide
			if (sd->qi_display[i].is_active) {
				sd->qi_display[i].is_active = false;
				sd->qi_display[i].icon = QTYPE_NONE;
				sd->qi_display[i].color = QMARK_NONE;
#if PACKETVER >= 20120410
				clif_quest_show_event(sd, &nd->bl, QTYPE_NONE, QMARK_NONE);
#else
				clif_quest_show_event(sd, &nd->bl, QTYPE_QUEST, QMARK_NONE);
#endif
			}
		}
	}
#endif
}

/**
 * Reinit the questinfo for player when changing map
 * @param sd Player
 **/
void pc_show_questinfo_reinit(struct map_session_data *sd) {
#if PACKETVER >= 20090218
	nullpo_retv(sd);

	sd->qi_display.clear();

	if (sd->bl.m < 0 || sd->bl.m >= MAX_MAPINDEX)
		return;

	struct map_data *mapdata = map_getmapdata(sd->bl.m);
	nullpo_retv(mapdata);

	if (mapdata->qi_npc.empty())
		return;

	sd->qi_display.reserve( mapdata->qi_npc.size() );

	for( int i = 0; i < mapdata->qi_npc.size(); i++ ){
		sd->qi_display.push_back( s_qi_display() );
	}
#endif
}

/**
 * Check if a job is allowed to enter the map
 * @param jobid Job ID see enum e_job or sd->status.class_
 * @param m ID -an index- for direct indexing map[] array
 * @return 1 if job is allowed, 0 otherwise
 **/
bool pc_job_can_entermap(enum e_job jobid, int m, int group_lv) {
	// Map is other map server.
	// !FIXME: Currently, a map-server doesn't recognized map's attributes on other server, so we assume it's fine to warp.
	if (m < 0)
		return true;

	struct map_data *mapdata = map_getmapdata(m);

	if (!mapdata->cell)
		return false;

	if (!pcdb_checkid(jobid))
		return false;

	std::shared_ptr<s_job_info> job = job_db.find(jobid);

	if (!job->noenter_map.zone || group_lv > job->noenter_map.group_lv)
		return true;

	if ((job->noenter_map.zone&1 && !mapdata_flag_vs2(mapdata)) || // Normal
		(job->noenter_map.zone&2 && mapdata->flag[MF_PVP]) || // PVP
		(job->noenter_map.zone&4 && mapdata_flag_gvg2_no_te(mapdata)) || // GVG
		(job->noenter_map.zone&8 && mapdata->flag[MF_BATTLEGROUND]) || // Battleground
		(job->noenter_map.zone&16 && mapdata_flag_gvg2_te(mapdata)) || // WOE:TE
		(job->noenter_map.zone&(mapdata->zone) && mapdata->flag[MF_RESTRICTED]) // Zone restriction
		)
		return false;

	return true;
}

/**
 * Tells client about player's costume view on mapchange for checking 'nocostume' mapflag.
 * @param sd
 **/
void pc_set_costume_view(struct map_session_data *sd) {
	int i = -1, head_low = 0, head_mid = 0, head_top = 0, robe = 0;
	struct item_data *id = NULL;

	nullpo_retv(sd);

	head_low = sd->status.head_bottom;
	head_mid = sd->status.head_mid;
	head_top = sd->status.head_top;
	robe = sd->status.robe;

	sd->status.head_bottom = sd->status.head_mid = sd->status.head_top = sd->status.robe = 0;

	//Added check to prevent sending the same look on multiple slots ->
	//causes client to redraw item on top of itself. (suggested by Lupus)
	// Normal headgear checks
	if ((i = sd->equip_index[EQI_HEAD_LOW]) != -1 && (id = sd->inventory_data[i])) {
		if (!(id->equip&(EQP_HEAD_MID|EQP_HEAD_TOP)))
			sd->status.head_bottom = id->look;
		else
			sd->status.head_bottom = 0;
	}
	if ((i = sd->equip_index[EQI_HEAD_MID]) != -1 && (id = sd->inventory_data[i])) {
		if (!(id->equip&(EQP_HEAD_TOP)))
			sd->status.head_mid = id->look;
		else
			sd->status.head_mid = 0;
	}
	if ((i = sd->equip_index[EQI_HEAD_TOP]) != -1 && (id = sd->inventory_data[i]))
		sd->status.head_top = id->look;
	if ((i = sd->equip_index[EQI_GARMENT]) != -1 && (id = sd->inventory_data[i]))
		sd->status.robe = id->look;

	// Costumes check
	if (!map_getmapflag(sd->bl.m, MF_NOCOSTUME)) {
		if ((i = sd->equip_index[EQI_COSTUME_HEAD_LOW]) != -1 && (id = sd->inventory_data[i])) {
			if (!(id->equip&(EQP_COSTUME_HEAD_MID|EQP_COSTUME_HEAD_TOP)))
				sd->status.head_bottom = id->look;
			else
				sd->status.head_bottom = 0;
		}
		if ((i = sd->equip_index[EQI_COSTUME_HEAD_MID]) != -1 && (id = sd->inventory_data[i])) {
			if (!(id->equip&EQP_COSTUME_HEAD_TOP))
				sd->status.head_mid = id->look;
			else
				sd->status.head_mid = 0;
		}
		if ((i = sd->equip_index[EQI_COSTUME_HEAD_TOP]) != -1 && (id = sd->inventory_data[i]))
			sd->status.head_top = id->look;
		if ((i = sd->equip_index[EQI_COSTUME_GARMENT]) != -1 && (id = sd->inventory_data[i]))
			sd->status.robe = id->look;
	}

	if (sd->setlook_head_bottom)
		sd->status.head_bottom = sd->setlook_head_bottom;
	if (sd->setlook_head_mid)
		sd->status.head_mid = sd->setlook_head_mid;
	if (sd->setlook_head_top)
		sd->status.head_top = sd->setlook_head_top;
	if (sd->setlook_robe)
		sd->status.robe = sd->setlook_robe;

	if (head_low != sd->status.head_bottom)
		clif_changelook(&sd->bl, LOOK_HEAD_BOTTOM, sd->status.head_bottom);
	if (head_mid != sd->status.head_mid)
		clif_changelook(&sd->bl, LOOK_HEAD_MID, sd->status.head_mid);
	if (head_top != sd->status.head_top)
		clif_changelook(&sd->bl, LOOK_HEAD_TOP, sd->status.head_top);
	if (robe != sd->status.robe)
		clif_changelook(&sd->bl, LOOK_ROBE, sd->status.robe);
}

std::shared_ptr<s_attendance_period> pc_attendance_period(){
	uint32 date = date_get(DT_YYYYMMDD);

	for( std::pair<const uint32,std::shared_ptr<s_attendance_period>>& pair : attendance_db ){
		std::shared_ptr<s_attendance_period> period = pair.second;

		if( period->start <= date && period->end >= date ){
			return period;
		}
	}

	return nullptr;
}

bool pc_attendance_enabled(){
	// Check if the attendance feature is disabled
	if( !battle_config.feature_attendance ){
		return false;
	}

	// Check if there is a running attendance period
	return pc_attendance_period() != nullptr;
}

static inline bool pc_attendance_rewarded_today( struct map_session_data* sd ){
	return pc_readreg2( sd, ATTENDANCE_DATE_VAR ) >= date_get(DT_YYYYMMDD);
}

int32 pc_attendance_counter( struct map_session_data* sd ){
	std::shared_ptr<s_attendance_period> period = pc_attendance_period();

	// No running attendance period
	if( period == nullptr ){
		return 0;
	}

	// Get the counter for the current period
	int counter = static_cast<int>(pc_readreg2( sd, ATTENDANCE_COUNT_VAR ));

	// Check if we have a remaining counter from a previous period
	if( counter > 0 && pc_readreg2( sd, ATTENDANCE_DATE_VAR ) < period->start ){
		// Reset the counter to zero
		pc_setreg2( sd, ATTENDANCE_COUNT_VAR, 0 );

		return 0;
	}

	return 10 * counter + ( ( pc_attendance_rewarded_today(sd) ) ? 1 : 0 );
}

void pc_attendance_claim_reward( struct map_session_data* sd ){
	// If the user's group does not have the permission
	if( !pc_has_permission( sd, PC_PERM_ATTENDANCE ) ){
		return;
	}

	// Check if the attendance feature is disabled
	if( !pc_attendance_enabled() ){
		return;
	}

	// Check if the user already got his reward today
	if( pc_attendance_rewarded_today( sd ) ){
		return;
	}

	int32 attendance_counter = static_cast<int32>(pc_readreg2( sd, ATTENDANCE_COUNT_VAR ));

	attendance_counter += 1;

	std::shared_ptr<s_attendance_period> period = pc_attendance_period();

	if( period == nullptr ){
		return;
	}

	if( period->rewards.size() < attendance_counter ){
		return;
	}

	pc_setreg2( sd, ATTENDANCE_DATE_VAR, date_get(DT_YYYYMMDD) );
	pc_setreg2( sd, ATTENDANCE_COUNT_VAR, attendance_counter );

	if( save_settings&CHARSAVE_ATTENDANCE )
		chrif_save(sd, CSAVE_NORMAL);

	std::shared_ptr<s_attendance_reward> reward = period->rewards[attendance_counter - 1];

	struct mail_message msg;

	memset( &msg, 0, sizeof( struct mail_message ) );

	msg.dest_id = sd->status.char_id;
	safestrncpy( msg.send_name, msg_txt( sd, 788 ), NAME_LENGTH );
	safesnprintf( msg.title, MAIL_TITLE_LENGTH, msg_txt( sd, 789 ), attendance_counter );
	safesnprintf( msg.body, MAIL_BODY_LENGTH, msg_txt( sd, 790 ), attendance_counter );

	msg.item[0].nameid = reward->item_id;
	msg.item[0].amount = reward->amount;
	msg.item[0].identify = 1;

	msg.status = MAIL_NEW;
	msg.type = MAIL_INBOX_NORMAL;
	msg.timestamp = time(NULL);

	intif_Mail_send(0, &msg);

	clif_attendence_response( sd, attendance_counter );
}

/*==========================================
 * pc Init/Terminate
 *------------------------------------------*/
void do_final_pc(void) {
	db_destroy(itemcd_db);
	do_final_pc_groups();

	ers_destroy(pc_sc_display_ers);
	ers_destroy(num_reg_ers);
	ers_destroy(str_reg_ers);

	attendance_db.clear();
	penalty_db.clear();
}

void do_init_pc(void) {

	itemcd_db = idb_alloc(DB_OPT_RELEASE_DATA);

	pc_readdb();
	pc_read_motd(); // Read MOTD [Valaris]
	attendance_db.load();

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
	add_timer_func_list(pc_on_expire_active, "pc_on_expire_active");

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

	pc_sc_display_ers = ers_new(sizeof(struct sc_display_entry), "pc.cpp:pc_sc_display_ers", ERS_OPT_FLEX_CHUNK);
	num_reg_ers = ers_new(sizeof(struct script_reg_num), "pc.cpp:num_reg_ers", (ERSOptions)(ERS_OPT_CLEAN|ERS_OPT_FLEX_CHUNK));
	str_reg_ers = ers_new(sizeof(struct script_reg_str), "pc.cpp:str_reg_ers", (ERSOptions)(ERS_OPT_CLEAN|ERS_OPT_FLEX_CHUNK));

	ers_chunk_size(pc_sc_display_ers, 150);
	ers_chunk_size(num_reg_ers, 300);
	ers_chunk_size(str_reg_ers, 50);
}
