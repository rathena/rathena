// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mmo.h"
#include "../common/socket.h"
#include "../common/sql.h"
#include "../common/random.h"
#include "../common/showmsg.h"
#include "../common/mapindex.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "../common/utils.h"
#include "../common/timer.h"
#include "inter.h"
#include "char.h"
#include "char_logif.h"
#include "char_mapif.h"
#include "char_clif.h"

#include <stdlib.h>


//------------------------------------------------
//Add On system
//------------------------------------------------
// reason
// 0: success
// 1: failed
void chclif_moveCharSlotReply( int fd, struct char_session_data* sd, unsigned short index, short reason ){
	WFIFOHEAD(fd,8);
	WFIFOW(fd,0) = 0x8d5;
	WFIFOW(fd,2) = 8;
	WFIFOW(fd,4) = reason;
	WFIFOW(fd,6) = sd->char_moves[index];
	WFIFOSET(fd,8);
}

/*
 * Client is requesting to move a charslot
 */
int chclif_parse_moveCharSlot( int fd, struct char_session_data* sd){
	uint16 from, to;

	if( RFIFOREST(fd) < 8 )
		return 0;
	from = RFIFOW(fd,2);
	to = RFIFOW(fd,4);
	//Cnt = RFIFOW(fd,6); //how many time we have left to change (client.. lol we don't trust him)
	RFIFOSKIP(fd,8);

	// Have we changed to often or is it disabled?
	if( (charserv_config.charmove_config.char_move_enabled)==0
	|| ( (charserv_config.charmove_config.char_moves_unlimited)==0 && sd->char_moves[from] <= 0 ) ){
		chclif_moveCharSlotReply( fd, sd, from, 1 );
		return 1;
	}

	// We don't even have a character on the chosen slot?
	if( sd->found_char[from] <= 0 || to >= sd->char_slots ){
		chclif_moveCharSlotReply( fd, sd, from, 1 );
		return 1;
	}

	if( sd->found_char[to] > 0 ){
		// We want to move to a used position
		if( charserv_config.charmove_config.char_movetoused ){ // TODO: check if the target is in deletion process
			// Admin is friendly and uses triangle exchange
			if( SQL_ERROR == Sql_QueryStr(sql_handle, "START TRANSACTION")
				|| SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `char_num`='%d' WHERE `char_id` = '%d'",schema_config.char_db, to, sd->found_char[from] )
				|| SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `char_num`='%d' WHERE `char_id` = '%d'", schema_config.char_db, from, sd->found_char[to] )
				|| SQL_ERROR == Sql_QueryStr(sql_handle, "COMMIT")
				){
				chclif_moveCharSlotReply( fd, sd, from, 1 );
				Sql_ShowDebug(sql_handle);
				Sql_QueryStr(sql_handle,"ROLLBACK");
				return 1;
			}
		}else{
			// Admin doesn't allow us to
			chclif_moveCharSlotReply( fd, sd, from, 1 );
			return 1;
		}
	}else if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `char_num`='%d' WHERE `char_id`='%d'", schema_config.char_db, to, sd->found_char[from] ) ){
		Sql_ShowDebug(sql_handle);
		chclif_moveCharSlotReply( fd, sd, from, 1 );
		return 1;
	}

	if( (charserv_config.charmove_config.char_moves_unlimited)==0 ){
		sd->char_moves[from]--;
		Sql_Query(sql_handle, "UPDATE `%s` SET `moves`='%d' WHERE `char_id`='%d'", schema_config.char_db, sd->char_moves[from], sd->found_char[from] );
	}

	// We successfully moved the char - time to notify the client
	chclif_moveCharSlotReply( fd, sd, from, 0 );
	chclif_mmo_char_send(fd, sd);
	return 1;
}

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
void chclif_pincode_sendstate( int fd, struct char_session_data* sd, enum pincode_state state ){
	WFIFOHEAD(fd, 12);
	WFIFOW(fd, 0) = 0x8b9;
	WFIFOL(fd, 2) = sd->pincode_seed = rnd() % 0xFFFF;
	WFIFOL(fd, 6) = sd->account_id;
	WFIFOW(fd,10) = state;
	WFIFOSET(fd,12);
}

/*
 * Client just entering charserv from login, send him pincode confirmation
 */
int chclif_parse_reqpincode_window(int fd, struct char_session_data* sd){
	if( RFIFOREST(fd) < 6 )
		return 0;
	if( charserv_config.pincode_config.pincode_enabled && RFIFOL(fd,2) == sd->account_id ){
		if( strlen( sd->pincode ) <= 0 ){
			chclif_pincode_sendstate( fd, sd, PINCODE_NEW );
		}else{
			chclif_pincode_sendstate( fd, sd, PINCODE_ASK );
		}
	}
	RFIFOSKIP(fd,6);
	return 1;
}

/*
 * Client as anwsered pincode questionning, checking if valid anwser
 */
int chclif_parse_pincode_check( int fd, struct char_session_data* sd ){
	char pin[PINCODE_LENGTH+1];

	if( RFIFOREST(fd) < 10 )
		return 0;
	if( charserv_config.pincode_config.pincode_enabled==0 || RFIFOL(fd,2) != sd->account_id )
		return 1;

	memset(pin,0,PINCODE_LENGTH+1);
	strncpy((char*)pin, (char*)RFIFOP(fd, 6), PINCODE_LENGTH);
	RFIFOSKIP(fd,10);

	char_pincode_decrypt(sd->pincode_seed, pin );
	if( char_pincode_compare( fd, sd, pin ) ){
		chclif_pincode_sendstate( fd, sd, PINCODE_PASSED );
	}
	return 1;
}

/*
 * Client request to change pincode
 */
int chclif_parse_pincode_change( int fd, struct char_session_data* sd ){
	if( RFIFOREST(fd) < 14 )
		return 0;
	if( charserv_config.pincode_config.pincode_enabled==0 || RFIFOL(fd,2) != sd->account_id )
		return 1;
	else {
		char oldpin[PINCODE_LENGTH+1];
		char newpin[PINCODE_LENGTH+1];
		
		memset(oldpin,0,PINCODE_LENGTH+1);
		memset(newpin,0,PINCODE_LENGTH+1);
		strncpy(oldpin, (char*)RFIFOP(fd,6), PINCODE_LENGTH);
		strncpy(newpin, (char*)RFIFOP(fd,10), PINCODE_LENGTH);
		RFIFOSKIP(fd,14);
		
		char_pincode_decrypt(sd->pincode_seed,oldpin);
		if( !char_pincode_compare( fd, sd, oldpin ) )
			return 1;
		char_pincode_decrypt(sd->pincode_seed,newpin);
		
		chlogif_pincode_notifyLoginPinUpdate( sd->account_id, newpin );
		strncpy(sd->pincode, newpin, sizeof(newpin));
		ShowInfo("Pincode changed for AID: %d\n", sd->account_id);
		
		chclif_pincode_sendstate( fd, sd, PINCODE_PASSED );
	}
	return 1;
}

/*
 * activate PIN system and set first PIN
 */
int chclif_parse_pincode_setnew( int fd, struct char_session_data* sd ){
	if( RFIFOREST(fd) < 10 )
		return 0;

	if( charserv_config.pincode_config.pincode_enabled==0 || RFIFOL(fd,2) != sd->account_id )
		return 1;
	else {
		char newpin[PINCODE_LENGTH+1];
		memset(newpin,0,PINCODE_LENGTH+1);
		strncpy( newpin, (char*)RFIFOP(fd,6), PINCODE_LENGTH );
		RFIFOSKIP(fd,10);

		char_pincode_decrypt( sd->pincode_seed, newpin );

		chlogif_pincode_notifyLoginPinUpdate( sd->account_id, newpin );
		strncpy( sd->pincode, newpin, strlen( newpin ) );

		chclif_pincode_sendstate( fd, sd, PINCODE_PASSED );
	}
	return 1;
}


//----------------------------------------
// Tell client how many pages, kRO sends 17 (Yommy)
//----------------------------------------
void chclif_charlist_notify( int fd, struct char_session_data* sd ){
	WFIFOHEAD(fd, 6);
	WFIFOW(fd, 0) = 0x9a0;
	// pages to req / send them all in 1 until mmo_chars_fromsql can split them up
	WFIFOL(fd, 2) = (sd->char_slots>3)?sd->char_slots/3:1; //int TotalCnt (nb page to load)
	WFIFOSET(fd,6);
}

//----------------------------------------
// Function to send characters to a player
//----------------------------------------
int chclif_mmo_send006b(int fd, struct char_session_data* sd){
	int j, offset = 0;
	bool newvers = (sd->version >= date2version(20100413) );
	if(newvers) //20100413
		offset += 3;
	if (charserv_config.save_log)
		ShowInfo("Loading Char Data ("CL_BOLD"%d"CL_RESET")\n",sd->account_id);

	j = 24 + offset; // offset
	WFIFOHEAD(fd,j + MAX_CHARS*MAX_CHAR_BUF);
	WFIFOW(fd,0) = 0x6b;
	if(newvers){ //20100413
		WFIFOB(fd,4) = MAX_CHARS; // Max slots.
		WFIFOB(fd,5) = sd->char_slots; // Available slots. (PremiumStartSlot)
		WFIFOB(fd,6) = MAX_CHARS; // Premium slots. (Any existent chars past sd->char_slots but within MAX_CHARS will show a 'Premium Service' in red)
	}
	memset(WFIFOP(fd,4 + offset), 0, 20); // unknown bytes
	j+=char_mmo_chars_fromsql(sd, WFIFOP(fd,j));
	WFIFOW(fd,2) = j; // packet len
	WFIFOSET(fd,j);

	return 0;
}

//----------------------------------------
// Notify client about charselect window data [Ind]
//----------------------------------------
void chclif_mmo_send082d(int fd, struct char_session_data* sd) {
	if (charserv_config.save_log)
		ShowInfo("Loading Char Data ("CL_BOLD"%d"CL_RESET")\n",sd->account_id);
	WFIFOHEAD(fd,29);
	WFIFOW(fd,0) = 0x82d;
	WFIFOW(fd,2) = 29;
	WFIFOB(fd,4) = sd->char_slots;
	WFIFOB(fd,5) = MAX_CHARS - sd->char_slots;
	WFIFOB(fd,6) = MAX_CHARS - sd->char_slots;
	WFIFOB(fd,7) = sd->char_slots;
	WFIFOB(fd,8) = sd->char_slots;
	memset(WFIFOP(fd,9), 0, 20); // unused bytes
	WFIFOSET(fd,29);
}

void chclif_mmo_send099d(int fd, struct char_session_data *sd) {
	WFIFOHEAD(fd,4 + (MAX_CHARS*MAX_CHAR_BUF));
	WFIFOW(fd,0) = 0x99d;
	WFIFOW(fd,2) = char_mmo_chars_fromsql(sd, WFIFOP(fd,4)) + 4;
	WFIFOSET(fd,WFIFOW(fd,2));
}


/*
 * Function to choose wich kind of charlist to send to client depending on his version
 */
void chclif_mmo_char_send(int fd, struct char_session_data* sd){
	ShowInfo("sd->version = %d\n",sd->version);
	if(sd->version >= date2version(20130000) ){
		chclif_mmo_send082d(fd,sd);
		chclif_charlist_notify(fd,sd);
		chclif_block_character(fd,sd);
	}
	//@FIXME dump from kro doesn't show 6b transmission
	chclif_mmo_send006b(fd,sd);
}

/*
 * Transmit auth result to client
 * <result>.B ()
 * result :
 *  1 : Server closed
 *  2 : Someone has already logged in with this id
 *  8 : already online
 */
void chclif_send_auth_result(int fd,char result){
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x81;
	WFIFOB(fd,2) = result;
	WFIFOSET(fd,3);
}

/// @param result
/// 0 (0x718): An unknown error has occurred.
/// 1: none/success
/// 3 (0x719): A database error occurred.
/// 4 (0x71a): To delete a character you must withdraw from the guild.
/// 5 (0x71b): To delete a character you must withdraw from the party.
/// Any (0x718): An unknown error has occurred.
/// HC: <0828>.W <char id>.L <Msg:0-5>.L <deleteDate>.L
void chclif_char_delete2_ack(int fd, uint32 char_id, uint32 result, time_t delete_date) {
	WFIFOHEAD(fd,14);
	WFIFOW(fd,0) = 0x828;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = result;
	WFIFOL(fd,10) = TOL(delete_date-time(NULL));
	WFIFOSET(fd,14);
}

/// @param result
/// 0 (0x718): An unknown error has occurred.
/// 1: none/success
/// 2 (0x71c): Due to system settings can not be deleted.
/// 3 (0x719): A database error occurred.
/// 4 (0x71d): Deleting not yet possible time.
/// 5 (0x71e): Date of birth do not match.
/// Any (0x718): An unknown error has occurred.
/// HC: <082a>.W <char id>.L <Msg:0-5>.L
void chclif_char_delete2_accept_ack(int fd, uint32 char_id, uint32 result) {
	if(result == 1 ){
		struct char_session_data* sd;
		sd = (struct char_session_data*)session[fd]->session_data;

		if( sd->version >= date2version(20130000) ){
			chclif_mmo_char_send(fd, sd);
		}
	}

	WFIFOHEAD(fd,10);
	WFIFOW(fd,0) = 0x82a;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = result;
	WFIFOSET(fd,10);
}

/// @param result
/// 1 (0x718): none/success, (if char id not in deletion process): An unknown error has occurred.
/// 2 (0x719): A database error occurred.
/// Any (0x718): An unknown error has occurred.
/// HC: <082c>.W <char id>.L <Msg:1-2>.L
void chclif_char_delete2_cancel_ack(int fd, uint32 char_id, uint32 result) {
	WFIFOHEAD(fd,10);
	WFIFOW(fd,0) = 0x82c;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = result;
	WFIFOSET(fd,10);
}

// CH: <0827>.W <char id>.L
int chclif_parse_char_delete2_req(int fd, struct char_session_data* sd) {
	FIFOSD_CHECK(6)
	{
		uint32 char_id, i;
		char* data;
		time_t delete_date;

		char_id = RFIFOL(fd,2);
		RFIFOSKIP(fd,6);

		ARR_FIND( 0, MAX_CHARS, i, sd->found_char[i] == char_id );
		if( i == MAX_CHARS )
		{// character not found
			chclif_char_delete2_ack(fd, char_id, 3, 0);
			return 1;
		}

		if( SQL_SUCCESS != Sql_Query(sql_handle, "SELECT `delete_date` FROM `%s` WHERE `char_id`='%d'", schema_config.char_db, char_id) || SQL_SUCCESS != Sql_NextRow(sql_handle) )
		{
			Sql_ShowDebug(sql_handle);
			chclif_char_delete2_ack(fd, char_id, 3, 0);
			return 1;
		}

		Sql_GetData(sql_handle, 0, &data, NULL); delete_date = strtoul(data, NULL, 10);

		if( delete_date ) {// character already queued for deletion
			chclif_char_delete2_ack(fd, char_id, 0, 0);
			return 1;
		}

	/*
		// Aegis imposes these checks probably to avoid dead member
		// entries in guilds/parties, otherwise they are not required.
		// TODO: Figure out how these are enforced during waiting.
		if( guild_id )
		{// character in guild
			chclif_char_delete2_ack(fd, char_id, 4, 0);
			return 1;
		}

		if( party_id )
		{// character in party
			chclif_char_delete2_ack(fd, char_id, 5, 0);
			return 1;
		}
	*/
		// success
		delete_date = time(NULL)+(charserv_config.char_config.char_del_delay);

		if( SQL_SUCCESS != Sql_Query(sql_handle, "UPDATE `%s` SET `delete_date`='%lu' WHERE `char_id`='%d'", schema_config.char_db, (unsigned long)delete_date, char_id) )
		{
			Sql_ShowDebug(sql_handle);
			chclif_char_delete2_ack(fd, char_id, 3, 0);
			return 1;
		}

		chclif_char_delete2_ack(fd, char_id, 1, delete_date);
	}
	return 1;
}

// CH: <0829>.W <char id>.L <birth date:YYMMDD>.6B
int chclif_parse_char_delete2_accept(int fd, struct char_session_data* sd) {
	FIFOSD_CHECK(12)
	{
		char birthdate[8+1];
		uint32 char_id;
		int i, k;
		unsigned int base_level;
		char* data;
		time_t delete_date;
		char_id = RFIFOL(fd,2);

		ShowInfo(CL_RED"Request Char Deletion: "CL_GREEN"%d (%d)"CL_RESET"\n", sd->account_id, char_id);

		// construct "YY-MM-DD"
		birthdate[0] = RFIFOB(fd,6);
		birthdate[1] = RFIFOB(fd,7);
		birthdate[2] = '-';
		birthdate[3] = RFIFOB(fd,8);
		birthdate[4] = RFIFOB(fd,9);
		birthdate[5] = '-';
		birthdate[6] = RFIFOB(fd,10);
		birthdate[7] = RFIFOB(fd,11);
		birthdate[8] = 0;
		RFIFOSKIP(fd,12);

		ARR_FIND( 0, MAX_CHARS, i, sd->found_char[i] == char_id );
		if( i == MAX_CHARS )
		{// character not found
			chclif_char_delete2_accept_ack(fd, char_id, 3);
			return 1;
		}

		if( SQL_SUCCESS != Sql_Query(sql_handle, "SELECT `base_level`,`delete_date` FROM `%s` WHERE `char_id`='%d'", schema_config.char_db, char_id) || SQL_SUCCESS != Sql_NextRow(sql_handle) )
		{// data error
			Sql_ShowDebug(sql_handle);
			chclif_char_delete2_accept_ack(fd, char_id, 3);
			return 1;
		}

		Sql_GetData(sql_handle, 0, &data, NULL); base_level = (unsigned int)strtoul(data, NULL, 10);
		Sql_GetData(sql_handle, 1, &data, NULL); delete_date = strtoul(data, NULL, 10);

		if( !delete_date || delete_date>time(NULL) )
		{// not queued or delay not yet passed
			chclif_char_delete2_accept_ack(fd, char_id, 4);
			return 1;
		}

		if( strcmp(sd->birthdate+2, birthdate) )  // +2 to cut off the century
		{// birth date is wrong
			chclif_char_delete2_accept_ack(fd, char_id, 5);
			return 1;
		}

		if( ( charserv_config.char_config.char_del_level > 0 && base_level >= (unsigned int)charserv_config.char_config.char_del_level )
		|| ( charserv_config.char_config.char_del_level < 0 && base_level <= (unsigned int)(-charserv_config.char_config.char_del_level) ) )
		{// character level config restriction
			chclif_char_delete2_accept_ack(fd, char_id, 2);
			return 1;
		}

		// success
		if( char_delete_char_sql(char_id) < 0 ){
			chclif_char_delete2_accept_ack(fd, char_id, 3);
			return 1;
		}

		// refresh character list cache
		for(k = i; k < MAX_CHARS-1; k++){
			sd->found_char[k] = sd->found_char[k+1];
		}
		sd->found_char[MAX_CHARS-1] = -1;

		chclif_char_delete2_accept_ack(fd, char_id, 1);
	}
	return 1;
}

// CH: <082b>.W <char id>.L
int chclif_parse_char_delete2_cancel(int fd, struct char_session_data* sd) {
	uint32 char_id;
	int i;

	FIFOSD_CHECK(6)

	char_id = RFIFOL(fd,2);
	RFIFOSKIP(fd,6);

	ARR_FIND( 0, MAX_CHARS, i, sd->found_char[i] == char_id );
	if( i == MAX_CHARS )
	{// character not found
		chclif_char_delete2_cancel_ack(fd, char_id, 2);
		return 1;
	}

	// there is no need to check, whether or not the character was
	// queued for deletion, as the client prints an error message by
	// itself, if it was not the case (@see char_delete2_cancel_ack)
	if( SQL_SUCCESS != Sql_Query(sql_handle, "UPDATE `%s` SET `delete_date`='0' WHERE `char_id`='%d'", schema_config.char_db, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		chclif_char_delete2_cancel_ack(fd, char_id, 2);
		return 1;
	}

	chclif_char_delete2_cancel_ack(fd, char_id, 1);
	return 1;
}

/*
 * Register a new mapserver into that char-serv
 * charserv can handle a MAX_SERVERS mapservs
 */
int chclif_parse_maplogin(int fd){
	if (RFIFOREST(fd) < 60)
		return 0;
	else {
		int i;
		char* l_user = (char*)RFIFOP(fd,2);
		char* l_pass = (char*)RFIFOP(fd,26);
		l_user[23] = '\0';
		l_pass[23] = '\0';
		ARR_FIND( 0, ARRAYLENGTH(map_server), i, map_server[i].fd <= 0 );
		if( runflag != CHARSERVER_ST_RUNNING ||
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
			memset(map_server[i].map, 0, sizeof(map_server[i].map));
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
int chclif_parse_reqtoconnect(int fd, struct char_session_data* sd,uint32 ipl){
	if( RFIFOREST(fd) < 17 ) // request to connect
		return 0;
	else {
		struct auth_node* node;
		DBMap *auth_db = char_get_authdb();

		uint32 account_id = RFIFOL(fd,2);
		uint32 login_id1 = RFIFOL(fd,6);
		uint32 login_id2 = RFIFOL(fd,10);
		int sex = RFIFOB(fd,16);
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

		// send back account_id
		WFIFOHEAD(fd,4);
		WFIFOL(fd,0) = account_id;
		WFIFOSET(fd,4);

		if( runflag != CHARSERVER_ST_RUNNING ) {
			chclif_reject(fd, 0); // rejected from server
			return 1;
		}

		// search authentification
		node = (struct auth_node*)idb_get(auth_db, account_id);
		if( node != NULL &&
			node->account_id == account_id &&
			node->login_id1  == login_id1 &&
			node->login_id2  == login_id2 /*&&
			node->ip         == ipl*/ )
		{// authentication found (coming from map server)
			sd->version = node->version;
			idb_remove(auth_db, account_id);
			char_auth_ok(fd, sd);
		}
		else
		{// authentication not found (coming from login server)
			if (login_fd > 0) { // don't send request if no login-server
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

//struct PACKET_CH_CHARLIST_REQ { 0x0 short PacketType}
int chclif_parse_req_charlist(int fd, struct char_session_data* sd){
	if( RFIFOREST(fd) < 2 )
		return 0;
	RFIFOSKIP(fd,2);
	chclif_mmo_send099d(fd,sd);
	return 1;
}

int chclif_parse_charselect(int fd, struct char_session_data* sd,uint32 ipl){
	FIFOSD_CHECK(3);
	{
		struct mmo_charstatus char_dat;
		struct mmo_charstatus *cd;
		char* data;
		uint32 char_id;
		uint32 subnet_map_ip;
		struct auth_node* node;
		int i, map_fd;
		DBMap *auth_db = char_get_authdb();
		DBMap *char_db_ = char_get_chardb();

		int slot = RFIFOB(fd,2);
		RFIFOSKIP(fd,3);

		if ( SQL_SUCCESS != Sql_Query(sql_handle, "SELECT `char_id` FROM `%s` WHERE `account_id`='%d' AND `char_num`='%d'", schema_config.char_db, sd->account_id, slot)
		  || SQL_SUCCESS != Sql_NextRow(sql_handle)
		  || SQL_SUCCESS != Sql_GetData(sql_handle, 0, &data, NULL) )
		{	//Not found?? May be forged packet.
			Sql_ShowDebug(sql_handle);
			Sql_FreeResult(sql_handle);
			chclif_reject(fd, 0); // rejected from server
			return 1;
		}

		char_id = atoi(data);
		Sql_FreeResult(sql_handle);

		// Prevent select a char while retrieving guild bound items
		if (sd->flag&1) {
			chclif_reject(fd, 0); // rejected from server
			return 1;
		}

		/* client doesn't let it get to this point if you're banned, so its a forged packet */
		if( sd->found_char[slot] == char_id && sd->unban_time[slot] > time(NULL) ) {
			chclif_reject(fd, 0); // rejected from server
			return 1;
		}

		/* set char as online prior to loading its data so 3rd party applications will realise the sql data is not reliable */
		char_set_char_online(-2,char_id,sd->account_id);
		if( !char_mmo_char_fromsql(char_id, &char_dat, true) ) { /* failed? set it back offline */
			char_set_char_offline(char_id, sd->account_id);
			/* failed to load something. REJECT! */
			chclif_reject(fd, 0); /* jump off this boat */
			return 1;
		}

		//Have to switch over to the DB instance otherwise data won't propagate [Kevin]
		cd = (struct mmo_charstatus *)idb_get(char_db_, char_id);
		if (cd->sex == 99)
			cd->sex = sd->sex;

		if (charserv_config.log_char) {
			char esc_name[NAME_LENGTH*2+1];

			Sql_EscapeStringLen(sql_handle, esc_name, char_dat.name, strnlen(char_dat.name, NAME_LENGTH));
			if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `%s`(`time`, `account_id`,`char_num`,`name`) VALUES (NOW(), '%d', '%d', '%s')",
				schema_config.charlog_db, sd->account_id, slot, esc_name) )
				Sql_ShowDebug(sql_handle);
		}
		ShowInfo("Selected char: (Account %d: %d - %s)\n", sd->account_id, slot, char_dat.name);

		// searching map server
		i = char_search_mapserver(cd->last_point.map, -1, -1);

		// if map is not found, we check major cities
		if (i < 0 || !cd->last_point.map) {
			unsigned short j;
			//First check that there's actually a map server online.
			ARR_FIND( 0, ARRAYLENGTH(map_server), j, map_server[j].fd >= 0 && map_server[j].map[0] );
			if (j == ARRAYLENGTH(map_server)) {
				ShowInfo("Connection Closed. No map servers available.\n");
				chclif_send_auth_result(fd,1); // 01 = Server closed
				return 1;
			}
			if ((i = char_search_mapserver((j=mapindex_name2id(MAP_PRONTERA)),-1,-1)) >= 0) {
				cd->last_point.x = 273;
				cd->last_point.y = 354;
			} else if ((i = char_search_mapserver((j=mapindex_name2id(MAP_GEFFEN)),-1,-1)) >= 0) {
				cd->last_point.x = 120;
				cd->last_point.y = 100;
			} else if ((i = char_search_mapserver((j=mapindex_name2id(MAP_MORROC)),-1,-1)) >= 0) {
				cd->last_point.x = 160;
				cd->last_point.y = 94;
			} else if ((i = char_search_mapserver((j=mapindex_name2id(MAP_ALBERTA)),-1,-1)) >= 0) {
				cd->last_point.x = 116;
				cd->last_point.y = 57;
			} else if ((i = char_search_mapserver((j=mapindex_name2id(MAP_PAYON)),-1,-1)) >= 0) {
				cd->last_point.x = 87;
				cd->last_point.y = 117;
			} else if ((i = char_search_mapserver((j=mapindex_name2id(MAP_IZLUDE)),-1,-1)) >= 0) {
				cd->last_point.x = 94;
				cd->last_point.y = 103;
			} else {
				ShowInfo("Connection Closed. No map server available that has a major city, and unable to find map-server for '%s'.\n", mapindex_id2name(cd->last_point.map));
				chclif_send_auth_result(fd,1); // 01 = Server closed
				return 1;
			}
			ShowWarning("Unable to find map-server for '%s', sending to major city '%s'.\n", mapindex_id2name(cd->last_point.map), mapindex_id2name(j));
			cd->last_point.map = j;
		}

		//Send NEW auth packet [Kevin]
		//FIXME: is this case even possible? [ultramage]
		if ((map_fd = map_server[i].fd) < 1 || session[map_fd] == NULL)
		{
			ShowError("parse_char: Attempting to write to invalid session %d! Map Server #%d disconnected.\n", map_fd, i);
			map_server[i].fd = -1;
			memset(&map_server[i], 0, sizeof(struct mmo_map_server));
			chclif_send_auth_result(fd,1);  //Send server closed.
			return 1;
		}

		//Send player to map
		WFIFOHEAD(fd,28);
		WFIFOW(fd,0) = 0x71;
		WFIFOL(fd,2) = cd->char_id;
		mapindex_getmapname_ext(mapindex_id2name(cd->last_point.map), (char*)WFIFOP(fd,6));
		subnet_map_ip = char_lan_subnetcheck(ipl); // Advanced subnet check [LuzZza]
		WFIFOL(fd,22) = htonl((subnet_map_ip) ? subnet_map_ip : map_server[i].ip);
		WFIFOW(fd,26) = ntows(htons(map_server[i].port)); // [!] LE byte order here [!]
		WFIFOSET(fd,28);

		// create temporary auth entry
		CREATE(node, struct auth_node, 1);
		node->account_id = sd->account_id;
		node->char_id = cd->char_id;
		node->login_id1 = sd->login_id1;
		node->login_id2 = sd->login_id2;
		node->sex = sd->sex;
		node->expiration_time = sd->expiration_time;
		node->group_id = sd->group_id;
		node->ip = ipl;
		idb_put(auth_db, sd->account_id, node);

	}
	return 1;
}

// S 0970 <name>.24B <slot>.B <hair color>.W <hair style>.W
// S 0067 <name>.24B <str>.B <agi>.B <vit>.B <int>.B <dex>.B <luk>.B <slot>.B <hair color>.W <hair style>.W
int chclif_parse_createnewchar(int fd, struct char_session_data* sd,int cmd){
	int i = 0;

	if (cmd == 0x970) FIFOSD_CHECK(31) //>=20120307
	else if (cmd == 0x67) FIFOSD_CHECK(37)
	else return 0;

	if( (charserv_config.char_new)==0 ) //turn character creation on/off [Kevin]
		i = -2;
	else {
#if PACKETVER < 20120307
			i = char_make_new_char_sql(sd, (char*)RFIFOP(fd,2),RFIFOB(fd,26),RFIFOB(fd,27),RFIFOB(fd,28),RFIFOB(fd,29),RFIFOB(fd,30),RFIFOB(fd,31),RFIFOB(fd,32),RFIFOW(fd,33),RFIFOW(fd,35));
			RFIFOSKIP(fd,37);
#else
			i = char_make_new_char_sql(sd, (char*)RFIFOP(fd,2),RFIFOB(fd,26),RFIFOW(fd,27),RFIFOW(fd,29));
			RFIFOSKIP(fd,31);
#endif
	}

	//'Charname already exists' (-1), 'Char creation denied' (-2) and 'You are underaged' (-3)
	if (i < 0) {
		WFIFOHEAD(fd,3);
		WFIFOW(fd,0) = 0x6e;
		/* Others I found [Ind] */
		/* 0x02 = Symbols in Character Names are forbidden */
		/* 0x03 = You are not elegible to open the Character Slot. */
		switch (i) {
			case -1: WFIFOB(fd,2) = 0x00; break;
			case -2: WFIFOB(fd,2) = 0xFF; break;
			case -3: WFIFOB(fd,2) = 0x01; break;
			case -4: WFIFOB(fd,2) = 0x03; break;
		}
		WFIFOSET(fd,3);
	} else {
		int len, ch;
		// retrieve data
		struct mmo_charstatus char_dat;
		char_mmo_char_fromsql(i, &char_dat, false); //Only the short data is needed.

		// send to player
		WFIFOHEAD(fd,2+MAX_CHAR_BUF);
		WFIFOW(fd,0) = 0x6d;
		len = 2 + char_mmo_char_tobuf(WFIFOP(fd,2), &char_dat);
		WFIFOSET(fd,len);

		// add new entry to the chars list
		ARR_FIND( 0, MAX_CHARS, ch, sd->found_char[ch] == -1 );
		if( ch < MAX_CHARS )
			sd->found_char[ch] = i; // the char_id of the new char
	}
	return 1;
}

/**
 * Inform client that his deletion request was refused
 * 0x70 <ErrorCode>B HC_REFUSE_DELETECHAR
 * @param fd
 * @param ErrorCode
 *   00 = Incorrect Email address
 */
void chclif_refuse_delchar(int fd, uint8 errCode){
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x70;
	WFIFOB(fd,2) = errCode;
	WFIFOSET(fd,3);
}

int chclif_parse_delchar(int fd,struct char_session_data* sd, int cmd){
    if (cmd == 0x68) FIFOSD_CHECK(46)
    else if (cmd == 0x1fb) FIFOSD_CHECK(56)
    else return 0;
    {
        char email[40];
        int i, ch;
        int cid = RFIFOL(fd,2);

        ShowInfo(CL_RED"Request Char Deletion: "CL_GREEN"%d (%d)"CL_RESET"\n", sd->account_id, cid);
        memcpy(email, RFIFOP(fd,6), 40);
        RFIFOSKIP(fd,( cmd == 0x68) ? 46 : 56);

        // Check if e-mail is correct
        if(strcmpi(email, sd->email) && //email does not matches and
        (
                strcmp("a@a.com", sd->email) || //it is not default email, or
                (strcmp("a@a.com", email) && strcmp("", email)) //email sent does not matches default
        )) {	//Fail
		chclif_refuse_delchar(fd,0); // 00 = Incorrect Email address
                return 1;
        }

        // check if this char exists
        ARR_FIND( 0, MAX_CHARS, i, sd->found_char[i] == cid );
        if( i == MAX_CHARS )
        { // Such a character does not exist in the account
		chclif_refuse_delchar(fd,0);
                return 1;
        }

        // remove char from list and compact it
        for(ch = i; ch < MAX_CHARS-1; ch++)
                sd->found_char[ch] = sd->found_char[ch+1];
        sd->found_char[MAX_CHARS-1] = -1;

        /* Delete character */
        if(char_delete_char_sql(cid)<0){
                //can't delete the char
                //either SQL error or can't delete by some CONFIG conditions
                //del fail
		chclif_refuse_delchar(fd,0);
                return 1;
        }
        /* Char successfully deleted.*/
        WFIFOHEAD(fd,2);
        WFIFOW(fd,0) = 0x6f;
        WFIFOSET(fd,2);
    }
    return 1;
}

// R 0187 <account ID>.l
int chclif_parse_keepalive(int fd){
	if (RFIFOREST(fd) < 6)
		return 0;
	//int aid = RFIFOL(fd,2);
	RFIFOSKIP(fd,6);
	return 1;
}

// R 08fc <char ID>.l <new name>.24B
// R 028d <account ID>.l <char ID>.l <new name>.24B
int chclif_parse_reqrename(int fd, struct char_session_data* sd, int cmd){
    int i, cid=0;
    char name[NAME_LENGTH];
    char esc_name[NAME_LENGTH*2+1];

    if(cmd == 0x8fc){
            FIFOSD_CHECK(30)
            cid =RFIFOL(fd,2);
            safestrncpy(name, (char *)RFIFOP(fd,6), NAME_LENGTH);
            RFIFOSKIP(fd,30);
    }
    else if(cmd == 0x28d) {
            int aid;
            FIFOSD_CHECK(34);
            aid = RFIFOL(fd,2);
            cid =RFIFOL(fd,6);
            safestrncpy(name, (char *)RFIFOP(fd,10), NAME_LENGTH);
            RFIFOSKIP(fd,34);
            if( aid != sd->account_id )
                    return 1;
    }

    ARR_FIND( 0, MAX_CHARS, i, sd->found_char[i] == cid );
    if( i == MAX_CHARS )
            return 1;

    normalize_name(name,TRIM_CHARS);
    Sql_EscapeStringLen(sql_handle, esc_name, name, strnlen(name, NAME_LENGTH));
    if( !char_check_char_name(name,esc_name) )
    {
            i = 1;
            safestrncpy(sd->new_name, name, NAME_LENGTH);
    }
    else
            i = 0;

    WFIFOHEAD(fd, 4);
    WFIFOW(fd,0) = 0x28e;
    WFIFOW(fd,2) = i;
    WFIFOSET(fd,4);
    return 1;
}


int charblock_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	struct char_session_data* sd=NULL;
	int i=0;
	ARR_FIND( 0, fd_max, i, session[i] && (sd = (struct char_session_data*)session[i]->session_data) && sd->account_id == id);

	if(sd == NULL || sd->charblock_timer==INVALID_TIMER) //has disconected or was required to stop
		return 0;
	if (sd->charblock_timer != tid){
		sd->charblock_timer = INVALID_TIMER;
		return 0;
	}
	chclif_block_character(i,sd);
	return 0;
}

/*
 * 0x20d <PacketLength>.W <TAG_CHARACTER_BLOCK_INFO>24B (HC_BLOCK_CHARACTER)
 * <GID>L <szExpireDate>20B (TAG_CHARACTER_BLOCK_INFO)
 */
void chclif_block_character( int fd, struct char_session_data* sd){
	int i=0, j=0, len=4;
	time_t now = time(NULL);

	WFIFOHEAD(fd, 4+MAX_CHARS*24);
	WFIFOW(fd, 0) = 0x20d;

	for(i=0; i<MAX_CHARS; i++){
		if(sd->found_char[i] == -1)
			continue;
		if(sd->unban_time[i]){
			if( sd->unban_time[i] > now ) {
				char szExpireDate[21];
				WFIFOL(fd, 4+j*24) = sd->found_char[i];
				timestamp2string(szExpireDate, 20, sd->unban_time[i], "%Y-%m-%d %H:%M:%S");
				memcpy(WFIFOP(fd,8+j*24),szExpireDate,20);
			}
			else {
				WFIFOL(fd, 4+j*24) = 0;
				sd->unban_time[i] = 0;
				if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `unban_time`='0' WHERE `char_id`='%d' LIMIT 1", schema_config.char_db, sd->found_char[i]) )
					Sql_ShowDebug(sql_handle);
			}
			len+=24;
			j++; //pkt list idx
		}
	}
	WFIFOW(fd, 2) = len; //packet len
	WFIFOSET(fd,len);

	ARR_FIND(0, MAX_CHARS, i, sd->unban_time[i] > now); //sd->charslot only have productible char
	if(i < MAX_CHARS ){
		sd->charblock_timer = add_timer(
			gettick() + 10000,	// each 10s resend that list
			charblock_timer, sd->account_id, 0);
	}
}

// 0x28f <char_id>.L
int chclif_parse_ackrename(int fd, struct char_session_data* sd){
    // 0: Successful
    // 1: This character's name has already been changed. You cannot change a character's name more than once.
    // 2: User information is not correct.
    // 3: You have failed to change this character's name.
    // 4: Another user is using this character name, so please select another one.
    FIFOSD_CHECK(6)
    {
        int i;
        int cid = RFIFOL(fd,2);
        RFIFOSKIP(fd,6);

        ARR_FIND( 0, MAX_CHARS, i, sd->found_char[i] == cid );
        if( i == MAX_CHARS )
                return 1;
        i = char_rename_char_sql(sd, cid);

        WFIFOHEAD(fd, 4);
        WFIFOW(fd,0) = 0x290;
        WFIFOW(fd,2) = i;
        WFIFOSET(fd,4);
    }
    return 1;
}

int chclif_ack_captcha(int fd){
    WFIFOHEAD(fd,5);
    WFIFOW(fd,0) = 0x7e9;
    WFIFOW(fd,2) = 5;
    WFIFOB(fd,4) = 1;
    WFIFOSET(fd,5);
    return 1;
}

// R 06C <ErrorCode>B HEADER_HC_REFUSE_ENTER
void chclif_reject(int fd, uint8 errCode){
    WFIFOHEAD(fd,3);
    WFIFOW(fd,0) = 0x6c;
    WFIFOB(fd,2) = errCode;// rejected from server
    WFIFOSET(fd,3);
}

// R 07e5 <?>.w <aid>.l
int chclif_parse_reqcaptcha(int fd){
    //FIFOSD_CHECK(8)
    RFIFOSKIP(fd,8);
    chclif_ack_captcha(fd); 
    return 1;
}

// R 07e7 <len>.w <aid>.l <code>.b10 <?>.b14
int chclif_parse_chkcaptcha(int fd){
    //FIFOSD_CHECK(32)
    RFIFOSKIP(fd,32);
    chclif_ack_captcha(fd);
    return 1;
}

/**
 * Entry point from client to char-serv
 * function that check incoming command then split it to correct handler.
 * @param fd: file descriptor to parse, (link to client)
 */
int chclif_parse(int fd) {
	struct char_session_data* sd = (struct char_session_data*)session[fd]->session_data;
	uint32 ipl = session[fd]->client_addr;
    
	// disconnect any player if no login-server.
	if(login_fd < 0)
		set_eof(fd);

	if(session[fd]->flag.eof)
	{
		if( sd != NULL && sd->auth )
		{	// already authed client
			DBMap *online_char_db = char_get_onlinedb();
			struct online_char_data* data = (struct online_char_data*)idb_get(online_char_db, sd->account_id);
			if( data != NULL && data->fd == fd)
				data->fd = -1;
			if( data == NULL || data->server == -1) //If it is not in any server, send it offline. [Skotlex]
				char_set_char_offline(-1,sd->account_id);
		}
		do_close(fd);
		return 0;
	}

	while( RFIFOREST(fd) >= 2 )
	{
		int next = 1;
		unsigned short cmd;

		cmd = RFIFOW(fd,0);
		switch( cmd )
		{
		case 0x65: next=chclif_parse_reqtoconnect(fd,sd,ipl); break;
		// char select
		case 0x66: next=chclif_parse_charselect(fd,sd,ipl); break;
		// createnewchar
		case 0x970: next=chclif_parse_createnewchar(fd,sd,cmd); break;
		case 0x67: next=chclif_parse_createnewchar(fd,sd,cmd); break;
		// delete char
		case 0x68: next=chclif_parse_delchar(fd,sd,cmd); break; //
		case 0x1fb: next=chclif_parse_delchar(fd,sd,cmd); break; // 2004-04-19aSakexe+ langtype 12 char deletion packet
		// client keep-alive packet (every 12 seconds)
		case 0x187: next=chclif_parse_keepalive(fd); break;
		// char rename
		case 0x8fc: next=chclif_parse_reqrename(fd,sd,cmd); break; //request <date/version?>
		case 0x28d: next=chclif_parse_reqrename(fd,sd,cmd); break; //request <date/version?>
		case 0x28f: next=chclif_parse_ackrename(fd,sd); break; //Confirm change name.
		// captcha
		case 0x7e5: next=chclif_parse_reqcaptcha(fd); break; // captcha code request (not implemented)
		case 0x7e7: next=chclif_parse_chkcaptcha(fd); break; // captcha code check (not implemented)
		// deletion timer request
		case 0x827: next=chclif_parse_char_delete2_req(fd, sd); break;
		// deletion accept request
		case 0x829: next=chclif_parse_char_delete2_accept(fd, sd); break;
		// deletion cancel request
		case 0x82b: next=chclif_parse_char_delete2_cancel(fd, sd); break;
		// login as map-server
		case 0x2af8: chclif_parse_maplogin(fd); return 0; // avoid processing of followup packets here
		//pincode
		case 0x8b8: next=chclif_parse_pincode_check( fd, sd ); break; // checks the entered pin
		case 0x8c5: next=chclif_parse_reqpincode_window(fd,sd); break; // request for PIN window
		case 0x8be: next=chclif_parse_pincode_change( fd, sd ); break; // pincode change request
		case 0x8ba: next=chclif_parse_pincode_setnew( fd, sd ); break; // activate PIN system and set first PIN
		// character movement request
		case 0x8d4: next=chclif_parse_moveCharSlot(fd,sd); break;
		case 0x9a1: next=chclif_parse_req_charlist(fd,sd); break;
		// unknown packet received
		default:
			ShowError("parse_char: Received unknown packet "CL_WHITE"0x%x"CL_RESET" from ip '"CL_WHITE"%s"CL_RESET"'! Disconnecting!\n", RFIFOW(fd,0), ip2str(ipl, NULL));
			set_eof(fd);
			return 0;
		}
		if(next==0) return 0; // avoid processing of followup packets (prev was probably incomplete)
	}

	RFIFOFLUSH(fd);
	return 0;
}
