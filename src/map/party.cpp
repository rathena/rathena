// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "party.hpp"

#include <cstdlib>

#include <common/cbasetypes.hpp>
#include <common/malloc.hpp>
#include <common/nullpo.hpp>
#include <common/random.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp> // last_tick, session_isActive
#include <common/strlib.hpp>
#include <common/timer.hpp>
#include <common/utils.hpp>

#include "achievement.hpp"
#include "atcommand.hpp"	//msg_txt()
#include "battle.hpp"
#include "chrif.hpp" // charserver_name
#include "clif.hpp"
#include "instance.hpp"
#include "intif.hpp"
#include "log.hpp"
#include "mapreg.hpp"
#include "mob.hpp"
#include "pc.hpp"
#include "pc_groups.hpp"
#include "trade.hpp"

static DBMap* party_db; // int32 party_id -> struct party_data* (releases data)
static DBMap* party_booking_db; // uint32 char_id -> struct party_booking_ad_info* (releases data) // Party Booking [Spiria]
static unsigned long party_booking_nextid = 1;

TIMER_FUNC(party_send_xy_timer);
int32 party_create_byscript;

/*==========================================
 * Fills the given party_member structure according to the sd provided.
 * Used when creating/adding people to a party. [Skotlex]
 *------------------------------------------*/
static void party_fill_member( struct party_member& member, map_session_data& sd, uint32 leader ){
	member.account_id = sd.status.account_id;
	member.char_id = sd.status.char_id;
	safestrncpy(member.name, sd.status.name, NAME_LENGTH);
	member.class_ = sd.status.class_;
	safestrncpy( member.map, mapindex_id2name( sd.mapindex ), sizeof( member.map ) );
	member.lv = sd.status.base_level;
	member.online = 1;
	member.leader = leader;
}

/// Get the member_id of a party member.
/// Return -1 if not in party.
int32 party_getmemberid(struct party_data* p, map_session_data* sd)
{
	int32 member_id;
	nullpo_retr(-1, p);
	if( sd == nullptr )
		return -1;// no player
	ARR_FIND(0, MAX_PARTY, member_id,
		p->party.member[member_id].account_id == sd->status.account_id &&
		p->party.member[member_id].char_id == sd->status.char_id);
	if( member_id == MAX_PARTY )
		return -1;// not found
	return member_id;
}

/*==========================================
 * Request an available sd of this party
 *------------------------------------------*/
map_session_data* party_getavailablesd(struct party_data *p)
{
	int32 i;
	nullpo_retr(nullptr, p);
	ARR_FIND(0, MAX_PARTY, i, p->data[i].sd != nullptr);
	return( i < MAX_PARTY ) ? p->data[i].sd : nullptr;
}

/*==========================================
 * Retrieves and validates the sd pointer for this party member [Skotlex]
 *------------------------------------------*/

static TBL_PC* party_sd_check(int32 party_id, uint32 account_id, uint32 char_id)
{
	TBL_PC* sd = map_id2sd(account_id);

	if (!(sd && sd->status.char_id == char_id))
		return nullptr;

	if( sd->status.party_id == 0 )
		sd->status.party_id = party_id;// auto-join if not in a party
	if (sd->status.party_id != party_id)
	{	//If player belongs to a different party, kick him out.
		intif_party_leave(party_id,account_id,char_id,sd->status.name,PARTY_MEMBER_WITHDRAW_LEAVE);
		return nullptr;
	}

	return sd;
}

/*==========================================
 * Destructor
 * Called in map shutdown, cleanup var
 *------------------------------------------*/
void do_final_party(void)
{
	party_db->destroy(party_db,nullptr);
	party_booking_db->destroy(party_booking_db,nullptr); // Party Booking [Spiria]
}
// Constructor, init vars
void do_init_party(void)
{
	party_db = idb_alloc(DB_OPT_RELEASE_DATA);
	party_booking_db = idb_alloc(DB_OPT_RELEASE_DATA); // Party Booking [Spiria]
	add_timer_func_list(party_send_xy_timer, "party_send_xy_timer");
	add_timer_interval(gettick()+battle_config.party_update_interval, party_send_xy_timer, 0, 0, battle_config.party_update_interval);
}

/// Party data lookup using party id.
struct party_data* party_search(int32 party_id)
{
	if(!party_id)
		return nullptr;
	return (struct party_data*)idb_get(party_db,party_id);
}

/// Party data lookup using party name.
struct party_data* party_searchname(const char* str)
{
	struct party_data* p;

	DBIterator *iter = db_iterator(party_db);
	for( p = (struct party_data*)dbi_first(iter); dbi_exists(iter); p = (struct party_data*)dbi_next(iter) ) {
		if( strncmpi(p->party.name,str,NAME_LENGTH) == 0 )
			break;
	}
	dbi_destroy(iter);

	return p;
}

int32 party_create( map_session_data& sd, char *name, int32 item, int32 item2 ){
	struct party_member leader = {};
	char tname[NAME_LENGTH];

	safestrncpy(tname, name, NAME_LENGTH);
	trim(tname);

	if( !tname[0] ) // empty name
		return 0;

	// already associated with a party
	if( sd.status.party_id > 0 || sd.party_joining || sd.party_creating ){
		clif_party_created( sd, 2 );
		return -2;
	}

	sd.party_creating = true;
	party_fill_member( leader, sd, 1 );
	intif_create_party(&leader,name,item,item2);

	return 1;
}

void party_created(uint32 account_id,uint32 char_id,int32 fail,int32 party_id,char *name)
{
	map_session_data *sd;

	sd = map_id2sd(account_id);

	if (!sd || sd->status.char_id != char_id || !sd->party_creating ) { // Character logged off before creation ack?
		if (!fail) // break up party since player could not be added to it.
			intif_party_leave(party_id,account_id,char_id,"",PARTY_MEMBER_WITHDRAW_LEAVE);
		return;
	}

	sd->party_creating = false;

	if( !fail ) {
		sd->status.party_id = party_id;
		clif_party_created( *sd, 0 ); // Success message

		achievement_update_objective(sd, AG_PARTY, 1, 1);

		// We don't do any further work here because the char-server sends a party info packet right after creating the party
		if(party_create_byscript) {	// returns party id in $@party_create_id if party is created by script
			mapreg_setreg(add_str("$@party_create_id"),party_id);
			party_create_byscript = 0;
		}
	} else
		clif_party_created( *sd, 1 ); // "party name already exists"
}

int32 party_request_info(int32 party_id, uint32 char_id)
{
	return intif_request_partyinfo(party_id, char_id);
}

/**
 * Close trade window if party member is kicked when trade a party bound item
 * @param sd
 **/
static void party_trade_bound_cancel( map_session_data& sd ){
#ifdef BOUND_ITEMS
	if (sd.state.isBoundTrading&(1<<BOUND_PARTY))
		trade_tradecancel( &sd );
#endif
}

/// Invoked (from char-server) when the party info is not found.
int32 party_recv_noinfo(int32 party_id, uint32 char_id)
{
	party_broken(party_id);
	if( char_id != 0 ) { // requester
		map_session_data* sd;

		sd = map_charid2sd(char_id);

		if( sd && sd->status.party_id == party_id )
			sd->status.party_id = 0;
	}

	return 0;
}

static void party_check_state(struct party_data *p)
{
	int32 i;
	memset(&p->state, 0, sizeof(p->state));
	for (i = 0; i < MAX_PARTY; i ++) {
		if (!p->party.member[i].online)
			continue; //Those not online shouldn't aport to skill usage and all that.

		switch (p->party.member[i].class_) {
			case JOB_MONK:
			case JOB_BABY_MONK:
			case JOB_CHAMPION:
			case JOB_SURA:
			case JOB_SURA_T:
			case JOB_BABY_SURA:
			case JOB_INQUISITOR:
				p->state.monk = 1;
			break;
			case JOB_STAR_GLADIATOR:
			case JOB_BABY_STAR_GLADIATOR:
			case JOB_STAR_EMPEROR:
			case JOB_BABY_STAR_EMPEROR:
			case JOB_SKY_EMPEROR:
				p->state.sg = 1;
			break;
			case JOB_SUPER_NOVICE:
			case JOB_SUPER_BABY:
			case JOB_SUPER_NOVICE_E:
			case JOB_SUPER_BABY_E:
			case JOB_HYPER_NOVICE:
				p->state.snovice = 1;
			break;
			case JOB_TAEKWON:
			case JOB_BABY_TAEKWON:
				p->state.tk = 1;
			break;
		}
	}
}

int32 party_recv_info(struct party* sp, uint32 char_id)
{
	struct party_data* p;
	struct party_member* member;
	map_session_data* sd;
	int32 removed[MAX_PARTY];// member_id in old data
	int32 removed_count = 0;
	int32 added[MAX_PARTY];// member_id in new data
	int32 added_count = 0;
	int32 member_id;
	bool rename = false;

	nullpo_ret(sp);

	p = (struct party_data*)idb_get(party_db, sp->party_id);

	if( p != nullptr ) { // diff members
		int32 i;

		for( member_id = 0; member_id < MAX_PARTY; ++member_id ) {
			member = &p->party.member[member_id];

			if( member->char_id == 0 )
				continue; // empty

			ARR_FIND(0, MAX_PARTY, i,
				sp->member[i].account_id == member->account_id &&
				sp->member[i].char_id == member->char_id);

			if( i == MAX_PARTY )
				removed[removed_count++] = member_id;
			// If the member already existed, compare the old to the (possible) new name
			else if( !rename && strcmp(member->name,sp->member[i].name) )
				rename = true;
		}

		for( member_id = 0; member_id < MAX_PARTY; ++member_id ) {
			member = &sp->member[member_id];

			if( member->char_id == 0 )
				continue; // empty

			ARR_FIND(0, MAX_PARTY, i,
				p->party.member[i].account_id == member->account_id &&
				p->party.member[i].char_id == member->char_id);

			if( i == MAX_PARTY )
				added[added_count++] = member_id;
		}
	} else {
		for( member_id = 0; member_id < MAX_PARTY; ++member_id )
			if( sp->member[member_id].char_id != 0 )
				added[added_count++] = member_id;

		CREATE(p, struct party_data, 1);
		idb_put(party_db, sp->party_id, p);
	}
	while( removed_count > 0 ) { // no longer in party
		member_id = removed[--removed_count];
		sd = p->data[member_id].sd;

		if( sd == nullptr )
			continue; // not online

		party_member_withdraw(sp->party_id, sd->status.account_id, sd->status.char_id, sd->status.name, PARTY_MEMBER_WITHDRAW_LEAVE);
	}

	memcpy(&p->party, sp, sizeof(struct party));
	memset(&p->state, 0, sizeof(p->state));
	memset(&p->data, 0, sizeof(p->data));

	for( member_id = 0; member_id < MAX_PARTY; member_id++ ) {
		member = &p->party.member[member_id];
		if ( member->char_id == 0 )
			continue;// empty
		p->data[member_id].sd = party_sd_check(sp->party_id, member->account_id, member->char_id);
	}

	party_check_state(p);

	while( added_count > 0 ) { // new in party
		member_id = added[--added_count];
		sd = p->data[member_id].sd;

		if( sd == nullptr )
			continue;// not online

		clif_name_area(sd); //Update other people's display. [Skotlex]
		clif_party_member_info( *p, *sd );
		// Only send this on party creation, otherwise it will be sent by party_send_movemap [Lemongrass]
		if( sd->party_creating ){
			clif_party_option(p,sd,0x100);
		}

		clif_party_info( *p );

		if (p->instance_id > 0)
			instance_reqinfo(sd, p->instance_id);
	}
	
	// If a player was renamed, make sure to resend the party information
	if( rename ){
		clif_party_info( *p );
	}

	if( char_id != 0 ) { // requester
		sd = map_charid2sd(char_id);
		if( sd && sd->status.party_id == sp->party_id && party_getmemberid(p,sd) == -1 )
			sd->status.party_id = 0;// was not in the party
	}

	return 0;
}

///! TODO: Party invitation cross map-server through inter-server, so does with the reply.
bool party_invite( map_session_data& sd, map_session_data *tsd ){
	struct party_data* p = party_search( sd.status.party_id );

	if( p == nullptr ){
		return false;
	}

	int32 i;

	// confirm if this player is a party leader
	ARR_FIND( 0, MAX_PARTY, i, p->data[i].sd == &sd );

	if( i == MAX_PARTY || !p->party.member[i].leader ) {
		clif_displaymessage( sd.fd, msg_txt( &sd, 282 ) );
		return false;
	}

	// Party locked.
	if( map_getmapflag( sd.m, MF_PARTYLOCK ) ){
		clif_displaymessage( sd.fd, msg_txt( &sd, 227 ) );
		return false;
	}

	if( p->instance_id > 0 && battle_config.instance_block_invite ){
		clif_party_invite_reply( sd, "", PARTY_REPLY_MEMORIALDUNGEON );
		return false;
	}

	if( tsd == nullptr ){
		clif_party_invite_reply( sd, "", PARTY_REPLY_OFFLINE );
		return false;
	}

	if( tsd->status.disable_partyinvite ){
		clif_party_invite_reply( sd, tsd->status.name, PARTY_REPLY_JOINMSG_REFUSE );
		return false;
	}

	// @noask [LuzZza]
	if( tsd->state.noask ){
		clif_noask_sub( sd, *tsd, 394 ); // Autorejected party invite from %s.
		return false;
	}

	if( battle_config.block_account_in_same_party ){
		ARR_FIND(0, MAX_PARTY, i, p->party.member[i].account_id == tsd->status.account_id);
		if (i < MAX_PARTY) {
			clif_party_invite_reply( sd, tsd->status.name, PARTY_REPLY_DUAL );
			return false;
		}
	}

	// confirm if there is an open slot in the party
	ARR_FIND(0, MAX_PARTY, i, p->party.member[i].account_id == 0);

	if( i == MAX_PARTY ) {
		clif_party_invite_reply( sd, tsd->status.name, PARTY_REPLY_FULL );
		return false;
	}

	// confirm whether the account has the ability to invite before checking the player
	if( !pc_has_permission( &sd, PC_PERM_PARTY ) || !pc_has_permission( tsd, PC_PERM_PARTY ) ) {
		clif_displaymessage( sd.fd, msg_txt( &sd, 81 ) ); // Your GM level doesn't authorize you to perform this action on the specified player.
		return false;
	}

	if( !battle_config.invite_request_check && ( tsd->guild_invite > 0 || tsd->state.trading || tsd->adopt_invite ) ){
		clif_party_invite_reply( sd, tsd->status.name, PARTY_REPLY_JOIN_OTHER_PARTY );
		return false;
	}

	// You can't invite someone who has already disconnected.
	if( !session_isActive( tsd->fd ) ){
		clif_party_invite_reply( sd, tsd->status.name, PARTY_REPLY_REJECTED );
		return false;
	}

	// Already associated with a party
	if( tsd->status.party_id > 0 || tsd->party_invite > 0 ){
		clif_party_invite_reply( sd, tsd->status.name, PARTY_REPLY_JOIN_OTHER_PARTY );
		return false;
	}

	tsd->party_invite = sd.status.party_id;
	tsd->party_invite_account = sd.status.account_id;

	clif_party_invite( sd, *tsd );

	return true;
}

bool party_isleader( map_session_data* sd ){
	if( sd == nullptr ){
		return false;
	}

	if( sd->status.party_id == 0 ){
		return false;
	}

	struct party_data* party = party_search( sd->status.party_id );

	if( party == nullptr ){
		return false;
	}

	for( int32 i = 0; i < MAX_PARTY; i++ ){
		if( party->party.member[i].char_id == sd->status.char_id ){
			return party->party.member[i].leader != 0;
		}
	}

	return false;
}

void party_join( map_session_data& sd, int32 party_id ){
	// Player is in a party already now
	if( sd.status.party_id != 0 ){
		return;
	}

	// Player is already associated with a party
	if( sd.party_creating || sd.party_joining ){
		return;
	}

	struct party_data* party = party_search( party_id );

	if( party == nullptr ){
		return;
	}

	int32 i;

	if( battle_config.block_account_in_same_party ){
		ARR_FIND( 0, MAX_PARTY, i, party->party.member[i].account_id == sd.status.account_id );

		if( i < MAX_PARTY ){
			// Player is in the party with a different character already
			return;
		}
	}

	// Confirm if there is an open slot in the party
	ARR_FIND( 0, MAX_PARTY, i, party->party.member[i].account_id == 0 );

	if( i == MAX_PARTY ){
		// Party is already full
		return;
	}

	struct party_member member = {};

	sd.party_joining = true;
	party_fill_member( member, sd, 0 );
	intif_party_addmember( party_id, &member );
}

bool party_booking_load( uint32 account_id, uint32 char_id, struct s_party_booking_requirement* booking ){
	char world_name[NAME_LENGTH * 2 + 1];

	Sql_EscapeString( mmysql_handle, world_name, charserver_name );

	if( Sql_Query( mmysql_handle, "SELECT `minimum_level`, `maximum_level` FROM `%s` WHERE `world_name` = '%s' AND `account_id` = '%u' AND `char_id` = '%u'", partybookings_table, world_name, account_id, char_id ) != SQL_SUCCESS) {
		Sql_ShowDebug( mmysql_handle );

		return false;
	}

	if( Sql_NextRow( mmysql_handle ) != SQL_SUCCESS ){
		Sql_FreeResult( mmysql_handle );

		return false;
	}

	char* data;

	Sql_GetData( mmysql_handle, 0, &data, nullptr );
	booking->minimum_level = (uint16)strtoul( data, nullptr, 10 );

	Sql_GetData( mmysql_handle, 1, &data, nullptr );
	booking->maximum_level = (uint16)strtoul( data, nullptr, 10 );

	Sql_FreeResult( mmysql_handle );

	return true;
}

bool party_reply_invite( map_session_data& sd, int32 party_id, int32 flag ){
	// forged
	if( sd.party_invite != party_id ) {
		sd.party_invite = 0;
		sd.party_invite_account = 0;
		return false;
	}

	// The character is already in a party, possibly left a party invite open and created his own party
	if( sd.status.party_id != 0 ){
		// On Aegis no rejection packet is sent to the inviting player
		sd.party_invite = 0;
		sd.party_invite_account = 0;
		return false;
	}

	// accepted and allowed
	if( flag == 1 && !sd.party_creating && !sd.party_joining ) {
		struct party_data* party = party_search( party_id );

		if( party && party->instance_id > 0 && battle_config.instance_block_invite ){
			sd.party_invite = 0;
			sd.party_invite_account = 0;
			return false;
		}

		struct party_member member = {};

		sd.party_joining = true;
		party_fill_member( member, sd, 0 );
		intif_party_addmember( sd.party_invite, &member );

		return true;
	}

	// rejected or failure
	map_session_data* tsd = map_id2sd( sd.party_invite_account );

	sd.party_invite = 0;
	sd.party_invite_account = 0;

	if( tsd != nullptr ) {
		clif_party_invite_reply( *tsd, sd.status.name, PARTY_REPLY_REJECTED );
	}

	return false;
}

//Invoked when a player joins:
//- Loads up party data if not in server
//- Sets up the pointer to him
//- Player must be authed/active and belong to a party before calling this method
void party_member_joined( map_session_data& sd ){
	struct party_data* p = party_search( sd.status.party_id );
	int32 i;

	if (!p) {
		party_request_info( sd.status.party_id, sd.status.char_id );
		return;
	}

	ARR_FIND( 0, MAX_PARTY, i, p->party.member[i].account_id == sd.status.account_id && p->party.member[i].char_id == sd.status.char_id );

	if (i < MAX_PARTY) {
		p->data[i].sd = &sd;
	} else
		sd.status.party_id = 0; //He does not belongs to the party really?
}

/// Invoked (from char-server) when a new member is added to the party.
/// flag: 0-success, 1-failure
int32 party_member_added(int32 party_id,uint32 account_id,uint32 char_id, int32 flag)
{
	map_session_data *sd = map_id2sd(account_id),*sd2;
	struct party_data *p = party_search(party_id);
	int32 i;

	if(sd == nullptr || sd->status.char_id != char_id || !sd->party_joining ) {
		if (!flag) //Char logged off before being accepted into party.
			intif_party_leave(party_id,account_id,char_id,"",PARTY_MEMBER_WITHDRAW_LEAVE);
		return 0;
	}

	sd2 = map_id2sd(sd->party_invite_account);

	sd->party_joining = false;
	sd->party_invite = 0;
	sd->party_invite_account = 0;

	if (!p) {
		ShowError("party_member_added: party %d not found.\n",party_id);
		intif_party_leave(party_id,account_id,char_id,"",PARTY_MEMBER_WITHDRAW_LEAVE);
		return 0;
	}

	if( flag ) { // failed
		if( sd2 != nullptr )
			clif_party_invite_reply( *sd2, sd->status.name, PARTY_REPLY_FULL );
		return 0;
	}

	sd->status.party_id = party_id;

	clif_party_member_info( *p, *sd );
	clif_party_option(p,sd,0x100);
	clif_party_info( *p, sd );

	if( sd2 != nullptr )
		clif_party_invite_reply( *sd2, sd->status.name, PARTY_REPLY_ACCEPTED );

	for( i = 0; i < ARRAYLENGTH(p->data); ++i ) { // hp of the other party members
		sd2 = p->data[i].sd;

		if( sd2 && sd2->status.account_id != account_id && sd2->status.char_id != char_id )
			clif_hpmeter_single( *sd, sd2->id, sd2->battle_status.hp, sd2->battle_status.max_hp );
	}

	clif_party_hp( *sd );
	clif_party_xy( *sd );
	clif_name_area(sd); //Update char name's display [Skotlex]

	if (p->instance_id > 0)
		instance_reqinfo(sd, p->instance_id);

	return 0;
}

/// Party member 'sd' requesting kick of member with <account_id, name>.
bool party_removemember( map_session_data& sd, uint32 account_id, const char* name ){
	// Party locked.
	if( map_getmapflag( sd.m, MF_PARTYLOCK ) ){
		clif_displaymessage( sd.fd, msg_txt( &sd, 227 ) );
		return false;
	}

	struct party_data* p = party_search( sd.status.party_id );

	if( p == nullptr )
		return false;

	if( p->instance_id > 0 && battle_config.instance_block_expulsion ){
		return false;
	}

	int32 i;

	// check the requesting char's party membership
	ARR_FIND( 0, MAX_PARTY, i, p->party.member[i].account_id == sd.status.account_id && p->party.member[i].char_id == sd.status.char_id );
	if( i == MAX_PARTY )
		return false; // request from someone not in party? o.O
	if( !p->party.member[i].leader )
		return false; // only party leader may remove members

	ARR_FIND( 0, MAX_PARTY, i, p->party.member[i].account_id == account_id && strncmp(p->party.member[i].name,name,NAME_LENGTH) == 0 );
	if( i == MAX_PARTY )
		return false; // no such char in party

	party_trade_bound_cancel(sd);
	intif_party_leave(p->party.party_id,account_id,p->party.member[i].char_id,p->party.member[i].name,PARTY_MEMBER_WITHDRAW_EXPEL);

	return true;
}

int32 party_removemember2(map_session_data *sd,uint32 char_id,int32 party_id)
{
	if( sd ) {
		if( !sd->status.party_id )
			return -3;

		party_trade_bound_cancel( *sd );
		intif_party_leave(sd->status.party_id,sd->status.account_id,sd->status.char_id,sd->status.name,PARTY_MEMBER_WITHDRAW_EXPEL);
		return 1;
	} else {
		int32 i;
		struct party_data *p;

		if( !(p = party_search(party_id)) )
			return -2;

		ARR_FIND(0,MAX_PARTY,i,p->party.member[i].char_id == char_id );
		if( i >= MAX_PARTY )
			return -1;
		intif_party_leave(party_id,p->party.member[i].account_id,char_id,p->party.member[i].name,PARTY_MEMBER_WITHDRAW_EXPEL);
		return 1;
	}
}

/// Party member 'sd' requesting exit from party.
bool party_leave( map_session_data& sd, bool showMessage ){
	// Party locked.
	if( map_getmapflag( sd.m, MF_PARTYLOCK ) ){
		// If it was not triggered by the user itself, but from a script for example
		if( showMessage ){
			clif_displaymessage( sd.fd, msg_txt( &sd,227 ) );
		}

		return false;
	}

	struct party_data* p = party_search( sd.status.party_id );

	if( p == nullptr ){
		return false;
	}

	if( p->instance_id > 0 && battle_config.instance_block_leave ){
		// If it was not triggered by the user itself, but from a script for example
		if( showMessage ){
			clif_party_withdraw( sd, sd.status.account_id, sd.status.name, PARTY_MEMBER_WITHDRAW_CANT_LEAVE, SELF );
		}

		return false;
	}

	int32 i;

	ARR_FIND( 0, MAX_PARTY, i, p->party.member[i].account_id == sd.status.account_id && p->party.member[i].char_id == sd.status.char_id );

	if( i == MAX_PARTY ){
		return false;
	}

	party_trade_bound_cancel( sd );
	intif_party_leave( p->party.party_id, sd.status.account_id, sd.status.char_id, sd.status.name, PARTY_MEMBER_WITHDRAW_LEAVE );

	return true;
}

/// Invoked (from char-server) when a party member leaves the party.
int32 party_member_withdraw(int32 party_id, uint32 account_id, uint32 char_id, char *name, enum e_party_member_withdraw type)
{
	map_session_data* sd = map_charid2sd(char_id);
	struct party_data* p = party_search(party_id);

	if( p ) {
		map_session_data* party_sd = party_getavailablesd( p );

		if( party_sd != nullptr ){
			clif_party_withdraw( *party_sd, account_id, name, type, PARTY );
		}

		int32 i;
		ARR_FIND( 0, MAX_PARTY, i, p->party.member[i].account_id == account_id && p->party.member[i].char_id == char_id );
		if( i < MAX_PARTY ) {
			memset(&p->party.member[i], 0, sizeof(p->party.member[0]));
			memset(&p->data[i], 0, sizeof(p->data[0]));
			p->party.count--;
			party_check_state(p);
		}
	}

	if( sd && sd->status.party_id == party_id ) {
#ifdef BOUND_ITEMS
		int32 idxlist[MAX_INVENTORY]; //or malloc to reduce consumtion
		int32 j,i;

		party_trade_bound_cancel( *sd );
		j = pc_bound_chk(sd,BOUND_PARTY,idxlist);

		for(i = 0; i < j; i++)
			pc_delitem(sd,idxlist[i],sd->inventory.u.items_inventory[idxlist[i]].amount,0,1,LOG_TYPE_BOUND_REMOVAL);
#endif

		sd->status.party_id = 0;
		clif_name_area(sd); //Update name display [Skotlex]
		//TODO: hp bars should be cleared too

		if( p != nullptr && p->instance_id ){
			struct map_data *mapdata = map_getmapdata(sd->m);

			// User was on the instance map of the party
			if( mapdata != nullptr && p->instance_id == mapdata->instance_id ){
				pc_setpos_savepoint( *sd );
			}
		}
	}

	return 0;
}

/// Invoked (from char-server) when a party is disbanded.
int32 party_broken(int32 party_id)
{
	struct party_data* p;
	int32 i;

	p = party_search(party_id);

	if( p == nullptr )
		return 0;

	if( p->instance_id )
		instance_destroy( p->instance_id );

	for( i = 0; i < MAX_PARTY; i++ ) {
		if( p->data[i].sd != nullptr ) {
			clif_party_withdraw( *p->data[i].sd, p->party.member[i].account_id, p->party.member[i].name, PARTY_MEMBER_WITHDRAW_EXPEL, SELF );
			p->data[i].sd->status.party_id=0;
		}
	}

	idb_remove(party_db,party_id);

	return 1;
}

int32 party_changeoption(map_session_data *sd,int32 exp,int32 item)
{
	nullpo_ret(sd);

	if( sd->status.party_id == 0 )
		return -3;

	intif_party_changeoption(sd->status.party_id,sd->status.account_id,exp,item);

	return 0;
}

//options: 0-exp, 1-item share, 2-pickup distribution
int32 party_setoption(struct party_data *party, int32 option, int32 flag)
{
	int32 i;

	ARR_FIND(0,MAX_PARTY,i,party->party.member[i].leader);
	if(i >= MAX_PARTY)
		return 0;

	switch(option) {
		case 0:
			intif_party_changeoption(party->party.party_id,party->party.member[i].account_id,flag,party->party.item);
			break;
		case 1:
			if(flag) flag = party->party.item|1;
			else flag = party->party.item&~1;
			intif_party_changeoption(party->party.party_id,party->party.member[i].account_id,party->party.exp,flag);
			break;
		case 2:
			if(flag) flag = party->party.item|2;
			else flag = party->party.item&~2;
			intif_party_changeoption(party->party.party_id,party->party.member[i].account_id,party->party.exp,flag);
			break;
		default:
			return 0;
			break;
	}

	return 1;
}

int32 party_optionchanged(int32 party_id,uint32 account_id,int32 exp,int32 item,int32 flag)
{
	struct party_data *p;
	map_session_data *sd=map_id2sd(account_id);

	if( (p = party_search(party_id)) == nullptr)
		return 0;

	//Flag&1: Exp change denied. Flag&2: Item change denied.
	if(!(flag&0x01) && p->party.exp != exp)
		p->party.exp = exp;
	if(!(flag&0x10) && p->party.item != item) {
		p->party.item = item;
	}

	clif_party_option(p,sd,flag);

	return 0;
}

int32 party_changeleader(map_session_data *sd, map_session_data *tsd, struct party_data *p)
{
	int32 mi, tmi;

	if ( !p ) {
		if (!sd || !sd->status.party_id)
			return -1;

		if (!tsd || tsd->status.party_id != sd->status.party_id) {
			clif_displaymessage(sd->fd, msg_txt(sd,283));
			return -3;
		}

		if ( map_getmapflag(sd->m, MF_PARTYLOCK) ) {
			clif_displaymessage(sd->fd, msg_txt(sd,287));
			return 0;
		}

		if ((p = party_search(sd->status.party_id)) == nullptr )
			return -1;

		if( p->instance_id > 0 && battle_config.instance_block_leaderchange ){
			return 0;
		}

		ARR_FIND( 0, MAX_PARTY, mi, p->data[mi].sd == sd );
		if (mi == MAX_PARTY)
			return 0; // Shouldn't happen

		if (!p->party.member[mi].leader) { // Need to be a party leader.
			clif_displaymessage(sd->fd, msg_txt(sd,282));
			return 0;
		}

		ARR_FIND( 0, MAX_PARTY, tmi, p->data[tmi].sd == tsd);
		if (tmi == MAX_PARTY)
			return 0; // Shouldn't happen

		if( battle_config.change_party_leader_samemap && strncmp( p->party.member[mi].map, p->party.member[tmi].map, sizeof( p->party.member[mi].map ) ) != 0 ){
			clif_msg( *sd, MSI_PARTY_MASTER_CHANGE_SAME_MAP );
			return 0;
		}
	} else {
		ARR_FIND(0,MAX_PARTY,mi,p->party.member[mi].leader);

		if (mi == MAX_PARTY)
			return 0; // Shouldn't happen

		ARR_FIND(0,MAX_PARTY,tmi,p->data[tmi].sd ==  tsd);

		if (tmi == MAX_PARTY)
			return 0; // Shouldn't happen
	}

	// Change leadership.
	p->party.member[mi].leader = 0;

	p->party.member[tmi].leader = 1;

	// Update members
	clif_party_leaderchanged(p->data[mi].sd, p->data[mi].sd->status.account_id, p->data[tmi].sd->status.account_id);

	// Update info.
	intif_party_leaderchange(p->party.party_id,p->party.member[tmi].account_id,p->party.member[tmi].char_id);
	clif_party_info( *p );

	return 1;
}

/// Invoked (from char-server) when a party member
/// - changes maps
/// - logs in or out
/// - gains a level (disabled)
int32 party_recv_movemap( int32 party_id, uint32 account_id, uint32 char_id, int32 online, int32 lv, const char* map ){
	struct party_member* m;
	struct party_data* p;
	int32 i;

	p = party_search(party_id);

	if( p == nullptr )
		return 0;

	ARR_FIND( 0, MAX_PARTY, i, p->party.member[i].account_id == account_id && p->party.member[i].char_id == char_id );
	if( i == MAX_PARTY ) {
		ShowError("party_recv_movemap: char %d/%d not found in party %s (id:%d)",account_id,char_id,p->party.name,party_id);
		return 0;
	}

	m = &p->party.member[i];
	safestrncpy( m->map, map, sizeof( m->map ) );
	m->online = online;
	m->lv = lv;
	//Check if they still exist on this map server
	p->data[i].sd = party_sd_check(party_id, account_id, char_id);

	clif_party_info( *p );

	return 0;
}

void party_send_movemap(map_session_data *sd)
{
	struct party_data *p;

	if( !sd->status.party_id )
		return;

	intif_party_changemap(sd,1);

	p=party_search(sd->status.party_id);
	if (!p) return;

	if(sd->state.connect_new) {
		//Note that this works because this function is invoked before connect_new is cleared.
		clif_party_option(p,sd,0x100);
		clif_party_info( *p, sd );
		clif_party_member_info( *p, *sd );
	}

	if (sd->fd) { // synchronize minimap positions with the rest of the party
		int32 i;
		for(i=0; i < MAX_PARTY; i++) {
			if (p->data[i].sd &&
				p->data[i].sd != sd &&
				p->data[i].sd->m == sd->m)
			{
				clif_party_xy_single( *sd, *p->data[i].sd );
				clif_party_xy_single( *p->data[i].sd, *sd );
			}
		}
	}
	return;
}

void party_send_levelup(map_session_data *sd)
{
	intif_party_changemap(sd,1);
}

int32 party_send_logout(map_session_data *sd)
{
	struct party_data *p;
	int32 i;

	if(!sd->status.party_id)
		return 0;

	intif_party_changemap(sd,0);
	p=party_search(sd->status.party_id);
	if(!p) return 0;

	ARR_FIND( 0, MAX_PARTY, i, p->data[i].sd == sd );
	if( i < MAX_PARTY )
		memset(&p->data[i], 0, sizeof(p->data[0]));
	else
		ShowError("party_send_logout: Failed to locate member %d:%d in party %d!\n", sd->status.account_id, sd->status.char_id, p->party.party_id);

	return 1;
}

int32 party_send_message(map_session_data *sd,const char *mes, size_t len)
{
	if(sd->status.party_id == 0)
		return 0;
	intif_party_message(sd->status.party_id,sd->status.account_id,mes,len);
	party_recv_message(sd->status.party_id,sd->status.account_id,mes,len);

	// Chat logging type 'P' / Party Chat
	log_chat(LOG_CHAT_PARTY, sd->status.party_id, sd->status.char_id, sd->status.account_id, mapindex_id2name(sd->mapindex), sd->x, sd->y, nullptr, mes);

	return 0;
}

int32 party_recv_message( int32 party_id, uint32 account_id, const char *mes, size_t len ){
	struct party_data *p;
	if( (p=party_search(party_id))==nullptr)
		return 0;
	clif_party_message( *p, account_id, mes, len );
	return 0;
}

int32 party_skill_check(map_session_data *sd, int32 party_id, uint16 skill_id, uint16 skill_lv)
{
	struct party_data *p;
	map_session_data *p_sd;
	int32 i;

	if(!party_id || (p = party_search(party_id)) == nullptr)
		return 0;
	party_check_state(p);
	switch(skill_id) {
		case TK_COUNTER: //Increase Triple Attack rate of Monks.
			if (!p->state.monk) return 0;
			break;
		case MO_COMBOFINISH: //Increase Counter rate of Star Gladiators
			if (!p->state.sg) return 0;
			break;
		case AM_TWILIGHT2: //Twilight Pharmacy, requires Super Novice
			return p->state.snovice;
		case AM_TWILIGHT3: //Twilight Pharmacy, Requires Taekwon
			return p->state.tk;
		default:
			return 0; //Unknown case?
	}

	for(i = 0; i < MAX_PARTY; i++) {
		if ((p_sd = p->data[i].sd) == nullptr)
			continue;

		if (sd->m != p_sd->m)
			continue;

		switch(skill_id) {
			case TK_COUNTER: //Increase Triple Attack rate of Monks.
				if((p_sd->class_&MAPID_UPPERMASK) == MAPID_MONK
					&& pc_checkskill(p_sd,MO_TRIPLEATTACK)) {
					sc_start4(p_sd,p_sd,SC_SKILLRATE_UP,100,MO_TRIPLEATTACK,
						50+50*skill_lv, //+100/150/200% rate
						0,0,skill_get_time(SG_FRIEND, 1));
				}
				break;
			case MO_COMBOFINISH: //Increase Counter rate of Star Gladiators
				if((p_sd->class_&MAPID_UPPERMASK) == MAPID_STAR_GLADIATOR
					&& p_sd->sc.getSCE(SC_READYCOUNTER)
					&& pc_checkskill(p_sd,SG_FRIEND)) {
					sc_start4(p_sd,p_sd,SC_SKILLRATE_UP,100,TK_COUNTER,
						50+50*pc_checkskill(p_sd,SG_FRIEND), //+100/150/200% rate
						0,0,skill_get_time(SG_FRIEND, 1));
				}
				break;
		}
	}

	return 0;
}

TIMER_FUNC(party_send_xy_timer){
	struct party_data* p;

	DBIterator *iter = db_iterator(party_db);

	// for each existing party
	for( p = (struct party_data*)dbi_first(iter); dbi_exists(iter); p = (struct party_data*)dbi_next(iter) ) {
		int32 i;

		if( !p->party.count ) // no online party members so do not iterate
			continue;

		// for each member of this party
		for( i = 0; i < MAX_PARTY; i++ ) {
			map_session_data* sd = p->data[i].sd;

			if( !sd )
				continue;

			if( p->data[i].x != sd->x || p->data[i].y != sd->y ) { // perform position update
				clif_party_xy( *sd );
				p->data[i].x = sd->x;
				p->data[i].y = sd->y;
			}

			if (battle_config.party_hp_mode && p->data[i].hp != sd->battle_status.hp) { // perform hp update
				clif_party_hp( *sd );
				p->data[i].hp = sd->battle_status.hp;
			}
		}
	}
	dbi_destroy(iter);

	return 0;
}

int32 party_send_xy_clear(struct party_data *p)
{
	int32 i;

	nullpo_ret(p);

	for(i = 0; i < MAX_PARTY; i++) {
		if(!p->data[i].sd)
			continue;

		p->data[i].hp = 0;
		p->data[i].x = 0;
		p->data[i].y = 0;
	}
	return 0;
}

/** Party EXP and Zeny sharing
 * @param p Party data
 * @param src EXP source (for renewal level penalty)
 * @param base_exp Base EXP gained from killed mob
 * @param job_exp Job EXP gained from killed mob
 * @param zeny Zeny gained from killed mob
 * @author Valaris
 **/
void party_exp_share(struct party_data* p, block_list* src, t_exp base_exp, t_exp job_exp, int32 zeny)
{
	map_session_data* sd[MAX_PARTY];
	uint32 i, c;
#ifdef RENEWAL_EXP
	TBL_MOB *md = BL_CAST(BL_MOB, src);

	if (!md)
		return;
#endif

	nullpo_retv(p);

	// count the number of players eligible for exp sharing
	for (i = c = 0; i < MAX_PARTY; i++) {
		if( (sd[c] = p->data[i].sd) == nullptr || sd[c]->m != src->m || pc_isdead(sd[c]) || (battle_config.idle_no_share && pc_isidle_party(sd[c])) )
			continue;
		c++;
	}
	if (c < 1)
		return;

	base_exp/=c;
	job_exp/=c;
	zeny/=c;

	if (battle_config.party_even_share_bonus && c > 1) {
		double bonus = 100 + battle_config.party_even_share_bonus*(c-1);

		if (base_exp)
			base_exp = (t_exp) cap_value(base_exp * bonus/100, 0, MAX_EXP);
		if (job_exp)
			job_exp = (t_exp) cap_value(job_exp * bonus/100, 0, MAX_EXP);
		if (zeny)
			zeny = (uint32)cap_value(zeny * bonus/100, INT_MIN, INT_MAX);
	}

	for (i = 0; i < c; i++) {
#ifdef RENEWAL_EXP
		t_exp base_gained = base_exp, job_gained = job_exp;
		if (base_exp || job_exp) {
			int32 rate = pc_level_penalty_mod( sd[i], PENALTY_EXP, nullptr, md );
			if (rate != 100) {
				if (base_exp)
					base_gained = (t_exp)cap_value(apply_rate(base_exp, rate), 1, MAX_EXP);
				if (job_exp)
					job_gained = (t_exp)cap_value(apply_rate(job_exp, rate), 1, MAX_EXP);
			}
		}
		pc_gainexp(sd[i], src, base_gained, job_gained, 0);
#else
		pc_gainexp(sd[i], src, base_exp, job_exp, 0);
#endif

		if (zeny) // zeny from mobs [Valaris]
			pc_getzeny(sd[i],zeny,LOG_TYPE_PICKDROP_MONSTER);
	}
}

//Does party loot. first_charid holds the charid of the player who has time priority to take the item.
int32 party_share_loot(struct party_data* p, map_session_data* sd, struct item* item, int32 first_charid)
{
	TBL_PC* target = nullptr;
	int32 i;

	if (p && p->party.item&2 && (first_charid || !(battle_config.party_share_type&1))) {
		//item distribution to party members.
		if (battle_config.party_share_type&2) { // Round Robin
			TBL_PC* psd;

			i = p->itemc;

			do {
				i++;
				if (i >= MAX_PARTY)
					i = 0;	// reset counter to 1st person in party so it'll stop when it reaches "itemc"

				if( (psd = p->data[i].sd) == nullptr || sd->m != psd->m || pc_isdead(psd) || (battle_config.idle_no_share && pc_isidle_party(psd)) )
					continue;

				if (pc_additem(psd,item,item->amount,LOG_TYPE_PICKDROP_PLAYER))
					continue; //Chosen char can't pick up loot.

				//Successful pick.
				p->itemc = i;
				target = psd;
				break;
			} while (i != p->itemc);
		} else { // Random pick
			TBL_PC* psd[MAX_PARTY];
			int32 count = 0;

			//Collect pick candidates
			for (i = 0; i < MAX_PARTY; i++) {
				if( (psd[count] = p->data[i].sd) == nullptr || psd[count]->m != sd->m || pc_isdead(psd[count]) || (battle_config.idle_no_share && pc_isidle_party(psd[count])) )
					continue;

				count++;
			}

			while (count > 0) { //Pick a random member.
				i = rnd_value(0, count-1);

				if (pc_additem(psd[i],item,item->amount,LOG_TYPE_PICKDROP_PLAYER)) { // Discard this receiver.
					psd[i] = psd[count-1];
					count--;
				} else { // Successful pick.
					target = psd[i];
					break;
				}
			}
		}
	}

	if (!target) {
		target = sd; //Give it to the char that picked it up

		if ((i = pc_additem(sd,item,item->amount,LOG_TYPE_PICKDROP_PLAYER)))
			return i;
	}

	if( p && battle_config.party_show_share_picker && battle_config.show_picker_item_type&(1<<itemdb_type(item->nameid)) )
		clif_party_show_picker(target, item);

	return 0;
}

int32 party_send_dot_remove(map_session_data *sd)
{
	if (sd->status.party_id)
		clif_party_xy_remove(sd);

	return 0;
}

/**
 * Check whether a party member is in autotrade or idle for count functions
 * @param bl: Object invoking the counter
 * @param ap: List of parameters
 * @return 1 when neither autotrading and not idle or 0 otherwise
 */
int32 party_sub_count(block_list *bl, va_list ap)
{
	map_session_data *sd = (TBL_PC *)bl;

	if (sd->state.autotrade)
		return 0;

	if (battle_config.idle_no_share && pc_isidle_party(sd))
		return 0;

	return 1;
}

/**
 * To use for counting classes in a party.
 * @param bl: Object invoking the counter
 * @param ap: List of parameters: Class_Mask, Class_ID
 * @return 1 when class exists in party or 0 otherwise
 */
int32 party_sub_count_class(block_list *bl, va_list ap)
{
	map_session_data *sd = (TBL_PC *)bl;
	uint32 mask = va_arg(ap, uint32);
	uint32 mapid_class = va_arg(ap, uint32);

	if( !party_sub_count(bl, ap) )
		return 0;

	if( (sd->class_&mask) != mapid_class )
		return 0;

	return 1;
}

/// Executes 'func' for each party member on the same map and in range (0:whole map)
int32 party_foreachsamemap(int32 (*func)(block_list*,va_list),map_session_data *sd,int32 range,...)
{
	struct party_data *p;
	int32 i;
	int32 x0,y0,x1,y1;
	block_list *list[MAX_PARTY];
	int32 blockcount=0;
	int32 total = 0; //Return value.

	nullpo_ret(sd);

	if((p = party_search(sd->status.party_id)) == nullptr)
		return 0;

	x0 = sd->x-range;
	y0 = sd->y-range;
	x1 = sd->x+range;
	y1 = sd->y+range;

	for(i = 0; i < MAX_PARTY; i++) {
		map_session_data *psd = p->data[i].sd;

		if(!psd)
			continue;

		if(psd->m!=sd->m || !psd->prev)
			continue;

		if(range &&
			(psd->x<x0 || psd->y<y0 ||
			 psd->x>x1 || psd->y>y1 ) )
			continue;

		list[blockcount++]=psd;
	}

	FreeBlockLock freeLock;

	for(i = 0; i < blockcount; i++) {
		va_list ap;
		va_start(ap, range);
		total += func(list[i], ap);
		va_end(ap);
	}

	return total;
}

/*==========================================
 * Party Booking in KRO [Spiria]
 *------------------------------------------*/
static struct party_booking_ad_info* create_party_booking_data(void)
{
	struct party_booking_ad_info *pb_ad;

	CREATE(pb_ad, struct party_booking_ad_info, 1);
	pb_ad->index = party_booking_nextid++;

	return pb_ad;
}

void party_booking_register(map_session_data *sd, int16 level, int16 mapid, int16* job)
{
	struct party_booking_ad_info *pb_ad;
	int32 i;

	pb_ad = (struct party_booking_ad_info*)idb_get(party_booking_db, sd->status.char_id);

	if( pb_ad == nullptr ) {
		pb_ad = create_party_booking_data();
		idb_put(party_booking_db, sd->status.char_id, pb_ad);
	} else { // already registered
		clif_PartyBookingRegisterAck(sd, 2);
		return;
	}

	memcpy(pb_ad->charname,sd->status.name,NAME_LENGTH);
	pb_ad->starttime = (int32)time(nullptr);
	pb_ad->p_detail.level = level;
	pb_ad->p_detail.mapid = mapid;

	for(i = 0; i < MAX_PARTY_BOOKING_JOBS; i++)
		if(job[i] != 0xFF)
			pb_ad->p_detail.job[i] = job[i];
		else pb_ad->p_detail.job[i] = -1;

	clif_PartyBookingRegisterAck(sd, 0);
	clif_PartyBookingInsertNotify(sd, pb_ad); // Notice
}

void party_booking_update(map_session_data *sd, int16* job)
{
	int32 i;
	struct party_booking_ad_info *pb_ad;

	pb_ad = (struct party_booking_ad_info*)idb_get(party_booking_db, sd->status.char_id);

	if( pb_ad == nullptr )
		return;

	pb_ad->starttime = (int32)time(nullptr);// Update time.

	for(i = 0; i < MAX_PARTY_BOOKING_JOBS; i++)
		if(job[i] != 0xFF)
			pb_ad->p_detail.job[i] = job[i];
		else
			pb_ad->p_detail.job[i] = -1;

	clif_PartyBookingUpdateNotify(sd, pb_ad);
}

void party_booking_search(map_session_data *sd, int16 level, int16 mapid, int16 job, unsigned long lastindex, int16 resultcount)
{
	struct party_booking_ad_info *pb_ad;
	int32 i, count=0;
	struct party_booking_ad_info* result_list[MAX_PARTY_BOOKING_RESULTS];
	bool more_result = false;
	DBIterator* iter = db_iterator(party_booking_db);

	memset(result_list, 0, sizeof(result_list));

	for( pb_ad = (struct party_booking_ad_info*)dbi_first(iter); dbi_exists(iter); pb_ad = (struct party_booking_ad_info*)dbi_next(iter) ) {
		if (pb_ad->index < lastindex || (level && (pb_ad->p_detail.level < level-15 || pb_ad->p_detail.level > level)))
			continue;

		if (count >= MAX_PARTY_BOOKING_RESULTS) {
			more_result = true;
			break;
		}

		if (mapid == 0 && job == -1)
			result_list[count] = pb_ad;
		else if (mapid == 0) {
			for(i=0; i<MAX_PARTY_BOOKING_JOBS; i++)
				if (pb_ad->p_detail.job[i] == job && job != -1)
					result_list[count] = pb_ad;
		} else if (job == -1) {
			if (pb_ad->p_detail.mapid == mapid)
				result_list[count] = pb_ad;
		}

		if( result_list[count] )
			count++;
	}

	dbi_destroy(iter);
	clif_PartyBookingSearchAck(sd->fd, result_list, count, more_result);
}

bool party_booking_delete(map_session_data *sd)
{
	struct party_booking_ad_info* pb_ad;

	if((pb_ad = (struct party_booking_ad_info*)idb_get(party_booking_db, sd->status.char_id)) != nullptr) {
		clif_PartyBookingDeleteNotify(sd, pb_ad->index);
		idb_remove(party_booking_db,sd->status.char_id);
	}

	return true;
}
