// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "int_mail.hpp"

#include <cstdlib>
#include <cstring>
#include <memory>

#include <common/mmo.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/sql.hpp>
#include <common/strlib.hpp>
#include <common/utilities.hpp>

#include "char.hpp"
#include "char_mapif.hpp"
#include "inter.hpp"

using namespace rathena;

bool mail_loadmessage(int32 mail_id, struct mail_message* msg);
void mapif_Mail_return( int32 fd, uint32 char_id, int32 mail_id, uint32 account_id_receiver = 0, uint32 account_id_sender = 0 );
bool mapif_Mail_delete( int32 fd, uint32 char_id, int32 mail_id, uint32 account_id = 0 );

int32 mail_fromsql(uint32 char_id, struct mail_data* md)
{
	int32 i;
	char *data;

	memset(md, 0, sizeof(struct mail_data));
	md->amount = 0;
	md->full = false;

	// First we prefill the msg ids
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `id` FROM `%s` WHERE `dest_id`='%d' AND `status` < 3 ORDER BY `id` LIMIT %d", schema_config.mail_db, char_id, MAIL_MAX_INBOX + 1) ){
		Sql_ShowDebug(sql_handle);
		return 0;
	}

	md->full = (Sql_NumRows(sql_handle) > MAIL_MAX_INBOX);

	for( i = 0; i < MAIL_MAX_INBOX && SQL_SUCCESS == Sql_NextRow(sql_handle); i++ ){
		Sql_GetData(sql_handle, 0, &data, nullptr); md->msg[i].id = atoi(data);
	}

	md->amount = i;
	Sql_FreeResult(sql_handle);

	// Now we load them
	for( i = 0; i < md->amount; i++ ){
		if( !mail_loadmessage( md->msg[i].id, &md->msg[i] ) ){
			return 0;
		}
	}

	md->unchecked = 0;
	md->unread = 0;
	for (i = 0; i < md->amount; i++)
	{
		struct mail_message *msg = &md->msg[i];

		if( msg->status == MAIL_NEW )
		{
			if ( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `status` = '%d' WHERE `id` = '%d'", schema_config.mail_db, MAIL_UNREAD, msg->id) )
				Sql_ShowDebug(sql_handle);

			msg->status = MAIL_UNREAD;
			md->unchecked++;
		}
		else if ( msg->status == MAIL_UNREAD )
			md->unread++;
	}

	ShowInfo("mail load complete from DB - id: %d (total: %d)\n", char_id, md->amount);
	return 1;
}

/// Stores a single message in the database.
/// Returns the message's ID if successful (or 0 if it fails).
int32 mail_savemessage(struct mail_message* msg)
{
	StringBuf buf;
	SqlStmt stmt{ *sql_handle };
	int32 i, j;
	bool found = false;

	if( SQL_ERROR == Sql_QueryStr( sql_handle, "START TRANSACTION" ) ){
		Sql_ShowDebug( sql_handle );
		return 0;
	}

	// build message save query
	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "INSERT INTO `%s` (`send_name`, `send_id`, `dest_name`, `dest_id`, `title`, `message`, `time`, `status`, `zeny`,`type`", schema_config.mail_db);
	StringBuf_Printf(&buf, ") VALUES (?, '%d', ?, '%d', ?, ?, '%lu', '%d', '%d', '%d'", msg->send_id, msg->dest_id, (unsigned long)msg->timestamp, msg->status, msg->zeny, msg->type);
	StringBuf_AppendStr(&buf, ")");

	// prepare and execute query
	if( SQL_SUCCESS != stmt.PrepareStr(StringBuf_Value(&buf))
	||  SQL_SUCCESS != stmt.BindParam(0, SQLDT_STRING, msg->send_name, strnlen(msg->send_name, NAME_LENGTH))
	||  SQL_SUCCESS != stmt.BindParam(1, SQLDT_STRING, msg->dest_name, strnlen(msg->dest_name, NAME_LENGTH))
	||  SQL_SUCCESS != stmt.BindParam(2, SQLDT_STRING, msg->title, strnlen(msg->title, MAIL_TITLE_LENGTH))
	||  SQL_SUCCESS != stmt.BindParam(3, SQLDT_STRING, msg->body, strnlen(msg->body, MAIL_BODY_LENGTH))
	||  SQL_SUCCESS != stmt.Execute() )
	{
		SqlStmt_ShowDebug(stmt);
		StringBuf_Destroy(&buf);
		Sql_QueryStr( sql_handle, "ROLLBACK" );
		return msg->id = 0;
	} else
		msg->id = (int32)stmt.LastInsertId();
	
	StringBuf_Clear(&buf);
	StringBuf_Printf(&buf,"INSERT INTO `%s` (`id`, `index`, `amount`, `nameid`, `refine`, `attribute`, `identify`, `unique_id`, `bound`, `enchantgrade`", schema_config.mail_attachment_db);
	for (j = 0; j < MAX_SLOTS; j++)
		StringBuf_Printf(&buf, ", `card%d`", j);
	for (j = 0; j < MAX_ITEM_RDM_OPT; ++j) {
		StringBuf_Printf(&buf, ", `option_id%d`", j);
		StringBuf_Printf(&buf, ", `option_val%d`", j);
		StringBuf_Printf(&buf, ", `option_parm%d`", j);
	}
	StringBuf_AppendStr(&buf, ") VALUES ");

	for( i = 0; i < MAIL_MAX_ITEM; i++ ){
		// skip empty and already matched entries
		if( msg->item[i].nameid == 0 )
			continue;

		if( found ){
			StringBuf_AppendStr(&buf, ",");
		}else{
			found = true;
		}

		StringBuf_Printf(&buf, "('%" PRIu64 "', '%hu', '%d', '%u', '%d', '%d', '%d', '%" PRIu64 "', '%d', '%d'", (uint64)msg->id, i, msg->item[i].amount, msg->item[i].nameid, msg->item[i].refine, msg->item[i].attribute, msg->item[i].identify, msg->item[i].unique_id, msg->item[i].bound, msg->item[i].enchantgrade);
		for (j = 0; j < MAX_SLOTS; j++)
			StringBuf_Printf(&buf, ", '%u'", msg->item[i].card[j]);
		for (j = 0; j < MAX_ITEM_RDM_OPT; ++j) {
			StringBuf_Printf(&buf, ", '%d'", msg->item[i].option[j].id);
			StringBuf_Printf(&buf, ", '%d'", msg->item[i].option[j].value);
			StringBuf_Printf(&buf, ", '%d'", msg->item[i].option[j].param);
		}
		StringBuf_AppendStr(&buf, ")");
	}

	if( found && SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) ){
		Sql_ShowDebug(sql_handle);
		msg->id = 0;
		Sql_QueryStr( sql_handle, "ROLLBACK" );
	}

	StringBuf_Destroy(&buf);

	if( msg->id && SQL_ERROR == Sql_QueryStr( sql_handle, "COMMIT" ) ){
		Sql_ShowDebug( sql_handle );
		return 0;
	}

	return msg->id;
}

/// Retrieves a single message from the database.
/// Returns true if the operation succeeds (or false if it fails).
bool mail_loadmessage(int32 mail_id, struct mail_message* msg)
{
	int32 i, j;
	StringBuf buf;
	char* data;

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `id`,`send_name`,`send_id`,`dest_name`,`dest_id`,`title`,`message`,`time`,`status`,`zeny`,`type` FROM `%s` WHERE `id` = '%d'", schema_config.mail_db, mail_id )
	||  SQL_SUCCESS != Sql_NextRow(sql_handle) ){
		Sql_ShowDebug(sql_handle);
		Sql_FreeResult(sql_handle);
		return false;
	}else{
		Sql_GetData(sql_handle, 0, &data, nullptr); msg->id = atoi(data);
		Sql_GetData(sql_handle, 1, &data, nullptr); safestrncpy(msg->send_name, data, NAME_LENGTH);
		Sql_GetData(sql_handle, 2, &data, nullptr); msg->send_id = atoi(data);
		Sql_GetData(sql_handle, 3, &data, nullptr); safestrncpy(msg->dest_name, data, NAME_LENGTH);
		Sql_GetData(sql_handle, 4, &data, nullptr); msg->dest_id = atoi(data);
		Sql_GetData(sql_handle, 5, &data, nullptr); safestrncpy(msg->title, data, MAIL_TITLE_LENGTH);
		Sql_GetData(sql_handle, 6, &data, nullptr); safestrncpy(msg->body, data, MAIL_BODY_LENGTH);
		Sql_GetData(sql_handle, 7, &data, nullptr); msg->timestamp = atoi(data);
		Sql_GetData(sql_handle, 8, &data, nullptr); msg->status = (mail_status)atoi(data);
		Sql_GetData(sql_handle, 9, &data, nullptr); msg->zeny = atoi(data);
		Sql_GetData(sql_handle,10, &data, nullptr); msg->type = (mail_inbox_type)atoi(data);

		if( msg->type == MAIL_INBOX_NORMAL && charserv_config.mail_return_days > 0 ){
			msg->scheduled_deletion = msg->timestamp + charserv_config.mail_return_days * 24 * 60 * 60;
		}else if( msg->type == MAIL_INBOX_RETURNED && charserv_config.mail_delete_days > 0 ){
			msg->scheduled_deletion = msg->timestamp + charserv_config.mail_delete_days * 24 * 60 * 60;
		}else{
			msg->scheduled_deletion = 0;
		}

		Sql_FreeResult(sql_handle);
	}

	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `amount`,`nameid`,`refine`,`attribute`,`identify`,`unique_id`,`bound`,`enchantgrade`");
	for (j = 0; j < MAX_SLOTS; j++)
		StringBuf_Printf(&buf, ",`card%d`", j);
	for (j = 0; j < MAX_ITEM_RDM_OPT; ++j) {
		StringBuf_Printf(&buf, ", `option_id%d`", j);
		StringBuf_Printf(&buf, ", `option_val%d`", j);
		StringBuf_Printf(&buf, ", `option_parm%d`", j);
	}
	StringBuf_Printf(&buf, " FROM `%s`", schema_config.mail_attachment_db);
	StringBuf_Printf(&buf, " WHERE `id` = '%d'", mail_id);
	StringBuf_AppendStr(&buf, " ORDER BY `index` ASC");
	StringBuf_Printf(&buf, " LIMIT %d", MAIL_MAX_ITEM);

	if( SQL_ERROR == Sql_Query(sql_handle, StringBuf_Value(&buf)) ){
		Sql_ShowDebug(sql_handle);
		Sql_FreeResult(sql_handle);
		StringBuf_Destroy(&buf);
		return false;
	}

	memset(msg->item, 0, sizeof(struct item) * MAIL_MAX_ITEM);

	for( i = 0; i < MAIL_MAX_ITEM && SQL_SUCCESS == Sql_NextRow(sql_handle); i++ ){
		Sql_GetData(sql_handle,0, &data, nullptr); msg->item[i].amount = (int16)atoi(data);
		Sql_GetData(sql_handle,1, &data, nullptr); msg->item[i].nameid = strtoul(data, nullptr, 10);
		Sql_GetData(sql_handle,2, &data, nullptr); msg->item[i].refine = atoi(data);
		Sql_GetData(sql_handle,3, &data, nullptr); msg->item[i].attribute = atoi(data);
		Sql_GetData(sql_handle,4, &data, nullptr); msg->item[i].identify = atoi(data);
		Sql_GetData(sql_handle,5, &data, nullptr); msg->item[i].unique_id = strtoull(data, nullptr, 10);
		Sql_GetData(sql_handle,6, &data, nullptr); msg->item[i].bound = atoi(data);
		Sql_GetData(sql_handle,7, &data, nullptr); msg->item[i].enchantgrade = atoi(data);
		msg->item[i].expire_time = 0;

		for( j = 0; j < MAX_SLOTS; j++ ){
			Sql_GetData(sql_handle,8 + j, &data, nullptr); msg->item[i].card[j] = strtoul(data, nullptr, 10);
		}

		for( j = 0; j < MAX_ITEM_RDM_OPT; j++ ){
			Sql_GetData(sql_handle, 8 + MAX_SLOTS + j * 3, &data, nullptr); msg->item[i].option[j].id = atoi(data);
			Sql_GetData(sql_handle, 9 + MAX_SLOTS + j * 3, &data, nullptr); msg->item[i].option[j].value = atoi(data);
			Sql_GetData(sql_handle,10 + MAX_SLOTS + j * 3, &data, nullptr); msg->item[i].option[j].param = atoi(data);
		}
	}

	StringBuf_Destroy(&buf);
	Sql_FreeResult(sql_handle);

	return true;
}

int32 mail_timer_sub( int32 limit, enum mail_inbox_type type ){
	// Start by deleting all expired mails sent by the server
	if( SQL_ERROR == Sql_Query( sql_handle, "DELETE FROM `%s`WHERE `type` = '%d' AND `send_id` = '0' AND `time` <= UNIX_TIMESTAMP( NOW() - INTERVAL %d DAY )", schema_config.mail_db, type, limit ) ){
		Sql_ShowDebug( sql_handle );
		return 0;
	}

	struct{
		int32 mail_id;
		int32 char_id;
		int32 account_id;
		int32 account_id_sender;
	}mails[MAIL_ITERATION_SIZE];

	if( limit <= 0 ){
		return 0;
	}

	memset(mails, 0, sizeof(mails));

	if( SQL_ERROR == Sql_Query( sql_handle,
		"SELECT `m`.`id`, `c`.`char_id`, `c`.`account_id`, `c2`.`account_id` "
		"FROM `%s` `m` "
		"INNER JOIN `%s` `c` ON `c`.`char_id`=`m`.`dest_id` "
		"INNER JOIN `%s` `c2` ON `c2`.`char_id`=`m`.`send_id` "
		"WHERE `type` = '%d' AND `time` <= UNIX_TIMESTAMP( NOW() - INTERVAL %d DAY ) "
		"ORDER BY `id` LIMIT %d",
		schema_config.mail_db, schema_config.char_db, schema_config.char_db, type, limit, MAIL_ITERATION_SIZE + 1 ) ){
		Sql_ShowDebug(sql_handle);
		return 0;
	}

	if( Sql_NumRows(sql_handle) <= 0 ){
		return 0;
	}

	for( int32 i = 0; i < MAIL_ITERATION_SIZE && SQL_SUCCESS == Sql_NextRow( sql_handle ); i++ ){
		char* data;

		Sql_GetData(sql_handle, 0, &data, nullptr); mails[i].mail_id = atoi(data);
		Sql_GetData(sql_handle, 1, &data, nullptr); mails[i].char_id = atoi(data);
		Sql_GetData(sql_handle, 2, &data, nullptr); mails[i].account_id = atoi(data);
		Sql_GetData( sql_handle, 3, &data, nullptr ); mails[i].account_id_sender = atoi( data );
	}

	Sql_FreeResult(sql_handle);

	for( int32 i = 0; i < MAIL_ITERATION_SIZE; i++ ){
		if( mails[i].mail_id == 0 ){
			break;
		}

		if( type == MAIL_INBOX_NORMAL ){
			mapif_Mail_return( 0, mails[i].char_id, mails[i].mail_id, mails[i].account_id, mails[i].account_id_sender );
		}else if( type == MAIL_INBOX_RETURNED ){
			mapif_Mail_delete( 0, mails[i].char_id, mails[i].mail_id, mails[i].account_id );
		}else{
			// Should not happen
			continue;
		}
	}

	return 0;
}

TIMER_FUNC(mail_return_timer){
	return mail_timer_sub( charserv_config.mail_return_days, MAIL_INBOX_NORMAL );
}

TIMER_FUNC(mail_delete_timer){
	return mail_timer_sub( charserv_config.mail_delete_days, MAIL_INBOX_RETURNED );
}

/*==========================================
 * Client Inbox Request
 *------------------------------------------*/
void mapif_Mail_sendinbox(int32 fd, uint32 char_id, unsigned char flag, enum mail_inbox_type type)
{
	struct mail_data md;
	mail_fromsql(char_id, &md);

	//FIXME: dumping the whole structure like this is unsafe [ultramage]
	WFIFOHEAD(fd, sizeof(md) + 10);
	WFIFOW(fd,0) = 0x3848;
	WFIFOW(fd,2) = sizeof(md) + 10;
	WFIFOL(fd,4) = char_id;
	WFIFOB(fd,8) = flag;
	WFIFOB(fd,9) = type;
	memcpy(WFIFOP(fd,10),&md,sizeof(md));
	WFIFOSET(fd,WFIFOW(fd,2));
}

void mapif_parse_Mail_requestinbox(int32 fd)
{
	mapif_Mail_sendinbox(fd, RFIFOL(fd,2), RFIFOB(fd,6), (mail_inbox_type)RFIFOB(fd,7));
}

/*==========================================
 * Mark mail as 'Read'
 *------------------------------------------*/
void mapif_parse_Mail_read(int32 fd)
{
	int32 mail_id = RFIFOL(fd,2);
	if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `status` = '%d' WHERE `id` = '%d'", schema_config.mail_db, MAIL_READ, mail_id) )
		Sql_ShowDebug(sql_handle);
}

/*==========================================
 * Client Attachment Request
 *------------------------------------------*/
bool mail_DeleteAttach(int32 mail_id){
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `id` = '%d'", schema_config.mail_attachment_db, mail_id ) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

void mapif_Mail_getattach(int32 fd, uint32 char_id, int32 mail_id, int32 type)
{
	struct mail_message msg;

	if( ( type&MAIL_ATT_ALL ) == 0 ){
		return;
	}

	if( !mail_loadmessage(mail_id, &msg) )
		return;

	if( msg.dest_id != char_id )
		return;

	if( charserv_config.mail_retrieve == 0 && msg.status != MAIL_READ )
		return;

	if( type & MAIL_ATT_ZENY ){
		if( msg.zeny > 0 ){
			if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `zeny` = 0 WHERE `id` = '%d'", schema_config.mail_db, mail_id ) ){
				Sql_ShowDebug(sql_handle);
				return;
			}
		}else{
			type &= ~MAIL_ATT_ZENY;
		}
	}

	if( type & MAIL_ATT_ITEM ){
		int32 i;

		ARR_FIND(0, MAIL_MAX_ITEM, i, msg.item[i].nameid > 0 && msg.item[i].amount > 0);

		// No item was found
		if( i == MAIL_MAX_ITEM ){
			type &= ~MAIL_ATT_ITEM;
		}else{
			if( !mail_DeleteAttach(mail_id) ){
				return;
			}
		}
	}

	if( type == MAIL_ATT_NONE )
		return; // No Attachment

	WFIFOHEAD(fd, sizeof(struct item)*MAIL_MAX_ITEM + 16);
	WFIFOW(fd,0) = 0x384a;
	WFIFOW(fd,2) = sizeof(struct item)*MAIL_MAX_ITEM + 16;
	WFIFOL(fd,4) = char_id;
	WFIFOL(fd,8) = mail_id;
	if( type & MAIL_ATT_ZENY ){
		WFIFOL(fd,12) = msg.zeny;
	}else{
		WFIFOL(fd, 12) = 0;
	}
	if( type & MAIL_ATT_ITEM ){
		memcpy(WFIFOP(fd, 16), &msg.item, sizeof(struct item)*MAIL_MAX_ITEM);
	}else{
		memset(WFIFOP(fd, 16), 0, sizeof(struct item)*MAIL_MAX_ITEM);
	}
	WFIFOSET(fd,WFIFOW(fd,2));
}

void mapif_parse_Mail_getattach(int32 fd)
{
	mapif_Mail_getattach(fd, RFIFOL(fd,2), RFIFOL(fd,6),RFIFOB(fd,10));
}

/*==========================================
 * Delete Mail
 *------------------------------------------*/
bool mapif_Mail_delete( int32 fd, uint32 char_id, int32 mail_id, uint32 account_id ){
	bool failed = false;

	if( SQL_ERROR == Sql_QueryStr( sql_handle, "START TRANSACTION" ) ||
		SQL_ERROR == Sql_Query( sql_handle, "DELETE FROM `%s` WHERE `id` = '%d'", schema_config.mail_db, mail_id ) ||
		SQL_ERROR == Sql_Query( sql_handle, "DELETE FROM `%s` WHERE `id` = '%d'", schema_config.mail_attachment_db, mail_id ) ||
		SQL_ERROR == Sql_QueryStr( sql_handle, "COMMIT" ) ){

		Sql_ShowDebug( sql_handle );
		Sql_QueryStr( sql_handle, "ROLLBACK" );

		// We do not want to trigger failure messages, if the map server did not send a request
		if( fd <= 0 ){
			return false;
		}

		failed = true;
	}

	// If the char server triggered this, check if we have to notify a map server
	if( fd <= 0 ){
		std::shared_ptr<struct online_char_data> character = util::umap_find( char_get_onlinedb(), account_id );

		// Check for online players
		if( character != nullptr && character->server >= 0 ){
			fd = map_server[character->server].fd;
		}else{
			// The request was triggered inside the character server or the player is offline now
			return !failed;
		}
	}

	WFIFOHEAD(fd,11);
	WFIFOW(fd,0) = 0x384b;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = mail_id;
	WFIFOB(fd,10) = failed;
	WFIFOSET(fd,11);

	return !failed;
}

void mapif_parse_Mail_delete(int32 fd)
{
	mapif_Mail_delete( fd, RFIFOL( fd, 2 ), RFIFOL( fd, 6 ) );
}

/*==========================================
 * Report New Mail to Map Server
 *------------------------------------------*/
void mapif_Mail_new(struct mail_message *msg)
{
	unsigned char buf[75];

	if( !msg || !msg->id )
		return;

	WBUFW(buf,0) = 0x3849;
	WBUFL(buf,2) = msg->dest_id;
	WBUFL(buf,6) = msg->id;
	memcpy(WBUFP(buf,10), msg->send_name, NAME_LENGTH);
	memcpy(WBUFP(buf,34), msg->title, MAIL_TITLE_LENGTH);
	WBUFB(buf,74) = msg->type;
	chmapif_sendall(buf, 75);
}

/*==========================================
 * Return Mail
 *------------------------------------------*/
void mapif_Mail_return( int32 fd, uint32 char_id, int32 mail_id, uint32 account_id_receiver, uint32 account_id_sender ){
	struct mail_message msg;

	if( !mail_loadmessage( mail_id, &msg ) ){
		return;
	}

	if( msg.dest_id != char_id ){
		return;
	}

	if( !mapif_Mail_delete( 0, char_id, mail_id, account_id_receiver ) ){
		// Stop processing to not duplicate the mail
		return;
	}

	// If it was sent by the server we do not want to return the mail
	if( msg.send_id == 0 ){
		return;
	}

	// If we do not want to return mails without any attachments and the request was not sent by a user
	if( fd <= 0 && !charserv_config.mail_return_empty ){
		int32 i;

		ARR_FIND( 0, MAIL_MAX_ITEM, i, msg.item[i].nameid > 0 );

		if( i == MAIL_MAX_ITEM && msg.zeny == 0 ){
			return;
		}
	}

	char temp_[MAIL_TITLE_LENGTH + 3];

	// swap sender and receiver
	std::swap( msg.send_id, msg.dest_id );
	safestrncpy( temp_, msg.send_name, NAME_LENGTH );
	safestrncpy( msg.send_name, msg.dest_name, NAME_LENGTH );
	safestrncpy( msg.dest_name, temp_, NAME_LENGTH );

	// set reply message title
	snprintf( temp_, sizeof( temp_ ), "RE:%s", msg.title );
	safestrncpy( msg.title, temp_, sizeof( temp_ ) );

	msg.status = MAIL_NEW;
	msg.type = MAIL_INBOX_RETURNED;
	msg.timestamp = time( nullptr );

	int32 new_mail = mail_savemessage( &msg );
	mapif_Mail_new( &msg );

	// If the char server triggered this, check if we have to notify a map server
	if( fd <= 0 ){
		std::shared_ptr<struct online_char_data> character = util::umap_find( char_get_onlinedb(), account_id_sender );

		// Check for online players
		if( character != nullptr && character->server >= 0 ){
			fd = map_server[character->server].fd;
		}else{
			// The request was triggered inside the character server or the player is offline now
			return;
		}
	}

	WFIFOHEAD(fd,11);
	WFIFOW(fd,0) = 0x384c;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = mail_id;
	WFIFOB(fd,10) = (new_mail == 0);
	WFIFOSET(fd,11);
}

void mapif_parse_Mail_return(int32 fd)
{
	mapif_Mail_return(fd, RFIFOL(fd,2), RFIFOL(fd,6));
}

/*==========================================
 * Send Mail
 *------------------------------------------*/
void mapif_Mail_send(int32 fd, struct mail_message* msg)
{
	int32 len = sizeof(struct mail_message) + 4;

	WFIFOHEAD(fd,len);
	WFIFOW(fd,0) = 0x384d;
	WFIFOW(fd,2) = len;
	memcpy(WFIFOP(fd,4), msg, sizeof(struct mail_message));
	WFIFOSET(fd,len);
}

void mapif_parse_Mail_send(int32 fd)
{
	struct mail_message msg;
	char esc_name[NAME_LENGTH*2+1];
	char *data;
	size_t len;

	if(RFIFOW(fd,2) != 8 + sizeof(struct mail_message))
		return;

	memcpy(&msg, RFIFOP(fd,8), sizeof(struct mail_message));

	if( msg.dest_id != 0 ){
		if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `char_id`, `name` FROM `%s` WHERE `char_id` = '%u'", schema_config.char_db, msg.dest_id) ){
			Sql_ShowDebug(sql_handle);
			return;
		}
		
		msg.dest_id = 0;
		msg.dest_name[0] = '\0';

		if( SQL_SUCCESS == Sql_NextRow(sql_handle) ){
			Sql_GetData(sql_handle, 0, &data, nullptr);
			msg.dest_id = atoi(data);
			Sql_GetData(sql_handle, 1, &data, &len);
			safestrncpy(msg.dest_name, data, NAME_LENGTH);
		}

		Sql_FreeResult(sql_handle);
	}

	// Try to find the Dest Char by Name
	Sql_EscapeStringLen(sql_handle, esc_name, msg.dest_name, strnlen(msg.dest_name, NAME_LENGTH));
	if ( SQL_ERROR == Sql_Query(sql_handle, "SELECT `account_id`, `char_id` FROM `%s` WHERE `name` = '%s'", schema_config.char_db, esc_name) ){
		Sql_ShowDebug(sql_handle);
	}else if ( SQL_SUCCESS == Sql_NextRow(sql_handle) ){
#if PACKETVER < 20150513
		uint32 account_id = RFIFOL(fd,4);

		Sql_GetData(sql_handle, 0, &data, nullptr);
		if (atoi(data) != account_id)
		{ // Cannot send mail to char in the same account
			Sql_GetData(sql_handle, 1, &data, nullptr);
			msg.dest_id = atoi(data);
		}
#else
		// In RODEX you can even send mails to yourself
		Sql_GetData(sql_handle, 1, &data, nullptr);
		msg.dest_id = atoi(data);
#endif
	}
	Sql_FreeResult(sql_handle);
	msg.status = MAIL_NEW;

	if( msg.dest_id > 0 )
		msg.id = mail_savemessage(&msg);

	mapif_Mail_send(fd, &msg); // notify sender
	mapif_Mail_new(&msg); // notify recipient
}

bool mail_sendmail(int32 send_id, const char* send_name, int32 dest_id, const char* dest_name, const char* title, const char* body, int32 zeny, struct item *item, int32 amount)
{
	struct mail_message msg;
	memset(&msg, 0, sizeof(struct mail_message));

	msg.send_id = send_id;
	safestrncpy(msg.send_name, send_name, NAME_LENGTH);
	msg.dest_id = dest_id;
	safestrncpy(msg.dest_name, dest_name, NAME_LENGTH);
	safestrncpy(msg.title, title, MAIL_TITLE_LENGTH);
	safestrncpy(msg.body, body, MAIL_BODY_LENGTH);
	msg.zeny = zeny;
	if( item != nullptr ){
		int32 i;

		for( i = 0; i < amount && i < MAIL_MAX_ITEM; i++ ){
			memcpy(&msg.item[i], &item[i], sizeof(struct item));
		}
	}

	msg.timestamp = time(nullptr);
	msg.type = MAIL_INBOX_NORMAL;

	if( !mail_savemessage(&msg) ){
		return false;
	}

	mapif_Mail_new(&msg);
	return true;
}

void mapif_Mail_receiver_send( int32 fd, int32 requesting_char_id, int32 char_id, int32 class_, int32 base_level, const char* name ){
	WFIFOHEAD(fd,38);
	WFIFOW(fd,0) = 0x384e;
	WFIFOL(fd,2) = requesting_char_id;
	WFIFOL(fd,6) = char_id;
	WFIFOW(fd,10) = class_;
	WFIFOW(fd,12) = base_level;
	strncpy(WFIFOCP(fd, 14), name, NAME_LENGTH);
	WFIFOSET(fd,38);
}

void mapif_parse_Mail_receiver_check( int32 fd ){
	char name[NAME_LENGTH], esc_name[NAME_LENGTH * 2 + 1];
	uint32 char_id = 0;
	uint16 class_ = 0, base_level = 0;

	safestrncpy( name, RFIFOCP(fd, 6), NAME_LENGTH );

	// Try to find the Dest Char by Name
	Sql_EscapeStringLen( sql_handle, esc_name, name, strnlen( name, NAME_LENGTH ) );

	if( SQL_ERROR == Sql_Query( sql_handle, "SELECT `char_id`,`class`,`base_level` FROM `%s` WHERE `name` = '%s'", schema_config.char_db, esc_name ) ){
		Sql_ShowDebug(sql_handle);
	}else if( SQL_SUCCESS == Sql_NextRow(sql_handle) ){
		char *data;

		Sql_GetData(sql_handle, 0, &data, nullptr); char_id = atoi(data);
		Sql_GetData(sql_handle, 1, &data, nullptr); class_ = atoi(data);
		Sql_GetData(sql_handle, 2, &data, nullptr); base_level = atoi(data);
	}

	Sql_FreeResult(sql_handle);
	mapif_Mail_receiver_send( fd, RFIFOL(fd, 2), char_id, class_, base_level, name );
}

/*==========================================
 * Packets From Map Server
 *------------------------------------------*/
int32 inter_mail_parse_frommap(int32 fd)
{
	switch(RFIFOW(fd,0))
	{
		case 0x3048: mapif_parse_Mail_requestinbox(fd); break;
		case 0x3049: mapif_parse_Mail_read(fd); break;
		case 0x304a: mapif_parse_Mail_getattach(fd); break;
		case 0x304b: mapif_parse_Mail_delete(fd); break;
		case 0x304c: mapif_parse_Mail_return(fd); break;
		case 0x304d: mapif_parse_Mail_send(fd); break;
		case 0x304e: mapif_parse_Mail_receiver_check(fd); break;
		default:
			return 0;
	}
	return 1;
}

int32 inter_mail_sql_init(void)
{
	return 1;
}

void inter_mail_sql_final(void)
{
	return;
}
