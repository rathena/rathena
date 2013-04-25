// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"

#include "atcommand.h"  // msg_txt
#include "clif.h"
#include "duel.h"
#include "pc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct duel duel_list[MAX_DUEL];
int duel_count = 0;

/*==========================================
 * Duel organizing functions [LuzZza]
 *------------------------------------------*/
void duel_savetime(struct map_session_data* sd)
{
	time_t timer;
	struct tm *t;

	time(&timer);
	t = localtime(&timer);

	pc_setglobalreg(sd, "PC_LAST_DUEL_TIME", t->tm_mday*24*60 + t->tm_hour*60 + t->tm_min);
}

int duel_checktime(struct map_session_data* sd)
{
	int diff;
	time_t timer;
	struct tm *t;

	time(&timer);
    t = localtime(&timer);

	diff = t->tm_mday*24*60 + t->tm_hour*60 + t->tm_min - pc_readglobalreg(sd, "PC_LAST_DUEL_TIME");

	return !(diff >= 0 && diff < battle_config.duel_time_interval);
}
static int duel_showinfo_sub(struct map_session_data* sd, va_list va)
{
	struct map_session_data *ssd = va_arg(va, struct map_session_data*);
	int *p = va_arg(va, int*);
	char output[256];

	if (sd->duel_group != ssd->duel_group) return 0;

	sprintf(output, "      %d. %s", ++(*p), sd->status.name);
	clif_disp_onlyself(ssd, output, strlen(output));
	return 1;
}

void duel_showinfo(const unsigned int did, struct map_session_data* sd)
{
	int p=0;
	char output[256];

	if(duel_list[did].max_players_limit > 0)
		sprintf(output, msg_txt(sd,370), //" -- Duels: %d/%d, Members: %d/%d, Max players: %d --"
			did, duel_count,
			duel_list[did].members_count,
			duel_list[did].members_count + duel_list[did].invites_count,
			duel_list[did].max_players_limit);
	else
		sprintf(output, msg_txt(sd,371), //" -- Duels: %d/%d, Members: %d/%d --"
			did, duel_count,
			duel_list[did].members_count,
			duel_list[did].members_count + duel_list[did].invites_count);

	clif_disp_onlyself(sd, output, strlen(output));
	map_foreachpc(duel_showinfo_sub, sd, &p);
}

int duel_create(struct map_session_data* sd, const unsigned int maxpl)
{
	int i=1;
	char output[256];

	while(duel_list[i].members_count > 0 && i < MAX_DUEL) i++;
	if(i == MAX_DUEL) return 0;

	duel_count++;
	sd->duel_group = i;
	duel_list[i].members_count++;
	duel_list[i].invites_count = 0;
	duel_list[i].max_players_limit = maxpl;

	strcpy(output, msg_txt(sd,372)); // " -- Duel has been created (@invite/@leave) --"
	clif_disp_onlyself(sd, output, strlen(output));

	clif_map_property(sd, MAPPROPERTY_FREEPVPZONE);
	clif_maptypeproperty2(&sd->bl,SELF);
	//clif_misceffect2(&sd->bl, 159);
	return i;
}

void duel_invite(const unsigned int did, struct map_session_data* sd, struct map_session_data* target_sd)
{
	char output[256];

	// " -- Player %s invites %s to duel --"
	sprintf(output, msg_txt(sd,373), sd->status.name, target_sd->status.name);
	clif_disp_message(&sd->bl, output, strlen(output), DUEL_WOS);

	target_sd->duel_invite = did;
	duel_list[did].invites_count++;

	// "Blue -- Player %s invites you to PVP duel (@accept/@reject) --"
	sprintf(output, msg_txt(sd,374), sd->status.name);
	clif_broadcast((struct block_list *)target_sd, output, strlen(output)+1, 0x10, SELF);
}

static int duel_leave_sub(struct map_session_data* sd, va_list va)
{
	int did = va_arg(va, int);
	if (sd->duel_invite == did)
		sd->duel_invite = 0;
	return 0;
}

void duel_leave(const unsigned int did, struct map_session_data* sd)
{
	char output[256];

	// " <- Player %s has left duel --"
	sprintf(output, msg_txt(sd,375), sd->status.name);
	clif_disp_message(&sd->bl, output, strlen(output), DUEL_WOS);

	duel_list[did].members_count--;

	if(duel_list[did].members_count == 0) {
		map_foreachpc(duel_leave_sub, did);
		duel_count--;
	}

	sd->duel_group = 0;
	duel_savetime(sd);
	clif_map_property(sd, MAPPROPERTY_NOTHING);
	clif_maptypeproperty2(&sd->bl,SELF);
}

void duel_accept(const unsigned int did, struct map_session_data* sd)
{
	char output[256];

	duel_list[did].members_count++;
	sd->duel_group = sd->duel_invite;
	duel_list[did].invites_count--;
	sd->duel_invite = 0;

	// " -> Player %s has accepted duel --"
	sprintf(output, msg_txt(sd,376), sd->status.name);
	clif_disp_message(&sd->bl, output, strlen(output), DUEL_WOS);

	clif_map_property(sd, MAPPROPERTY_FREEPVPZONE);
	clif_maptypeproperty2(&sd->bl,SELF);
	//clif_misceffect2(&sd->bl, 159);
}

void duel_reject(const unsigned int did, struct map_session_data* sd)
{
	char output[256];

	// " -- Player %s has rejected duel --"
	sprintf(output, msg_txt(sd,377), sd->status.name);
	clif_disp_message(&sd->bl, output, strlen(output), DUEL_WOS);

	duel_list[did].invites_count--;
	sd->duel_invite = 0;
}

void do_final_duel(void)
{
}

void do_init_duel(void)
{
	memset(&duel_list[0], 0, sizeof(duel_list));
}
