// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "channel.hpp"

#include <stdlib.h>

#include "../common/cbasetypes.hpp"
#include "../common/conf.hpp" //libconfig
#include "../common/malloc.hpp"
#include "../common/nullpo.hpp"
#include "../common/showmsg.hpp"
#include "../common/socket.hpp" //set_eof
#include "../common/strlib.hpp" //safestrncpy
#include "../common/timer.hpp"  // DIFF_TICK

#include "battle.hpp"
#include "clif.hpp" //clif_chsys_msg
#include "guild.hpp"
#include "map.hpp" //msg_conf
#include "pc.hpp"
#include "pc_groups.hpp"

static DBMap* channel_db; // channels

struct Channel_Config channel_config;

DBMap* channel_get_db(void){ return channel_db; }

/**
 * Create a channel
 * - Will then add it in the channel_db if the type is not map or ally
 * @param name: Channel name, can't be null
 * @param pass: Channel password, can be null
 * @param color: Display color
 * @param chantype: Channel type
 * @return NULL on failure or Channel on success
 */
struct Channel* channel_create(struct Channel *tmp_chan) {
	struct Channel* channel;

	if (!tmp_chan->name[0])
		return NULL;

	CREATE(channel, struct Channel, 1); //will exit on fail allocation
	//channel->id = tmp_chan->id;
	channel->users = idb_alloc(DB_OPT_BASE);
	channel->banned = idb_alloc(static_cast<DBOptions>(DB_OPT_BASE|DB_OPT_RELEASE_DATA) );
	channel->opt = tmp_chan->opt;
	channel->type = tmp_chan->type;
	channel->color = tmp_chan->color;
	safestrncpy(channel->name, tmp_chan->name, CHAN_NAME_LENGTH); // Store channel cname without '#'
	safestrncpy(channel->alias, tmp_chan->alias[0] ? tmp_chan->alias : tmp_chan->name, CHAN_NAME_LENGTH);
	if (!tmp_chan->pass[0])
		channel->pass[0] = '\0';
	else
		safestrncpy(channel->pass, tmp_chan->pass, CHAN_NAME_LENGTH);
	channel->msg_delay = tmp_chan->msg_delay;

	switch (channel->type) {
		case CHAN_TYPE_MAP:
			channel->m = tmp_chan->m;
			break;
		case CHAN_TYPE_ALLY:
			channel->gid = tmp_chan->gid;
			break;
		case CHAN_TYPE_PRIVATE:
			channel->char_id = tmp_chan->char_id;
		default:
			strdb_put(channel_db, channel->name, channel);
			break;
	}

	if (battle_config.etc_log)
		ShowInfo("Create channel %s alias %s type=%d, owner=%d/%d/%d\n",channel->name,channel->alias,channel->type,channel->char_id,channel->m,channel->gid);

	return channel;
}

/**
 * Allocates the proper settings depending on the channel type
 * @param name: Channel name, can't be null
 * @param pass: Channel password, can be null
 * @param chantype: Channel type
 * @param owner: Owner ID
 * @return NULL on failure or Channel on success
 */
struct Channel* channel_create_simple(char *name, char *pass, enum Channel_Type chantype, unsigned int owner) {
	struct Channel tmp_chan;
	memset(&tmp_chan, 0, sizeof(struct Channel));

	switch (chantype) {
		case CHAN_TYPE_ALLY:
			memcpy(&tmp_chan, &channel_config.ally_tmpl, sizeof(channel_config.ally_tmpl));
			tmp_chan.gid = (int)owner;
			break;
		case CHAN_TYPE_MAP:
			memcpy(&tmp_chan, &channel_config.map_tmpl, sizeof(channel_config.map_tmpl));
			tmp_chan.m = (uint16)owner;
			break;
		case CHAN_TYPE_PUBLIC:
			safestrncpy(tmp_chan.name, name+1, sizeof(tmp_chan.name));
			if (pass)
				safestrncpy(tmp_chan.pass, pass, sizeof(tmp_chan.pass));
			else
				tmp_chan.pass[0] = '\0';
			safestrncpy(tmp_chan.alias, name, sizeof(tmp_chan.name));
			tmp_chan.type = CHAN_TYPE_PUBLIC;
			tmp_chan.opt = CHAN_OPT_BASE;
			tmp_chan.msg_delay = 1000;
			tmp_chan.color = channel_getColor("Default");
			break;
		case CHAN_TYPE_PRIVATE:
			safestrncpy(tmp_chan.name, name+1, sizeof(tmp_chan.name));
			if (pass)
				safestrncpy(tmp_chan.pass, pass, sizeof(tmp_chan.pass));
			else
				tmp_chan.pass[0] = '\0';
			safestrncpy(tmp_chan.alias, name, sizeof(tmp_chan.name));
			tmp_chan.type = CHAN_TYPE_PRIVATE;
			tmp_chan.opt = channel_config.private_channel.opt;
			tmp_chan.msg_delay = channel_config.private_channel.delay;
			tmp_chan.color = channel_config.private_channel.color;
			tmp_chan.char_id = owner;
			break;
		default:
			return NULL;
	}

	return channel_create(&tmp_chan);
}

/**
 * Delete a channel
 * - Checks if there is any user in channel and make them quit
 * @param channel: Channel data
 * @param force: Forcefully remove channel
 * @return
 *   0: Success
 *  -1: Invalid channel
 *  -2: Can't delete now
 */
int channel_delete(struct Channel *channel, bool force) {
	if(!channel)
		return -1;
	if(!force && channel->type == CHAN_TYPE_PUBLIC && runflag == MAPSERVER_ST_RUNNING) //only delete those serv stop
		return -2;
	if( db_size(channel->users)) {
		struct map_session_data *sd;
		DBIterator *iter = db_iterator(channel->users);
		for( sd = (struct map_session_data *)dbi_first(iter); dbi_exists(iter); sd = (struct map_session_data *)dbi_next(iter) ) { //for all users
			channel_clean(channel,sd,1); //make all quit
		}
		dbi_destroy(iter);
	}
	if (battle_config.etc_log)
		ShowInfo("Deleting channel %s alias %s type %d\n",channel->name,channel->alias,channel->type);
	db_destroy(channel->users);
	db_destroy(channel->banned);
	if (channel->groups)
		aFree(channel->groups);
	channel->groups = NULL;
	channel->group_count = 0;
	switch(channel->type){
	case CHAN_TYPE_MAP:
		map_getmapdata(channel->m)->channel = NULL;
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

/**
 * Make a player join a channel
 * - Add player to channel user list
 * - Add channel to user channel list
 * @param channel: Channel data
 * @param sd: Player data
 * @return
 *   0: Success
 *  -1: Invalid channel or player
 *  -2: Player already in channel
 *  -3: Player banned
 *  -4: Reached max limit
 */
int channel_join(struct Channel *channel, struct map_session_data *sd) {
	if(!channel || !sd)
		return -1;
	if(sd->state.autotrade)
		return 0; // fake success
	if(channel_haspc(channel,sd)==1)
		return -2;
	if (!pc_has_permission(sd, PC_PERM_CHANNEL_ADMIN) && !channel_pccheckgroup(channel, sd->group_id))
		return -2;

	if(channel_haspcbanned(channel,sd)==1){
		char output[CHAT_SIZE_MAX];
		sprintf(output, msg_txt(sd,1438),channel->name); //You're currently banned from the '%s' channel.
		clif_displaymessage(sd->fd, output);
		return -3;
	}

	if (channel->type == CHAN_TYPE_PRIVATE && db_size(channel->users) >= channel_config.private_channel.max_member) {
		char output[CHAT_SIZE_MAX];
		sprintf(output, msg_txt(sd,760), channel->name, channel_config.private_channel.max_member); // You cannot join channel '%s'. Limit of %d has been met.
		clif_displaymessage(sd->fd, output);
		return -4;
	}

	RECREATE(sd->channels, struct Channel *, ++sd->channel_count);
	sd->channels[ sd->channel_count - 1 ] = channel;
	idb_put(channel->users, sd->status.char_id, sd);
	RECREATE(sd->channel_tick, unsigned int, sd->channel_count);
	sd->channel_tick[sd->channel_count-1] = 0;

	if( sd->stealth ) {
		sd->stealth = false;
	} else if( channel->opt & CHAN_OPT_ANNOUNCE_JOIN ) {
		char output[CHAT_SIZE_MAX];
		safesnprintf(output, CHAT_SIZE_MAX, msg_txt(sd,761), channel->alias, sd->status.name); // %s %s has joined.
		clif_channel_msg(channel,output,channel->color);
	}

	/* someone is cheating, we kindly disconnect the bastard */
	if( sd->channel_count > 200 ) {
		set_eof(sd->fd);
	}

	return 0;
}

/**
 * Make a player join the map channel
 * - Create the map_channel if it does not exist
 * @param sd: Player data
 * @return
 *  -1: Invalid player
 *  -2: Player already in channel (channel_join)
 *  -3: Player banned (channel_join)
 */
int channel_mjoin(struct map_session_data *sd) {
	char mout[60];
	if(!sd) return -1;

	struct map_data *mapdata = map_getmapdata(sd->bl.m);

	if( !mapdata->channel ) {
		mapdata->channel = channel_create_simple(NULL,NULL,CHAN_TYPE_MAP,sd->bl.m);
	}

	if( mapdata->channel->opt & CHAN_OPT_ANNOUNCE_SELF ) {
		sprintf(mout, msg_txt(sd,1435),mapdata->channel->name,mapdata->name); // You're now in the '#%s' channel for '%s'.
		clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], mout, false, SELF);
	}

	return channel_join(mapdata->channel,sd);
}

/**
 * Make all ally members of a guild join the guild channel
 * - They only join if they are in their own guild channel (if not they probably left it)
 * @param g: Guild data
 * @return
 *   0: Success
 *  -1: Invalid guild or no channel for guild
 */
int channel_ajoin(struct guild *g){
	int i, j;
	struct map_session_data *pl_sd;

	if(!g || !g->channel) return -1;
	for (i = 0; i < MAX_GUILDALLIANCE; i++){
		struct guild *ag; //allied guld
		struct guild_alliance *ga = &g->alliance[i]; //guild alliance
		if(ga->guild_id && (ga->opposition==0) && (ag=guild_search(ga->guild_id))){
			for (j = 0; j < ag->max_member; j++){ //load all guildmember
				pl_sd = ag->member[j].sd;
				if(channel_haspc(ag->channel,pl_sd)==1)  //only if they are in their own guildchan
					channel_join(g->channel,pl_sd);
			}
		}
	}
	return 0;
}

/**
 * Make a player join the guild channel
 * - Create a guild channel if it does not exist
 * @param sd: Player data
 * @param flag: Join type (1 - Guild chat, 2 - Ally chat)
 * @return
 *   0: Success
 *  -1: Invalid player
 *  -2: Player has no guild attached
 */
int channel_gjoin(struct map_session_data *sd, int flag){
	struct Channel *channel;
	struct guild *g;

	if(!sd || sd->state.autotrade) return -1;
	g = sd->guild;
	if(!g) return -2;

	channel = g->channel;
	if(!channel){
		channel = channel_create_simple(NULL,NULL,CHAN_TYPE_ALLY,g->guild_id);
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

/**
 * Make player leave the channel and cleanup association
 * - If no one remains in the chat, delete it
 * @param channel: Channel data
 * @param sd: Player data
 * @param flag: Called from deletion process, do not recall delete
 * @return
 *  0: Success
 * -1: Invalid player or channel
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
		sd->channel_tick[i] = 0;
		for(; i < sd->channel_count; i++ ) { //slice move list down
			if( sd->channels[i] == NULL )
				continue;
			if(i != cursor) {
				sd->channels[cursor] = sd->channels[i];
				sd->channel_tick[cursor] = sd->channel_tick[i];
			}
			cursor++;
		}
		if ( !(sd->channel_count = cursor) ) { //if in no more chan delete db
			aFree(sd->channels);
			aFree(sd->channel_tick);
			sd->channels = NULL;
			sd->channel_tick = NULL;
		}
	}

	idb_remove(channel->users,sd->status.char_id); //remove user for channel user list
	//auto delete when no more user in
	if( !db_size(channel->users) && !(flag&1) )
		channel_delete(channel,false);

	return 0;
}

/**
 * Make a player leave a type of channel
 * @param sd: Player data
 * @param type: Quit type (1 - Quit guild channel, 2 - Quit ally channel, 4 - Quit map channel, 8 - Quit all users in channel)
 * @return
 *  0: Success
 * -1: Invalid player
 */
int channel_pcquit(struct map_session_data *sd, int type){
	int i;

	//On closing state we could have clean all chan by sd but pcquit is more used to free unit when
	//he quit a map_server, not call in map_quit cause we need to cleanup when we switch map-server as well
	if(!sd) return -1;

	// Leave all chat channels.
	if(type&(1|2) && channel_config.ally_tmpl.name[0] && sd->guild){ //quit guild and ally chan
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

	struct map_data *mapdata = map_getmapdata(sd->bl.m);

	if(type&4 && channel_config.map_tmpl.name[0] && channel_haspc(mapdata->channel,sd)==1){ //quit map chan
		channel_clean(mapdata->channel,sd,0);
	}
	if(type&8 && sd->channel_count ) { //quit all chan
		uint8 count = sd->channel_count;
		for( i = count-1; i >= 0; i--) { //going backward to avoid shifting
			channel_clean(sd->channels[i],sd,0);
		}
	}
	return 0;
}

/**
 * Format message from player to send to the channel
 * - Also truncate extra characters if message is too long
 * @param channel: Channel data
 * @param sd: Player data
 * @param msg: Message to send
 * @return
 *  0: Success
 * -1: Invalid player, channel, or message
 * -2: Delay message from last message
 */
int channel_send(struct Channel *channel, struct map_session_data *sd, const char *msg) {
	int idx = 0;

	if(!channel || !sd || !msg || (idx = channel_pc_haschan(sd, channel)) < 0)
		return -1;

	if(!pc_has_permission(sd, PC_PERM_CHANNEL_ADMIN) && channel->msg_delay != 0 && DIFF_TICK(sd->channel_tick[idx] + channel->msg_delay, gettick()) > 0) {
		clif_messagecolor(&sd->bl,color_table[COLOR_RED],msg_txt(sd,1455),false,SELF); //You're talking too fast!
		return -2;
	}
	else {
		char output[CHAT_SIZE_MAX];
		unsigned long color = channel->color;
		if((channel->opt&CHAN_OPT_COLOR_OVERRIDE) && sd->fontcolor && sd->fontcolor < channel_config.colors_count && channel_config.colors[sd->fontcolor])
			color = channel_config.colors[sd->fontcolor];
		safesnprintf(output, CHAT_SIZE_MAX, "%s %s : %s", channel->alias, sd->status.name, msg);
		clif_channel_msg(channel,output,color);
		sd->channel_tick[idx] = gettick();
	}
	return 0;
}

/**
 * Check parameters for channel creation
 * @param chname: Channel name
 * @param chpass: Channel password
 * @param type: Check types (1 - Check name # + length, 2 - Check if channel already exists, 4 - Check password length)
 * @return
 *  0: Success
 * -1: Bad channel name
 * -2: bad channel name length
 * -3: Password given too long
 * -4: Channel already exists
 */
int channel_chk(char *chname, char *chpass, int type){
	if(type&1){ //check name
		if( chname[0] != '#' )
			return -1;
		if ( strlen(chname) < 3 || strlen(chname) > CHAN_NAME_LENGTH )
			return -2;
		if( (type&2) && (
			strcmpi(chname + 1,channel_config.map_tmpl.name) == 0
			|| strcmpi(chname + 1,channel_config.ally_tmpl.name) == 0
			|| strdb_exists(channel_db, chname + 1) )
			) {
			return -4;
		}
	}
	if (type&4 && (chpass[0] != '\0' && strlen(chpass) > CHAN_NAME_LENGTH ) ) {
		return -3;
	}

	return 0;
}

/**
 * Lookup a channel name
 * @param chname: Channel name
 * @param sd: Player data, can be NULL, used to solve #map and #ally cases
 * @param flag: Lookup types (1 - Create channel if it does not exist (map or ally only), 2 - Join the channel if not joined yet (map or ally only))
 * @return NULL on channel not found or channel data on success
 */
struct Channel* channel_name2channel(char *chname, struct map_session_data *sd, int flag){
	if(channel_chk(chname, NULL, 1))
		return NULL;

	struct map_data *mapdata = sd ? map_getmapdata(sd->bl.m) : NULL;

	if(sd && strcmpi(chname + 1,channel_config.map_tmpl.name) == 0){
		if(flag&1 && !mapdata->channel)
			mapdata->channel = channel_create_simple(NULL,NULL,CHAN_TYPE_MAP,sd->bl.m);
		if(flag&2 && channel_pc_haschan(sd,mapdata->channel) < 1)
			channel_mjoin(sd);
		return mapdata->channel;
	}
	else if(sd && (strcmpi(chname + 1,channel_config.ally_tmpl.name) == 0) && sd->guild){
		if(flag&1 && !sd->guild->channel)
			sd->guild->channel = channel_create_simple(NULL,NULL,CHAN_TYPE_ALLY,sd->guild->guild_id);
		if(flag&2 && channel_pc_haschan(sd,mapdata->channel) < 1)
			channel_gjoin(sd,3);
		return sd->guild->channel;
	}
	else
		return (struct Channel*) strdb_get(channel_db, chname + 1);
}

/**
 * Check if player is in a channel
 * @param channel: Channel data
 * @param sd: Player data
 * @return
 * -1: Invalid player or channel
 *  0: Player not found or not banned
 *  1: Player is in channel
 */
int channel_haspc(struct Channel *channel,struct map_session_data *sd){
	if(!channel || !sd) return -1;
	return (idb_exists(channel->users, sd->status.char_id))?1:0;
}

/**
 * Check if player is banned from channel
 * @return
 * -1: Invalid player or channel
 *  0: Player not found or not banned
 *  1: Player is banned
 */
int channel_haspcbanned(struct Channel *channel,struct map_session_data *sd){
	if(!channel || !sd) return -1;
	return (idb_exists(channel->banned, sd->status.char_id))?1:0;
}


/**
 * Check if player has channel in their channel list
 * @param sd: Player data
 * @param channel: Channel data
 * @return
 * -1: Invalid channel or player
 * -2: Player not found or not in channel
 * x > 0: has_channel at index x
 */
int channel_pc_haschan(struct map_session_data *sd, struct Channel *channel){
	int k;
	if(!channel || !sd) return -1; //channel or player doesn't exist
	ARR_FIND(0, sd->channel_count, k, strcmpi(channel->name,sd->channels[k]->name) == 0);
	if( k >= sd->channel_count ) return -2;
	return k;
}

/**
 * Display some information to users in channel
 * @param sd: Player data
 * @param options:
 *   colors: Display available colors for channel system
 *   mine: List of players in channel and number of users
 *   void: List of public channel and map and guild and number of users
 * @return 0 on success or -1 on failure
 */
int channel_display_list(struct map_session_data *sd, const char *options){

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
				clif_messagecolor(&sd->bl,channel_config.colors[k],msg,false,SELF);
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
				char output[CHAT_SIZE_MAX];
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
		char output[CHAT_SIZE_MAX];
		struct map_data *mapdata = map_getmapdata(sd->bl.m);

		clif_displaymessage(sd->fd, msg_txt(sd,1410)); // ---- Public Channels ----
		if( channel_config.map_tmpl.name[0] && mapdata->channel ) {
			sprintf(output, msg_txt(sd,1409), mapdata->channel->name, db_size(mapdata->channel->users));// - #%s (%d users)
			clif_displaymessage(sd->fd, output);
		}
		if( channel_config.ally_tmpl.name[0] && sd->status.guild_id ) {
			struct guild *g = sd->guild;
			if (g && g->channel) {
				sprintf(output, msg_txt(sd,1409), g->channel->name, db_size(((struct Channel *)g->channel)->users));// - #%s (%d users)
				clif_displaymessage(sd->fd, output);
			}
		}
		iter = db_iterator(channel_db);
		for(channel = (struct Channel *)dbi_first(iter); dbi_exists(iter); channel = (struct Channel *)dbi_next(iter)) {
			if (!has_perm && !channel_pccheckgroup(channel, sd->group_id))
				continue;
			if( has_perm || channel->type == CHAN_TYPE_PUBLIC ) {
				sprintf(output, msg_txt(sd,1409), channel->name, db_size(channel->users));// - #%s (%d users)
				clif_displaymessage(sd->fd, output);
			}
		}
		dbi_destroy(iter);
	}

	return 0;
}

/**
 * A player is attempting to create a channel
 * @param sd: Player data
 * @param chname: Channel name
 * @param chpass: Channel password
 * @return 0 on success or -1 on failure
 */
int channel_pccreate(struct map_session_data *sd, char *chname, char *chpass){
	char output[CHAT_SIZE_MAX];
	int8 res;

	if(!sd || !chname)
		return 0;

	res = channel_chk(chname,chpass,7);
	if(res==0){ //success
		struct Channel *channel = channel_create_simple(chname,chpass,CHAN_TYPE_PRIVATE,sd->status.char_id);
		channel_join(channel,sd);
		if( ( channel->opt & CHAN_OPT_ANNOUNCE_SELF ) ) {
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

/**
 * A player is attempting to delete a channel
 * @param sd: Player data
 * @param chname: Channel name
 * @return 0 on success or -1 on failure
 */
int channel_pcdelete(struct map_session_data *sd, char *chname){
	struct Channel *channel;
	char output[CHAT_SIZE_MAX];

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
	channel_delete(channel,false);

	sprintf(output, msg_txt(sd,1448),chname); // Channel '%s' deleted.
	clif_displaymessage(sd->fd, output);

	return 0;
}

/**
 * A player is attempting to leave a channel
 * @param sd: Player data
 * @param chname: Channel name
 * @return 0 on success or -1 on failure
 */
int channel_pcleave(struct map_session_data *sd, char *chname){
	struct Channel *channel;
	char output[CHAT_SIZE_MAX];

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

	if (!(channel->opt&CHAN_OPT_CAN_LEAVE)) {
		sprintf(output, msg_txt(sd,762), chname); // You cannot leave channel '%s'.
		clif_displaymessage(sd->fd, output);
		return -1;
	}

	if( !channel_config.closing && (channel->opt & CHAN_OPT_ANNOUNCE_LEAVE) ) {
		safesnprintf(output, CHAT_SIZE_MAX, msg_txt(sd,763), channel->alias, sd->status.name); // %s %s left.
		clif_channel_msg(channel,output,channel->color);
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

/**
 * A player is attempting to join a channel
 * @param sd: Player data
 * @param chname: Channel name
 * @param pass: Channel password
 * @return 0 on success or -1 on failure
 */
int channel_pcjoin(struct map_session_data *sd, char *chname, char *pass){
	struct Channel *channel;
	char output[CHAT_SIZE_MAX];

	if(!sd || !chname)
		return 0;

	if( channel_chk(chname,NULL,1) ) {
		clif_displaymessage(sd->fd, msg_txt(sd,1405));// Channel name must start with '#'.
		return -1;
	}

	channel = channel_name2channel(chname,sd,1);
	if(channel){
		if (!pc_has_permission(sd, PC_PERM_CHANNEL_ADMIN) && !channel_pccheckgroup(channel, sd->group_id)) {
			sprintf(output, msg_txt(sd,1407), chname); // Channel '%s' is not available.
			clif_displaymessage(sd->fd, output);
			return -1;
		}
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

	switch(channel->type){
	case CHAN_TYPE_ALLY: channel_gjoin(sd,3); break;
	case CHAN_TYPE_MAP: channel_mjoin(sd); break;
	default: //private and public atm
		if (channel_join(channel,sd) != 0)
			return -1;
	}

	if( ( channel->opt & CHAN_OPT_ANNOUNCE_SELF ) ) {
		sprintf(output, msg_txt(sd,1403),chname); // You're now in the '%s' channel.
		clif_displaymessage(sd->fd, output);
	}

	return 0;
}

/**
 * A player is attempting to change the channel color
 * @param sd: Player data
 * @param chname: Channel name
 * @param color: New color
 * @return 0 on success or -1 on failure
 */
int channel_pccolor(struct map_session_data *sd, char *chname, char *color){
	struct Channel *channel;
	char output[CHAT_SIZE_MAX];
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

	if( !pc_has_permission(sd, PC_PERM_CHANNEL_ADMIN) ) {
		if (channel->char_id != sd->status.char_id) {
			sprintf(output, msg_txt(sd,1412), chname);// You're not the owner of channel '%s'.
			clif_displaymessage(sd->fd, output);
			return -1;
		}
		else if (!(channel->opt&CHAN_OPT_COLOR_OVERRIDE)) {
			sprintf(output, msg_txt(sd,764), chname); // You cannot change the color for channel '%s'.
			clif_displaymessage(sd->fd, output);
			return -1;
		}
	}

	ARR_FIND(0,channel_config.colors_count,k,( strcmpi(color,channel_config.colors_name[k]) == 0 ) );
	if( k >= channel_config.colors_count ) {
		sprintf(output, msg_txt(sd,1411), color);// Unknown color '%s'.
		clif_displaymessage(sd->fd, output);
		return -1;
	}
	channel->color = channel_config.colors[k];
	sprintf(output, msg_txt(sd,1413),chname,channel_config.colors_name[k]);// '%s' channel color updated to '%s'.
	clif_displaymessage(sd->fd, output);
	return 0;
}

/**
 * A player is attempting to bind
 * - Makes default message output display to the channel
 * @param sd: Player data
 * @param chname: Channel name
 * @return 0 on success, -1 on incorrect channel name, or -2 channel doesn't exist or player didn't join
 */
int channel_pcbind(struct map_session_data *sd, char *chname){
	struct Channel *channel;
	char output[CHAT_SIZE_MAX];

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

/**
 * A player is attempting to unbind
 * @param sd: Player data
 * @return 0 on success or -1 on failure
 */
int channel_pcunbind(struct map_session_data *sd){
	char output[CHAT_SIZE_MAX];

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

/**
 * A player is attempting to modify the banlist
 * @param sd: Player data
 * @param chname: Channel name
 * @param pname: Player to ban or unban
 * @param flag: Ban options (0 - Ban, 1 - Unban, 2 - Unban all, 3 - Ban list)
 * @return 0 on success or -1 on failure
 */
int channel_pcban(struct map_session_data *sd, char *chname, char *pname, int flag){
	struct Channel *channel;
	char output[CHAT_SIZE_MAX];
	struct map_session_data *tsd = map_nick2sd(pname,false);

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

	if( !pc_has_permission(sd, PC_PERM_CHANNEL_ADMIN) ) {
		if (channel->char_id != sd->status.char_id) {
			sprintf(output, msg_txt(sd,1412), chname);// You're not the owner of channel '%s'.
			clif_displaymessage(sd->fd, output);
			return -1;
		} else if (!channel_config.private_channel.ban) {
			sprintf(output, msg_txt(sd,765), chname); // You're not allowed to ban a player.
			clif_displaymessage(sd->fd, output);
			return -1;
		}
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
			return 0;
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
		for( cbe = (struct chan_banentry *)dbi_first(iter); dbi_exists(iter); cbe = (struct chan_banentry *)dbi_next(iter) ) { //for all users
			if (cbe->char_name[0])
				sprintf(output, "%d: %s",cbe->char_id,cbe->char_name);
			else
				sprintf(output, "%d: ****",cbe->char_id);
			clif_displaymessage(sd->fd, output);
		}
		dbi_destroy(iter);
		}
		return 0;
	}
	clif_displaymessage(sd->fd, output);

	return 0;
}

/**
 * A player is attemting to kick a player
 * @param sd: Player data
 * @param chname: Channel name
 * @param pname: Player name to kick
 * @return 0 on success or -1 on failure
 */
int channel_pckick(struct map_session_data *sd, char *chname, char *pname) {
	struct Channel *channel;
	char output[CHAT_SIZE_MAX];
	struct map_session_data *tsd = map_nick2sd(pname,false);

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

	if (!tsd) {
		clif_displaymessage(sd->fd, msg_txt(sd,3));
		return -1;
	}

	if( !pc_has_permission(sd, PC_PERM_CHANNEL_ADMIN) ) {
		if (channel->char_id != sd->status.char_id) {
			sprintf(output, msg_txt(sd,1412), chname);// You're not the owner of channel '%s'.
			clif_displaymessage(sd->fd, output);
		} else if (!channel_config.private_channel.kick) {
			sprintf(output, msg_txt(sd,766), chname); // You cannot kick a player from channel '%s'.
			clif_displaymessage(sd->fd, output);
		}
		return -1;
	}

	if (channel_pc_haschan(sd, channel) < 0) {
		sprintf(output, msg_txt(sd,1425), chname); // You're not part of the '%s' channel.
		clif_displaymessage(sd->fd, output);
		return -1;
	}

	if (channel->char_id == sd->status.char_id) {
		clif_displaymessage(sd->fd, msg_txt(sd, 767)); // You're not allowed to kick a player.
		return -1;
	}

	if( !channel_config.closing && (channel->opt & CHAN_OPT_ANNOUNCE_LEAVE) ) {
		safesnprintf(output, CHAT_SIZE_MAX, msg_txt(sd,768), channel->alias, tsd->status.name); // %s %s has been kicked.
		clif_channel_msg(channel,output,channel->color);
	}

	switch(channel->type){
		case CHAN_TYPE_ALLY: channel_pcquit(tsd,3); break;
		case CHAN_TYPE_MAP: channel_pcquit(tsd,4); break;
		default: //private and public atm
			channel_clean(channel,tsd,0);
	}

	return 1;
}

/**
 * A player is attempting to set an option on the channel
 * @param sd: Player data
 * @param chname: Channel name
 * @param option: Option to change
 * @param val: Option value
 * @return 0 on success or -1 on failure
 */
int channel_pcsetopt(struct map_session_data *sd, char *chname, const char *option, const char *val){
	struct Channel *channel;
	char output[CHAT_SIZE_MAX];
	int k, s = 0, opt;
	const char* opt_str[] = {
		"None",
		"SelfAnnounce",
		"JoinAnnounce",
		"LeaveAnnounce",
		"MessageDelay",
		"ColorOverride",
		"CanChat",
		"CanLeave",
		"Autojoin",
	};

	if( channel_chk(chname,NULL,1) ) {
		clif_displaymessage(sd->fd, msg_txt(sd,1405));// Channel name must start with '#'.
		return -1;
	}

	if (!sd)
		return - 1;

	channel = channel_name2channel(chname,sd,0);
	if( !channel ) {
		sprintf(output, msg_txt(sd,1407), chname);// Channel '%s' is not available.
		clif_displaymessage(sd->fd, output);
		return -1;
	}

	if( sd && channel->char_id != sd->status.char_id && !pc_has_permission(sd, PC_PERM_CHANNEL_ADMIN) ) {
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

	opt = 1<<(k-1);

	if (channel->type == CHAN_TYPE_PRIVATE && !pc_has_permission(sd, PC_PERM_CHANNEL_ADMIN)) {
		switch (opt) {
			case CHAN_OPT_MSG_DELAY:
				if (!channel_config.private_channel.change_delay)
					return -1;
				break;
			case CHAN_OPT_COLOR_OVERRIDE:
				if (!channel_config.private_channel.color_override)
					return -1;
				break;
		}
	}

	if( val[0] == '\0' ) {
		if ( opt == CHAN_OPT_MSG_DELAY ) {
			sprintf(output, msg_txt(sd,1466), opt_str[k]);// Input the number of seconds (0-10) for the '%s' option.
			clif_displaymessage(sd->fd, output);
			return -1;
		} else if( channel->opt & opt ) {
			sprintf(output, msg_txt(sd,1449), opt_str[k],opt_str[k]); // Option '%s' is already enabled (use '@channel setopt %s 0' to disable).
			clif_displaymessage(sd->fd, output);
			return -1;
		} else {
			channel->opt |= opt;
			sprintf(output, msg_txt(sd,1450), opt_str[k],channel->name);// Option '%s' is enabled for channel '#%s'.
			clif_displaymessage(sd->fd, output);
		}
	} else {
		int v = atoi(val);
		if( opt == CHAN_OPT_MSG_DELAY ) {
			if( v < 0 || v > 10 ) {
				sprintf(output, msg_txt(sd,1451), v, opt_str[k]);// Value '%d' for option '%s' is out of range (limit 0-10).
				clif_displaymessage(sd->fd, output);
				return -1;
			}
			if( v == 0 ) {
				channel->opt &=~ opt;
				channel->msg_delay = 0;
				sprintf(output, msg_txt(sd,1453), opt_str[k],channel->name,v);// Option '%s' is disabled for channel '#%s'.
				clif_displaymessage(sd->fd, output);
			} else {
				channel->opt |= opt;
				channel->msg_delay = v * 1000;
				sprintf(output, msg_txt(sd,1452), opt_str[k],channel->name,v);// Option '%s' is enabled for channel '#%s' at %d seconds.
				clif_displaymessage(sd->fd, output);
			}
		} else {
			if( v ) {
				if( channel->opt & opt ) {
					sprintf(output, msg_txt(sd,1449), opt_str[k],opt_str[k]); // Option '%s' is already enabled (use '@channel setopt %s 0' to disable).
					clif_displaymessage(sd->fd, output);
					return -1;
				} else {
					channel->opt |= opt;
					sprintf(output, msg_txt(sd,1450), opt_str[k],channel->name);// Option '%s' is enabled for channel '#%s'.
					clif_displaymessage(sd->fd, output);
				}
			} else {
				if( !(channel->opt & opt) ) {
					sprintf(output, msg_txt(sd,1450), opt_str[k],channel->name); // Option '%s' is enabled for channel '#%s'.
					clif_displaymessage(sd->fd, output);
					return -1;
				} else {
					channel->opt &=~ opt;
					sprintf(output, msg_txt(sd,1453), opt_str[k],channel->name);// Option '%s' is disabled for channel '#%s'.
					clif_displaymessage(sd->fd, output);
				}
			}
		}
	}
	return 0;
}

/**
 * Check if the given group ID can join the channel
 * @param channel: Channel data
 * @param group_id: Group ID to check
 * @return True on success or false on failure
 */
bool channel_pccheckgroup(struct Channel *channel, int group_id) {
	unsigned short i;

	nullpo_ret(channel);

	if (!channel->groups || !channel->group_count)
		return true;

	for (i = 0; i < channel->group_count; i++) {
		if (channel->groups[i] == group_id)
			return true;
	}

	return false;
}

/**
 * Attempt to autojoin a player to a channel
 */
int channel_pcautojoin_sub(DBKey key, DBData *data, va_list ap) {
	struct Channel *channel = (struct Channel *)db_data2ptr(data);
	struct map_session_data *sd = NULL;
	char channame[CHAN_NAME_LENGTH+1];

	nullpo_ret(channel);
	nullpo_ret((sd = va_arg(ap, struct map_session_data *)));

	if (channel->pass[0])
		return 0;
	if (!(channel->opt&CHAN_OPT_AUTOJOIN))
		return 0;
	if (!pc_has_permission(sd, PC_PERM_CHANNEL_ADMIN) && !channel_pccheckgroup(channel, sd->group_id))
		return 0;
	safesnprintf(channame, sizeof(channame), "#%s", channel->name);
	channel_pcjoin(sd, channame, NULL);

	return 1;
}

/**
 * Attempt to autojoin a player to a channel
 * @param sd: Player data
 */
void channel_autojoin(struct map_session_data *sd) {
	nullpo_retv(sd);
	if (sd->state.autotrade || !sd->fd)
		return;
	channel_db->foreach(channel_db, channel_pcautojoin_sub, sd);
}

/**
 * Get color by name or RGB hex code
 * @param color_str: Color name
 * @return color in RGB
 */
unsigned long channel_getColor(const char *color_str) {
	int i;
	for (i = 0; i < channel_config.colors_count; i++) {
		if (strcmpi(channel_config.colors_name[i], color_str) == 0)
			return channel_config.colors[i];
	}
	return channel_config.colors[0];
}

/**
 * Attempt to create a global channel from the channel config
 * @param chan: Channel list
 * @param tmp_chan: Temporary channel data
 * @param i: Index
 * @return True on success or false on failure
 */
bool channel_read_sub(config_setting_t *chan, struct Channel *tmp_chan, uint8 i) {
	config_setting_t  *group_list = NULL;
	int delay = 1000, autojoin = 0, leave = 1, chat = 1, color_override = 0,
		self_notif = 1, join_notif = 0, leave_notif = 0;
	int type = CHAN_TYPE_PUBLIC, group_count = 0;
	const char *name = NULL, *password = NULL, *alias = NULL, *color_str = "Default", *type_str = NULL;

	if (tmp_chan == NULL)
		return false;

	if (!config_setting_lookup_string(chan, "name", &name)) {
		ShowError("Please input channel 'name' at '%s' line '%d'! Skipping...\n", chan->file, chan->line);
		return false;
	}
	if (config_setting_lookup_string(chan, "type", &type_str) && !script_get_constant(type_str, &type)) {
		ShowError("Invalid channel type %s at '%s' line '%d'! Skipping...\n", type_str, chan->file, chan->line);
		return false;
	}
	config_setting_lookup_string(chan, "password", &password);
	config_setting_lookup_string(chan, "alias", &alias);
	config_setting_lookup_string(chan, "color", &color_str);
	config_setting_lookup_int(chan, "delay", &delay);
	config_setting_lookup_bool(chan, "autojoin", &autojoin);
	config_setting_lookup_bool(chan, "leave", &leave);
	config_setting_lookup_bool(chan, "chat", &chat);
	config_setting_lookup_bool(chan, "color_override", &color_override);
	config_setting_lookup_bool(chan, "self_notif", &self_notif);
	config_setting_lookup_bool(chan, "join_notif", &join_notif);
	config_setting_lookup_bool(chan, "leave_notif", &leave_notif);

	safestrncpy(tmp_chan->name,name+1,sizeof(tmp_chan->name));
	if (password)
		safestrncpy(tmp_chan->pass,password,sizeof(tmp_chan->pass));
	else
		tmp_chan->pass[0] = '\0';
	safestrncpy(tmp_chan->alias,alias?alias:name,sizeof(tmp_chan->alias));
	tmp_chan->msg_delay = delay;
	tmp_chan->type = (enum Channel_Type)type;
	tmp_chan->color = channel_getColor(color_str);

	tmp_chan->opt = (autojoin ? CHAN_OPT_AUTOJOIN : 0) |
		(leave ? CHAN_OPT_CAN_LEAVE : 0) |
		(chat ? CHAN_OPT_CAN_CHAT : 0) |
		(color_override ? CHAN_OPT_COLOR_OVERRIDE : 0) |
		(self_notif ? CHAN_OPT_ANNOUNCE_SELF : 0) |
		(join_notif ? CHAN_OPT_ANNOUNCE_JOIN : 0) |
		(leave_notif ? CHAN_OPT_ANNOUNCE_LEAVE : 0);

	if ((group_list = config_setting_get_member(chan, "groupid")) && (group_count = config_setting_length(group_list)) > 0) {
		int j;
		CREATE(tmp_chan->groups, unsigned short, group_count);
		tmp_chan->group_count = group_count;
		for (j = 0; j < group_count; j++) {
			int groupid = config_setting_get_int_elem(group_list, j);
			tmp_chan->groups[j] = groupid;
		}
	}

	return true;
}

/**
 * Read and verify configuration in file
 * Assign table value with value
 */
void channel_read_config(void) {
	config_t channels_conf;
	config_setting_t *chan_setting = NULL;

	if (conf_read_file(&channels_conf, channel_conf)) {
		ShowError("Cannot read file '%s' for channel connfig.\n", channel_conf);
		return;
	}

	chan_setting = config_lookup(&channels_conf, "channel_config");
	if (chan_setting != NULL) {
		config_setting_t *colors, *private_channel = NULL, *channels = NULL;
		int count = 0, channel_count = 0;

		colors = config_setting_get_member(chan_setting, "colors");
		if (colors != NULL) {
			int i, color_count = config_setting_length(colors);

			CREATE(channel_config.colors, unsigned long, color_count);
			CREATE(channel_config.colors_name, char *, color_count);
			for (i = 0; i < color_count; i++) {
				config_setting_t *color = config_setting_get_elem(colors, i);
				CREATE(channel_config.colors_name[i], char, CHAN_NAME_LENGTH);

				safestrncpy(channel_config.colors_name[i], config_setting_name(color), CHAN_NAME_LENGTH);
				channel_config.colors[i] = strtoul(config_setting_get_string_elem(colors,i),NULL,0);
				channel_config.colors[i] = (channel_config.colors[i] & 0x0000FF) << 16 | (channel_config.colors[i] & 0x00FF00) | (channel_config.colors[i] & 0xFF0000) >> 16;//RGB to BGR
			}
			channel_config.colors_count = color_count;
		}

		private_channel = config_setting_get_member(chan_setting, "private_channel");
		if (private_channel != NULL) {
			int allow = 1, ban = 1, kick = 1, color_override = 0, change_delay = 0,
				self_notif = 1, join_notif = 0, leave_notif = 0,
				delay = 1000, max_member = 1000;
			const char *color_str = "Default";

			config_setting_lookup_bool(private_channel, "allow", &allow);
			config_setting_lookup_int(private_channel, "delay", &delay);
			config_setting_lookup_string(private_channel, "color", &color_str);
			config_setting_lookup_int(private_channel, "max_member", &max_member);
			config_setting_lookup_bool(private_channel, "self_notif", &self_notif);
			config_setting_lookup_bool(private_channel, "join_notif", &join_notif);
			config_setting_lookup_bool(private_channel, "leave_notif", &leave_notif);
			config_setting_lookup_bool(private_channel, "ban", &ban);
			config_setting_lookup_bool(private_channel, "kick", &kick);
			config_setting_lookup_bool(private_channel, "color_override", &color_override);
			config_setting_lookup_bool(private_channel, "change_delay", &change_delay);

			channel_config.private_channel.allow = allow;
			channel_config.private_channel.color = channel_getColor(color_str);
			channel_config.private_channel.delay = delay;
			channel_config.private_channel.max_member = min(max_member, UINT16_MAX);
			channel_config.private_channel.ban = ban;
			channel_config.private_channel.kick = kick;
			channel_config.private_channel.color_override = color_override;
			channel_config.private_channel.change_delay = change_delay;
			channel_config.private_channel.opt = CHAN_OPT_CAN_CHAT|CHAN_OPT_CAN_LEAVE|
				(color_override ? CHAN_OPT_COLOR_OVERRIDE : 0) |
				(self_notif ? CHAN_OPT_ANNOUNCE_SELF : 0) |
				(join_notif ? CHAN_OPT_ANNOUNCE_JOIN : 0) |
				(leave_notif ? CHAN_OPT_ANNOUNCE_LEAVE : 0);
		}

		channels = config_setting_get_member(chan_setting, "ally");
		if (channels != NULL) {
			memset(&channel_config.ally_tmpl, 0, sizeof(struct Channel));
			channel_read_sub(channels, &channel_config.ally_tmpl, 0);
		}
		channels = config_setting_get_member(chan_setting, "map");
		if (channels != NULL) {
			memset(&channel_config.map_tmpl, 0, sizeof(struct Channel));
			channel_read_sub(channels, &channel_config.map_tmpl, 0);
		}

		channels = config_setting_get_member(chan_setting, "channels");
		if (channels != NULL && (count = config_setting_length(channels)) > 0) {
			int i;
			for (i = 0; i < count; i++) {
				config_setting_t *chan = config_setting_get_elem(channels, i);
				struct Channel *channel = NULL, tmp_chan;

				memset(&tmp_chan, 0, sizeof(struct Channel));
				if (!channel_read_sub(chan, &tmp_chan, i))
					continue;

				if ((channel = channel_create(&tmp_chan))) {
					channel_count++;
					channel->group_count = tmp_chan.group_count;
					channel->groups = tmp_chan.groups;
				}
			}
		}

		ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' channels in '" CL_WHITE "%s" CL_RESET "'.\n", db_size(channel_db), channel_conf);
		config_destroy(&channels_conf);
	}
}

/**
 * Initialise db and read configuration
 */
void do_init_channel(void) {
	channel_db = stridb_alloc(static_cast<DBOptions>(DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA), CHAN_NAME_LENGTH);
	memset(&channel_config.private_channel, 0, sizeof(struct Channel));
	memset(&channel_config.ally_tmpl, 0, sizeof(struct Channel));
	memset(&channel_config.map_tmpl, 0, sizeof(struct Channel));
	channel_read_config();
}

/**
 * Close all channels and cleanup
 */
void do_final_channel(void) {
	DBIterator *iter;
	struct Channel *channel;
	
	//delete all in remaining chan db
	iter = db_iterator(channel_db);
	for( channel = (struct Channel *)dbi_first(iter); dbi_exists(iter); channel = (struct Channel *)dbi_next(iter) ) {
		channel_delete(channel,false);
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
