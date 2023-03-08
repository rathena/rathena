
#include <memory>

#include "channel_db.hpp"
#include "channel_config_loader.hpp"

// create a new ChannelConfigLoader and call its load() method
// this is the only place where ChannelConfigLoader is used
void ChannelDatabase::load() {
	auto channel_config_loader = std::make_unique<ChannelConfigLoader>(*this);
	channel_config_loader->load();
}

std::shared_ptr<Channel> ChannelDatabase::createChannel(Channel * orig) {
	auto channel = std::make_shared<Channel>(*orig);
	channels[channel->name] = channel;
	return channel;
}