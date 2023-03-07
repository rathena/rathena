// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "duel.hpp"

#include <stdio.h>
#include <string.h>
#include <unordered_map>

#include <common/cbasetypes.hpp>
#include <common/timer.hpp>

#include "atcommand.hpp"  // msg_txt
#include "battle.hpp"
#include "clif.hpp"
#include "pc.hpp"

//std::recursive_mutex> duel_list_mutex; //preparation for multithread
std::unordered_map<size_t,struct duel> duel_list;

std::unordered_map<size_t,struct duel> duel_get_list() { return duel_list; } //this could be exposed but not really necessarly, (hiden impl)
bool duel_exist( size_t did ) { return duel_list.find( did ) != duel_list.end(); }
duel& duel_get_duelid(size_t did) { return duel_list.at(did); }

/**
 * Number of duels created
 * @return duel_list size
 */
size_t duel_counttotal() {
	return duel_list.size();  
}

/**
 * Number of active duels (player has accepted the duel)
 * @return active duels
 */
size_t duel_countactives() 
{ 
	size_t count = 0;
	for ( const auto& lcur : duel_list )
		if ( lcur.second.members_count > 1 ) ++count;
	return count; 
} 

static void duel_set(const size_t did, map_session_data* sd);

/*
 * Save the current time of the duel in PC_LAST_DUEL_TIME
 */
void duel_savetime(map_session_data* sd)
{
	time_t timer;
	struct tm *t;

	time(&timer);
	t = localtime(&timer);

	pc_setglobalreg(sd, add_str("PC_LAST_DUEL_TIME"), t->tm_mday*24*60 + t->tm_hour*60 + t->tm_min);
}

/*
 * Check if the time elapsed between last duel is enough to launch another.
 */
bool duel_checktime(map_session_data* sd)
{
	int64 diff;
	time_t timer;
	struct tm *t;

	time(&timer);
	t = localtime(&timer);

	diff = t->tm_mday*24*60 + t->tm_hour*60 + t->tm_min - pc_readglobalreg(sd, add_str("PC_LAST_DUEL_TIME"));

	return !(diff >= 0 && diff < battle_config.duel_time_interval);
}

/*
 * Check if duel respect the member limit
 */
bool duel_check_player_limit(struct duel& pDuel)
{
	if(pDuel.max_players_limit > 0 &&
		pDuel.members_count >= pDuel.max_players_limit) {
		return false;
	}
	return true;
}

/*
 * Display opponents name of sd
 */
static int duel_showinfo_sub(map_session_data* sd, va_list va)
{
	map_session_data *ssd = va_arg(va, map_session_data*);
	int *p = va_arg(va, int*);

	if (sd->duel_group != ssd->duel_group) 
		return 0;

	char output[256];
	sprintf(output, "      %d. %s", ++(*p), sd->status.name);
	clif_messagecolor(&ssd->bl, color_table[COLOR_LIGHT_GREEN], output, false, SELF);
	return 1;
}

/*
 * Display duel infos,
 * Number of duely...
 */
void duel_showinfo(const size_t did, map_session_data* sd)
{
	//std::lock_guard<std::recursive_mutex> _(duel_list_mutex); //or shared_ptr	
	if ( !duel_exist( did ) )
		return;

	int p=0;
	char output[256];

	if(duel_list[did].max_players_limit > 0)
		sprintf(output, msg_txt(sd,370), //" -- Duels: %d/%d, Members: %d/%d, Max players: %d --"
			did, duel_counttotal(),
			duel_list[did].members_count,
			duel_list[did].members_count + duel_list[did].invites_count,
			duel_list[did].max_players_limit);
	else
		sprintf(output, msg_txt(sd,371), //" -- Duels: %d/%d, Members: %d/%d --"
			did, duel_counttotal(),
			duel_list[did].members_count,
			duel_list[did].members_count + duel_list[did].invites_count);

	clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], output, false, SELF);
	map_foreachpc(duel_showinfo_sub, sd, &p);
}

/*
* Moves sd to duel
*/
static void duel_set(const size_t did, map_session_data* sd) {
	sd->state.changemap = 1;
	sd->state.warping = 1;

	// As you move to a different plane, ground effects need to be cleared
	skill_clear_unitgroup(&sd->bl);
	skill_unit_move(&sd->bl, gettick(), 2);
	skill_cleartimerskill(&sd->bl);

	sd->duel_group = did;

	skill_unit_move(&sd->bl, gettick(), 3);

	sd->state.changemap = 0;
	sd->state.warping = 0;
}

/*
 * Create a new duel for sd
 * return new duel_id or 0 when fail
 */
size_t duel_create(map_session_data* sd, const unsigned int maxpl)
{
	static size_t lastID=0;
	lastID++;

	{ //mutex scope
		//std::lock_guard<std::recursive_mutex> _(duel_list_mutex);
		duel_list[lastID].members_count++;
		duel_list[lastID].invites_count = 0;
		duel_list[lastID].max_players_limit = maxpl;
	}
	duel_set(lastID, sd);

	char output[256];
	strcpy(output, msg_txt(sd,372)); // " -- Duel has been created (@invite/@leave) --"
	clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], output, false, SELF);
	clif_map_property(&sd->bl, MAPPROPERTY_FREEPVPZONE, SELF);
	//clif_misceffect2(&sd->bl, 159);
	return lastID;
}

/*
 * Invite opponent into the duel.
 * @did = duel id
 * @sd = inviting player
 * @target_sd = invited player
 */
bool duel_invite(const size_t did, map_session_data* sd, map_session_data* target_sd)
{
	//std::lock_guard<std::recursive_mutex> _(duel_list_mutex);
	if ( !duel_exist( did ) )
		return false;

	target_sd->duel_invite = did;
	duel_list[did].invites_count++;

	char output[256];
	// " -- Player %s invites %s to duel --"
	sprintf(output, msg_txt(sd,373), sd->status.name, target_sd->status.name);
	clif_disp_message(&sd->bl, output, strlen(output), DUEL_WOS);
	// "Blue -- Player %s invites you to PVP duel (@accept/@reject) --"
	sprintf(output, msg_txt(sd,374), sd->status.name);
	clif_broadcast((struct block_list *)target_sd, output, strlen(output)+1, BC_BLUE, SELF);
	return true;
}

/*
 * Sub function to loop trough all duely to remove invite for sd
 * @sd = leaving player
 * @va = list(only contain duel_id atm)
 */
static int duel_leave_sub(map_session_data* sd, va_list va)
{
	size_t did = va_arg(va, size_t);
	if (sd->duel_invite == did)
		sd->duel_invite = 0;
	return 0;
}

/*
 * Make a player leave a duel
 * @did = duel id
 * @sd = leaving player
 */
bool duel_leave(const size_t did, map_session_data* sd)
{
	//std::lock_guard<std::recursive_mutex> _(duel_list_mutex);
	if ( !duel_exist( did ) )
		return false;

	duel_list[did].members_count--;

	if(duel_list[did].members_count == 0) {
		map_foreachpc(duel_leave_sub, did);
		duel_list.erase( did );
	}
	duel_set(0, sd);
	duel_savetime(sd);

	char output[256];
	// " <- Player %s has left duel --"
	sprintf(output, msg_txt(sd,375), sd->status.name);
	clif_disp_message(&sd->bl, output, strlen(output), DUEL_WOS);
	clif_map_property(&sd->bl, MAPPROPERTY_NOTHING, SELF);
	return true;
}

/*
 * Make a player accept a duel
 * @did = duel id
 * @sd = player accepting duel
 */
bool duel_accept(const size_t did, map_session_data* sd)
{
	{ //mutex scope
		//std::lock_guard<std::recursive_mutex> _(duel_list_mutex);
		if ( !duel_exist( did ) )
			return false;

		duel_list[did].members_count++;
		duel_list[did].invites_count--;
	}
	duel_set( sd->duel_invite, sd );
	sd->duel_invite = 0;

	char output[256];
	// " -> Player %s has accepted duel --"
	sprintf(output, msg_txt(sd,376), sd->status.name);
	clif_disp_message(&sd->bl, output, strlen(output), DUEL_WOS);
	clif_map_property(&sd->bl, MAPPROPERTY_FREEPVPZONE, SELF);
	//clif_misceffect2(&sd->bl, 159);
	return true;
}

/*
 * Make a player decline a duel
 * @did = duel id
 * @sd = player refusing duel
 */
bool duel_reject(const size_t did, map_session_data* sd)
{
	{
		//std::lock_guard<std::recursive_mutex> _(duel_list_mutex);
		if ( !duel_exist( did ) )
			return false;

		duel_list[did].invites_count--;
		sd->duel_invite = 0;
	}

	char output[256];
	// " -- Player %s has rejected duel --"
	sprintf(output, msg_txt(sd,377), sd->status.name);
	clif_disp_message(&sd->bl, output, strlen(output), DUEL_WOS);
	return true;
}

/*
 * Destructor of duel module
 * Put cleanup code here before server end
 */
void do_final_duel(void)
{
	duel_list.clear();
}

/*
 * Initialisator of duel module
 */
void do_init_duel(void) {
	duel_list.clear();
}
