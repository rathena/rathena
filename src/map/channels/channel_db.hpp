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

	int deleteChannel(const std::string& name, bool force);

   private:
	ChannelConfig config_;
	std::unordered_map<std::string, std::shared_ptr<Channel>> channels;
};

extern std::unique_ptr<ChannelDatabase> channel_db;

#endif /* CHANNEL_DB_HPP */
