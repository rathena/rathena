// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "char_mapif.hpp"

#include <stdlib.h>
#include <string.h> //memcpy

#include "../common/malloc.hpp"
#include "../common/showmsg.hpp"
#include "../common/socket.hpp"
#include "../common/sql.hpp"
#include "../common/strlib.hpp"
#include "../common/timer.hpp"

#include "char.hpp"
#include "char_logif.hpp"
#include "inter.hpp"

/**
 * Packet send to all map-servers, attach to ourself
 * @param buf: packet to send in form of an array buffer
 * @param len: size of packet
 * @return : the number of map-serv the packet was sent to
 */
int chmapif_sendall(unsigned char *buf, unsigned int len){
	int i, c;

	c = 0;
	for(i = 0; i < ARRAYLENGTH(map_server); i++) {
		int fd;
		if ((fd = map_server[i].fd) > 0) {
			WFIFOHEAD(fd,len);
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			c++;
		}
	}

	return c;
}

/**
 * Packet send to all map-servers, except one. (wos: without our self) attach to ourself
 * @param sfd: fd to discard sending to
 * @param buf: packet to send in form of an array buffer
 * @param len: size of packet
 * @return : the number of map-serv the packet was sent to
 */
int chmapif_sendallwos(int sfd, unsigned char *buf, unsigned int len){
	int i, c;

	c = 0;
	for(i = 0; i < ARRAYLENGTH(map_server); i++) {
		int fd;
		if ((fd = map_server[i].fd) > 0 && fd != sfd) {
			WFIFOHEAD(fd,len);
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			c++;
		}
	}

	return c;
}

/**
 * Packet send to all char-servers, except one. (wos: without our self)
 * @param fd: fd to send packet too
 * @param buf: packet to send in form of an array buffer
 * @param len: size of packet
 * @return : the number of map-serv the packet was sent to (O|1)
 */
int chmapif_send(int fd, unsigned char *buf, unsigned int len){
	if (fd >= 0) {
		int i;
		ARR_FIND( 0, ARRAYLENGTH(map_server), i, fd == map_server[i].fd );
		if( i < ARRAYLENGTH(map_server) )
		{
			WFIFOHEAD(fd,len);
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			return 1;
		}
	}
	return 0;
}



/**
 * Send map-servers fames ranking lists
 *  Defaut fame list are 32B, (id+point+names)
 *  S <len>.W <len bs + alchi>.W <len bs>.W <smith_rank>?B <alchi_rank>?B <taek_rank>?B
 * @param fd: fd to send packet too (map-serv) if -1 send to all
 * @return : 0 success
 */
int chmapif_send_fame_list(int fd){
	int i, len = 8;
	unsigned char buf[32000];

	WBUFW(buf,0) = 0x2b1b;

	for(i = 0; i < fame_list_size_smith && smith_fame_list[i].id; i++) {
		memcpy(WBUFP(buf, len), &smith_fame_list[i], sizeof(struct fame_list));
		len += sizeof(struct fame_list);
	}
	// add blacksmith's block length
	WBUFW(buf, 6) = len;

	for(i = 0; i < fame_list_size_chemist && chemist_fame_list[i].id; i++) {
		memcpy(WBUFP(buf, len), &chemist_fame_list[i], sizeof(struct fame_list));
		len += sizeof(struct fame_list);
	}
	// add alchemist's block length
	WBUFW(buf, 4) = len;

	for(i = 0; i < fame_list_size_taekwon && taekwon_fame_list[i].id; i++) {
		memcpy(WBUFP(buf, len), &taekwon_fame_list[i], sizeof(struct fame_list));
		len += sizeof(struct fame_list);
	}
	// add total packet length
	WBUFW(buf, 2) = len;

	if (fd != -1)
		chmapif_send(fd, buf, len);
	else
		chmapif_sendall(buf, len);

	return 0;
}

/**
 * Send to map-servers the updated fame ranking lists
 *  We actually just send this one when we only need to update rankpoint but pos didn't change
 * @param type: ranking type
 * @param index: position in the ranking
 * @param fame: number of points
 */
void chmapif_update_fame_list(int type, int index, int fame) {
	unsigned char buf[8];
	WBUFW(buf,0) = 0x2b22;
	WBUFB(buf,2) = type;
	WBUFB(buf,3) = index;
	WBUFL(buf,4) = fame;
	chmapif_sendall(buf, 8);
}

/**
 * Send to map-servers the users count on this char-serv, (meaning the total of all mapserv)
 * @param users: number of players on this char-serv
 */
void chmapif_sendall_playercount(int users){
	uint8 buf[6];
	// send number of players to all map-servers
	WBUFW(buf,0) = 0x2b00;
	WBUFL(buf,2) = users;
	chmapif_sendall(buf,6);
}

/**
 * Send some misc info to new map-server.
 * - Server name for whisper name
 * - Default map
 * HZ 0x2afb <size>.W <status>.B <name>.24B <mapname>.11B <map_x>.W <map_y>.W
 * @param fd
 **/
void chmapif_send_misc(int fd) {
	uint16 offs = 5;
	unsigned char buf[45];

	memset(buf, '\0', sizeof(buf));
	WBUFW(buf, 0) = 0x2afb;
	// 0 succes, 1:failure
	WBUFB(buf, 4) = 0;
	// Send name for wisp to player
	memcpy(WBUFP(buf, 5), charserv_config.wisp_server_name, NAME_LENGTH);
	// Default map
	memcpy(WBUFP(buf, (offs+=NAME_LENGTH)), charserv_config.default_map, MAP_NAME_LENGTH); // 29
	WBUFW(buf, (offs+=MAP_NAME_LENGTH)) = charserv_config.default_map_x; // 41
	WBUFW(buf, (offs+=2)) = charserv_config.default_map_y; // 43
	offs+=2;

	// Length
	WBUFW(buf, 2) = offs;
	chmapif_send(fd, buf, offs);
}

/**
 * Sends maps to all map-server
 * HZ 0x2b04 <size>.W <ip>.L <port>.W { <map>.?B }.?B
 * @param fd
 * @param map_id
 * @param count Number of map from new map-server has
 **/
void chmapif_send_maps(int fd, int map_id, int count, unsigned char *mapbuf) {
	uint16 x;

	if (count == 0) {
		ShowWarning("Map-server %d has NO maps.\n", map_id);
	}
	else {
		unsigned char buf[16384];
		// Transmitting maps information to the other map-servers
		WBUFW(buf,0) = 0x2b04;
		WBUFW(buf,2) = count * 4 + 10;
		WBUFL(buf,4) = htonl(map_server[map_id].ip);
		WBUFW(buf,8) = htons(map_server[map_id].port);
		memcpy(WBUFP(buf,10), mapbuf, count * 4);
		chmapif_sendallwos(fd, buf, WBUFW(buf,2));
	}

	// Transmitting the maps of the other map-servers to the new map-server
	for (x = 0; x < ARRAYLENGTH(map_server); x++) {
		if (map_server[x].fd > 0 && x != map_id) {
			uint16 i, j;

			WFIFOHEAD(fd,10 +4*map_server[x].map.size());
			WFIFOW(fd,0) = 0x2b04;
			WFIFOL(fd,4) = htonl(map_server[x].ip);
			WFIFOW(fd,8) = htons(map_server[x].port);
			j = 0;
			for(i = 0; i < map_server[x].map.size(); i++)
				if (map_server[x].map[i])
					WFIFOW(fd,10+(j++)*4) = map_server[x].map[i];
			if (j > 0) {
				WFIFOW(fd,2) = j * 4 + 10;
				WFIFOSET(fd,WFIFOW(fd,2));
			}
		}
	}
}

/**
 * This function is called when the map-serv initialise is chrif interface.
 * Map-serv sent us his map indexes so we can transfert a player from a map-serv to another when necessary
 * We reply by sending back the char_serv_wisp_name  fame list and
 * @param fd: wich fd to parse from
 * @param id: wich map_serv id
 * @return : 0 not enough data received, 1 success
 */
int chmapif_parse_getmapname(int fd, int id){
	int i = 0;
	unsigned char *mapbuf;

	if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
		return 0;

	//Retain what map-index that map-serv contains
	map_server[id].map = {};
	for(i = 4; i < RFIFOW(fd,2); i += 4)
		map_server[id].map.push_back(RFIFOW(fd, i));

	mapbuf = RFIFOP(fd,4);
	RFIFOSKIP(fd,RFIFOW(fd,2));

	ShowStatus("Map-Server %d connected: %" PRIuPTR " maps, from IP %d.%d.%d.%d port %d.\n",
				id, map_server[id].map.size(), CONVIP(map_server[id].ip), map_server[id].port);
	ShowStatus("Map-server %d loading complete.\n", id);

	chmapif_send_misc(fd);
	chmapif_send_fame_list(fd); //Send fame list.
	chmapif_send_maps(fd, id, map_server[id].map.size(), mapbuf);

	return 1;
}

/**
 * Map-serv requesting to send the list of sc_data the player has saved
 * @author [Skotlex]
 * @param fd: wich fd to parse from
 * @return : 0 not enough data received, 1 success
 */
int chmapif_parse_askscdata(int fd){
	if (RFIFOREST(fd) < 10)
		return 0;
	else {
#ifdef ENABLE_SC_SAVING
		int aid, cid;
		aid = RFIFOL(fd,2);
		cid = RFIFOL(fd,6);
		if( SQL_ERROR == Sql_Query(sql_handle, "SELECT type, tick, val1, val2, val3, val4 from `%s` WHERE `account_id` = '%d' AND `char_id`='%d'",
			schema_config.scdata_db, aid, cid) )
		{
			Sql_ShowDebug(sql_handle);
			return 1;
		}
		if( Sql_NumRows(sql_handle) > 0 )
		{
			struct status_change_data scdata;
			int count;
			char* data;

			WFIFOHEAD(fd,14+50*sizeof(struct status_change_data));
			WFIFOW(fd,0) = 0x2b1d;
			WFIFOL(fd,4) = aid;
			WFIFOL(fd,8) = cid;
			for( count = 0; count < 50 && SQL_SUCCESS == Sql_NextRow(sql_handle); ++count )
			{
				Sql_GetData(sql_handle, 0, &data, NULL); scdata.type = atoi(data);
				Sql_GetData(sql_handle, 1, &data, NULL); scdata.tick = strtoll( data, nullptr, 10 );
				Sql_GetData(sql_handle, 2, &data, NULL); scdata.val1 = atoi(data);
				Sql_GetData(sql_handle, 3, &data, NULL); scdata.val2 = atoi(data);
				Sql_GetData(sql_handle, 4, &data, NULL); scdata.val3 = atoi(data);
				Sql_GetData(sql_handle, 5, &data, NULL); scdata.val4 = atoi(data);
				memcpy(WFIFOP(fd, 14+count*sizeof(struct status_change_data)), &scdata, sizeof(struct status_change_data));
			}
			if (count >= 50)
				ShowWarning("Too many status changes for %d:%d, some of them were not loaded.\n", aid, cid);
			if (count > 0)
			{
				WFIFOW(fd,2) = 14 + count*sizeof(struct status_change_data);
				WFIFOW(fd,12) = count;
				WFIFOSET(fd,WFIFOW(fd,2));
			}
		} else { // No Status Changes to load but still send a response
			WFIFOHEAD(fd,14);
			WFIFOW(fd,0) = 0x2b1d;
			WFIFOW(fd,2) = 14;
			WFIFOL(fd,4) = aid;
			WFIFOL(fd,8) = cid;
			WFIFOW(fd,12) = 0;
			WFIFOSET(fd,WFIFOW(fd,2));
		}
		Sql_FreeResult(sql_handle);
#endif
		RFIFOSKIP(fd, 10);
	}
	return 1;
}

/**
 * Map-serv sent us his new users count, updating info
 * @param fd: wich fd to parse from
 * @param id: wich map_serv id
 * @return : 0 not enough data received, 1 success
 */
int chmapif_parse_getusercount(int fd, int id){
	if (RFIFOREST(fd) < 4)
		return 0;
	if (RFIFOW(fd,2) != map_server[id].users) {
		map_server[id].users = RFIFOW(fd,2);
		ShowInfo("User Count: %d (Server: %d)\n", map_server[id].users, id);
	}
	RFIFOSKIP(fd, 4);
	return 1;
}

/**
 * Map-serv sent us all his users info, (aid and cid) so we can update online_char_db
 * @param fd: wich fd to parse from
 * @param id: wich map_serv id
 * @return : 0 not enough data received, 1 success
 */
int chmapif_parse_regmapuser(int fd, int id){
	if (RFIFOREST(fd) < 6 || RFIFOREST(fd) < RFIFOW(fd,2))
		return 0;
	else {
		//TODO: When data mismatches memory, update guild/party online/offline states.
		DBMap* online_char_db = char_get_onlinedb();
		int i;

		map_server[id].users = RFIFOW(fd,4);
		online_char_db->foreach(online_char_db,char_db_setoffline,id); //Set all chars from this server as 'unknown'
		for(i = 0; i < map_server[id].users; i++) {
			int aid = RFIFOL(fd,6+i*8);
			int cid = RFIFOL(fd,6+i*8+4);
			struct online_char_data* character = (struct online_char_data*)idb_ensure(online_char_db, aid, char_create_online_data);
			if( character->server > -1 && character->server != id )
			{
				ShowNotice("Set map user: Character (%d:%d) marked on map server %d, but map server %d claims to have (%d:%d) online!\n",
					character->account_id, character->char_id, character->server, id, aid, cid);
				mapif_disconnectplayer(map_server[character->server].fd, character->account_id, character->char_id, 2);
			}
			character->server = id;
			character->char_id = cid;
		}
		//If any chars remain in -2, they will be cleaned in the cleanup timer.
		RFIFOSKIP(fd,RFIFOW(fd,2));
	}
	return 1;
}

/**
 * Map-serv request to save mmo_char_status in sql
 * Receive character data from map-server for saving
 * @param fd: wich fd to parse from
 * @param id: wich map_serv id
 * @return : 0 not enough data received, 1 success
 */
int chmapif_parse_reqsavechar(int fd, int id){
	if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
		return 0;
	else {
		int aid = RFIFOL(fd,4), cid = RFIFOL(fd,8), size = RFIFOW(fd,2);
		struct online_char_data* character;
		DBMap* online_char_db = char_get_onlinedb();

		if (size - 13 != sizeof(struct mmo_charstatus))
		{
			ShowError("parse_from_map (save-char): Size mismatch! %d != %" PRIuPTR "\n", size-13, sizeof(struct mmo_charstatus));
			RFIFOSKIP(fd,size);
			return 1;
		}
		//Check account only if this ain't final save. Final-save goes through because of the char-map reconnect
		if (RFIFOB(fd,12) || RFIFOB(fd,13) || (
			(character = (struct online_char_data*)idb_get(online_char_db, aid)) != NULL &&
			character->char_id == cid))
		{
			struct mmo_charstatus char_dat;
			memcpy(&char_dat, RFIFOP(fd,13), sizeof(struct mmo_charstatus));
			char_mmo_char_tosql(cid, &char_dat);
		} else {	//This may be valid on char-server reconnection, when re-sending characters that already logged off.
			ShowError("parse_from_map (save-char): Received data for non-existant/offline character (%d:%d).\n", aid, cid);
			char_set_char_online(id, cid, aid);
		}

		if (RFIFOB(fd,12))
		{	//Flag, set character offline after saving. [Skotlex]
			char_set_char_offline(cid, aid);
			WFIFOHEAD(fd,10);
			WFIFOW(fd,0) = 0x2b21; //Save ack only needed on final save.
			WFIFOL(fd,2) = aid;
			WFIFOL(fd,6) = cid;
			WFIFOSET(fd,10);
		}
		RFIFOSKIP(fd,size);
	}
	return 1;
}

/**
 * Inform mapserv of a new character selection request
 * @param fd : FD link tomapserv
 * @param aid : Player account id
 * @param res : result, 0=not ok, 1=ok
 */
void chmapif_charselres(int fd, uint32 aid, uint8 res){
	WFIFOHEAD(fd,7);
	WFIFOW(fd,0) = 0x2b03;
	WFIFOL(fd,2) = aid;
	WFIFOB(fd,6) = res;
	WFIFOSET(fd,7);
}

/**
 * Player Requesting char-select from map_serv
 * @param fd: wich fd to parse from
 * @return : 0 not enough data received, 1 success
 */
int chmapif_parse_authok(int fd){
	if( RFIFOREST(fd) < 18 )
		return 0;
	else{
		uint32 account_id = RFIFOL(fd,2);
		uint32 login_id1 = RFIFOL(fd,6);
		uint32 login_id2 = RFIFOL(fd,10);
		uint32 ip = RFIFOL(fd,14);
		RFIFOSKIP(fd,18);

		if( runflag != CHARSERVER_ST_RUNNING ){
			chmapif_charselres(fd,account_id,0);
		}else{
			struct auth_node* node;
			DBMap*  auth_db = char_get_authdb();
			DBMap* online_char_db = char_get_onlinedb();

			// create temporary auth entry
			CREATE(node, struct auth_node, 1);
			node->account_id = account_id;
			node->char_id = 0;
			node->login_id1 = login_id1;
			node->login_id2 = login_id2;
			//node->sex = 0;
			node->ip = ntohl(ip);
			//node->expiration_time = 0; // unlimited/unknown time by default (not display in map-server)
			//node->gmlevel = 0;
			idb_put(auth_db, account_id, node);

			//Set char to "@ char select" in online db [Kevin]
			char_set_charselect(account_id);
			{
				struct online_char_data* character = (struct online_char_data*)idb_get(online_char_db, account_id);
				if( character != NULL ){
					character->pincode_success = true;
				}
			}
			chmapif_charselres(fd,account_id,1);
		}
	}
	return 1;
}

//Request to save skill cooldown data
int chmapif_parse_req_saveskillcooldown(int fd){
	if( RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2) )
		return 0;
	else {
		int count, aid, cid;
		aid = RFIFOL(fd,4);
		cid = RFIFOL(fd,8);
		count = RFIFOW(fd,12);
		if( count > 0 )
		{
			struct skill_cooldown_data data;
			StringBuf buf;
			int i;

			StringBuf_Init(&buf);
			StringBuf_Printf(&buf, "INSERT INTO `%s` (`account_id`, `char_id`, `skill`, `tick`) VALUES ", schema_config.skillcooldown_db);
			for( i = 0; i < count; ++i )
			{
				memcpy(&data,RFIFOP(fd,14+i*sizeof(struct skill_cooldown_data)),sizeof(struct skill_cooldown_data));
				if( i > 0 )
					StringBuf_AppendStr(&buf, ", ");
				StringBuf_Printf(&buf, "('%d','%d','%d','%" PRtf "')", aid, cid, data.skill_id, data.tick);
			}
			if( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
				Sql_ShowDebug(sql_handle);
			StringBuf_Destroy(&buf);
		}
		RFIFOSKIP(fd, RFIFOW(fd, 2));
	}
	return 1;
}

//Request skillcooldown data 0x2b0a
int chmapif_parse_req_skillcooldown(int fd){
	if (RFIFOREST(fd) < 10)
		return 0;
	else {
		int aid, cid;
		aid = RFIFOL(fd,2);
		cid = RFIFOL(fd,6);
		RFIFOSKIP(fd, 10);
		if( SQL_ERROR == Sql_Query(sql_handle, "SELECT skill, tick FROM `%s` WHERE `account_id` = '%d' AND `char_id`='%d'",
			schema_config.skillcooldown_db, aid, cid) )
		{
			Sql_ShowDebug(sql_handle);
			return 1;
		}
		if( Sql_NumRows(sql_handle) > 0 )
		{
			int count;
			char* data;
			struct skill_cooldown_data scd;

			WFIFOHEAD(fd,14 + MAX_SKILLCOOLDOWN * sizeof(struct skill_cooldown_data));
			WFIFOW(fd,0) = 0x2b0b;
			WFIFOL(fd,4) = aid;
			WFIFOL(fd,8) = cid;
			for( count = 0; count < MAX_SKILLCOOLDOWN && SQL_SUCCESS == Sql_NextRow(sql_handle); ++count )
			{
				Sql_GetData(sql_handle, 0, &data, NULL); scd.skill_id = atoi(data);
				Sql_GetData(sql_handle, 1, &data, NULL); scd.tick = strtoll( data, nullptr, 10 );
				memcpy(WFIFOP(fd,14+count*sizeof(struct skill_cooldown_data)), &scd, sizeof(struct skill_cooldown_data));
			}
			if( count >= MAX_SKILLCOOLDOWN )
				ShowWarning("Too many skillcooldowns for %d:%d, some of them were not loaded.\n", aid, cid);
			if( count > 0 )
			{
				WFIFOW(fd,2) = 14 + count * sizeof(struct skill_cooldown_data);
				WFIFOW(fd,12) = count;
				WFIFOSET(fd,WFIFOW(fd,2));
				//Clear the data once loaded.
				if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = '%d' AND `char_id`='%d'", schema_config.skillcooldown_db, aid, cid) )
					Sql_ShowDebug(sql_handle);
			}
		}
		Sql_FreeResult(sql_handle);
	}
	return 1;
}

/**
 * Inform the mapserv, of a change mapserv request
 * @param fd :Link to mapserv
 * @param nok : 0=accepted or no=1
 */
void chmapif_changemapserv_ack(int fd, bool nok){
    WFIFOHEAD(fd,30);
    WFIFOW(fd,0) = 0x2b06;
    memcpy(WFIFOP(fd,2), RFIFOP(fd,2), 28);
    if(nok) 
	WFIFOL(fd,6) = 0; //Set login1 to 0.(not ok)
    WFIFOSET(fd,30);
}

/**
 * Player requesting to change map-serv
 * @param fd: wich fd to parse from
 * @return : 0 not enough data received, 1 success
 */
int chmapif_parse_reqchangemapserv(int fd){
	if (RFIFOREST(fd) < 39)
		return 0;
	else {
		int map_id, map_fd = -1;
		struct mmo_charstatus* char_data;
		struct mmo_charstatus char_dat;
		DBMap* char_db_ = char_get_chardb();

		map_id = char_search_mapserver(RFIFOW(fd,18), ntohl(RFIFOL(fd,24)), ntohs(RFIFOW(fd,28))); //Locate mapserver by ip and port.
		if (map_id >= 0)
			map_fd = map_server[map_id].fd;
		//Char should just had been saved before this packet, so this should be safe. [Skotlex]
		char_data = (struct mmo_charstatus*)uidb_get(char_db_,RFIFOL(fd,14));
		if (char_data == NULL) {	//Really shouldn't happen.
			char_mmo_char_fromsql(RFIFOL(fd,14), &char_dat, true);
			char_data = (struct mmo_charstatus*)uidb_get(char_db_,RFIFOL(fd,14));
		}

		if( runflag == CHARSERVER_ST_RUNNING &&
			session_isActive(map_fd) &&
			char_data )
		{	//Send the map server the auth of this player.
			struct online_char_data* data;
			struct auth_node* node;
			DBMap*  auth_db = char_get_authdb();
			DBMap* online_char_db = char_get_onlinedb();

			int aid = RFIFOL(fd,2);

			//Update the "last map" as this is where the player must be spawned on the new map server.
			char_data->last_point.map = RFIFOW(fd,18);
			char_data->last_point.x = RFIFOW(fd,20);
			char_data->last_point.y = RFIFOW(fd,22);
			char_data->sex = RFIFOB(fd,30);

			// create temporary auth entry
			CREATE(node, struct auth_node, 1);
			node->account_id = aid;
			node->char_id = RFIFOL(fd,14);
			node->login_id1 = RFIFOL(fd,6);
			node->login_id2 = RFIFOL(fd,10);
			node->sex = RFIFOB(fd,30);
			node->expiration_time = 0; // FIXME (this thing isn't really supported we could as well purge it instead of fixing)
			node->ip = ntohl(RFIFOL(fd,31));
			node->group_id = RFIFOL(fd,35);
			node->changing_mapservers = 1;
			idb_put(auth_db, aid, node);

			data = (struct online_char_data*)idb_ensure(online_char_db, aid, char_create_online_data);
			data->char_id = char_data->char_id;
			data->server = map_id; //Update server where char is.

			//Reply with an ack.
			chmapif_changemapserv_ack(fd,0);
		} else { //Reply with nak
			chmapif_changemapserv_ack(fd,1);
		}
		RFIFOSKIP(fd,39);
	}
	return 1;
}

/**
 * Player requesting to remove friend from list
 * Remove RFIFOL(fd,6) (friend_id) from RFIFOL(fd,2) (char_id) friend list
 * @author [Ind]
 * @param fd: wich fd to parse from
 * @return : 0 not enough data received, 1 success
 */
int chmapif_parse_askrmfriend(int fd){
	if (RFIFOREST(fd) < 10)
		return 0;
	else {
		uint32 char_id, friend_id;
		char_id = RFIFOL(fd,2);
		friend_id = RFIFOL(fd,6);
		if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d' AND `friend_id`='%d' LIMIT 1",
			schema_config.friend_db, char_id, friend_id) ) {
			Sql_ShowDebug(sql_handle);
			return 1;
		}
		RFIFOSKIP(fd,10);
	}
	return 1;
}

/**
 * Lookup to search if that char_id correspond to a name.
 * Comming from map-serv to search on other map-serv
 * @param fd: wich fd to parse from
 * @return : 0 not enough data received, 1 success
 */
int chmapif_parse_reqcharname(int fd){
	if (RFIFOREST(fd) < 6)
		return 0;

	WFIFOHEAD(fd,30);
	WFIFOW(fd,0) = 0x2b09;
	WFIFOL(fd,2) = RFIFOL(fd,2);
	char_loadName((int)RFIFOL(fd,2), WFIFOCP(fd,6));
	WFIFOSET(fd,30);

	RFIFOSKIP(fd,6);
	return 1;
}

/**
 * Forward an email update request to login-serv
 * Map server send information to change an email of an account -> login-server
 * @param fd: wich fd to parse from
 * @return : 0 not enough data received, 1 success
 */
int chmapif_parse_reqnewemail(int fd){
	if (RFIFOREST(fd) < 86)
		return 0;
	if (chlogif_isconnected()) { // don't send request if no login-server
		WFIFOHEAD(login_fd,86);
		memcpy(WFIFOP(login_fd,0), RFIFOP(fd,0),86); // 0x2722 <account_id>.L <actual_e-mail>.40B <new_e-mail>.40B
		WFIFOW(login_fd,0) = 0x2722;
		WFIFOSET(login_fd,86);
	}
	RFIFOSKIP(fd, 86);
	return 1;
}

/**
 * Forward a change of status for account to login-serv
 * @param fd: which fd to parse from
 * @return : 0 not enough data received, 1 success
 */
int chmapif_parse_fwlog_changestatus(int fd){
	if (RFIFOREST(fd) < 44)
		return 0;
	else {
		int result = 0; // 0-login-server request done, 1-player not found, 2-gm level too low, 3-login-server offline, 4-current group level > VIP group level
		char esc_name[NAME_LENGTH*2+1];
		char answer = true;
		int aid = RFIFOL(fd,2); // account_id of who ask (-1 if server itself made this request)
		const char* name = RFIFOCP(fd,6); // name of the target character
		int operation = RFIFOW(fd,30); // type of operation @see enum chrif_req_op
		int32 timediff = 0;
		int val1 = 0, sex = SEX_MALE;

		if (operation == CHRIF_OP_LOGIN_BAN || operation == CHRIF_OP_LOGIN_VIP) {
			timediff = RFIFOL(fd, 32);
			val1 = RFIFOL(fd, 36);
		} else if (operation == CHRIF_OP_CHANGECHARSEX)
			sex = RFIFOB(fd, 32);
		RFIFOSKIP(fd,44);

		Sql_EscapeStringLen(sql_handle, esc_name, name, strnlen(name, NAME_LENGTH));
		if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `account_id`, `char_id` FROM `%s` WHERE `name` = '%s'", schema_config.char_db, esc_name) )
			Sql_ShowDebug(sql_handle);
		else if( Sql_NumRows(sql_handle) == 0 ) {
			result = 1; // 1-player not found
		}
		else if( SQL_SUCCESS != Sql_NextRow(sql_handle) ) {
			Sql_ShowDebug(sql_handle);
			result = 1;
		} else {
			int t_aid; // target account id
			int t_cid; // target char id
			char* data;

			Sql_GetData(sql_handle, 0, &data, NULL); t_aid = atoi(data);
			Sql_GetData(sql_handle, 1, &data, NULL); t_cid = atoi(data);
			Sql_FreeResult(sql_handle);

			if(!chlogif_isconnected())
				result = 3; // 3-login-server offline
			//FIXME: need to move this check to login server [ultramage]
			//	if( acc != -1 && isGM(acc) < isGM(account_id) )
			//		result = 2; // 2-gm level too low
			else {
				//! NOTE: See src/char/chrif.hpp::enum chrif_req_op for the number
				switch( operation ) {
					case CHRIF_OP_LOGIN_BLOCK: // block
						WFIFOHEAD(login_fd,10);
						WFIFOW(login_fd,0) = 0x2724;
						WFIFOL(login_fd,2) = t_aid;
						WFIFOL(login_fd,6) = 5; // new account status
						WFIFOSET(login_fd,10);
						break;
					case CHRIF_OP_LOGIN_BAN: // ban
						WFIFOHEAD(login_fd,10);
						WFIFOW(login_fd, 0) = 0x2725;
						WFIFOL(login_fd, 2) = t_aid;
						WFIFOL(login_fd, 6) = timediff;
						WFIFOSET(login_fd,10);
						break;
					case CHRIF_OP_LOGIN_UNBLOCK: // unblock
						WFIFOHEAD(login_fd,10);
						WFIFOW(login_fd,0) = 0x2724;
						WFIFOL(login_fd,2) = t_aid;
						WFIFOL(login_fd,6) = 0; // new account status
						WFIFOSET(login_fd,10);
						break;
					case CHRIF_OP_LOGIN_UNBAN: // unban
						WFIFOHEAD(login_fd,6);
						WFIFOW(login_fd,0) = 0x272a;
						WFIFOL(login_fd,2) = t_aid;
						WFIFOSET(login_fd,6);
						break;
					case CHRIF_OP_LOGIN_CHANGESEX: // changesex
						answer = false;
						WFIFOHEAD(login_fd,6);
						WFIFOW(login_fd,0) = 0x2727;
						WFIFOL(login_fd,2) = t_aid;
						WFIFOSET(login_fd,6);
						break;
					case CHRIF_OP_LOGIN_VIP: // vip
						answer = (val1&4); // vip_req val1=type, &1 login send return, &2 update timestamp, &4 map send answer
						chlogif_reqvipdata(t_aid, val1, timediff, fd);
						break;
					case CHRIF_OP_CHANGECHARSEX: // changecharsex
						answer = false;
						chlogif_parse_ackchangecharsex(t_cid, sex);
						break;
				} //end switch operation
			} //login is connected
		}

		// send answer if a player asks, not if the server asks
		if( aid != -1 && answer) { // Don't send answer for changesex/changecharsex
			WFIFOHEAD(fd,34);
			WFIFOW(fd, 0) = 0x2b0f;
			WFIFOL(fd, 2) = aid;
			safestrncpy(WFIFOCP(fd,6), name, NAME_LENGTH);
			WFIFOW(fd,30) = operation;
			WFIFOW(fd,32) = result;
			WFIFOSET(fd,34);
		}
	}
	return 1;
}

/**
 * Transmit the acknowledgement of divorce of partner_id1 and partner_id2
 * Update the list associated and transmit the new ranking
 * @param partner_id1: char id1 divorced
 * @param partner_id2: char id2 divorced
 */
void chmapif_send_ackdivorce(int partner_id1, int partner_id2){
	unsigned char buf[11];
	WBUFW(buf,0) = 0x2b12;
	WBUFL(buf,2) = partner_id1;
	WBUFL(buf,6) = partner_id2;
	chmapif_sendall(buf,10);
}

/**
 * Received a divorce request
 * @param fd: wich fd to parse from
 * @return : 0 not enough data received, 1 success
 */
int chmapif_parse_reqdivorce(int fd){
	if( RFIFOREST(fd) < 10 )
		return 0;
	char_divorce_char_sql(RFIFOL(fd,2), RFIFOL(fd,6));
	RFIFOSKIP(fd,10);
	return 1;
}

/**
 *  Character disconnected set online 0
 * @author [Wizputer]
 * @param fd: wich fd to parse from
 * @return : 0 not enough data received, 1 success
 */
int chmapif_parse_setcharoffline(int fd){
	if (RFIFOREST(fd) < 6)
		return 0;
	char_set_char_offline(RFIFOL(fd,2),RFIFOL(fd,6));
	RFIFOSKIP(fd,10);
	return 1;
}


/**
 * Reset all chars to offline
 * @author [Wizputer]
 * @param fd: wich fd to parse from
 * @param id: wich map_serv id
 * @return : 0 not enough data received, 1 success
 */
int chmapif_parse_setalloffline(int fd, int id){
	char_set_all_offline(id);
	RFIFOSKIP(fd,2);
	return 1;
}

/**
 * Character set online
 * @author [Wizputer]
 * @param fd: wich fd to parse from
 * @param id: wich map_serv id
 * @return : 0 not enough data received, 1 success
 */
int chmapif_parse_setcharonline(int fd, int id){
	if (RFIFOREST(fd) < 10)
		return 0;
	char_set_char_online(id, RFIFOL(fd,2),RFIFOL(fd,6));
	RFIFOSKIP(fd,10);
	return 1;
}

/**
 * Build and send fame ranking lists
 * @author [DracoRPG]
 * @param fd: wich fd to parse from
 * @param id: wich map_serv id
 * @return : 0 not enough data received, 1 success
 */
int chmapif_parse_reqfamelist(int fd){
	if (RFIFOREST(fd) < 2)
		return 0;
	char_read_fame_list();
	chmapif_send_fame_list(-1);
	RFIFOSKIP(fd,2);
	return 1;
}

/**
 * Request to save status change data.s
 * @author [Skotlex]
 * @param fd: wich fd to parse from
 * @return : 0 not enough data received, 1 success
 */
int chmapif_parse_save_scdata(int fd){
	if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
		return 0;
	{
#ifdef ENABLE_SC_SAVING
		int count, aid, cid;

		aid = RFIFOL(fd, 4);
		cid = RFIFOL(fd, 8);
		count = RFIFOW(fd, 12);

		// Whatever comes from the mapserver, now is the time to drop previous entries
		if( Sql_Query( sql_handle, "DELETE FROM `%s` where `account_id` = %d and `char_id` = %d;", schema_config.scdata_db, aid, cid ) != SQL_SUCCESS ){
			Sql_ShowDebug( sql_handle );
		}
		else if( count > 0 )
		{
			struct status_change_data data;
			StringBuf buf;
			int i;

			StringBuf_Init(&buf);
			StringBuf_Printf(&buf, "INSERT INTO `%s` (`account_id`, `char_id`, `type`, `tick`, `val1`, `val2`, `val3`, `val4`) VALUES ", schema_config.scdata_db);
			for( i = 0; i < count; ++i )
			{
				memcpy (&data, RFIFOP(fd, 14+i*sizeof(struct status_change_data)), sizeof(struct status_change_data));
				if( i > 0 )
					StringBuf_AppendStr(&buf, ", ");
				StringBuf_Printf(&buf, "('%d','%d','%hu','%" PRtf "','%ld','%ld','%ld','%ld')", aid, cid,
					data.type, data.tick, data.val1, data.val2, data.val3, data.val4);
			}
			if( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
				Sql_ShowDebug(sql_handle);
			StringBuf_Destroy(&buf);
		}
#endif
		RFIFOSKIP(fd, RFIFOW(fd, 2));
	}
	return 1;
}

/**
 * map-server keep alive packet, awnser back map that we alive as well
 * @param fd: wich fd to parse from
 * @return : 0 not enough data received, 1 success
 */
int chmapif_parse_keepalive(int fd){
	WFIFOHEAD(fd,2);
	WFIFOW(fd,0) = 0x2b24;
	WFIFOSET(fd,2);
	RFIFOSKIP(fd,2);
	return 1;
}

/**
 * auth request from map-server
 * @param fd: wich fd to parse from
 * @return : 0 not enough data received, 1 success
 */
int chmapif_parse_reqauth(int fd, int id){
	if (RFIFOREST(fd) < 20)
		return 0;
	else {
		uint32 account_id;
		uint32 char_id;
		uint32 login_id1;
		unsigned char sex;
		uint32 ip;
		struct auth_node* node;
		struct mmo_charstatus* cd;
		struct mmo_charstatus char_dat;
		bool autotrade;

		DBMap*  auth_db = char_get_authdb();
		DBMap* char_db_ = char_get_chardb();

		account_id = RFIFOL(fd,2);
		char_id    = RFIFOL(fd,6);
		login_id1  = RFIFOL(fd,10);
		sex        = RFIFOB(fd,14);
		ip         = ntohl(RFIFOL(fd,15));
		autotrade  = RFIFOB(fd,19) != 0;
		RFIFOSKIP(fd,20);

		node = (struct auth_node*)idb_get(auth_db, account_id);
		cd = (struct mmo_charstatus*)uidb_get(char_db_,char_id);
		if( cd == NULL )
		{	//Really shouldn't happen. (or autotrade)
				char_mmo_char_fromsql(char_id, &char_dat, true);
				cd = (struct mmo_charstatus*)uidb_get(char_db_,char_id);
		}
		if( runflag == CHARSERVER_ST_RUNNING && autotrade && cd ){
			uint16 mmo_charstatus_len = sizeof(struct mmo_charstatus) + 25;
			if (cd->sex == SEX_ACCOUNT)
				cd->sex = sex;

			WFIFOHEAD(fd,mmo_charstatus_len);
			WFIFOW(fd,0) = 0x2afd;
			WFIFOW(fd,2) = mmo_charstatus_len;
			WFIFOL(fd,4) = account_id;
			WFIFOL(fd,8) = 0;
			WFIFOL(fd,12) = 0;
			WFIFOL(fd,16) = 0;
			WFIFOL(fd,20) = 0;
			WFIFOB(fd,24) = 0;
			memcpy(WFIFOP(fd,25), cd, sizeof(struct mmo_charstatus));
			WFIFOSET(fd, WFIFOW(fd,2));

			char_set_char_online(id, char_id, account_id);
		} else if( runflag == CHARSERVER_ST_RUNNING &&
			cd != NULL &&
			node != NULL &&
			node->account_id == account_id &&
			node->char_id == char_id &&
			node->login_id1 == login_id1
			//&& node->ip == ip
#if PACKETVER < 20141016
			&& node->sex == sex
#endif
			)
		{// auth ok
			uint16 mmo_charstatus_len = sizeof(struct mmo_charstatus) + 25;
			if (cd->sex == SEX_ACCOUNT)
				cd->sex = sex;

			WFIFOHEAD(fd,mmo_charstatus_len);
			WFIFOW(fd,0) = 0x2afd;
			WFIFOW(fd,2) = mmo_charstatus_len;
			WFIFOL(fd,4) = account_id;
			WFIFOL(fd,8) = node->login_id1;
			WFIFOL(fd,12) = node->login_id2;
			WFIFOL(fd,16) = (uint32)node->expiration_time; // FIXME: will wrap to negative after "19-Jan-2038, 03:14:07 AM GMT"
			WFIFOL(fd,20) = node->group_id;
			WFIFOB(fd,24) = node->changing_mapservers;
			memcpy(WFIFOP(fd,25), cd, sizeof(struct mmo_charstatus));
			WFIFOSET(fd, WFIFOW(fd,2));

			// only use the auth once and mark user online
			idb_remove(auth_db, account_id);
			char_set_char_online(id, char_id, account_id);
		} else {// auth failed
			WFIFOHEAD(fd,19);
			WFIFOW(fd,0) = 0x2b27;
			WFIFOL(fd,2) = account_id;
			WFIFOL(fd,6) = char_id;
			WFIFOL(fd,10) = login_id1;
			WFIFOB(fd,14) = sex;
			WFIFOL(fd,15) = htonl(ip);
			WFIFOSET(fd,19);
		}
	}
	return 1;
}

/**
 * ip address update
 * @param fd: wich fd to parse from
 * @return : 0 not enough data received, 1 success
 */
int chmapif_parse_updmapip(int fd, int id){
	if (RFIFOREST(fd) < 6) 
		return 0;
	map_server[id].ip = ntohl(RFIFOL(fd, 2));
	ShowInfo("Updated IP address of map-server #%d to %d.%d.%d.%d.\n", id, CONVIP(map_server[id].ip));
	RFIFOSKIP(fd,6);
	return 1;
}

/**
 * Received an update of fame point  for char_id cid
 * Update the list associated and transmit the new ranking
 * @param fd: wich fd to parse from
 * @return : 0 not enough data received, 1 success
 */
int chmapif_parse_updfamelist(int fd){
    if (RFIFOREST(fd) < 11)
        return 0;
    {
            int cid = RFIFOL(fd, 2);
            int fame = RFIFOL(fd, 6);
            char type = RFIFOB(fd, 10);
            int size;
            struct fame_list* list;
            int player_pos;
            int fame_pos;

            switch(type)
            {
				case RANK_BLACKSMITH:	size = fame_list_size_smith;	list = smith_fame_list;		break;
				case RANK_ALCHEMIST:	size = fame_list_size_chemist;	list = chemist_fame_list;	break;
				case RANK_TAEKWON:		size = fame_list_size_taekwon;	list = taekwon_fame_list;	break;
				default:				size = 0;						list = NULL;				break;
            }

            ARR_FIND(0, size, player_pos, list[player_pos].id == cid);// position of the player
            ARR_FIND(0, size, fame_pos, list[fame_pos].fame <= fame);// where the player should be

            if( player_pos == size && fame_pos == size )
                    ;// not on list and not enough fame to get on it
            else if( fame_pos == player_pos )
            {// same position
                    list[player_pos].fame = fame;
                    chmapif_update_fame_list(type, player_pos, fame);
            }
            else
            {// move in the list
                    if( player_pos == size )
                    {// new ranker - not in the list
                            ARR_MOVE(size - 1, fame_pos, list, struct fame_list);
                            list[fame_pos].id = cid;
                            list[fame_pos].fame = fame;
                            char_loadName(cid, list[fame_pos].name);
                    }
                    else
                    {// already in the list
                            if( fame_pos == size )
                                    --fame_pos;// move to the end of the list
                            ARR_MOVE(player_pos, fame_pos, list, struct fame_list);
                            list[fame_pos].fame = fame;
                    }
                    chmapif_send_fame_list(-1);
            }

            RFIFOSKIP(fd,11);
    }
    return 1;
}

/*
 * HZ 0x2b2b
 * Transmist vip data to mapserv
 */
int chmapif_vipack(int mapfd, uint32 aid, uint32 vip_time, uint32 groupid, uint8 flag) {
#ifdef VIP_ENABLE
	uint8 buf[15];
	WBUFW(buf,0) = 0x2b2b;
	WBUFL(buf,2) = aid;
	WBUFL(buf,6) = vip_time;
	WBUFL(buf,10) = groupid;
	WBUFB(buf,14) = flag;
	chmapif_send(mapfd,buf,15);  // inform the mapserv back
#endif
	return 0;
}

int chmapif_parse_reqcharban(int fd){
	if (RFIFOREST(fd) < 10+NAME_LENGTH)
		return 0;
	else {
		//int aid = RFIFOL(fd,2); aid of player who as requested the ban
		int timediff = RFIFOL(fd,6);
		const char* name = RFIFOCP(fd,10); // name of the target character
		char esc_name[NAME_LENGTH*2+1];
		RFIFOSKIP(fd,10+NAME_LENGTH);

		Sql_EscapeStringLen(sql_handle, esc_name, name, strnlen(name, NAME_LENGTH));
		if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `account_id`,`char_id`,`unban_time` FROM `%s` WHERE `name` = '%s'", schema_config.char_db, esc_name) )
			Sql_ShowDebug(sql_handle);
		else if( Sql_NumRows(sql_handle) == 0 ){
			return 1; // 1-player not found
		}
		else if( SQL_SUCCESS != Sql_NextRow(sql_handle) ){
			Sql_ShowDebug(sql_handle);
			Sql_FreeResult(sql_handle);
			return 1;
		} else {
			int t_cid=0,t_aid=0;
			char* data;
			time_t unban_time;
			time_t now = time(NULL);
			SqlStmt* stmt = SqlStmt_Malloc(sql_handle);

			Sql_GetData(sql_handle, 0, &data, NULL); t_aid = atoi(data);
			Sql_GetData(sql_handle, 1, &data, NULL); t_cid = atoi(data);
			Sql_GetData(sql_handle, 2, &data, NULL); unban_time = atol(data);
			Sql_FreeResult(sql_handle);

			if(timediff<0 && unban_time==0) 
				return 1; //attemp to reduce time of a non banned account ?!?
			else if(unban_time<now) unban_time=now; //new entry
			unban_time += timediff; //alterate the time
			if( unban_time < now ) unban_time=0; //we have totally reduce the time

			if( SQL_SUCCESS != SqlStmt_Prepare(stmt,
					  "UPDATE `%s` SET `unban_time` = ? WHERE `char_id` = ? LIMIT 1",
					  schema_config.char_db)
				|| SQL_SUCCESS != SqlStmt_BindParam(stmt,  0, SQLDT_LONG,   (void*)&unban_time,   sizeof(unban_time))
				|| SQL_SUCCESS != SqlStmt_BindParam(stmt,  1, SQLDT_INT,    (void*)&t_cid,     sizeof(t_cid))
				|| SQL_SUCCESS != SqlStmt_Execute(stmt)

				)
			{
				SqlStmt_ShowDebug(stmt);
				SqlStmt_Free(stmt);
				return 1;
			}
			SqlStmt_Free(stmt);

			// condition applies; send to all map-servers to disconnect the player
			if( unban_time > now ) {
					unsigned char buf[11];
					WBUFW(buf,0) = 0x2b14;
					WBUFL(buf,2) = t_cid;
					WBUFB(buf,6) = 2;
					WBUFL(buf,7) = (unsigned int)unban_time;
					chmapif_sendall(buf, 11);
					// disconnect player if online on char-server
					char_disconnect_player(t_aid);
			}
		}
	}
	return 1;
}

int chmapif_parse_reqcharunban(int fd){
	if (RFIFOREST(fd) < 6+NAME_LENGTH)
		return 0;
	else {
		const char* name = RFIFOCP(fd,6);
		char esc_name[NAME_LENGTH*2+1];
		RFIFOSKIP(fd,6+NAME_LENGTH);

		Sql_EscapeStringLen(sql_handle, esc_name, name, strnlen(name, NAME_LENGTH));
		if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `unban_time` = '0' WHERE `name` = '%s' LIMIT 1", schema_config.char_db, esc_name) ) {
			Sql_ShowDebug(sql_handle);
			return 1;
		}
	}
	return 1;
}

/**
 * ZA 0x2b2d
 * <cmd>.W <char_id>.L
 * AZ 0x2b2f
 * <cmd>.W <len>.W <cid>.L <count>.B { <bonus_script_data>.?B }
 * Get bonus_script data(s) from table to load then send to player
 * @param fd
 * @author [Cydh]
 **/
int chmapif_bonus_script_get(int fd) {
	if (RFIFOREST(fd) < 6)
		return 0;
	else {
		uint8 num_rows = 0;
		uint32 cid = RFIFOL(fd,2);
		struct bonus_script_data tmp_bsdata;
		SqlStmt* stmt = SqlStmt_Malloc(sql_handle);

		RFIFOSKIP(fd,6);

		if (SQL_ERROR == SqlStmt_Prepare(stmt,
			"SELECT `script`, `tick`, `flag`, `type`, `icon` FROM `%s` WHERE `char_id` = '%d' LIMIT %d",
			schema_config.bonus_script_db, cid, MAX_PC_BONUS_SCRIPT) ||
			SQL_ERROR == SqlStmt_Execute(stmt) ||
			SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_STRING, &tmp_bsdata.script_str, sizeof(tmp_bsdata.script_str), NULL, NULL) ||
			SQL_ERROR == SqlStmt_BindColumn(stmt, 1, SQLDT_INT64, &tmp_bsdata.tick, 0, NULL, NULL) ||
			SQL_ERROR == SqlStmt_BindColumn(stmt, 2, SQLDT_UINT16, &tmp_bsdata.flag, 0, NULL, NULL) ||
			SQL_ERROR == SqlStmt_BindColumn(stmt, 3, SQLDT_UINT8,  &tmp_bsdata.type, 0, NULL, NULL) ||
			SQL_ERROR == SqlStmt_BindColumn(stmt, 4, SQLDT_INT16,  &tmp_bsdata.icon, 0, NULL, NULL)
			)
		{
			SqlStmt_ShowDebug(stmt);
			SqlStmt_Free(stmt);
			return 1;
		}

		if ((num_rows = (uint8)SqlStmt_NumRows(stmt)) > 0) {
			uint8 i;
			uint32 size = 9 + num_rows * sizeof(struct bonus_script_data);

			WFIFOHEAD(fd, size);
			WFIFOW(fd, 0) = 0x2b2f;
			WFIFOW(fd, 2) = size;
			WFIFOL(fd, 4) = cid;
			WFIFOB(fd, 8) = num_rows;

			for (i = 0; i < num_rows && SQL_SUCCESS == SqlStmt_NextRow(stmt); i++) {
				struct bonus_script_data bsdata;
				memset(&bsdata, 0, sizeof(bsdata));
				memset(bsdata.script_str, '\0', sizeof(bsdata.script_str));

				safestrncpy(bsdata.script_str, tmp_bsdata.script_str, strlen(tmp_bsdata.script_str)+1);
				bsdata.tick = tmp_bsdata.tick;
				bsdata.flag = tmp_bsdata.flag;
				bsdata.type = tmp_bsdata.type;
				bsdata.icon = tmp_bsdata.icon;
				memcpy(WFIFOP(fd, 9 + i * sizeof(struct bonus_script_data)), &bsdata, sizeof(struct bonus_script_data));
			}

			WFIFOSET(fd, size);

			ShowInfo("Bonus Script loaded for CID=%d. Total: %d.\n", cid, i);

			if (SQL_ERROR == SqlStmt_Prepare(stmt,"DELETE FROM `%s` WHERE `char_id`='%d'",schema_config.bonus_script_db,cid) ||
				SQL_ERROR == SqlStmt_Execute(stmt))
				SqlStmt_ShowDebug(stmt);
		}
		SqlStmt_Free(stmt);
	}
	return 1;
}

/**
 * ZA 0x2b2e
 * <cmd>.W <len>.W <char_id>.L <count>.B { <bonus_script>.?B }
 * Save bonus_script data(s) to the table
 * @param fd
 * @author [Cydh]
 **/
int chmapif_bonus_script_save(int fd) {
	if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
		return 0;
	else {
		uint32 cid = RFIFOL(fd,4);
		uint8 count = RFIFOB(fd,8);

		if (SQL_ERROR == Sql_Query(sql_handle,"DELETE FROM `%s` WHERE `char_id` = '%d'", schema_config.bonus_script_db, cid))
			Sql_ShowDebug(sql_handle);

		if (count > 0) {
			char esc_script[MAX_BONUS_SCRIPT_LENGTH*2];
			struct bonus_script_data bsdata;
			StringBuf buf;
			uint8 i;

			StringBuf_Init(&buf);
			StringBuf_Printf(&buf, "INSERT INTO `%s` (`char_id`, `script`, `tick`, `flag`, `type`, `icon`) VALUES ", schema_config.bonus_script_db);
			for (i = 0; i < count; ++i) {
				memcpy(&bsdata, RFIFOP(fd, 9 + i*sizeof(struct bonus_script_data)), sizeof(struct bonus_script_data));
				Sql_EscapeString(sql_handle, esc_script, bsdata.script_str);
				if (i > 0)
					StringBuf_AppendStr(&buf,", ");
				StringBuf_Printf(&buf, "('%d','%s','%" PRtf "','%d','%d','%d')", cid, esc_script, bsdata.tick, bsdata.flag, bsdata.type, bsdata.icon);
			}
			if (SQL_ERROR == Sql_QueryStr(sql_handle,StringBuf_Value(&buf)))
				Sql_ShowDebug(sql_handle);

			StringBuf_Destroy(&buf);
			ShowInfo("Bonus Script saved for CID=%d. Total: %d.\n", cid, count);
		}
		RFIFOSKIP(fd,RFIFOW(fd,2));
	}
	return 1;
}

/**
 * Inform the mapserv wheater his login attemp to us was a success or not
 * @param fd : file descriptor to parse, (link to mapserv)
 * @param errCode 0:success, 3:fail
 */
void chmapif_connectack(int fd, uint8 errCode){
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x2af9;
	WFIFOB(fd,2) = errCode;
	WFIFOSET(fd,3);
}

/**
 * Entry point from map-server to char-server.
 * Function that checks incoming command, then splits it to the correct handler.
 * If not found any hander here transmis packet to inter
 * @param fd: file descriptor to parse, (link to map-serv)
 * @return 0=invalid server,marked for disconnection,unknow packet; 1=success
 */
int chmapif_parse(int fd){
	int id; //mapserv id

	ARR_FIND( 0, ARRAYLENGTH(map_server), id, map_server[id].fd == fd );
	if( id == ARRAYLENGTH(map_server) )
	{// not a map server
		ShowDebug("parse_frommap: Disconnecting invalid session #%d (is not a map-server)\n", fd);
		do_close(fd);
		return 0;
	}
	if( session[fd]->flag.eof )
	{
		do_close(fd);
		map_server[id].fd = -1;
		chmapif_on_disconnect(id);
		return 0;
	}

	while(RFIFOREST(fd) >= 2){
		int next=1;
		switch(RFIFOW(fd,0)){
			case 0x2afa: next=chmapif_parse_getmapname(fd,id); break;
			case 0x2afc: next=chmapif_parse_askscdata(fd); break;
			case 0x2afe: next=chmapif_parse_getusercount(fd,id); break; //get nb user
			case 0x2aff: next=chmapif_parse_regmapuser(fd,id); break; //register users
			case 0x2b01: next=chmapif_parse_reqsavechar(fd,id); break;
			case 0x2b02: next=chmapif_parse_authok(fd); break;
			case 0x2b05: next=chmapif_parse_reqchangemapserv(fd); break;
			case 0x2b07: next=chmapif_parse_askrmfriend(fd); break;
			case 0x2b08: next=chmapif_parse_reqcharname(fd); break;
			case 0x2b0a: next=chmapif_parse_req_skillcooldown(fd); break;
			case 0x2b0c: next=chmapif_parse_reqnewemail(fd); break;
			case 0x2b0e: next=chmapif_parse_fwlog_changestatus(fd); break;
			case 0x2b10: next=chmapif_parse_updfamelist(fd); break;
			case 0x2b11: next=chmapif_parse_reqdivorce(fd); break;
			case 0x2b13: next=chmapif_parse_updmapip(fd,id); break;
			case 0x2b15: next=chmapif_parse_req_saveskillcooldown(fd); break;
			case 0x2b17: next=chmapif_parse_setcharoffline(fd); break;
			case 0x2b18: next=chmapif_parse_setalloffline(fd,id); break;
			case 0x2b19: next=chmapif_parse_setcharonline(fd,id); break;
			case 0x2b1a: next=chmapif_parse_reqfamelist(fd); break;
			case 0x2b1c: next=chmapif_parse_save_scdata(fd); break;
			case 0x2b23: next=chmapif_parse_keepalive(fd); break;
			case 0x2b26: next=chmapif_parse_reqauth(fd,id); break;
			case 0x2b28: next=chmapif_parse_reqcharban(fd); break; //charban
			case 0x2b2a: next=chmapif_parse_reqcharunban(fd); break; //charunban
			//case 0x2b2c: /*free*/; break;
			case 0x2b2d: next=chmapif_bonus_script_get(fd); break; //Load data
			case 0x2b2e: next=chmapif_bonus_script_save(fd); break;//Save data
			default:
			{
					// inter server - packet
					int r = inter_parse_frommap(fd);
					if (r == 1) break;		// processed
					if (r == 2) return 0;	// need more packet
					// no inter server packet. no char server packet -> disconnect
					ShowError("Unknown packet 0x%04x from map server, disconnecting.\n", RFIFOW(fd,0));
					set_eof(fd);
					return 0;
			}
		} // switch
		if(next==0) return 0; //avoid processing rest of packet
	} // while
	return 1;
}


// Initialization process (currently only initialization inter_mapif)
int chmapif_init(int fd){
	return inter_mapif_init(fd);
}

/**
 * Initializes a server structure.
 * @param id: id of map-serv (should be >0, FIXME)
 */
void chmapif_server_init(int id) {
	memset(&map_server[id], 0, sizeof(map_server[id]));
	map_server[id].fd = -1;
}

/**
 * Destroys a server structure.
 * @param id: id of map-serv (should be >0, FIXME)
 */
void chmapif_server_destroy(int id){
	if( map_server[id].fd == -1 ){
		do_close(map_server[id].fd);
		map_server[id].fd = -1;
	}
}

/**
 * chmapif constructor
 *  Initialisation, function called at start of the char-serv.
 */
void do_init_chmapif(void){
	int i;
	for( i = 0; i < ARRAYLENGTH(map_server); ++i )
		chmapif_server_init(i);
}

/**
 * Resets all the data related to a server.
 *  Actually destroys then recreates the struct.
 * @param id: id of map-serv (should be >0, FIXME)
 */
void chmapif_server_reset(int id){
	int j = 0;
	unsigned char buf[16384];
	int fd = map_server[id].fd;
	DBMap* online_char_db = char_get_onlinedb();

	//Notify other map servers that this one is gone. [Skotlex]
	WBUFW(buf,0) = 0x2b20;
	WBUFL(buf,4) = htonl(map_server[id].ip);
	WBUFW(buf,8) = htons(map_server[id].port);
	for(uint16 i = 0; i < map_server[id].map.size(); i++)
		if (map_server[id].map[i])
			WBUFW(buf,10+(j++)*4) = map_server[id].map[i];
	if (j > 0) {
		WBUFW(buf,2) = j * 4 + 10;
		chmapif_sendallwos(fd, buf, WBUFW(buf,2));
	}
	online_char_db->foreach(online_char_db,char_db_setoffline,id); //Tag relevant chars as 'in disconnected' server.
	chmapif_server_destroy(id);
	chmapif_server_init(id);
}


/**
 * Called when the connection to Map Server is disconnected.
 * @param id: id of map-serv (should be >0, FIXME)
 */
void chmapif_on_disconnect(int id){
	ShowStatus("Map-server #%d has disconnected.\n", id);
	chmapif_server_reset(id);
}


/**
 * chmapif destructor
 *  dealloc..., function called at exit of the char-serv
 */
void do_final_chmapif(void){
	int i;
	for( i = 0; i < ARRAYLENGTH(map_server); ++i )
		chmapif_server_destroy(i);
}
