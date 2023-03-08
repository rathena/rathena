
#include <memory>

#include <common/showmsg.hpp>
#include <common/strlib.hpp>

#include "../battle.hpp"

#include "channel_db.hpp"
#include "channel_config_loader.hpp"

// create a new ChannelConfigLoader and call its load() method
// this is the only place where ChannelConfigLoader is used
void ChannelDatabase::load() {
	auto channel_config_loader = std::make_unique<ChannelConfigLoader>(*this);
	channel_config_loader->load();
}

std::shared_ptr<Channel> ChannelDatabase::createChannel(const Channel &tmp) {
	if (!tmp.name[0])
		return nullptr;
	
	auto channel = std::make_shared<Channel>();
	channel->opt = tmp.opt;
	channel->type = tmp.type;
	channel->color = tmp.color;
	safestrncpy(channel->name, tmp.name, sizeof(channel->name));
	safestrncpy(channel->alias, tmp.alias, sizeof(channel->alias));
	if (tmp.pass[0])
		safestrncpy(channel->pass, tmp.pass, sizeof(channel->pass));
	else
		channel->pass[0] = '\0';
	channel->msg_delay = tmp.msg_delay;

	switch (channel->type) {
		case ChannelType::Public:
			channels[channel->name] = channel;
			break;
		case ChannelType::Private:
			channel->char_id = tmp.char_id;
			channels[channel->name] = channel;
			break;
		case ChannelType::Ally:
			channel->gid = tmp.gid;
			break;
		case ChannelType::Map:
			channel->m = tmp.m;
			break;
		default:
			ShowError("createChannel: unknown channel type %d", channel->type);
			return nullptr;
			break;
	}

	if (battle_config.etc_log)
		ShowInfo("Create channel %s alias %s type=%d, owner=%d/%d/%d\n", channel->name,
				 channel->alias, channel->type, channel->char_id, channel->m, channel->gid);

	return channel;
}


std::shared_ptr<Channel> ChannelDatabase::createChannelSimple(const std::string &name, const std::string &pass, ChannelType chantype, unsigned int owner) {
	Channel tmp;
	tmp.type = chantype;

	switch (chantype) {
		case ChannelType::Public:
			safestrncpy(tmp.name, name.c_str(), sizeof(tmp.name));
			safestrncpy(tmp.pass, pass.c_str(), sizeof(tmp.pass));
			safestrncpy(tmp.alias, name.c_str(), sizeof(tmp.alias));
			tmp.opt = 0;
			tmp.msg_delay = 1000;
			tmp.color = config_.getColor("Default");
			break;
		case ChannelType::Private:
			safestrncpy(tmp.name, name.c_str(), sizeof(tmp.name));
			safestrncpy(tmp.pass, pass.c_str(), sizeof(tmp.pass));
			safestrncpy(tmp.alias, name.c_str(), sizeof(tmp.alias));
			tmp.opt = config_.private_channel.opt;
			tmp.msg_delay = config_.private_channel.msg_delay;
			tmp.color = config_.private_channel.color;
			tmp.char_id = owner;
			break;
		case ChannelType::Ally:
			tmp = config_.ally_tmpl;
			tmp.gid = static_cast<int>(owner);
			break;
		case ChannelType::Map:
			tmp = config_.map_tmpl;
			tmp.m = static_cast<uint16>(owner);
			break;
		default:
			ShowError("createChannelSimple: unknown channel type %d", chantype);
			return nullptr;
	}
	return createChannel(tmp);
}

std::shared_ptr<Channel> ChannelDatabase::getChannel(const std::string &name) const {
	auto it = channels.find(name);
	if (it == channels.end())
		return nullptr;
	return it->second;
}

int ChannelDatabase::deleteChannel(const std::string &name, bool force) {
	auto it = channels.find(name);
	if (it == channels.end())
		return -1;

	auto channel = it->second;

	if (channel->type == ChannelType::Public && !force && global_core->is_running())
		return -1;

	for (auto &sd : channel->users) {
		channel->clean(sd, 1);
	}

	if (battle_config.etc_log)
		ShowInfo("Delete channel %s alias %s type=%d, owner=%d/%d/%d\n", it->second->name,
				 it->second->alias, it->second->type, it->second->char_id, it->second->m, it->second->gid);

	channels.erase(it);
	return 0;
}


// I don't like having the global object here, maybe we should add this to MapServer in map.hpp
std::unique_ptr<ChannelDatabase> channel_db;
