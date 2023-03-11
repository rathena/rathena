// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "channel.hpp"

#include <stdlib.h>

#include <common/cbasetypes.hpp>
#include <common/malloc.hpp>
#include <common/nullpo.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp> //set_eof
#include <common/strlib.hpp> //safestrncpy
#include <common/timer.hpp>  // DIFF_TICK
#include "common/utils.hpp"

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
 * Lookup a channel name
 * @param chname: Channel name
 * @param sd: Player data, can be NULL, used to solve #map and #ally cases
 * @param flag: Lookup types (1 - Create channel if it does not exist (map or ally only), 2 - Join the channel if not joined yet (map or ally only))
 * @return NULL on channel not found or channel data on success
 */
struct Channel* channel_name2channel(char *chname, map_session_data *sd, int flag){
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
 * A player is attempting to change the channel color
 * @param sd: Player data
 * @param chname: Channel name
 * @param color: New color
 * @return 0 on success or -1 on failure
 */
int channel_pccolor(map_session_data *sd, char *chname, char *color){
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
		else if (!(channel->opt&CHAN_OPT_CHANGECOLOR)) {
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
int channel_pcbind(map_session_data *sd, char *chname){
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
int channel_pcunbind(map_session_data *sd){
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
int channel_pcban(map_session_data *sd, char *chname, char *pname, int flag){
	struct Channel *channel;
	char output[CHAT_SIZE_MAX];
	map_session_data *tsd = map_nick2sd(pname,false);

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
		} else if (!(channel_config.private_channel.opt & CHAN_OPT_BAN)) {
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
		struct s_chan_banentry *cbe;
		if (!tsd)
			return -1;
		CREATE(cbe, struct s_chan_banentry, 1);
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
		struct s_chan_banentry *cbe;
		sprintf(output, msg_txt(sd,1443), channel->name);// ---- '#%s' Ban List:
		clif_displaymessage(sd->fd, output);
		for( cbe = (struct s_chan_banentry *)dbi_first(iter); dbi_exists(iter); cbe = (struct s_chan_banentry *)dbi_next(iter) ) { //for all users
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
int channel_pckick(map_session_data *sd, char *chname, char *pname) {
	struct Channel *channel;
	char output[CHAT_SIZE_MAX];
	map_session_data *tsd = map_nick2sd(pname,false);

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
		} else if (!(channel_config.private_channel.opt & CHAN_OPT_KICK)) {
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

	if( !channel_config.closing && (channel->opt & CHAN_OPT_LEAVEANNOUNCE) ) {
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
int channel_pcsetopt(map_session_data *sd, char *chname, const char *option, const char *val){
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
			case CHAN_OPT_CHANGEDELAY:
				if (!(channel_config.private_channel.opt & CHAN_OPT_CHANGEDELAY))
					return -1;
				break;
			case CHAN_OPT_CHANGECOLOR:
				if (!(channel_config.private_channel.opt & CHAN_OPT_CHANGECOLOR))
					return -1;
				break;
		}
	}

	if( val[0] == '\0' ) {
		if ( opt == CHAN_OPT_CHANGEDELAY ) {
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
		if( opt == CHAN_OPT_CHANGEDELAY ) {
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
	nullpo_ret(channel);

	return channel->groups.count(group_id) > 0;
}

/**
 * Attempt to autojoin a player to a channel
 */
int channel_pcautojoin_sub(DBKey key, DBData *data, va_list ap) {
	struct Channel *channel = (struct Channel *)db_data2ptr(data);
	map_session_data *sd = NULL;
	char channame[CHAN_NAME_LENGTH+1];

	nullpo_ret(channel);
	nullpo_ret((sd = va_arg(ap, map_session_data *)));

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
void channel_autojoin(map_session_data *sd) {
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
 * Read and verify configuration in file
 * Assign table value with value
 */
void channel_read_config(void) {

	channel_config.private_channel.allow = false;
	channel_config.private_channel.color = 0xFF'FF'FF;
	channel_config.private_channel.delay = 1000;
	channel_config.private_channel.max_member = 1000;
	channel_config.private_channel.opt = CHAN_OPT_BAN | CHAN_OPT_KICK;

	channel_config.ally_tmpl.color = 0xFF'FF'FF;
	channel_config.ally_tmpl.msg_delay = 1000;
	channel_config.ally_tmpl.opt = CHAN_OPT_CANLEAVE | CHAN_OPT_CANCHAT;

	channel_config.map_tmpl.color = 0xFF'FF'FF;
	channel_config.map_tmpl.msg_delay = 1000;
	channel_config.map_tmpl.opt = CHAN_OPT_CANLEAVE | CHAN_OPT_CANCHAT;

	channel_cl.load();

	ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' channels.\n", db_size(channel_db));
}

/**
 * Initialise db and read configuration
 */
void do_init_channel(void) {
	channel_db = stridb_alloc(static_cast<DBOptions>(DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA), CHAN_NAME_LENGTH);
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

