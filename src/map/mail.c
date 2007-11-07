// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef TXT_ONLY

#include "../common/nullpo.h"

#include "mail.h"
#include "itemdb.h"
#include "clif.h"
#include "pc.h"

#include <time.h>
#include <string.h>

time_t mail_calctimes(void)
{
	time_t temp = time(NULL);
	return mktime(localtime(&temp));
}

int mail_removeitem(struct map_session_data *sd, short flag)
{
	nullpo_retr(0,sd);

	if( sd->mail.amount )
	{
		if (flag)
			pc_delitem(sd, sd->mail.index, sd->mail.amount, 1);
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
	nullpo_retr(0,sd);

	if (flag && sd->mail.zeny > 0)
		sd->status.zeny -= sd->mail.zeny;

	sd->mail.zeny = 0;
	clif_updatestatus(sd, SP_ZENY);

	return 1;
}

unsigned char mail_setitem(struct map_session_data *sd, int idx, int amount)
{
	if (idx == 0)
	{ // Zeny Transfer
		if (amount < 0)
			return 0;
		if (amount > sd->status.zeny)
			amount = sd->status.zeny;

		sd->mail.zeny = amount;
		clif_updatestatus(sd, SP_ZENY);
		return 0;
	}
	else
	{ // Item Transfer
		idx -= 2;
		mail_removeitem(sd, 0);

		if( idx < 0 || idx > MAX_INVENTORY )
			return 1;
		if( amount < 0 || amount > sd->status.inventory[idx].amount )
			return 1;
		if( !itemdb_isdropable(&sd->status.inventory[idx], pc_isGM(sd)) )
			return 1;

		sd->mail.index = idx;
		sd->mail.nameid = sd->status.inventory[idx].nameid;
		sd->mail.amount = amount;
		
		return 0;
	}
}

bool mail_getattach(struct map_session_data *sd, struct mail_message *msg)
{
	int n;
	
	nullpo_retr(0,sd);
	nullpo_retr(0,msg);

	if( sd->mail.zeny < 0 || sd->mail.zeny > sd->status.zeny )
		return false;

	n = sd->mail.index;
	if( sd->mail.amount )
	{
		if( sd->status.inventory[n].nameid != sd->mail.nameid )
			return false;

		if( sd->status.inventory[n].amount < sd->mail.amount )
			return false;

		memcpy(&msg->item, &sd->status.inventory[n], sizeof(struct item));
		msg->item.amount = sd->mail.amount;
	}
	else
		memset(&msg->item, 0x00, sizeof(struct item));

	msg->zeny = sd->mail.zeny;

	return true;
}

bool mail_checkattach(struct map_session_data *sd)
{
	nullpo_retr(false,sd);

	if( sd->mail.zeny > 0 && sd->status.zeny < sd->status.zeny )
		return false;

	if( sd->mail.amount > 0 )
	{
		if( sd->status.inventory[sd->mail.index].nameid != sd->mail.nameid )
			return false;

		if( sd->status.inventory[sd->mail.index].amount < sd->mail.amount )
			return false;
	}

	mail_removeitem(sd,1);
	mail_removezeny(sd,1);

	return true;
}

int mail_openmail(struct map_session_data *sd)
{
	nullpo_retr(0,sd);

	if( sd->state.finalsave == 1 || sd->state.storage_flag || sd->vender_id || sd->state.trading )
		return 0;

	clif_Mail_openmail(sd->fd);	

	return 0;
}

#endif
