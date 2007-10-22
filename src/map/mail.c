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

char mail_setitem(struct map_session_data *sd, int idx, int amount)
{
	nullpo_retr(-1,sd);

	if (idx == 0)
	{ // Zeny Transfer
		if (amount < 0)
			return 2;
		if (amount > sd->status.zeny)
			amount = sd->status.zeny;

		sd->mail.zeny = amount;
		clif_updatestatus(sd, SP_ZENY);
	}
	else
	{ // Item Transfer
		idx -= 2;
		mail_removeitem(sd, 0);

		if( idx < 0 || idx > MAX_INVENTORY )
			return 2;
		if( amount < 0 || amount > sd->status.inventory[idx].amount )
			return 2;
		if( itemdb_isdropable(&sd->status.inventory[idx], pc_isGM(sd)) == 0 )
			return 2;

		sd->mail.index = idx;
		sd->mail.amount = amount;

		clif_delitem(sd, idx, amount);

		return 0;
	}

	return -1;
}

int mail_getattach(struct map_session_data *sd, struct mail_message *msg)
{
	int n;
	
	nullpo_retr(0,sd);
	nullpo_retr(0,msg);

	if( sd->mail.zeny < 0 || sd->mail.zeny > sd->status.zeny )
		return 0;

	n = sd->mail.index;
	if( sd->mail.amount )
	{
		if( sd->status.inventory[n].amount < sd->mail.amount )
			return 0;

		memcpy(&msg->item, &sd->status.inventory[n], sizeof(struct item));
		msg->item.amount = sd->mail.amount;
	}

	msg->zeny = sd->mail.zeny;

	return 1;
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
