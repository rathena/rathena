// Copyright (C) rAthena Dev Team - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "helpers.hpp"

#include "../pc.hpp"
#include "channel_db.hpp"

int channel_pccreate(map_session_data &sd, const char *name, const char *pass) {
	char output[CHAT_SIZE_MAX];
	auto res = channel_db->checkParameters(name, pass, 1 | 2 | 4);
	switch (res) {
		case -1:
			snprintf(output, sizeof(output),
					 msg_txt(&sd, 1405));  // Channel name must start with '#'.
			clif_displaymessage(sd.fd, output);
			return -1;
		case -2:
			snprintf(output, sizeof(output), msg_txt(&sd, 1406),
					 name);	 // Channel length must be between 3 and %d.
			clif_displaymessage(sd.fd, output);
			return -1;
		case -3:
			snprintf(output, sizeof(output), msg_txt(&sd, 1436),
					 name);	 // Channel password can't be over %d characters.
			clif_displaymessage(sd.fd, output);
			return -1;
		case -4:
			snprintf(output, sizeof(output), msg_txt(&sd, 1407),
					 name);	 // Channel '%s' is not available.
			clif_displaymessage(sd.fd, output);
			return -1;
		default:
			break;
	}

	// check succeeded
	auto channel = channel_db->createChannelSimple(name, pass, ChannelType::Private, sd.status.char_id);
	channel->join(sd);
	if (channel->opt & CHAN_OPT_SELFANNOUNCE) {
		snprintf(output, sizeof(output), msg_txt(&sd, 1403), channel->name);
		clif_displaymessage(sd.fd, output);
	}
	return 0;
}

int channel_pcdelete(map_session_data &sd, const char *name) {
	if (!name || name[0] == '\0')
		return 0;

	if (!pc_has_permission(&sd, PC_PERM_CHANNEL_ADMIN))
		return -1;

	if (channel_db->checkParameters(name, nullptr, 1) != 0) {
		clif_displaymessage(sd.fd, msg_txt(&sd, 1405));	 // Channel name must start with '#'.
		return -1;
	}

	char output[CHAT_SIZE_MAX];
	auto channel = channel_db->findChannel(name, &sd);
	if (!sd.hasChannel(channel)) {
		safesnprintf(output, sizeof(output), msg_txt(&sd, 1425),
					 name);	 // You're not part of the '%s' channel.
		clif_displaymessage(sd.fd, output);
		return -1;
	}

	channel_db->deleteChannel(channel, false);
	channel = nullptr;
	safesnprintf(output, sizeof(output), msg_txt(&sd, 1448), name);
	clif_displaymessage(sd.fd, output);
	return 0;
}

int channel_pcjoin(map_session_data &sd, const char *name, const char *pass) {
	if (!name || name[0] == '\0')
		return 0;

	if (channel_db->checkParameters(name, nullptr, 1) != 0) {
		clif_displaymessage(sd.fd, msg_txt(&sd, 1405));	 // Channel name must start with '#'.
		return -1;
	}

	char output[CHAT_SIZE_MAX];
	auto channel = channel_db->findChannel(name, &sd);
	if (!channel) {
		safesnprintf(output, sizeof(output), msg_txt(&sd, 1400), name,
					 "@join");	// Unknown channel '%s' (usage: %s <#channel_name>).
		clif_displaymessage(sd.fd, output);
		return -1;
	}

	switch(channel->type) {
		case ChannelType::Ally:
			channel_db->joinGuild(sd, 1|2);
			break;
		case ChannelType::Map:
			channel_db->joinMap(sd);
			break;
		default: 
			if (channel->join(sd, pass)) {
				return -1;
			}
			break;
	}
	
	return 0;
}

int channel_pcleave(map_session_data &sd, const char *name) {
	if (!name || name[0] == '\0')
		return 0;

	if (channel_db->checkParameters(name, nullptr, 1) != 0) {
		clif_displaymessage(sd.fd, msg_txt(&sd, 1405));	 // Channel name must start with '#'.
		return -1;
	}

	char output[CHAT_SIZE_MAX];
	auto channel = channel_db->findChannel(name, &sd);
	if (!sd.hasChannel(channel)) {
		safesnprintf(output, sizeof(output), msg_txt(&sd, 1425),
					 name);	 // You're not part of the '%s' channel.
		clif_displaymessage(sd.fd, output);
		return -1;
	}

	if (!(channel->opt & CHAN_OPT_CANLEAVE)) {
		safesnprintf(output, sizeof(output), msg_txt(&sd, 726),
					 name);	 // You can't leave the '%s' channel.
		clif_displaymessage(sd.fd, output);
		return -1;
	}

	switch (channel->type) {
		case ChannelType::Ally:
			sd.quitChannels(1|2);
			break;
		case ChannelType::Map:
			sd.quitChannels(4);
			break;
		default:
			channel->leave(sd, 0);
			break;
	}
	return 0;
}

int channel_display_list(map_session_data &sd, const char *options) {
	if (!options)
		return -1;

	char output[CHAT_SIZE_MAX];
	if (strcmpi(options, "colors") == 0) {
		clif_displaymessage(sd.fd, msg_txt(&sd, 1444));
		std::for_each(channel_db->getChannelConfig().colors.begin(),
					  channel_db->getChannelConfig().colors.end(),
					  [&sd, &output](const auto &pair) {
						  snprintf(output, sizeof(output), msg_txt(&sd, 1445), pair.first.c_str());
						  clif_displaymessage(sd.fd, output);
					  });
		return 0;
	}

	if (strcmpi(options, "mine") == 0) {
		clif_displaymessage(sd.fd, msg_txt(&sd, 1475));	 // -- My Channels --
		if (sd.channels.empty()) {
			clif_displaymessage(sd.fd, msg_txt(&sd, 1476));	 // You have not joined any channels.
			return 1;
		}

		std::for_each(sd.channels.begin(), sd.channels.end(), [&sd, &output](const auto &pair) {
			snprintf(output, sizeof(output), msg_txt(&sd, 1409), channel->name,
					 channel->users.size());
			clif_displaymessage(sd.fd, output);
		});
		return 0;
	}

	bool has_perm = pc_has_permission(&sd, PC_PERM_CHANNEL_ADMIN);
	clif_displaymessage(sd.fd, msg_txt(&sd, 1410));	 // -- Public Channels --
	if (channel_db->getChannelConfig().map_tmpl.name[0]) {
		auto map_channel = map_getmapdata(sd.bl.m)->channel;
		if (map_channel) {
			snprintf(output, sizeof(output), msg_txt(&sd, 1409), map_channel->name,
					 map_channel->users.size());
			clif_displaymessage(sd.fd, output);
		}
	}

	if (channel_db->getChannelConfig().ally_tmpl.name[0] && sd.guild && sd.guild->channel) {
		snprintf(output, sizeof(output), msg_txt(&sd, 1409), sd.guild->channel->name,
				 sd.guild->channel->users.size());
		clif_displaymessage(sd.fd, output);
	}

	std::for_each(sd.channels.begin(), sd.channels.end(),
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

/**
 * A player is attempting to change the channel color
 * @param sd: Player data
 * @param chname: Channel name
 * @param color: New color
 * @return 0 on success or -1 on failure
 */
int channel_pccolor(const map_session_data &sd, char *chname, char *color) {
	struct Channel *channel;
	char output[CHAT_SIZE_MAX];
	int k;

	auto channel = channel_db->findChannel(chname, &sd);

	if (!channel) {
		sprintf(output, msg_txt(&sd, 1407), chname);	 // Channel '%s' is not available.
		clif_displaymessage(sd.fd, output);
		return -1;
	}

	if (!pc_has_permission(&sd, PC_PERM_CHANNEL_ADMIN)) {
		if (channel->char_id != sd.status.char_id) {
			sprintf(output, msg_txt(&sd, 1412), chname);	 // You're not the owner of channel '%s'.
			clif_displaymessage(sd.fd, output);
			return -1;
		} else if (!(channel->opt & CHAN_OPT_CHANGECOLOR)) {
			sprintf(output, msg_txt(&sd, 764),
					chname);  // You cannot change the color for channel '%s'.
			clif_displaymessage(sd.fd, output);
			return -1;
		}
	}

	auto col = channel_db->getChannelConfig().colors.find(color);
	if (col == channel_db->getChannelConfig().colors.end()) {
		sprintf(output, msg_txt(&sd, 1411), color);	// Unknown color '%s'.
		clif_displaymessage(sd.fd, output);
		return -1;
	}

	channel->color = col->second;
	sprintf(output, msg_txt(&sd, 1413), chname,
			col->first.c_str());  // '%s' channel color updated to '%s'.
	clif_displaymessage(sd.fd, output);
	return 0;
}
