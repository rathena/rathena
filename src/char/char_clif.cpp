// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "char_clif.hpp"

#include <cstdlib>
#include <cstring>
#include <memory>
#include <unordered_map>
#include <vector>

#include <common/malloc.hpp>
#include <common/mapindex.hpp>
#include <common/mmo.hpp>
#include <common/packets.hpp>
#include <common/random.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/sql.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>
#include <common/utilities.hpp>
#include <common/utils.hpp>

#include "char.hpp"
#include "char_logif.hpp"
#include "char_mapif.hpp"
#include "inter.hpp"

using namespace rathena;

std::vector<struct s_point_str> accessible_maps{
	s_point_str{ MAP_PRONTERA, 116, 73 },
	s_point_str{ MAP_PAYON, 162, 58 },
	s_point_str{ MAP_GEFFEN, 121, 37 },
	s_point_str{ MAP_ALDEBARAN, 167, 112 },
	s_point_str{ MAP_MORROC, 157, 45 },
	s_point_str{ MAP_COMODO, 179, 152 },
	s_point_str{ MAP_VEINS, 204, 103 },
	s_point_str{ MAP_AYOTHAYA, 218, 187 },
	s_point_str{ MAP_LIGHTHALZEN, 159, 95 },
#ifdef RENEWAL
	s_point_str{ MAP_MORA, 57, 143 }
#endif
};

void chclif_block_character( int fd, char_session_data& sd );
#if PACKETVER_SUPPORTS_PINCODE
bool pincode_allowed( char* pincode );
#endif

//------------------------------------------------
//Add On system
//------------------------------------------------
// reason
// 0: success
// 1: failed
void chclif_moveCharSlotReply( int32 fd, char_session_data& sd, uint16 index, int16 reason ){
	PACKET_HC_ACK_CHANGE_CHARACTER_SLOT p = {};

	p.packetType = HEADER_HC_ACK_CHANGE_CHARACTER_SLOT;
	p.packetLength = sizeof( p );
	p.reason = reason;
	p.moves = sd.char_moves[index];

	socket_send( fd, p );
}

/*
 * Client is requesting to move a charslot
 */
bool chclif_parse_moveCharSlot( int32 fd, char_session_data& sd ){
	const PACKET_CH_REQ_CHANGE_CHARACTER_SLOT* p = reinterpret_cast<PACKET_CH_REQ_CHANGE_CHARACTER_SLOT*>( RFIFOP( fd, 0 ) );

	uint16 from = p->slot_before;
	uint16 to = p->slot_after;

	// Bounds check
	if( from >= MAX_CHARS ){
		chclif_moveCharSlotReply( fd, sd, from, 1 );
		return 1;
	}

	// Have we changed too often or is it disabled?
	if( (charserv_config.charmove_config.char_move_enabled)==0
	|| ( (charserv_config.charmove_config.char_moves_unlimited)==0 && sd.char_moves[from] <= 0 ) ){
		chclif_moveCharSlotReply( fd, sd, from, 1 );
		return true;
	}

	// Check if there is a character on this slot
	if( sd.found_char[from] <= 0 ){
		chclif_moveCharSlotReply( fd, sd, from, 1 );
		return true;
	}

	// Bounds check
	if( to >= MAX_CHARS ){
		chclif_moveCharSlotReply( fd, sd, from, 1 );
		return 1;
	}

	// Check maximum allowed char slot for this account
	if( to >= sd.char_slots ){
		chclif_moveCharSlotReply( fd, sd, from, 1 );
		return 1;
	}

	if( sd.found_char[to] > 0 ){
		// We want to move to a used position
		if( charserv_config.charmove_config.char_movetoused ){ // TODO: check if the target is in deletion process
			// Admin is friendly and uses triangle exchange
			if( SQL_ERROR == Sql_QueryStr(sql_handle, "START TRANSACTION")
				|| SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `char_num`='%d' WHERE `char_id` = '%d'",schema_config.char_db, to, sd.found_char[from] )
				|| SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `char_num`='%d' WHERE `char_id` = '%d'", schema_config.char_db, from, sd.found_char[to] )
				|| SQL_ERROR == Sql_QueryStr(sql_handle, "COMMIT")
				){
				chclif_moveCharSlotReply( fd, sd, from, 1 );
				Sql_ShowDebug(sql_handle);
				Sql_QueryStr(sql_handle,"ROLLBACK");
				return true;
			}
		}else{
			// Admin doesn't allow us to
			chclif_moveCharSlotReply( fd, sd, from, 1 );
			return true;
		}
	}else if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `char_num`='%d' WHERE `char_id`='%d'", schema_config.char_db, to, sd.found_char[from] ) ){
		Sql_ShowDebug(sql_handle);
		chclif_moveCharSlotReply( fd, sd, from, 1 );
		return true;
	}

	if( (charserv_config.charmove_config.char_moves_unlimited)==0 ){
		sd.char_moves[from]--;
		Sql_Query(sql_handle, "UPDATE `%s` SET `moves`='%d' WHERE `char_id`='%d'", schema_config.char_db, sd.char_moves[from], sd.found_char[from] );
	}

	// We successfully moved the char - time to notify the client
	chclif_moveCharSlotReply( fd, sd, from, 0 );
	chclif_mmo_char_send(fd, sd);

	return true;
}

#if PACKETVER_SUPPORTS_PINCODE
/* pincode_sendstate transmist the pincode state to client
 * S 08b9 <seed>.L <aid>.L <state>.W (HC_SECOND_PASSWD_LOGIN)
 * state :
 *   0 = disabled / pin is correct
 *   1 = ask for pin - client sends 0x8b8
 *   2 = create new pin - client sends 0x8ba
 *   3 = pin must be changed - client 0x8be
 *   4 = create new pin - client sends 0x8ba
 *   5 = client shows msgstr(1896)
 *   6 = client shows msgstr(1897) Unable to use your KSSN number
 *   7 = char select window shows a button - client sends 0x8c5
 *   8 = pincode was incorrect
*/
void chclif_pincode_sendstate( int32 fd, char_session_data& sd, enum pincode_state state ){
	PACKET_HC_SECOND_PASSWD_LOGIN p = {};

	p.packetType = HEADER_HC_SECOND_PASSWD_LOGIN;
	p.seed = sd.pincode_seed = rnd() % 0xFFFF;
	p.AID = sd.account_id;
	p.result = state;

	socket_send( fd, p );
}

/*
 * Client just entering charserv from login, send him pincode confirmation
 */
bool chclif_parse_reqpincode_window( int32 fd, char_session_data& sd ){
	const PACKET_CH_AVAILABLE_SECOND_PASSWD* p = reinterpret_cast<PACKET_CH_AVAILABLE_SECOND_PASSWD*>( RFIFOP( fd, 0 ) );

	if( p->AID != sd.account_id ){
		return false;
	}

	if( charserv_config.pincode_config.pincode_enabled == 0 ){
		return true;
	}

	if( strlen( sd.pincode ) <= 0 ){
		chclif_pincode_sendstate( fd, sd, PINCODE_NEW );
	}else{
		chclif_pincode_sendstate( fd, sd, PINCODE_ASK );
	}

	return true;
}

/*
 * Client as anwsered pincode questionning, checking if valid anwser
 */
bool chclif_parse_pincode_check( int32 fd, char_session_data& sd ){
	const PACKET_CH_SECOND_PASSWD_ACK* p = reinterpret_cast<PACKET_CH_SECOND_PASSWD_ACK*>( RFIFOP( fd, 0 ) );

	if( charserv_config.pincode_config.pincode_enabled == 0 ){
		set_eof(fd);
		return false;
	}

	if( p->AID != sd.account_id ){
		set_eof(fd);
		return false;
	}

	char pin[PINCODE_LENGTH + 1];

	safestrncpy( pin, p->pin, PINCODE_LENGTH + 1 );

	if (!char_pincode_decrypt(sd.pincode_seed, pin )) {
		set_eof(fd);
		return false;
	}

	if( char_pincode_compare( fd, sd, pin ) ){
		sd.pincode_correct = true;
		chclif_pincode_sendstate( fd, sd, PINCODE_PASSED );
	}

	return true;
}

/*
 * Helper function to check if a new pincode contains illegal characters or combinations
 */
bool pincode_allowed( char* pincode ){
	int32 i;
	char c, n, compare[PINCODE_LENGTH+1];

	memset( compare, 0, PINCODE_LENGTH+1);

	// Sanity check for bots to prevent errors
	for( i = 0; i < PINCODE_LENGTH; i++ ){
		c = pincode[i];

		if( c < '0' || c > '9' ){
			return false;
		}
	}

	// Is it forbidden to use only the same character?
	if( !charserv_config.pincode_config.pincode_allow_repeated ){
		c = pincode[0];

		// Check if the first character equals the rest of the input
		for( i = 0; i < PINCODE_LENGTH; i++ ){
			compare[i] = c;
		}

		if( strncmp( pincode, compare, PINCODE_LENGTH + 1 ) == 0 ){
			return false;
		}
	}

	// Is it forbidden to use a sequential combination of numbers?
	if( !charserv_config.pincode_config.pincode_allow_sequential ){
		c = pincode[0];

		// Check if it is an ascending sequence
		for( i = 0; i < PINCODE_LENGTH; i++ ){
			n = c + i;

			if( n > '9' ){
				compare[i] = '0' + ( n - '9' ) - 1;
			}else{
				compare[i] = n;
			}
		}

		if( strncmp( pincode, compare, PINCODE_LENGTH + 1 ) == 0 ){
			return false;
		}

		// Check if it is an descending sequence
		for( i = 0; i < PINCODE_LENGTH; i++ ){
			n = c - i;

			if( n < '0' ){
				compare[i] = '9' - ( '0' - n ) + 1;
			}else{
				compare[i] = n;
			}
		}

		if( strncmp( pincode, compare, PINCODE_LENGTH + 1 ) == 0 ){
			return false;
		}
	}

	return true;
}

/*
 * Client request to change pincode
 */
bool chclif_parse_pincode_change( int32 fd, char_session_data& sd ){
	const PACKET_CH_EDIT_SECOND_PASSWD* p = reinterpret_cast<PACKET_CH_EDIT_SECOND_PASSWD*>( RFIFOP( fd, 0 ) );

	if( p->AID != sd.account_id ){
		set_eof(fd);
		return false;
	}

	if( charserv_config.pincode_config.pincode_enabled == 0 ){
		set_eof(fd);
		return false;
	}

	char oldpin[PINCODE_LENGTH + 1];
	char newpin[PINCODE_LENGTH + 1];

	safestrncpy( oldpin, p->old_pin, PINCODE_LENGTH + 1 );
	safestrncpy( newpin, p->new_pin, PINCODE_LENGTH + 1 );

	if (!char_pincode_decrypt(sd.pincode_seed,oldpin) || !char_pincode_decrypt(sd.pincode_seed,newpin)) {
		set_eof(fd);
		return 1;
	}

	if( !char_pincode_compare( fd, sd, oldpin ) ){
		return true;
	}

	if( !pincode_allowed( newpin ) ){
		chclif_pincode_sendstate( fd, sd, PINCODE_ILLEGAL );

		return true;
	}

	chlogif_pincode_notifyLoginPinUpdate( sd.account_id, newpin );
	sd.pincode_correct = true;

	safestrncpy( sd.pincode, newpin, sizeof( sd.pincode ) );

	ShowInfo( "Pincode changed for AID: %u\n", sd.account_id );
		
	chclif_pincode_sendstate( fd, sd, PINCODE_PASSED );

	return true;
}

/*
 * activate PIN system and set first PIN
 */
bool chclif_parse_pincode_setnew( int32 fd, char_session_data& sd ){
	const PACKET_CH_MAKE_SECOND_PASSWD* p = reinterpret_cast<PACKET_CH_MAKE_SECOND_PASSWD*>( RFIFOP( fd, 0 ) );

	if( p->AID != sd.account_id ){
		set_eof(fd);
		return false;
	}

	if( charserv_config.pincode_config.pincode_enabled == 0 ){
		set_eof(fd);
		return false;
	}

	char newpin[PINCODE_LENGTH + 1];

	safestrncpy( newpin, p->pin, PINCODE_LENGTH + 1 );

	if( !char_pincode_decrypt( sd.pincode_seed, newpin ) ){
		set_eof(fd);
		return 1;
	}

	if( !pincode_allowed( newpin ) ){
		chclif_pincode_sendstate( fd, sd, PINCODE_ILLEGAL );

		return true;
	}

	chlogif_pincode_notifyLoginPinUpdate( sd.account_id, newpin );

	safestrncpy( sd.pincode, newpin, sizeof( sd.pincode ) );

	ShowInfo( "Pincode added for AID: %u\n", sd.account_id );

	sd.pincode_correct = true;
	chclif_pincode_sendstate( fd, sd, PINCODE_PASSED );

	return true;
}
#endif

//----------------------------------------
// Tell client how many pages, kRO sends 17 (Yommy)
//----------------------------------------
void chclif_charlist_notify( int32 fd, struct char_session_data* sd ){
	PACKET_HC_CHARLIST_NOTIFY p = {};

	p.packetType = HEADER_HC_CHARLIST_NOTIFY;
	p.total = std::max( sd->char_slots / 3, 1 );
#if PACKETVER_RE_NUM >= 20151001 && PACKETVER_RE_NUM < 20180103
	p.slots = sd->char_slots;
#endif

	socket_send( fd, p );
}

//----------------------------------------
// Function to send characters to a player
//----------------------------------------
int32 chclif_mmo_send006b( int32 fd, struct char_session_data& sd ){
	if (charserv_config.save_log)
		ShowInfo( "Loading Char Data (" CL_BOLD "%d" CL_RESET ")\n", sd.account_id );

	PACKET_HC_ACCEPT_ENTER* p = reinterpret_cast<PACKET_HC_ACCEPT_ENTER*>( packet_buffer );

	p->packetType = HEADER_HC_ACCEPT_ENTER;
	p->packetLength = sizeof( *p );
#if PACKETVER >= 20100413
	// Max slots.
	p->total = MAX_CHARS;
	// Available slots. (PremiumStartSlot)
	p->premium_start = MIN_CHARS;
	// Premium slots. (Any existent chars past sd->char_slots but within MAX_CHARS will show a 'Premium Service' in red)
	p->premium_end = MIN_CHARS + sd.chars_vip;
#endif
	safestrncpy( p->extension, "", sizeof( p->extension ) );
	p->packetLength += char_mmo_chars_fromsql( sd, p->characters );

	socket_send( fd, p );

	return 0;
}

//----------------------------------------
// Notify client about charselect window data [Ind]
//----------------------------------------
void chclif_mmo_send082d( int32 fd, char_session_data& sd){
	if (charserv_config.save_log)
		ShowInfo( "Loading Char Data (" CL_BOLD "%d" CL_RESET ")\n", sd.account_id );

	PACKET_HC_ACCEPT_ENTER2* p = reinterpret_cast<PACKET_HC_ACCEPT_ENTER2*>( packet_buffer );

	p->packetType = HEADER_HC_ACCEPT_ENTER2;
	p->packetLength = sizeof( *p );
	p->normal = MIN_CHARS; // normal_slot
	p->premium = sd.chars_vip; // premium_slot
	p->billing = sd.chars_billing; // billing_slot
	p->producible = sd.char_slots; // producible_slot
	p->total = MAX_CHARS; // valid_slot
	safestrncpy( p->extension, "", sizeof( p->extension ) );

	socket_send( fd, p );
}

void chclif_mmo_send099d( int32 fd, struct char_session_data& sd ){
	uint8 count = 0;

	PACKET_HC_ACK_CHARINFO_PER_PAGE* p = reinterpret_cast<PACKET_HC_ACK_CHARINFO_PER_PAGE*>( packet_buffer );

	p->packetType = HEADER_HC_ACK_CHARINFO_PER_PAGE;
	p->packetLength = sizeof( *p );
	p->packetLength += char_mmo_chars_fromsql( sd, p->characters, &count );

	socket_send( fd, p );

	// This is something special Gravity came up with.
	// The client triggers some finalization code only if count is != 3.
	if( count == 3 ){
		p->packetLength = sizeof( *p );

		socket_send( fd, p );
	}
}


/*
 * Function to choose wich kind of charlist to send to client depending on his version
 */
void chclif_mmo_char_send( int32 fd, char_session_data& sd ){
#if PACKETVER >= 20130000
	chclif_mmo_send082d(fd, sd);
	chclif_mmo_send006b(fd, sd);
	chclif_charlist_notify( fd, &sd );
#else
	chclif_mmo_send006b(fd,sd);
	//@FIXME dump from kro doesn't show 6b transmission
#endif

#if PACKETVER >= 20060819
 	chclif_block_character(fd,sd);
#endif
}

/*
 * Transmit auth result to client
 * <result>.B ()
 * result :
 *  1 : Server closed
 *  2 : Someone has already logged in with this id
 *  3 : Time gap between client and server
 *  4 : Server is overpopulated
 *  5 : You are underaged and cannot join this server
 *  6 : You didn't pay for this account
 *  7 : Server is overpopulated
 *  8 : Already online
 *  9 : IP capacity of Internet Cafe is full
 * 10 : Out of available playing time
 * 11 : Your account is suspended
 * 12 : Connection is terminated due to changes to the billing policy
 * 13 : Connection is terminated because your IP doesn't match authorized Ip
 * 14 : Connection is terminated to prevent charging from your account's play time
 * 15 : Disconnected from server!
 * 
 */
void chclif_send_auth_result(int32 fd,char result){
	PACKET_SC_NOTIFY_BAN p = {};

	p.packetType = HEADER_SC_NOTIFY_BAN;
	p.result = result;

	socket_send( fd, p );
}

/// @param result
/// 0 (0x718): An unknown error has occurred.
/// 1: none/success
/// 3 (0x719): A database error occurred.
/// 4 (0x71a): To delete a character you must withdraw from the guild.
/// 5 (0x71b): To delete a character you must withdraw from the party.
/// Any (0x718): An unknown error has occurred.
/// HC: <0828>.W <char id>.L <Msg:0-5>.L <deleteDate>.L
void chclif_char_delete2_ack(int32 fd, uint32 char_id, uint32 result, time_t delete_date) {
	PACKET_HC_DELETE_CHAR3_RESERVED p = {};

	p.packetType = HEADER_HC_DELETE_CHAR3_RESERVED;
	p.CID = char_id;
	p.result = result;
#if PACKETVER_CHAR_DELETEDATE
	p.date = TOL( delete_date - time( nullptr ) );
#else
	p.date = TOL( delete_date );
#endif

	socket_send( fd, p );
}

/// @param result
/// 0 (0x718): An unknown error has occurred.
/// 1: none/success
/// 2 (0x71c): Due to system settings can not be deleted.
/// 3 (0x719): A database error occurred.
/// 4 (0x71d): Deleting not yet possible time.
/// 5 (0x71e): Date of birth do not match.
/// 6 Name does not match.
/// 7 Character Deletion has failed because you have entered an incorrect e-mail address.
/// Any (0x718): An unknown error has occurred.
/// HC: <082a>.W <char id>.L <Msg>.L
void chclif_char_delete2_accept_ack(int32 fd, uint32 char_id, uint32 result) {
#if PACKETVER >= 20130000
	if(result == 1 ){
		chclif_mmo_char_send(fd, *((char_session_data*)session[fd]->session_data) );
	}
#endif

	PACKET_HC_DELETE_CHAR3 p = {};

	p.packetType = HEADER_HC_DELETE_CHAR3;
	p.CID = char_id;
	p.result = result;

	socket_send( fd, p );
}

/// @param result
/// 1 (0x718): none/success, (if char id not in deletion process): An unknown error has occurred.
/// 2 (0x719): A database error occurred.
/// Any (0x718): An unknown error has occurred.
/// HC: <082c>.W <char id>.L <Msg:1-2>.L
void chclif_char_delete2_cancel_ack(int32 fd, uint32 char_id, uint32 result) {
	PACKET_HC_DELETE_CHAR3_CANCEL p = {};

	p.packetType = HEADER_HC_DELETE_CHAR3_CANCEL;
	p.CID = char_id;
	p.result = result;

	socket_send( fd, p );
}

// CH: <0827>.W <char id>.L
bool chclif_parse_char_delete2_req( int32 fd, char_session_data& sd ){
	const PACKET_CH_DELETE_CHAR3_RESERVED* p = reinterpret_cast<PACKET_CH_DELETE_CHAR3_RESERVED*>( RFIFOP( fd, 0 ) );

	uint32 char_id = p->CID;
	size_t i;

	ARR_FIND( 0, MAX_CHARS, i, sd.found_char[i] == char_id );

	// character not found
	if( i == MAX_CHARS ){
		chclif_char_delete2_ack( fd, char_id, 3, 0 );

		return true;
	}

	if( SQL_SUCCESS != Sql_Query( sql_handle, "SELECT `delete_date`,`party_id`,`guild_id` FROM `%s` WHERE `char_id`='%d'", schema_config.char_db, char_id ) ){
		Sql_ShowDebug( sql_handle );
		chclif_char_delete2_ack( fd, char_id, 3, 0 );

		return true;
	}

	// character not found
	if( SQL_SUCCESS != Sql_NextRow( sql_handle ) ){
		Sql_FreeResult( sql_handle );
		chclif_char_delete2_ack( fd, char_id, 3, 0 );

		return true;
	}

	char* data;

	Sql_GetData( sql_handle, 0, &data, nullptr);
	time_t delete_date = strtoul( data, nullptr, 10 );

	Sql_GetData( sql_handle, 1, &data, nullptr );
	uint32 party_id = strtoul( data, nullptr, 10 );

	Sql_GetData( sql_handle, 2, &data, nullptr );
	uint32 guild_id = strtoul( data, nullptr, 10 );

	Sql_FreeResult( sql_handle );

	// character already queued for deletion
	if( delete_date ){
		chclif_char_delete2_ack( fd, char_id, 0, 0 );

		return true;
	}

	// character is in guild
	if( charserv_config.char_config.char_del_restriction&CHAR_DEL_RESTRICT_GUILD && guild_id != 0 ){
		chclif_char_delete2_ack( fd, char_id, 4, 0 );
		return 1;
	}

	// character is in party
	if( charserv_config.char_config.char_del_restriction&CHAR_DEL_RESTRICT_PARTY && party_id != 0 ){
		chclif_char_delete2_ack( fd, char_id, 5, 0 );
		return 1;
	}

	// success
	delete_date = time( nullptr ) + charserv_config.char_config.char_del_delay;

	if( SQL_SUCCESS != Sql_Query( sql_handle, "UPDATE `%s` SET `delete_date`='%lu' WHERE `char_id`='%d'", schema_config.char_db, (unsigned long)delete_date, char_id ) ){
		Sql_ShowDebug( sql_handle );
		chclif_char_delete2_ack( fd, char_id, 3, 0 );

		return true;
	}

	chclif_char_delete2_ack( fd, char_id, 1, delete_date );

	return true;
}

/**
 * Check char deletion code
 * @param sd
 * @param delcode E-mail or birthdate
 * @param flag Delete flag
 * @return true:Success, false:Failure
 **/
bool chclif_delchar_check(struct char_session_data *sd, char *delcode, uint8 flag) {
	// E-Mail check
	if (flag&CHAR_DEL_EMAIL && (
			!stricmp(delcode, sd->email) || //email does not match or
			(
				!stricmp("a@a.com", sd->email) && //it is default email and
				!strcmp("", delcode) //user sent an empty email
			))) {
			ShowInfo("" CL_RED "Char Deleted" CL_RESET " " CL_GREEN "(E-Mail)" CL_RESET ".\n");
			return true;
	}
	// Birthdate (YYMMDD)
	if (flag&CHAR_DEL_BIRTHDATE && (
		!strcmp(sd->birthdate+2, delcode) || // +2 to cut off the century
		(
			!strcmp("",sd->birthdate) && // it is default birthdate and
			!strcmp("",delcode) // user sent an empty birthdate
		))) {
		ShowInfo("" CL_RED "Char Deleted" CL_RESET " " CL_GREEN "(Birthdate)" CL_RESET ".\n");
		return true;
	}
	return false;
}

// CH: <0829>.W <char id>.L <birth date:YYMMDD>.6B
bool chclif_parse_char_delete2_accept( int32 fd, char_session_data& sd ){
	const PACKET_CH_DELETE_CHAR3* p = reinterpret_cast<PACKET_CH_DELETE_CHAR3*>( RFIFOP( fd, 0 ) );

	uint32 char_id = p->CID;

	ShowInfo( CL_RED "Request Char Deletion: " CL_GREEN "%d (%d)" CL_RESET "\n", sd.account_id, char_id );

	// construct "YY-MM-DD"
	char birthdate[8 + 1];

	birthdate[0] = p->birthdate[0];
	birthdate[1] = p->birthdate[1];
	birthdate[2] = '-';
	birthdate[3] = p->birthdate[2];
	birthdate[4] = p->birthdate[3];
	birthdate[5] = '-';
	birthdate[6] = p->birthdate[4];
	birthdate[7] = p->birthdate[5];
	birthdate[8] = '\0';

	// Only check for birthdate
	if( !chclif_delchar_check( &sd, birthdate, CHAR_DEL_BIRTHDATE ) ){
		chclif_char_delete2_accept_ack( fd, char_id, 5 );

		return true;
	}

	switch( char_delete( &sd, char_id ) ){
		// success
		case CHAR_DELETE_OK:
			chclif_char_delete2_accept_ack( fd, char_id, 1 );
			break;
		// data error
		case CHAR_DELETE_DATABASE:
		// character not found
		case CHAR_DELETE_NOTFOUND:
			chclif_char_delete2_accept_ack( fd, char_id, 3 );
			break;
		// in a party
		case CHAR_DELETE_PARTY:
		// in a guild
		case CHAR_DELETE_GUILD:
		// character level config restriction
		case CHAR_DELETE_BASELEVEL:
			chclif_char_delete2_accept_ack( fd, char_id, 2 );
			break;
		// not queued or delay not yet passed
		case CHAR_DELETE_TIME:
			chclif_char_delete2_accept_ack( fd, char_id, 4 );
			break;
	}

	return true;
}

// CH: <082b>.W <char id>.L
bool chclif_parse_char_delete2_cancel( int32 fd, char_session_data& sd ){
	const PACKET_CH_DELETE_CHAR3_CANCEL* p = reinterpret_cast<PACKET_CH_DELETE_CHAR3_CANCEL*>( RFIFOP( fd, 0 ) );

	size_t i;

	ARR_FIND( 0, MAX_CHARS, i, sd.found_char[i] == p->CID );

	// character not found
	if( i == MAX_CHARS ){
		chclif_char_delete2_cancel_ack( fd, p->CID, 2 );

		return true;
	}

	// there is no need to check, whether or not the character was
	// queued for deletion, as the client prints an error message by
	// itself, if it was not the case (@see char_delete2_cancel_ack)
	if( SQL_SUCCESS != Sql_Query( sql_handle, "UPDATE `%s` SET `delete_date`='0' WHERE `char_id`='%d'", schema_config.char_db, p->CID ) ){
		Sql_ShowDebug(sql_handle);
		chclif_char_delete2_cancel_ack( fd, p->CID, 2 );

		return true;
	}

	chclif_char_delete2_cancel_ack( fd, p->CID, 1 );

	return true;
}

/*
 * Register a new mapserver into that char-serv
 * charserv can handle a MAX_SERVERS mapservs
 */
int32 chclif_parse_maplogin(int32 fd){
	if (RFIFOREST(fd) < 60)
		return 0;
	else {
		int32 i;
		char* l_user = RFIFOCP(fd,2);
		char* l_pass = RFIFOCP(fd,26);
		l_user[23] = '\0';
		l_pass[23] = '\0';
		ARR_FIND( 0, ARRAYLENGTH(map_server), i, map_server[i].fd <= 0 );
		if( !global_core->is_running() ||
			i == ARRAYLENGTH(map_server) ||
			strcmp(l_user, charserv_config.userid) != 0 ||
			strcmp(l_pass, charserv_config.passwd) != 0 )
		{
			chmapif_connectack(fd, 3); //fail
		} else {
			chmapif_connectack(fd, 0); //success

			map_server[i].fd = fd;
			map_server[i].ip = ntohl(RFIFOL(fd,54));
			map_server[i].port = ntohs(RFIFOW(fd,58));
			map_server[i].users = 0;
			map_server[i].maps = {};
			session[fd]->func_parse = chmapif_parse;
			session[fd]->flag.server = 1;
			realloc_fifo(fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);
			chmapif_init(fd);
		}
		RFIFOSKIP(fd,60);
	}
	return 0;
}

// 0065 <account id>.L <login id1>.L <login id2>.L <???>.W <sex>.B
int32 chclif_parse_reqtoconnect(int32 fd, struct char_session_data* sd,uint32 ipl){
	if( RFIFOREST(fd) < 17 ) // request to connect
		return 0;
	else {
		uint32 account_id = RFIFOL(fd,2);
		uint32 login_id1 = RFIFOL(fd,6);
		uint32 login_id2 = RFIFOL(fd,10);
		int32 sex = RFIFOB(fd,16);
		RFIFOSKIP(fd,17);

		ShowInfo("request connect - account_id:%d/login_id1:%d/login_id2:%d\n", account_id, login_id1, login_id2);

		if (sd) {
			//Received again auth packet for already authentified account?? Discard it.
			//TODO: Perhaps log this as a hack attempt?
			//TODO: and perhaps send back a reply?
			ShowInfo("Already registered break\n");
			return 1;
		}

		CREATE(session[fd]->session_data, struct char_session_data, 1);
		sd = (struct char_session_data*)session[fd]->session_data;
		sd->account_id = account_id;
		sd->login_id1 = login_id1;
		sd->login_id2 = login_id2;
		sd->sex = sex;
		sd->auth = false; // not authed yet
		sd->pincode_correct = false; // not entered pincode correctly yet

		// send back account_id
		WFIFOHEAD(fd,4);
		WFIFOL(fd,0) = account_id;
		WFIFOSET(fd,4);

		if( !global_core->is_running() ){
			chclif_reject(fd, 0); // rejected from server
			return 1;
		}

		// search authentification
		std::shared_ptr<struct auth_node> node = util::umap_find( char_get_authdb(), account_id);

		if( node != nullptr &&
			node->account_id == account_id &&
			node->login_id1  == login_id1 &&
			node->login_id2  == login_id2 /*&&
			node->ip         == ipl*/ )
		{// authentication found (coming from map server)
			char_get_authdb().erase(account_id);
			char_auth_ok(fd, sd);
			sd->pincode_correct = true; // already entered pincode correctly yet
		}
		else
		{// authentication not found (coming from login server)
			if (session_isValid(login_fd)) { // don't send request if no login-server
				WFIFOHEAD(login_fd,23);
				WFIFOW(login_fd,0) = 0x2712; // ask login-server to authentify an account
				WFIFOL(login_fd,2) = sd->account_id;
				WFIFOL(login_fd,6) = sd->login_id1;
				WFIFOL(login_fd,10) = sd->login_id2;
				WFIFOB(login_fd,14) = sd->sex;
				WFIFOL(login_fd,15) = htonl(ipl);
				WFIFOL(login_fd,19) = fd;
				WFIFOSET(login_fd,23);
			} else { // if no login-server, we must refuse connection
				chclif_reject(fd, 0); // rejected from server
			}
		}
	}
	return 1;
}

bool chclif_parse_req_charlist( int32 fd, char_session_data& sd ){
	const PACKET_CH_CHARLIST_REQ* p = reinterpret_cast<PACKET_CH_CHARLIST_REQ*>( RFIFOP( fd, 0 ) );

	chclif_mmo_send099d( fd, sd );

	return true;
}

//Send player to map
void chclif_send_map_data( int32 fd, std::shared_ptr<struct mmo_charstatus> cd, int32 map_server_index ){
	PACKET_HC_NOTIFY_ZONESVR p = {};

	p.packetType = HEADER_HC_NOTIFY_ZONESVR;
	p.CID = cd->char_id;
	mapindex_getmapname_ext( cd->last_point.map, p.mapname );
	uint32 subnet_map_ip = char_lan_subnetcheck( session[fd]->client_addr ); // Advanced subnet check [LuzZza]
	p.ip = htonl( ( subnet_map_ip ) ? subnet_map_ip : map_server[map_server_index].ip );
	p.port = ntows( htons( map_server[map_server_index].port ) ); // [!] LE byte order here [!]
#if PACKETVER >= 20170315
	safestrncpy( p.domain, "", sizeof( p.domain ) );
#endif
#ifdef DEBUG
	ShowDebug("Sending the client (%d %d.%d.%d.%d) to map-server with ip %d.%d.%d.%d and port %hu\n",
			  cd->account_id, CONVIP( session[fd]->client_addr ), CONVIP((subnet_map_ip) ? subnet_map_ip : map_server[map_server_index].ip),
			  map_server[map_server_index].port);
#endif

	socket_send( fd, p );
}

bool chclif_parse_select_accessible_map( int32 fd, struct char_session_data& sd ){
#if PACKETVER >= 20100714
	const PACKET_CH_SELECT_ACCESSIBLE_MAPNAME* p = reinterpret_cast<PACKET_CH_SELECT_ACCESSIBLE_MAPNAME*>( RFIFOP( fd, 0 ) );

	char* data;

	// Check if the character exists and is not scheduled for deletion
	if( SQL_SUCCESS != Sql_Query( sql_handle, "SELECT `char_id` FROM `%s` WHERE `account_id`='%d' AND `char_num`='%d' AND `delete_date` = 0", schema_config.char_db, sd.account_id, p->slot )
		|| SQL_SUCCESS != Sql_NextRow( sql_handle )
		|| SQL_SUCCESS != Sql_GetData( sql_handle, 0, &data, nullptr ) ){
		// Not found?? May be forged packet.
		Sql_ShowDebug( sql_handle );
		Sql_FreeResult( sql_handle );
		chclif_reject( fd, 0 ); // rejected from server
		return 1;
	}

	uint32 char_id = atoi( data );
	Sql_FreeResult( sql_handle );

	// Prevent select a char while retrieving guild bound items
	if( sd.flag&1 ){
		chclif_reject( fd, 0 ); // rejected from server
		return 1;
	}

	/* client doesn't let it get to this point if you're banned, so its a forged packet */
	if( sd.found_char[p->slot] == char_id && sd.unban_time[p->slot] > time( nullptr ) ) {
		chclif_reject( fd, 0 ); // rejected from server
		return 1;
	}

	/* set char as online prior to loading its data so 3rd party applications will realise the sql data is not reliable */
	char_set_char_online( -2, char_id, sd.account_id );

	struct mmo_charstatus char_dat;

	if( !char_mmo_char_fromsql( char_id, &char_dat, true ) ) {
		/* failed? set it back offline */
		char_set_char_offline( char_id, sd.account_id );
		/* failed to load something. REJECT! */
		chclif_reject( fd, 0 ); // rejected from server
		return 1;
	}

	// Have to switch over to the DB instance otherwise data won't propagate [Kevin]
	std::shared_ptr<struct mmo_charstatus> cd = util::umap_find( char_get_chardb(), char_id );

	if( charserv_config.log_char ){
		char esc_name[NAME_LENGTH*2+1];

		Sql_EscapeStringLen( sql_handle, esc_name, char_dat.name, strnlen( char_dat.name, NAME_LENGTH ) );

		if( SQL_ERROR == Sql_Query( sql_handle, "INSERT INTO `%s`(`time`, `account_id`,`char_num`,`name`) VALUES (NOW(), '%d', '%d', '%s')", schema_config.charlog_db, sd.account_id, p->slot, esc_name ) ){
			Sql_ShowDebug( sql_handle );
		}
	}

	ShowInfo( "Selected char: (Account %d: %d - %s)\n", sd.account_id, p->slot, char_dat.name );

	// Check if there is really no mapserver for the last point where the player was
	int32 mapserver = char_search_mapserver( cd->last_point.map, -1, -1 );

	// It was not an unavailable map
	if( mapserver >= 0 ){
		chclif_reject( fd, 0 ); // rejected from server
		return 1;
	}

	if( static_cast<size_t>( p->mapnumber ) >= accessible_maps.size() ){
		chclif_reject( fd, 0 ); // rejected from server
		return 1;
	}

	s_point_str& accessible_map = accessible_maps[p->mapnumber];

	safestrncpy( cd->last_point.map, accessible_map.map, sizeof( cd->last_point.map ) );
	cd->last_point.x = accessible_map.x;
	cd->last_point.y = accessible_map.y;

	mapserver = char_search_mapserver( cd->last_point.map, -1, -1 );

	// No mapserver found for our accessible map
	if( mapserver < 0 ){
		chclif_reject( fd, 0 ); // rejected from server
		return 1;
	}

	int32 map_fd;

	// Send NEW auth packet [Kevin]
	// FIXME: is this case even possible? [ultramage]
	if( ( map_fd = map_server[mapserver].fd ) < 1 || session[map_fd] == nullptr ){
		ShowError( "parse_char: Attempting to write to invalid session %d! Map Server #%d disconnected.\n", map_fd, mapserver );
		map_server[mapserver] = {};
		map_server[mapserver].fd = -1;
		chclif_send_auth_result( fd, 1 ); // Send server closed.
		return 1;
	}

	chclif_send_map_data( fd, cd, mapserver );

	// create temporary auth entry
	std::shared_ptr<struct auth_node> node = std::make_shared<struct auth_node>();

	node->account_id = sd.account_id;
	node->char_id = cd->char_id;
	node->login_id1 = sd.login_id1;
	node->login_id2 = sd.login_id2;
	node->sex = sd.sex;
	node->expiration_time = sd.expiration_time;
	node->group_id = sd.group_id;
	node->ip = session[fd]->client_addr;

	char_get_authdb()[node->account_id] = node;

	return true;
#else
	return false;
#endif
}

void chclif_accessible_maps( int32 fd ){
#if PACKETVER >= 20100714
	PACKET_HC_NOTIFY_ACCESSIBLE_MAPNAME* p = reinterpret_cast<PACKET_HC_NOTIFY_ACCESSIBLE_MAPNAME*>( packet_buffer );

	p->packetType = HEADER_HC_NOTIFY_ACCESSIBLE_MAPNAME;
	p->packetLength = sizeof( *p );

	int32 count = 0;
	for( s_point_str& accessible_map : accessible_maps ){
		PACKET_HC_NOTIFY_ACCESSIBLE_MAPNAME_sub& entry = p->maps[count];

		if( int32 mapserver = char_search_mapserver( accessible_map.map, -1, -1 ); mapserver < 0 ){
			entry.status = 1;
		}else{
			entry.status = 0;
		}

		mapindex_getmapname_ext( accessible_map.map, entry.map );

		p->packetLength += static_cast<decltype(p->packetLength)>( sizeof( entry ) );
		count++;
	}

	socket_send( fd, p );
#else
	chclif_send_auth_result( fd, 1 ); // 01 = Server closed
#endif
}

bool chclif_parse_charselect( int32 fd, struct char_session_data& sd ){
	const PACKET_CH_SELECT_CHAR* p = reinterpret_cast<PACKET_CH_SELECT_CHAR*>( RFIFOP( fd, 0 ) );

	int32 server_id;

	ARR_FIND( 0, ARRAYLENGTH(map_server), server_id, session_isValid(map_server[server_id].fd) && !map_server[server_id].maps.empty() );
	// Map-server not available, tell the client to wait (client wont close, char select will respawn)
	if (server_id == ARRAYLENGTH(map_server)) {
		chclif_accessible_maps( fd );
		return 1;
	}

	// Check if the character exists and is not scheduled for deletion
	int slot = p->slot;
	char* data;

	if ( SQL_SUCCESS != Sql_Query(sql_handle, "SELECT `char_id` FROM `%s` WHERE `account_id`='%d' AND `char_num`='%d' AND `delete_date` = 0", schema_config.char_db, sd.account_id, slot)
		|| SQL_SUCCESS != Sql_NextRow(sql_handle)
		|| SQL_SUCCESS != Sql_GetData(sql_handle, 0, &data, NULL) )
	{	//Not found?? May be forged packet.
		Sql_ShowDebug(sql_handle);
		Sql_FreeResult(sql_handle);
		chclif_reject(fd, 0); // rejected from server
		return 1;
	}

	uint32 char_id = atoi(data);
	Sql_FreeResult(sql_handle);

	// Prevent select a char while retrieving guild bound items
	if (sd.flag&1) {
		chclif_reject(fd, 0); // rejected from server
		return 1;
	}

	/* client doesn't let it get to this point if you're banned, so its a forged packet */
	if( sd.found_char[slot] == char_id && sd.unban_time[slot] > time(nullptr) ) {
		chclif_reject(fd, 0); // rejected from server
		return 1;
	}

	/* set char as online prior to loading its data so 3rd party applications will realise the sql data is not reliable */
	char_set_char_online(-2,char_id,sd.account_id);

	struct mmo_charstatus char_dat;
	if( !char_mmo_char_fromsql(char_id, &char_dat, true) ) { /* failed? set it back offline */
		char_set_char_offline(char_id, sd.account_id);
		/* failed to load something. REJECT! */
		chclif_reject(fd, 0); // rejected from server
		return 1;
	}

	//Have to switch over to the DB instance otherwise data won't propagate [Kevin]
	std::shared_ptr<struct mmo_charstatus> cd = util::umap_find( char_get_chardb(), char_id );

	if (charserv_config.log_char) {
		char esc_name[NAME_LENGTH*2+1];

		Sql_EscapeStringLen(sql_handle, esc_name, char_dat.name, strnlen(char_dat.name, NAME_LENGTH));
		if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `%s`(`time`, `account_id`,`char_num`,`name`) VALUES (NOW(), '%d', '%d', '%s')",
			schema_config.charlog_db, sd.account_id, slot, esc_name) )
			Sql_ShowDebug(sql_handle);
	}
	ShowInfo("Selected char: (Account %d: %d - %s)\n", sd.account_id, slot, char_dat.name);

	// searching map server
	int i = char_search_mapserver( cd->last_point.map, -1, -1 );

	// if map is not found, we check major cities
	if( i < 0 ){
#if PACKETVER >= 20100714
		// Let the user select a map
		chclif_accessible_maps( fd );

		return 0;
#else
		// Try to select a map for the user
		uint16 j;
		//First check that there's actually a map server online.
		ARR_FIND( 0, ARRAYLENGTH(map_server), j, session_isValid(map_server[j].fd) && !map_server[j].maps.empty() );
		if (j == ARRAYLENGTH(map_server)) {
			ShowInfo("Connection Closed. No map servers available.\n");
			chclif_send_auth_result(fd,1); // 01 = Server closed
			return 1;
		}

		for( struct s_point_str& accessible_map : accessible_maps ){
			i = char_search_mapserver( accessible_map.map, -1, -1 );

			// Found a map-server for a map
			if( i >= 0 ){
				ShowWarning( "Unable to find map-server for '%s', sending to major city '%s'.\n", cd->last_point.map, accessible_map.map );
				memcpy( &cd->last_point, &accessible_map, sizeof( cd->last_point ) );
				break;
			}
		}

		if( i < 0 ){
			ShowInfo( "Connection Closed. No map server available that has a major city, and unable to find map-server for '%s'.\n", cd->last_point.map );
			chclif_send_auth_result(fd,1); // 01 = Server closed
			return 1;
		}
#endif
	}

	//Send NEW auth packet [Kevin]
	//FIXME: is this case even possible? [ultramage]
	if( !session_isValid( map_server[i].fd ) ){
		ShowError( "parse_char: Attempting to write to invalid session %d! Map Server #%d disconnected.\n", map_server[i].fd, i );
		map_server[i] = {};
		map_server[i].fd = -1;
		chclif_send_auth_result(fd,1);  //Send server closed.
		return 1;
	}

	chclif_send_map_data( fd, cd, i );

	// create temporary auth entry
	std::shared_ptr<struct auth_node> node = std::make_shared<struct auth_node>();

	node->account_id = sd.account_id;
	node->char_id = cd->char_id;
	node->login_id1 = sd.login_id1;
	node->login_id2 = sd.login_id2;
	node->sex = sd.sex;
	node->expiration_time = sd.expiration_time;
	node->group_id = sd.group_id;
	node->ip = session[fd]->client_addr;

	char_get_authdb()[node->account_id] = node;

	return 1;
}

void chclif_createnewchar_refuse( int fd, int error ){
	// deny character creation
	PACKET_HC_REFUSE_MAKECHAR p = {};

	p.packetType = HEADER_HC_REFUSE_MAKECHAR;

	switch ( error ) {
		// 'Charname already exists' (-1)
		case -1:
			p.error = 0x00;
			break;
		// 'Char creation denied' (-2)
		case -2:
			p.error = 0xFF;
			break;
		// 'You are underaged' (-3)
		case -3:
			p.error = 0x01;
			break;
		//  'You are not elegible to open the Character Slot' (-4)
		case -4:
			p.error = 0x03;
			break;
		/* Unused: 0x02 = Symbols in Character Names are forbidden [Ind]*/
	}

	socket_send( fd, p );
}

void chclif_createnewchar( int fd, mmo_charstatus& char_dat ){
	// send to player
	PACKET_HC_ACCEPT_MAKECHAR p = {};

	p.packetType = HEADER_HC_ACCEPT_MAKECHAR;
	char_mmo_char_tobuf( p.character, char_dat );

	socket_send( fd, p );
}

// S 0970 <name>.24B <slot>.B <hair color>.W <hair style>.W
// S 0067 <name>.24B <str>.B <agi>.B <vit>.B <int>.B <dex>.B <luk>.B <slot>.B <hair color>.W <hair style>.W
// S 0a39 <name>.24B <slot>.B <hair color>.W <hair style>.W <starting job ID>.W <Unknown>.(W or 2 B's)??? <sex>.B
bool chclif_parse_createnewchar( int32 fd, struct char_session_data& sd ){
	// Check if character creation is turned off
	if( charserv_config.char_new == 0 ){ 
		chclif_createnewchar_refuse( fd, -2 );
		return true;
	}

	char name[NAME_LENGTH];
	int32 str, agi, vit, int_, dex, luk;
	int32 slot;
	int32 hair_color;
	int32 hair_style;
	int32 start_job;
	int32 sex;

	const PACKET_CH_MAKE_CHAR* p = reinterpret_cast<PACKET_CH_MAKE_CHAR*>( RFIFOP( fd, 0 ) );

	// Sent values
	safestrncpy( name, p->name, NAME_LENGTH );
	slot = p->slot;
	hair_color = p->hair_color;
	hair_style = p->hair_style;

#if PACKETVER >= 20151001
	// Sent values
	start_job = p->job;
	sex = p->sex;

	// Default values
	str = 1;
	agi = 1;
	vit = 1;
	int_ = 1;
	dex = 1;
	luk = 1;
#elif PACKETVER >= 20120307
	// Default values
	str = 1;
	agi = 1;
	vit = 1;
	int_ = 1;
	dex = 1;
	luk = 1;
	start_job = JOB_NOVICE;
	sex = sd->sex;
#else
	// Sent values
	str = p->str;
	agi = p->agi;
	vit = p->vit;
	int_ = p->int_;
	dex = p->dex;
	luk = p->luk;

	// Default values
	start_job = JOB_NOVICE;
	sex = sd->sex;
#endif

	int char_id = char_make_new_char( &sd, name, str, agi, vit, int_, dex, luk, slot, hair_color, hair_style, start_job, sex );

	if( char_id < 0 ){
		chclif_createnewchar_refuse( fd, char_id );
		return true;
	}

	// retrieve data
	struct mmo_charstatus char_dat;

	// Only the short data is needed.
	char_mmo_char_fromsql( char_id, &char_dat, false );

	chclif_createnewchar( fd, char_dat );

	// add new entry to the chars list
	sd.found_char[char_dat.slot] = char_id;

	return true;
}

/**
 * Inform client that his deletion request was refused
 * 0x70 <ErrorCode>B HC_REFUSE_DELETECHAR
 * @param fd
 * @param ErrorCode
 *	00 = Incorrect Email address
 *	01 = Invalid Slot
 *	02 = In a party or guild
 */
void chclif_refuse_delchar(int32 fd, uint8 errCode){
	PACKET_HC_REFUSE_DELETECHAR p = {};

	p.packetType = HEADER_HC_REFUSE_DELETECHAR;
	p.error = errCode;

	socket_send( fd, p );
}

void chclif_delchar( int32 fd ){
	PACKET_HC_ACCEPT_DELETECHAR p = {};

	p.packetType = HEADER_HC_ACCEPT_DELETECHAR;

	socket_send( fd, p );
}

bool chclif_parse_delchar( int fd, struct char_session_data& sd ){
	const PACKET_CH_DELETE_CHAR* p = reinterpret_cast<PACKET_CH_DELETE_CHAR*>( RFIFOP( fd, 0 ) );

	char email[40];
	uint32 cid = p->CID;

	ShowInfo(CL_RED "Request Char Deletion: " CL_GREEN "%u (%u)" CL_RESET "\n", sd.account_id, cid);
	safestrncpy( email, p->key, sizeof( email ) );

	if (!chclif_delchar_check(&sd, email, charserv_config.char_config.char_del_option)) {
		chclif_refuse_delchar(fd,0); // 00 = Incorrect Email address
		return true;
	}

	/* Delete character */
	switch( char_delete(&sd,cid) ){
		case CHAR_DELETE_OK:
			// Char successfully deleted.
			chclif_delchar( fd );
			break;
		case CHAR_DELETE_DATABASE:
		case CHAR_DELETE_BASELEVEL:
		case CHAR_DELETE_TIME:
			chclif_refuse_delchar(fd, 0);
			break;
		case CHAR_DELETE_NOTFOUND:
			chclif_refuse_delchar(fd, 1);
			break;
		case CHAR_DELETE_GUILD:
		case CHAR_DELETE_PARTY:
			chclif_refuse_delchar(fd, 2);
			break;
	}

	return true;
}

// Client keep-alive packet (every 12 seconds)
// R 0187 <account ID>.l
bool chclif_parse_keepalive( int32 fd, char_session_data& sd ){
	const PACKET_PING* p = reinterpret_cast<PACKET_PING*>( RFIFOP( fd, 0 ) );

	if( p->AID != sd.account_id ){
		return false;
	}

	return true;
}

// Tells the client if the name was accepted or not
// 028e <result>.W (HC_ACK_IS_VALID_CHARNAME)
// result:
//		0 = name is not OK
//		1 = name is OK
void chclif_reqrename_response( int32 fd, bool name_valid ){
	PACKET_HC_ACK_IS_VALID_CHARNAME p = {};

	p.packetType = HEADER_HC_ACK_IS_VALID_CHARNAME;
	p.result = name_valid;

	socket_send( fd, p );
}

// Request for checking the new name on character renaming
// 028d <account ID>.l <char ID>.l <new name>.24B (CH_REQ_IS_VALID_CHARNAME)
bool chclif_parse_reqrename( int32 fd, char_session_data& sd ){
	const PACKET_CH_REQ_IS_VALID_CHARNAME* p = reinterpret_cast<PACKET_CH_REQ_IS_VALID_CHARNAME*>( RFIFOP( fd, 0 ) );

	if( p->AID != sd.account_id ){
		return false;
	}

	size_t i;

	ARR_FIND( 0, MAX_CHARS, i, sd.found_char[i] == p->CID );

	if( i == MAX_CHARS ){
		return true;
	}

	char name[NAME_LENGTH];

	safestrncpy( name, p->new_name, NAME_LENGTH );

	normalize_name( name, TRIM_CHARS );

	char esc_name[NAME_LENGTH * 2 + 1];

	Sql_EscapeStringLen( sql_handle, esc_name, name, strnlen( name, NAME_LENGTH ) );

	if( char_check_char_name( name, esc_name ) != 0 ){
		chclif_reqrename_response( fd, false );

		return true;	
	}

	// Name is okay
	safestrncpy( sd.new_name, name, NAME_LENGTH );

	chclif_reqrename_response( fd, true );

	return 1;
}


TIMER_FUNC(charblock_timer){
	struct char_session_data* sd=nullptr;
	int32 i=0;
	ARR_FIND( 0, fd_max, i, session[i] && (sd = (struct char_session_data*)session[i]->session_data) && sd->account_id == id);

	if(sd == nullptr || sd->charblock_timer==INVALID_TIMER) //has disconected or was required to stop
		return 0;
	if (sd->charblock_timer != tid){
		sd->charblock_timer = INVALID_TIMER;
		return 0;
	}
	chclif_block_character( i, *sd );
	return 0;
}

/*
 * 0x20d <PacketLength>.W <TAG_CHARACTER_BLOCK_INFO>24B (HC_BLOCK_CHARACTER)
 * <GID>L <szExpireDate>20B (TAG_CHARACTER_BLOCK_INFO)
 */
void chclif_block_character( int32 fd, char_session_data& sd){
	time_t now = time( nullptr );

	PACKET_HC_BLOCK_CHARACTER* p = reinterpret_cast<PACKET_HC_BLOCK_CHARACTER*>( packet_buffer );

	p->packetType = HEADER_HC_BLOCK_CHARACTER;
	p->packetLength = sizeof( *p );

	for( size_t i = 0, j = 0; i < MAX_CHARS; i++ ){
		if( sd.found_char[i] == -1 )
			continue;
		if( sd.unban_time[i] ){
			if( sd.unban_time[i] > now ){
				PACKET_HC_BLOCK_CHARACTER_sub& character = p->characters[j];

				character.CID = sd.found_char[i];
				timestamp2string( character.unblock_time, sizeof( character.unblock_time ), sd.unban_time[i], "%Y-%m-%d %H:%M:%S" );

				p->packetLength += sizeof( character );
				j++;
			}else{
				sd.unban_time[i] = 0;
				if( SQL_ERROR == Sql_Query( sql_handle, "UPDATE `%s` SET `unban_time`='0' WHERE `char_id`='%d' LIMIT 1", schema_config.char_db, sd.found_char[i] ) )
					Sql_ShowDebug(sql_handle);
			}
		}
	}

	socket_send( fd, p );

	size_t i;

	ARR_FIND( 0, MAX_CHARS, i, sd.unban_time[i] > now ); //sd->charslot only have productible char
	if(i < MAX_CHARS ){
		sd.charblock_timer = add_timer(
			gettick() + 10000,	// each 10s resend that list
			charblock_timer, sd.account_id, 0);
	}
}

// Sends the response to a rename request to the client.
// 0290 <result>.W (HC_ACK_CHANGE_CHARNAME)
// 08fd <result>.L (HC_ACK_CHANGE_CHARACTERNAME)
// result:
//		0: Successful
//		1: This character's name has already been changed. You cannot change a character's name more than once.
//		2: User information is not correct.
//		3: You have failed to change this character's name.
//		4: Another user is using this character name, so please select another one.
//		5: In order to change the character name, you must leave the guild.
//		6: In order to change the character name, you must leave the party.
//		7: Length exceeds the maximum size of the character name you want to change.
//		8: Name contains invalid characters. Character name change failed.
//		9: The name change is prohibited. Character name change failed.
//		10: Character name change failed, due an unknown error.
void chclif_rename_response( int32 fd, int16 response ){
	PACKET_HC_ACK_CHANGE_CHARNAME p = {};

	p.packetType = HEADER_HC_ACK_CHANGE_CHARNAME;
	p.result = response;

	socket_send( fd, p );
}

// Request to change a character name
// 028f <char_id>.L (CH_REQ_CHANGE_CHARNAME)
// 08fc <char_id>.L <new name>.24B (CH_REQ_CHANGE_CHARACTERNAME)
bool chclif_parse_ackrename( int32 fd, char_session_data& sd ){
	const PACKET_CH_REQ_CHANGE_CHARNAME* p = reinterpret_cast<PACKET_CH_REQ_CHANGE_CHARNAME*>( RFIFOP( fd, 0 ) );

	size_t i;
	uint32 cid = p->CID;

	ARR_FIND( 0, MAX_CHARS, i, sd.found_char[i] == cid );

	if( i == MAX_CHARS ){
		return true;
	}

#if PACKETVER >= 20111101
	char name[NAME_LENGTH], esc_name[NAME_LENGTH * 2 + 1];

	safestrncpy( name, p->new_name, NAME_LENGTH );

	normalize_name( name, TRIM_CHARS );
	Sql_EscapeStringLen( sql_handle, esc_name, name, strnlen( name, NAME_LENGTH ) );

	safestrncpy( sd.new_name, name, NAME_LENGTH );
#endif

	// Start the renaming process
	int16 result = char_rename_char_sql( &sd, cid );

	chclif_rename_response( fd, result );

#if PACKETVER >= 20111101
	// If the renaming was successful, we need to resend the characters
	if( result == 0 ){
		chclif_mmo_char_send( fd, sd );
	}
#endif

	return true;
}

// R 06C <ErrorCode>B HC_REFUSE_ENTER
void chclif_reject(int32 fd, uint8 errCode){
	PACKET_HC_REFUSE_ENTER p = {};

	p.packetType = HEADER_HC_REFUSE_ENTER;
	p.error = errCode;

	socket_send( fd, p );
}

class CharPacketDatabase : public PacketDatabase<char_session_data>{
public:
	CharPacketDatabase(){
		this->add( HEADER_CH_SELECT_CHAR, true, sizeof( PACKET_CH_SELECT_CHAR ), chclif_parse_charselect );
		this->add( HEADER_CH_MAKE_CHAR, true, sizeof( PACKET_CH_MAKE_CHAR ), chclif_parse_createnewchar );
		this->add( HEADER_CH_DELETE_CHAR, true, sizeof( PACKET_CH_DELETE_CHAR ), chclif_parse_delchar );
		this->add( HEADER_CH_DELETE_CHAR3_CANCEL, true, sizeof( PACKET_CH_DELETE_CHAR3_CANCEL ), chclif_parse_char_delete2_cancel );
		this->add( HEADER_CH_DELETE_CHAR3, true, sizeof( PACKET_CH_DELETE_CHAR3 ), chclif_parse_char_delete2_accept );
		this->add( HEADER_CH_DELETE_CHAR3_RESERVED, true, sizeof( PACKET_CH_DELETE_CHAR3_RESERVED ), chclif_parse_char_delete2_req );
		this->add( HEADER_PING, true, sizeof( PACKET_PING ), chclif_parse_keepalive );
		this->add( HEADER_CH_REQ_IS_VALID_CHARNAME, true, sizeof( PACKET_CH_REQ_IS_VALID_CHARNAME ), chclif_parse_reqrename );
		this->add( HEADER_CH_REQ_CHANGE_CHARNAME, true, sizeof( PACKET_CH_REQ_CHANGE_CHARNAME ), chclif_parse_ackrename );
		this->add( HEADER_CH_REQ_CHANGE_CHARACTER_SLOT, true, sizeof( PACKET_CH_REQ_CHANGE_CHARACTER_SLOT ), chclif_parse_moveCharSlot );
		this->add( HEADER_CH_CHARLIST_REQ, true, sizeof( PACKET_CH_CHARLIST_REQ ), chclif_parse_req_charlist );
		this->add( HEADER_CH_SELECT_ACCESSIBLE_MAPNAME, true, sizeof( PACKET_CH_SELECT_ACCESSIBLE_MAPNAME ), chclif_parse_select_accessible_map );
#if PACKETVER_SUPPORTS_PINCODE
		this->add( HEADER_CH_SECOND_PASSWD_ACK, true, sizeof( PACKET_CH_SECOND_PASSWD_ACK ), chclif_parse_pincode_check );
		this->add( HEADER_CH_AVAILABLE_SECOND_PASSWD, true, sizeof( PACKET_CH_AVAILABLE_SECOND_PASSWD ), chclif_parse_reqpincode_window );
		this->add( HEADER_CH_EDIT_SECOND_PASSWD, true, sizeof( PACKET_CH_EDIT_SECOND_PASSWD ), chclif_parse_pincode_change );
		this->add( HEADER_CH_MAKE_SECOND_PASSWD, true, sizeof( PACKET_CH_MAKE_SECOND_PASSWD ), chclif_parse_pincode_setnew );
#endif
	}
} char_packet_db;

/**
 * Entry point from client to char-serv
 * function that check incoming command then split it to correct handler.
 * @param fd: file descriptor to parse, (link to client)
 */
int32 chclif_parse(int32 fd) {
	struct char_session_data* sd = (struct char_session_data*)session[fd]->session_data;
	uint32 ipl = session[fd]->client_addr;

	// disconnect any player if no login-server.
	if(login_fd < 0)
		set_eof(fd);

	if(session[fd]->flag.eof) {
		if( sd != nullptr && sd->auth ) { // already authed client
			std::shared_ptr<struct online_char_data> data = util::umap_find( char_get_onlinedb(), sd->account_id );

			if( data != nullptr && data->fd == fd ){
				data->fd = -1;
			}

			// If it is not in any server, send it offline. [Skotlex]
			if( data == nullptr || data->server == -1 ){
				char_set_char_offline(-1,sd->account_id);
			}
		}
		do_close(fd);
		return 0;
	}

	while( RFIFOREST(fd) >= 2 ) {
		int32 next = 1;
		uint16 cmd;

		cmd = RFIFOW(fd,0);

#if PACKETVER_SUPPORTS_PINCODE
		// If the pincode system is enabled
		if( charserv_config.pincode_config.pincode_enabled ){
			switch( cmd ){
				// Connect of player
				case 0x65:
				// Client keep-alive packet (every 12 seconds)
				case HEADER_PING:
				// Checks the entered pin
				case HEADER_CH_SECOND_PASSWD_ACK:
				// Request PIN change
				case HEADER_CH_EDIT_SECOND_PASSWD:
				// Request for PIN window
				case HEADER_CH_AVAILABLE_SECOND_PASSWD:
				// Request character list
				case HEADER_CH_CHARLIST_REQ:
				// Connect of map-server
				case 0x2af8:
					break;

				// Before processing any other packets, do a few checks
				default:
					// To reach this block the client should have attained a session already
					if( sd != nullptr ){
						// If the pincode was entered correctly
						if( sd->pincode_correct ){
							break;
						}

						// If no pincode is set (yet)
						if( strlen( sd->pincode ) <= 0 ){
							break;
						}

						// The pincode was not entered correctly, yet the player (=bot) tried to send a different packet => Goodbye!
						set_eof( fd );
						return 0;
					}else{
						// Unknown packet received
						ShowError( "chclif_parse: Received unknown packet " CL_WHITE "0x%x" CL_RESET " from ip '" CL_WHITE "%s" CL_RESET "'! Disconnecting!\n", cmd, ip2str( ipl, nullptr ) );
						set_eof( fd );
						return 0;
					}
			}
		}
#endif

		switch( cmd ) {
			case 0x65: next=chclif_parse_reqtoconnect(fd,sd,ipl); break;
			// login as map-server
			case 0x2af8: chclif_parse_maplogin(fd); return 0; // avoid processing of followup packets here
			default:
				if( !char_packet_db.handle( fd, *sd ) ){
					return 0;
				}
				break;
		}
		if (next == 0)
			return 0; // avoid processing of followup packets (prev was probably incomplete)
	}

	RFIFOFLUSH(fd);
	return 0;
}
