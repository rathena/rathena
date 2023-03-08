#ifndef CHANNEL_DB_HPP
#define CHANNEL_DB_HPP

#include <memory>
#include <unordered_map>

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

	std::shared_ptr<Channel> createChannel(const Channel& tmp);
	std::shared_ptr<Channel> createChannelSimple(const std::string& name, const std::string& pass,
												 ChannelType chantype, unsigned int owner);

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

   private:
	ChannelConfig config_;
	std::unordered_map<std::string, std::shared_ptr<Channel>> channels;
};

extern std::unique_ptr<ChannelDatabase> channel_db;

#endif /* CHANNEL_DB_HPP */
