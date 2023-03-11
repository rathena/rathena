
#include <memory>

#include <common/core.hpp>
#include <common/mmo.hpp>
#include <common/showmsg.hpp>
#include <common/strlib.hpp>
#include <common/utilities.hpp>

#include "../battle.hpp"
#include "../guild.hpp"
#include "../pc.hpp"

#include "channel_db.hpp"
#include "channel_config_loader.hpp"

using namespace rathena;

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

int ChannelDatabase::deleteChannel(std::shared_ptr<Channel> channel, bool force) {
	if (channel->type == ChannelType::Public && !force && global_core->is_running())
		return -1;

	for (auto &sd : channel->users) {
		channel->leave(*sd, 1);
	}

	if (battle_config.etc_log)
		ShowInfo("Delete channel %s alias %s type=%d, owner=%d/%d/%d\n", channel->name,
				 channel->alias, channel->type, channel->char_id, channel->m, channel->gid);

	switch (channel->type) {
		case ChannelType::Ally: {
			struct guild *g = guild_search(channel->gid);
			if (g)
				g->channel = nullptr;
			break;
		}
		case ChannelType::Map:
			map_getmapdata(channel->m)->channel = nullptr;
			break;
		case ChannelType::Public:
		case ChannelType::Private:
			channels.erase(channel->name);
			break;
		default:
			ShowError("deleteChannel: unknown channel type %d", channel->type);
			return -1;
			break;
	}

	return 0;
}

int ChannelDatabase::joinMap(map_session_data &sd) {
	char output[CHAT_SIZE_MAX];

	auto mapdata = map_getmapdata(sd.bl.m);
	if (!mapdata)
		return -1;

	if (!mapdata->channel) {
		mapdata->channel = createChannelSimple("", "", ChannelType::Map, sd.bl.m);
	}

	auto ret = mapdata->channel->join(sd);

	if (ret != 0)
		return ret;

	if (mapdata->channel->opt & CHAN_OPT_SELFANNOUNCE) {
		snprintf(output, sizeof(output), msg_txt(&sd, 1435), mapdata->channel->name, mapdata->name);
		clif_messagecolor(&sd.bl, color_table[COLOR_LIGHT_GREEN], output, false, SELF);
	}

	return 0;
}

int ChannelDatabase::joinGuild(map_session_data &sd, int flag) {
	if (sd.state.autotrade)
		return -1;
	
	guild *g = sd.guild;

	if (!g)
		return -2;

	if (!g->channel) {
		// TODO(vstumpf): this requires #7612 to be merged
		// g->channel = createChannelSimple("", "", ChannelType::Ally, g->guild_id);
		g->channel->joinAlly(*g);
	}

	if (flag & 1) {
		g->channel->join(sd);
	}

	if (flag & 2) {
		for (int i = 0; i < MAX_GUILDALLIANCE; i++) {
			guild_alliance &ga = g->alliance[i];
			if (ga.guild_id && ga.opposition == 0) {
				auto ag = guild_search(ga.guild_id);
				if (ag && ag->channel) {
					ag->channel->join(sd);
				}
			}
		}
	}
	return 0;
}

int ChannelDatabase::checkParameters(const char *name, const char *pass, int type) const {
	if (type & 1) {
		if (!name)
			return -1;
		auto len = strlen(name);
		if (len < 3 || len > CHAN_NAME_LENGTH) {
			return -2;
		}
	}
	if (type & 2) {
		if (strncmpi(name + 1, config_.map_tmpl.name, CHAN_NAME_LENGTH) == 0 ||
			strncmpi(name + 1, config_.ally_tmpl.name, CHAN_NAME_LENGTH) == 0 ||
			channels.find(name) != channels.end())
			return -4;
	}

	if (type & 4) {
		if (!pass)
			return -3;
		auto len = strlen(pass);
		if (len < 3 || len > CHAN_NAME_LENGTH) {
			return -3;
		}
	}
	return 0;
}

/**
 * This is a replacement for channel_name2channel
 * This does NOT create a new channel, use the other functions for that
*/
std::shared_ptr<Channel> ChannelDatabase::findChannel(const std::string &name, const map_session_data *sd) const {
	if (checkParameters(name.c_str(), nullptr, 1) != 0)
		return nullptr;
	
	if (sd && name == config_.map_tmpl.name) {
		auto mapdata = map_getmapdata(sd->bl.m);
		if (mapdata && mapdata->channel)
			return mapdata->channel;
	}

	if (sd && name == config_.ally_tmpl.name) {
		if (sd->guild && sd->guild->channel) {
			// TODO(vstumpf): this requires #7612 to be merged, just return nullptr for now
			// return sd->guild->channel;
			return nullptr; 
		}
	}

	return getChannel(name.substr(1));
}

int ChannelDatabase::displayInfo(map_session_data &sd, const char *options) {
	if (!options)
		return -1;

	char output[CHAT_SIZE_MAX];
	if (strcmpi(options, "colors") == 0) {
		clif_displaymessage(sd.fd, msg_txt(&sd, 1444));
		std::for_each(config_.colors.begin(), config_.colors.end(),
					  [&sd, &output](const auto &pair) {
						  snprintf(output, sizeof(output), msg_txt(&sd, 1445), pair.first.c_str());
						  clif_displaymessage(sd.fd, output);
					  });
		return 0;
	}

	if (strcmpi(options, "mine") == 0) {
		clif_displaymessage(sd.fd, msg_txt(&sd, 1475)); // -- My Channels --
		if (sd.channels.empty()) {
			clif_displaymessage(sd.fd, msg_txt(&sd, 1476)); // You have not joined any channels.
			return 1;
		}

		std::for_each(sd.channels.cbegin(), sd.channels.cend(), [&sd, &output](const auto &pair) {
			const auto &channel = pair.second;
			snprintf(output, sizeof(output), msg_txt(&sd, 1409), channel->name,
					 channel->users.size());
			clif_displaymessage(sd.fd, output);
		});
		return 0;
	}

	bool has_perm = pc_has_permission(&sd, PC_PERM_CHANNEL_ADMIN);
	clif_displaymessage(sd.fd, msg_txt(&sd, 1410)); // -- Public Channels --
	if (config_.map_tmpl.name[0]) {
		auto map_channel = map_getmapdata(sd.bl.m)->channel;
		if (map_channel) {
			snprintf(output, sizeof(output), msg_txt(&sd, 1409), map_channel->name,
					 map_channel->users.size());
			clif_displaymessage(sd.fd, output);
		}
	}

	if (config_.ally_tmpl.name[0] && sd.guild && sd.guild->channel) {
		snprintf(output, sizeof(output), msg_txt(&sd, 1409), sd.guild->channel->name,
				 sd.guild->channel->users.size());
		clif_displaymessage(sd.fd, output);
	}

	std::for_each(channels.begin(), channels.end(),
				  [&sd, &output, has_perm](const auto &pair) {
					  auto channel = pair.second;
					  if (channel->type == ChannelType::Public || has_perm) {
						  snprintf(output, sizeof(output), msg_txt(&sd, 1409), channel->name,
								   channel->users.size());
						  clif_displaymessage(sd.fd, output);
					  }
				  });
	return 0;
}


// I don't like having the global object here, maybe we should add this to MapServer in map.hpp
// then we could use something like global_core->get_channel_db()
std::unique_ptr<ChannelDatabase> channel_db;
