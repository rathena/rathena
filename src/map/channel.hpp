// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <unordered_set>

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>
#include "common/database.hpp"

//namespace rA {

class map_session_data;
struct guild;
struct DBMap;

#define CHAN_NAME_LENGTH 20
#define CHAN_MSG_LENGTH 150

enum e_channel_options : uint16 {
	CHAN_OPT_SELFANNOUNCE	= 1 << 1,	// Shows info when player joined/left channel to self
	CHAN_OPT_JOINANNOUNCE	= 1 << 2,// Shows info if player joined the channel
	CHAN_OPT_LEAVEANNOUNCE	= 1 << 3,	// Shows info if player left the channel
	CHAN_OPT_CHANGEDELAY 	= 1 << 4,	// Enables owner to change the chat delay
	CHAN_OPT_CHANGECOLOR	= 1 << 5,	// Enables owner to allow channel color to be changed by player's font color
	CHAN_OPT_BAN			= 1 << 6,	// Enables owner to ban players
	CHAN_OPT_KICK			= 1 << 7,	// Enables owner to kick players
	CHAN_OPT_CANCHAT		= 1 << 8,	// Allows player to chat in the channel
	CHAN_OPT_CANLEAVE		= 1 << 9,	// Allows player to leave the channel
	CHAN_OPT_AUTOJOIN		= 1 << 10,	// Player will be autojoined to the channel
};

enum Channel_Type : uint8 {
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
	uint16 opt;					  ///< Channel options @see enum Channel_Opt
	unsigned short msg_delay;	  ///< Chat delay in miliseconds
	unsigned int char_id;		  ///< If CHAN_TYPE_PRIVATE, owner is char_id of channel creator
	uint16 m;					  ///< If CHAN_TYPE_MAP, owner is map id
	int gid;					  ///< If CHAN_TYPE_ALLY, owner is first logged guild_id
	DBMap *users;				  ///< List of users
	DBMap *banned;				  ///< List of banned chars -> char_id
	std::unordered_set<uint32> groups;	// Set of group ids that can join
};

struct s_chan_banentry {
	uint32 char_id;
	char char_name[NAME_LENGTH];
};
extern s_chan_banentry chan_banentry;

struct Channel_Config {
	unsigned long *colors;		///< List of available colors
	char **colors_name;			///< Name list of available colors
	unsigned char colors_count;	///< Number of available colors

	/// Private channel default configs
	struct {
		uint16 opt;					 ///< Options @see enum Channel_Opt
		unsigned long color;		 ///< Default color
		unsigned int delay;			 ///< Message delay
		unsigned short max_member;	 ///< Max member for each channel
		unsigned allow : 1;			 ///< Allow private channel creation?
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

int channel_join(struct Channel *channel, map_session_data *sd);
int channel_mjoin(map_session_data *sd);
int channel_gjoin(map_session_data *sd, int flag);
int channel_ajoin(struct guild *g);
int channel_clean(struct Channel *channel, map_session_data *sd, int flag);
int channel_pcquit(map_session_data *sd, int type);

unsigned long channel_getColor(const char *color_str);

int channel_send(struct Channel *channel, map_session_data *sd, const char *msg);
void channel_read_config(void);

int channel_chk(char *name, char *pass, int type);
struct Channel* channel_name2channel(char *chname, map_session_data *sd, int flag);
int channel_haspc(struct Channel *channel,map_session_data *sd);
int channel_haspcbanned(struct Channel *channel,map_session_data *sd);
int channel_pc_haschan(map_session_data *sd, struct Channel *channel);
int channel_display_list(map_session_data *sd, const char *option);

void channel_autojoin(map_session_data *sd);
bool channel_pccheckgroup(struct Channel *channel, int group_id);

int channel_pccreate(map_session_data *sd, char *chname, char *pass);
int channel_pcdelete(map_session_data *sd, char *chname);
int channel_pcjoin(map_session_data *sd, char *chname, char *pass);
int channel_pcleave(map_session_data *sd, char *chname);
int channel_pccolor(map_session_data *sd, char *chname, char *color);
int channel_pcbind(map_session_data *sd, char *chname);
int channel_pcunbind(map_session_data *sd);
int channel_pcban(map_session_data *sd, char *chname, char *pname, int flag);
int channel_pckick(map_session_data *sd, char *chname, char *pname);
int channel_pcsetopt(map_session_data *sd, char *chname, const char *option, const char *val);


class ChannelConfigLoader : public YamlDatabase {
public:
	ChannelConfigLoader() : YamlDatabase("CHANNEL_CONF", 1) {}

	const std::string getDefaultLocation() override;
	bool parseBody(const ryml::NodeRef& body) override;
	bool parsePrivate(const ryml::NodeRef& node);
	bool parsePublicNode(const ryml::NodeRef& node);
	bool parseOptFlag(const ryml::NodeRef& node, const char * name, enum e_channel_options opt, uint16 &opts);

	void clear() override {};
	uint64 parseBodyNode(const ryml::NodeRef& node) override {return 0;};
};

void do_init_channel(void);
void do_final_channel(void);

#endif /* CHANNEL_HPP */
