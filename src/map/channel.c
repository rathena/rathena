// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/conf.h" //libconfig
#include "../common/showmsg.h"
#include "../common/strlib.h" //safestrncpy
#include "../common/socket.h" //set_eof

#include "map.h" //msg_conf
#include "clif.h" //clif_chsys_msg
#include "channel.h"

#include <stdlib.h>

static DBMap* channel_db; // channels

struct Channel_Config channel_config;

DBMap* channel_get_db(void){ return channel_db; }

struct chan_banentry {
	uint32 char_id;
	char char_name[NAME_LENGTH];
} chan_banentry;

/*
 * Create *channel
 * - will then add it in channel_db if type not map or ally
 * @name : the name channel will be given, can't be null
 * @pass : can be null. if we want to restrain access
 * @color : display color type
 * @chantype : type of channel
 * return
 *  NULL : creation failed
 */
struct Channel* channel_create(char *name, char *pass, unsigned char color, enum Channel_Type chantype, int val) {
	struct Channel* channel;

	if(!name) return NULL;

	CREATE( channel, struct Channel, 1 ); //will exit on fail allocation
	channel->users = idb_alloc(DB_OPT_BASE);
	channel->banned = idb_alloc(DB_OPT_BASE|DB_OPT_RELEASE_DATA);
	channel->opt = CHAN_OPT_BASE;
	channel->type = chantype;
	channel->color = color;
	safestrncpy(channel->name, name, CHAN_NAME_LENGTH);
	if( !pass )
		channel->pass[0] = '\0';
	else
		safestrncpy(channel->pass, pass, CHAN_NAME_LENGTH);

	//ShowInfo("Create channel %s type=%d, val=%d\n",channel->name,chantype,val);
	switch(channel->type){
	case CHAN_TYPE_MAP: channel->m = val; break;
	case CHAN_TYPE_ALLY: channel->gid = val; break;
	case CHAN_TYPE_PRIVATE: channel->owner = val; //dont break here private need to put in db
	default: strdb_put(channel_db, channel->name, channel);
	}

	return channel;
}

/*
 * Delete channel *channel
 * - check if there is any user in channel and make them all quit
 * return
 *  0 : success
 *  -1 : invalid channel
 *  -2 : can't delete now
 */
int channel_delete(struct Channel *channel) {
	if(!channel)
		return -1;
	if(channel->type == CHAN_TYPE_PUBLIC && runflag == MAPSERVER_ST_RUNNING) //only delete those serv stop
		return -2;
	if( db_size(channel->users)) {
		struct map_session_data *sd;
		DBIterator *iter = db_iterator(channel->users);
		for( sd = dbi_first(iter); dbi_exists(iter); sd = dbi_next(iter) ) { //for all users
			channel_clean(channel,sd,1); //make all quit
		}
		dbi_destroy(iter);
	}
	//ShowInfo("Deleting channel %s\n",channel->name);
	db_destroy(channel->users);
	db_destroy(channel->banned);
	switch(channel->type){
	case CHAN_TYPE_MAP:
		map[channel->m].channel = NULL;
		aFree(channel);
		break;
	case CHAN_TYPE_ALLY: {
		struct guild *g = guild_search(channel->gid);
		if(g) g->channel = NULL;
		aFree(channel);
		break;
	}
	default:
		strdb_remove(channel_db, channel->name);
		break;
	}
	return 0;
}

/*
 * Make player *sd join a channel *channel
 * - add charid to channel user list
 * - add *channel to user channel list
 * return
 *  0 : success
 * -1 : invalid channel or sd
 * -2 : sd already in channel
 * -3 : sd banned
 */
int channel_join(struct Channel *channel, struct map_session_data *sd) {
	if(!channel || !sd)
		return -1;
	if(sd->state.autotrade)
		return 0; // fake success
	if(channel_haspc(channel,sd)==1)
		return -2;

	if(channel_haspcbanned(channel,sd)==1){
		char output[128];
		sprintf(output, msg_txt(sd,1438),channel->name); //You're currently banned from the '%s' channel.
		clif_displaymessage(sd->fd, output);
		return -3;
	}

	RECREATE(sd->channels, struct Channel *, ++sd->channel_count);
	sd->channels[ sd->channel_count - 1 ] = channel;
	idb_put(channel->users, sd->status.char_id, sd);

	if( sd->stealth ) {
		sd->stealth = false;
	} else if( channel->opt & CHAN_OPT_ANNOUNCE_JOIN ) {
		char message[60];
		sprintf(message, "[ #%s ] '%s' has joined.",channel->name,sd->status.name);
		clif_channel_msg(channel,sd,message,channel->color);
	}

	/* someone is cheating, we kindly disconnect the bastard */
	if( sd->channel_count > 200 ) {
		set_eof(sd->fd);
	}

	return 0;
}

/*
 * Make *sd join the map channel
 * create the map_channel if not exist
 * return
 *  -1 : invalid sd
 *  -2 : sd already in channel (channel_join)
 *  -3 : sd banned (channel_join)
 */
int channel_mjoin(struct map_session_data *sd) {
	if(!sd) return -1;

	if( !map[sd->bl.m].channel ) {
		map[sd->bl.m].channel = channel_create(channel_config.map_chname,NULL,channel_config.map_chcolor,CHAN_TYPE_MAP,sd->bl.m);
	}

	if( !( map[sd->bl.m].channel->opt & CHAN_OPT_ANNOUNCE_JOIN ) ) {
		char mout[60];
		sprintf(mout, msg_txt(sd,1435),channel_config.map_chname,map[sd->bl.m].name); // You're now in the '#%s' channel for '%s'.
		clif_disp_onlyself(sd, mout, strlen(mout));
	}

	return channel_join(map[sd->bl.m].channel,sd);
}

/*
 * Make all ally member of guild *g join our guild chan
 * nb : they only join if they are into their own guild channel (if they not they probably left it)
 * return
 *  0 : success
 *  -1 : invalid guild or no channel for guild
 */
int channel_ajoin(struct guild *g){
	int i;
	struct map_session_data *pl_sd;

	if(!g || !g->channel) return -1;
	for (i = 0; i < MAX_GUILDALLIANCE; i++){
		struct guild *ag; //allied guld
		struct guild_alliance *ga = &g->alliance[i]; //guild alliance
		if(ga->guild_id && (ga->opposition==0) && (ag=guild_search(ga->guild_id))){
			for (i = 0; i < ag->max_member; i++){ //load all guildmember
				pl_sd = ag->member[i].sd;
				if(channel_haspc(ag->channel,pl_sd)==1)  //only if they are in their own guildchan
					channel_join(g->channel,pl_sd);
			}
		}
	}
	return 0;
}

/*
 * Make *sd join the guild channel
 * create a chan guild if not exist
 * return
 *   0 : success
 *  -1 : invalid sd
 *  -2 : sd has no guild attached
 */
int channel_gjoin(struct map_session_data *sd, int flag){
	struct Channel *channel;
	struct guild *g;

	if(!sd) return -1;
	g = sd->guild;
	if(!g) return -2;

	channel = g->channel;
	if(!channel){
		channel = channel_create(channel_config.ally_chname,NULL,channel_config.ally_chcolor,CHAN_TYPE_ALLY,g->guild_id);
		g->channel = channel;
		channel_ajoin(g);
	}
	if(flag&1) {
		channel_join(channel,sd);	//join our guild chat
	}
	if(flag&2){
		int i;
		for (i = 0; i < MAX_GUILDALLIANCE; i++){
			struct guild *ag; //allied guld
			struct guild_alliance *ga = &g->alliance[i]; //guild alliance
			if(ga->guild_id && (ga->opposition==0) && (ag=guild_search(ga->guild_id)) ) //only join allies
				channel_join(ag->channel,sd);
		}
	}
	return 0;
}

/*
 * Make *sd leave *channel and cleanup association.
 *  - if no one remain in chat delete it
 * @flag&1 called from delete do not recall delete
 *return
 *  0 : success
 *  -1 : invalid sd or channel
 */
int channel_clean(struct Channel *channel, struct map_session_data *sd, int flag) {
	unsigned char i;

	if(!channel || !sd)
		return -1;

	if( channel == sd->gcbind )
		sd->gcbind = NULL;

	ARR_FIND(0, sd->channel_count, i, sd->channels[i] == channel);
	if( i < sd->channel_count ) {
		unsigned char cursor = i;
		sd->channels[i] = NULL;
		for(; i < sd->channel_count; i++ ) { //slice move list down
			if( sd->channels[i] == NULL )
				continue;
			if(i != cursor)
				sd->channels[cursor] = sd->channels[i];
			cursor++;
		}
		if ( !(sd->channel_count = cursor) ) { //if in no more chan delete db
			aFree(sd->channels);
			sd->channels = NULL;
		}
	}

	idb_remove(channel->users,sd->status.char_id); //remove user for channel user list
	//auto delete when no more user in
	if( !db_size(channel->users) && !(flag&1) )
		channel_delete(channel);

	return 0;
}

/*
 * Make a *sd leave a type of chan.
 * @type&1 : quit guild chan
 * @type&2 : quit ally chans
 * @type&4 : quit map chan
 * @type&8 : quit all users joined chan
 * return
 *  0 : success
 *  -1 : invalid sd
 *
 */
int channel_pcquit(struct map_session_data *sd, int type){
	int i;

	//On closing state we could have clean all chan by sd but pcquit is more used to free unit when
	//he quit a map_server, not call in map_quit cause we need to cleanup when we switch map-server as well
	if(!sd) return -1;

	// Leave all chat channels.
	if(type&(1|2) && channel_config.ally_enable && sd->guild){ //quit guild and ally chan
		struct guild *g = sd->guild;
		if(type&1 && channel_haspc(g->channel,sd)==1){
			channel_clean(g->channel,sd,0); //leave guild chan
		}
		if(type&2){
			for (i = 0; i < MAX_GUILDALLIANCE; i++) { //leave all alliance chan
				struct guild *ag; //allied guild
				if( g->alliance[i].guild_id && (ag = guild_search(g->alliance[i].guild_id) ) ) {
					if(channel_haspc(ag->channel,sd) == 1)
						channel_clean(ag->channel,sd,0);
					break;
				}
			}
		}
	}
	if(type&4 && channel_config.map_enable && channel_haspc(map[sd->bl.m].channel,sd)==1){ //quit map chan
		channel_clean(map[sd->bl.m].channel,sd,0);
	}
	if(type&8 && sd->channel_count ) { //quit all chan
		uint8 count = sd->channel_count;
		for( i = count-1; i >= 0; i--) { //going backward to avoid shifting
			channel_clean(sd->channels[i],sd,0);
		}
	}
	return 0;
}

/*
 * Format *msg from *sd to send it in *channel
 * Also truncate extra char if msg too long (max=RACHSYS_MSG_LENGTH)
 * return
 *  0 : success
 *  -1 : invalid sd, channel, or msg
 *  -2 : delay msg too low since last talk
 */
int channel_send(struct Channel *channel, struct map_session_data *sd, const char *msg) {
	if(!channel || !sd || !msg)
		return -1;

	if(!pc_has_permission(sd, PC_PERM_CHANNEL_ADMIN) && channel->msg_delay != 0 && DIFF_TICK(sd->channel_tick + ( channel->msg_delay * 1000 ), gettick()) > 0) {
		clif_colormes(sd->fd,color_table[COLOR_RED],msg_txt(sd,1455)); //You're talking too fast!
		return -2;
	}
	else {
		char message[CHAN_MSG_LENGTH], color;
		if((channel->opt)&CHAN_OPT_COLOR_OVERRIDE && sd->fontcolor)
			color = sd->fontcolor;
		else
			color = channel->color;
		snprintf(message, CHAN_MSG_LENGTH, "[ #%s ] %s : %s",channel->name,sd->status.name, msg);
		clif_channel_msg(channel,sd,message,color);
		sd->channel_tick = gettick();
	}
	return 0;
}

/*
 * Check parameters for channel creation
 * @type (bitflag)
 *	1 : check name # + length
 *	2 : check if already exist, need 1
 *	4 : check password length
 * return
 *  0 : success
 *  -1 : bad chan name
 *  -2 : bad chan name length
 *  -3 : pass given too long
 *  -4 : chan already exists
 */
int channel_chk(char *chname, char *chpass, int type){
	if(type&1){ //check name
		if( chname[0] != '#' )
			return -1;
		if ( strlen(chname) < 3 || strlen(chname) > CHAN_NAME_LENGTH )
			return -2;
		if( (type&2) && (
			strcmpi(chname + 1,channel_config.map_chname) == 0
			|| strcmpi(chname + 1,channel_config.ally_chname) == 0
			|| strdb_exists(channel_db, chname + 1) )
			) {
			return -4;
		}
	}
	if (type&4 && (chpass != '\0' && strlen(chpass) > CHAN_NAME_LENGTH ) ) {
		return -3;
	}

	return 0;
}

/*
 * Lookup to found a channel from his name.
 * @chname : channel name
 * @sd : can be NULL, use to solve #map and #ally case
 * @flag&1 : create channel if not exist (map or ally only)
 * @flag&2 : join the channel (map or ally only)
 * return
 *  NULL : channel not found
 */
struct Channel* channel_name2channel(char *chname, struct map_session_data *sd, int flag){
	if(channel_chk(chname, NULL, 1))
		return NULL;
	if(sd && strcmpi(chname + 1,channel_config.map_chname) == 0){
		if(flag&1 && !map[sd->bl.m].channel)
			map[sd->bl.m].channel = channel_create(channel_config.map_chname,NULL,channel_config.map_chcolor,CHAN_TYPE_MAP,sd->bl.m);
		if(flag&2)
			channel_mjoin(sd);
		return map[sd->bl.m].channel;
	}
	else if(sd && (strcmpi(chname + 1,channel_config.ally_chname) == 0) && sd->guild){
		if(flag&1 && !sd->guild->channel)
			sd->guild->channel = channel_create(channel_config.ally_chname,NULL,channel_config.ally_chcolor,CHAN_TYPE_ALLY,sd->guild->guild_id);
		if(flag&2)
			channel_gjoin(sd,3);
		return sd->guild->channel;
	}
	else
		return (struct Channel*) strdb_get(channel_db, chname + 1);
	return NULL;
}

/*
 * Channel check if he has *sd in his user list
 * return
 *  -1 : invalid sd or channel
 *  0 : not found
 *  1 : has pc
 */
int channel_haspc(struct Channel *channel,struct map_session_data *sd){
	if(!channel || !sd) return -1;
	return (idb_exists(channel->users, sd->status.char_id))?1:0;
}

/*
 * Channel check if *sd is banned from channel (banned ?)
 * return
 *  -1 : invalid sd or channel
 *  0 : not found
 *  1 : has pc
 */
int channel_haspcbanned(struct Channel *channel,struct map_session_data *sd){
	if(!channel || !sd) return -1;
	return (idb_exists(channel->banned, sd->status.char_id))?1:0;
}


/*
 * Player *sd check if he has Channel *channel in his channel list
 * return
 *  -1 : invalid channel or sd
 *  -2 : not found
 *  x>0 : has_chan at index x
 */
int channel_pc_haschan(struct map_session_data *sd, struct Channel *channel){
	int k;
	if(!channel || !sd) return -1; //channel or player doesn't exist
	ARR_FIND(0, sd->channel_count, k, strcmpi(channel->name,sd->channels[k]->name) == 0);
	if( k >= sd->channel_count ) return -2;
	return k;
}

/*
 * Display some info to user *sd on channels
 * @options :
 *  colors : display availables colors for chan system
 *  mine : list of chan *sd is in + nb of user
 *  void : list of public chan + map + guild + nb of user
 * return
 *  0 : success
 *  -1 : fail
 */
int channel_display_list(struct map_session_data *sd, char *options){

	if(!sd || !options)
		return -1;

	//display availaible colors
	if( options[0] != '\0' && strcmpi(options,"colors") == 0 ) {
		char msg[40];
		unsigned char k;
		clif_displaymessage(sd->fd, msg_txt(sd,1444)); // ---- Available Colors ----
		for( k = 0; k < channel_config.colors_count; k++ ) {
			if (channel_config.colors[k]) {
				sprintf(msg, msg_txt(sd,1445),channel_config.colors_name[k]);// - '%s'
				clif_colormes(sd->fd,channel_config.colors[k],msg);
			}
		}
	}
	else if( options[0] != '\0' && strcmpi(options,"mine") == 0 ) { //display chan I'm into
		clif_displaymessage(sd->fd, msg_txt(sd,1475)); // ---- My Channels ----
		if(!sd->channel_count)
			clif_displaymessage(sd->fd, msg_txt(sd,1476)); // You have not joined any channels.
		else {
			unsigned char k;

			for(k = 0; k < sd->channel_count; k++) {
				char output[128];
				struct Channel *channel;

				if (!(channel = sd->channels[k]))
					continue;

				sprintf(output, msg_txt(sd,1409), channel->name, db_size(channel->users));// - #%s (%d users)
				clif_displaymessage(sd->fd, output);
			}
		}
	}
	else { //display public chanels
		DBIterator *iter;
		bool has_perm = pc_has_permission(sd, PC_PERM_CHANNEL_ADMIN) ? true : false;
		struct Channel *channel;
		char output[128];

		clif_displaymessage(sd->fd, msg_txt(sd,1410)); // ---- Public Channels ----
		if( channel_config.map_enable && map[sd->bl.m].channel ) {
			sprintf(output, msg_txt(sd,1409), channel_config.map_chname, map[sd->bl.m].channel ? db_size(map[sd->bl.m].channel->users) : 0);// - #%s (%d users)
			clif_displaymessage(sd->fd, output);
		}
		if( channel_config.ally_enable && sd->status.guild_id ) {
			struct guild *g = sd->guild;
			if (g && g->channel) {
				sprintf(output, msg_txt(sd,1409), channel_config.ally_chname, db_size(((struct Channel *)g->channel)->users));// - #%s (%d users)
				clif_displaymessage(sd->fd, output);
			}
		}
		iter = db_iterator(channel_db);
		for(channel = dbi_first(iter); dbi_exists(iter); channel = dbi_next(iter)) {
			if( has_perm || channel->type == CHAN_TYPE_PUBLIC ) {
				sprintf(output, msg_txt(sd,1409), channel->name, db_size(channel->users));// - #%s (%d users)
				clif_displaymessage(sd->fd, output);
			}
		}
		dbi_destroy(iter);
	}

	return 0;
}

/*
 * A user *sd is attempting to create a channel named *chname with pass *chpass
 * return
 *  0 : success
 *  -1 : fail
 */
int channel_pccreate(struct map_session_data *sd, char *chname, char *chpass){
	char output[128];
	int8 res;

	if(!sd || !chname)
		return 0;

	res = channel_chk(chname,chpass,7);
	if(res==0){ //success
		struct Channel *channel = channel_create(chname + 1,chpass,0,CHAN_TYPE_PRIVATE,sd->status.char_id);
		channel_join(channel,sd);
		if( !( channel->opt & CHAN_OPT_ANNOUNCE_JOIN ) ) {
			sprintf(output, msg_txt(sd,1403),chname); // You're now in the '%s' channel.
			clif_displaymessage(sd->fd, output);
		}
	} else { //failure display cause
		switch(res){
		case -1: sprintf(output, msg_txt(sd,1405), CHAN_NAME_LENGTH); break;// Channel name must start with '#'.
		case -2: sprintf(output, msg_txt(sd,1406), CHAN_NAME_LENGTH); break;// Channel length must be between 3 and %d.
		case -3: sprintf(output, msg_txt(sd,1436), CHAN_NAME_LENGTH); break;// Channel password can't be over %d characters.
		case -4: sprintf(output, msg_txt(sd,1407), chname);// Channel '%s' is not available.
		}
		clif_displaymessage(sd->fd, output);
		return -1;
	}
	return 0;
}

/*
 * A user *sd is attempting to delete a channel named *chname
 * return
 *  0 : success
 *  -1 : fail
 */
int channel_pcdelete(struct map_session_data *sd, char *chname){
	struct Channel *channel;
	char output[128];

	if(!sd || !chname) return 0;

	if( channel_chk(chname,NULL,1) ) {
		clif_displaymessage(sd->fd, msg_txt(sd,1405));// Channel name must start with '#'.
		return -1;
	}

	channel = channel_name2channel(chname,sd,0);
	if(channel_pc_haschan(sd,channel)<0){
		sprintf(output, msg_txt(sd,1425),chname);// You're not part of the '%s' channel.
		clif_displaymessage(sd->fd, output);
		return -2; //channel doesn't exist or player don't have it
	}
	channel_delete(channel);

	sprintf(output, msg_txt(sd,1448),chname); // Channel '%s' deleted.
	clif_displaymessage(sd->fd, output);

	return 0;
}

/*
 * A user *sd is attempting to leave a channel named *chname
 * return
 *  0 : success
 *  -1 : fail
 */
int channel_pcleave(struct map_session_data *sd, char *chname){
	struct Channel *channel;
	char output[128];

	if(!sd || !chname)
		return 0;

	if( channel_chk(chname,NULL,1) ) {
		clif_displaymessage(sd->fd, msg_txt(sd,1405));// Channel name must start with '#'.
		return -1;
	}

	channel = channel_name2channel(chname,sd,0);
	if(channel_pc_haschan(sd,channel)<0){
		sprintf(output, msg_txt(sd,1425),chname);// You're not part of the '%s' channel.
		clif_displaymessage(sd->fd, output);
		return -2; //channel doesn't exist or player don't have it
	}

	if( !channel_config.closing && (channel->opt & CHAN_OPT_ANNOUNCE_JOIN) ) {
		char message[60];
		sprintf(message, "#%s '%s' left",channel->name,sd->status.name);
		clif_channel_msg(channel,sd,message,channel->color);
	}
	switch(channel->type){
	case CHAN_TYPE_ALLY: channel_pcquit(sd,3); break;
	case CHAN_TYPE_MAP: channel_pcquit(sd,4); break;
	default: //private and public atm
		channel_clean(channel,sd,0);
	}

	sprintf(output, msg_txt(sd,1426),chname); // You've left the '%s' channel.
	clif_displaymessage(sd->fd, output);
	return 0;
}

/*
 * A user *sd is attempting to join a channel named *chname
 * return
 *  0 : success
 *  -1 : fail
 */
int channel_pcjoin(struct map_session_data *sd, char *chname, char *pass){
	struct Channel *channel;
	char output[128];

	if(!sd || !chname)
		return 0;

	if( channel_chk(chname,NULL,1) ) {
		clif_displaymessage(sd->fd, msg_txt(sd,1405));// Channel name must start with '#'.
		return -1;
	}

	channel = channel_name2channel(chname,sd,1);
	if(channel){
		if(channel_haspc(channel,sd)==1) {
			sprintf(output, msg_txt(sd,1434),chname); // You're already in the '%s' channel.
			clif_displaymessage(sd->fd, output);
			return -1;
		}
		else if( channel->pass[0] != '\0') { //chan has a pass
			if(strcmp(channel->pass,pass) != 0){ //wrong pass entry
				if( pc_has_permission(sd, PC_PERM_CHANNEL_ADMIN) ) {
					sd->stealth = true;
				} else {
					sprintf(output, msg_txt(sd,1401),chname,"@join"); // Channel '%s' is password-protected (usage: %s <#channel_name> <password>).
					clif_displaymessage(sd->fd, output);
					return -1;
				}
			}
		}
	}
	else {
		sprintf(output, msg_txt(sd,1400),chname,"@join"); // Unknown channel '%s' (usage: %s <#channel_name>).
		clif_displaymessage(sd->fd, output);
		return -1;
	}

	if( !( channel->opt & CHAN_OPT_ANNOUNCE_JOIN ) ) {
		sprintf(output, msg_txt(sd,1403),chname); // You're now in the '%s' channel.
		clif_displaymessage(sd->fd, output);
	}

	switch(channel->type){
	case CHAN_TYPE_ALLY: channel_gjoin(sd,3); break;
	case CHAN_TYPE_MAP: channel_mjoin(sd); break;
	default: //private and public atm
		channel_join(channel,sd);
	}

	return 0;
}

/*
 * A user *sd is attempting to change color with *color of  a channel named *chname
 * return
 *  0 : success
 *  -1 : fail
 */
int channel_pccolor(struct map_session_data *sd, char *chname, char *color){
	struct Channel *channel;
	char output[128];
	int k;

	if(!sd)
		return 0;

	if( channel_chk(chname,NULL,1) ) {
		clif_displaymessage(sd->fd, msg_txt(sd,1405));// Channel name must start with '#'.
		return -1;
	}


	channel = channel_name2channel(chname,sd,0);
	if( !channel ) {
		sprintf(output, msg_txt(sd,1407), chname);// Channel '%s' is not available.
		clif_displaymessage(sd->fd, output);
		return -1;
	}

	if( channel->owner != sd->status.char_id && !pc_has_permission(sd, PC_PERM_CHANNEL_ADMIN) ) {
		sprintf(output, msg_txt(sd,1412), chname);// You're not the owner of channel '%s'.
		clif_displaymessage(sd->fd, output);
		return -1;
	}

	ARR_FIND(0,channel_config.colors_count,k,( strcmpi(color,channel_config.colors_name[k]) == 0 ) );
	if( k >= channel_config.colors_count ) {
		sprintf(output, msg_txt(sd,1411), color);// Unknown color '%s'.
		clif_displaymessage(sd->fd, output);
		return -1;
	}
	channel->color = k;
	sprintf(output, msg_txt(sd,1413),chname,channel_config.colors_name[k]);// '%s' channel color updated to '%s'.
	clif_displaymessage(sd->fd, output);
	return 0;
}

/*
 * A user *sd is attempting to bind (make default message output display chan talk)
 * from a channel named *chname
 * return
 *  0 : success
 *  -1 : fail
 */
int channel_pcbind(struct map_session_data *sd, char *chname){
	struct Channel *channel;
	char output[128];

	if(!sd)
		return 0;

	if( channel_chk(chname,NULL,1) ) {
		clif_displaymessage(sd->fd, msg_txt(sd,1405));// Channel name must start with '#'.
		return -1;
	}

	channel = channel_name2channel(chname,sd,0);
	if(channel_pc_haschan(sd,channel)<0){
		sprintf(output, msg_txt(sd,1425),chname);// You're not part of the '%s' channel.
		clif_displaymessage(sd->fd, output);
		return -2; //channel doesn't exist or player don't have it
	}
	sd->gcbind = channel;
	sprintf(output, msg_txt(sd,1431),chname); // Your global chat is now binded to the '%s' channel.
	clif_displaymessage(sd->fd, output);
	return 0;
}

/*
 * A user *sd is attempting to unbind
 * return
 *  0 : success
 *  -1 : fail
 */
int channel_pcunbind(struct map_session_data *sd){
	char output[128];

	if(!sd)
		return 0;

	if( sd->gcbind == NULL ) {
		clif_displaymessage(sd->fd, msg_txt(sd,1432));// Your global chat is not binded to any channel.
		return -1;
	}
	sprintf(output, msg_txt(sd,1433),sd->gcbind->name); // Your global chat is now unbinded from the '#%s' channel.
	clif_displaymessage(sd->fd, output);
	sd->gcbind = NULL;
	return 0;
}

/*
 * A user *sd is attempting to do something with the banlist
 * @flag == 0 : ban
 * @flag == 1 : unban
 * @flag == 2 : unbanall
 * @flag == 3 : banlist
 *  return
 *  0 : success
 *  -1 : fail
 */
int channel_pcban(struct map_session_data *sd, char *chname, char *pname, int flag){
	struct Channel *channel;
	char output[128];
	struct map_session_data *tsd = map_nick2sd(pname);

	if( channel_chk(chname,NULL,1) ) {
		clif_displaymessage(sd->fd, msg_txt(sd,1405));// Channel name must start with '#'.
		return -1;
	}

	channel = channel_name2channel(chname,sd,0);
	if( !channel ) {
		sprintf(output, msg_txt(sd,1407), chname);// Channel '%s' is not available.
		clif_displaymessage(sd->fd, output);
		return -1;
	}

	if( channel->owner != sd->status.char_id && !pc_has_permission(sd, PC_PERM_CHANNEL_ADMIN) ) {
		sprintf(output, msg_txt(sd,1412), chname);// You're not the owner of channel '%s'.
		clif_displaymessage(sd->fd, output);
		return -1;
	}

	if(flag != 2 && flag != 3){
		char banned;
		if(!tsd || pc_has_permission(tsd, PC_PERM_CHANNEL_ADMIN) ) {
			sprintf(output, msg_txt(sd,1464), pname);// Ban failed for player '%s'.
			clif_displaymessage(sd->fd, output);
			return -1;
		}

		banned = channel_haspcbanned(channel,tsd);
		if(!flag &&  banned==1) {
			sprintf(output, msg_txt(sd,1465), tsd->status.name);// Player '%s' is already banned from this channel.
			clif_displaymessage(sd->fd, output);
			return -1;
		}
		else if(flag==1 && banned==0) {
			sprintf(output, msg_txt(sd,1440), tsd->status.name);// Player '%s' is not banned from this channel.
			clif_displaymessage(sd->fd, output);
			return -1;
		}
	}
	else {
		if( !db_size(channel->banned) ) {
			sprintf(output, msg_txt(sd,1439), chname);// Channel '%s' contains no banned players.
			clif_displaymessage(sd->fd, output);
			return -1;
		}
	}

	//let properly alter the list now
	switch(flag){
	case 0: {
		struct chan_banentry *cbe;
		if (!tsd)
			return -1;
		CREATE(cbe, struct chan_banentry, 1);
		cbe->char_id = tsd->status.char_id;
		strcpy(cbe->char_name,tsd->status.name);
		idb_put(channel->banned, tsd->status.char_id, cbe);
		channel_clean(channel,tsd,0);
		sprintf(output, msg_txt(sd,1437),tsd->status.name,chname); // Player '%s' is banned from the '%s' channel.
		break;
		}
	case 1:
		if (!tsd)
			return -1;
		idb_remove(channel->banned, tsd->status.char_id);
		sprintf(output, msg_txt(sd,1441),tsd->status.name,chname); // Player '%s' is unbanned from the '%s' channel.
		break;
	case 2:
		db_clear(channel->banned);
		sprintf(output, msg_txt(sd,1442),chname); // Cleared all bans from the '%s' channel.
		break;
	case 3: {
		DBIterator *iter = db_iterator(channel->banned);
		struct chan_banentry *cbe;
		sprintf(output, msg_txt(sd,1443), channel->name);// ---- '#%s' Ban List:
		clif_displaymessage(sd->fd, output);
		for( cbe = dbi_first(iter); dbi_exists(iter); cbe = dbi_next(iter) ) { //for all users
			sprintf(output, "%d: %s",cbe->char_id,cbe->char_name);
			clif_displaymessage(sd->fd, output);
		}
		dbi_destroy(iter);
		}
		return 0;
	}
	clif_displaymessage(sd->fd, output);

	return 0;
}

/*
 * A user *sd is attempting to set an option on channel named *chname
 * @chname = channel name
 * @option = available = opt_str
 * @val = value to assign to option
 * return
 *  0 : success
 *  -1 : fail
 */
int channel_pcsetopt(struct map_session_data *sd, char *chname, const char *option, const char *val){
	struct Channel *channel;
	char output[128];
	int k, s=0;
	const char* opt_str[] = {
		"None",
		"JoinAnnounce",
		"MessageDelay",
		"ColorOverride",
	};

	if( channel_chk(chname,NULL,1) ) {
		clif_displaymessage(sd->fd, msg_txt(sd,1405));// Channel name must start with '#'.
		return -1;
	}

	channel = channel_name2channel(chname,sd,0);
	if( !channel ) {
		sprintf(output, msg_txt(sd,1407), chname);// Channel '%s' is not available.
		clif_displaymessage(sd->fd, output);
		return -1;
	}

	if( channel->owner != sd->status.char_id && !pc_has_permission(sd, PC_PERM_CHANNEL_ADMIN) ) {
		sprintf(output, msg_txt(sd,1412), chname);// You're not the owner of channel '%s'.
		clif_displaymessage(sd->fd, output);
		return -1;
	}
	
	s = ARRAYLENGTH(opt_str);
	ARR_FIND(1,s,k,( strncmpi(option,opt_str[k],3) == 0 )); //we only cmp 3 letter atm
	if(!option || option[0] == '\0' || k >= s ) {
		sprintf(output, msg_txt(sd,1447), option);// Unknown channel option '%s'.
		clif_displaymessage(sd->fd, output);
		clif_displaymessage(sd->fd, msg_txt(sd,1414));// ---- Available options:
		for( k = 1; k < s; k++ ) {
			sprintf(output, msg_txt(sd,1445), opt_str[k]);// - '%s'
			clif_displaymessage(sd->fd, output);
		}
		return -1;
	}

	if( val[0] == '\0' ) {
		if ( k == CHAN_OPT_MSG_DELAY ) {
			sprintf(output, msg_txt(sd,1466), opt_str[k]);// Input the number of seconds (0-10) for the '%s' option.
			clif_displaymessage(sd->fd, output);
			return -1;
		} else if( channel->opt & k ) {
			sprintf(output, msg_txt(sd,1449), opt_str[k],opt_str[k]); // Option '%s' is already enabled (use '@channel setopt %s 0' to disable).
			clif_displaymessage(sd->fd, output);
			return -1;
		} else {
			channel->opt |= k;
			sprintf(output, msg_txt(sd,1450), opt_str[k],channel->name);// Option '%s' is enabled for channel '#%s'.
			clif_displaymessage(sd->fd, output);
		}
	} else {
		int v = atoi(val);
		if( k == CHAN_OPT_MSG_DELAY ) {
			if( v < 0 || v > 10 ) {
				sprintf(output, msg_txt(sd,1451), v, opt_str[k]);// Value '%d' for option '%s' is out of range (limit 0-10).
				clif_displaymessage(sd->fd, output);
				return -1;
			}
			if( v == 0 ) {
				channel->opt &=~ k;
				channel->msg_delay = 0;
				sprintf(output, msg_txt(sd,1453), opt_str[k],channel->name,v);// Option '%s' is disabled for channel '#%s'.
				clif_displaymessage(sd->fd, output);
			} else {
				channel->opt |= k;
				channel->msg_delay = v;
				sprintf(output, msg_txt(sd,1452), opt_str[k],channel->name,v);// Option '%s' is enabled for channel '#%s' at %d seconds.
				clif_displaymessage(sd->fd, output);
			}
		} else {
			if( v ) {
				if( channel->opt & k ) {
					sprintf(output, msg_txt(sd,1449), opt_str[k],opt_str[k]); // Option '%s' is already enabled (use '@channel setopt %s 0' to disable).
					clif_displaymessage(sd->fd, output);
					return -1;
				} else {
					channel->opt |= k;
					sprintf(output, msg_txt(sd,1450), opt_str[k],channel->name);// Option '%s' is enabled for channel '#%s'.
					clif_displaymessage(sd->fd, output);
				}
			} else {
				if( !(channel->opt & k) ) {
					sprintf(output, msg_txt(sd,1450), opt_str[k],channel->name); // Option '%s' is enabled for channel '#%s'.
					clif_displaymessage(sd->fd, output);
					return -1;
				} else {
					channel->opt &=~ k;
					sprintf(output, msg_txt(sd,1453), opt_str[k],channel->name);// Option '%s' is disabled for channel '#%s'.
					clif_displaymessage(sd->fd, output);
				}
			}
		}
	}
	return 0;
}

/*
 * Read and verify configuration in confif_filename
 * Assign table value with value
 */
void channel_read_config(void) {
	config_t channels_conf;
	config_setting_t *chsys = NULL;
	const char *config_filename = "conf/channels.conf"; // FIXME hardcoded name

	if (conf_read_file(&channels_conf, config_filename))
		return;

	chsys = config_lookup(&channels_conf, "chsys");

	if (chsys != NULL) {
		config_setting_t *settings = config_setting_get_elem(chsys, 0);
		config_setting_t *channels;
		config_setting_t *colors;
		int i,k;
		const char *map_chname, *ally_chname,*map_color, *ally_color;
		int ally_enabled = 0, local_enabled = 0;
		int local_autojoin = 0, ally_autojoin = 0;
		int allow_user_channel_creation = 0;

		if( !config_setting_lookup_string(settings, "map_local_channel_name", &map_chname) )
			map_chname = "map";
		safestrncpy(channel_config.map_chname, map_chname, CHAN_NAME_LENGTH);

		if( !config_setting_lookup_string(settings, "ally_channel_name", &ally_chname) )
			ally_chname = "ally";
		safestrncpy(channel_config.ally_chname, ally_chname, CHAN_NAME_LENGTH);

		config_setting_lookup_bool(settings, "map_local_channel", &local_enabled);
		config_setting_lookup_bool(settings, "ally_channel_enabled", &ally_enabled);

		if( local_enabled )
			channel_config.map_enable = true;
		if( ally_enabled )
			channel_config.ally_enable = true;

		config_setting_lookup_bool(settings, "map_local_channel_autojoin", &local_autojoin);
		config_setting_lookup_bool(settings, "ally_channel_autojoin", &ally_autojoin);

		if( local_autojoin )
			channel_config.map_autojoin = true;
		if( ally_autojoin )
			channel_config.ally_autojoin = true;

		config_setting_lookup_bool(settings, "allow_user_channel_creation", &allow_user_channel_creation);

		if( allow_user_channel_creation )
			channel_config.user_chenable = true;

		if( (colors = config_setting_get_member(settings, "colors")) != NULL ) {
			int color_count = config_setting_length(colors);
			CREATE( channel_config.colors, unsigned long, color_count );
			CREATE( channel_config.colors_name, char *, color_count );
			for(i = 0; i < color_count; i++) {
				config_setting_t *color = config_setting_get_elem(colors, i);
				CREATE( channel_config.colors_name[i], char, CHAN_NAME_LENGTH );

				safestrncpy(channel_config.colors_name[i], config_setting_name(color), CHAN_NAME_LENGTH);
				channel_config.colors[i] = strtoul(config_setting_get_string_elem(colors,i),NULL,0);
				channel_config.colors[i] = (channel_config.colors[i] & 0x0000FF) << 16 | (channel_config.colors[i] & 0x00FF00) | (channel_config.colors[i] & 0xFF0000) >> 16;//RGB to BGR
			}
			channel_config.colors_count = color_count;
		}

		config_setting_lookup_string(settings, "map_local_channel_color", &map_color);

		for (k = 0; k < channel_config.colors_count; k++) {
			if( strcmpi(channel_config.colors_name[k],map_color) == 0 )
				break;
		}

		if( k < channel_config.colors_count ) {
			channel_config.map_chcolor = k;
		} else {
			ShowError("channels.conf: unknown color '%s' for 'map_local_channel_color', disabling '#%s'...\n",map_color,map_chname);
			channel_config.map_enable = false;
		}

		config_setting_lookup_string(settings, "ally_channel_color", &ally_color);

		for (k = 0; k < channel_config.colors_count; k++) {
			if( strcmpi(channel_config.colors_name[k],ally_color) == 0 )
				break;
		}

		if( k < channel_config.colors_count ) {
			channel_config.ally_chcolor = k;
		} else {
			ShowError("channels.conf: unknown color '%s' for 'ally_channel_color', disabling '#%s'...\n",ally_color,ally_chname);
			channel_config.ally_enable = false;
		}

		if( (channels = config_setting_get_member(settings, "default_channels")) != NULL ) {
			int channel_count = config_setting_length(channels);

			for(i = 0; i < channel_count; i++) {
				config_setting_t *channel = config_setting_get_elem(channels, i);
				const char *color = config_setting_get_string_elem(channels,i);
				char *name = config_setting_name(channel);
				for (k = 0; k < channel_config.colors_count; k++) {
					if( strcmpi(channel_config.colors_name[k],color) == 0 )
						break;
				}
				if( k == channel_config.colors_count ) {
					ShowError("channels.conf: unknown color '%s' for channel '%s', skipping channel...\n",color,name);
					continue;
				}
				if( strcmpi(name,channel_config.map_chname) == 0 || strcmpi(name,channel_config.ally_chname) == 0 || strdb_exists(channel_db, name) ) {
					ShowError("channels.conf: duplicate channel '%s', skipping channel...\n",name);
					continue;
				}
				channel_create(name,NULL,k,CHAN_TYPE_PUBLIC,0);
			}
		}

		ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' channels in '"CL_WHITE"%s"CL_RESET"'.\n", db_size(channel_db), config_filename);
		config_destroy(&channels_conf);
	}
}

/*
 * Initialise db and read config
 * return
 *  0 : success
 */
void do_init_channel(void) {
	channel_db = stridb_alloc(DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA, CHAN_NAME_LENGTH);
	channel_config.ally_enable = channel_config.map_enable = channel_config.ally_autojoin = channel_config.map_autojoin = false;
	channel_read_config();
}

/*
 * Close all and cleanup
 * NB map and guild need to cleanup their chan as well
 */
void do_final_channel(void) {
	DBIterator *iter;
	struct Channel *channel;
	
	//delete all in remaining chan db
	iter = db_iterator(channel_db);
	for( channel = dbi_first(iter); dbi_exists(iter); channel = dbi_next(iter) ) {
		channel_delete(channel);
	}
	dbi_destroy(iter);
	//at this point all user should have left their channel (private and public should be gone)
	db_destroy(channel_db);

	//delete all color thing
	if( channel_config.colors_count ) {
		int i=0;
		for(i = 0; i < channel_config.colors_count; i++) {
			aFree(channel_config.colors_name[i]);
		}
		aFree(channel_config.colors_name);
		aFree(channel_config.colors);
	}
}
