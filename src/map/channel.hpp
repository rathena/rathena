// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "../common/cbasetypes.hpp"
#include "../common/mmo.hpp"

//namespace rA {

struct map_session_data;
struct guild;
struct DBMap;

#define CHAN_NAME_LENGTH 20
#define CHAN_MSG_LENGTH 150

enum Channel_Opt {
	CHAN_OPT_NONE		    = 0,	///< None
	CHAN_OPT_ANNOUNCE_SELF  = 0x01,	///< Shows info when player joined/left channel to self
	CHAN_OPT_ANNOUNCE_JOIN  = 0x02,	///< Shows info if player joined the channel
	CHAN_OPT_ANNOUNCE_LEAVE = 0x04,	///< Shows info if player left the channel
	CHAN_OPT_MSG_DELAY	    = 0x08,	///< Enables chat delay
	CHAN_OPT_COLOR_OVERRIDE = 0x10,	///< Enables color channel be override by player's font color
	CHAN_OPT_CAN_CHAT		= 0x20,	///< Allows player to chat in the channel
	CHAN_OPT_CAN_LEAVE		= 0x40,	///< Allows player to leave the channel
	CHAN_OPT_AUTOJOIN		= 0x80,	///< Player will be autojoined to the channel

	CHAN_OPT_BASE = CHAN_OPT_ANNOUNCE_SELF|CHAN_OPT_MSG_DELAY|CHAN_OPT_CAN_CHAT|CHAN_OPT_CAN_LEAVE,
};

enum Channel_Type {
	CHAN_TYPE_PUBLIC  = 0, ///< Config file made
	CHAN_TYPE_PRIVATE = 1, ///< User's channel
	CHAN_TYPE_MAP	  = 2, ///< Local map
	CHAN_TYPE_ALLY	  = 3, ///< Guild + its alliance
};

struct Channel {
	//unsigned short id;			  ///< Channel ID (unused yet)
	char name[CHAN_NAME_LENGTH];  ///< Channel Name
	char pass[CHAN_NAME_LENGTH];  ///< Password
	char alias[CHAN_NAME_LENGTH]; ///< Channel display name
	enum Channel_Type type;		  ///< Channel type @see enum Channel_Type
	unsigned long color;		  ///< Channel color in BGR
	unsigned char opt;			  ///< Channel options @see enum Channel_Opt
	unsigned short msg_delay;	  ///< Chat delay in miliseconds
	unsigned int char_id;		  ///< If CHAN_TYPE_PRIVATE, owner is char_id of channel creator
	uint16 m;					  ///< If CHAN_TYPE_MAP, owner is map id
	int gid;					  ///< If CHAN_TYPE_ALLY, owner is first logged guild_id
	DBMap *users;				  ///< List of users
	DBMap *banned;				  ///< List of banned chars -> char_id
	unsigned short group_count;	  ///< Number of group id
	unsigned short *groups;		  ///< List of group id, only these groups can join the channel
};

struct chan_banentry {
	uint32 char_id;
	char char_name[NAME_LENGTH];
};
extern chan_banentry chan_banentry;

struct Channel_Config {
	unsigned long *colors;		///< List of available colors
	char **colors_name;			///< Name list of available colors
	unsigned char colors_count;	///< Number of available colors

	/// Private channel default configs
	struct {
		unsigned char opt;			 ///< Options @see enum Channel_Opt
		unsigned long color;		 ///< Default color
		unsigned int delay;			 ///< Message delay
		unsigned short max_member;	 ///< Max member for each channel
		unsigned allow : 1;			 ///< Allow private channel creation?
		unsigned ban : 1;			 ///< Allow player to ban
		unsigned kick : 1;			 ///< Allow player to kick
		unsigned color_override : 1; ///< Owner cannot change the color_override
		unsigned change_delay : 1;	 ///< Owner cannot change the delay
	} private_channel;

	struct Channel map_tmpl;  ///< Map channel default config
	struct Channel ally_tmpl; ///< Alliance channel default config

	bool closing; ///< Server is closing
};
extern struct Channel_Config channel_config;

DBMap* channel_get_db(void);

struct Channel* channel_create(struct Channel *tmp_chan);
struct Channel* channel_create_simple(char *name, char *pass, enum Channel_Type chantype, unsigned int owner);
int channel_delete(struct Channel *channel, bool force);

int channel_join(struct Channel *channel, struct map_session_data *sd);
int channel_mjoin(struct map_session_data *sd);
int channel_gjoin(struct map_session_data *sd, int flag);
int channel_ajoin(struct guild *g);
int channel_clean(struct Channel *channel, struct map_session_data *sd, int flag);
int channel_pcquit(struct map_session_data *sd, int type);

unsigned long channel_getColor(const char *color_str);

int channel_send(struct Channel *channel, struct map_session_data *sd, const char *msg);
void channel_read_config(void);

int channel_chk(char *name, char *pass, int type);
struct Channel* channel_name2channel(char *chname, struct map_session_data *sd, int flag);
int channel_haspc(struct Channel *channel,struct map_session_data *sd);
int channel_haspcbanned(struct Channel *channel,struct map_session_data *sd);
int channel_pc_haschan(struct map_session_data *sd, struct Channel *channel);
int channel_display_list(struct map_session_data *sd, const char *option);

void channel_autojoin(struct map_session_data *sd);
bool channel_pccheckgroup(struct Channel *channel, int group_id);

int channel_pccreate(struct map_session_data *sd, char *chname, char *pass);
int channel_pcdelete(struct map_session_data *sd, char *chname);
int channel_pcjoin(struct map_session_data *sd, char *chname, char *pass);
int channel_pcleave(struct map_session_data *sd, char *chname);
int channel_pccolor(struct map_session_data *sd, char *chname, char *color);
int channel_pcbind(struct map_session_data *sd, char *chname);
int channel_pcunbind(struct map_session_data *sd);
int channel_pcban(struct map_session_data *sd, char *chname, char *pname, int flag);
int channel_pckick(struct map_session_data *sd, char *chname, char *pname);
int channel_pcsetopt(struct map_session_data *sd, char *chname, const char *option, const char *val);

void do_init_channel(void);
void do_final_channel(void);

#endif /* CHANNEL_HPP */
