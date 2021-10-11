// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "int_auction.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/malloc.hpp"
#include "../common/mmo.hpp"
#include "../common/showmsg.hpp"
#include "../common/socket.hpp"
#include "../common/sql.hpp"
#include "../common/strlib.hpp"
#include "../common/timer.hpp"

#include "char.hpp"
#include "char_mapif.hpp"
#include "inter.hpp"
#include "int_mail.hpp"

static DBMap* auction_db_ = NULL; // int auction_id -> struct auction_data*

void auction_delete(struct auction_data *auction);
TIMER_FUNC(auction_end_timer);

int auction_count(uint32 char_id, bool buy)
{
	int i = 0;
	struct auction_data *auction;
	DBIterator *iter = db_iterator(auction_db_);

	for( auction = (struct auction_data *)dbi_first(iter); dbi_exists(iter); auction = (struct auction_data *)dbi_next(iter) )
	{
		if( (buy && auction->buyer_id == char_id) || (!buy && auction->seller_id == char_id) )
			i++;
	}
	dbi_destroy(iter);

	return i;
}

void auction_save(struct auction_data *auction)
{
	int j;
	StringBuf buf;
	SqlStmt* stmt;

	if( !auction )
		return;

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "UPDATE `%s` SET `seller_id` = '%d', `seller_name` = ?, `buyer_id` = '%d', `buyer_name` = ?, `price` = '%d', `buynow` = '%d', `hours` = '%d', `timestamp` = '%lu', `nameid` = '%u', `item_name` = ?, `type` = '%d', `refine` = '%d', `attribute` = '%d', `enchantgrade` ='%d'",
		schema_config.auction_db, auction->seller_id, auction->buyer_id, auction->price, auction->buynow, auction->hours, (unsigned long)auction->timestamp, auction->item.nameid, auction->type, auction->item.refine, auction->item.attribute, auction->item.enchantgrade);
	for( j = 0; j < MAX_SLOTS; j++ )
		StringBuf_Printf(&buf, ", `card%d` = '%u'", j, auction->item.card[j]);
	for (j = 0; j < MAX_ITEM_RDM_OPT; j++) {
		StringBuf_Printf(&buf, ", `option_id%d` = '%d'", j, auction->item.option[j].id);
		StringBuf_Printf(&buf, ", `option_val%d` = '%d'", j, auction->item.option[j].value);
		StringBuf_Printf(&buf, ", `option_parm%d` = '%d'", j, auction->item.option[j].param);
	}
	StringBuf_Printf(&buf, " WHERE `auction_id` = '%d'", auction->auction_id);

	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_SUCCESS != SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_STRING, auction->seller_name, strnlen(auction->seller_name, NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, auction->buyer_name, strnlen(auction->buyer_name, NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 2, SQLDT_STRING, auction->item_name, strnlen(auction->item_name, ITEM_NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
	}

	SqlStmt_Free(stmt);
	StringBuf_Destroy(&buf);
}

unsigned int auction_create(struct auction_data *auction)
{
	int j;
	StringBuf buf;
	SqlStmt* stmt;

	if( !auction )
		return false;

	auction->timestamp = time(NULL) + (auction->hours * 3600);

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "INSERT INTO `%s` (`seller_id`,`seller_name`,`buyer_id`,`buyer_name`,`price`,`buynow`,`hours`,`timestamp`,`nameid`,`item_name`,`type`,`refine`,`attribute`,`unique_id`,`enchantgrade`", schema_config.auction_db);
	for( j = 0; j < MAX_SLOTS; j++ )
		StringBuf_Printf(&buf, ",`card%d`", j);
	for (j = 0; j < MAX_ITEM_RDM_OPT; ++j) {
		StringBuf_Printf(&buf, ", `option_id%d`", j);
		StringBuf_Printf(&buf, ", `option_val%d`", j);
		StringBuf_Printf(&buf, ", `option_parm%d`", j);
	}
	StringBuf_Printf(&buf, ") VALUES ('%d',?,'%d',?,'%d','%d','%d','%lu','%u',?,'%d','%d','%d','%" PRIu64 "','%d'",
		auction->seller_id, auction->buyer_id, auction->price, auction->buynow, auction->hours, (unsigned long)auction->timestamp, auction->item.nameid, auction->type, auction->item.refine, auction->item.attribute, auction->item.unique_id, auction->item.enchantgrade);
	for( j = 0; j < MAX_SLOTS; j++ )	
		StringBuf_Printf(&buf, ",'%u'", auction->item.card[j]);
	for (j = 0; j < MAX_ITEM_RDM_OPT; ++j) {
		StringBuf_Printf(&buf, ", '%d'", auction->item.option[j].id);
		StringBuf_Printf(&buf, ", '%d'", auction->item.option[j].value);
		StringBuf_Printf(&buf, ", '%d'", auction->item.option[j].param);
	}
	StringBuf_AppendStr(&buf, ")");

	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_SUCCESS != SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_STRING, auction->seller_name, strnlen(auction->seller_name, NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, auction->buyer_name, strnlen(auction->buyer_name, NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 2, SQLDT_STRING, auction->item_name, strnlen(auction->item_name, ITEM_NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		auction->auction_id = 0;
	}
	else
	{
		struct auction_data *auction_;
		t_tick tick = auction->hours * 3600000;

		auction->item.amount = 1;
		auction->item.identify = 1;
		auction->item.expire_time = 0;

		auction->auction_id = (unsigned int)SqlStmt_LastInsertId(stmt);
		auction->auction_end_timer = add_timer( gettick() + tick , auction_end_timer, auction->auction_id, 0);
		ShowInfo("New Auction %u | time left %" PRtf " ms | By %s.\n", auction->auction_id, tick, auction->seller_name);

		CREATE(auction_, struct auction_data, 1);
		memcpy(auction_, auction, sizeof(struct auction_data));
		idb_put(auction_db_, auction_->auction_id, auction_);
	}

	SqlStmt_Free(stmt);
	StringBuf_Destroy(&buf);

	return auction->auction_id;
}

void mapif_Auction_message(uint32 char_id, unsigned char result)
{
	unsigned char buf[74];

	WBUFW(buf,0) = 0x3854;
	WBUFL(buf,2) = char_id;
	WBUFL(buf,6) = result;
	chmapif_sendall(buf,7);
}

TIMER_FUNC(auction_end_timer){
	struct auction_data *auction;
	if( (auction = (struct auction_data *)idb_get(auction_db_, id)) != NULL )
	{
		if( auction->buyer_id )
		{
			mail_sendmail(0, msg_txt(200), auction->buyer_id, auction->buyer_name, msg_txt(201), msg_txt(202), 0, &auction->item, 1);
			mapif_Auction_message(auction->buyer_id, 6); // You have won the auction
			mail_sendmail(0, msg_txt(200), auction->seller_id, auction->seller_name, msg_txt(201), msg_txt(203), auction->price, NULL, 0);
		}
		else
			mail_sendmail(0, msg_txt(200), auction->seller_id, auction->seller_name, msg_txt(201), msg_txt(204), 0, &auction->item, 1);

		ShowInfo("Auction End: id %u.\n", auction->auction_id);

		auction->auction_end_timer = INVALID_TIMER;
		auction_delete(auction);
	}

	return 0;
}

void auction_delete(struct auction_data *auction)
{
	unsigned int auction_id = auction->auction_id;

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `auction_id` = '%d'", schema_config.auction_db, auction_id) )
		Sql_ShowDebug(sql_handle);

	if( auction->auction_end_timer != INVALID_TIMER )
		delete_timer(auction->auction_end_timer, auction_end_timer);

	idb_remove(auction_db_, auction_id);
}

void inter_auctions_fromsql(void)
{
	int i;
	char *data;
	StringBuf buf;
	t_tick tick = gettick(), endtick;
	time_t now = time(NULL);

	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `auction_id`,`seller_id`,`seller_name`,`buyer_id`,`buyer_name`,"
		"`price`,`buynow`,`hours`,`timestamp`,`nameid`,`item_name`,`type`,`refine`,`attribute`,`unique_id`,`enchantgrade`");
	for( i = 0; i < MAX_SLOTS; i++ )
		StringBuf_Printf(&buf, ",`card%d`", i);
	for (i = 0; i < MAX_ITEM_RDM_OPT; ++i) {
		StringBuf_Printf(&buf, ", `option_id%d`", i);
		StringBuf_Printf(&buf, ", `option_val%d`", i);
		StringBuf_Printf(&buf, ", `option_parm%d`", i);
	}
	StringBuf_Printf(&buf, " FROM `%s` ORDER BY `auction_id` DESC", schema_config.auction_db);

	if( SQL_ERROR == Sql_Query(sql_handle, StringBuf_Value(&buf)) )
		Sql_ShowDebug(sql_handle);

	StringBuf_Destroy(&buf);

	while( SQL_SUCCESS == Sql_NextRow(sql_handle) )
	{
		struct item *item;
		struct auction_data *auction;
		CREATE(auction, struct auction_data, 1);
		Sql_GetData(sql_handle, 0, &data, NULL); auction->auction_id = atoi(data);
		Sql_GetData(sql_handle, 1, &data, NULL); auction->seller_id = atoi(data);
		Sql_GetData(sql_handle, 2, &data, NULL); safestrncpy(auction->seller_name, data, NAME_LENGTH);
		Sql_GetData(sql_handle, 3, &data, NULL); auction->buyer_id = atoi(data);
		Sql_GetData(sql_handle, 4, &data, NULL); safestrncpy(auction->buyer_name, data, NAME_LENGTH);
		Sql_GetData(sql_handle, 5, &data, NULL); auction->price	= atoi(data);
		Sql_GetData(sql_handle, 6, &data, NULL); auction->buynow = atoi(data);
		Sql_GetData(sql_handle, 7, &data, NULL); auction->hours = atoi(data);
		Sql_GetData(sql_handle, 8, &data, NULL); auction->timestamp = atoi(data);

		item = &auction->item;
		Sql_GetData(sql_handle, 9, &data, NULL); item->nameid = strtoul(data, nullptr, 10);
		Sql_GetData(sql_handle,10, &data, NULL); safestrncpy(auction->item_name, data, ITEM_NAME_LENGTH);
		Sql_GetData(sql_handle,11, &data, NULL); auction->type = atoi(data);

		Sql_GetData(sql_handle,12, &data, NULL); item->refine = atoi(data);
		Sql_GetData(sql_handle,13, &data, NULL); item->attribute = atoi(data);
		Sql_GetData(sql_handle,14, &data, NULL); item->unique_id = strtoull(data, NULL, 10);
		Sql_GetData(sql_handle,15, &data, NULL); item->enchantgrade = atoi(data);

		item->identify = 1;
		item->amount = 1;
		item->expire_time = 0;

		for( i = 0; i < MAX_SLOTS; i++ )
		{
			Sql_GetData(sql_handle, 16 + i, &data, NULL);
			item->card[i] = strtoul(data, nullptr, 10);
		}

		for (i = 0; i < MAX_ITEM_RDM_OPT; i++) {
			Sql_GetData(sql_handle, 16 + MAX_SLOTS + i*3, &data, NULL);
			item->option[i].id = atoi(data);
			Sql_GetData(sql_handle, 17 + MAX_SLOTS + i*3, &data, NULL);
			item->option[i].value = atoi(data);
			Sql_GetData(sql_handle, 18 + MAX_SLOTS + i*3, &data, NULL);
			item->option[i].param = atoi(data);
		}

		if( auction->timestamp > now )
			endtick = ((unsigned int)(auction->timestamp - now) * 1000) + tick;
		else
			endtick = tick + 10000; // 10 Second's to process ended auctions

		auction->auction_end_timer = add_timer(endtick, auction_end_timer, auction->auction_id, 0);
		idb_put(auction_db_, auction->auction_id, auction);
	}

	Sql_FreeResult(sql_handle);
}

void mapif_Auction_sendlist(int fd, uint32 char_id, short count, short pages, unsigned char *buf)
{
	int len = (sizeof(struct auction_data) * count) + 12;

	WFIFOHEAD(fd, len);
	WFIFOW(fd,0) = 0x3850;
	WFIFOW(fd,2) = len;
	WFIFOL(fd,4) = char_id;
	WFIFOW(fd,8) = count;
	WFIFOW(fd,10) = pages;
	memcpy(WFIFOP(fd,12), buf, len - 12);
	WFIFOSET(fd,len);
}

void mapif_parse_Auction_requestlist(int fd)
{
	char searchtext[NAME_LENGTH];
	uint32 char_id = RFIFOL(fd,4), len = sizeof(struct auction_data);
	int price = RFIFOL(fd,10);
	short type = RFIFOW(fd,8), page = max(1,RFIFOW(fd,14));
	unsigned char buf[5 * sizeof(struct auction_data)];
	DBIterator *iter = db_iterator(auction_db_);
	struct auction_data *auction;
	short i = 0, j = 0, pages = 1;

	memcpy(searchtext, RFIFOP(fd,16), NAME_LENGTH);

	for( auction = static_cast<auction_data *>(dbi_first(iter)); dbi_exists(iter); auction = static_cast<auction_data *>(dbi_next(iter)) )
	{
		if( (type == 0 && auction->type != IT_ARMOR && auction->type != IT_PETARMOR) ||
			(type == 1 && auction->type != IT_WEAPON) ||
			(type == 2 && auction->type != IT_CARD) ||
			(type == 3 && auction->type != IT_ETC) ||
			(type == 4 && !strstr(auction->item_name, searchtext)) ||
			(type == 5 && auction->price > price) ||
			(type == 6 && auction->seller_id != char_id) ||
			(type == 7 && auction->buyer_id != char_id) )
			continue;

		i++;
		if( i > 5 )
		{ // Counting Pages of Total Results (5 Results per Page)
			pages++;
			i = 1; // First Result of This Page
		}

		if( page != pages )
			continue; // This is not the requested Page

		memcpy(WBUFP(buf, j * len), auction, len);
		j++; // Found Results
	}
	dbi_destroy(iter);

	mapif_Auction_sendlist(fd, char_id, j, pages, buf);
}

void mapif_Auction_register(int fd, struct auction_data *auction)
{
	int len = sizeof(struct auction_data) + 4;

	WFIFOHEAD(fd,len);
	WFIFOW(fd,0) = 0x3851;
	WFIFOW(fd,2) = len;
	memcpy(WFIFOP(fd,4), auction, sizeof(struct auction_data));
	WFIFOSET(fd,len);
}

void mapif_parse_Auction_register(int fd)
{
	struct auction_data auction;
	if( RFIFOW(fd,2) != sizeof(struct auction_data) + 4 )
		return;

	memcpy(&auction, RFIFOP(fd,4), sizeof(struct auction_data));
	if( auction_count(auction.seller_id, false) < 5 )
		auction.auction_id = auction_create(&auction);

	mapif_Auction_register(fd, &auction);
}

void mapif_Auction_cancel(int fd, uint32 char_id, unsigned char result)
{
	WFIFOHEAD(fd,7);
	WFIFOW(fd,0) = 0x3852;
	WFIFOL(fd,2) = char_id;
	WFIFOB(fd,6) = result;
	WFIFOSET(fd,7);
}

void mapif_parse_Auction_cancel(int fd)
{
	uint32 char_id = RFIFOL(fd,2), auction_id = RFIFOL(fd,6);
	struct auction_data *auction;

	if( (auction = (struct auction_data *)idb_get(auction_db_, auction_id)) == NULL )
	{
		mapif_Auction_cancel(fd, char_id, 1); // Bid Number is Incorrect
		return;
	}

	if( auction->seller_id != char_id )
	{
		mapif_Auction_cancel(fd, char_id, 2); // You cannot end the auction
		return;
	}

	if( auction->buyer_id > 0 )
	{
		mapif_Auction_cancel(fd, char_id, 3); // An auction with at least one bidder cannot be canceled
		return;
	}

	mail_sendmail(0, msg_txt(200), auction->seller_id, auction->seller_name, msg_txt(201), msg_txt(205), 0, &auction->item, 1);
	auction_delete(auction);

	mapif_Auction_cancel(fd, char_id, 0); // The auction has been canceled
}

void mapif_Auction_close(int fd, uint32 char_id, unsigned char result)
{
	WFIFOHEAD(fd,7);
	WFIFOW(fd,0) = 0x3853;
	WFIFOL(fd,2) = char_id;
	WFIFOB(fd,6) = result;
	WFIFOSET(fd,7);
}

void mapif_parse_Auction_close(int fd)
{
	uint32 char_id = RFIFOL(fd,2), auction_id = RFIFOL(fd,6);
	struct auction_data *auction;

	if( (auction = (struct auction_data *)idb_get(auction_db_, auction_id)) == NULL )
	{
		mapif_Auction_close(fd, char_id, 2); // Bid Number is Incorrect
		return;
	}

	if( auction->seller_id != char_id )
	{
		mapif_Auction_close(fd, char_id, 1); // You cannot end the auction
		return;
	}

	if( auction->buyer_id == 0 )
	{
		mapif_Auction_close(fd, char_id, 1); // You cannot end the auction
		return;
	}

	// Send Money to Seller
	mail_sendmail(0, msg_txt(200), auction->seller_id, auction->seller_name, msg_txt(201), msg_txt(206), auction->price, NULL, 0);
	// Send Item to Buyer
	mail_sendmail(0, msg_txt(200), auction->buyer_id, auction->buyer_name, msg_txt(201), msg_txt(207), 0, &auction->item, 1);
	mapif_Auction_message(auction->buyer_id, 6); // You have won the auction
	auction_delete(auction);

	mapif_Auction_close(fd, char_id, 0); // You have ended the auction
}

void mapif_Auction_bid(int fd, uint32 char_id, int bid, unsigned char result)
{
	WFIFOHEAD(fd,11);
	WFIFOW(fd,0) = 0x3855;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = bid; // To Return Zeny
	WFIFOB(fd,10) = result;
	WFIFOSET(fd,11);
}

void mapif_parse_Auction_bid(int fd)
{
	uint32 char_id = RFIFOL(fd,4), auction_id = RFIFOL(fd,8);
	int bid = RFIFOL(fd,12);
	struct auction_data *auction;

	if( (auction = (struct auction_data *)idb_get(auction_db_, auction_id)) == NULL || auction->price >= bid || auction->seller_id == char_id )
	{
		mapif_Auction_bid(fd, char_id, bid, 0); // You have failed to bid in the auction
		return;
	}

	if( auction_count(char_id, true) > 4 && bid < auction->buynow && auction->buyer_id != char_id )
	{
		mapif_Auction_bid(fd, char_id, bid, 9); // You cannot place more than 5 bids at a time
		return;
	}

	if( auction->buyer_id > 0 )
	{ // Send Money back to the previous Buyer
		if( auction->buyer_id != char_id )
		{
			mail_sendmail(0, msg_txt(200), auction->buyer_id, auction->buyer_name, msg_txt(201), msg_txt(208), auction->price, NULL, 0);
			mapif_Auction_message(auction->buyer_id, 7); // You have failed to win the auction
		}
		else
			mail_sendmail(0, msg_txt(200), auction->buyer_id, auction->buyer_name, msg_txt(201), msg_txt(209), auction->price, NULL, 0);
	}

	auction->buyer_id = char_id;
	safestrncpy(auction->buyer_name, RFIFOCP(fd,16), NAME_LENGTH);
	auction->price = bid;

	if( bid >= auction->buynow )
	{ // Automatic won the auction
		mapif_Auction_bid(fd, char_id, bid - auction->buynow, 1); // You have successfully bid in the auction

		mail_sendmail(0, msg_txt(200), auction->buyer_id, auction->buyer_name, msg_txt(201), msg_txt(210), 0, &auction->item, 1);
		mapif_Auction_message(char_id, 6); // You have won the auction
		mail_sendmail(0, msg_txt(200), auction->seller_id, auction->seller_name, msg_txt(201), msg_txt(211), auction->buynow, NULL, 0);

		auction_delete(auction);
		return;
	}

	auction_save(auction);

	mapif_Auction_bid(fd, char_id, 0, 1); // You have successfully bid in the auction
}

/*==========================================
 * Packets From Map Server
 *------------------------------------------*/
int inter_auction_parse_frommap(int fd)
{
	switch(RFIFOW(fd,0))
	{
		case 0x3050: mapif_parse_Auction_requestlist(fd); break;
		case 0x3051: mapif_parse_Auction_register(fd); break;
		case 0x3052: mapif_parse_Auction_cancel(fd); break;
		case 0x3053: mapif_parse_Auction_close(fd); break;
		case 0x3055: mapif_parse_Auction_bid(fd); break;
		default:
			return 0;
	}
	return 1;
}

int inter_auction_sql_init(void)
{
	auction_db_ = idb_alloc(DB_OPT_RELEASE_DATA);
	inter_auctions_fromsql();

	return 0;
}

void inter_auction_sql_final(void)
{
	auction_db_->destroy(auction_db_,NULL);

	return;
}
