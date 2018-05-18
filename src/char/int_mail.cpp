// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "int_mail.hpp"

#include <stdlib.h>
#include <string.h>

#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/sql.h"

#include "char.hpp"
#include "char_mapif.hpp"
#include "inter.hpp"

bool mail_loadmessage(int mail_id, struct mail_message* msg);
void mapif_Mail_return(int fd, uint32 char_id, int mail_id);
void mapif_Mail_delete(int fd, uint32 char_id, int mail_id, bool deleted);

int mail_fromsql(uint32 char_id, struct mail_data* md)
{
	int i;
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
		Sql_GetData(sql_handle, 0, &data, NULL); md->msg[i].id = atoi(data);
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
int mail_savemessage(struct mail_message* msg)
{
	StringBuf buf;
	SqlStmt* stmt;
	int i, j;
	bool found = false;

	// build message save query
	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "INSERT INTO `%s` (`send_name`, `send_id`, `dest_name`, `dest_id`, `title`, `message`, `time`, `status`, `zeny`,`type`", schema_config.mail_db);
	StringBuf_Printf(&buf, ") VALUES (?, '%d', ?, '%d', ?, ?, '%lu', '%d', '%d', '%d'", msg->send_id, msg->dest_id, (unsigned long)msg->timestamp, msg->status, msg->zeny, msg->type);
	StringBuf_AppendStr(&buf, ")");

	// prepare and execute query
	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_SUCCESS != SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_STRING, msg->send_name, strnlen(msg->send_name, NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, msg->dest_name, strnlen(msg->dest_name, NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 2, SQLDT_STRING, msg->title, strnlen(msg->title, MAIL_TITLE_LENGTH))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 3, SQLDT_STRING, msg->body, strnlen(msg->body, MAIL_BODY_LENGTH))
	||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		StringBuf_Destroy(&buf);
		return msg->id = 0;
	} else
		msg->id = (int)SqlStmt_LastInsertId(stmt);

	SqlStmt_Free(stmt);
	
	StringBuf_Clear(&buf);
	StringBuf_Printf(&buf,"INSERT INTO `%s` (`id`, `index`, `amount`, `nameid`, `refine`, `attribute`, `identify`, `unique_id`, `bound`", schema_config.mail_attachment_db);
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

		StringBuf_Printf(&buf, "('%" PRIu64 "', '%hu', '%d', '%hu', '%d', '%d', '%d', '%" PRIu64 "', '%d'", (uint64)msg->id, i, msg->item[i].amount, msg->item[i].nameid, msg->item[i].refine, msg->item[i].attribute, msg->item[i].identify, msg->item[i].unique_id, msg->item[i].bound);
		for (j = 0; j < MAX_SLOTS; j++)
			StringBuf_Printf(&buf, ", '%hu'", msg->item[i].card[j]);
		for (j = 0; j < MAX_ITEM_RDM_OPT; ++j) {
			StringBuf_Printf(&buf, ", '%d'", msg->item[i].option[j].id);
			StringBuf_Printf(&buf, ", '%d'", msg->item[i].option[j].value);
			StringBuf_Printf(&buf, ", '%d'", msg->item[i].option[j].param);
		}
		StringBuf_AppendStr(&buf, ")");
	}

	if( found && SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) ){
		Sql_ShowDebug(sql_handle);
	}

	StringBuf_Destroy(&buf);

	return msg->id;
}

/// Retrieves a single message from the database.
/// Returns true if the operation succeeds (or false if it fails).
bool mail_loadmessage(int mail_id, struct mail_message* msg)
{
	int i, j;
	StringBuf buf;
	char* data;

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `id`,`send_name`,`send_id`,`dest_name`,`dest_id`,`title`,`message`,`time`,`status`,`zeny`,`type` FROM `%s` WHERE `id` = '%d'", schema_config.mail_db, mail_id )
	||  SQL_SUCCESS != Sql_NextRow(sql_handle) ){
		Sql_ShowDebug(sql_handle);
		Sql_FreeResult(sql_handle);
		return false;
	}else{
		Sql_GetData(sql_handle, 0, &data, NULL); msg->id = atoi(data);
		Sql_GetData(sql_handle, 1, &data, NULL); safestrncpy(msg->send_name, data, NAME_LENGTH);
		Sql_GetData(sql_handle, 2, &data, NULL); msg->send_id = atoi(data);
		Sql_GetData(sql_handle, 3, &data, NULL); safestrncpy(msg->dest_name, data, NAME_LENGTH);
		Sql_GetData(sql_handle, 4, &data, NULL); msg->dest_id = atoi(data);
		Sql_GetData(sql_handle, 5, &data, NULL); safestrncpy(msg->title, data, MAIL_TITLE_LENGTH);
		Sql_GetData(sql_handle, 6, &data, NULL); safestrncpy(msg->body, data, MAIL_BODY_LENGTH);
		Sql_GetData(sql_handle, 7, &data, NULL); msg->timestamp = atoi(data);
		Sql_GetData(sql_handle, 8, &data, NULL); msg->status = (mail_status)atoi(data);
		Sql_GetData(sql_handle, 9, &data, NULL); msg->zeny = atoi(data);
		Sql_GetData(sql_handle,10, &data, NULL); msg->type = (mail_inbox_type)atoi(data);

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
	StringBuf_AppendStr(&buf, "SELECT `amount`,`nameid`,`refine`,`attribute`,`identify`,`unique_id`,`bound`");
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
		Sql_GetData(sql_handle,0, &data, NULL); msg->item[i].amount = (short)atoi(data);
		Sql_GetData(sql_handle,1, &data, NULL); msg->item[i].nameid = atoi(data);
		Sql_GetData(sql_handle,2, &data, NULL); msg->item[i].refine = atoi(data);
		Sql_GetData(sql_handle,3, &data, NULL); msg->item[i].attribute = atoi(data);
		Sql_GetData(sql_handle,4, &data, NULL); msg->item[i].identify = atoi(data);
		Sql_GetData(sql_handle,5, &data, NULL); msg->item[i].unique_id = strtoull(data, NULL, 10);
		Sql_GetData(sql_handle,6, &data, NULL); msg->item[i].bound = atoi(data);
		msg->item[i].expire_time = 0;

		for( j = 0; j < MAX_SLOTS; j++ ){
			Sql_GetData(sql_handle,7 + j, &data, NULL); msg->item[i].card[j] = atoi(data);
		}

		for( j = 0; j < MAX_ITEM_RDM_OPT; j++ ){
			Sql_GetData(sql_handle, 7 + MAX_SLOTS + j * 3, &data, NULL); msg->item[i].option[j].id = atoi(data);
			Sql_GetData(sql_handle, 8 + MAX_SLOTS + j * 3, &data, NULL); msg->item[i].option[j].value = atoi(data);
			Sql_GetData(sql_handle, 9 + MAX_SLOTS + j * 3, &data, NULL); msg->item[i].option[j].param = atoi(data);
		}
	}

	StringBuf_Destroy(&buf);
	Sql_FreeResult(sql_handle);

	return true;
}

int mail_timer_sub( int limit, enum mail_inbox_type type ){
	struct{
		int mail_id;
		int char_id;
		int account_id;
	}mails[MAIL_MAX_INBOX];
	int i, map_fd;
	char* data;
	struct online_char_data* character;

	if( limit <= 0 ){
		return 0;
	}

	memset(mails, 0, sizeof(mails));

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `id`,`char_id`,`account_id` FROM `%s` `m` INNER JOIN `%s` `c` ON `c`.`char_id`=`m`.`dest_id` WHERE `type` = '%d' AND `time` <= UNIX_TIMESTAMP( NOW() - INTERVAL %d DAY ) ORDER BY `id` LIMIT %d", schema_config.mail_db, schema_config.char_db, type, limit, MAIL_MAX_INBOX + 1) ){
		Sql_ShowDebug(sql_handle);
		return 0;
	}

	if( Sql_NumRows(sql_handle) <= 0 ){
		return 0;
	}

	for( i = 0; i < MAIL_MAX_INBOX && SQL_SUCCESS == Sql_NextRow(sql_handle); i++ ){
		Sql_GetData(sql_handle, 0, &data, NULL); mails[i].mail_id = atoi(data);
		Sql_GetData(sql_handle, 1, &data, NULL); mails[i].char_id = atoi(data);
		Sql_GetData(sql_handle, 2, &data, NULL); mails[i].account_id = atoi(data);
	}

	Sql_FreeResult(sql_handle);

	for( i = 0; i < MAIL_MAX_INBOX; i++ ){
		if( mails[i].mail_id == 0 ){
			break;
		}

		// Check for online players
		if( ( character = (struct online_char_data*)idb_get(char_get_onlinedb(), mails[i].account_id) ) != NULL && character->server >= 0 ){
			map_fd = map_server[character->server].fd;
		}else{
			map_fd = 0;
		}

		if( type == MAIL_INBOX_NORMAL ){
			mapif_Mail_return( 0, mails[i].char_id, mails[i].mail_id );
			mapif_Mail_delete( map_fd, mails[i].char_id, mails[i].mail_id, true );
		}else if( type == MAIL_INBOX_RETURNED ){
			mapif_Mail_delete( map_fd, mails[i].char_id, mails[i].mail_id, false );
		}else{
			// Should not happen
			continue;
		}
	}

	return 0;
}

int mail_return_timer( int tid, unsigned int tick, int id, intptr_t ptr ){
	return mail_timer_sub( charserv_config.mail_return_days, MAIL_INBOX_NORMAL );
}

int mail_delete_timer( int tid, unsigned int tick, int id, intptr_t data ){
	return mail_timer_sub( charserv_config.mail_delete_days, MAIL_INBOX_RETURNED );
}

/*==========================================
 * Client Inbox Request
 *------------------------------------------*/
void mapif_Mail_sendinbox(int fd, uint32 char_id, unsigned char flag, enum mail_inbox_type type)
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

void mapif_parse_Mail_requestinbox(int fd)
{
	mapif_Mail_sendinbox(fd, RFIFOL(fd,2), RFIFOB(fd,6), (mail_inbox_type)RFIFOB(fd,7));
}

/*==========================================
 * Mark mail as 'Read'
 *------------------------------------------*/
void mapif_parse_Mail_read(int fd)
{
	int mail_id = RFIFOL(fd,2);
	if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `status` = '%d' WHERE `id` = '%d'", schema_config.mail_db, MAIL_READ, mail_id) )
		Sql_ShowDebug(sql_handle);
}

/*==========================================
 * Client Attachment Request
 *------------------------------------------*/
bool mail_DeleteAttach(int mail_id){
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `id` = '%d'", schema_config.mail_attachment_db, mail_id ) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

void mapif_Mail_getattach(int fd, uint32 char_id, int mail_id, int type)
{
	struct mail_message msg;

	if( ( type&MAIL_ATT_ALL ) == 0 ){
		return;
	}

	if( !mail_loadmessage(mail_id, &msg) )
		return;

	if( msg.dest_id != char_id )
		return;

	if( msg.status != MAIL_READ )
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
		int i;

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

void mapif_parse_Mail_getattach(int fd)
{
	mapif_Mail_getattach(fd, RFIFOL(fd,2), RFIFOL(fd,6),RFIFOB(fd,10));
}

/*==========================================
 * Delete Mail
 *------------------------------------------*/
void mapif_Mail_delete(int fd, uint32 char_id, int mail_id, bool deleted)
{
	bool failed = false;

	if( !deleted ){
		if ( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `id` = '%d'", schema_config.mail_db, mail_id) ||
			SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `id` = '%d'", schema_config.mail_attachment_db, mail_id) )
		{
			Sql_ShowDebug(sql_handle);
			failed = true;
		}
	}

	// Only if the request came from a map-server and was not timer triggered for an offline character
	if( fd <= 0 ){
		return;
	}

	WFIFOHEAD(fd,11);
	WFIFOW(fd,0) = 0x384b;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = mail_id;
	WFIFOB(fd,10) = failed;
	WFIFOSET(fd,11);
}

void mapif_parse_Mail_delete(int fd)
{
	mapif_Mail_delete(fd, RFIFOL(fd,2), RFIFOL(fd,6), false);
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
void mapif_Mail_return(int fd, uint32 char_id, int mail_id)
{
	struct mail_message msg;
	int new_mail = 0;

	if( mail_loadmessage(mail_id, &msg) )
	{
		if( msg.dest_id != char_id)
			return;
		else if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `id` = '%d'", schema_config.mail_db, mail_id)
			|| SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `id` = '%d'", schema_config.mail_attachment_db, mail_id) )
			Sql_ShowDebug(sql_handle);
		// If it was not sent by the server, since we do not want to return mails to the server
		else if( msg.send_id != 0 )
		{
			char temp_[MAIL_TITLE_LENGTH];

			// swap sender and receiver
			SWAP(msg.send_id, msg.dest_id);
			safestrncpy(temp_, msg.send_name, NAME_LENGTH);
			safestrncpy(msg.send_name, msg.dest_name, NAME_LENGTH);
			safestrncpy(msg.dest_name, temp_, NAME_LENGTH);

			// set reply message title
			snprintf(temp_, MAIL_TITLE_LENGTH, "RE:%s", msg.title);
			safestrncpy(msg.title, temp_, MAIL_TITLE_LENGTH);

			msg.status = MAIL_NEW;
			msg.type = MAIL_INBOX_RETURNED;
			msg.timestamp = time(NULL);

			new_mail = mail_savemessage(&msg);
			mapif_Mail_new(&msg);
		}
	}

	// Only if the request came from a map-server and was not timer triggered for an offline character
	if( fd <= 0 ){
		return;
	}

	WFIFOHEAD(fd,11);
	WFIFOW(fd,0) = 0x384c;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = mail_id;
	WFIFOB(fd,10) = (new_mail == 0);
	WFIFOSET(fd,11);
}

void mapif_parse_Mail_return(int fd)
{
	mapif_Mail_return(fd, RFIFOL(fd,2), RFIFOL(fd,6));
}

/*==========================================
 * Send Mail
 *------------------------------------------*/
void mapif_Mail_send(int fd, struct mail_message* msg)
{
	int len = sizeof(struct mail_message) + 4;

	WFIFOHEAD(fd,len);
	WFIFOW(fd,0) = 0x384d;
	WFIFOW(fd,2) = len;
	memcpy(WFIFOP(fd,4), msg, sizeof(struct mail_message));
	WFIFOSET(fd,len);
}

void mapif_parse_Mail_send(int fd)
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
			Sql_GetData(sql_handle, 0, &data, NULL);
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

		Sql_GetData(sql_handle, 0, &data, NULL);
		if (atoi(data) != account_id)
		{ // Cannot send mail to char in the same account
			Sql_GetData(sql_handle, 1, &data, NULL);
			msg.dest_id = atoi(data);
		}
#else
		// In RODEX you can even send mails to yourself
		Sql_GetData(sql_handle, 1, &data, NULL);
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

bool mail_sendmail(int send_id, const char* send_name, int dest_id, const char* dest_name, const char* title, const char* body, int zeny, struct item *item, int amount)
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
	if( item != NULL ){
		int i;

		for( i = 0; i < amount && i < MAIL_MAX_ITEM; i++ ){
			memcpy(&msg.item[i], &item[i], sizeof(struct item));
		}
	}

	msg.timestamp = time(NULL);
	msg.type = MAIL_INBOX_NORMAL;

	if( !mail_savemessage(&msg) ){
		return false;
	}

	mapif_Mail_new(&msg);
	return true;
}

void mapif_Mail_receiver_send( int fd, int requesting_char_id, int char_id, int class_, int base_level, const char* name ){
	WFIFOHEAD(fd,38);
	WFIFOW(fd,0) = 0x384e;
	WFIFOL(fd,2) = requesting_char_id;
	WFIFOL(fd,6) = char_id;
	WFIFOW(fd,10) = class_;
	WFIFOW(fd,12) = base_level;
	strncpy(WFIFOCP(fd, 14), name, NAME_LENGTH);
	WFIFOSET(fd,38);
}

void mapif_parse_Mail_receiver_check( int fd ){
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

		Sql_GetData(sql_handle, 0, &data, NULL); char_id = atoi(data);
		Sql_GetData(sql_handle, 1, &data, NULL); class_ = atoi(data);
		Sql_GetData(sql_handle, 2, &data, NULL); base_level = atoi(data);
	}

	Sql_FreeResult(sql_handle);
	mapif_Mail_receiver_send( fd, RFIFOL(fd, 2), char_id, class_, base_level, name );
}

/*==========================================
 * Packets From Map Server
 *------------------------------------------*/
int inter_mail_parse_frommap(int fd)
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

int inter_mail_sql_init(void)
{
	return 1;
}

void inter_mail_sql_final(void)
{
	return;
}
