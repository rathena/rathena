// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/nullpo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"

#include "mail.h"
#include "atcommand.h"
#include "itemdb.h"
#include "clif.h"
#include "pc.h"
#include "intif.h"

void mail_clear(struct map_session_data *sd)
{
	sd->mail.nameid = 0;
	sd->mail.index = 0;
	sd->mail.amount = 0;
	sd->mail.zeny = 0;

	return;
}

int mail_removeitem(struct map_session_data *sd, short flag)
{
	nullpo_ret(sd);

	if( sd->mail.amount )
	{
		if (flag) // Item send
			pc_delitem(sd, sd->mail.index, sd->mail.amount, 1, 0, LOG_TYPE_MAIL);
		else
			clif_additem(sd, sd->mail.index, sd->mail.amount, 0);
	}

	sd->mail.nameid = 0;
	sd->mail.index = 0;
	sd->mail.amount = 0;
	return 1;
}

int mail_removezeny(struct map_session_data *sd, short flag)
{
	nullpo_ret(sd);

	if (flag && sd->mail.zeny > 0)
	{  //Zeny send
		pc_payzeny(sd,sd->mail.zeny,LOG_TYPE_MAIL, NULL);
	}
	if (sd->mail.zeny > 0)
		clif_updatestatus(sd, SP_ZENY);
	sd->mail.zeny = 0;

	return 1;
}

/**
* Attempt to set item or zeny
* @param sd
* @param idx 0 - Zeny; >= 2 - Inventory item
* @param amount
* @return True if item/zeny can be set, False if failed
*/
bool mail_setitem(struct map_session_data *sd, short idx, int amount) {

	if( pc_istrading(sd) )
		return false;

	if( idx == 0 ) { // Zeny Transfer
		if( !pc_can_give_items(sd) )
			return false;

		if( amount > sd->status.zeny )
			amount = sd->status.zeny;

		sd->mail.zeny = amount;
		// clif_updatestatus(sd, SP_ZENY);
		return true;
	} else { // Item Transfer
		idx -= 2;
		mail_removeitem(sd, 0);

		if( idx < 0 || idx >= MAX_INVENTORY )
			return false;
		if( amount > sd->status.inventory[idx].amount )
			return false;
		if( !pc_can_give_items(sd) || sd->status.inventory[idx].expire_time
			|| !itemdb_available(sd->status.inventory[idx].nameid)
			|| !itemdb_canmail(&sd->status.inventory[idx],pc_get_group_level(sd))
			|| (sd->status.inventory[idx].bound && !pc_can_give_bounded_items(sd)) )
			return false;

		sd->mail.index = idx;
		sd->mail.nameid = sd->status.inventory[idx].nameid;
		sd->mail.amount = amount;
		return true;
	}
}

bool mail_setattachment(struct map_session_data *sd, struct mail_message *msg)
{
	int n;

	nullpo_retr(false,sd);
	nullpo_retr(false,msg);

	if( sd->mail.zeny < 0 || sd->mail.zeny > sd->status.zeny )
		return false;

	n = sd->mail.index;
	if( sd->mail.amount )
	{
		if( sd->status.inventory[n].nameid != sd->mail.nameid )
			return false;

		if( sd->status.inventory[n].amount < sd->mail.amount )
			return false;

		if( sd->weight > sd->max_weight )
			return false;

		memcpy(&msg->item, &sd->status.inventory[n], sizeof(struct item));
		msg->item.amount = sd->mail.amount;
	}
	else
		memset(&msg->item, 0x00, sizeof(struct item));

	msg->zeny = sd->mail.zeny;

	// Removes the attachment from sender
	mail_removeitem(sd,1);
	mail_removezeny(sd,1);

	return true;
}

void mail_getattachment(struct map_session_data* sd, int zeny, struct item* item)
{
	if( item->nameid > 0 && item->amount > 0 )
	{
		pc_additem(sd, item, item->amount, LOG_TYPE_MAIL);
		clif_Mail_getattachment(sd->fd, 0);
	}

	if( zeny > 0 )
	{  //Zeny receive
		pc_getzeny(sd, zeny,LOG_TYPE_MAIL, NULL);
	}
}

int mail_openmail(struct map_session_data *sd)
{
	nullpo_ret(sd);

	if( sd->state.storage_flag || sd->state.vending || sd->state.buyingstore || sd->state.trading )
		return 0;

	clif_Mail_window(sd->fd, 0);

	return 1;
}

void mail_deliveryfail(struct map_session_data *sd, struct mail_message *msg)
{
	nullpo_retv(sd);
	nullpo_retv(msg);

	if( msg->item.amount > 0 )
	{
		// Item receive (due to failure)
		pc_additem(sd, &msg->item, msg->item.amount, LOG_TYPE_MAIL);
	}

	if( msg->zeny > 0 )
	{
		pc_getzeny(sd,msg->zeny,LOG_TYPE_MAIL, NULL); //Zeny receive (due to failure)
	}

	clif_Mail_send(sd->fd, true);
}

// This function only check if the mail operations are valid
bool mail_invalid_operation(struct map_session_data *sd)
{
	if( !map[sd->bl.m].flag.town && !pc_can_use_command(sd, "mail", COMMAND_ATCOMMAND) )
	{
		ShowWarning("clif_parse_Mail: char '%s' trying to do invalid mail operations.\n", sd->status.name);
		return true;
	}

	return false;
}

/**
* Attempt to send mail
* @param sd Sender
* @param dest_name Destination name
* @param title Mail title
* @param body_msg Mail message
* @param body_len Message's length
*/
void mail_send(struct map_session_data *sd, const char *dest_name, const char *title, const char *body_msg, int body_len) {
	struct mail_message msg;

	nullpo_retv(sd);

	if( sd->state.trading )
		return;

	if( DIFF_TICK(sd->cansendmail_tick, gettick()) > 0 ) {
		clif_displaymessage(sd->fd,msg_txt(sd,675)); //"Cannot send mails too fast!!."
		clif_Mail_send(sd->fd, true); // fail
		return;
	}

	if( body_len > MAIL_BODY_LENGTH )
		body_len = MAIL_BODY_LENGTH;

	if( !mail_setattachment(sd, &msg) ) { // Invalid Append condition
		clif_Mail_send(sd->fd, true); // fail
		mail_removeitem(sd,0);
		mail_removezeny(sd,0);
		return;
	}

	msg.id = 0; // id will be assigned by charserver
	msg.send_id = sd->status.char_id;
	msg.dest_id = 0; // will attempt to resolve name
	safestrncpy(msg.send_name, sd->status.name, NAME_LENGTH);
	safestrncpy(msg.dest_name, (char*)dest_name, NAME_LENGTH);
	safestrncpy(msg.title, (char*)title, MAIL_TITLE_LENGTH);

	if (msg.title[0] == '\0') {
		return; // Message has no length and somehow client verification was skipped.
	}

	if (body_len)
		safestrncpy(msg.body, (char*)body_msg, body_len + 1);
	else
		memset(msg.body, 0x00, MAIL_BODY_LENGTH);

	msg.timestamp = time(NULL);
	if( !intif_Mail_send(sd->status.account_id, &msg) )
		mail_deliveryfail(sd, &msg);

	sd->cansendmail_tick = gettick() + battle_config.mail_delay; // Flood Protection
}
