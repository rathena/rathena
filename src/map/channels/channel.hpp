#ifndef CHANNELS_CHANNEL_HPP
#define CHANNELS_CHANNEL_HPP

#include <unordered_set>
#include <vector>

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>

#define CHAN_NAME_LENGTH 20
#define CHAN_MSG_LENGTH 150

// forward declarations
class map_session_data;

struct s_chan_banentry {
	uint32 char_id;
	char char_name[NAME_LENGTH];
};
extern s_chan_banentry chan_banentry;

enum e_channel_options : uint16 {
	CHAN_OPT_SELFANNOUNCE = 1 << 1,	  // Shows info when player joined/left channel to self
	CHAN_OPT_JOINANNOUNCE = 1 << 2,	  // Shows info if player joined the channel
	CHAN_OPT_LEAVEANNOUNCE = 1 << 3,  // Shows info if player left the channel
	CHAN_OPT_CHANGEDELAY = 1 << 4,	  // Enables owner to change the chat delay
	CHAN_OPT_CHANGECOLOR =
		1 << 5,	 // Enables owner to allow channel color to be changed by player's font color
	CHAN_OPT_BAN = 1 << 6,		  // Enables owner to ban players
	CHAN_OPT_KICK = 1 << 7,		  // Enables owner to kick players
	CHAN_OPT_CANCHAT = 1 << 8,	  // Allows player to chat in the channel
	CHAN_OPT_CANLEAVE = 1 << 9,	  // Allows player to leave the channel
	CHAN_OPT_AUTOJOIN = 1 << 10,  // Player will be autojoined to the channel
};

enum class ChannelType : uint8 {
	Public = 0,	  // Config file made
	Private = 1,  // User's channel
	Map = 2,	  // Local map
	Ally = 3,	  // Guild + its alliance
};

class Channel {
public:
	char name[CHAN_NAME_LENGTH];   // Channel Name
	char pass[CHAN_NAME_LENGTH];   // Password
	char alias[CHAN_NAME_LENGTH];  // Channel display name
	ChannelType type;		   // Channel type @see enum Channel_Type
	unsigned long color;		   // Channel color in BGR
	uint16 opt;					   // Channel options @see enum Channel_Opt
	unsigned short msg_delay;	   // Chat delay in miliseconds
	unsigned int char_id;		   // If CHAN_TYPE_PRIVATE, owner is char_id of channel creator
	uint16 m;					   // If CHAN_TYPE_MAP, owner is map id
	int gid;					   // If CHAN_TYPE_ALLY, owner is first logged guild_id
	std::vector<map_session_data *> users;	// List of users
	std::vector<s_chan_banentry> banned;	// List of banned chars -> char_id
	std::unordered_set<uint32> groups;	// Set of group ids that can join

	/**
	 * Make player leave the channel and cleanup association
	 * - If no one remains in the chat, delete it
	 * @param sd: Player data
	 * @param flag: &1 Called from deletion process, do not call delete again
	 * @return
	 * 0: Success
	 * -1: Invalid player or channel
	 */
	int clean(map_session_data *sd, int flag);
};

#endif // CHANNELS_CHANNEL_HPP