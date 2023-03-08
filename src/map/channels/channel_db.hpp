#ifndef CHANNEL_DB_HPP
#define CHANNEL_DB_HPP

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

	const Channel_Config& getChannelConfig() const { return channel_config; }
	Channel_Config& getMutChannelConfig() { return channel_config; }

	std::shared_ptr<Channel> getChannel(const std::string& name) const;

	std::shared_ptr<Channel> createChannel(Channel * orig);

private:
	Channel_Config channel_config;
	std::unordered_map<std::string, std::shared_ptr<Channel>> channels;
};



#endif /* CHANNEL_DB_HPP */
