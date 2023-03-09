#ifndef CHANNEL_DB_HPP
#define CHANNEL_DB_HPP

#include <memory>
#include <unordered_map>

#include "../pc.hpp"
#include "channel.hpp"
#include "channel_config.hpp"

class ChannelDatabase {
   public:
	ChannelDatabase() = default;
	~ChannelDatabase() = default;

	void load();
	void reload();
	void clear();

	const ChannelConfig& getChannelConfig() const { return config_; }
	ChannelConfig& getMutChannelConfig() { return config_; }

	std::shared_ptr<Channel> getChannel(const std::string& name) const;
	std::shared_ptr<Channel> findChannel(const std::string& name, map_session_data *sd) const;

	std::shared_ptr<Channel> createChannel(const Channel& tmp);
	std::shared_ptr<Channel> createChannelSimple(const std::string& name, const std::string& pass,
												 ChannelType chantype, unsigned int owner);

	/**
	 * A player is attempting to create a channel
	 * @param sd: Player data
	 * @param chname: Channel name
	 * @param chpass: Channel password
	 * @return 0 on success or -1 on failure
	 */
	int createChannelPC(map_session_data& sd, const std::string& name, const std::string& pass);

	/**
	 * Delete a channel
	 * - Checks if there is any user in channel and make them quit
	 * - This requires a copy of the shared ptr so we don't have memory issues
	 * @param channel: Channel
	 * @param force: Forcefully remove channel
	 * @return
	 *   0: Success
	 *  -1: Invalid channel
	 *  -2: Can't delete now
	 */
	int deleteChannel(std::shared_ptr<Channel> channel, bool force);

	/**
	 * A player is attempting to delete a channel
	 * @param sd: Player data
	 * @param name: Channel name
	 * @return 0 on success or -1 on failure
	 */
	int deleteChannelPC(map_session_data& sd, const std::string& name);

	/**
	 * A player is attempting to leave a channel
	 * @param sd: Player data
	 * @param chname: Channel name
	 * @return 0 on success or -1 on failure
	 */
	int leaveChannelPC(map_session_data& sd, const std::string& name);

	/**
	 * Make a player join the map channel
	 * - Create the map_channel if it does not exist
	 * @param sd: Player data
	 * @return
	 *  -1: Invalid player
	 *  -2: Player already in channel (channel::join)
	 *  -3: Player banned (channel::join)
	 */
	int joinMap(map_session_data& sd);

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
	int joinGuild(map_session_data& sd, int flag);

	/**
	 * Check parameters for channel creation
	 * @param name: Channel name
	 * @param pass: Channel password
	 * @param type: Check types (1 - Check name # + length, 2 - Check if channel already exists,
	 * 4 - Check password length)
	 * @return
	 *  0: Success
	 * -1: Bad channel name
	 * -2: bad channel name length
	 * -3: Password given too long
	 * -4: Channel already exists
	 */
	int checkParameters(const char* name, const char* pass, int type) const;

	/**
	 * Display some information to users in channel
	 * @param sd: Player data
	 * @param options:
	 *   colors: Display available colors for channel system
	 *   mine: List of players in channel and number of users
	 *   void: List of public channel and map and guild and number of users
	 * @return 0 on success or -1 on failure
	 */
	int displayInfo(map_session_data& sd, const char* options);

   private:
	ChannelConfig config_;
	std::unordered_map<std::string, std::shared_ptr<Channel>> channels;
	};

extern std::unique_ptr<ChannelDatabase> channel_db;

#endif /* CHANNEL_DB_HPP */
